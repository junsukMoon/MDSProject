#pragma once

#include "CoreMinimal.h"
#include "MassEntityElementTypes.h"
#include "MDSMassEnemyFragments.generated.h"

USTRUCT()
struct MDSPROJECT_API FMDSMassEnemyTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct MDSPROJECT_API FMDSMassSpawnFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SpawnIndex = INDEX_NONE;

	UPROPERTY()
	FVector SpawnLocation = FVector::ZeroVector;
};
