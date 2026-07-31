#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MDSProjectPlayerState.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;

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

	int32 GetMatchCurrency() const { return MatchCurrency; }
	int32 GetCurrentExperience() const { return CurrentExperience; }
	int32 GetCurrentLevel() const { return CurrentLevel; }
	int32 GetPendingLevelUpChoices() const { return PendingLevelUpChoices; }
	int32 GetExperienceRequiredForNextLevel() const;

private:
	UFUNCTION()
	void OnRep_Progression();

	UPROPERTY(EditDefaultsOnly, Category = "MDS|Abilities")
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	UPROPERTY(VisibleAnywhere, Category = "MDS|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 MatchCurrency = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 CurrentExperience = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 CurrentLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleInstanceOnly, Category = "MDS|Progression")
	int32 PendingLevelUpChoices = 0;

	FGameplayAbilitySpecHandle FireAbilityHandle;
	FVector PendingFireAimPoint = FVector::ZeroVector;
	bool bHasPendingFireAimPoint = false;
};
