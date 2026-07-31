#include "Effects/MDSLevelUpUpgradeEffects.h"

#include "Attributes/MDSCombatAttributeSet.h"

namespace
{
void AddInfiniteModifier(
	UGameplayEffect& Effect,
	const FGameplayAttribute& Attribute,
	const float AdditiveMagnitude)
{
	Effect.DurationPolicy = EGameplayEffectDurationType::Infinite;
	FGameplayModifierInfo& Modifier = Effect.Modifiers.AddDefaulted_GetRef();
	Modifier.Attribute = Attribute;
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FScalableFloat(AdditiveMagnitude);
}
}

UMDSAttackPowerUpgradeEffect::UMDSAttackPowerUpgradeEffect()
{
	AddInfiniteModifier(*this, UMDSCombatAttributeSet::GetAttackPowerMultiplierAttribute(), 0.15f);
}

UMDSFireRateUpgradeEffect::UMDSFireRateUpgradeEffect()
{
	AddInfiniteModifier(*this, UMDSCombatAttributeSet::GetFireRateMultiplierAttribute(), 0.15f);
}

UMDSMoveSpeedUpgradeEffect::UMDSMoveSpeedUpgradeEffect()
{
	AddInfiniteModifier(*this, UMDSCombatAttributeSet::GetMoveSpeedMultiplierAttribute(), 0.10f);
}
