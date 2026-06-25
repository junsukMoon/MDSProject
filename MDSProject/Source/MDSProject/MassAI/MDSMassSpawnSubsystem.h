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

	bool bSpawned = false;
	int32 SpawnedEntityCount = 0;
};
