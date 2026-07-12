// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectGameMode.h"

#include "ActorAI/MDSActorEnemySpawnSubsystem.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectGameState.h"
#include "GameFramework/Pawn.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSGameMode, Log, All);

AMDSProjectGameMode::AMDSProjectGameMode()
{
	GameStateClass = AMDSProjectGameState::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AMDSProjectPlayerController::StaticClass();
}

void AMDSProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeWaveDisplayState();
	TryAutoStartWaveFromCommandLine();
}

void AMDSProjectGameMode::StartWave(const int32 WaveIndex, const int32 TotalEnemies)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Rejected StartWave on non-authority."));
		return;
	}

	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to start wave because AMDSProjectGameState is unavailable."));
		return;
	}

	const int32 ClampedWaveIndex = FMath::Max(1, WaveIndex);
	const int32 RequestedEnemyCount = FMath::Max(0, TotalEnemies);
	int32 SpawnedEnemyCount = 0;

	if (UWorld* World = GetWorld())
	{
		if (UMDSActorEnemySpawnSubsystem* EnemySpawnSubsystem = World->GetSubsystem<UMDSActorEnemySpawnSubsystem>())
		{
			SpawnedEnemyCount = EnemySpawnSubsystem->SpawnCombatEnemiesForWave(RequestedEnemyCount);
		}
		else
		{
			UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to start wave spawn because UMDSActorEnemySpawnSubsystem is unavailable."));
		}
	}

	MDSGameState->SetWaveState(ClampedWaveIndex, SpawnedEnemyCount, SpawnedEnemyCount > 0, SpawnedEnemyCount);

	UE_LOG(LogMDSGameMode, Log, TEXT("Wave started on server: Wave=%d RequestedEnemies=%d SpawnedEnemies=%d Active=%s."),
		ClampedWaveIndex,
		RequestedEnemyCount,
		SpawnedEnemyCount,
		SpawnedEnemyCount > 0 ? TEXT("true") : TEXT("false"));

	CompleteWaveIfCleared();
}

void AMDSProjectGameMode::HandleEnemyDeathForWave()
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Rejected HandleEnemyDeathForWave on non-authority."));
		return;
	}

	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to consume enemy death because AMDSProjectGameState is unavailable."));
		return;
	}

	if (!MDSGameState->IsWaveActive())
	{
		UE_LOG(LogMDSGameMode, Log, TEXT("Ignored enemy death for Wave because no wave is active."));
		return;
	}

	const int32 PreviousEnemiesRemaining = MDSGameState->GetEnemiesRemaining();
	const int32 NewEnemiesRemaining = FMath::Max(0, PreviousEnemiesRemaining - 1);
	MDSGameState->SetEnemiesRemaining(NewEnemiesRemaining);

	UE_LOG(LogMDSGameMode, Log, TEXT("Wave enemy death consumed on server: Wave=%d Remaining=%d -> %d."),
		MDSGameState->GetCurrentWaveIndex(),
		PreviousEnemiesRemaining,
		NewEnemiesRemaining);

	CompleteWaveIfCleared();
}

void AMDSProjectGameMode::InitializeWaveDisplayState()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to initialize wave display state because AMDSProjectGameState is unavailable."));
		return;
	}

	MDSGameState->SetWaveState(1, 0, false, 0);
	UE_LOG(LogMDSGameMode, Log, TEXT("Initialized wave display state on server: Wave=1 Remaining=0 Total=0 Active=false."));
}

void AMDSProjectGameMode::TryAutoStartWaveFromCommandLine()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!FParse::Param(FCommandLine::Get(), TEXT("MDSAutoStartWave")))
	{
		return;
	}

	int32 WaveIndex = 1;
	int32 EnemyCount = 4;
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveIndex="), WaveIndex);
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveEnemyCount="), EnemyCount);

	WaveIndex = FMath::Max(1, WaveIndex);
	EnemyCount = FMath::Max(0, EnemyCount);

	UE_LOG(LogMDSGameMode, Log, TEXT("Auto-starting wave on server from command line: Wave=%d EnemyCount=%d."),
		WaveIndex,
		EnemyCount);

	FTimerDelegate AutoStartWaveDelegate = FTimerDelegate::CreateWeakLambda(this, [this, WaveIndex, EnemyCount]()
	{
		StartWave(WaveIndex, EnemyCount);
	});

	GetWorldTimerManager().SetTimerForNextTick(AutoStartWaveDelegate);
}

void AMDSProjectGameMode::CompleteWaveIfCleared()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to evaluate wave clear because AMDSProjectGameState is unavailable."));
		return;
	}

	if (!MDSGameState->IsWaveActive())
	{
		return;
	}

	if (MDSGameState->GetEnemiesRemaining() > 0)
	{
		return;
	}

	MDSGameState->SetWaveActive(false);
	UE_LOG(LogMDSGameMode, Log, TEXT("Wave cleared on server: Wave=%d."), MDSGameState->GetCurrentWaveIndex());
}
