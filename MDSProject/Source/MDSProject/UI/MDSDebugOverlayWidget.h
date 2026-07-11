#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TimerManager.h"
#include "MDSDebugOverlayWidget.generated.h"

class UTextBlock;

UCLASS(Abstract, BlueprintType, Blueprintable)
class MDSPROJECT_API UMDSDebugOverlayWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MDS|Debug")
	void RefreshFromWorld();

	UFUNCTION(BlueprintPure, Category = "MDS|Debug")
	FText GetNetModeText() const;

	UFUNCTION(BlueprintPure, Category = "MDS|Debug")
	FText GetObjectiveHealthText() const;

	UFUNCTION(BlueprintPure, Category = "MDS|Debug")
	FText GetWaveSummaryText() const;

	UFUNCTION(BlueprintPure, Category = "MDS|Debug")
	FText GetMassSummaryText() const;

	UFUNCTION(BlueprintPure, Category = "MDS|Debug")
	FText GetActorSummaryText() const;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|Debug", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NetModeTextBlock;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|Debug", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveHealthTextBlock;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|Debug", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveSummaryTextBlock;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|Debug", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MassSummaryTextBlock;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|Debug", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ActorSummaryTextBlock;

private:
	FString GetNetModeLabel() const;
	void ClearRefreshTimer();
	void UpdateBoundTextBlocks();

	FTimerHandle RefreshTimerHandle;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|Debug", meta = (AllowPrivateAccess = "true"))
	FText NetModeText;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|Debug", meta = (AllowPrivateAccess = "true"))
	FText ObjectiveHealthText;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|Debug", meta = (AllowPrivateAccess = "true"))
	FText WaveSummaryText;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|Debug", meta = (AllowPrivateAccess = "true"))
	FText MassSummaryText;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|Debug", meta = (AllowPrivateAccess = "true"))
	FText ActorSummaryText;
};
