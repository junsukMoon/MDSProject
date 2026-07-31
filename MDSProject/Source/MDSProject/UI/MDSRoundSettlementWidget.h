#pragma once

#include "Blueprint/UserWidget.h"
#include "MDSRoundSettlementWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class MDSPROJECT_API UMDSRoundSettlementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshSettlement();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	void EnsureFallbackLayout();
	void SubmitPurchase(int32 OfferIndex);

	UFUNCTION()
	void HandleOffer0();
	UFUNCTION()
	void HandleOffer1();
	UFUNCTION()
	void HandleOffer2();
	UFUNCTION()
	void HandleSettlementAction();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettlementActionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SettlementActionText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> OfferButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> OfferTexts;

	TArray<FName> DisplayedProductIds;
};
