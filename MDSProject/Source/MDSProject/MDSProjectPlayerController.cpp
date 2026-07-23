// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MDSProjectCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "MDSProject.h"
#include "Combat/MDSCombatEnemyActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"
#include "UI/MDSDebugOverlayWidget.h"
#include "UI/MDSMatchHUDWidget.h"
#include "UnrealClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSPlayerCombat, Log, All);

namespace
{
const TCHAR* DefaultMappingContextPath = TEXT("/Game/TopDown/Input/IMC_Default.IMC_Default");
const TCHAR* DebugOverlayWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSDebugOverlay.WBP_MDSDebugOverlay_C");
const TCHAR* MatchHUDWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSMatchHUD.WBP_MDSMatchHUD_C");
const TCHAR* AttackPresentationMontagePath = TEXT("/Game/Characters/Mannequins/Anims/Pistol/MDS_Pistol_Fire_Montage.MDS_Pistol_Fire_Montage");
constexpr float DirectionalAttackHitRadius = 100.0f;
constexpr float AttackFacingFallbackDuration = 0.2f;

bool ShouldLogPlayerCombatPresentation()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatPresentationLog"));
}

bool ShouldCaptureCombatAnimationVisibleScreenshot()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatAnimationVisibleShot"));
}

bool ShouldCaptureCombatAnimationPoseDelta()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatAnimationPoseDelta"));
}

FString GetCombatAnimationVisibleScreenshotPath(const FString& PresentationType)
{
	FString ScreenshotDirectory;
	if (!FParse::Value(FCommandLine::Get(), TEXT("-MDSCombatAnimationVisibleShotDir="), ScreenshotDirectory))
	{
		ScreenshotDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("SavedVerifyLogs"));
	}

	const FString ScreenshotFileName = FString::Printf(TEXT("MDS_CombatAnimationVisible_%s.png"), *PresentationType);
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(ScreenshotDirectory, ScreenshotFileName));
}

const TCHAR* GetMDSPlayerNetModeName(const ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Standalone:
		return TEXT("Standalone");
	case NM_DedicatedServer:
		return TEXT("DedicatedServer");
	case NM_ListenServer:
		return TEXT("ListenServer");
	case NM_Client:
		return TEXT("Client");
	default:
		return TEXT("Unknown");
	}
}

const TCHAR* GetMDSPlayerRoleName(const ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return TEXT("None");
	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");
	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");
	case ROLE_Authority:
		return TEXT("Authority");
	default:
		return TEXT("Unknown");
	}
}
}

AMDSProjectPlayerController::AMDSProjectPlayerController()
{
	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(DefaultMappingContextPath);
	if (DefaultMappingContextFinder.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextFinder.Object;
	}

	AttackPresentationMontage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(AttackPresentationMontagePath));
}

void AMDSProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ConfigureAttackFromCommandLine();
	StartMovementSnapshotVerification();

	if (IsLocalPlayerController())
	{
		StartCombatAnimationPoseDeltaBaselineCapture();
		GetOrCreateMatchHUD();
		GetOrCreateDebugOverlay();

		if (FParse::Param(FCommandLine::Get(), TEXT("MDSReplicatedUIViewportShot")))
		{
			GetWorldTimerManager().SetTimer(
				ReplicatedUIViewportScreenshotTimerHandle,
				this,
				&AMDSProjectPlayerController::RequestReplicatedUIViewportScreenshot,
				6.0f,
				false);
		}

		StartAutoAttackVerification();
		StartAttackRejectVerification();
		StartPresentationOnlyVerification();
		StartAutoMoveVerification();
	}
}

void AMDSProjectPlayerController::PlayerTick(float DeltaTime)
{
	ApplyKeyboardMovementInput();

	if (bAutoMoveVerificationActive)
	{
		TickAutoMoveVerification();
	}

	Super::PlayerTick(DeltaTime);
}

void AMDSProjectPlayerController::ApplyKeyboardMovementInput()
{
	if (!IsLocalPlayerController() || bAutoMoveVerificationActive)
	{
		return;
	}

	FVector2D MovementInput(
		(IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f),
		(IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f));
	MovementInput = MovementInput.GetSafeNormal();

	if (AMDSProjectCharacter* ControlledCharacter = Cast<AMDSProjectCharacter>(GetPawn()))
	{
		ControlledCharacter->ApplyMovementInput(MovementInput);
	}
}

void AMDSProjectPlayerController::StartMovementSnapshotVerification()
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("MDSMovementVerificationLog")))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		MovementSnapshotTimerHandle,
		this,
		&AMDSProjectPlayerController::LogMovementVerificationSnapshots,
		0.25f,
		true);
}

