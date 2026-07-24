#include "Objective/MDSObjectiveActor.h"

#include "MDSAssetPaths.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "UI/MDSObjectiveWorldWidget.h"
#include "Net/UnrealNetwork.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSObjective, Log, All);

namespace
{
constexpr int32 ObjectiveWorldUITrackingSampleCount = 4;
constexpr float ObjectiveWorldUITrackingSampleIntervalSeconds = 1.0f;

bool ShouldLogObjectiveWorldUITracking()
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

	ObjectiveVisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ObjectiveVisualRoot"));
	ObjectiveVisualRoot->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BuildingMeshFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildingMaterialFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray_02.MI_PrototypeGrid_Gray_02"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RoofMaterialFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark.MI_PrototypeGrid_TopDark"));

	const auto ConfigureBuildingPart = [this](
		UStaticMeshComponent* MeshComponent,
		const FVector& RelativeLocation,
		const FVector& RelativeScale,
		UMaterialInterface* Material)
	{
		MeshComponent->SetupAttachment(ObjectiveVisualRoot);
		MeshComponent->SetRelativeLocation(RelativeLocation);
		MeshComponent->SetRelativeScale3D(RelativeScale);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCanEverAffectNavigation(false);

		if (BuildingMeshFinder.Succeeded())
		{
			MeshComponent->SetStaticMesh(BuildingMeshFinder.Object);
		}
		if (Material)
		{
			MeshComponent->SetMaterial(0, Material);
		}
	};

	UMaterialInterface* BuildingMaterial = BuildingMaterialFinder.Succeeded() ? BuildingMaterialFinder.Object.Get() : nullptr;
	UMaterialInterface* RoofMaterial = RoofMaterialFinder.Succeeded() ? RoofMaterialFinder.Object.Get() : BuildingMaterial;

	FoundationMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoundationMesh"));
	ConfigureBuildingPart(FoundationMeshComponent, FVector(0.0f, 0.0f, 25.0f), FVector(4.2f, 3.4f, 0.5f), RoofMaterial);

	MainBuildingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBuildingMesh"));
	ConfigureBuildingPart(MainBuildingMeshComponent, FVector(0.0f, 0.0f, 125.0f), FVector(3.2f, 2.4f, 1.5f), BuildingMaterial);

	MainRoofMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainRoofMesh"));
	ConfigureBuildingPart(MainRoofMeshComponent, FVector(0.0f, 0.0f, 220.0f), FVector(3.6f, 2.8f, 0.35f), RoofMaterial);

	TowerMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	ConfigureBuildingPart(TowerMeshComponent, FVector(0.0f, 0.0f, 285.0f), FVector(1.4f, 1.4f, 1.0f), BuildingMaterial);

	TowerRoofMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerRoofMesh"));
	ConfigureBuildingPart(TowerRoofMeshComponent, FVector(0.0f, 0.0f, 350.0f), FVector(1.8f, 1.8f, 0.25f), RoofMaterial);

	EntranceMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntranceMesh"));
	ConfigureBuildingPart(EntranceMeshComponent, FVector(166.0f, 0.0f, 105.0f), FVector(0.18f, 0.7f, 0.9f), RoofMaterial);

	ObjectiveWorldWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ObjectiveWorldWidget"));
	ObjectiveWorldWidgetComponent->SetupAttachment(SceneRoot);
	ObjectiveWorldWidgetComponent->SetWidgetClass(UMDSObjectiveWorldWidget::StaticClass());
	ObjectiveWorldWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ObjectiveWorldWidgetComponent->SetDrawAtDesiredSize(false);
	ObjectiveWorldWidgetComponent->SetDrawSize(FVector2D(260.0f, 56.0f));
	ObjectiveWorldWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	ObjectiveWorldWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 410.0f));
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
			if (ObjectiveVisualRoot)
			{
				ObjectiveVisualRoot->SetVisibility(false, true);
			}
			ObjectiveWorldWidgetComponent->SetVisibility(false);
		}
		else
		{
			const FSoftClassPath WidgetClassPath(MDSAssetPaths::ObjectiveWorldWidgetClass);
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
	if (!ObjectiveWorldWidgetComponent || GetNetMode() == NM_DedicatedServer || !ShouldLogObjectiveWorldUITracking())
	{
		return;
	}

	WorldUITrackingLogSamplesRemaining = ObjectiveWorldUITrackingSampleCount;
	LogWorldUITrackingSample();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WorldUITrackingLogTimerHandle,
			this,
			&AMDSObjectiveActor::LogWorldUITrackingSample,
			ObjectiveWorldUITrackingSampleIntervalSeconds,
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
