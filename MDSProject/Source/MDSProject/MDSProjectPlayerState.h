#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MDSProjectPlayerState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class MDSPROJECT_API AMDSProjectPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDSProjectPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "MDS|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
