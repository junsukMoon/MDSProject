// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MDSProjectPlayerController.generated.h"

class UAnimMontage;
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
	// Retained for existing Blueprint serialization compatibility; desktop/touch move bindings are intentionally disabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	float ShortPressThreshold = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SetDestinationClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SetDestinationTouchAction;

public:

	/** Constructor */
	AMDSProjectPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;
	
	/** Input handlers */
	void ToggleDebugOverlay();
	void OnAttackPressed();

private:
	UMDSDebugOverlayWidget* GetOrCreateDebugOverlay();
	UMDSMatchHUDWidget* GetOrCreateMatchHUD();
	void RequestReplicatedUIViewportScreenshot();
	void ServerProcessDirectionalAttack(FVector_NetQuantize RequestedAimPoint);
	void ApplyKeyboardMovementInput();
	FVector GetAimPointFromCursor() const;
	FVector ResolvePredictedShotEnd(const FVector& AimPoint) const;
	float GetAttackFacingDuration() const;
	void ConfigureAttackFromCommandLine();
	void StartAutoAttackVerification();
	void StartAttackRejectVerification();
	void TriggerAttackRejectVerification();
	void TryAutoAttackNearestEnemy();
	class AMDSCombatEnemyActor* FindNearestAutoAttackEnemy(float& OutDistance) const;
	void RequestLocalAttackPresentation(FName PresentationSource);
	void PlayControlledPawnAttackAnimationPresentation(FName PresentationSource);
	void ScheduleCombatAnimationVisibleScreenshot(FName PresentationType, float DelaySeconds);
	void RequestCombatAnimationVisibleScreenshot(FName PresentationType);
	void StartCombatAnimationPoseDeltaBaselineCapture();
	void StartPresentationOnlyVerification();
	void TriggerPresentationOnlyAttackMarker();
	void StartAutoMoveVerification();
	void BeginAutoMoveVerification();
	void TickAutoMoveVerification();
	void StartMovementSnapshotVerification();
	void LogMovementVerificationSnapshots();

	UFUNCTION(Server, Reliable)
	void ServerRequestAttack(FVector_NetQuantize RequestedAimPoint);

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSDebugOverlayWidget> DebugOverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSMatchHUDWidget> MatchHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AttackRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackCooldownSeconds = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat Presentation", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UAnimMontage> AttackPresentationMontage;

	UPROPERTY(Transient)
	TObjectPtr<UMDSDebugOverlayWidget> DebugOverlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMDSMatchHUDWidget> MatchHUDWidget;

	FTimerHandle ReplicatedUIViewportScreenshotTimerHandle;
	FTimerHandle AttackVisibleScreenshotTimerHandle;
	FTimerHandle AttackPoseBaselineTimerHandle;
	FTimerHandle HitPoseBaselineTimerHandle;
	FTimerHandle DeathPoseBaselineTimerHandle;
	FTimerHandle AutoAttackTimerHandle;
	FTimerHandle AttackRejectVerificationTimerHandle;
	FTimerHandle PresentationOnlyAttackTimerHandle;
	FTimerHandle AutoMoveStartTimerHandle;
	FTimerHandle MovementSnapshotTimerHandle;
	TMap<TWeakObjectPtr<AActor>, FVector> MovementVerificationStartLocations;
	double LastServerAttackTimeSeconds = -1000000.0;
	double AutoMoveEndTimeSeconds = 0.0;
	int32 AutoAttackAttemptsRemaining = 0;
	FString AttackRejectVerificationScenario;
	float AutoAttackRetryIntervalSeconds = 0.75f;
	float AutoMoveDurationSeconds = 3.0f;
	FVector AutoMoveWorldDirection = FVector::ForwardVector;
	bool bAttackVisibleScreenshotRequested = false;
	bool bAutoMoveDiagnosticSampleLogged = false;
	bool bAutoMoveVerificationActive = false;
};


