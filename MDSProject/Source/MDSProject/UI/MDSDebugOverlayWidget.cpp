#include "UI/MDSDebugOverlayWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "Engine/World.h"
#include "MDSProject.h"
#include "Styling/SlateColor.h"

#define LOCTEXT_NAMESPACE "MDSDebugOverlayWidget"

void UMDSDebugOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFallbackLayout();
	RefreshFromWorld();
}

void UMDSDebugOverlayWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	RefreshFromWorld();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UMDSDebugOverlayWidget::RefreshFromWorld, 1.0f, true, 1.0f);
	}
}

void UMDSDebugOverlayWidget::NativeOnDeactivated()
{
	ClearRefreshTimer();

	Super::NativeOnDeactivated();
}

void UMDSDebugOverlayWidget::NativeDestruct()
{
	ClearRefreshTimer();

	Super::NativeDestruct();
}

void UMDSDebugOverlayWidget::RefreshFromWorld()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		NetModeText = LOCTEXT("NoWorldNetMode", "NetMode: NoWorld");
		ObjectiveHealthText = LOCTEXT("NoWorldObjectiveHealth", "Objective HP: -");
		WaveSummaryText = LOCTEXT("NoWorldWaveSummary", "Wave: -");
		MassSummaryText = LOCTEXT("NoWorldMassSummary", "Mass: -");
		ActorSummaryText = LOCTEXT("NoWorldActorSummary", "Actor: -");
		UpdateBoundTextBlocks();
		return;
	}

	const UMDSDebugStateSubsystem* DebugStateSubsystem = World->GetSubsystem<UMDSDebugStateSubsystem>();
	if (!DebugStateSubsystem)
	{
		NetModeText = FText::FromString(FString::Printf(TEXT("NetMode: %s"), *GetNetModeLabel()));
		ObjectiveHealthText = LOCTEXT("NoDebugStateObjectiveHealth", "Objective HP: DebugState unavailable");
		WaveSummaryText = LOCTEXT("NoDebugStateWaveSummary", "Wave: DebugState unavailable");
		MassSummaryText = LOCTEXT("NoDebugStateMassSummary", "Mass: DebugState unavailable");
		ActorSummaryText = LOCTEXT("NoDebugStateActorSummary", "Actor: DebugState unavailable");
		UpdateBoundTextBlocks();
		return;
	}

	const FMDSDebugStateSnapshot Snapshot = DebugStateSubsystem->GetSnapshot();
	NetModeText = FText::FromString(FString::Printf(TEXT("NetMode: %s"), *GetNetModeLabel()));
	ObjectiveHealthText = FText::FromString(FString::Printf(
		TEXT("Objective HP: %.0f / %.0f"),
		Snapshot.ObjectiveCurrentHealth,
		Snapshot.ObjectiveMaxHealth));
	WaveSummaryText = FText::FromString(FString::Printf(
		TEXT("Wave: %d | Active %s | Remaining %d / %d"),
		Snapshot.CurrentWaveIndex,
		Snapshot.bWaveActive ? TEXT("true") : TEXT("false"),
		Snapshot.EnemiesRemaining,
		Snapshot.TotalEnemiesThisWave));
	MassSummaryText = FText::FromString(FString::Printf(
		TEXT("Mass: Spawned %d | Moved %d | Arrived %d | Damage %d"),
		Snapshot.MassSpawnedCount,
		Snapshot.MassMovedCount,
		Snapshot.MassArrivedCount,
		Snapshot.MassDamageAppliedCount));
	ActorSummaryText = FText::FromString(FString::Printf(
		TEXT("Actor: Spawned %d | ActiveTicks %d | Arrived %d | Damage %d"),
		Snapshot.ActorSpawnedCount,
		Snapshot.ActorActiveCount,
		Snapshot.ActorArrivedCount,
		Snapshot.ActorDamageAppliedCount));

	UpdateBoundTextBlocks();
}

FText UMDSDebugOverlayWidget::GetNetModeText() const
{
	return NetModeText;
}

FText UMDSDebugOverlayWidget::GetObjectiveHealthText() const
{
	return ObjectiveHealthText;
}

FText UMDSDebugOverlayWidget::GetWaveSummaryText() const
{
	return WaveSummaryText;
}

FText UMDSDebugOverlayWidget::GetMassSummaryText() const
{
	return MassSummaryText;
}

FText UMDSDebugOverlayWidget::GetActorSummaryText() const
{
	return ActorSummaryText;
}