void AMDSProjectPlayerController::LogMovementVerificationSnapshots()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ACharacter> CharacterIt(World); CharacterIt; ++CharacterIt)
	{
		ACharacter* CharacterActor = *CharacterIt;
		FVector& StartLocation = MovementVerificationStartLocations.FindOrAdd(CharacterActor, CharacterActor->GetActorLocation());
		const FVector CurrentLocation = CharacterActor->GetActorLocation();
		const FVector CurrentVelocity = CharacterActor->GetVelocity();
		const UCharacterMovementComponent* MovementComponent = CharacterActor->GetCharacterMovement();
		const USceneComponent* UpdatedComponent = MovementComponent ? MovementComponent->UpdatedComponent.Get() : nullptr;

		UE_LOG(LogMDSPlayerCombat, Log,
			TEXT("MDS CharacterMovement | Snapshot | Actor=%s | Controller=%s | NetMode=%s | LocalRole=%s | RemoteRole=%s | LocallyControlled=%s | LocationX=%.1f | LocationY=%.1f | Speed2D=%.1f | DistanceFromStart=%.1f | MovementMode=%s | PendingInput=%s | LastInput=%s | MoveIgnored=%s | ComponentActive=%s | ComponentTick=%s | UpdatedComponent=%s | MaxWalkSpeed=%.1f | MaxAcceleration=%.1f."),
			*GetNameSafe(CharacterActor),
			*GetNameSafe(CharacterActor->GetController()),
			GetMDSPlayerNetModeName(CharacterActor->GetNetMode()),
			GetMDSPlayerRoleName(CharacterActor->GetLocalRole()),
			GetMDSPlayerRoleName(CharacterActor->GetRemoteRole()),
			CharacterActor->IsLocallyControlled() ? TEXT("true") : TEXT("false"),
			CurrentLocation.X,
			CurrentLocation.Y,
			CurrentVelocity.Size2D(),
			FVector::Dist2D(CurrentLocation, StartLocation),
			MovementComponent ? *MovementComponent->GetMovementName() : TEXT("None"),
			*CharacterActor->GetPendingMovementInputVector().ToCompactString(),
			*CharacterActor->GetLastMovementInputVector().ToCompactString(),
			CharacterActor->IsMoveInputIgnored() ? TEXT("true") : TEXT("false"),
			MovementComponent && MovementComponent->IsActive() ? TEXT("true") : TEXT("false"),
			MovementComponent && MovementComponent->IsComponentTickEnabled() ? TEXT("true") : TEXT("false"),
			*GetNameSafe(UpdatedComponent),
			MovementComponent ? MovementComponent->MaxWalkSpeed : 0.0f,
			MovementComponent ? MovementComponent->MaxAcceleration : 0.0f);
	}
}

void AMDSProjectPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		if (!DefaultMappingContext)
		{
			DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, DefaultMappingContextPath);
		}

		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			else
			{
				UE_LOG(LogMDSProject, Warning, TEXT("Default input mapping context is not configured on %s."), *GetNameSafe(this));
			}
		}

		InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &AMDSProjectPlayerController::ToggleDebugOverlay);
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMDSProjectPlayerController::OnAttackPressed);
	}
}

void AMDSProjectPlayerController::ToggleDebugOverlay()
{
	UMDSDebugOverlayWidget* OverlayWidget = GetOrCreateDebugOverlay();
	if (!OverlayWidget)
	{
		return;
	}

	if (OverlayWidget->IsActivated())
	{
		OverlayWidget->DeactivateWidget();
		OverlayWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	OverlayWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	OverlayWidget->ActivateWidget();
}

void AMDSProjectPlayerController::OnAttackPressed()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	if (ShouldCaptureCombatAnimationPoseDelta())
	{
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS CombatAnimationVisibleCapture | ManualAttackIgnored | Reason=PoseDeltaVerification."));
		return;
	}

	const FVector AimPoint = GetAimPointFromCursor();
	const FVector PredictedShotEnd = ResolvePredictedShotEnd(AimPoint);
	const FVector AimDirection = AimPoint - GetPawn()->GetActorLocation();
	if (AMDSProjectCharacter* ControlledCharacter = Cast<AMDSProjectCharacter>(GetPawn()))
	{
		ControlledCharacter->BeginTemporaryFireFacing(AimDirection, GetAttackFacingDuration());
		ControlledCharacter->PlayShotTracerPresentation(PredictedShotEnd);
	}

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | ClientAttackIntent | Controller=%s | Direction=%s | AimPoint=%s."),
		*GetNameSafe(this),
		*AimDirection.GetSafeNormal().ToCompactString(),
		*AimPoint.ToCompactString());

	RequestLocalAttackPresentation(TEXT("ManualInput"));
	ServerRequestAttack(AimPoint);
}

FVector AMDSProjectPlayerController::ResolvePredictedShotEnd(const FVector& AimPoint) const
{
	const APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return AimPoint;
	}

	const FVector TraceStart = ControlledPawn->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
	FVector ShotDirection2D = AimPoint - TraceStart;
	ShotDirection2D.Z = 0.0f;
	if (!ShotDirection2D.Normalize())
	{
		return AimPoint;
	}

	AMDSCombatEnemyActor* ClosestEnemy = nullptr;
	float ClosestDistanceAlongShot = AttackRange + 1.0f;
	for (TActorIterator<AMDSCombatEnemyActor> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AMDSCombatEnemyActor* Enemy = *EnemyIt;
		if (!Enemy || Enemy->IsDead())
		{
			continue;
		}

		FVector ToEnemy = Enemy->GetActorLocation() - TraceStart;
		ToEnemy.Z = 0.0f;
		const float DistanceAlongShot = FVector::DotProduct(ToEnemy, ShotDirection2D);
		const float DistanceFromShot = FVector::Dist2D(ToEnemy, ShotDirection2D * DistanceAlongShot);
		if (DistanceAlongShot >= 0.0f
			&& DistanceAlongShot <= AttackRange
			&& DistanceAlongShot < ClosestDistanceAlongShot
			&& DistanceFromShot <= DirectionalAttackHitRadius)
		{
			ClosestEnemy = Enemy;
			ClosestDistanceAlongShot = DistanceAlongShot;
		}
	}

	return ClosestEnemy ? ClosestEnemy->GetActorLocation() : AimPoint;
}

