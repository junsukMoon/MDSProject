#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Progression/MDSLevelUpTypes.h"
#include "Progression/MDSRoundResultTypes.h"
#include "Shop/MDSShopTypes.h"
#include "MDSProjectPlayerState.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UMDSCombatAttributeSet;

UCLASS()
class MDSPROJECT_API AMDSProjectPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDSProjectPlayerState();

	virtual void BeginPlay() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool TryActivateFireAbility(const FVector& RequestedAimPoint);
	bool ConsumePendingFireAimPoint(FVector& OutAimPoint);
	void GrantMatchReward(int32 CurrencyAmount, int32 ExperienceAmount);
	void PrepareLevelUpChoices();
	bool TryApplyLevelUpChoice(EMDSLevelUpUpgrade Upgrade);
	bool TryPurchaseShopOffer(const FMDSShopOffer& Offer);
	void BeginRoundTracking();
	void FinalizeRoundResult();
	void SetReadyForNextRound(bool bInReady);

	int32 GetMatchCurrency() const { return MatchCurrency; }
	int32 GetCurrentExperience() const { return CurrentExperience; }
	int32 GetCurrentLevel() const { return CurrentLevel; }
	int32 GetPendingLevelUpChoices() const { return PendingLevelUpChoices; }
	const TArray<EMDSLevelUpUpgrade>& GetActiveLevelUpChoices() const { return ActiveLevelUpChoices; }
	int32 GetExperienceRequiredForNextLevel() const;
	float GetAttackPowerMultiplier() const;
	float GetFireRateMultiplier() const;
	float GetMoveSpeedMultiplier() const;
	const FMDSPlayerRoundResult& GetLastRoundResult() const { return LastRoundResult; }
	const TArray<FName>& GetPurchasedShopProductIds() const { return PurchasedShopProductIds; }
	bool IsReadyForNextRound() const { return bReadyForNextRound; }

private:
	UFUNCTION()
	void OnRep_Progression();

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Abilities")
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	UPROPERTY(VisibleAnywhere, Category = "MDS|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "MDS|Abilities")
	TObjectPtr<UMDSCombatAttributeSet> CombatAttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 MatchCurrency = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 CurrentExperience = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 CurrentLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 PendingLevelUpChoices = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	TArray<EMDSLevelUpUpgrade> ActiveLevelUpChoices;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	FMDSPlayerRoundResult LastRoundResult;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Shop")
	TArray<FName> PurchasedShopProductIds;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Round")
	bool bReadyForNextRound = false;

	FGameplayAbilitySpecHandle FireAbilityHandle;
	FMDSPlayerRoundResult CurrentRoundResult;
	FVector PendingFireAimPoint = FVector::ZeroVector;
	float CachedBaseWalkSpeed = 0.0f;
	bool bHasPendingFireAimPoint = false;
	bool ApplyUpgradeEffect(EMDSLevelUpUpgrade Upgrade);
};
