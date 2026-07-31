#pragma once

#include "CoreMinimal.h"
#include "Progression/MDSLevelUpTypes.h"
#include "MDSShopTypes.generated.h"

USTRUCT(BlueprintType)
struct FMDSShopOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ProductId;

	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FText EffectDescription;

	UPROPERTY(BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(BlueprintReadOnly)
	EMDSLevelUpUpgrade Upgrade = EMDSLevelUpUpgrade::AttackPower;
};
