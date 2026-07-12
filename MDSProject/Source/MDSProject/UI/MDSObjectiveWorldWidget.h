#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "MDSObjectiveWorldWidget.generated.h"

class AMDSObjectiveActor;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MDSPROJECT_API UMDSObjectiveWorldWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetObjectiveActor(AMDSObjectiveActor* InObjectiveActor);

	UFUNCTION(BlueprintCallable, Category = "MDS|UI")
	void RefreshFromObjective();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveHealthTextBlock;

private:
	void EnsureFallbackLayout();
	void UpdateTextBlock();

	FTimerHandle RefreshTimerHandle;
	bool bLoggedInitialRead = false;

	UPROPERTY(Transient)
	TObjectPtr<AMDSObjectiveActor> ObjectiveActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	FText ObjectiveHealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FallbackObjectiveHealthTextBlock;
};
