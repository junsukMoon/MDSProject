#include "MDSProjectGameState.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSGameState, Log, All);

AMDSProjectGameState::AMDSProjectGameState()
{
	bReplicates = true;
}

void AMDSProjectGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMDSProjectGameState, CurrentWaveIndex);
	DOREPLIFETIME(AMDSProjectGameState, EnemiesRemaining);
	DOREPLIFETIME(AMDSProjectGameState, TotalEnemiesThisWave);
	DOREPLIFETIME(AMDSProjectGameState, bWaveActive);
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

bool AMDSProjectGameState::HasWaveStateAuthority(const TCHAR* Context) const
{
	if (HasAuthority())
	{
		return true;
	}

	UE_LOG(LogMDSGameState, Warning, TEXT("Rejected non-authority wave state update: %s."), Context);
	return false;
}
