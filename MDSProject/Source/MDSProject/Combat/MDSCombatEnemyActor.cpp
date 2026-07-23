#include "Combat/MDSCombatEnemyActor.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MDSProjectGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Objective/MDSObjectiveActor.h"
#include "UI/MDSEnemyWorldWidget.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSCombatEnemy, Log, All);

namespace
{
const TCHAR* EnemyWorldWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSEnemyWorldUI.WBP_MDSEnemyWorldUI_C");
const TCHAR* EnemyPresentationMeshPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple");
const TCHAR* EnemyPresentationAnimClassPath = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C");
const TCHAR* HitReactionAnimationPath = TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_01.MM_HitReact_Front_Lgt_01");
const TCHAR* DeathAnimationPath = TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01.MM_Death_Front_01");
constexpr int32 WorldUITrackingSampleCount = 4;
constexpr float WorldUITrackingSampleIntervalSeconds = 1.0f;
constexpr float DeathBodyHoldSeconds = 2.0f;
constexpr float DeathFadeDurationSeconds = 1.0f;
constexpr float DeathSinkDistance = 120.0f;
constexpr float HitMovementPauseSeconds = 0.35f;
const FName PresentationSlotName = TEXT("DefaultSlot");

bool ShouldLogWorldUITracking()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSWorldUITrackingLog"));
}

bool ShouldLogEnemyCombatPresentation()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatPresentationLog"));
}

bool ShouldCaptureEnemyCombatAnimationVisibleScreenshot()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatAnimationVisibleShot"));
}

bool ShouldCaptureEnemyCombatAnimationPoseDelta()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatAnimationPoseDelta"));
}

FString GetEnemyCombatAnimationVisibleScreenshotPath(const FString& PresentationType)
{
	FString ScreenshotDirectory;
	if (!FParse::Value(FCommandLine::Get(), TEXT("-MDSCombatAnimationVisibleShotDir="), ScreenshotDirectory))
	{
		ScreenshotDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("SavedVerifyLogs"));
	}

	const FString ScreenshotFileName = FString::Printf(TEXT("MDS_CombatAnimationVisible_%s.png"), *PresentationType);
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(ScreenshotDirectory, ScreenshotFileName));
}

const TCHAR* GetMDSEnemyNetModeName(const ENetMode NetMode)
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
}

AMDSCombatEnemyActor::AMDSCombatEnemyActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicateMovement(true);

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(55.0f, 96.0f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionObjectType(ECC_Pawn);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(0.8f));

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->MaxWalkSpeed = MoveSpeed;
	Movement->MaxStepHeight = 60.0f;
	Movement->SetWalkableFloorAngle(50.0f);
	Movement->bRunPhysicsWithNoController = true;

	EnemyWorldWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyWorldWidget"));
	EnemyWorldWidgetComponent->SetupAttachment(GetCapsuleComponent());
	EnemyWorldWidgetComponent->SetWidgetClass(UMDSEnemyWorldWidget::StaticClass());
	EnemyWorldWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyWorldWidgetComponent->SetDrawAtDesiredSize(false);
	EnemyWorldWidgetComponent->SetDrawSize(FVector2D(220.0f, 48.0f));
	EnemyWorldWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	EnemyWorldWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	EnemyWorldWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnemyPresentationMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(EnemyPresentationMeshPath));
	EnemyPresentationAnimClass = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(EnemyPresentationAnimClassPath));
	HitReactionAnimation = TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(HitReactionAnimationPath));
	DeathAnimation = TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(DeathAnimationPath));
}

void AMDSCombatEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		bDeathHandled = false;
		bHasArrivedAtObjective = false;
		bDeathPresentationHandled = false;

		UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy initialized on server with %.1f / %.1f HP at %s."),
			CurrentHealth,
			MaxHealth,
			*GetActorLocation().ToCompactString());
	}

	InitializePresentationMesh();

	if (EnemyWorldWidgetComponent)
	{
		if (GetNetMode() == NM_DedicatedServer)
		{
			EnemyWorldWidgetComponent->SetVisibility(false);
		}
		else
		{
			const FSoftClassPath WidgetClassPath(EnemyWorldWidgetClassPath);
			if (UClass* LoadedWidgetClass = WidgetClassPath.TryLoadClass<UMDSEnemyWorldWidget>())
			{
				EnemyWorldWidgetComponent->SetWidgetClass(LoadedWidgetClass);
			}

			EnemyWorldWidgetComponent->InitWidget();
			if (UMDSEnemyWorldWidget* InitializedEnemyWidget = Cast<UMDSEnemyWorldWidget>(EnemyWorldWidgetComponent->GetUserWidgetObject()))
			{
				InitializedEnemyWidget->SetEnemyActor(this);
				UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy World UI widget initialized on %s using %s."),
					*GetNameSafe(this),
					*GetNameSafe(EnemyWorldWidgetComponent->GetWidgetClass()));
				StartWorldUITrackingLog();
			}
		}
	}
}

void AMDSCombatEnemyActor::InitializePresentationMesh()
{
	USkeletalMeshComponent* PresentationMesh = GetMesh();
	if (!PresentationMesh)
	{
		return;
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		PresentationMesh->SetVisibility(false);
		return;
	}

	USkeletalMesh* LoadedMesh = EnemyPresentationMesh.LoadSynchronous();
	if (LoadedMesh)
	{
		PresentationMesh->SetSkeletalMesh(LoadedMesh);
	}

	UClass* LoadedAnimClass = EnemyPresentationAnimClass.LoadSynchronous();
	if (LoadedAnimClass)
	{
		PresentationMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		PresentationMesh->SetAnimInstanceClass(LoadedAnimClass);
		PresentationMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		PresentationMesh->bPauseAnims = false;
		PresentationMesh->SetComponentTickEnabled(true);
		PresentationMesh->InitAnim(true);
	}

	UE_LOG(LogMDSCombatEnemy, Log,
		TEXT("MDS CombatAnimationPlayback | EnemyPresentationMeshInitialized | Enemy=%s | Mesh=%s | AnimClass=%s | AnimInstance=%s | AnimationMode=%s | ComponentTick=%s | PauseAnims=%s | NetMode=%s | GameplayDamage=false."),
		*GetNameSafe(this),
		*GetNameSafe(PresentationMesh->GetSkeletalMeshAsset()),
		*GetNameSafe(PresentationMesh->GetAnimClass()),
		*GetNameSafe(PresentationMesh->GetAnimInstance()),
		*UEnum::GetValueAsString(PresentationMesh->GetAnimationMode()),
		PresentationMesh->IsComponentTickEnabled() ? TEXT("true") : TEXT("false"),
		PresentationMesh->bPauseAnims ? TEXT("true") : TEXT("false"),
		GetMDSEnemyNetModeName(GetNetMode()));
}

void AMDSCombatEnemyActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDeathFadeActive)
	{
		UpdateDeathFade(DeltaSeconds);
		return;
	}

	if (!HasAuthority() || IsDead() || bHasArrivedAtObjective || bMovementPausedForHitReaction || !ObjectiveActor)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = ObjectiveActor->GetActorLocation();
	FVector ToTarget = TargetLocation - CurrentLocation;
	ToTarget.Z = 0.0f;
	const float DistanceToTarget = ToTarget.Size2D();

	if (DistanceToTarget <= ArrivalDistance)
	{
		HandleObjectiveArrivalOnce();
		return;
	}

	if (MoveSpeed <= 0.0f)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		const FVector MoveDirection = ToTarget.GetSafeNormal2D();
		AddMovementInput(MoveDirection);
		if (!bMovementDiagnosticLogged)
		{
			bMovementDiagnosticLogged = true;
			UE_LOG(LogMDSCombatEnemy, Log,
				TEXT("Combat enemy standard movement input active. Enemy=%s MovementMode=%s Direction=%s UpdatedComponent=%s RunPhysicsWithoutController=%s."),
				*GetNameSafe(this),
				*UEnum::GetValueAsString(Movement->MovementMode),
				*MoveDirection.ToCompactString(),
				*GetNameSafe(Movement->UpdatedComponent),
				Movement->bRunPhysicsWithNoController ? TEXT("true") : TEXT("false"));
		}
	}
	if (FVector::Dist2D(GetActorLocation(), TargetLocation) <= ArrivalDistance)
	{
		HandleObjectiveArrivalOnce();
	}
}

