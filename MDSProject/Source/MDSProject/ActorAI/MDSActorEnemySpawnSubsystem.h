#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MDSActorEnemySpawnSubsystem.generated.h"

class AMDSObjectiveActor;

UCLASS()
class MDSPROJECT_API UMDSActorEnemySpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	int32 SpawnCombatEnemiesForWave(int32 EnemyCount);

private:
	static constexpr float SpawnDistanceFromObjective = 1200.0f;
	static constexpr float SpawnLaneSpacing = 180.0f;
	static constexpr float SpawnHeightOffset = 120.0f;
	static constexpr float ArrivalDistance = 150.0f;
	static constexpr float ObjectiveDamagePerArrival = 5.0f;

	static bool IsActorBaselineEnabled();
	static int32 GetSpawnEnemyCount();
	static float GetMovementSpeed();
	static FVector CalculateSpawnLocation(const FVector& ObjectiveLocation, int32 SpawnIndex);
	static FVector GetCardinalSpawnDirection(int32 SpawnIndex);

	void SpawnActorBaseline();
	AMDSObjectiveActor* FindOrSpawnObjective(UWorld& InWorld, const FVector& ObjectiveLocation) const;

	bool bSpawned = false;
	int32 SpawnedEnemyCount = 0;
};
