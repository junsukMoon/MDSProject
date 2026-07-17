#include "Objective/MDSObjectiveActor.h"

#include "Components/WidgetComponent.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "UI/MDSObjectiveWorldWidget.h"
#include "Net/UnrealNetwork.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSObjective, Log, All);

namespace
{
const TCHAR* ObjectiveWorldWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSObjectiveWorldUI.WBP_MDSObjectiveWorldUI_C");
}

AMDSObjectiveActor::AMDSObjectiveActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObjectiveWorldWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ObjectiveWorldWidget"));
	ObjectiveWorldWidgetComponent->SetupAttachment(SceneRoot);
	ObjectiveWorldWidgetComponent->SetWidgetClass(UMDSObjectiveWorldWidget::StaticClass());
	ObjectiveWorldWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ObjectiveWorldWidgetComponent->SetDrawAtDesiredSize(true);
	ObjectiveWorldWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	ObjectiveWorldWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	if (ObjectiveWorldWidgetComponent)
	{
		if (GetNetMode() == NM_DedicatedServer)
		{
			ObjectiveWorldWidgetComponent->SetVisibility(false);
		}
		else
		{
			const FSoftClassPath WidgetClassPath(ObjectiveWorldWidgetClassPath);
			if (UClass* LoadedWidgetClass = WidgetClassPath.TryLoadClass<UMDSObjectiveWorldWidget>())
			{
				ObjectiveWorldWidgetComponent->SetWidgetClass(LoadedWidgetClass);
			}

			ObjectiveWorldWidgetComponent->InitWidget();
			if (UMDSObjectiveWorldWidget* InitializedObjectiveWidget = Cast<UMDSObjectiveWorldWidget>(ObjectiveWorldWidgetComponent->GetUserWidgetObject()))
			{
				InitializedObjectiveWidget->SetObjectiveActor(this);
				UE_LOG(LogMDSObjective, Log, TEXT("Objective World UI widget initialized on %s using %s."),
					*GetNameSafe(this),
					*GetNameSafe(ObjectiveWorldWidgetComponent->GetWidgetClass()));
			}
		}
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

	if (ObjectiveWorldWidgetComponent)
	{
		if (UMDSObjectiveWorldWidget* ObjectiveWidget = Cast<UMDSObjectiveWorldWidget>(ObjectiveWorldWidgetComponent->GetUserWidgetObject()))
		{
			ObjectiveWidget->RefreshFromObjective();
		}
	}
}
