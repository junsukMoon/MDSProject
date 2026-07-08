#include "ActorAI/MDSActorEnemySpawnSubsystem.h"

#include "Combat/MDSCombatEnemyActor.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Objective/MDSObjectiveActor.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSActorEnemySpawn, Log, All);

static TAutoConsoleVariable<int32> CVarMDSActorBaselineEnabled(
	TEXT("mds.ActorBaseline.Enabled"),
	0,
	TEXT("Enables the minimal server-side combat enemy spawn baseline when set before world begin play."));

static TAutoConsoleVariable<int32> CVarMDSActorBaselineCount(
	TEXT("mds.ActorBaseline.Count"),
	16,
	TEXT("Combat enemy baseline count. Use MDSActorBaselineCount=<N> on the command line for early startup override."));

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
	const FVector Direction = GetCardinalSpawnDirection(SpawnIndex);
	const FVector Tangent = FVector(-Direction.Y, Direction.X, 0.0f);
	const int32 LaneIndex = SpawnIndex / 4;
	const float LaneOffset = (LaneIndex % 2 == 0 ? 1.0f : -1.0f) * static_cast<float>((LaneIndex + 1) / 2) * SpawnLaneSpacing;

	return SpawnOrigin
		+ Direction * SpawnDistanceFromObjective
		+ Tangent * LaneOffset
		+ FVector(0.0f, 0.0f, SpawnHeightOffset);
}

FVector UMDSActorEnemySpawnSubsystem::GetCardinalSpawnDirection(const int32 SpawnIndex)
{
	switch (SpawnIndex % 4)
	{
	case 0:
		return FVector(1.0f, 0.0f, 0.0f);
	case 1:
		return FVector(-1.0f, 0.0f, 0.0f);
	case 2:
		return FVector(0.0f, 1.0f, 0.0f);
	default:
		return FVector(0.0f, -1.0f, 0.0f);
	}
}

void UMDSActorEnemySpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!IsActorBaselineEnabled())
	{
		UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Combat enemy spawn baseline disabled. Use -MDSActorBaseline or set mds.ActorBaseline.Enabled=1 before world begin play to enable it."));
		return;
	}

	if (InWorld.GetNetMode() == NM_Client)
	{
		UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Skipping combat enemy spawn baseline on client world."));
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

	const int32 SpawnEnemyCount = GetSpawnEnemyCount();
	const int32 WaveSpawnedEnemyCount = SpawnCombatEnemiesForWave(SpawnEnemyCount);
	if (WaveSpawnedEnemyCount <= 0)
	{
		return;
	}

	bSpawned = true;
}

int32 UMDSActorEnemySpawnSubsystem::SpawnCombatEnemiesForWave(const int32 EnemyCount)
{
	const int32 ClampedEnemyCount = FMath::Max(0, EnemyCount);
	if (ClampedEnemyCount <= 0)
	{
		UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Skipped combat enemy wave spawn because requested count is zero."));
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogMDSActorEnemySpawn, Warning, TEXT("Rejected combat enemy wave spawn on client world."));
		return 0;
	}

	AMDSObjectiveActor* ObjectiveActor = FindOrSpawnObjective(*World, FVector::ZeroVector);
	if (!ObjectiveActor)
	{
		UE_LOG(LogMDSActorEnemySpawn, Warning, TEXT("Combat enemy spawn baseline failed: objective actor is unavailable."));
		return 0;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;

	int32 WaveSpawnedEnemyCount = 0;
	for (int32 EnemyIndex = 0; EnemyIndex < ClampedEnemyCount; ++EnemyIndex)
	{
		const FVector SpawnLocation = CalculateSpawnLocation(ObjectiveActor->GetActorLocation(), EnemyIndex);
		AMDSCombatEnemyActor* CombatEnemy = World->SpawnActor<AMDSCombatEnemyActor>(AMDSCombatEnemyActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		if (!CombatEnemy)
		{
			UE_LOG(LogMDSActorEnemySpawn, Warning, TEXT("Combat enemy spawn baseline failed to spawn enemy index %d."), EnemyIndex);
			continue;
		}

		CombatEnemy->InitializeCombatEnemy(ObjectiveActor, MovementSpeed, ArrivalDistance, ObjectiveDamagePerArrival);
		++WaveSpawnedEnemyCount;
		++SpawnedEnemyCount;
	}

	if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
	{
		DebugState->SetActorEnemyCounts(SpawnedEnemyCount, 0, 0);
	}

	UE_LOG(LogMDSActorEnemySpawn, Log, TEXT("Combat enemy wave spawn created %d/%d enemies around objective %s at %s. Total spawned=%d."),
		WaveSpawnedEnemyCount,
		ClampedEnemyCount,
		*GetNameSafe(ObjectiveActor),
		*ObjectiveActor->GetActorLocation().ToCompactString(),
		SpawnedEnemyCount);

	return WaveSpawnedEnemyCount;
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

