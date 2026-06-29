#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDSActorEnemy.generated.h"

class AMDSObjectiveActor;
class USceneComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FMDSActorEnemyArrivedSignature, bool /*bDamageApplied*/, const FVector& /*ArrivalLocation*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FMDSActorEnemyActiveTickSignature, int32 /*ActiveDelta*/);

UCLASS()
class MDSPROJECT_API AMDSActorEnemy : public AActor
{
	GENERATED_BODY()

public:
	AMDSActorEnemy();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeActorEnemy(AMDSObjectiveActor* InObjectiveActor, float InMoveSpeed, float InArrivalDistance, float InObjectiveDamageAmount);
	FMDSActorEnemyArrivedSignature& OnActorEnemyArrived() { return ActorEnemyArrived; }
	FMDSActorEnemyActiveTickSignature& OnActorEnemyActiveTick() { return ActorEnemyActiveTick; }

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
	FMDSActorEnemyActiveTickSignature ActorEnemyActiveTick;
};
