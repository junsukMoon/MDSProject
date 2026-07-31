#include "UI/MDSRoundSettlementWidget.h"

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
#include "MDSProjectGameState.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectPlayerState.h"

namespace
{
void ConfigureSettlementText(UTextBlock* TextBlock, const int32 FontSize, const FLinearColor Color, const ETextJustify::Type Justification = ETextJustify::Left)
{
	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(Justification);
}
}

TSharedRef<SWidget> UMDSRoundSettlementWidget::RebuildWidget()
{
	EnsureFallbackLayout();
	return Super::RebuildWidget();
}

void UMDSRoundSettlementWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureFallbackLayout();
	RefreshSettlement();
}

void UMDSRoundSettlementWidget::EnsureFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SettlementRoot"));
	WidgetTree->RootWidget = Root;
	UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SettlementDimmer"));
	Dimmer->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.015f, 0.68f));
	UCanvasPanelSlot* DimmerSlot = Root->AddChildToCanvas(Dimmer);
	DimmerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	DimmerSlot->SetOffsets(FMargin(0.0f));

	USizeBox* WindowSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SettlementWindowSize"));
	WindowSize->SetWidthOverride(1120.0f);
	WindowSize->SetHeightOverride(650.0f);
	UCanvasPanelSlot* WindowSlot = Root->AddChildToCanvas(WindowSize);
	WindowSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	WindowSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	WindowSlot->SetAutoSize(true);

	UBorder* WindowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SettlementWindowBorder"));
	WindowBorder->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.06f, 0.98f));
	WindowBorder->SetPadding(FMargin(38.0f, 28.0f));
	WindowSize->AddChild(WindowBorder);
	UVerticalBox* WindowContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettlementWindowContent"));
	WindowBorder->AddChild(WindowContent);
	UTextBlock* WindowTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettlementWindowTitle"));
	WindowTitle->SetText(FText::FromString(TEXT("라운드 정산")));
	ConfigureSettlementText(WindowTitle, 34, FLinearColor(0.95f, 0.82f, 0.38f, 1.0f), ETextJustify::Center);
	WindowContent->AddChildToVerticalBox(WindowTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));

	UHorizontalBox* PanelRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SettlementPanelRow"));
	UVerticalBoxSlot* PanelRowSlot = WindowContent->AddChildToVerticalBox(PanelRow);
	PanelRowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	UVerticalBox* ResultPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RoundResultPanel"));
	UVerticalBox* ShopPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShopPanel"));
	UHorizontalBoxSlot* ResultPanelSlot = PanelRow->AddChildToHorizontalBox(ResultPanel);
	ResultPanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ResultPanelSlot->SetPadding(FMargin(12.0f, 0.0f, 28.0f, 0.0f));
	UHorizontalBoxSlot* ShopPanelSlot = PanelRow->AddChildToHorizontalBox(ShopPanel);
	ShopPanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ShopPanelSlot->SetPadding(FMargin(28.0f, 0.0f, 12.0f, 0.0f));

	ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundResultText"));
	ConfigureSettlementText(ResultText, 20, FLinearColor(0.9f, 0.94f, 1.0f, 1.0f));
	ResultText->SetLineHeightPercentage(1.12f);
	UVerticalBoxSlot* ResultTextSlot = ResultPanel->AddChildToVerticalBox(ResultText);
	ResultTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UTextBlock* ShopTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ShopTitle"));
	ShopTitle->SetText(FText::FromString(TEXT("라운드 상점")));
	ConfigureSettlementText(ShopTitle, 25, FLinearColor(0.35f, 0.82f, 1.0f, 1.0f), ETextJustify::Center);
	ShopPanel->AddChildToVerticalBox(ShopTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	for (int32 Index = 0; Index < 3; ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("ShopOffer%d"), Index));
		Button->SetBackgroundColor(FLinearColor(0.10f, 0.28f, 0.42f, 1.0f));
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetAutoWrapText(true);
		ConfigureSettlementText(Text, 17, FLinearColor::White, ETextJustify::Center);
		Button->AddChild(Text);
		UVerticalBoxSlot* OfferSlot = ShopPanel->AddChildToVerticalBox(Button);
		OfferSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		OfferSlot->SetPadding(FMargin(0.0f, 7.0f));
		OfferButtons.Add(Button);
		OfferTexts.Add(Text);
	}
	OfferButtons[0]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer0);
	OfferButtons[1]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer1);
	OfferButtons[2]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer2);

	SettlementActionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NextRoundReadyButton"));
	SettlementActionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettlementActionText"));
	ConfigureSettlementText(SettlementActionText, 19, FLinearColor::White, ETextJustify::Center);
	SettlementActionButton->SetBackgroundColor(FLinearColor(0.72f, 0.48f, 0.12f, 1.0f));
	SettlementActionButton->AddChild(SettlementActionText);
	ResultPanel->AddChildToVerticalBox(SettlementActionButton)->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
	SettlementActionButton->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleSettlementAction);
}

