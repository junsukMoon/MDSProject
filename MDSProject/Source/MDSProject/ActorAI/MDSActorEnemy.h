#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDSActorEnemy.generated.h"

class AMDSObjectiveActor;

DECLARE_MULTICAST_DELEGATE_TwoParams(FMDSActorEnemyArrivedSignature, bool /*bDamageApplied*/, const FVector& /*ArrivalLocation*/);

UCLASS()
class MDSPROJECT_API AMDSActorEnemy : public AActor
{
	GENERATED_BODY()

public:
	AMDSActorEnemy();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeActorEnemy(AMDSObjectiveActor* InObjectiveActor, float InMoveSpeed, float InArrivalDistance, float InObjectiveDamageAmount);
	FMDSActorEnemyArrivedSignature& OnActorEnemyArrived() { return ActorEnemyArrived; }

protected:
	virtual void BeginPlay() override;

private:
	void HandleArrival();

	UPROPERTY(VisibleDefaultsOnly, Category = "Actor Enemy")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<AMDSObjectiveActor> ObjectiveActor;

	float MoveSpeed = 320.0f;
	float ArrivalDistance = 75.0f;
	float ObjectiveDamageAmount = 5.0f;
	bool bHasArrived = false;

	FMDSActorEnemyArrivedSignature ActorEnemyArrived;
};