void AMDSCombatEnemyActor::StartWorldUITrackingLog()
{
	if (!EnemyWorldWidgetComponent || GetNetMode() == NM_DedicatedServer || !ShouldLogWorldUITracking())
	{
		return;
	}

	WorldUITrackingLogSamplesRemaining = WorldUITrackingSampleCount;
	LogWorldUITrackingSample();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WorldUITrackingLogTimerHandle,
			this,
			&AMDSCombatEnemyActor::LogWorldUITrackingSample,
			WorldUITrackingSampleIntervalSeconds,
			true);
	}
}

void AMDSCombatEnemyActor::LogWorldUITrackingSample()
{
	if (!EnemyWorldWidgetComponent || WorldUITrackingLogSamplesRemaining <= 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WorldUITrackingLogTimerHandle);
		}
		return;
	}

	--WorldUITrackingLogSamplesRemaining;

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	bool bProjected = false;
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		bProjected = UGameplayStatics::ProjectWorldToScreen(
			PlayerController,
			EnemyWorldWidgetComponent->GetComponentLocation(),
			ScreenPosition,
			false);
	}

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("EnemyWorldUITrack Actor=%s ActorWorld=%s WidgetWorld=%s Screen=(%.1f,%.1f) Projected=%s WidgetClass=%s."),
		*GetNameSafe(this),
		*GetActorLocation().ToCompactString(),
		*EnemyWorldWidgetComponent->GetComponentLocation().ToCompactString(),
		ScreenPosition.X,
		ScreenPosition.Y,
		bProjected ? TEXT("true") : TEXT("false"),
		*GetNameSafe(EnemyWorldWidgetComponent->GetWidgetClass()));
}

void AMDSCombatEnemyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSCombatEnemyActor, CurrentHealth);
}

void AMDSCombatEnemyActor::InitializeCombatEnemy(AMDSObjectiveActor* InObjectiveActor, const float InMoveSpeed, const float InArrivalDistance, const float InObjectiveDamageAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSCombatEnemy, Warning, TEXT("Rejected combat enemy initialization on non-authority."));
		return;
	}

	ObjectiveActor = InObjectiveActor;
	MoveSpeed = FMath::Max(0.0f, InMoveSpeed);
	ArrivalDistance = FMath::Max(1.0f, InArrivalDistance);
	ObjectiveDamageAmount = FMath::Max(0.0f, InObjectiveDamageAmount);
	bHasArrivedAtObjective = false;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bRunPhysicsWithNoController = true;
		Movement->MaxWalkSpeed = MoveSpeed;
		Movement->SetMovementMode(MOVE_Walking);
	}

	SetActorTickEnabled(ObjectiveActor != nullptr && !IsDead());

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy movement initialized on server. Target=%s MoveSpeed=%.1f ArrivalDistance=%.1f ObjectiveDamage=%.1f."),
		*GetNameSafe(ObjectiveActor),
		MoveSpeed,
		ArrivalDistance,
		ObjectiveDamageAmount);
}

bool AMDSCombatEnemyActor::ApplyEnemyDamage(const float DamageAmount, const FName DamageSource)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSCombatEnemy, Warning, TEXT("Rejected non-authority enemy damage request from %s."), *DamageSource.ToString());
		return false;
	}

	if (DamageAmount <= 0.0f || IsDead())
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy damage applied by %s: %.1f damage, HP %.1f -> %.1f."),
		*DamageSource.ToString(),
		DamageAmount,
		PreviousHealth,
		CurrentHealth);

	const bool bShouldPlayAuthorityPresentation =
		GetNetMode() == NM_Standalone || GetNetMode() == NM_ListenServer;
	if (IsDead())
	{
		if (bShouldPlayAuthorityPresentation)
		{
			RequestDeathPresentation(PreviousHealth);
		}
		HandleDeathOnce(DamageSource);
	}
	else
	{
		if (bShouldPlayAuthorityPresentation)
		{
			RequestHitPresentation(PreviousHealth);
		}
		PauseMovementForHitReaction();
	}

	return CurrentHealth < PreviousHealth;
}

