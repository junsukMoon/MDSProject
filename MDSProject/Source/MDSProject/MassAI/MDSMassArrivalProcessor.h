#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MDSMassArrivalProcessor.generated.h"

class AMDSObjectiveActor;

UCLASS()
class MDSPROJECT_API UMDSMassArrivalProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMDSMassArrivalProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	TWeakObjectPtr<AMDSObjectiveActor> CachedObjectiveActor;

	FMassEntityQuery EntityQuery;
};
