#include "UI/MDSLevelUpChoiceWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectPlayerState.h"

namespace
{
void ConfigureLevelUpText(UTextBlock* TextBlock, const int32 FontSize, const FLinearColor Color)
{
	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(ETextJustify::Center);
}

FText GetUpgradeTitle(const EMDSLevelUpUpgrade Upgrade)
{
	switch (Upgrade)
	{
	case EMDSLevelUpUpgrade::AttackPower: return FText::FromString(TEXT("공격력 증폭"));
	case EMDSLevelUpUpgrade::FireRate: return FText::FromString(TEXT("속사 강화"));
	case EMDSLevelUpUpgrade::MoveSpeed: return FText::FromString(TEXT("기동력 강화"));
	default: return FText::FromString(TEXT("알 수 없는 강화"));
	}
}

FText GetUpgradeDescription(const EMDSLevelUpUpgrade Upgrade)
{
	switch (Upgrade)
	{
	case EMDSLevelUpUpgrade::AttackPower: return FText::FromString(TEXT("공격력이 15% 증가합니다.\n더 강한 한 발로 적을 제압하세요."));
	case EMDSLevelUpUpgrade::FireRate: return FText::FromString(TEXT("발사속도가 15% 증가합니다.\n더 빠르게 화력을 집중하세요."));
	case EMDSLevelUpUpgrade::MoveSpeed: return FText::FromString(TEXT("이동속도가 10% 증가합니다.\n전장을 빠르게 가로지르세요."));
	default: return FText::GetEmpty();
	}
}

FLinearColor GetUpgradeAccent(const int32 Index)
{
	static const FLinearColor Accents[] = {
		FLinearColor(0.82f, 0.24f, 0.18f, 1.0f),
		FLinearColor(0.22f, 0.58f, 0.92f, 1.0f),
		FLinearColor(0.22f, 0.78f, 0.48f, 1.0f)
	};
	return Accents[FMath::Clamp(Index, 0, 2)];
}
}

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
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("LevelUpRoot"));
	WidgetTree->RootWidget = Root;

	UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LevelUpDimmer"));
	Dimmer->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.018f, 0.82f));
	UCanvasPanelSlot* DimmerSlot = Root->AddChildToCanvas(Dimmer);
	DimmerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	DimmerSlot->SetOffsets(FMargin(0.0f));

	USizeBox* ModalSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LevelUpModalSize"));
	ModalSize->SetWidthOverride(1180.0f);
	ModalSize->SetHeightOverride(560.0f);
	UCanvasPanelSlot* ModalSlot = Root->AddChildToCanvas(ModalSize);
	ModalSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	ModalSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ModalSlot->SetAutoSize(true);

	UBorder* ModalBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LevelUpModalBorder"));
	ModalBorder->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.065f, 0.98f));
	ModalBorder->SetPadding(FMargin(42.0f, 28.0f));
	ModalSize->AddChild(ModalBorder);
	UVerticalBox* ModalContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LevelUpModalContent"));
	ModalBorder->AddChild(ModalContent);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelUpTitle"));
	Title->SetText(FText::FromString(TEXT("증강 선택")));
	ConfigureLevelUpText(Title, 38, FLinearColor(0.95f, 0.82f, 0.38f, 1.0f));
	ModalContent->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelUpSubtitle"));
	Subtitle->SetText(FText::FromString(TEXT("전투를 이어갈 강화 하나를 선택하세요")));
	ConfigureLevelUpText(Subtitle, 19, FLinearColor(0.72f, 0.78f, 0.9f, 1.0f));
	ModalContent->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 26.0f));

	UHorizontalBox* CardRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LevelUpCardRow"));
	UVerticalBoxSlot* CardRowSlot = ModalContent->AddChildToVerticalBox(CardRow);
	CardRowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("AugmentCard%d"), Index));
		Button->SetBackgroundColor(GetUpgradeAccent(Index));
		UVerticalBox* CardContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Button->AddChild(CardContent);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		ConfigureLevelUpText(Text, 25, FLinearColor::White);
		CardContent->AddChildToVerticalBox(Text)->SetPadding(FMargin(12.0f, 38.0f, 12.0f, 18.0f));
		UTextBlock* Description = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Description->SetAutoWrapText(true);
		ConfigureLevelUpText(Description, 17, FLinearColor(0.92f, 0.94f, 1.0f, 1.0f));
		UVerticalBoxSlot* DescriptionSlot = CardContent->AddChildToVerticalBox(Description);
		DescriptionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		DescriptionSlot->SetPadding(FMargin(18.0f, 0.0f, 18.0f, 18.0f));
		UTextBlock* SelectText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		SelectText->SetText(FText::FromString(TEXT("선택")));
		ConfigureLevelUpText(SelectText, 19, FLinearColor(1.0f, 0.9f, 0.52f, 1.0f));
		CardContent->AddChildToVerticalBox(SelectText)->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 26.0f));
		UHorizontalBoxSlot* ButtonSlot = CardRow->AddChildToHorizontalBox(Button);
		ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ButtonSlot->SetPadding(FMargin(9.0f));
		ChoiceButtons.Add(Button);
		ChoiceTexts.Add(Text);
		ChoiceDescriptionTexts.Add(Description);
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
		const bool bHasChoice = DisplayedChoices.IsValidIndex(Index);
		ChoiceTexts[Index]->SetText(bHasChoice ? GetUpgradeTitle(DisplayedChoices[Index]) : FText::FromString(TEXT("선택지 준비 중")));
		ChoiceDescriptionTexts[Index]->SetText(bHasChoice ? GetUpgradeDescription(DisplayedChoices[Index]) : FText::GetEmpty());
		ChoiceButtons[Index]->SetIsEnabled(bHasChoice);
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

void UMDSLevelUpChoiceWidget::ClickChoiceForVerification(const int32 ChoiceIndex)
{
	if (ChoiceButtons.IsValidIndex(ChoiceIndex) && ChoiceButtons[ChoiceIndex]->GetIsEnabled())
	{
		ChoiceButtons[ChoiceIndex]->OnClicked.Broadcast();
	}
}

void UMDSLevelUpChoiceWidget::HandleChoice0() { SubmitChoice(0); }
void UMDSLevelUpChoiceWidget::HandleChoice1() { SubmitChoice(1); }
void UMDSLevelUpChoiceWidget::HandleChoice2() { SubmitChoice(2); }