void AMDSCombatEnemyActor::PauseMovementForHitReaction()
{
	if (!HasAuthority() || IsDead())
	{
		return;
	}

	bMovementPausedForHitReaction = true;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
	GetWorldTimerManager().SetTimer(
		HitMovementPauseTimerHandle,
		this,
		&AMDSCombatEnemyActor::ResumeMovementAfterHitReaction,
		HitMovementPauseSeconds,
		false);
}

void AMDSCombatEnemyActor::ResumeMovementAfterHitReaction()
{
	if (!HasAuthority() || IsDead() || bHasArrivedAtObjective)
	{
		return;
	}

	bMovementPausedForHitReaction = false;
	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy movement resumed after hit reaction. Enemy=%s PauseDuration=%.2f."),
		*GetNameSafe(this),
		HitMovementPauseSeconds);
}

void AMDSCombatEnemyActor::OnRep_CurrentHealth(const float PreviousHealth)
{
	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy HP replicated on client: %.1f / %.1f. Dead=%s."),
		CurrentHealth,
		MaxHealth,
		IsDead() ? TEXT("true") : TEXT("false"));

	if (EnemyWorldWidgetComponent)
	{
		if (UMDSEnemyWorldWidget* EnemyWidget = Cast<UMDSEnemyWorldWidget>(EnemyWorldWidgetComponent->GetUserWidgetObject()))
		{
			EnemyWidget->RefreshFromEnemy();
		}
	}

	if (GetNetMode() == NM_DedicatedServer || CurrentHealth >= PreviousHealth)
	{
		return;
	}

	if (IsDead())
	{
		RequestDeathPresentation(PreviousHealth);
	}
	else
	{
		RequestHitPresentation(PreviousHealth);
	}
}

void AMDSCombatEnemyActor::HandleDeathOnce(const FName DamageSource)
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(true);
	GetWorldTimerManager().SetTimer(
		DeathFadeDelayTimerHandle,
		this,
		&AMDSCombatEnemyActor::BeginDeathFade,
		DeathBodyHoldSeconds,
		false);
	SetLifeSpan(DeathBodyHoldSeconds + DeathFadeDurationSeconds + 0.1f);

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy death handled on server from %s at %s."),
		*DamageSource.ToString(),
		*GetActorLocation().ToCompactString());

	if (UWorld* World = GetWorld())
	{
		if (AMDSProjectGameMode* MDSGameMode = World->GetAuthGameMode<AMDSProjectGameMode>())
		{
			MDSGameMode->HandleEnemyDeathForWave();
		}
	}
}

void AMDSCombatEnemyActor::HandleObjectiveArrivalOnce()
{
	if (bHasArrivedAtObjective)
	{
		return;
	}

	bHasArrivedAtObjective = true;
	SetActorTickEnabled(false);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	bool bDamageApplied = false;
	if (ObjectiveActor)
	{
		bDamageApplied = ObjectiveActor->ApplyObjectiveDamage(ObjectiveDamageAmount, TEXT("CombatEnemyArrival"));
	}

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy arrived at objective. DamageApplied=%s Location=%s."),
		bDamageApplied ? TEXT("true") : TEXT("false"),
		*GetActorLocation().ToCompactString());
}

void AMDSCombatEnemyActor::RequestHitPresentation(const float PreviousHealth)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UAnimSequenceBase* HitAnimationAsset = HitReactionAnimation.LoadSynchronous();
	PlayEnemyAnimationPresentation(HitAnimationAsset, TEXT("Hit"), PreviousHealth, CurrentHealth);

	if (ShouldLogEnemyCombatPresentation())
	{
		UE_LOG(LogMDSCombatEnemy, Log, TEXT("MDS CombatPresentation | EnemyHitPresentationRequested | Enemy=%s | EnemyHP=%.1f->%.1f | GameplayDamage=false."),
			*GetNameSafe(this),
			PreviousHealth,
			CurrentHealth);
	}

	BP_OnHitPresentationRequested(PreviousHealth, CurrentHealth);
}