FVector AMDSProjectPlayerController::GetAimPointFromCursor() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return FVector::ForwardVector * AttackRange;
	}

	FHitResult CursorHit;
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, CursorHit))
	{
		return CursorHit.ImpactPoint;
	}

	FVector MouseWorldOrigin = FVector::ZeroVector;
	FVector MouseWorldDirection = FVector::ZeroVector;
	if (DeprojectMousePositionToWorld(MouseWorldOrigin, MouseWorldDirection))
	{
		return MouseWorldOrigin + MouseWorldDirection.GetSafeNormal() * AttackRange;
	}

	return ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * AttackRange;
}

float AMDSProjectPlayerController::GetAttackFacingDuration() const
{
	if (const UAnimMontage* Montage = AttackPresentationMontage.LoadSynchronous())
	{
		return FMath::Max(AttackFacingFallbackDuration, Montage->GetPlayLength());
	}

	return AttackFacingFallbackDuration;
}

void AMDSProjectPlayerController::ConfigureAttackFromCommandLine()
{
	bool bConfigured = false;
	float ParsedAttackDamage = AttackDamage;
	if (FParse::Value(FCommandLine::Get(), TEXT("MDSAttackDamage="), ParsedAttackDamage))
	{
		AttackDamage = FMath::Max(0.0f, ParsedAttackDamage);
		bConfigured = true;
	}

	float ParsedAttackRange = AttackRange;
	if (FParse::Value(FCommandLine::Get(), TEXT("MDSAttackRange="), ParsedAttackRange))
	{
		AttackRange = FMath::Max(1.0f, ParsedAttackRange);
		bConfigured = true;
	}

	float ParsedAttackCooldownSeconds = AttackCooldownSeconds;
	if (FParse::Value(FCommandLine::Get(), TEXT("MDSAttackCooldown="), ParsedAttackCooldownSeconds))
	{
		AttackCooldownSeconds = FMath::Max(0.0f, ParsedAttackCooldownSeconds);
		bConfigured = true;
	}

	if (bConfigured || FParse::Param(FCommandLine::Get(), TEXT("MDSAutoAttackNearestEnemy")))
	{
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | AttackConfig | Controller=%s | Damage=%.1f | Range=%.1f | Cooldown=%.2f."),
			*GetNameSafe(this),
			AttackDamage,
			AttackRange,
			AttackCooldownSeconds);
	}
}

void AMDSProjectPlayerController::StartAutoAttackVerification()
{
	if (!IsLocalPlayerController() || !FParse::Param(FCommandLine::Get(), TEXT("MDSAutoAttackNearestEnemy")))
	{
		return;
	}

	float AutoAttackDelaySeconds = 3.0f;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackDelay="), AutoAttackDelaySeconds);
	AutoAttackDelaySeconds = FMath::Max(0.0f, AutoAttackDelaySeconds);

	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackRetryInterval="), AutoAttackRetryIntervalSeconds);
	AutoAttackRetryIntervalSeconds = FMath::Max(0.1f, AutoAttackRetryIntervalSeconds);

	int32 AutoAttackCount = 4;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackCount="), AutoAttackCount);
	AutoAttackAttemptsRemaining = FMath::Max(1, AutoAttackCount);

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | AutoAttackScheduled | Controller=%s | Delay=%.2f | RetryInterval=%.2f | Attempts=%d."),
		*GetNameSafe(this),
		AutoAttackDelaySeconds,
		AutoAttackRetryIntervalSeconds,
		AutoAttackAttemptsRemaining);

	GetWorldTimerManager().SetTimer(
		AutoAttackTimerHandle,
		this,
		&AMDSProjectPlayerController::TryAutoAttackNearestEnemy,
		AutoAttackRetryIntervalSeconds,
		true,
		AutoAttackDelaySeconds);
}

void AMDSProjectPlayerController::TryAutoAttackNearestEnemy()
{
	if (!IsLocalPlayerController())
	{
		GetWorldTimerManager().ClearTimer(AutoAttackTimerHandle);
		return;
	}

	if (AutoAttackAttemptsRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(AutoAttackTimerHandle);
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | AutoAttackFinished | Controller=%s."), *GetNameSafe(this));
		return;
	}

	--AutoAttackAttemptsRemaining;

	float DistanceToTarget = 0.0f;
	AMDSCombatEnemyActor* TargetEnemy = FindNearestAutoAttackEnemy(DistanceToTarget);
	if (!TargetEnemy)
	{
		UE_LOG(LogMDSPlayerCombat, Warning, TEXT("MDS Combat | AutoAttackNoTarget | Controller=%s | AttemptsRemaining=%d."),
			*GetNameSafe(this),
			AutoAttackAttemptsRemaining);
		return;
	}

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | AutoAttackIntent | Controller=%s | Target=%s | Distance=%.1f | AttemptsRemaining=%d."),
		*GetNameSafe(this),
		*GetNameSafe(TargetEnemy),
		DistanceToTarget,
		AutoAttackAttemptsRemaining);

	RequestLocalAttackPresentation(TEXT("AutoAttack"));
	const FVector AimPoint = TargetEnemy->GetActorLocation();
	const FVector AttackDirection = AimPoint - GetPawn()->GetActorLocation();
	if (AMDSProjectCharacter* ControlledCharacter = Cast<AMDSProjectCharacter>(GetPawn()))
	{
		ControlledCharacter->BeginTemporaryFireFacing(AttackDirection, GetAttackFacingDuration());
		ControlledCharacter->PlayShotTracerPresentation(AimPoint);
	}
	ServerRequestAttack(AimPoint);
}

