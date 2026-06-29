#include "MassAI/MDSMassMovementProcessor.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "MassAI/MDSMassEnemyFragments.h"
#include "MassExecutionContext.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSMassMovement, Log, All);

static TAutoConsoleVariable<int32> CVarMDSMassDebugDrawEnabled(
	TEXT("mds.MassDebugDraw.Enabled"),
	1,
	TEXT("Enables Mass debug draw. Use -NoMDSMassDebugDraw for profiling runs."));

static bool IsMassMovementDebugDrawEnabled()
{
	return CVarMDSMassDebugDrawEnabled.GetValueOnGameThread() != 0 && !FParse::Param(FCommandLine::Get(), TEXT("NoMDSMassDebugDraw"));
}

UMDSMassMovementProcessor::UMDSMassMovementProcessor()
	: EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Server);
	bRequiresGameThreadExecution = true;
}

void UMDSMassMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMDSMassEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMDSMassSpawnFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMDSMassMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMDSMassArrivalFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RequireMutatingWorldAccess();
}

void UMDSMassMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	int32 MovedEntityCount = 0;
	const float DeltaTimeSeconds = Context.GetDeltaTimeSeconds();
	const bool bDrawDebug = IsMassMovementDebugDrawEnabled();

	EntityQuery.ForEachEntityChunk(Context, [World, DeltaTimeSeconds, bDrawDebug, &MovedEntityCount](FMassExecutionContext& Context)
	{
		TArrayView<FMDSMassMovementFragment> MovementFragments = Context.GetMutableFragmentView<FMDSMassMovementFragment>();
		TConstArrayView<FMDSMassArrivalFragment> ArrivalFragments = Context.GetFragmentView<FMDSMassArrivalFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (ArrivalFragments[EntityIt].bHasArrived)
			{
				continue;
			}

			FMDSMassMovementFragment& MovementFragment = MovementFragments[EntityIt];
			const FVector ToTarget = MovementFragment.TargetLocation - MovementFragment.CurrentLocation;
			const float MaxStep = MovementFragment.MoveSpeed * DeltaTimeSeconds;

			if (!ToTarget.IsNearlyZero() && MaxStep > 0.0f)
			{
				MovementFragment.CurrentLocation += ToTarget.GetClampedToMaxSize(MaxStep);
				++MovedEntityCount;
			}

			if (bDrawDebug)
			{
				DrawDebugSphere(World, MovementFragment.CurrentLocation, 45.0f, 12, FColor::Yellow, false, 0.0f, 0, 4.0f);
				DrawDebugLine(World, MovementFragment.CurrentLocation, MovementFragment.TargetLocation, FColor::Orange, false, 0.0f, 0, 2.0f);
			}
		}
	});

	if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
	{
		DebugState->SetMassMovedCount(MovedEntityCount);
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastMovementLogTime >= 1.0)
	{
		LastMovementLogTime = CurrentTime;
		UE_LOG(LogMDSMassMovement, Log, TEXT("Mass movement-only processor updated %d entities."), MovedEntityCount);
	}
}
