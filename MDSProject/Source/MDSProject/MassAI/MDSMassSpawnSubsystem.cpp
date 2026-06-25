#include "MassAI/MDSMassSpawnSubsystem.h"

#include "MassAI/MDSMassEnemyFragments.h"
#include "MassArchetypeTypes.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSMassSpawn, Log, All);

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
		FMDSMassSpawnFragment::StaticStruct()
	};

	const FMassArchetypeHandle EnemyArchetype = EntityManager.CreateArchetype(EntityComposition);

	TArray<FMassEntityHandle> SpawnedEntities;
	EntityManager.BatchCreateEntities(EnemyArchetype, SpawnOnlyEntityCount, SpawnedEntities);

	SpawnedEntityCount = SpawnedEntities.Num();
	bSpawned = true;

	UE_LOG(LogMDSMassSpawn, Log, TEXT("Mass spawn-only probe created %d entities."), SpawnedEntityCount);
}
