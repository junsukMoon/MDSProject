#include "Debug/MDSDebugStateSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSDebugState, Log, All);

void UMDSDebugStateSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	InWorld.GetTimerManager().SetTimer(PublishTimerHandle, this, &UMDSDebugStateSubsystem::PublishDebugState, 1.0f, true, 1.0f);
}

void UMDSDebugStateSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PublishTimerHandle);
	}

	Super::Deinitialize();
}

void UMDSDebugStateSubsystem::SetMassSpawnedCount(const int32 InSpawnedCount)
{
	SpawnedCount = InSpawnedCount;
}

void UMDSDebugStateSubsystem::SetMassMovedCount(const int32 InMovedCount)
{
	LastMovedCount = InMovedCount;
}

void UMDSDebugStateSubsystem::SetMassArrivalCounts(const int32 InArrivedCount, const int32 InDamageAppliedCount)
{
	ArrivedCount = InArrivedCount;
	DamageAppliedCount += InDamageAppliedCount;
}

void UMDSDebugStateSubsystem::SetActorActiveCount(const int32 InActiveCount)
{
	ActorActiveCount = InActiveCount;
}

void UMDSDebugStateSubsystem::SetActorEnemyCounts(const int32 InSpawnedCount, const int32 InArrivedCount, const int32 InDamageAppliedCount)
{
	ActorSpawnedCount = InSpawnedCount;
	ActorArrivedCount = InArrivedCount;
	ActorDamageAppliedCount = InDamageAppliedCount;
}

void UMDSDebugStateSubsystem::SetObjectiveHealth(const float InCurrentHealth, const float InMaxHealth)
{
	ObjectiveCurrentHealth = InCurrentHealth;
	ObjectiveMaxHealth = InMaxHealth;
}

FMDSDebugStateSnapshot UMDSDebugStateSubsystem::GetSnapshot() const
{
	FMDSDebugStateSnapshot Snapshot;
	Snapshot.MassSpawnedCount = SpawnedCount;
	Snapshot.MassMovedCount = LastMovedCount;
	Snapshot.MassArrivedCount = ArrivedCount;
	Snapshot.MassDamageAppliedCount = DamageAppliedCount;
	Snapshot.ActorSpawnedCount = ActorSpawnedCount;
	Snapshot.ActorActiveCount = ActorActiveCount;
	Snapshot.ActorArrivedCount = ActorArrivedCount;
	Snapshot.ActorDamageAppliedCount = ActorDamageAppliedCount;
	Snapshot.ObjectiveCurrentHealth = ObjectiveCurrentHealth;
	Snapshot.ObjectiveMaxHealth = ObjectiveMaxHealth;
	return Snapshot;
}

FString UMDSDebugStateSubsystem::BuildDebugLine() const
{
	return FString::Printf(
		TEXT("MDS Debug | NetMode=%s | ObjectiveHP=%.0f/%.0f | Mass Spawned=%d Moved=%d Arrived=%d Damage=%d | Actor Spawned=%d ActiveTicks=%d Arrived=%d Damage=%d"),
		*GetNetModeLabel(),
		ObjectiveCurrentHealth,
		ObjectiveMaxHealth,
		SpawnedCount,
		LastMovedCount,
		ArrivedCount,
		DamageAppliedCount,
		ActorSpawnedCount,
		ActorActiveCount,
		ActorArrivedCount,
		ActorDamageAppliedCount);
}

FString UMDSDebugStateSubsystem::GetNetModeLabel() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return TEXT("NoWorld");
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return TEXT("Standalone");
	case NM_DedicatedServer:
		return TEXT("DedicatedServer");
	case NM_ListenServer:
		return TEXT("ListenServer");
	case NM_Client:
		return TEXT("Client");
	default:
		return TEXT("Unknown");
	}
}

void UMDSDebugStateSubsystem::PublishDebugState()
{
	const FString DebugLine = BuildDebugLine();
	UE_LOG(LogMDSDebugState, Log, TEXT("%s"), *DebugLine);

	if (GEngine && GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		GEngine->AddOnScreenDebugMessage(
			1237001,
			1.1f,
			FColor::Green,
			DebugLine);
	}
}
