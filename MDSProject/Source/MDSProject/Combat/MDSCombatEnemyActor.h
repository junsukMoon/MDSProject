#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "MDSCombatEnemyActor.generated.h"

class AMDSObjectiveActor;
class USceneComponent;
class UWidgetComponent;

UCLASS()
class MDSPROJECT_API AMDSCombatEnemyActor : public AActor
{
	GENERATED_BODY()

public:
	AMDSCombatEnemyActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	void InitializeCombatEnemy(AMDSObjectiveActor* InObjectiveActor, float InMoveSpeed, float InArrivalDistance, float InObjectiveDamageAmount);
	bool ApplyEnemyDamage(float DamageAmount, FName DamageSource);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentHealth();

private:
	void HandleDeathOnce(FName DamageSource);
	void HandleObjectiveArrivalOnce();
	void StartWorldUITrackingLog();
	void LogWorldUITrackingSample();

	UPROPERTY(VisibleDefaultsOnly, Category = "Combat Enemy")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = "Combat Enemy|UI")
	TObjectPtr<UWidgetComponent> EnemyWorldWidgetComponent;

	UPROPERTY()
	TObjectPtr<AMDSObjectiveActor> ObjectiveActor;

	UPROPERTY(EditDefaultsOnly, Category = "Combat Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleInstanceOnly, Category = "Combat Enemy")
	float CurrentHealth = 100.0f;

	float MoveSpeed = 320.0f;
	float ArrivalDistance = 150.0f;
	float ObjectiveDamageAmount = 5.0f;

	bool bDeathHandled = false;
	bool bHasArrivedAtObjective = false;

	FTimerHandle WorldUITrackingLogTimerHandle;
	int32 WorldUITrackingLogSamplesRemaining = 0;
};
