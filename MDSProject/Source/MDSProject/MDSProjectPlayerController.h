// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "Progression/MDSLevelUpTypes.h"
#include "MDSProjectPlayerController.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class UMDSDebugOverlayWidget;
class UMDSMatchHUDWidget;
class UMDSLevelUpChoiceWidget;
class UMDSRoundSettlementWidget;
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
	void RequestLevelUpChoice(EMDSLevelUpUpgrade Upgrade);
	void RequestShopPurchase(FName ProductId);
	void RequestSettlementAction();

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
	void UpdateLevelUpChoiceUI();
	void UpdateRoundSettlementUI();
	void UpdateModalInputMode();

	UFUNCTION(Server, Reliable)
	void ServerRequestAttack(FVector_NetQuantize RequestedAimPoint);

	UFUNCTION(Server, Reliable)
	void ServerSelectLevelUpChoice(EMDSLevelUpUpgrade Upgrade);

	UFUNCTION(Server, Reliable)
	void ServerPurchaseShopProduct(FName ProductId);

	UFUNCTION(Server, Reliable)
	void ServerRequestSettlementAction();

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSDebugOverlayWidget> DebugOverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMDSMatchHUDWidget> MatchHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AttackRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Combat Presentation", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UAnimMontage> AttackPresentationMontage;

	UPROPERTY(Transient)
	TObjectPtr<UMDSDebugOverlayWidget> DebugOverlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMDSMatchHUDWidget> MatchHUDWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMDSLevelUpChoiceWidget> LevelUpChoiceWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMDSRoundSettlementWidget> RoundSettlementWidget;

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
	double AutoMoveEndTimeSeconds = 0.0;
	int32 AutoAttackAttemptsRemaining = 0;
	FString AttackRejectVerificationScenario;
	float AutoAttackRetryIntervalSeconds = 0.75f;
	float AutoMoveDurationSeconds = 3.0f;
	FVector AutoMoveWorldDirection = FVector::ForwardVector;
	bool bAttackVisibleScreenshotRequested = false;
	bool bAutoMoveDiagnosticSampleLogged = false;
	bool bAutoMoveVerificationActive = false;
	bool bAutoAttackSuspensionLogged = false;
	bool bAutoLevelUpChoiceSubmitted = false;
	bool bAutoShopPurchaseSubmitted = false;
	bool bAutoSettlementActionSubmitted = false;
	uint8 ActiveModalInputMode = 0;
};


