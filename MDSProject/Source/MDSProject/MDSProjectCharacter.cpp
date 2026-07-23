// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSCombatPresentation, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogMDSCharacterMovement, Log, All);

namespace
{
const TCHAR* MoveActionPath = TEXT("/Game/TopDown/Input/Actions/IA_Move.IA_Move");
const TCHAR* AttackPresentationMontagePath = TEXT("/Game/Characters/Mannequins/Anims/Pistol/MDS_Pistol_Fire_Montage.MDS_Pistol_Fire_Montage");

bool ShouldLogCharacterCombatPresentation()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatPresentationLog"));
}

bool ShouldLogCharacterMovementVerification()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSMovementVerificationLog"));
}

const TCHAR* GetMDSCharacterNetModeName(const ENetMode NetMode)
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

const TCHAR* GetMDSCharacterRoleName(const ENetRole Role)
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

AMDSProjectCharacter::AMDSProjectCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionFinder(MoveActionPath);
	if (MoveActionFinder.Succeeded())
	{
		MoveAction = MoveActionFinder.Object;
	}

}

void AMDSProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMDSProjectCharacter::OnMoveInput(const FInputActionValue& Value)
{
	ApplyMovementInput(Value.Get<FVector2D>());
}

void AMDSProjectCharacter::ApplyMovementInput(const FVector2D& MovementInput)
{
	if (!Controller || MovementInput.IsNearlyZero())
	{
		return;
	}

	AddMovementInput(FVector::ForwardVector, MovementInput.Y);
	AddMovementInput(FVector::RightVector, MovementInput.X);
}

void AMDSProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ShouldLogCharacterMovementVerification())
	{
		MovementVerificationStartLocation = GetActorLocation();
		LastMovementVerificationLogTimeSeconds = -1000000.0;
		bMovementVerificationInitialized = true;
	}
}

void AMDSProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFireFacingLocked)
	{
		SetActorRotation(LockedFireFacingDirection.Rotation());
	}

	if (!ShouldLogCharacterMovementVerification())
	{
		return;
	}

	if (!bMovementVerificationInitialized)
	{
		MovementVerificationStartLocation = GetActorLocation();
		bMovementVerificationInitialized = true;
	}

	const UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	if (CurrentTimeSeconds - LastMovementVerificationLogTimeSeconds < 0.25)
	{
		return;
	}

	LastMovementVerificationLogTimeSeconds = CurrentTimeSeconds;
	const FVector CurrentLocation = GetActorLocation();
	const FVector CurrentVelocity = GetVelocity();
	const float DistanceFromStart = FVector::Dist2D(CurrentLocation, MovementVerificationStartLocation);
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const FString MovementModeName = MovementComponent ? MovementComponent->GetMovementName() : TEXT("None");

	UE_LOG(LogMDSCharacterMovement, Log,
		TEXT("MDS CharacterMovement | Snapshot | Actor=%s | Controller=%s | NetMode=%s | LocalRole=%s | RemoteRole=%s | LocallyControlled=%s | LocationX=%.1f | LocationY=%.1f | Speed2D=%.1f | DistanceFromStart=%.1f | MovementMode=%s."),
		*GetNameSafe(this),
		*GetNameSafe(GetController()),
		GetMDSCharacterNetModeName(GetNetMode()),
		GetMDSCharacterRoleName(GetLocalRole()),
		GetMDSCharacterRoleName(GetRemoteRole()),
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		CurrentLocation.X,
		CurrentLocation.Y,
		CurrentVelocity.Size2D(),
		DistanceFromStart,
		*MovementModeName);
}

