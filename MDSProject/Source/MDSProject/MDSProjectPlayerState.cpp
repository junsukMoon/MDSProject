#include "MDSProjectPlayerState.h"

#include "AbilitySystemComponent.h"

AMDSProjectPlayerState::AMDSProjectPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AMDSProjectPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
