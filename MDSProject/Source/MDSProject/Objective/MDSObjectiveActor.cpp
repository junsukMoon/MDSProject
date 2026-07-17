#include "Objective/MDSObjectiveActor.h"

#include "Components/WidgetComponent.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/MDSObjectiveWorldWidget.h"
#include "Net/UnrealNetwork.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSObjective, Log, All);

namespace
{
const TCHAR* ObjectiveWorldWidgetClassPath = TEXT("/Game/MDS/UI/WBP_MDSObjectiveWorldUI.WBP_MDSObjectiveWorldUI_C");
constexpr int32 WorldUITrackingSampleCount = 4;
constexpr float WorldUITrackingSampleIntervalSeconds = 1.0f;

bool ShouldLogWorldUITracking()
{
	return FParse::Param(FCommandLine::Get(), TEXT("MDSWorldUITrackingLog"));
}
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
	ObjectiveWorldWidgetComponent->SetDrawAtDesiredSize(false);
	ObjectiveWorldWidgetComponent->SetDrawSize(FVector2D(260.0f, 56.0f));
	ObjectiveWorldWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
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
				StartWorldUITrackingLog();
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

void AMDSObjectiveActor::StartWorldUITrackingLog()
{
	if (!ObjectiveWorldWidgetComponent || GetNetMode() == NM_DedicatedServer || !ShouldLogWorldUITracking())
	{
		return;
	}

	WorldUITrackingLogSamplesRemaining = WorldUITrackingSampleCount;
	LogWorldUITrackingSample();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WorldUITrackingLogTimerHandle,
			this,
			&AMDSObjectiveActor::LogWorldUITrackingSample,
			WorldUITrackingSampleIntervalSeconds,
			true);
	}
}

void AMDSObjectiveActor::LogWorldUITrackingSample()
{
	if (!ObjectiveWorldWidgetComponent || WorldUITrackingLogSamplesRemaining <= 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WorldUITrackingLogTimerHandle);
		}
		return;
	}

	--WorldUITrackingLogSamplesRemaining;

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	bool bProjected = false;
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		bProjected = UGameplayStatics::ProjectWorldToScreen(
			PlayerController,
			ObjectiveWorldWidgetComponent->GetComponentLocation(),
			ScreenPosition,
			false);
	}

	UE_LOG(LogMDSObjective, Log, TEXT("ObjectiveWorldUITrack Actor=%s ActorWorld=%s WidgetWorld=%s Screen=(%.1f,%.1f) Projected=%s WidgetClass=%s."),
		*GetNameSafe(this),
		*GetActorLocation().ToCompactString(),
		*ObjectiveWorldWidgetComponent->GetComponentLocation().ToCompactString(),
		ScreenPosition.X,
		ScreenPosition.Y,
		bProjected ? TEXT("true") : TEXT("false"),
		*GetNameSafe(ObjectiveWorldWidgetComponent->GetWidgetClass()));
}
