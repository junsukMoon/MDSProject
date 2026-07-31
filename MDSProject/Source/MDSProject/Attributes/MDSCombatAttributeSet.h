#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "MDSCombatAttributeSet.generated.h"

#define MDS_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MDSPROJECT_API UMDSCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMDSCombatAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPowerMultiplier, Category = "MDS|Combat")
	FGameplayAttributeData AttackPowerMultiplier;
	MDS_ATTRIBUTE_ACCESSORS(UMDSCombatAttributeSet, AttackPowerMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireRateMultiplier, Category = "MDS|Combat")
	FGameplayAttributeData FireRateMultiplier;
	MDS_ATTRIBUTE_ACCESSORS(UMDSCombatAttributeSet, FireRateMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeedMultiplier, Category = "MDS|Combat")
	FGameplayAttributeData MoveSpeedMultiplier;
	MDS_ATTRIBUTE_ACCESSORS(UMDSCombatAttributeSet, MoveSpeedMultiplier)

private:
	UFUNCTION()
	void OnRep_AttackPowerMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_FireRateMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldValue);
};

#undef MDS_ATTRIBUTE_ACCESSORS
