// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "MDSProjectCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "MDSProject.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/MDSDebugOverlayWidget.h"

AMDSProjectPlayerController::AMDSProjectPlayerController()
{
	bIsTouch = false;
	bMoveToMouseCursor = false;

	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;

	static ConstructorHelpers::FClassFinder<UMDSDebugOverlayWidget> DebugOverlayWidgetClassFinder(
		TEXT("/Game/MDS/UI/WBP_MDSDebugOverlay"));
	if (DebugOverlayWidgetClassFinder.Succeeded())
	{
		DebugOverlayWidgetClass = DebugOverlayWidgetClassFinder.Class;
		UE_LOG(LogMDSProject, Log, TEXT("Debug overlay widget class configured as %s."),
			*GetNameSafe(DebugOverlayWidgetClass));
	}
}

void AMDSProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		GetOrCreateDebugOverlay();
	}
}

void AMDSProjectPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Setup mouse input events
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AMDSProjectPlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AMDSProjectPlayerController::OnSetDestinationTriggered);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AMDSProjectPlayerController::OnSetDestinationReleased);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AMDSProjectPlayerController::OnSetDestinationReleased);

			// Setup touch input events
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AMDSProjectPlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AMDSProjectPlayerController::OnTouchTriggered);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AMDSProjectPlayerController::OnTouchReleased);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AMDSProjectPlayerController::OnTouchReleased);
		}
		else
		{
			UE_LOG(LogMDSProject, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		}

		InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &AMDSProjectPlayerController::ToggleDebugOverlay);
	}
}

void AMDSProjectPlayerController::OnInputStarted()
{
	StopMovement();
}

void AMDSProjectPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMDSProjectPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AMDSProjectPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AMDSProjectPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
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
		UE_LOG(LogMDSProject, Log, TEXT("Debug overlay widget class is not configured on %s."), *GetNameSafe(this));
		return nullptr;
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
