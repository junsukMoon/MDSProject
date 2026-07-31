#include "UI/MDSLevelUpChoiceWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectPlayerState.h"

TSharedRef<SWidget> UMDSLevelUpChoiceWidget::RebuildWidget()
{
	EnsureFallbackLayout();
	return Super::RebuildWidget();
}

void UMDSLevelUpChoiceWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureFallbackLayout();
	RefreshChoices();
}

void UMDSLevelUpChoiceWidget::EnsureFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LevelUpRoot"));
	WidgetTree->RootWidget = Root;
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelUpTitle"));
	Title->SetText(FText::FromString(TEXT("레벨 업 - 능력치를 선택하세요")));
	Root->AddChildToVerticalBox(Title);

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Button->AddChild(Text);
		Root->AddChildToVerticalBox(Button);
		ChoiceButtons.Add(Button);
		ChoiceTexts.Add(Text);
	}
	ChoiceButtons[0]->OnClicked.AddDynamic(this, &UMDSLevelUpChoiceWidget::HandleChoice0);
	ChoiceButtons[1]->OnClicked.AddDynamic(this, &UMDSLevelUpChoiceWidget::HandleChoice1);
	ChoiceButtons[2]->OnClicked.AddDynamic(this, &UMDSLevelUpChoiceWidget::HandleChoice2);
}

void UMDSLevelUpChoiceWidget::RefreshChoices()
{
	const AMDSProjectPlayerState* PlayerState = GetOwningPlayerState<AMDSProjectPlayerState>();
	if (!PlayerState)
	{
		return;
	}
	DisplayedChoices = PlayerState->GetActiveLevelUpChoices();
	for (int32 Index = 0; Index < ChoiceTexts.Num(); ++Index)
	{
		const FString Label = DisplayedChoices.IsValidIndex(Index)
			? FString(LexToString(DisplayedChoices[Index]))
			: TEXT("-");
		ChoiceTexts[Index]->SetText(FText::FromString(Label));
	}
}

void UMDSLevelUpChoiceWidget::SubmitChoice(const int32 ChoiceIndex)
{
	if (DisplayedChoices.IsValidIndex(ChoiceIndex))
	{
		if (AMDSProjectPlayerController* Controller = GetOwningPlayer<AMDSProjectPlayerController>())
		{
			Controller->RequestLevelUpChoice(DisplayedChoices[ChoiceIndex]);
		}
	}
}

void UMDSLevelUpChoiceWidget::HandleChoice0() { SubmitChoice(0); }
void UMDSLevelUpChoiceWidget::HandleChoice1() { SubmitChoice(1); }
void UMDSLevelUpChoiceWidget::HandleChoice2() { SubmitChoice(2); }
