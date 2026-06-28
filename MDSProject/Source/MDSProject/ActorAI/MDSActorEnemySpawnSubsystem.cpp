#include "ActorAI/MDSActorEnemySpawnSubsystem.h"

#include "ActorAI/MDSActorEnemy.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Objective/MDSObjectiveActor.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSActorEnemySpawn, Log, All);

static TAutoConsoleVariable<int32> CVarMDSActorBaselineEnabled(
	TEXT("mds.ActorBaseline.Enabled"),
	0,
	TEXT("Enables the minimal server-only Actor enemy baseline when set before world begin play."));

static TAutoConsoleVariable<int32> CVarMDSActorBaselineCount(
	TEXT("mds.ActorBaseline.Count"),
	16,
	TEXT("Actor baseline enemy count. Use MDSActorBaselineCount=<N> on the command line for early startup override."));

bool UMDSActorEnemySpawnSubsystem::IsActorBaselineEnabled()
{
	return CVarMDSActorBaselineEnabled.GetValueOnGameThread() != 0 || FParse::Param(FCommandLine::Get(), TEXT("MDSActorBaseline"));
}

int32 UMDSActorEnemySpawnSubsystem::GetSpawnEnemyCount()
{
	int32 SpawnCount = CVarMDSActorBaselineCount.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSActorBaselineCount="), SpawnCount);
	return FMath::Max(1, SpawnCount);
}

FVector UMDSActorEnemySpawnSubsystem::CalculateSpawnLocation(const FVector& SpawnOrigin, const int32 SpawnIndex)
{
	const int32 Row = SpawnIndex / SpawnGridColumns;
	const int32 Column = SpawnIndex % SpawnGridColumns;
	constexpr float HalfGridOffset = (SpawnGridColumns - 1) * SpawnGridSpacing * 0.5f;

	return SpawnOrigin + FVector(
		(Column * SpawnGridSpacing) - HalfGridOffset,
		(Row * SpawnGridSpacing) - HalfGridOffset,
		120.0f);
}

FVector UMDSActorEnemySpawnSubsystem::CalculateMovementTargetLocation(const FVector& SpawnOrigin)
{
	return SpawnOrigin + FVector(MovementTargetOffset, 0.0f, 120.0f);
}

void UMDSActorEnemySpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!IsActorBaselineEnabled())
	{
		UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Actor enemy baseline disabled. Use -MDSActorBaseline or set mds.ActorBaseline.Enabled=1 before world begin play to enable it."));
		return;
	}

	if (InWorld.GetNetMode() == NM_Client)
	{
		UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Skipping Actor enemy baseline on client world."));
		return;
	}

	FTimerDelegate SpawnDelegate = FTimerDelegate::CreateUObject(this, &UMDSActorEnemySpawnSubsystem::SpawnActorBaseline);
	InWorld.GetTimerManager().SetTimerForNextTick(SpawnDelegate);
}

void UMDSActorEnemySpawnSubsystem::SpawnActorBaseline()
{
	if (bSpawned)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector SpawnOrigin = FVector::ZeroVector;
	for (TActorIterator<APlayerStart> PlayerStartIt(World); PlayerStartIt; ++PlayerStartIt)
	{
		SpawnOrigin = PlayerStartIt->GetActorLocation();
		break;
	}

	const FVector MovementTargetLocation = CalculateMovementTargetLocation(SpawnOrigin);
	AMDSObjectiveActor* ObjectiveActor = FindOrSpawnObjective(*World, MovementTargetLocation);
	if (!ObjectiveActor)
	{
		UE_LOG(LogMDSActorEnemySpawn, Warning, TEXT("Actor enemy baseline failed: objective actor is unavailable."));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;

	const int32 SpawnEnemyCount = GetSpawnEnemyCount();
	for (int32 EnemyIndex = 0; EnemyIndex < SpawnEnemyCount; ++EnemyIndex)
	{
		const FVector SpawnLocation = CalculateSpawnLocation(SpawnOrigin, EnemyIndex);
		AMDSActorEnemy* ActorEnemy = World->SpawnActor<AMDSActorEnemy>(AMDSActorEnemy::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		if (!ActorEnemy)
		{
			UE_LOG(LogMDSActorEnemySpawn, Warning, TEXT("Actor enemy baseline failed to spawn enemy index %d."), EnemyIndex);
			continue;
		}

		ActorEnemy->OnActorEnemyArrived().AddUObject(this, &UMDSActorEnemySpawnSubsystem::HandleEnemyArrived);
		ActorEnemy->InitializeActorEnemy(ObjectiveActor, MovementSpeed, ArrivalDistance, ObjectiveDamagePerArrival);
		++SpawnedEnemyCount;
	}

	bSpawned = true;
	if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
	{
		DebugState->SetActorEnemyCounts(SpawnedEnemyCount, ArrivedEnemyCount, DamageAppliedCount);
	}

	UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Actor enemy baseline spawned %d enemies near %s moving toward objective at %s. Requested count: %d. Damage per arrival: %.1f."), SpawnedEnemyCount, *SpawnOrigin.ToCompactString(), *ObjectiveActor->GetActorLocation().ToCompactString(), SpawnEnemyCount, ObjectiveDamagePerArrival);
}

AMDSObjectiveActor* UMDSActorEnemySpawnSubsystem::FindOrSpawnObjective(UWorld& InWorld, const FVector& ObjectiveLocation) const
{
	static const FName ActorObjectiveName(TEXT("MDS_ActorObjectiveProbe"));

	for (TActorIterator<AMDSObjectiveActor> ObjectiveIt(&InWorld); ObjectiveIt; ++ObjectiveIt)
	{
		if (ObjectiveIt->GetFName() == ActorObjectiveName)
		{
			return *ObjectiveIt;
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = ActorObjectiveName;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;

	return InWorld.SpawnActor<AMDSObjectiveActor>(ObjectiveLocation, FRotator::ZeroRotator, SpawnParameters);
}

void UMDSActorEnemySpawnSubsystem::HandleEnemyArrived(const bool bDamageApplied, const FVector& ArrivalLocation)
{
	++ArrivedEnemyCount;
	if (bDamageApplied)
	{
		++DamageAppliedCount;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetActorEnemyCounts(SpawnedEnemyCount, ArrivedEnemyCount, DamageAppliedCount);
		}
	}

	UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Actor enemy baseline counts: Spawned=%d Arrived=%d DamageApplied=%d LastArrival=%s."), SpawnedEnemyCount, ArrivedEnemyCount, DamageAppliedCount, *ArrivalLocation.ToCompactString());
}
