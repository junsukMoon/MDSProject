#include "MassAI/MDSMassMovementProcessor.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "DrawDebugHelpers.h"
#include "MassAI/MDSMassEnemyFragments.h"
#include "MassExecutionContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSMassMovement, Log, All);

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

	EntityQuery.ForEachEntityChunk(Context, [World, DeltaTimeSeconds, &MovedEntityCount](FMassExecutionContext& Context)
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

			DrawDebugSphere(World, MovementFragment.CurrentLocation, 45.0f, 12, FColor::Yellow, false, 0.0f, 0, 4.0f);
			DrawDebugLine(World, MovementFragment.CurrentLocation, MovementFragment.TargetLocation, FColor::Orange, false, 0.0f, 0, 2.0f);
		}
	});

	if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
	{
		DebugState->SetMassMovedCount(MovedEntityCount);
	}

	static double LastLogTime = 0.0;
	const double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastLogTime >= 1.0)
	{
		LastLogTime = CurrentTime;
		UE_LOG(LogMDSMassMovement, Log, TEXT("Mass movement-only processor updated %d entities."), MovedEntityCount);
	}
}
