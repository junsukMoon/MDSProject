#pragma once

#include "Abilities/GameplayAbility.h"
#include "MDSPlayerFireAbility.generated.h"

UCLASS()
class MDSPROJECT_API UMDSPlayerFireAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMDSPlayerFireAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	double LastServerActivationTimeSeconds = -1000000.0;
};
