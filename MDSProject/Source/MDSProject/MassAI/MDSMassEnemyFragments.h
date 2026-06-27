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

USTRUCT()
struct MDSPROJECT_API FMDSMassMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector CurrentLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float MoveSpeed = 250.0f;
};
