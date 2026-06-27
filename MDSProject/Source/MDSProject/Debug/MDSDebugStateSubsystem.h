#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MDSDebugStateSubsystem.generated.h"

UCLASS()
class MDSPROJECT_API UMDSDebugStateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	void SetMassSpawnedCount(int32 InSpawnedCount);
	void SetMassMovedCount(int32 InMovedCount);
	void SetMassArrivalCounts(int32 InArrivedCount, int32 InDamageAppliedCount);
	void SetObjectiveHealth(float InCurrentHealth, float InMaxHealth);

private:
	FString BuildDebugLine() const;
	FString GetNetModeLabel() const;
	void PublishDebugState();

	FTimerHandle PublishTimerHandle;

	int32 SpawnedCount = 0;
	int32 LastMovedCount = 0;
	int32 ArrivedCount = 0;
	int32 DamageAppliedCount = 0;
	float ObjectiveCurrentHealth = 0.0f;
	float ObjectiveMaxHealth = 0.0f;
};
