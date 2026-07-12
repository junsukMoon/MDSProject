#include "UI/MDSMatchHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "MDSProject.h"
#include "MDSProjectGameState.h"

#define LOCTEXT_NAMESPACE "MDSMatchHUDWidget"

void UMDSMatchHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFallbackLayout();
	RefreshFromGameState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UMDSMatchHUDWidget::RefreshFromGameState, 0.5f, true, 0.5f);
	}
}

void UMDSMatchHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UMDSMatchHUDWidget::RefreshFromGameState()
{
	const UWorld* World = GetWorld();
	const AMDSProjectGameState* MDSGameState = World ? World->GetGameState<AMDSProjectGameState>() : nullptr;
	if (!MDSGameState)
	{
		WaveText = LOCTEXT("NoGameStateWave", "Wave: -");
		EnemiesText = LOCTEXT("NoGameStateEnemies", "Enemies: -");
		UpdateTextBlocks();
		return;
	}

	WaveText = FText::FromString(FString::Printf(
		TEXT("Wave %d | Active %s"),
		MDSGameState->GetCurrentWaveIndex(),
		MDSGameState->IsWaveActive() ? TEXT("true") : TEXT("false")));
	EnemiesText = FText::FromString(FString::Printf(
		TEXT("Enemies %d / %d"),
		MDSGameState->GetEnemiesRemaining(),
		MDSGameState->GetTotalEnemiesThisWave()));

	if (!bLoggedInitialRead)
	{
		bLoggedInitialRead = true;
		UE_LOG(LogMDSProject, Log, TEXT("MDS Match HUD read GameState wave state: Wave=%d Remaining=%d Total=%d Active=%s."),
			MDSGameState->GetCurrentWaveIndex(),
			MDSGameState->GetEnemiesRemaining(),
			MDSGameState->GetTotalEnemiesThisWave(),
			MDSGameState->IsWaveActive() ? TEXT("true") : TEXT("false"));
	}

	UpdateTextBlocks();
}

void UMDSMatchHUDWidget::EnsureFallbackLayout()
{
	if ((WaveTextBlock && EnemiesTextBlock) || FallbackWaveTextBlock || !WidgetTree)
	{
		return;
	}

	UVerticalBox* FallbackRoot = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MatchHUDFallbackRoot"));
	if (!FallbackRoot)
	{
		return;
	}

	WidgetTree->RootWidget = FallbackRoot;
	FallbackWaveTextBlock = CreateFallbackTextBlock(*FallbackRoot, TEXT("FallbackWaveTextBlock"), LOCTEXT("FallbackWave", "Wave: -"));
	FallbackEnemiesTextBlock = CreateFallbackTextBlock(*FallbackRoot, TEXT("FallbackEnemiesTextBlock"), LOCTEXT("FallbackEnemies", "Enemies: -"));

	UE_LOG(LogMDSProject, Log, TEXT("MDS Match HUD fallback layout initialized on %s."), *GetNameSafe(this));
}

UTextBlock* UMDSMatchHUDWidget::CreateFallbackTextBlock(UVerticalBox& Parent, const FName WidgetName, const FText& InitialText)
{
	if (!WidgetTree)
	{
		return nullptr;
	}

	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	if (!TextBlock)
	{
		return nullptr;
	}

	TextBlock->SetText(InitialText);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor::Black);
	Parent.AddChildToVerticalBox(TextBlock);
	return TextBlock;
}

void UMDSMatchHUDWidget::UpdateTextBlocks()
{
	if (WaveTextBlock)
	{
		WaveTextBlock->SetText(WaveText);
	}
	if (FallbackWaveTextBlock)
	{
		FallbackWaveTextBlock->SetText(WaveText);
	}

	if (EnemiesTextBlock)
	{
		EnemiesTextBlock->SetText(EnemiesText);
	}
	if (FallbackEnemiesTextBlock)
	{
		FallbackEnemiesTextBlock->SetText(EnemiesText);
	}
}

#undef LOCTEXT_NAMESPACE
