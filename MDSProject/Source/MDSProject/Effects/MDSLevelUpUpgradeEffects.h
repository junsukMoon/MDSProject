#pragma once

#include "GameplayEffect.h"
#include "MDSLevelUpUpgradeEffects.generated.h"

UCLASS()
class MDSPROJECT_API UMDSAttackPowerUpgradeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMDSAttackPowerUpgradeEffect();
};

UCLASS()
class MDSPROJECT_API UMDSFireRateUpgradeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMDSFireRateUpgradeEffect();
};

UCLASS()
class MDSPROJECT_API UMDSMoveSpeedUpgradeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMDSMoveSpeedUpgradeEffect();
};