void AMDSCombatEnemyActor::RequestDeathPresentation(const float PreviousHealth)
{
	if (GetNetMode() == NM_DedicatedServer || bDeathPresentationHandled)
	{
		return;
	}

	bDeathPresentationHandled = true;

	UAnimSequenceBase* DeathAnimationAsset = DeathAnimation.LoadSynchronous();
	PlayEnemyAnimationPresentation(DeathAnimationAsset, TEXT("Death"), PreviousHealth, CurrentHealth);
	if (DeathAnimationAsset)
	{
		GetWorldTimerManager().SetTimer(
			DeathPoseFreezeTimerHandle,
			this,
			&AMDSCombatEnemyActor::FreezeDeathPose,
			FMath::Max(0.05f, DeathAnimationAsset->GetPlayLength() * 0.9f),
			false);
	}
	GetWorldTimerManager().SetTimer(
		DeathFadeDelayTimerHandle,
		this,
		&AMDSCombatEnemyActor::BeginDeathFade,
		DeathBodyHoldSeconds,
		false);

	if (ShouldLogEnemyCombatPresentation())
	{
		UE_LOG(LogMDSCombatEnemy, Log, TEXT("MDS CombatPresentation | EnemyDeathPresentationRequested | Enemy=%s | EnemyHP=%.1f->%.1f | GameplayDamage=false."),
			*GetNameSafe(this),
			PreviousHealth,
			CurrentHealth);
	}

	BP_OnDeathPresentationRequested(PreviousHealth);
}

void AMDSCombatEnemyActor::FreezeDeathPose()
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && ActiveDeathMontage)
	{
		AnimInstance->Montage_Pause(ActiveDeathMontage);
		UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy death pose frozen before fade. Enemy=%s Montage=%s."),
			*GetNameSafe(this),
			*GetNameSafe(ActiveDeathMontage));
	}
}

void AMDSCombatEnemyActor::BeginDeathFade()
{
	bDeathFadeActive = true;
	DeathFadeElapsedSeconds = 0.0f;
	SetActorTickEnabled(true);
	if (EnemyWorldWidgetComponent)
	{
		EnemyWorldWidgetComponent->SetVisibility(false);
	}

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy death fade started at %s. Duration=%.1f."),
		*GetActorLocation().ToCompactString(),
		DeathFadeDurationSeconds);
}

void AMDSCombatEnemyActor::UpdateDeathFade(const float DeltaSeconds)
{
	DeathFadeElapsedSeconds += DeltaSeconds;
	const float Alpha = FMath::Clamp(DeathFadeElapsedSeconds / DeathFadeDurationSeconds, 0.0f, 1.0f);

	if (HasAuthority())
	{
		AddActorWorldOffset(FVector(0.0f, 0.0f, -DeathSinkDistance * DeltaSeconds / DeathFadeDurationSeconds), false);
	}

	if (USkeletalMeshComponent* PresentationMesh = GetMesh())
	{
		PresentationMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 1.0f - Alpha);
		PresentationMesh->SetScalarParameterValueOnMaterials(TEXT("Fade"), Alpha);
		if (Alpha >= 1.0f)
		{
			PresentationMesh->SetVisibility(false);
		}
	}
}

