#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MDSMassMovementProcessor.generated.h"

UCLASS()
class MDSPROJECT_API UMDSMassMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMDSMassMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	double LastMovementLogTime = 0.0;

	FMassEntityQuery EntityQuery;
};
