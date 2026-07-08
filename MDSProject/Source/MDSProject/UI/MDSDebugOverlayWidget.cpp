#include "UI/MDSDebugOverlayWidget.h"

#include "Components/TextBlock.h"
#include "Debug/MDSDebugStateSubsystem.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "MDSDebugOverlayWidget"

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

	if (ObjectiveHealthTextBlock)
	{
		ObjectiveHealthTextBlock->SetText(ObjectiveHealthText);
	}

	if (WaveSummaryTextBlock)
	{
		WaveSummaryTextBlock->SetText(WaveSummaryText);
	}

	if (MassSummaryTextBlock)
	{
		MassSummaryTextBlock->SetText(MassSummaryText);
	}

	if (ActorSummaryTextBlock)
	{
		ActorSummaryTextBlock->SetText(ActorSummaryText);
	}
}

#undef LOCTEXT_NAMESPACE
