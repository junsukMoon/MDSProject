#include "UI/MDSObjectiveWorldWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "MDSProject.h"
#include "Objective/MDSObjectiveActor.h"

#define LOCTEXT_NAMESPACE "MDSObjectiveWorldWidget"

TSharedRef<SWidget> UMDSObjectiveWorldWidget::RebuildWidget()
{
	EnsureFallbackLayout();
	return Super::RebuildWidget();
}

void UMDSObjectiveWorldWidget::SetObjectiveActor(AMDSObjectiveActor* InObjectiveActor)
{
	ObjectiveActor = InObjectiveActor;
	RefreshFromObjective();
}

void UMDSObjectiveWorldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFallbackLayout();
	RefreshFromObjective();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UMDSObjectiveWorldWidget::RefreshFromObjective, 0.5f, true, 0.5f);
	}
}

void UMDSObjectiveWorldWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UMDSObjectiveWorldWidget::RefreshFromObjective()
{
	if (!ObjectiveActor)
	{
		ObjectiveHealthText = LOCTEXT("NoObjective", "Objective HP: -");
		UpdateTextBlock();
		return;
	}

	ObjectiveHealthText = FText::FromString(FString::Printf(
		TEXT("Objective HP %.0f / %.0f"),
		ObjectiveActor->GetCurrentHealth(),
		ObjectiveActor->GetMaxHealth()));

	if (!bLoggedInitialRead)
	{
		bLoggedInitialRead = true;
		UE_LOG(LogMDSProject, Log, TEXT("MDS Objective World UI read ObjectiveActor health: %.1f / %.1f on %s."),
			ObjectiveActor->GetCurrentHealth(),
			ObjectiveActor->GetMaxHealth(),
			*GetNameSafe(ObjectiveActor));
	}

	UpdateTextBlock();
}

void UMDSObjectiveWorldWidget::EnsureFallbackLayout()
{
	if (ObjectiveHealthTextBlock || FallbackObjectiveHealthTextBlock || !WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	FallbackObjectiveHealthTextBlock = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("FallbackObjectiveHealthTextBlock"));
	if (!FallbackObjectiveHealthTextBlock)
	{
		return;
	}

	FallbackObjectiveHealthTextBlock->SetText(LOCTEXT("FallbackObjectiveHealth", "Objective HP: -"));
	FallbackObjectiveHealthTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FallbackObjectiveHealthTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	FallbackObjectiveHealthTextBlock->SetShadowColorAndOpacity(FLinearColor::Black);
	WidgetTree->RootWidget = FallbackObjectiveHealthTextBlock;

	UE_LOG(LogMDSProject, Log, TEXT("MDS Objective World UI fallback layout initialized on %s."), *GetNameSafe(this));
}

void UMDSObjectiveWorldWidget::UpdateTextBlock()
{
	if (ObjectiveHealthTextBlock)
	{
		ObjectiveHealthTextBlock->SetText(ObjectiveHealthText);
	}
	if (FallbackObjectiveHealthTextBlock)
	{
		FallbackObjectiveHealthTextBlock->SetText(ObjectiveHealthText);
	}
}

#undef LOCTEXT_NAMESPACE
