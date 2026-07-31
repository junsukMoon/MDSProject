#include "MDSProjectGameState.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSGameState, Log, All);

AMDSProjectGameState::AMDSProjectGameState()
{
	bReplicates = true;
}

void AMDSProjectGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSProjectGameState, MatchPhase);
	DOREPLIFETIME(AMDSProjectGameState, CurrentRoundIndex);
	DOREPLIFETIME(AMDSProjectGameState, bCombatSuspended);
	DOREPLIFETIME(AMDSProjectGameState, LevelUpFlowState);
	DOREPLIFETIME(AMDSProjectGameState, LastRoundResult);
	DOREPLIFETIME(AMDSProjectGameState, ActiveShopOffers);
	DOREPLIFETIME(AMDSProjectGameState, CurrentWaveIndex);
	DOREPLIFETIME(AMDSProjectGameState, EnemiesRemaining);
	DOREPLIFETIME(AMDSProjectGameState, TotalEnemiesThisWave);
	DOREPLIFETIME(AMDSProjectGameState, bWaveActive);
}

void AMDSProjectGameState::SetActiveShopOffers(const TArray<FMDSShopOffer>& InShopOffers)
{
	if (!HasWaveStateAuthority(TEXT("SetActiveShopOffers")))
	{
		return;
	}
	ActiveShopOffers = InShopOffers;
	UE_LOG(LogMDSGameState, Log, TEXT("MDS Shop | OffersPublished | Count=%d."), ActiveShopOffers.Num());
	ForceNetUpdate();
}

void AMDSProjectGameState::SetRoundResult(const FMDSRoundResult& InRoundResult)
{
	if (!HasWaveStateAuthority(TEXT("SetRoundResult")))
	{
		return;
	}

	LastRoundResult = InRoundResult;
	UE_LOG(LogMDSGameState, Log,
		TEXT("MDS RoundResult | TeamFinalized | Round=%d | ClearTime=%.2f | Enemies=%d | CastleDamage=%.1f | CastleHP=%.1f | CastlePercent=%.3f."),
		LastRoundResult.RoundIndex,
		LastRoundResult.ClearTime,
		LastRoundResult.TotalEnemyCount,
		LastRoundResult.CastleDamageTaken,
		LastRoundResult.CastleHealthRemaining,
		LastRoundResult.CastleHealthPercent);
	ForceNetUpdate();
}

void AMDSProjectGameState::SetLevelUpFlowState(const EMDSLevelUpFlowState InFlowState)
{
	if (!HasWaveStateAuthority(TEXT("SetLevelUpFlowState")))
	{
		return;
	}

	LevelUpFlowState = InFlowState;
	const float TimeDilation =
		(InFlowState == EMDSLevelUpFlowState::TransitionIn || InFlowState == EMDSLevelUpFlowState::TransitionOut)
		? 0.25f
		: 1.0f;
	UGameplayStatics::SetGlobalTimeDilation(this, TimeDilation);
	UE_LOG(LogMDSGameState, Log, TEXT("MDS LevelUp | FlowState=%s | TimeDilation=%.2f."),
		*UEnum::GetValueAsString(LevelUpFlowState),
		TimeDilation);
	ForceNetUpdate();
}

void AMDSProjectGameState::SetCombatSuspended(const bool bInCombatSuspended)
{
	if (!HasWaveStateAuthority(TEXT("SetCombatSuspended")))
	{
		return;
	}

	const bool bWasCombatSuspended = bCombatSuspended;
	bCombatSuspended = bInCombatSuspended;
	UE_LOG(LogMDSGameState, Log,
		TEXT("Combat suspension set on server: Suspended=%s->%s Phase=%s Round=%d Wave=%d."),
		bWasCombatSuspended ? TEXT("true") : TEXT("false"),
		bCombatSuspended ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(MatchPhase),
		CurrentRoundIndex,
		CurrentWaveIndex);
	ForceNetUpdate();
}

void AMDSProjectGameState::SetMatchState(const EMDSMatchPhase InMatchPhase, const int32 InCurrentRoundIndex)
{
	if (!HasWaveStateAuthority(TEXT("SetMatchState")))
	{
		return;
	}

	const EMDSMatchPhase PreviousPhase = MatchPhase;
	const int32 PreviousRoundIndex = CurrentRoundIndex;
	MatchPhase = InMatchPhase;
	CurrentRoundIndex = FMath::Max(0, InCurrentRoundIndex);

	UE_LOG(LogMDSGameState, Log,
		TEXT("Match state set on server: Phase=%s->%s Round=%d->%d."),
		*UEnum::GetValueAsString(PreviousPhase),
		*UEnum::GetValueAsString(MatchPhase),
		PreviousRoundIndex,
		CurrentRoundIndex);
}

