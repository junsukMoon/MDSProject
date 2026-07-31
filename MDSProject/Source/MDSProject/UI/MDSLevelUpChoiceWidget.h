#pragma once

#include "Blueprint/UserWidget.h"
#include "Progression/MDSLevelUpTypes.h"
#include "MDSLevelUpChoiceWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class MDSPROJECT_API UMDSLevelUpChoiceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshChoices();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	void EnsureFallbackLayout();
	void SubmitChoice(int32 ChoiceIndex);

	UFUNCTION()
	void HandleChoice0();
	UFUNCTION()
	void HandleChoice1();
	UFUNCTION()
	void HandleChoice2();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> ChoiceButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ChoiceTexts;

	TArray<EMDSLevelUpUpgrade> DisplayedChoices;
};
