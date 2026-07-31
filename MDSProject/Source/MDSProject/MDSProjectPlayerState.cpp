#include "MDSProjectPlayerState.h"

#include "Abilities/MDSPlayerFireAbility.h"
#include "AbilitySystemComponent.h"

AMDSProjectPlayerState::AMDSProjectPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	FireAbilityClass = UMDSPlayerFireAbility::StaticClass();
}

void AMDSProjectPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && AbilitySystemComponent && FireAbilityClass)
	{
		FireAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbilityClass, 1));
	}
}

UAbilitySystemComponent* AMDSProjectPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AMDSProjectPlayerState::TryActivateFireAbility(const FVector& RequestedAimPoint)
{
	if (!HasAuthority() || !AbilitySystemComponent || !FireAbilityHandle.IsValid())
	{
		return false;
	}

	PendingFireAimPoint = RequestedAimPoint;
	bHasPendingFireAimPoint = true;
	const bool bActivated = AbilitySystemComponent->TryActivateAbility(FireAbilityHandle);
	if (!bActivated)
	{
		bHasPendingFireAimPoint = false;
	}
	return bActivated;
}

bool AMDSProjectPlayerState::ConsumePendingFireAimPoint(FVector& OutAimPoint)
{
	if (!HasAuthority() || !bHasPendingFireAimPoint)
	{
		return false;
	}

	OutAimPoint = PendingFireAimPoint;
	bHasPendingFireAimPoint = false;
	return true;
}
