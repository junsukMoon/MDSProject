#include "Combat/MDSCombatEnemyActor.h"

#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "MDSProjectGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Objective/MDSObjectiveActor.h"
#include "UI/MDSEnemyWorldWidget.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSCombatEnemy, Log, All);

namespace
{
const TCHAR* EnemyWorldWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSEnemyWorldUI.WBP_MDSEnemyWorldUI_C");
}

AMDSCombatEnemyActor::AMDSCombatEnemyActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EnemyWorldWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyWorldWidget"));
	EnemyWorldWidgetComponent->SetupAttachment(SceneRoot);
	EnemyWorldWidgetComponent->SetWidgetClass(UMDSEnemyWorldWidget::StaticClass());
	EnemyWorldWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyWorldWidgetComponent->SetDrawAtDesiredSize(true);
	EnemyWorldWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	EnemyWorldWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMDSCombatEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		bDeathHandled = false;
		bHasArrivedAtObjective = false;

		UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy initialized on server with %.1f / %.1f HP at %s."),
			CurrentHealth,
			MaxHealth,
			*GetActorLocation().ToCompactString());
	}

	if (EnemyWorldWidgetComponent)
	{
		if (GetNetMode() == NM_DedicatedServer)
		{
			EnemyWorldWidgetComponent->SetVisibility(false);
		}
		else
		{
			const FSoftClassPath WidgetClassPath(EnemyWorldWidgetClassPath);
			if (UClass* LoadedWidgetClass = WidgetClassPath.TryLoadClass<UMDSEnemyWorldWidget>())
			{
				EnemyWorldWidgetComponent->SetWidgetClass(LoadedWidgetClass);
			}

			EnemyWorldWidgetComponent->InitWidget();
			if (UMDSEnemyWorldWidget* InitializedEnemyWidget = Cast<UMDSEnemyWorldWidget>(EnemyWorldWidgetComponent->GetUserWidgetObject()))
			{
				InitializedEnemyWidget->SetEnemyActor(this);
				UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy World UI widget initialized on %s using %s."),
					*GetNameSafe(this),
					*GetNameSafe(EnemyWorldWidgetComponent->GetWidgetClass()));
			}
		}
	}
}

void AMDSCombatEnemyActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || IsDead() || bHasArrivedAtObjective || !ObjectiveActor)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = ObjectiveActor->GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= ArrivalDistance)
	{
		HandleObjectiveArrivalOnce();
		return;
	}

	const float MoveDistance = FMath::Min(MoveSpeed * DeltaSeconds, DistanceToTarget);
	if (MoveDistance <= 0.0f)
	{
		return;
	}

	const FVector NewLocation = CurrentLocation + (ToTarget / DistanceToTarget) * MoveDistance;
	SetActorLocation(NewLocation, false);

	if (FVector::Dist(NewLocation, TargetLocation) <= ArrivalDistance)
	{
		HandleObjectiveArrivalOnce();
	}
}

void AMDSCombatEnemyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSCombatEnemyActor, CurrentHealth);
}

void AMDSCombatEnemyActor::InitializeCombatEnemy(AMDSObjectiveActor* InObjectiveActor, const float InMoveSpeed, const float InArrivalDistance, const float InObjectiveDamageAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSCombatEnemy, Warning, TEXT("Rejected combat enemy initialization on non-authority."));
		return;
	}

	ObjectiveActor = InObjectiveActor;
	MoveSpeed = FMath::Max(0.0f, InMoveSpeed);
	ArrivalDistance = FMath::Max(1.0f, InArrivalDistance);
	ObjectiveDamageAmount = FMath::Max(0.0f, InObjectiveDamageAmount);
	bHasArrivedAtObjective = false;

	SetActorTickEnabled(ObjectiveActor != nullptr && !IsDead());

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy movement initialized on server. Target=%s MoveSpeed=%.1f ArrivalDistance=%.1f ObjectiveDamage=%.1f."),
		*GetNameSafe(ObjectiveActor),
		MoveSpeed,
		ArrivalDistance,
		ObjectiveDamageAmount);
}

bool AMDSCombatEnemyActor::ApplyEnemyDamage(const float DamageAmount, const FName DamageSource)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSCombatEnemy, Warning, TEXT("Rejected non-authority enemy damage request from %s."), *DamageSource.ToString());
		return false;
	}

	if (DamageAmount <= 0.0f || IsDead())
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy damage applied by %s: %.1f damage, HP %.1f -> %.1f."),
		*DamageSource.ToString(),
		DamageAmount,
		PreviousHealth,
		CurrentHealth);

	if (IsDead())
	{
		HandleDeathOnce(DamageSource);
	}

	return CurrentHealth < PreviousHealth;
}

void AMDSCombatEnemyActor::OnRep_CurrentHealth()
{
	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy HP replicated on client: %.1f / %.1f. Dead=%s."),
		CurrentHealth,
		MaxHealth,
		IsDead() ? TEXT("true") : TEXT("false"));

	if (EnemyWorldWidgetComponent)
	{
		if (UMDSEnemyWorldWidget* EnemyWidget = Cast<UMDSEnemyWorldWidget>(EnemyWorldWidgetComponent->GetUserWidgetObject()))
		{
			EnemyWidget->RefreshFromEnemy();
		}
	}
}

void AMDSCombatEnemyActor::HandleDeathOnce(const FName DamageSource)
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;
	SetActorTickEnabled(false);

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Enemy death handled on server from %s at %s."),
		*DamageSource.ToString(),
		*GetActorLocation().ToCompactString());

	if (UWorld* World = GetWorld())
	{
		if (AMDSProjectGameMode* MDSGameMode = World->GetAuthGameMode<AMDSProjectGameMode>())
		{
			MDSGameMode->HandleEnemyDeathForWave();
		}
	}
}

void AMDSCombatEnemyActor::HandleObjectiveArrivalOnce()
{
	if (bHasArrivedAtObjective)
	{
		return;
	}

	bHasArrivedAtObjective = true;
	SetActorTickEnabled(false);

	bool bDamageApplied = false;
	if (ObjectiveActor)
	{
		bDamageApplied = ObjectiveActor->ApplyObjectiveDamage(ObjectiveDamageAmount, TEXT("CombatEnemyArrival"));
	}

	UE_LOG(LogMDSCombatEnemy, Log, TEXT("Combat enemy arrived at objective. DamageApplied=%s Location=%s."),
		bDamageApplied ? TEXT("true") : TEXT("false"),
		*GetActorLocation().ToCompactString());
}
