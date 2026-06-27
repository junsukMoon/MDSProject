#include "MassAI/MDSMassSpawnSubsystem.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "MassAI/MDSMassEnemyFragments.h"
#include "MassArchetypeTypes.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSMassSpawn, Log, All);

FVector UMDSMassSpawnSubsystem::CalculateSpawnLocation(const FVector& SpawnOrigin, const int32 SpawnIndex)
{
	const int32 Row = SpawnIndex / SpawnGridColumns;
	const int32 Column = SpawnIndex % SpawnGridColumns;
	constexpr float HalfGridOffset = (SpawnGridColumns - 1) * SpawnGridSpacing * 0.5f;

	return SpawnOrigin + FVector(
		(Column * SpawnGridSpacing) - HalfGridOffset,
		(Row * SpawnGridSpacing) - HalfGridOffset,
		120.0f);
}

FVector UMDSMassSpawnSubsystem::CalculateMovementTargetLocation(const FVector& SpawnOrigin)
{
	return SpawnOrigin + FVector(MovementTargetOffset, 0.0f, 120.0f);
}

void UMDSMassSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (bSpawned)
	{
		return;
	}

	if (InWorld.GetNetMode() == NM_Client)
	{
		UE_LOG(LogMDSMassSpawn, Log, TEXT("Skipping Mass spawn-only probe on client world."));
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		UE_LOG(LogMDSMassSpawn, Warning, TEXT("Mass spawn-only probe failed: UMassEntitySubsystem is unavailable."));
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const TArray<const UScriptStruct*> EntityComposition = {
		FMDSMassEnemyTag::StaticStruct(),
		FMDSMassSpawnFragment::StaticStruct(),
		FMDSMassMovementFragment::StaticStruct(),
		FMDSMassArrivalFragment::StaticStruct()
	};

	const FMassArchetypeHandle EnemyArchetype = EntityManager.CreateArchetype(EntityComposition);

	TArray<FMassEntityHandle> SpawnedEntities;
	EntityManager.BatchCreateEntities(EnemyArchetype, SpawnOnlyEntityCount, SpawnedEntities);

	SpawnedEntityCount = SpawnedEntities.Num();
	bSpawned = true;

	FVector SpawnOrigin = FVector::ZeroVector;
	for (TActorIterator<APlayerStart> PlayerStartIt(&InWorld); PlayerStartIt; ++PlayerStartIt)
	{
		SpawnOrigin = PlayerStartIt->GetActorLocation();
		break;
	}

	const FVector MovementTargetLocation = CalculateMovementTargetLocation(SpawnOrigin);
	DrawDebugSphere(&InWorld, MovementTargetLocation, SpawnDebugRadius, 16, FColor::Red, false, SpawnDebugLifetime, 0, SpawnDebugThickness);

	for (int32 EntityIndex = 0; EntityIndex < SpawnedEntities.Num(); ++EntityIndex)
	{
		const FVector SpawnLocation = CalculateSpawnLocation(SpawnOrigin, EntityIndex);

		if (FMDSMassSpawnFragment* SpawnFragment = EntityManager.GetFragmentDataPtr<FMDSMassSpawnFragment>(SpawnedEntities[EntityIndex]))
		{
			SpawnFragment->SpawnIndex = EntityIndex;
			SpawnFragment->SpawnLocation = SpawnLocation;
		}

		if (FMDSMassMovementFragment* MovementFragment = EntityManager.GetFragmentDataPtr<FMDSMassMovementFragment>(SpawnedEntities[EntityIndex]))
		{
			MovementFragment->CurrentLocation = SpawnLocation;
			MovementFragment->TargetLocation = MovementTargetLocation;
			MovementFragment->MoveSpeed = MovementSpeed;
		}

		if (FMDSMassArrivalFragment* ArrivalFragment = EntityManager.GetFragmentDataPtr<FMDSMassArrivalFragment>(SpawnedEntities[EntityIndex]))
		{
			ArrivalFragment->ArrivalDistance = ArrivalDistance;
			ArrivalFragment->bHasArrived = false;
		}

		DrawDebugSphere(&InWorld, SpawnLocation, SpawnDebugRadius, 16, FColor::Green, false, SpawnDebugLifetime, 0, SpawnDebugThickness);
	}

	UE_LOG(LogMDSMassSpawn, Log, TEXT("Mass arrival-only probe initialized %d entities near %s moving toward %s."), SpawnedEntityCount, *SpawnOrigin.ToCompactString(), *MovementTargetLocation.ToCompactString());
}
