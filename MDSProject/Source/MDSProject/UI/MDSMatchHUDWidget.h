#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "MDSMatchHUDWidget.generated.h"

class UTextBlock;
class UVerticalBox;

UCLASS(BlueprintType, Blueprintable)
class MDSPROJECT_API UMDSMatchHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MDS|UI")
	void RefreshFromGameState();

	UFUNCTION(BlueprintPure, Category = "MDS|UI")
	FText GetWaveText() const { return WaveText; }

	UFUNCTION(BlueprintPure, Category = "MDS|UI")
	FText GetEnemiesText() const { return EnemiesText; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveTextBlock;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemiesTextBlock;

private:
	void EnsureFallbackLayout();
	UTextBlock* CreateFallbackTextBlock(UVerticalBox& Parent, FName WidgetName, const FText& InitialText);
	void UpdateTextBlocks();

	FTimerHandle RefreshTimerHandle;
	bool bLoggedInitialRead = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	FText WaveText;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	FText EnemiesText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FallbackWaveTextBlock;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FallbackEnemiesTextBlock;
};
