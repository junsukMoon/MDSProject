#pragma once

#include "CoreMinimal.h"
#include "MDSLevelUpTypes.generated.h"

UENUM(BlueprintType)
enum class EMDSLevelUpUpgrade : uint8
{
	AttackPower,
	FireRate,
	MoveSpeed
};

inline const TCHAR* LexToString(const EMDSLevelUpUpgrade Upgrade)
{
	switch (Upgrade)
	{
	case EMDSLevelUpUpgrade::AttackPower:
		return TEXT("AttackPower");
	case EMDSLevelUpUpgrade::FireRate:
		return TEXT("FireRate");
	case EMDSLevelUpUpgrade::MoveSpeed:
		return TEXT("MoveSpeed");
	default:
		return TEXT("Unknown");
	}
}