void AMDSProjectPlayerController::RequestLocalAttackPresentation(const FName PresentationSource)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (ShouldLogPlayerCombatPresentation())
	{
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS CombatPresentation | AttackPresentationRequested | Controller=%s | Pawn=%s | Source=%s."),
			*GetNameSafe(this),
			*GetNameSafe(ControlledPawn),
			*PresentationSource.ToString());
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS CombatPresentation | AttackTimingMarker | Controller=%s | Pawn=%s | Source=%s | GameplayDamage=false."),
			*GetNameSafe(this),
			*GetNameSafe(ControlledPawn),
			*PresentationSource.ToString());
	}

	if (AMDSProjectCharacter* MDSCharacter = Cast<AMDSProjectCharacter>(GetPawn()))
	{
		MDSCharacter->RequestLocalAttackPresentation(PresentationSource);
	}
}

void AMDSProjectPlayerController::StartAttackRejectVerification()
{
	if (!IsLocalPlayerController()
		|| !FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackReject="), AttackRejectVerificationScenario))
	{
		return;
	}

	float VerificationDelaySeconds = 4.0f;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackRejectDelay="), VerificationDelaySeconds);
	VerificationDelaySeconds = FMath::Max(0.0f, VerificationDelaySeconds);

	UE_LOG(LogMDSPlayerCombat, Log,
		TEXT("MDS Combat | AttackRejectVerificationScheduled | Controller=%s | Scenario=%s | Delay=%.2f."),
		*GetNameSafe(this),
		*AttackRejectVerificationScenario,
		VerificationDelaySeconds);

	GetWorldTimerManager().SetTimer(
		AttackRejectVerificationTimerHandle,
		this,
		&AMDSProjectPlayerController::TriggerAttackRejectVerification,
		VerificationDelaySeconds,
		false);
}

void AMDSProjectPlayerController::TriggerAttackRejectVerification()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const bool bInvalidDirection = AttackRejectVerificationScenario.Equals(TEXT("InvalidDirection"), ESearchCase::IgnoreCase);
	const FVector PawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
	const FVector TraceStart = PawnLocation + FVector(0.0f, 0.0f, 65.0f);
	const FVector RequestedAimPoint = bInvalidDirection ? TraceStart : TraceStart + FVector::ForwardVector * AttackRange;
	UE_LOG(LogMDSPlayerCombat, Log,
		TEXT("MDS Combat | AttackRejectVerificationIntent | Controller=%s | Scenario=%s | AimPoint=%s."),
		*GetNameSafe(this),
		*AttackRejectVerificationScenario,
		*RequestedAimPoint.ToCompactString());
	ServerRequestAttack(RequestedAimPoint);
}

void AMDSProjectPlayerController::PlayControlledPawnAttackAnimationPresentation(const FName PresentationSource)
{
	if (!IsLocalPlayerController() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	USkeletalMeshComponent* CharacterMesh = ControlledCharacter ? ControlledCharacter->GetMesh() : nullptr;
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	UAnimMontage* Montage = AttackPresentationMontage.LoadSynchronous();
	const float PlaybackDuration = ControlledCharacter && Montage ? ControlledCharacter->PlayAnimMontage(Montage) : 0.0f;
	const bool bPlaybackSucceeded = PlaybackDuration > 0.0f;
	const TCHAR* FailureReason = TEXT("None");
	if (!bPlaybackSucceeded)
	{
		if (!ControlledCharacter)
		{
			FailureReason = TEXT("ControlledPawnIsNotCharacter");
		}
		else if (!Montage)
		{
			FailureReason = TEXT("AssetLoadFailed");
		}
		else if (!CharacterMesh)
		{
			FailureReason = TEXT("MissingMesh");
		}
		else if (!AnimInstance)
		{
			FailureReason = TEXT("MissingAnimInstance");
		}
		else
		{
			FailureReason = TEXT("MontagePlayReturnedZero");
		}
	}

	if (ShouldLogPlayerCombatPresentation())
	{
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS CombatAnimationPlayback | AttackMontagePlaybackAttempted | Controller=%s | Pawn=%s | Source=%s | Asset=%s | Mesh=%s | AnimInstance=%s | NetMode=%s | Duration=%.3f | PlaybackSucceeded=%s | FailureReason=%s | GameplayDamage=false."),
			*GetNameSafe(this),
			*GetNameSafe(ControlledCharacter),
			*PresentationSource.ToString(),
			*GetNameSafe(Montage),
			*GetNameSafe(CharacterMesh),
			*GetNameSafe(AnimInstance),
			GetMDSPlayerNetModeName(GetNetMode()),
			PlaybackDuration,
			bPlaybackSucceeded ? TEXT("true") : TEXT("false"),
			FailureReason);
	}

	if (bPlaybackSucceeded)
	{
		ScheduleCombatAnimationVisibleScreenshot(
			ShouldCaptureCombatAnimationPoseDelta() ? TEXT("AttackPose") : TEXT("Attack"),
			0.15f);
	}
}

