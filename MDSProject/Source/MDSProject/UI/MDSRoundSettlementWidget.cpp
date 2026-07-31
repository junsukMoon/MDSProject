#include "UI/MDSRoundSettlementWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "MDSProjectGameState.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectPlayerState.h"

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

	UHorizontalBox* Root = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SettlementRoot"));
	WidgetTree->RootWidget = Root;
	UVerticalBox* ResultPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RoundResultPanel"));
	UVerticalBox* ShopPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShopPanel"));
	Root->AddChildToHorizontalBox(ResultPanel);
	Root->AddChildToHorizontalBox(ShopPanel);

	ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundResultText"));
	ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	ResultPanel->AddChildToVerticalBox(ResultText);

	UTextBlock* ShopTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ShopTitle"));
	ShopTitle->SetText(FText::FromString(TEXT("라운드 상점")));
	ShopPanel->AddChildToVerticalBox(ShopTitle);
	for (int32 Index = 0; Index < 3; ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Button->AddChild(Text);
		ShopPanel->AddChildToVerticalBox(Button);
		OfferButtons.Add(Button);
		OfferTexts.Add(Text);
	}
	OfferButtons[0]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer0);
	OfferButtons[1]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer1);
	OfferButtons[2]->OnClicked.AddDynamic(this, &UMDSRoundSettlementWidget::HandleOffer2);

	UTextBlock* ReadyPlaceholder = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundReadyButtonPlaceholder"));
	ReadyPlaceholder->SetText(FText::FromString(TEXT("다음 라운드 준비 기능은 Phase R11에서 연결됩니다.")));
	ResultPanel->AddChildToVerticalBox(ReadyPlaceholder);
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
		OfferButtons[Index]->SetIsEnabled(!bPurchased && bAffordable);
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