void AMDSCombatEnemyActor::PlayEnemyAnimationPresentation(UAnimSequenceBase* AnimationAsset, const FName PresentationType, const float PreviousHealth, const float NewHealth)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* DynamicMontage = AnimInstance && AnimationAsset
		? AnimInstance->PlaySlotAnimationAsDynamicMontage(
			AnimationAsset,
			PresentationSlotName,
			0.1f,
			PresentationType == FName(TEXT("Death")) ? 0.0f : 0.1f)
		: nullptr;
	if (PresentationType == FName(TEXT("Death")))
	{
		ActiveDeathMontage = DynamicMontage;
	}
	const bool bPlaybackSucceeded = DynamicMontage != nullptr;
	const TCHAR* FailureReason = TEXT("None");
	if (!bPlaybackSucceeded)
	{
		if (!GetMesh())
		{
			FailureReason = TEXT("MissingMeshComponent");
		}
		else if (!AnimationAsset)
		{
			FailureReason = TEXT("AssetLoadFailed");
		}
		else if (!AnimInstance)
		{
			FailureReason = TEXT("MissingAnimInstance");
		}
		else
		{
			FailureReason = TEXT("DynamicMontagePlayFailed");
		}
	}

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("MDS CombatAnimationPlayback | Enemy%sAnimationPlaybackAttempted | Enemy=%s | Asset=%s | Mesh=%s | AnimInstance=%s | NetMode=%s | EnemyHP=%.1f->%.1f | PlaybackSucceeded=%s | FailureReason=%s | GameplayDamage=false."),
		*PresentationType.ToString(),
		*GetNameSafe(this),
		*GetNameSafe(AnimationAsset),
		*GetNameSafe(GetMesh() ? GetMesh()->GetSkeletalMeshAsset() : nullptr),
		*GetNameSafe(AnimInstance),
		GetMDSEnemyNetModeName(GetNetMode()),
		PreviousHealth,
		NewHealth,
		bPlaybackSucceeded ? TEXT("true") : TEXT("false"),
		FailureReason);

	if (bPlaybackSucceeded)
	{
		const float CaptureDelaySeconds = PresentationType == FName(TEXT("Hit")) ? 0.65f : 0.85f;
		const FName CaptureType = ShouldCaptureEnemyCombatAnimationPoseDelta()
			? FName(*FString::Printf(TEXT("%sPose"), *PresentationType.ToString()))
			: PresentationType;
		ScheduleCombatAnimationVisibleScreenshot(CaptureType, CaptureDelaySeconds);
	}
}

void AMDSCombatEnemyActor::ScheduleCombatAnimationVisibleScreenshot(const FName PresentationType, const float DelaySeconds)
{
	if (GetNetMode() == NM_DedicatedServer || !ShouldCaptureEnemyCombatAnimationVisibleScreenshot())
	{
		return;
	}

	FTimerHandle* TimerHandle = nullptr;
	if (PresentationType == FName(TEXT("Hit")) || PresentationType == FName(TEXT("HitPose")))
	{
		if (bHitVisibleScreenshotRequested)
		{
			return;
		}

		bHitVisibleScreenshotRequested = true;
		TimerHandle = &HitVisibleScreenshotTimerHandle;
	}
	else if (PresentationType == FName(TEXT("Death")) || PresentationType == FName(TEXT("DeathPose")))
	{
		if (bDeathVisibleScreenshotRequested)
		{
			return;
		}

		bDeathVisibleScreenshotRequested = true;
		TimerHandle = &DeathVisibleScreenshotTimerHandle;
	}

	if (!TimerHandle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate ScreenshotDelegate;
		ScreenshotDelegate.BindUObject(this, &AMDSCombatEnemyActor::RequestCombatAnimationVisibleScreenshot, PresentationType);
		World->GetTimerManager().SetTimer(
			*TimerHandle,
			ScreenshotDelegate,
			FMath::Max(0.0f, DelaySeconds),
			false);
	}
}

void AMDSCombatEnemyActor::RequestCombatAnimationVisibleScreenshot(const FName PresentationType)
{
	if (GetNetMode() == NM_DedicatedServer || !ShouldCaptureEnemyCombatAnimationVisibleScreenshot())
	{
		return;
	}

	const FString ScreenshotPath = GetEnemyCombatAnimationVisibleScreenshotPath(PresentationType.ToString());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(ScreenshotPath, true, false);

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("MDS CombatAnimationVisibleCapture | ScreenshotRequested | Type=%s | Enemy=%s | Path=%s | NetMode=%s."),
		*PresentationType.ToString(),
		*GetNameSafe(this),
		*ScreenshotPath,
		GetMDSEnemyNetModeName(GetNetMode()));
}