void AMDSProjectPlayerController::StartCombatAnimationPoseDeltaBaselineCapture()
{
	if (!IsLocalPlayerController() || GetNetMode() == NM_DedicatedServer || !ShouldCaptureCombatAnimationPoseDelta())
	{
		return;
	}

	struct FBaselineCapture
	{
		FTimerHandle* TimerHandle;
		FName Type;
		float DelaySeconds;
	};

	const FBaselineCapture Captures[] = {
		{ &AttackPoseBaselineTimerHandle, TEXT("AttackBefore"), 2.0f },
		{ &HitPoseBaselineTimerHandle, TEXT("HitBefore"), 2.3f },
		{ &DeathPoseBaselineTimerHandle, TEXT("DeathBefore"), 2.6f }
	};

	for (const FBaselineCapture& Capture : Captures)
	{
		FTimerDelegate ScreenshotDelegate;
		ScreenshotDelegate.BindUObject(this, &AMDSProjectPlayerController::RequestCombatAnimationVisibleScreenshot, Capture.Type);
		GetWorldTimerManager().SetTimer(*Capture.TimerHandle, ScreenshotDelegate, Capture.DelaySeconds, false);
	}
}

void AMDSProjectPlayerController::ScheduleCombatAnimationVisibleScreenshot(const FName PresentationType, const float DelaySeconds)
{
	if (!IsLocalPlayerController() || GetNetMode() == NM_DedicatedServer || !ShouldCaptureCombatAnimationVisibleScreenshot())
	{
		return;
	}

	if (PresentationType == FName(TEXT("Attack")) || PresentationType == FName(TEXT("AttackPose")))
	{
		if (bAttackVisibleScreenshotRequested)
		{
			return;
		}

		bAttackVisibleScreenshotRequested = true;
	}

	FTimerDelegate ScreenshotDelegate;
	ScreenshotDelegate.BindUObject(this, &AMDSProjectPlayerController::RequestCombatAnimationVisibleScreenshot, PresentationType);
	GetWorldTimerManager().SetTimer(
		AttackVisibleScreenshotTimerHandle,
		ScreenshotDelegate,
		FMath::Max(0.0f, DelaySeconds),
		false);
}

void AMDSProjectPlayerController::RequestCombatAnimationVisibleScreenshot(const FName PresentationType)
{
	if (!IsLocalPlayerController() || GetNetMode() == NM_DedicatedServer || !ShouldCaptureCombatAnimationVisibleScreenshot())
	{
		return;
	}

	const FString ScreenshotPath = GetCombatAnimationVisibleScreenshotPath(PresentationType.ToString());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(ScreenshotPath, true, false);

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS CombatAnimationVisibleCapture | ScreenshotRequested | Type=%s | Path=%s | NetMode=%s."),
		*PresentationType.ToString(),
		*ScreenshotPath,
		GetMDSPlayerNetModeName(GetNetMode()));
}

void AMDSProjectPlayerController::StartPresentationOnlyVerification()
{
	if (!IsLocalPlayerController() || !FParse::Param(FCommandLine::Get(), TEXT("MDSPresentationOnlyAttackMarker")))
	{
		return;
	}

	float PresentationOnlyDelaySeconds = 3.0f;
	FParse::Value(FCommandLine::Get(), TEXT("MDSPresentationOnlyAttackDelay="), PresentationOnlyDelaySeconds);
	PresentationOnlyDelaySeconds = FMath::Max(0.0f, PresentationOnlyDelaySeconds);

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | PresentationOnlyAttackScheduled | Controller=%s | Delay=%.2f."),
		*GetNameSafe(this),
		PresentationOnlyDelaySeconds);

	GetWorldTimerManager().SetTimer(
		PresentationOnlyAttackTimerHandle,
		this,
		&AMDSProjectPlayerController::TriggerPresentationOnlyAttackMarker,
		PresentationOnlyDelaySeconds,
		false);
}

void AMDSProjectPlayerController::TriggerPresentationOnlyAttackMarker()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | PresentationOnlyAttackMarker | Controller=%s | ServerRequestSent=false."),
		*GetNameSafe(this));
	RequestLocalAttackPresentation(TEXT("PresentationOnly"));
}

void AMDSProjectPlayerController::StartAutoMoveVerification()
{
	if (!IsLocalPlayerController() || !FParse::Param(FCommandLine::Get(), TEXT("MDSAutoMoveVerification")))
	{
		return;
	}

	float StartDelaySeconds = 6.0f;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoMoveDelay="), StartDelaySeconds);
	StartDelaySeconds = FMath::Max(0.0f, StartDelaySeconds);

	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoMoveDuration="), AutoMoveDurationSeconds);
	AutoMoveDurationSeconds = FMath::Max(0.25f, AutoMoveDurationSeconds);

	float DirectionX = 0.0f;
	float DirectionY = 1.0f;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoMoveDirectionX="), DirectionX);
	FParse::Value(FCommandLine::Get(), TEXT("MDSAutoMoveDirectionY="), DirectionY);
	AutoMoveWorldDirection = FVector(DirectionX, DirectionY, 0.0f).GetSafeNormal();
	if (AutoMoveWorldDirection.IsNearlyZero())
	{
		AutoMoveWorldDirection = FVector::RightVector;
	}

	UE_LOG(LogMDSPlayerCombat, Log,
		TEXT("MDS CharacterMovement | AutoMoveScheduled | Controller=%s | Delay=%.2f | Duration=%.2f | DirectionX=%.2f | DirectionY=%.2f."),
		*GetNameSafe(this),
		StartDelaySeconds,
		AutoMoveDurationSeconds,
		AutoMoveWorldDirection.X,
		AutoMoveWorldDirection.Y);

	GetWorldTimerManager().SetTimer(
		AutoMoveStartTimerHandle,
		this,
		&AMDSProjectPlayerController::BeginAutoMoveVerification,
		StartDelaySeconds,
		false);
}

