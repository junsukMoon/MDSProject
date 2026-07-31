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

	bool TryActivateFireAbility(const FVector& RequestedAimPoint);
	bool ConsumePendingFireAimPoint(FVector& OutAimPoint);

private:
	UPROPERTY(EditDefaultsOnly, Category = "MDS|Abilities")
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	UPROPERTY(VisibleAnywhere, Category = "MDS|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FGameplayAbilitySpecHandle FireAbilityHandle;
	FVector PendingFireAimPoint = FVector::ZeroVector;
	bool bHasPendingFireAimPoint = false;
};
