#pragma once

#include "Abilities/GameplayAbility.h"
#include "MDSEnemyAttackCastleAbility.generated.h"

UCLASS()
class MDSPROJECT_API UMDSEnemyAttackCastleAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMDSEnemyAttackCastleAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