void AMDSProjectPlayerController::BeginAutoMoveVerification()
{
	if (!IsLocalPlayerController() || !GetPawn())
	{
		UE_LOG(LogMDSPlayerCombat, Warning,
			TEXT("MDS CharacterMovement | AutoMoveFailed | Controller=%s | Reason=MissingLocalPawn."),
			*GetNameSafe(this));
		return;
	}

	UWorld* World = GetWorld();
	AutoMoveEndTimeSeconds = (World ? World->GetTimeSeconds() : 0.0) + AutoMoveDurationSeconds;
	bAutoMoveDiagnosticSampleLogged = false;
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	const UCharacterMovementComponent* MovementComponent = ControlledCharacter ? ControlledCharacter->GetCharacterMovement() : nullptr;
	UE_LOG(LogMDSPlayerCombat, Log,
		TEXT("MDS CharacterMovement | AutoMoveStarted | Controller=%s | Pawn=%s | PawnClass=%s | NativeParent=%s | StartX=%.1f | StartY=%.1f | MoveIgnored=%s | Component=%s | ComponentActive=%s | ComponentTick=%s | UpdatedComponent=%s | MaxWalkSpeed=%.1f | MaxAcceleration=%.1f."),
		*GetNameSafe(this),
		*GetNameSafe(GetPawn()),
		*GetNameSafe(GetPawn()->GetClass()),
		*GetNameSafe(GetPawn()->GetClass()->GetSuperClass()),
		GetPawn()->GetActorLocation().X,
		GetPawn()->GetActorLocation().Y,
		GetPawn()->IsMoveInputIgnored() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(MovementComponent),
		MovementComponent && MovementComponent->IsActive() ? TEXT("true") : TEXT("false"),
		MovementComponent && MovementComponent->IsComponentTickEnabled() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(MovementComponent ? MovementComponent->UpdatedComponent : nullptr),
		MovementComponent ? MovementComponent->MaxWalkSpeed : 0.0f,
		MovementComponent ? MovementComponent->MaxAcceleration : 0.0f);

	bAutoMoveVerificationActive = true;
}

void AMDSProjectPlayerController::TickAutoMoveVerification()
{
	APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	if (!IsLocalPlayerController() || !ControlledPawn || CurrentTimeSeconds >= AutoMoveEndTimeSeconds)
	{
		bAutoMoveVerificationActive = false;
		UE_LOG(LogMDSPlayerCombat, Log,
			TEXT("MDS CharacterMovement | AutoMoveFinished | Controller=%s | Pawn=%s | EndX=%.1f | EndY=%.1f."),
			*GetNameSafe(this),
			*GetNameSafe(ControlledPawn),
			ControlledPawn ? ControlledPawn->GetActorLocation().X : 0.0f,
			ControlledPawn ? ControlledPawn->GetActorLocation().Y : 0.0f);
		return;
	}

	const FVector PendingInputBefore = ControlledPawn->GetPendingMovementInputVector();
	if (AMDSProjectCharacter* ControlledCharacter = Cast<AMDSProjectCharacter>(ControlledPawn))
	{
		ControlledCharacter->ApplyMovementInput(FVector2D(0.0f, 1.0f));
	}
	if (!bAutoMoveDiagnosticSampleLogged)
	{
		bAutoMoveDiagnosticSampleLogged = true;
		UE_LOG(LogMDSPlayerCombat, Log,
			TEXT("MDS CharacterMovement | InputInjected | Pawn=%s | Before=%s | After=%s | LastInput=%s | MoveIgnored=%s."),
			*GetNameSafe(ControlledPawn),
			*PendingInputBefore.ToCompactString(),
			*ControlledPawn->GetPendingMovementInputVector().ToCompactString(),
			*ControlledPawn->GetLastMovementInputVector().ToCompactString(),
			ControlledPawn->IsMoveInputIgnored() ? TEXT("true") : TEXT("false"));
	}
}

AMDSCombatEnemyActor* AMDSProjectPlayerController::FindNearestAutoAttackEnemy(float& OutDistance) const
{
	OutDistance = 0.0f;

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const APawn* RequestingPawn = GetPawn();
	const FVector SearchOrigin = RequestingPawn ? RequestingPawn->GetActorLocation() : GetFocalLocation();
	AMDSCombatEnemyActor* BestEnemy = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<AMDSCombatEnemyActor> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AMDSCombatEnemyActor* Enemy = *EnemyIt;
		if (!Enemy || Enemy->IsDead())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(SearchOrigin, Enemy->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestEnemy = Enemy;
		}
	}

	if (BestEnemy)
	{
		OutDistance = FMath::Sqrt(BestDistanceSquared);
	}

	return BestEnemy;
}

