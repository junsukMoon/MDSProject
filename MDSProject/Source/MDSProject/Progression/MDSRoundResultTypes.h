#pragma once

#include "CoreMinimal.h"
#include "Progression/MDSLevelUpTypes.h"
#include "MDSRoundResultTypes.generated.h"

USTRUCT(BlueprintType)
struct FMDSRoundResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RoundIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	float ClearTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalEnemyCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float CastleDamageTaken = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CastleHealthRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CastleHealthPercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMDSPlayerRoundResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrencyEarned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ExperienceEarned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrencySpent = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCurrency = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<EMDSLevelUpUpgrade> SelectedUpgrades;

	UPROPERTY(BlueprintReadOnly)
	bool bReadyForNextRound = false;
};
