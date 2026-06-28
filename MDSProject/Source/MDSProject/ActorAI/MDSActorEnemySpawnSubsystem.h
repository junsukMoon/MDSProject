#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MDSActorEnemySpawnSubsystem.generated.h"

class AMDSActorEnemy;
class AMDSObjectiveActor;

UCLASS()
class MDSPROJECT_API UMDSActorEnemySpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	static constexpr int32 SpawnGridColumns = 4;
	static constexpr float SpawnGridSpacing = 150.0f;
	static constexpr float MovementTargetOffset = 1200.0f;
	static constexpr float MovementSpeed = 320.0f;
	static constexpr float ArrivalDistance = 75.0f;
	static constexpr float ObjectiveDamagePerArrival = 5.0f;

	static bool IsActorBaselineEnabled();
	static int32 GetSpawnEnemyCount();
	static FVector CalculateSpawnLocation(const FVector& SpawnOrigin, int32 SpawnIndex);
	static FVector CalculateMovementTargetLocation(const FVector& SpawnOrigin);

	void SpawnActorBaseline();
	AMDSObjectiveActor* FindOrSpawnObjective(UWorld& InWorld, const FVector& ObjectiveLocation) const;
	void HandleEnemyActiveTick(int32 ActiveDelta);
	void HandleEnemyArrived(bool bDamageApplied, const FVector& ArrivalLocation);

	bool bSpawned = false;
	int32 SpawnedEnemyCount = 0;
	int32 ActiveEnemyTickCount = 0;
	int32 ArrivedEnemyCount = 0;
	int32 DamageAppliedCount = 0;
};
