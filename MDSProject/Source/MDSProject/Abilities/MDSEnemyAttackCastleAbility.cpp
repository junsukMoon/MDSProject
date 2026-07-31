#include "Abilities/MDSEnemyAttackCastleAbility.h"

#include "Combat/MDSCombatEnemyActor.h"
#include "MDSProjectGameState.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSEnemyAttackCastleAbility, Log, All);

UMDSEnemyAttackCastleAbility::UMDSEnemyAttackCastleAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMDSEnemyAttackCastleAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AMDSCombatEnemyActor* Enemy = Cast<AMDSCombatEnemyActor>(GetAvatarActorFromActorInfo());
	if (!Enemy || !Enemy->HasAuthority())
	{
		UE_LOG(LogMDSEnemyAttackCastleAbility, Warning,
			TEXT("MDS GAS EnemyAttackCastle | Rejected | Reason=InvalidAuthorityContext | Enemy=%s."),
			*GetNameSafe(Enemy));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const AMDSProjectGameState* MDSGameState = GetWorld() ? GetWorld()->GetGameState<AMDSProjectGameState>() : nullptr;
	if (MDSGameState && MDSGameState->IsCombatSuspended())
	{
		UE_LOG(LogMDSEnemyAttackCastleAbility, Log,
			TEXT("MDS GAS EnemyAttackCastle | Rejected | Reason=CombatSuspended | Enemy=%s."),
			*GetNameSafe(Enemy));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const bool bDamageApplied = Enemy->ResolveObjectiveAttackAbility();
	UE_LOG(LogMDSEnemyAttackCastleAbility, Log,
		TEXT("MDS GAS EnemyAttackCastle | Resolved | Enemy=%s | DamageApplied=%s."),
		*GetNameSafe(Enemy),
		bDamageApplied ? TEXT("true") : TEXT("false"));

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bDamageApplied);
}