void AMDSProjectPlayerController::ServerRequestAttack_Implementation(const FVector_NetQuantize RequestedAimPoint)
{
	FString RejectVerificationScenario;
	FVector ServerAimPoint = RequestedAimPoint;
	if (FParse::Value(FCommandLine::Get(), TEXT("MDSAutoAttackReject="), RejectVerificationScenario))
	{
		if (RejectVerificationScenario.Equals(TEXT("NoPawn"), ESearchCase::IgnoreCase))
		{
			UnPossess();
		}
		else if (RejectVerificationScenario.Equals(TEXT("InvalidDirection"), ESearchCase::IgnoreCase) && GetPawn())
		{
			ServerAimPoint = GetPawn()->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
		}
	}

	ServerProcessDirectionalAttack(ServerAimPoint);
}

void AMDSProjectPlayerController::ServerProcessDirectionalAttack(const FVector_NetQuantize RequestedAimPoint)
{
	APawn* RequestingPawn = GetPawn();
	if (!RequestingPawn)
	{
		UE_LOG(LogMDSPlayerCombat, Warning, TEXT("MDS Combat | ServerAttackRejected | Reason=NoPawn | Controller=%s."),
			*GetNameSafe(this));
		return;
	}

	const FVector TraceStart = RequestingPawn->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
	FVector ShotDirection2D = RequestedAimPoint - TraceStart;
	ShotDirection2D.Z = 0.0f;
	if (!ShotDirection2D.Normalize())
	{
		UE_LOG(LogMDSPlayerCombat, Warning, TEXT("MDS Combat | ServerAttackRejected | Reason=InvalidDirection | Requester=%s."),
			*GetNameSafe(RequestingPawn));
		return;
	}
	const FVector ShotDirection = ShotDirection2D;
	const float TraceDistance = AttackRange;

	if (AttackDamage <= 0.0f)
	{
		UE_LOG(LogMDSPlayerCombat, Warning, TEXT("MDS Combat | ServerAttackRejected | Reason=InvalidDamage | Requester=%s | Damage=%.1f."),
			*GetNameSafe(RequestingPawn),
			AttackDamage);
		return;
	}

	UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	const double CooldownRemaining = LastServerAttackTimeSeconds + AttackCooldownSeconds - CurrentTimeSeconds;
	if (CooldownRemaining > 0.0)
	{
		UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | ServerAttackRejected | Reason=Cooldown | Requester=%s | CooldownRemaining=%.2f."),
			*GetNameSafe(RequestingPawn),
			CooldownRemaining);
		return;
	}

	AMDSCombatEnemyActor* TargetEnemy = nullptr;
	float ClosestDistanceAlongShot = TraceDistance + 1.0f;
	if (World)
	{
		for (TActorIterator<AMDSCombatEnemyActor> EnemyIt(World); EnemyIt; ++EnemyIt)
		{
			AMDSCombatEnemyActor* CandidateEnemy = *EnemyIt;
			if (!CandidateEnemy || CandidateEnemy->IsDead())
			{
				continue;
			}

			FVector ToEnemy = CandidateEnemy->GetActorLocation() - TraceStart;
			ToEnemy.Z = 0.0f;
			const float DistanceAlongShot = FVector::DotProduct(ToEnemy, ShotDirection);
			if (DistanceAlongShot < 0.0f || DistanceAlongShot > TraceDistance || DistanceAlongShot >= ClosestDistanceAlongShot)
			{
				continue;
			}

			const FVector ClosestPointOnShot = ShotDirection * DistanceAlongShot;
			const float DistanceFromShot = FVector::Dist2D(ToEnemy, ClosestPointOnShot);
			if (DistanceFromShot <= DirectionalAttackHitRadius)
			{
				TargetEnemy = CandidateEnemy;
				ClosestDistanceAlongShot = DistanceAlongShot;
			}
		}
	}
	const FVector TraceEnd = TargetEnemy ? TargetEnemy->GetActorLocation() : FVector(RequestedAimPoint);

	LastServerAttackTimeSeconds = CurrentTimeSeconds;
	if (AMDSProjectCharacter* RequestingCharacter = Cast<AMDSProjectCharacter>(RequestingPawn))
	{
		RequestingCharacter->MulticastPlayRemoteAttackPresentation(
			TEXT("ServerDirectionalFire"),
			ShotDirection,
			TraceEnd,
			GetAttackFacingDuration());
	}

	if (!TargetEnemy || TargetEnemy->IsDead())
	{
		UE_LOG(LogMDSPlayerCombat, Log,
			TEXT("MDS Combat | ServerAttackResolved | Requester=%s | Target=%s | Valid=true | Hit=false | Direction=%s | TraceEnd=%s | Range=%.1f | HitRadius=%.1f | DamageApplied=false."),
			*GetNameSafe(RequestingPawn),
			*GetNameSafe(TargetEnemy),
			*ShotDirection.ToCompactString(),
			*TraceEnd.ToCompactString(),
			AttackRange,
			DirectionalAttackHitRadius);
		return;
	}

	const float PreviousHealth = TargetEnemy->GetCurrentHealth();
	const bool bDamageApplied = TargetEnemy->ApplyEnemyDamage(AttackDamage, TEXT("PlayerAttack"));
	const float NewHealth = TargetEnemy->GetCurrentHealth();

	UE_LOG(LogMDSPlayerCombat, Log, TEXT("MDS Combat | ServerAttackResolved | Requester=%s | Target=%s | Valid=true | Hit=true | Direction=%s | TraceEnd=%s | DamageApplied=%s | Damage=%.1f | EnemyHP=%.1f->%.1f."),
		*GetNameSafe(RequestingPawn),
		*GetNameSafe(TargetEnemy),
		*ShotDirection.ToCompactString(),
		*TraceEnd.ToCompactString(),
		bDamageApplied ? TEXT("true") : TEXT("false"),
		AttackDamage,
		PreviousHealth,
		NewHealth);
}

