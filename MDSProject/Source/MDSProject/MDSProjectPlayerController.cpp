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
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "MDSProject.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"
#include "UI/MDSDebugOverlayWidget.h"
#include "UI/MDSMatchHUDWidget.h"
#include "UnrealClient.h"

namespace
{
const TCHAR* CursorSystemPath = TEXT("/Game/TopDown/Cursor/FX_Cursor_Success.FX_Cursor_Success");
const TCHAR* DefaultMappingContextPath = TEXT("/Game/TopDown/Input/IMC_Default.IMC_Default");
const TCHAR* ClickActionPath = TEXT("/Game/TopDown/Input/Actions/IA_SetDestination_Click.IA_SetDestination_Click");
const TCHAR* TouchActionPath = TEXT("/Game/TopDown/Input/Actions/IA_SetDestination_Touch.IA_SetDestination_Touch");
const TCHAR* DebugOverlayWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSDebugOverlay.WBP_MDSDebugOverlay_C");
const TCHAR* MatchHUDWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSMatchHUD.WBP_MDSMatchHUD_C");
}

AMDSProjectPlayerController::AMDSProjectPlayerController()
{
	bIsTouch = false;
	bMoveToMouseCursor = false;

	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	ShortPressThreshold = 0.3f;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> CursorSystemFinder(CursorSystemPath);
	if (CursorSystemFinder.Succeeded())
	{
		FXCursor = CursorSystemFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(DefaultMappingContextPath);
	if (DefaultMappingContextFinder.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ClickActionFinder(ClickActionPath);
	if (ClickActionFinder.Succeeded())
	{
		SetDestinationClickAction = ClickActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> TouchActionFinder(TouchActionPath);
	if (TouchActionFinder.Succeeded())
	{
		SetDestinationTouchAction = TouchActionFinder.Object;
	}

}

void AMDSProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
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

		if (!SetDestinationClickAction)
		{
			SetDestinationClickAction = LoadObject<UInputAction>(nullptr, ClickActionPath);
		}

		if (!SetDestinationTouchAction)
		{
			SetDestinationTouchAction = LoadObject<UInputAction>(nullptr, TouchActionPath);
		}

		if (!FXCursor)
		{
			FXCursor = LoadObject<UNiagaraSystem>(nullptr, CursorSystemPath);
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

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (SetDestinationClickAction)
			{
				// Setup mouse input events
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AMDSProjectPlayerController::OnInputStarted);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AMDSProjectPlayerController::OnSetDestinationTriggered);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AMDSProjectPlayerController::OnSetDestinationReleased);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AMDSProjectPlayerController::OnSetDestinationReleased);
			}
			else
			{
				UE_LOG(LogMDSProject, Warning, TEXT("Click input action is not configured on %s."), *GetNameSafe(this));
			}

			if (SetDestinationTouchAction)
			{
				// Setup touch input events
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AMDSProjectPlayerController::OnInputStarted);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AMDSProjectPlayerController::OnTouchTriggered);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AMDSProjectPlayerController::OnTouchReleased);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AMDSProjectPlayerController::OnTouchReleased);
			}
			else
			{
				UE_LOG(LogMDSProject, Warning, TEXT("Touch input action is not configured on %s."), *GetNameSafe(this));
			}
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
