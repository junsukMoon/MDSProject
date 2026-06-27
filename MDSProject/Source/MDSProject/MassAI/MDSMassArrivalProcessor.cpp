#include "MassAI/MDSMassArrivalProcessor.h"

#include "DrawDebugHelpers.h"
#include "MassAI/MDSMassEnemyFragments.h"
#include "MassExecutionContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSMassArrival, Log, All);

UMDSMassArrivalProcessor::UMDSMassArrivalProcessor()
	: EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Server);
	bRequiresGameThreadExecution = true;
}

void UMDSMassArrivalProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMDSMassEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMDSMassMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMDSMassArrivalFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RequireMutatingWorldAccess();
}

void UMDSMassArrivalProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	int32 NewArrivalCount = 0;
	int32 TotalArrivalCount = 0;
	int32 CheckedEntityCount = 0;

	EntityQuery.ForEachEntityChunk(Context, [World, &NewArrivalCount, &TotalArrivalCount, &CheckedEntityCount](FMassExecutionContext& Context)
	{
		TConstArrayView<FMDSMassMovementFragment> MovementFragments = Context.GetFragmentView<FMDSMassMovementFragment>();
		TArrayView<FMDSMassArrivalFragment> ArrivalFragments = Context.GetMutableFragmentView<FMDSMassArrivalFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			++CheckedEntityCount;

			FMDSMassArrivalFragment& ArrivalFragment = ArrivalFragments[EntityIt];
			const FMDSMassMovementFragment& MovementFragment = MovementFragments[EntityIt];
			const float DistanceToTarget = FVector::Dist(MovementFragment.CurrentLocation, MovementFragment.TargetLocation);

			if (!ArrivalFragment.bHasArrived && DistanceToTarget <= ArrivalFragment.ArrivalDistance)
			{
				ArrivalFragment.bHasArrived = true;
				++NewArrivalCount;
			}

			if (ArrivalFragment.bHasArrived)
			{
				++TotalArrivalCount;
				DrawDebugSphere(World, MovementFragment.CurrentLocation, 55.0f, 12, FColor::Cyan, false, 5.0f, 0, 5.0f);
			}
		}
	});

	if (NewArrivalCount > 0)
	{
		UE_LOG(LogMDSMassArrival, Log, TEXT("Mass arrival-only processor detected %d new arrivals, %d total arrived."), NewArrivalCount, TotalArrivalCount);
	}
	else
	{
		static double LastLogTime = 0.0;
		const double CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastLogTime >= 1.0)
		{
			LastLogTime = CurrentTime;
			UE_LOG(LogMDSMassArrival, Log, TEXT("Mass arrival-only processor checked %d entities, %d total arrived."), CheckedEntityCount, TotalArrivalCount);
		}
	}
}
