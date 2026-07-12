#include "UI/MDSEnemyWorldWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Combat/MDSCombatEnemyActor.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "MDSProject.h"

#define LOCTEXT_NAMESPACE "MDSEnemyWorldWidget"

void UMDSEnemyWorldWidget::SetEnemyActor(AMDSCombatEnemyActor* InEnemyActor)
{
	EnemyActor = InEnemyActor;
	RefreshFromEnemy();
}

void UMDSEnemyWorldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFallbackLayout();
	RefreshFromEnemy();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UMDSEnemyWorldWidget::RefreshFromEnemy, 0.5f, true, 0.5f);
	}
}

void UMDSEnemyWorldWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UMDSEnemyWorldWidget::RefreshFromEnemy()
{
	if (!EnemyActor)
	{
		EnemyHealthText = LOCTEXT("NoEnemy", "Enemy HP: -");
		UpdateTextBlock();
		return;
	}

	EnemyHealthText = FText::FromString(FString::Printf(
		TEXT("Enemy HP %.0f / %.0f"),
		EnemyActor->GetCurrentHealth(),
		EnemyActor->GetMaxHealth()));

	if (!bLoggedInitialRead)
	{
		bLoggedInitialRead = true;
		UE_LOG(LogMDSProject, Log, TEXT("MDS Enemy World UI read CombatEnemy health: %.1f / %.1f on %s."),
			EnemyActor->GetCurrentHealth(),
			EnemyActor->GetMaxHealth(),
			*GetNameSafe(EnemyActor));
	}

	UpdateTextBlock();
}

void UMDSEnemyWorldWidget::EnsureFallbackLayout()
{
	if (EnemyHealthTextBlock || FallbackEnemyHealthTextBlock || !WidgetTree)
	{
		return;
	}

	FallbackEnemyHealthTextBlock = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("FallbackEnemyHealthTextBlock"));
	if (!FallbackEnemyHealthTextBlock)
	{
		return;
	}

	FallbackEnemyHealthTextBlock->SetText(LOCTEXT("FallbackEnemyHealth", "Enemy HP: -"));
	WidgetTree->RootWidget = FallbackEnemyHealthTextBlock;

	UE_LOG(LogMDSProject, Log, TEXT("MDS Enemy World UI fallback layout initialized on %s."), *GetNameSafe(this));
}

void UMDSEnemyWorldWidget::UpdateTextBlock()
{
	if (EnemyHealthTextBlock)
	{
		EnemyHealthTextBlock->SetText(EnemyHealthText);
	}
	if (FallbackEnemyHealthTextBlock)
	{
		FallbackEnemyHealthTextBlock->SetText(EnemyHealthText);
	}
}

#undef LOCTEXT_NAMESPACE
