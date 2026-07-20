// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSCombatPresentation, Log, All);

namespace
{
bool ShouldLogCombatPresentation()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSCombatPresentationLog"));
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
}

void AMDSProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	// stub
}

void AMDSProjectCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// stub
}

void AMDSProjectCharacter::RequestLocalAttackPresentation(const FName PresentationSource)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ShouldLogCombatPresentation())
	{
		UE_LOG(LogMDSCombatPresentation, Log, TEXT("MDS CombatPresentation | AttackPresentationRequested | Character=%s | Source=%s."),
			*GetNameSafe(this),
			*PresentationSource.ToString());
		UE_LOG(LogMDSCombatPresentation, Log, TEXT("MDS CombatPresentation | AttackTimingMarker | Character=%s | Source=%s | GameplayDamage=false."),
			*GetNameSafe(this),
			*PresentationSource.ToString());
	}

	BP_OnLocalAttackPresentationRequested(PresentationSource);
}