void AMDSProjectCharacter::RequestLocalAttackPresentation(const FName PresentationSource)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ShouldLogCharacterCombatPresentation())
	{
		UE_LOG(LogMDSCombatPresentation, Log, TEXT("MDS CombatPresentation | AttackPresentationRequested | Character=%s | Source=%s."),
			*GetNameSafe(this),
			*PresentationSource.ToString());
		UE_LOG(LogMDSCombatPresentation, Log, TEXT("MDS CombatPresentation | AttackTimingMarker | Character=%s | Source=%s | GameplayDamage=false."),
			*GetNameSafe(this),
			*PresentationSource.ToString());
	}

	PlayAttackMontagePresentation(PresentationSource);
	BP_OnLocalAttackPresentationRequested(PresentationSource);
}

void AMDSProjectCharacter::BeginTemporaryFireFacing(const FVector& AimDirection, const float DurationSeconds)
{
	FVector HorizontalAimDirection = AimDirection;
	HorizontalAimDirection.Z = 0.0f;
	if (!HorizontalAimDirection.Normalize())
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
	}
	LockedFireFacingDirection = HorizontalAimDirection;
	bFireFacingLocked = true;
	SetActorRotation(LockedFireFacingDirection.Rotation());

	GetWorldTimerManager().SetTimer(
		FireFacingTimerHandle,
		this,
		&AMDSProjectCharacter::RestoreMovementFacing,
		FMath::Max(0.05f, DurationSeconds),
		false);
}

void AMDSProjectCharacter::RestoreMovementFacing()
{
	bFireFacingLocked = false;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
	}
}

void AMDSProjectCharacter::PlayShotTracerPresentation(const FVector& TraceEnd)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector TraceOrigin = GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
	const FVector ShotDirection = (TraceEnd - TraceOrigin).GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	const FVector TraceStart = TraceOrigin + ShotDirection * 55.0f;
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 220, 40), false, 0.14f, 0, 5.0f);
	DrawDebugPoint(GetWorld(), TraceStart, 14.0f, FColor::Orange, false, 0.14f);
	DrawDebugPoint(GetWorld(), TraceEnd, 10.0f, FColor::Yellow, false, 0.14f);
}

void AMDSProjectCharacter::MulticastPlayRemoteAttackPresentation_Implementation(
	const FName PresentationSource,
	const FVector_NetQuantizeNormal AimDirection,
	const FVector_NetQuantize TraceEnd,
	const float DurationSeconds)
{
	if (!IsLocallyControlled())
	{
		BeginTemporaryFireFacing(AimDirection, DurationSeconds);
		PlayShotTracerPresentation(TraceEnd);
		if (ShouldLogCharacterCombatPresentation() && GetNetMode() != NM_DedicatedServer)
		{
			UE_LOG(LogMDSCombatPresentation, Log,
				TEXT("MDS CombatPresentation | RemoteAttackPresentationReceived | Character=%s | NetMode=%s | LocalRole=%s | RemoteRole=%s | LocallyControlled=false | Direction=%s | Duration=%.2f."),
				*GetNameSafe(this),
				GetMDSCharacterNetModeName(GetNetMode()),
				GetMDSCharacterRoleName(GetLocalRole()),
				GetMDSCharacterRoleName(GetRemoteRole()),
				*AimDirection.ToCompactString(),
				DurationSeconds);
		}
	}

	if (GetNetMode() == NM_DedicatedServer || IsLocallyControlled())
	{
		return;
	}

	RequestLocalAttackPresentation(PresentationSource);
}

void AMDSProjectCharacter::PlayAttackMontagePresentation(const FName PresentationSource)
{
	UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, AttackPresentationMontagePath);
	const float PlaybackDuration = Montage ? PlayAnimMontage(Montage) : 0.0f;

	if (ShouldLogCharacterCombatPresentation())
	{
		UE_LOG(LogMDSCombatPresentation, Log,
			TEXT("MDS CombatAnimationPlayback | AttackMontagePlaybackAttempted | Character=%s | Source=%s | PlaybackSucceeded=%s | GameplayDamage=false."),
			*GetNameSafe(this),
			*PresentationSource.ToString(),
			PlaybackDuration > 0.0f ? TEXT("true") : TEXT("false"));
	}
}
