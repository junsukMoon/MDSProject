#include "Attributes/MDSCombatAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMDSCombatAttributeSet::UMDSCombatAttributeSet()
{
	InitAttackPowerMultiplier(1.0f);
	InitFireRateMultiplier(1.0f);
	InitMoveSpeedMultiplier(1.0f);
}

void UMDSCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMDSCombatAttributeSet, AttackPowerMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDSCombatAttributeSet, FireRateMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDSCombatAttributeSet, MoveSpeedMultiplier, COND_None, REPNOTIFY_Always);
}

void UMDSCombatAttributeSet::OnRep_AttackPowerMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDSCombatAttributeSet, AttackPowerMultiplier, OldValue);
}

void UMDSCombatAttributeSet::OnRep_FireRateMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDSCombatAttributeSet, FireRateMultiplier, OldValue);
}

void UMDSCombatAttributeSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDSCombatAttributeSet, MoveSpeedMultiplier, OldValue);
}
