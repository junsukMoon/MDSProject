#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "MDSObjectiveActor.generated.h"

class USceneComponent;
class UWidgetComponent;

UCLASS()
class MDSPROJECT_API AMDSObjectiveActor : public AActor
{
	GENERATED_BODY()

public:
	AMDSObjectiveActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }

	bool ApplyObjectiveDamage(float DamageAmount, FName DamageSource);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentHealth();

private:
	void StartWorldUITrackingLog();
	void LogWorldUITrackingSample();

	UPROPERTY(VisibleDefaultsOnly, Category = "Objective")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = "Objective|UI")
	TObjectPtr<UWidgetComponent> ObjectiveWorldWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Objective", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleInstanceOnly, Category = "Objective")
	float CurrentHealth = 100.0f;

	FTimerHandle WorldUITrackingLogTimerHandle;
	int32 WorldUITrackingLogSamplesRemaining = 0;
};
