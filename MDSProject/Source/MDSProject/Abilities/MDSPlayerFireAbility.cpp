#include "Abilities/MDSPlayerFireAbility.h"

#include "Combat/MDSCombatEnemyActor.h"
#include "EngineUtils.h"
#include "MDSProjectCharacter.h"
#include "MDSProjectPlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSPlayerFireAbility, Log, All);

namespace
{
constexpr float DefaultAttackDamage = 25.0f;
constexpr float DefaultAttackRange = 5000.0f;
constexpr float DefaultAttackCooldownSeconds = 0.5f;
constexpr float PlayerFireAbilityHitRadius = 100.0f;
constexpr float AttackFacingDurationSeconds = 0.2f;
}

UMDSPlayerFireAbility::UMDSPlayerFireAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMDSPlayerFireAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AMDSProjectPlayerState* MDSPlayerState = Cast<AMDSProjectPlayerState>(GetOwningActorFromActorInfo());
	AMDSProjectCharacter* RequestingCharacter = Cast<AMDSProjectCharacter>(GetAvatarActorFromActorInfo());
	if (!MDSPlayerState || !RequestingCharacter || !RequestingCharacter->HasAuthority())
	{
		UE_LOG(LogMDSPlayerFireAbility, Warning,
			TEXT("MDS GAS Fire | Rejected | Reason=InvalidAuthorityContext | Owner=%s | Avatar=%s."),
			*GetNameSafe(MDSPlayerState),
			*GetNameSafe(RequestingCharacter));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector RequestedAimPoint = FVector::ZeroVector;
	if (!MDSPlayerState->ConsumePendingFireAimPoint(RequestedAimPoint))
	{
		UE_LOG(LogMDSPlayerFireAbility, Warning,
			TEXT("MDS GAS Fire | Rejected | Reason=MissingAimPoint | Requester=%s."),
			*GetNameSafe(RequestingCharacter));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float AttackDamage = DefaultAttackDamage;
	float AttackRange = DefaultAttackRange;
	float AttackCooldownSeconds = DefaultAttackCooldownSeconds;
	FParse::Value(FCommandLine::Get(), TEXT("MDSAttackDamage="), AttackDamage);
	FParse::Value(FCommandLine::Get(), TEXT("MDSAttackRange="), AttackRange);
	FParse::Value(FCommandLine::Get(), TEXT("MDSAttackCooldown="), AttackCooldownSeconds);
	AttackDamage = FMath::Max(0.0f, AttackDamage);
	AttackRange = FMath::Max(1.0f, AttackRange);
	AttackCooldownSeconds = FMath::Max(0.0f, AttackCooldownSeconds);

	const FVector TraceStart = RequestingCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
	FVector ShotDirection = RequestedAimPoint - TraceStart;
	ShotDirection.Z = 0.0f;
	if (!ShotDirection.Normalize())
	{
		UE_LOG(LogMDSPlayerFireAbility, Warning,
			TEXT("MDS GAS Fire | Rejected | Reason=InvalidDirection | Requester=%s."),
			*GetNameSafe(RequestingCharacter));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttackDamage <= 0.0f)
	{
		UE_LOG(LogMDSPlayerFireAbility, Warning,
			TEXT("MDS GAS Fire | Rejected | Reason=InvalidDamage | Requester=%s | Damage=%.1f."),
			*GetNameSafe(RequestingCharacter),
			AttackDamage);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	const double CooldownRemaining = LastServerActivationTimeSeconds + AttackCooldownSeconds - CurrentTimeSeconds;
	if (CooldownRemaining > 0.0)
	{
		UE_LOG(LogMDSPlayerFireAbility, Log,
			TEXT("MDS GAS Fire | Rejected | Reason=Cooldown | Requester=%s | CooldownRemaining=%.2f."),
			*GetNameSafe(RequestingCharacter),
			CooldownRemaining);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AMDSCombatEnemyActor* TargetEnemy = nullptr;
	float ClosestDistanceAlongShot = AttackRange + 1.0f;
	if (World)
	{
		for (TActorIterator<AMDSCombatEnemyActor> EnemyIt(World); EnemyIt; ++EnemyIt)
		{
			AMDSCombatEnemyActor* CandidateEnemy = *EnemyIt;
			if (!CandidateEnemy || CandidateEnemy->IsDead())
			{
				continue;
			}

			FVector ToEnemy = CandidateEnemy->GetActorLocation() - TraceStart;
			ToEnemy.Z = 0.0f;
			const float DistanceAlongShot = FVector::DotProduct(ToEnemy, ShotDirection);
			if (DistanceAlongShot < 0.0f
				|| DistanceAlongShot > AttackRange
				|| DistanceAlongShot >= ClosestDistanceAlongShot)
			{
				continue;
			}

			const float DistanceFromShot = FVector::Dist2D(ToEnemy, ShotDirection * DistanceAlongShot);
			if (DistanceFromShot <= PlayerFireAbilityHitRadius)
			{
				TargetEnemy = CandidateEnemy;
				ClosestDistanceAlongShot = DistanceAlongShot;
			}
		}
	}

	LastServerActivationTimeSeconds = CurrentTimeSeconds;
	const FVector TraceEnd = TargetEnemy ? TargetEnemy->GetActorLocation() : RequestedAimPoint;
	RequestingCharacter->MulticastPlayRemoteAttackPresentation(
		TEXT("GASPlayerFire"),
		ShotDirection,
		TraceEnd,
		AttackFacingDurationSeconds);

	if (!TargetEnemy || TargetEnemy->IsDead())
	{
		UE_LOG(LogMDSPlayerFireAbility, Log,
			TEXT("MDS GAS Fire | Resolved | Requester=%s | Target=%s | Hit=false | DamageApplied=false | Range=%.1f."),
			*GetNameSafe(RequestingCharacter),
			*GetNameSafe(TargetEnemy),
			AttackRange);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const float PreviousHealth = TargetEnemy->GetCurrentHealth();
	const bool bDamageApplied = TargetEnemy->ApplyEnemyDamage(AttackDamage, TEXT("GA_Player_Fire"));
	const float NewHealth = TargetEnemy->GetCurrentHealth();

	UE_LOG(LogMDSPlayerFireAbility, Log,
		TEXT("MDS GAS Fire | Resolved | Requester=%s | Target=%s | Hit=true | DamageApplied=%s | Damage=%.1f | EnemyHP=%.1f->%.1f."),
		*GetNameSafe(RequestingCharacter),
		*GetNameSafe(TargetEnemy),
		bDamageApplied ? TEXT("true") : TEXT("false"),
		AttackDamage,
		PreviousHealth,
		NewHealth);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
