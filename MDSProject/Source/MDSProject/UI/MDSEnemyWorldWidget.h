#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "MDSEnemyWorldWidget.generated.h"

class AMDSCombatEnemyActor;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MDSPROJECT_API UMDSEnemyWorldWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetEnemyActor(AMDSCombatEnemyActor* InEnemyActor);

	UFUNCTION(BlueprintCallable, Category = "MDS|UI")
	void RefreshFromEnemy();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "MDS|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyHealthTextBlock;

private:
	void EnsureFallbackLayout();
	void UpdateTextBlock();

	FTimerHandle RefreshTimerHandle;
	bool bLoggedInitialRead = false;

	UPROPERTY(Transient)
	TObjectPtr<AMDSCombatEnemyActor> EnemyActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MDS|UI", meta = (AllowPrivateAccess = "true"))
	FText EnemyHealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FallbackEnemyHealthTextBlock;
};
