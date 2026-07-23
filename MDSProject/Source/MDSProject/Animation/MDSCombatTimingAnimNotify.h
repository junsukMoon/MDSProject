#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MDSCombatTimingAnimNotify.generated.h"

/** Presentation-only timing marker authored into the attack montage; never applies gameplay damage. */
UCLASS(meta = (DisplayName = "MDS Combat Timing Marker"))
class UMDSCombatTimingAnimNotify final : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
