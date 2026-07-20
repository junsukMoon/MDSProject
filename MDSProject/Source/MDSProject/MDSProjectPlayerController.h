// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MDSProjectPlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class UMDSDebugOverlayWidget;
class UMDSMatchHUDWidget;
class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  Player controller for a top-down perspective game.
 *  Implements point and click based controls
 */
UCLASS()
class AMDSProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SetDestinationClickAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SetDestinationTouchAction;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	/** Set to true if we're using touch input */
	uint32 bIsTouch : 1;

	/** Saved location of the character movement destination */
	FVector CachedDestination;

	/** Time that the click input has been pressed */
	float FollowTime = 0.0f;

public:

	/** Constructor */
	AMDSProjectPlayerController();

protected:
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;
	
	/** Input handlers */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();
	void ToggleDebugOverlay();
	void OnAttackPressed();

private:
	UMDSDebugOverlayWidget* GetOrCreateDebugOverlay();
	UMDSMatchHUDWidget* GetOrCreateMatchHUD();
	void RequestReplicatedUIViewportScreenshot();
	void ServerProcessAttack(AActor* RequestedTarget);
	void ConfigureAttackFromCommandLine();
	void StartAutoAttackVerification();
	void TryAutoAttackNearestEnemy();
	class AMDSCombatEnemyActor* FindNearestAutoAttackEnemy(float& OutDistance) const;
	void RequestLocalAttackPresentation(FName PresentationSource);
	void StartPresentationOnlyVerification();
	void TriggerPresentationOnlyAttackMarker();

	UFUNCTION(Server, Reliable)
	void ServerRequestAttack(AActor* RequestedTarget);

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSDebugOverlayWidget> DebugOverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSMatchHUDWidget> MatchHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AttackRange = 450.0f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackCooldownSeconds = 0.5f;

	UPROPERTY(Transient)
	TObjectPtr<UMDSDebugOverlayWidget> DebugOverlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMDSMatchHUDWidget> MatchHUDWidget;

	FTimerHandle ReplicatedUIViewportScreenshotTimerHandle;
	FTimerHandle AutoAttackTimerHandle;
	FTimerHandle PresentationOnlyAttackTimerHandle;
	double LastServerAttackTimeSeconds = -1000000.0;
	int32 AutoAttackAttemptsRemaining = 0;
	float AutoAttackRetryIntervalSeconds = 0.75f;
};


