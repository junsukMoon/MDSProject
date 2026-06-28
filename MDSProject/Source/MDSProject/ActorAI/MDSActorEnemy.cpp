#include "ActorAI/MDSActorEnemy.h"

#include "Components/SceneComponent.h"
#include "Objective/MDSObjectiveActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSActorEnemy, Log, All);

AMDSActorEnemy::AMDSActorEnemy()
{
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AMDSActorEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		SetActorTickEnabled(false);
	}
}

void AMDSActorEnemy::InitializeActorEnemy(AMDSObjectiveActor* InObjectiveActor, const float InMoveSpeed, const float InArrivalDistance, const float InObjectiveDamageAmount)
{
	ObjectiveActor = InObjectiveActor;
	MoveSpeed = InMoveSpeed;
	ArrivalDistance = InArrivalDistance;
	ObjectiveDamageAmount = InObjectiveDamageAmount;
	bHasArrived = false;

	SetActorTickEnabled(HasAuthority() && ObjectiveActor != nullptr);
}

void AMDSActorEnemy::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || bHasArrived || !ObjectiveActor)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = ObjectiveActor->GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= ArrivalDistance)
	{
		HandleArrival();
		return;
	}

	const float MoveDistance = FMath::Min(MoveSpeed * DeltaSeconds, DistanceToTarget);
	if (MoveDistance <= 0.0f)
	{
		return;
	}

	ActorEnemyActiveTick.Broadcast(1);

	const FVector NewLocation = CurrentLocation + (ToTarget / DistanceToTarget) * MoveDistance;
	SetActorLocation(NewLocation, false);

	if (FVector::Dist(NewLocation, TargetLocation) <= ArrivalDistance)
	{
		HandleArrival();
	}
}

void AMDSActorEnemy::HandleArrival()
{
	if (bHasArrived)
	{
		return;
	}

	bHasArrived = true;
	SetActorTickEnabled(false);

	bool bDamageApplied = false;
	if (ObjectiveActor)
	{
		bDamageApplied = ObjectiveActor->ApplyObjectiveDamage(ObjectiveDamageAmount, TEXT("ActorArrival"));
	}

	UE_LOG(LogMDSActorEnemy, Log, TEXT("Actor enemy arrived at %s. Objective damage applied: %s."), *GetActorLocation().ToCompactString(), bDamageApplied ? TEXT("true") : TEXT("false"));
	ActorEnemyArrived.Broadcast(bDamageApplied, GetActorLocation());
}