FString UMDSDebugOverlayWidget::GetNetModeLabel() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return TEXT("NoWorld");
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return TEXT("Standalone");
	case NM_DedicatedServer:
		return TEXT("DedicatedServer");
	case NM_ListenServer:
		return TEXT("ListenServer");
	case NM_Client:
		return TEXT("Client");
	default:
		return TEXT("Unknown");
	}
}

bool UMDSDebugOverlayWidget::HasBoundTextBlocks() const
{
	return NetModeTextBlock
		|| ObjectiveHealthTextBlock
		|| WaveSummaryTextBlock
		|| MassSummaryTextBlock
		|| ActorSummaryTextBlock;
}

void UMDSDebugOverlayWidget::EnsureFallbackLayout()
{
	if (HasBoundTextBlocks()
		|| FallbackNetModeTextBlock
		|| !WidgetTree)
	{
		return;
	}

	UVerticalBox* FallbackRoot = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		TEXT("DebugOverlayFallbackRoot"));
	if (!FallbackRoot)
	{
		return;
	}

	WidgetTree->RootWidget = FallbackRoot;
	FallbackNetModeTextBlock = CreateFallbackTextBlock(
		*FallbackRoot,
		TEXT("FallbackNetModeTextBlock"),
		LOCTEXT("FallbackNetMode", "NetMode: -"));
	FallbackObjectiveHealthTextBlock = CreateFallbackTextBlock(
		*FallbackRoot,
		TEXT("FallbackObjectiveHealthTextBlock"),
		LOCTEXT("FallbackObjectiveHealth", "Objective HP: -"));
	FallbackWaveSummaryTextBlock = CreateFallbackTextBlock(
		*FallbackRoot,
		TEXT("FallbackWaveSummaryTextBlock"),
		LOCTEXT("FallbackWave", "Wave: -"));
	FallbackMassSummaryTextBlock = CreateFallbackTextBlock(
		*FallbackRoot,
		TEXT("FallbackMassSummaryTextBlock"),
		LOCTEXT("FallbackMass", "Mass: -"));
	FallbackActorSummaryTextBlock = CreateFallbackTextBlock(
		*FallbackRoot,
		TEXT("FallbackActorSummaryTextBlock"),
		LOCTEXT("FallbackActor", "Actor: -"));

	UE_LOG(LogMDSProject, Log, TEXT("Debug overlay fallback layout initialized on %s."), *GetNameSafe(this));
}

UTextBlock* UMDSDebugOverlayWidget::CreateFallbackTextBlock(
	UVerticalBox& Parent,
	const FName WidgetName,
	const FText& InitialText)
{
	if (!WidgetTree)
	{
		return nullptr;
	}

	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		WidgetName);
	if (!TextBlock)
	{
		return nullptr;
	}

	TextBlock->SetText(InitialText);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetAutoWrapText(false);
	Parent.AddChildToVerticalBox(TextBlock);
	return TextBlock;
}

void UMDSDebugOverlayWidget::ClearRefreshTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
}

void UMDSDebugOverlayWidget::UpdateBoundTextBlocks()
{
	if (NetModeTextBlock)
	{
		NetModeTextBlock->SetText(NetModeText);
	}
	if (FallbackNetModeTextBlock)
	{
		FallbackNetModeTextBlock->SetText(NetModeText);
	}

	if (ObjectiveHealthTextBlock)
	{
		ObjectiveHealthTextBlock->SetText(ObjectiveHealthText);
	}
	if (FallbackObjectiveHealthTextBlock)
	{
		FallbackObjectiveHealthTextBlock->SetText(ObjectiveHealthText);
	}

	if (WaveSummaryTextBlock)
	{
		WaveSummaryTextBlock->SetText(WaveSummaryText);
	}
	if (FallbackWaveSummaryTextBlock)
	{
		FallbackWaveSummaryTextBlock->SetText(WaveSummaryText);
	}

	if (MassSummaryTextBlock)
	{
		MassSummaryTextBlock->SetText(MassSummaryText);
	}
	if (FallbackMassSummaryTextBlock)
	{
		FallbackMassSummaryTextBlock->SetText(MassSummaryText);
	}

	if (ActorSummaryTextBlock)
	{
		ActorSummaryTextBlock->SetText(ActorSummaryText);
	}
	if (FallbackActorSummaryTextBlock)
	{
		FallbackActorSummaryTextBlock->SetText(ActorSummaryText);
	}
}

#undef LOCTEXT_NAMESPACE
