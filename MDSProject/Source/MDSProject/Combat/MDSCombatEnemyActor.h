#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "MDSCombatEnemyActor.generated.h"

class AMDSObjectiveActor;
class AMDSProjectPlayerState;
class UAnimInstance;
class UAnimMontage;
class UAnimSequenceBase;
class USkeletalMesh;
class UWidgetComponent;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class MDSPROJECT_API AMDSCombatEnemyActor : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDSCombatEnemyActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	void InitializeCombatEnemy(AMDSObjectiveActor* InObjectiveActor, float InMoveSpeed, float InArrivalDistance, float InObjectiveDamageAmount);
	bool ApplyEnemyDamage(float DamageAmount, FName DamageSource, AMDSProjectPlayerState* RewardRecipient = nullptr);
	bool ResolveObjectiveAttackAbility();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentHealth(float PreviousHealth);

	UFUNCTION()
	void OnRep_ObjectiveAttackState();

private:
	void HandleDeathOnce(FName DamageSource, AMDSProjectPlayerState* RewardRecipient);
	void HandleObjectiveArrivalOnce();
	void RequestObjectiveAttackAbility();
	void StartObjectiveAttackPresentation();
	void PlayObjectiveAttackPresentation();
	void StopObjectiveAttackPresentation();
	void PauseMovementForHitReaction();
	void ResumeMovementAfterHitReaction();
	void InitializePresentationMesh();
	void PlayEnemyAnimationPresentation(UAnimSequenceBase* AnimationAsset, FName PresentationType, float PreviousHealth, float NewHealth);
	void ScheduleCombatAnimationVisibleScreenshot(FName PresentationType, float DelaySeconds);
	void RequestCombatAnimationVisibleScreenshot(FName PresentationType);
	void RequestHitPresentation(float PreviousHealth);
	void RequestDeathPresentation(float PreviousHealth);
	void FreezeDeathPose();
	void BeginDeathFade();
	void UpdateDeathFade(float DeltaSeconds);
	void StartWorldUITrackingLog();
	void LogWorldUITrackingSample();

	UPROPERTY(VisibleDefaultsOnly, Category = "Combat Enemy|UI")
	TObjectPtr<UWidgetComponent> EnemyWorldWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat Enemy|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Abilities")
	TSubclassOf<UGameplayAbility> ObjectiveAttackAbilityClass;

	UPROPERTY()
	TObjectPtr<AMDSObjectiveActor> ObjectiveActor;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Presentation")
	TSoftObjectPtr<USkeletalMesh> EnemyPresentationMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Presentation")
	TSoftClassPtr<UAnimInstance> EnemyPresentationAnimClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Presentation")
	TSoftObjectPtr<UAnimSequenceBase> ObjectiveAttackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Presentation")
	TSoftObjectPtr<UAnimSequenceBase> HitReactionAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy|Presentation")
	TSoftObjectPtr<UAnimSequenceBase> DeathAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveDeathMontage;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleInstanceOnly, Category = "Combat Enemy")
	float CurrentHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ObjectiveAttackState, VisibleInstanceOnly, Category = "Combat Enemy")
	bool bIsAttackingObjective = false;

	float MoveSpeed = 320.0f;
	float ArrivalDistance = 150.0f;
	float ObjectiveDamageAmount = 5.0f;

	bool bDeathHandled = false;
	bool bHasArrivedAtObjective = false;
	bool bDeathPresentationHandled = false;
	bool bDeathFadeActive = false;
	bool bMovementDiagnosticLogged = false;
	bool bMovementPausedForHitReaction = false;
	float DeathFadeElapsedSeconds = 0.0f;

	FTimerHandle WorldUITrackingLogTimerHandle;
	FTimerHandle DeathFadeDelayTimerHandle;
	FTimerHandle DeathPoseFreezeTimerHandle;
	FTimerHandle HitMovementPauseTimerHandle;
	FTimerHandle ObjectiveDamageTimerHandle;
	FTimerHandle ObjectiveAttackPresentationTimerHandle;
	FTimerHandle HitVisibleScreenshotTimerHandle;
	FTimerHandle DeathVisibleScreenshotTimerHandle;
	FGameplayAbilitySpecHandle ObjectiveAttackAbilityHandle;
	int32 WorldUITrackingLogSamplesRemaining = 0;
	bool bHitVisibleScreenshotRequested = false;
	bool bDeathVisibleScreenshotRequested = false;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "MDS|Combat Presentation")
	void BP_OnHitPresentationRequested(float PreviousHealth, float NewHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "MDS|Combat Presentation")
	void BP_OnDeathPresentationRequested(float PreviousHealth);
};
