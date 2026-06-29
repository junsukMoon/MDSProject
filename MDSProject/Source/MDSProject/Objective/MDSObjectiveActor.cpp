#include "Objective/MDSObjectiveActor.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSObjective, Log, All);

AMDSObjectiveActor::AMDSObjectiveActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AMDSObjectiveActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		if (UWorld* World = GetWorld())
		{
			if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
			{
				DebugState->SetObjectiveHealth(CurrentHealth, MaxHealth);
			}
		}

		UE_LOG(LogMDSObjective, Log, TEXT("Objective initialized on server with %.1f / %.1f HP at %s."), CurrentHealth, MaxHealth, *GetActorLocation().ToCompactString());
	}
}

void AMDSObjectiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSObjectiveActor, CurrentHealth);
}

bool AMDSObjectiveActor::ApplyObjectiveDamage(const float DamageAmount, const FName DamageSource)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSObjective, Warning, TEXT("Rejected non-authority objective damage request from %s."), *DamageSource.ToString());
		return false;
	}

	if (DamageAmount <= 0.0f || CurrentHealth <= 0.0f)
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetObjectiveHealth(CurrentHealth, MaxHealth);
		}
	}

	UE_LOG(LogMDSObjective, Log, TEXT("Objective damage applied by %s: %.1f damage, HP %.1f -> %.1f."), *DamageSource.ToString(), DamageAmount, PreviousHealth, CurrentHealth);
	return CurrentHealth < PreviousHealth;
}

void AMDSObjectiveActor::OnRep_CurrentHealth()
{
	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetObjectiveHealth(CurrentHealth, MaxHealth);
		}
	}

	UE_LOG(LogMDSObjective, Log, TEXT("Objective HP replicated on client: %.1f / %.1f."), CurrentHealth, MaxHealth);
}