void UMDSRoundSettlementWidget::RefreshSettlement()
{
	const UWorld* World = GetWorld();
	const AMDSProjectGameState* GameState = World ? World->GetGameState<AMDSProjectGameState>() : nullptr;
	const AMDSProjectPlayerState* PlayerState = GetOwningPlayerState<AMDSProjectPlayerState>();
	if (!GameState || !PlayerState || !ResultText)
	{
		return;
	}

	const FMDSRoundResult& Team = GameState->GetLastRoundResult();
	const FMDSPlayerRoundResult& Player = PlayerState->GetLastRoundResult();
	if (SettlementActionText && SettlementActionButton)
	{
		const bool bFinal = GameState->IsFinalRoundSettlement();
		SettlementActionText->SetText(FText::FromString(bFinal ? TEXT("매치 종료") : TEXT("다음 라운드 준비")));
		SettlementActionButton->SetIsEnabled(bFinal || !PlayerState->IsReadyForNextRound());
	}
	FString UpgradeSummary = TEXT("없음");
	if (!Player.SelectedUpgrades.IsEmpty())
	{
		UpgradeSummary.Reset();
		for (const EMDSLevelUpUpgrade Upgrade : Player.SelectedUpgrades)
		{
			if (!UpgradeSummary.IsEmpty())
			{
				UpgradeSummary += TEXT(", ");
			}
			UpgradeSummary += LexToString(Upgrade);
		}
	}
	ResultText->SetText(FText::FromString(FString::Printf(
		TEXT("라운드 %d 완료\n클리어 시간 %.2f초\n처치 %d / 전체 적 %d\n획득 재화 %d / 경험치 %d\n성 피해 %.0f\n성 HP %.0f (%.0f%%)\n현재 레벨 %d\n보유 재화 %d\n선택 강화: %s"),
		Team.RoundIndex, Team.ClearTime, Player.KillCount, Team.TotalEnemyCount,
		Player.CurrencyEarned, Player.ExperienceEarned, Team.CastleDamageTaken,
		Team.CastleHealthRemaining, Team.CastleHealthPercent * 100.0f,
		Player.CurrentLevel, PlayerState->GetMatchCurrency(), *UpgradeSummary)));

	DisplayedProductIds.Reset();
	const TArray<FMDSShopOffer>& Offers = GameState->GetActiveShopOffers();
	for (int32 Index = 0; Index < OfferTexts.Num(); ++Index)
	{
		if (!Offers.IsValidIndex(Index))
		{
			OfferTexts[Index]->SetText(FText::FromString(TEXT("상품 없음")));
			OfferButtons[Index]->SetIsEnabled(false);
			continue;
		}
		const FMDSShopOffer& Offer = Offers[Index];
		DisplayedProductIds.Add(Offer.ProductId);
		const bool bPurchased = PlayerState->GetPurchasedShopProductIds().Contains(Offer.ProductId);
		const bool bAffordable = PlayerState->GetMatchCurrency() >= Offer.Price;
		OfferTexts[Index]->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s\n가격 %d%s"),
			*Offer.DisplayName.ToString(), *Offer.EffectDescription.ToString(), Offer.Price,
			bPurchased ? TEXT(" (구매 완료)") : TEXT(""))));
		OfferButtons[Index]->SetIsEnabled(!GameState->IsFinalRoundSettlement() && !bPurchased && bAffordable);
	}
}

void UMDSRoundSettlementWidget::SubmitPurchase(const int32 OfferIndex)
{
	if (DisplayedProductIds.IsValidIndex(OfferIndex))
	{
		if (AMDSProjectPlayerController* Controller = GetOwningPlayer<AMDSProjectPlayerController>())
		{
			Controller->RequestShopPurchase(DisplayedProductIds[OfferIndex]);
		}
	}
}

void UMDSRoundSettlementWidget::HandleOffer0() { SubmitPurchase(0); }
void UMDSRoundSettlementWidget::HandleOffer1() { SubmitPurchase(1); }
void UMDSRoundSettlementWidget::HandleOffer2() { SubmitPurchase(2); }

void UMDSRoundSettlementWidget::HandleSettlementAction()
{
	if (AMDSProjectPlayerController* Controller = GetOwningPlayer<AMDSProjectPlayerController>())
	{
		Controller->RequestSettlementAction();
	}
}
