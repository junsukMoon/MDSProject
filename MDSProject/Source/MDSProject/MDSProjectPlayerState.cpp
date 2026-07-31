#include "MDSProjectPlayerState.h"

#include "Abilities/MDSPlayerFireAbility.h"
#include "AbilitySystemComponent.h"
#include "Attributes/MDSCombatAttributeSet.h"
#include "Effects/MDSLevelUpUpgradeEffects.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSPlayerProgression, Log, All);

namespace
{
constexpr int32 BaseExperiencePerLevel = 100;
constexpr int32 ExperienceIncreasePerLevel = 50;
}

AMDSProjectPlayerState::AMDSProjectPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	CombatAttributeSet = CreateDefaultSubobject<UMDSCombatAttributeSet>(TEXT("CombatAttributeSet"));
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

void AMDSProjectPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSProjectPlayerState, MatchCurrency);
	DOREPLIFETIME(AMDSProjectPlayerState, CurrentExperience);
	DOREPLIFETIME(AMDSProjectPlayerState, CurrentLevel);
	DOREPLIFETIME(AMDSProjectPlayerState, PendingLevelUpChoices);
	DOREPLIFETIME(AMDSProjectPlayerState, ActiveLevelUpChoices);
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

void AMDSProjectPlayerState::GrantMatchReward(const int32 CurrencyAmount, const int32 ExperienceAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSPlayerProgression, Warning,
			TEXT("Rejected non-authority match reward grant. PlayerState=%s."),
			*GetNameSafe(this));
		return;
	}

	const int32 PreviousCurrency = MatchCurrency;
	const int32 PreviousExperience = CurrentExperience;
	const int32 PreviousLevel = CurrentLevel;
	const int32 PreviousPendingChoices = PendingLevelUpChoices;

	MatchCurrency += FMath::Max(0, CurrencyAmount);
	CurrentExperience += FMath::Max(0, ExperienceAmount);

	while (CurrentExperience >= GetExperienceRequiredForNextLevel())
	{
		CurrentExperience -= GetExperienceRequiredForNextLevel();
		++CurrentLevel;
		++PendingLevelUpChoices;
	}

	UE_LOG(LogMDSPlayerProgression, Log,
		TEXT("MDS Progression | RewardGranted | PlayerState=%s | Currency=%d->%d | Experience=%d->%d | Level=%d->%d | PendingChoices=%d->%d."),
		*GetNameSafe(this),
		PreviousCurrency,
		MatchCurrency,
		PreviousExperience,
		CurrentExperience,
		PreviousLevel,
		CurrentLevel,
		PreviousPendingChoices,
		PendingLevelUpChoices);

	ForceNetUpdate();
}

void AMDSProjectPlayerState::PrepareLevelUpChoices()
{
	if (!HasAuthority() || PendingLevelUpChoices <= 0)
	{
		return;
	}

	ActiveLevelUpChoices = {
		EMDSLevelUpUpgrade::AttackPower,
		EMDSLevelUpUpgrade::FireRate,
		EMDSLevelUpUpgrade::MoveSpeed
	};
	ForceNetUpdate();
}

bool AMDSProjectPlayerState::TryApplyLevelUpChoice(const EMDSLevelUpUpgrade Upgrade)
{
	if (!HasAuthority() || PendingLevelUpChoices <= 0 || !ActiveLevelUpChoices.Contains(Upgrade)
		|| !AbilitySystemComponent)
	{
		UE_LOG(LogMDSPlayerProgression, Warning,
			TEXT("MDS LevelUp | ChoiceRejected | PlayerState=%s | Upgrade=%s | Pending=%d."),
			*GetNameSafe(this),
			LexToString(Upgrade),
			PendingLevelUpChoices);
		return false;
	}

	TSubclassOf<UGameplayEffect> EffectClass;
	switch (Upgrade)
	{
	case EMDSLevelUpUpgrade::AttackPower:
		EffectClass = UMDSAttackPowerUpgradeEffect::StaticClass();
		break;
	case EMDSLevelUpUpgrade::FireRate:
		EffectClass = UMDSFireRateUpgradeEffect::StaticClass();
		break;
	case EMDSLevelUpUpgrade::MoveSpeed:
		EffectClass = UMDSMoveSpeedUpgradeEffect::StaticClass();
		break;
	default:
		return false;
	}

	const UGameplayEffect* Effect = EffectClass->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle AppliedHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
		Effect,
		1.0f,
		AbilitySystemComponent->MakeEffectContext());
	if (!AppliedHandle.IsValid())
	{
		return false;
	}

	--PendingLevelUpChoices;
	ActiveLevelUpChoices.Reset();
	if (PendingLevelUpChoices > 0)
	{
		PrepareLevelUpChoices();
	}

	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			if (CachedBaseWalkSpeed <= 0.0f)
			{
				CachedBaseWalkSpeed = Movement->MaxWalkSpeed;
			}
			Movement->MaxWalkSpeed = CachedBaseWalkSpeed * GetMoveSpeedMultiplier();
		}
	}

	UE_LOG(LogMDSPlayerProgression, Log,
		TEXT("MDS LevelUp | ChoiceApplied | PlayerState=%s | Upgrade=%s | Pending=%d | Attack=%.2f | FireRate=%.2f | MoveSpeed=%.2f."),
		*GetNameSafe(this),
		LexToString(Upgrade),
		PendingLevelUpChoices,
		GetAttackPowerMultiplier(),
		GetFireRateMultiplier(),
		GetMoveSpeedMultiplier());
	ForceNetUpdate();
	return true;
}

int32 AMDSProjectPlayerState::GetExperienceRequiredForNextLevel() const
{
	return BaseExperiencePerLevel + FMath::Max(0, CurrentLevel - 1) * ExperienceIncreasePerLevel;
}

float AMDSProjectPlayerState::GetAttackPowerMultiplier() const
{
	return CombatAttributeSet ? CombatAttributeSet->GetAttackPowerMultiplier() : 1.0f;
}

float AMDSProjectPlayerState::GetFireRateMultiplier() const
{
	return CombatAttributeSet ? CombatAttributeSet->GetFireRateMultiplier() : 1.0f;
}

float AMDSProjectPlayerState::GetMoveSpeedMultiplier() const
{
	return CombatAttributeSet ? CombatAttributeSet->GetMoveSpeedMultiplier() : 1.0f;
}

void AMDSProjectPlayerState::OnRep_Progression()
{
	UE_LOG(LogMDSPlayerProgression, Log,
		TEXT("MDS Progression | Replicated | PlayerState=%s | Currency=%d | Experience=%d | Level=%d | PendingChoices=%d."),
		*GetNameSafe(this),
		MatchCurrency,
		CurrentExperience,
		CurrentLevel,
		PendingLevelUpChoices);
}