void AMDSProjectGameState::SetWaveState(const int32 InCurrentWaveIndex, const int32 InEnemiesRemaining, const bool bInWaveActive, const int32 InTotalEnemiesThisWave)
{
	if (!HasWaveStateAuthority(TEXT("SetWaveState")))
	{
		return;
	}

	CurrentWaveIndex = FMath::Max(0, InCurrentWaveIndex);
	EnemiesRemaining = FMath::Max(0, InEnemiesRemaining);
	TotalEnemiesThisWave = FMath::Max(0, InTotalEnemiesThisWave);
	bWaveActive = bInWaveActive;

	UE_LOG(LogMDSGameState, Log, TEXT("Wave state set on server: Wave=%d Remaining=%d Total=%d Active=%s."),
		CurrentWaveIndex,
		EnemiesRemaining,
		TotalEnemiesThisWave,
		bWaveActive ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetWaveState(CurrentWaveIndex, EnemiesRemaining, TotalEnemiesThisWave, bWaveActive);
		}
	}
}

void AMDSProjectGameState::SetEnemiesRemaining(const int32 InEnemiesRemaining)
{
	if (!HasWaveStateAuthority(TEXT("SetEnemiesRemaining")))
	{
		return;
	}

	const int32 PreviousEnemiesRemaining = EnemiesRemaining;
	EnemiesRemaining = FMath::Max(0, InEnemiesRemaining);

	UE_LOG(LogMDSGameState, Log, TEXT("Enemies remaining changed on server: %d -> %d."),
		PreviousEnemiesRemaining,
		EnemiesRemaining);

	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetWaveState(CurrentWaveIndex, EnemiesRemaining, TotalEnemiesThisWave, bWaveActive);
		}
	}
}

void AMDSProjectGameState::SetWaveActive(const bool bInWaveActive)
{
	if (!HasWaveStateAuthority(TEXT("SetWaveActive")))
	{
		return;
	}

	const bool bWasWaveActive = bWaveActive;
	bWaveActive = bInWaveActive;

	UE_LOG(LogMDSGameState, Log, TEXT("Wave active changed on server: %s -> %s."),
		bWasWaveActive ? TEXT("true") : TEXT("false"),
		bWaveActive ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetWaveState(CurrentWaveIndex, EnemiesRemaining, TotalEnemiesThisWave, bWaveActive);
		}
	}
}

void AMDSProjectGameState::OnRep_WaveState()
{
	UE_LOG(LogMDSGameState, Log, TEXT("Wave state replicated on client: Wave=%d Remaining=%d Total=%d Active=%s."),
		CurrentWaveIndex,
		EnemiesRemaining,
		TotalEnemiesThisWave,
		bWaveActive ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		if (UMDSDebugStateSubsystem* DebugState = World->GetSubsystem<UMDSDebugStateSubsystem>())
		{
			DebugState->SetWaveState(CurrentWaveIndex, EnemiesRemaining, TotalEnemiesThisWave, bWaveActive);
		}
	}
}

void AMDSProjectGameState::OnRep_MatchState()
{
	const float TimeDilation =
		(LevelUpFlowState == EMDSLevelUpFlowState::TransitionIn || LevelUpFlowState == EMDSLevelUpFlowState::TransitionOut)
		? 0.25f
		: 1.0f;
	UGameplayStatics::SetGlobalTimeDilation(this, TimeDilation);
	UE_LOG(LogMDSGameState, Log,
		TEXT("Match state replicated on client: Phase=%s Round=%d CombatSuspended=%s LevelUpFlow=%s ResultRound=%d."),
		*UEnum::GetValueAsString(MatchPhase),
		CurrentRoundIndex,
		bCombatSuspended ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(LevelUpFlowState),
		LastRoundResult.RoundIndex);
}

bool AMDSProjectGameState::HasWaveStateAuthority(const TCHAR* Context) const
{
	if (HasAuthority())
	{
		return true;
	}

	UE_LOG(LogMDSGameState, Warning, TEXT("Rejected non-authority wave state update: %s."), Context);
	return false;
}
