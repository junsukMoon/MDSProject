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
	static constexpr int32 SpawnOnlyEntityCount = 16;
	static constexpr int32 SpawnGridColumns = 4;
	static constexpr float SpawnGridSpacing = 150.0f;
	static constexpr float SpawnDebugLifetime = 30.0f;
	static constexpr float SpawnDebugRadius = 75.0f;
	static constexpr float SpawnDebugThickness = 6.0f;

	static FVector CalculateSpawnLocation(const FVector& SpawnOrigin, int32 SpawnIndex);

	bool bSpawned = false;
	int32 SpawnedEntityCount = 0;
};
