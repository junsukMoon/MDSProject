#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MDSMassSpawnSubsystem.generated.h"

UCLASS()
class MDSPROJECT_API UMDSMassSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	int32 GetSpawnedEntityCount() const { return SpawnedEntityCount; }

private:
	static constexpr int32 SpawnGridColumns = 4;
	static constexpr float SpawnGridSpacing = 150.0f;
	static constexpr float SpawnDebugLifetime = 0.75f;
	static constexpr float SpawnDebugRadius = 45.0f;
	static constexpr float SpawnDebugThickness = 6.0f;
	static constexpr float MovementTargetOffset = 1200.0f;
	static constexpr float MovementSpeed = 320.0f;
	static constexpr float ArrivalDistance = 75.0f;
	static constexpr float ObjectiveDamagePerArrival = 5.0f;

	static bool IsMassBaselineEnabled();
	static int32 GetSpawnEntityCount();
	static FVector CalculateSpawnLocation(const FVector& SpawnOrigin, int32 SpawnIndex);
	static FVector CalculateMovementTargetLocation(const FVector& SpawnOrigin);

	bool bSpawned = false;
	int32 SpawnedEntityCount = 0;
};
