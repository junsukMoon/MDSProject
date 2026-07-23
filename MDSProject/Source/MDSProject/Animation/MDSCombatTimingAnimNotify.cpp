#include "Animation/MDSCombatTimingAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSCombatTimingAnimNotify, Log, All);

void UMDSCombatTimingAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!FParse::Param(FCommandLine::Get(), TEXT("MDSCombatAnimNotifyVerify")))
	{
		return;
	}

	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	const ENetMode NetMode = Owner ? Owner->GetNetMode() : NM_Standalone;
	const TCHAR* NetModeName = NetMode == NM_Client
		? TEXT("Client")
		: NetMode == NM_DedicatedServer
			? TEXT("DedicatedServer")
			: NetMode == NM_ListenServer
				? TEXT("ListenServer")
				: TEXT("Standalone");

	UE_LOG(LogMDSCombatTimingAnimNotify, Log,
		TEXT("MDS CombatAnimNotify | Fired | Owner=%s | Mesh=%s | Animation=%s | NetMode=%s | GameplayDamage=false | ServerRequestSent=false."),
		*GetNameSafe(Owner),
		*GetNameSafe(MeshComp),
		*GetNameSafe(Animation),
		NetModeName);
}

FString UMDSCombatTimingAnimNotify::GetNotifyName_Implementation() const
{
	return TEXT("MDS Combat Timing Marker");
}