UMDSDebugOverlayWidget* AMDSProjectPlayerController::GetOrCreateDebugOverlay()
{
	if (!IsLocalPlayerController())
	{
		return nullptr;
	}

	if (DebugOverlayWidget)
	{
		return DebugOverlayWidget;
	}

	if (!DebugOverlayWidgetClass)
	{
		const FSoftClassPath WidgetClassPath(DebugOverlayWidgetClassPath);
		if (UClass* LoadedWidgetClass = WidgetClassPath.TryLoadClass<UMDSDebugOverlayWidget>())
		{
			DebugOverlayWidgetClass = LoadedWidgetClass;
			UE_LOG(LogMDSProject, Log, TEXT("Debug overlay widget class configured as %s."),
				*GetNameSafe(DebugOverlayWidgetClass));
		}
		else
		{
			UE_LOG(LogMDSProject, Log, TEXT("Debug overlay widget class is not configured on %s."), *GetNameSafe(this));
			return nullptr;
		}
	}

	DebugOverlayWidget = CreateWidget<UMDSDebugOverlayWidget>(this, DebugOverlayWidgetClass);
	if (DebugOverlayWidget)
	{
		DebugOverlayWidget->AddToViewport(10);
		DebugOverlayWidget->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogMDSProject, Log, TEXT("Debug overlay widget created on %s using %s."),
			*GetNameSafe(this),
			*GetNameSafe(DebugOverlayWidgetClass));
	}
	else
	{
		UE_LOG(LogMDSProject, Warning, TEXT("Debug overlay widget creation failed on %s using %s."),
			*GetNameSafe(this),
			*GetNameSafe(DebugOverlayWidgetClass));
	}

	return DebugOverlayWidget;
}

UMDSMatchHUDWidget* AMDSProjectPlayerController::GetOrCreateMatchHUD()
{
	if (!IsLocalPlayerController())
	{
		return nullptr;
	}

	if (MatchHUDWidget)
	{
		return MatchHUDWidget;
	}

	if (MatchHUDWidgetClass)
	{
		UE_LOG(LogMDSProject, Log, TEXT("MDS Match HUD widget class configured as %s."),
			*GetNameSafe(MatchHUDWidgetClass));
	}
	else
	{
		const FSoftClassPath WidgetClassPath(MatchHUDWidgetClassPath);
		if (UClass* LoadedWidgetClass = WidgetClassPath.TryLoadClass<UMDSMatchHUDWidget>())
		{
			MatchHUDWidgetClass = LoadedWidgetClass;
			UE_LOG(LogMDSProject, Log, TEXT("MDS Match HUD widget class configured as %s."),
				*GetNameSafe(MatchHUDWidgetClass));
		}
	}

	TSubclassOf<UMDSMatchHUDWidget> WidgetClass = UMDSMatchHUDWidget::StaticClass();
	if (MatchHUDWidgetClass)
	{
		WidgetClass = MatchHUDWidgetClass;
	}
	MatchHUDWidget = CreateWidget<UMDSMatchHUDWidget>(this, WidgetClass);
	if (MatchHUDWidget)
	{
		MatchHUDWidget->AddToPlayerScreen(5);
		MatchHUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		MatchHUDWidget->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
		MatchHUDWidget->SetPositionInViewport(FVector2D(24.0f, 24.0f), false);
		MatchHUDWidget->SetDesiredSizeInViewport(FVector2D(320.0f, 80.0f));
		UE_LOG(LogMDSProject, Log, TEXT("MDS Match HUD widget created on %s using %s."),
			*GetNameSafe(this),
			*GetNameSafe(WidgetClass));
	}
	else
	{
		UE_LOG(LogMDSProject, Warning, TEXT("MDS Match HUD widget creation failed on %s using %s."),
			*GetNameSafe(this),
			*GetNameSafe(WidgetClass));
	}

	return MatchHUDWidget;
}

void AMDSProjectPlayerController::RequestReplicatedUIViewportScreenshot()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FString ScreenshotPath;
	if (!FParse::Value(FCommandLine::Get(), TEXT("-MDSReplicatedUIViewportShotPath="), ScreenshotPath))
	{
		const FString ScreenshotDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("SavedVerifyLogs"));
		ScreenshotPath = FPaths::Combine(ScreenshotDirectory, TEXT("MDS_ReplicatedUIViewport_Client_EngineShot.png"));
	}

	ScreenshotPath = FPaths::ConvertRelativePathToFull(ScreenshotPath);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(ScreenshotPath, true, false);

	UE_LOG(LogMDSProject, Log, TEXT("MDS replicated UI viewport screenshot requested: %s"), *ScreenshotPath);
}
