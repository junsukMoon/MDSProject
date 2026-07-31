// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDSProjectGameMode.h"

#include "ActorAI/MDSActorEnemySpawnSubsystem.h"
#include "Combat/MDSCombatEnemyActor.h"
#include "MDSProjectCharacter.h"
#include "MDSProjectPlayerController.h"
#include "MDSProjectPlayerState.h"
#include "MDSProjectGameState.h"
#include "Objective/MDSObjectiveActor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "EngineUtils.h"
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
	PlayerStateClass = AMDSProjectPlayerState::StaticClass();
}

void AMDSProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeWaveDisplayState();
	ConfigureWaveLoopFromCommandLine();
	if (FParse::Param(FCommandLine::Get(), TEXT("MDSAutoStartWave")))
	{
		bContinuousWaveLoopEnabled = false;
		UE_LOG(LogMDSGameMode, Log, TEXT("Continuous wave loop disabled because MDSAutoStartWave one-shot verification takes precedence."));
		TryAutoStartWaveFromCommandLine();
	}
	else if (bContinuousWaveLoopEnabled)
	{
		ScheduleWaveStart(1, WaveIntermissionSeconds);
	}
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

	if (MDSGameState->IsWaveActive())
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Rejected overlapping StartWave while Wave=%d is active."),
			MDSGameState->GetCurrentWaveIndex());
		return;
	}

	GetWorldTimerManager().ClearTimer(WaveStartTimerHandle);
	ScheduledWaveIndex = 0;

	const int32 ClampedWaveIndex = FMath::Max(1, WaveIndex);
	MDSGameState->SetMatchState(EMDSMatchPhase::Combat, ClampedWaveIndex);
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
	BeginRoundResultTracking(ClampedWaveIndex, SpawnedEnemyCount);

	UE_LOG(LogMDSGameMode, Log, TEXT("Round combat started on server: Round=%d Wave=%d RequestedEnemies=%d SpawnedEnemies=%d Active=%s."),
		MDSGameState->GetCurrentRoundIndex(),
		ClampedWaveIndex,
		RequestedEnemyCount,
		SpawnedEnemyCount,
		SpawnedEnemyCount > 0 ? TEXT("true") : TEXT("false"));

	if (SpawnedEnemyCount == 0 && bContinuousWaveLoopEnabled)
	{
		UE_LOG(LogMDSGameMode, Warning,
			TEXT("Wave spawn produced no enemies; retrying Wave=%d after %.2f seconds."),
			ClampedWaveIndex,
			WaveIntermissionSeconds);
		ScheduleWaveStart(ClampedWaveIndex, WaveIntermissionSeconds);
		return;
	}

	CompleteWaveIfCleared();
}

void AMDSProjectGameMode::HandleEnemyDeathForWave(AMDSProjectPlayerState* RewardRecipient)
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

	const int32 PreviousPendingLevelUpChoices = RewardRecipient
		? RewardRecipient->GetPendingLevelUpChoices()
		: 0;
	if (RewardRecipient)
	{
		RewardRecipient->GrantMatchReward(KillCurrencyReward, KillExperienceReward);
	}
	else
	{
		UE_LOG(LogMDSGameMode, Log,
			TEXT("Enemy death had no player reward recipient; Wave progress will still be consumed."));
	}

	const int32 PreviousEnemiesRemaining = MDSGameState->GetEnemiesRemaining();
	const int32 NewEnemiesRemaining = FMath::Max(0, PreviousEnemiesRemaining - 1);
	MDSGameState->SetEnemiesRemaining(NewEnemiesRemaining);

	UE_LOG(LogMDSGameMode, Log, TEXT("Wave enemy death consumed on server: Wave=%d Remaining=%d -> %d."),
		MDSGameState->GetCurrentWaveIndex(),
		PreviousEnemiesRemaining,
		NewEnemiesRemaining);

	AMDSProjectGameState* UpdatedGameState = GetGameState<AMDSProjectGameState>();
	const bool bNewLevelUpPending = RewardRecipient
		&& RewardRecipient->GetPendingLevelUpChoices() > PreviousPendingLevelUpChoices;
	if (bNewLevelUpPending
		&& UpdatedGameState
		&& UpdatedGameState->GetMatchPhase() == EMDSMatchPhase::Combat)
	{
		BeginLevelUpFlow();
	}
	CompleteWaveIfCleared();
}

void AMDSProjectGameMode::BeginLevelUpFlow()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!HasAuthority() || !MDSGameState || MDSGameState->GetLevelUpFlowState() != EMDSLevelUpFlowState::None)
	{
		return;
	}

	for (APlayerState* BasePlayerState : MDSGameState->PlayerArray)
	{
		if (AMDSProjectPlayerState* PlayerState = Cast<AMDSProjectPlayerState>(BasePlayerState);
			PlayerState && PlayerState->GetPendingLevelUpChoices() > 0)
		{
			PlayerState->PrepareLevelUpChoices();
		}
	}

	MDSGameState->SetLevelUpFlowState(EMDSLevelUpFlowState::TransitionIn);
	LevelUpPauseStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	bLevelUpPauseTimingActive = true;
	GetWorldTimerManager().SetTimer(
		LevelUpTransitionTimerHandle,
		this,
		&AMDSProjectGameMode::EnterLevelUpSelection,
		0.0875f,
		false);
}

void AMDSProjectGameMode::EnterLevelUpSelection()
{
	if (AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>())
	{
		MDSGameState->SetLevelUpFlowState(EMDSLevelUpFlowState::Selection);
		SetCombatSuspended(true);
	}
}

void AMDSProjectGameMode::HandleLevelUpChoice(AMDSProjectPlayerState* PlayerState, const EMDSLevelUpUpgrade Upgrade)
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!HasAuthority() || !MDSGameState || MDSGameState->GetLevelUpFlowState() != EMDSLevelUpFlowState::Selection
		|| !MDSGameState->IsCombatSuspended() || !PlayerState || !PlayerState->TryApplyLevelUpChoice(Upgrade))
	{
		return;
	}

	if (DoAllPlayersHaveNoPendingLevelUpChoices())
	{
		BeginLevelUpResume();
	}
}

void AMDSProjectGameMode::HandleShopPurchase(AMDSProjectPlayerState* PlayerState, const FName ProductId)
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!HasAuthority() || !MDSGameState || MDSGameState->GetMatchPhase() != EMDSMatchPhase::RoundSettlement
		|| !PlayerState || ProductId.IsNone())
	{
		return;
	}

	const FMDSShopOffer* Offer = MDSGameState->GetActiveShopOffers().FindByPredicate(
		[ProductId](const FMDSShopOffer& Candidate) { return Candidate.ProductId == ProductId; });
	if (!Offer)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("MDS Shop | PurchaseRejected | Reason=UnknownProduct | Product=%s."),
			*ProductId.ToString());
		return;
	}
	PlayerState->TryPurchaseShopOffer(*Offer);
}

bool AMDSProjectGameMode::DoAllPlayersHaveNoPendingLevelUpChoices() const
{
	const AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		return false;
	}
	for (const APlayerState* BasePlayerState : MDSGameState->PlayerArray)
	{
		const AMDSProjectPlayerState* PlayerState = Cast<AMDSProjectPlayerState>(BasePlayerState);
		if (PlayerState && PlayerState->GetPendingLevelUpChoices() > 0)
		{
			return false;
		}
	}
	return true;
}

void AMDSProjectGameMode::BeginLevelUpResume()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		return;
	}
	SetCombatSuspended(false);
	MDSGameState->SetLevelUpFlowState(EMDSLevelUpFlowState::TransitionOut);
	GetWorldTimerManager().SetTimer(
		LevelUpTransitionTimerHandle,
		this,
		&AMDSProjectGameMode::FinishLevelUpResume,
		0.0875f,
		false);
}

void AMDSProjectGameMode::FinishLevelUpResume()
{
	if (bLevelUpPauseTimingActive)
	{
		const double CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : LevelUpPauseStartTimeSeconds;
		AccumulatedLevelUpPauseSeconds += FMath::Max(0.0, CurrentTimeSeconds - LevelUpPauseStartTimeSeconds);
		bLevelUpPauseTimingActive = false;
	}
	if (AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>())
	{
		MDSGameState->SetLevelUpFlowState(EMDSLevelUpFlowState::None);
	}
	CompleteWaveIfCleared();
}

void AMDSProjectGameMode::SetCombatSuspended(const bool bInCombatSuspended)
{
	if (!HasAuthority())
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Rejected non-authority combat suspension request."));
		return;
	}

	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState
		|| (bInCombatSuspended && MDSGameState->GetMatchPhase() != EMDSMatchPhase::Combat)
		|| MDSGameState->IsCombatSuspended() == bInCombatSuspended)
	{
		return;
	}

	MDSGameState->SetCombatSuspended(bInCombatSuspended);
	if (bInCombatSuspended)
	{
		GetWorldTimerManager().PauseTimer(WaveStartTimerHandle);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(WaveStartTimerHandle);
	}

	int32 SuspendedPlayerCount = 0;
	for (TActorIterator<AMDSProjectCharacter> CharacterIt(GetWorld()); CharacterIt; ++CharacterIt)
	{
		CharacterIt->SetCombatSuspended(bInCombatSuspended);
		++SuspendedPlayerCount;
	}

	int32 SuspendedEnemyCount = 0;
	for (TActorIterator<AMDSCombatEnemyActor> EnemyIt(GetWorld()); EnemyIt; ++EnemyIt)
	{
		EnemyIt->SetCombatSuspended(bInCombatSuspended);
		++SuspendedEnemyCount;
	}

	UE_LOG(LogMDSGameMode, Log,
		TEXT("MDS CombatSuspension | Applied | Suspended=%s | Players=%d | Enemies=%d | WorldPaused=false."),
		bInCombatSuspended ? TEXT("true") : TEXT("false"),
		SuspendedPlayerCount,
		SuspendedEnemyCount);

	if (bInCombatSuspended)
	{
		float AutoResumeDelaySeconds = 0.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("MDSAutoCombatResumeDelay="), AutoResumeDelaySeconds))
		{
			GetWorldTimerManager().SetTimer(
				CombatResumeVerificationTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					SetCombatSuspended(false);
				}),
				FMath::Max(0.01f, AutoResumeDelaySeconds),
				false);
			UE_LOG(LogMDSGameMode, Log,
				TEXT("MDS CombatSuspension | VerificationResumeScheduled | Delay=%.2f."),
				AutoResumeDelaySeconds);
		}
	}
}

void AMDSProjectGameMode::InitializeWaveDisplayState()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		UE_LOG(LogMDSGameMode, Warning, TEXT("Unable to initialize wave display state because AMDSProjectGameState is unavailable."));
		return;
	}

	MDSGameState->SetMatchState(EMDSMatchPhase::Waiting, 0);
	MDSGameState->SetWaveState(0, 0, false, 0);
	UE_LOG(LogMDSGameMode, Log,
		TEXT("Initialized match state on server: Phase=Waiting Round=0 Wave=0 Remaining=0 Total=0 Active=false."));
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

void AMDSProjectGameMode::ConfigureWaveLoopFromCommandLine()
{
	bContinuousWaveLoopEnabled = !FParse::Param(FCommandLine::Get(), TEXT("NoMDSWaveLoop"));
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveMaxCount="), MaxWaveCount);
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveInitialEnemyCount="), InitialWaveEnemyCount);
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveEnemyIncrement="), EnemyIncrementPerWave);
	FParse::Value(FCommandLine::Get(), TEXT("MDSWaveIntermission="), WaveIntermissionSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("MDSKillCurrency="), KillCurrencyReward);
	FParse::Value(FCommandLine::Get(), TEXT("MDSKillExperience="), KillExperienceReward);

	MaxWaveCount = FMath::Max(1, MaxWaveCount);
	InitialWaveEnemyCount = FMath::Max(1, InitialWaveEnemyCount);
	EnemyIncrementPerWave = FMath::Max(0, EnemyIncrementPerWave);
	KillCurrencyReward = FMath::Max(0, KillCurrencyReward);
	KillExperienceReward = FMath::Max(0, KillExperienceReward);
	WaveIntermissionSeconds = FMath::Max(0.0f, WaveIntermissionSeconds);

	UE_LOG(LogMDSGameMode, Log,
		TEXT("Wave loop configured on server: Enabled=%s MaxWaves=%d InitialEnemies=%d EnemyIncrement=%d Intermission=%.2f KillCurrency=%d KillExperience=%d."),
		bContinuousWaveLoopEnabled ? TEXT("true") : TEXT("false"),
		MaxWaveCount,
		InitialWaveEnemyCount,
		EnemyIncrementPerWave,
		WaveIntermissionSeconds,
		KillCurrencyReward,
		KillExperienceReward);
}

void AMDSProjectGameMode::ScheduleWaveStart(const int32 WaveIndex, const float DelaySeconds)
{
	if (!HasAuthority() || !bContinuousWaveLoopEnabled || WaveIndex < 1 || WaveIndex > MaxWaveCount)
	{
		return;
	}

	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState || MDSGameState->IsWaveActive() || GetWorldTimerManager().IsTimerActive(WaveStartTimerHandle))
	{
		return;
	}

	ScheduledWaveIndex = WaveIndex;
	GetWorldTimerManager().SetTimer(
		WaveStartTimerHandle,
		this,
		&AMDSProjectGameMode::StartScheduledWave,
		FMath::Max(0.01f, DelaySeconds),
		false);

	UE_LOG(LogMDSGameMode, Log, TEXT("Next wave scheduled on server: Wave=%d EnemyCount=%d Delay=%.2f."),
		ScheduledWaveIndex,
		GetEnemyCountForWave(ScheduledWaveIndex),
		DelaySeconds);
}

void AMDSProjectGameMode::StartScheduledWave()
{
	const int32 WaveIndex = ScheduledWaveIndex;
	ScheduledWaveIndex = 0;
	if (!bContinuousWaveLoopEnabled || WaveIndex < 1 || WaveIndex > MaxWaveCount)
	{
		return;
	}

	StartWave(WaveIndex, GetEnemyCountForWave(WaveIndex));
}

int32 AMDSProjectGameMode::GetEnemyCountForWave(const int32 WaveIndex) const
{
	return InitialWaveEnemyCount + FMath::Max(0, WaveIndex - 1) * EnemyIncrementPerWave;
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

	if (MDSGameState->GetLevelUpFlowState() != EMDSLevelUpFlowState::None)
	{
		return;
	}

	if (MDSGameState->GetEnemiesRemaining() > 0)
	{
		return;
	}

	MDSGameState->SetWaveActive(false);
	FinalizeRoundResults();
	PublishRoundShopOffers();
	MDSGameState->SetMatchState(EMDSMatchPhase::RoundSettlement, MDSGameState->GetCurrentRoundIndex());
	UE_LOG(LogMDSGameMode, Log, TEXT("Round entered settlement on server: Round=%d Wave=%d."),
		MDSGameState->GetCurrentRoundIndex(),
		MDSGameState->GetCurrentWaveIndex());

	const int32 ClearedWaveIndex = MDSGameState->GetCurrentWaveIndex();
	if (!bContinuousWaveLoopEnabled)
	{
		return;
	}

	if (ClearedWaveIndex >= MaxWaveCount)
	{
		UE_LOG(LogMDSGameMode, Log, TEXT("Demo wave loop completed on server: FinalWave=%d."), ClearedWaveIndex);
		return;
	}

	ScheduleWaveStart(ClearedWaveIndex + 1, WaveIntermissionSeconds);
}

void AMDSProjectGameMode::PublishRoundShopOffers()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		return;
	}

	TArray<FMDSShopOffer> Offers;
	auto AddOffer = [&Offers](const FName ProductId, const TCHAR* Name, const TCHAR* Description,
		const int32 Price, const EMDSLevelUpUpgrade Upgrade)
	{
		FMDSShopOffer& Offer = Offers.AddDefaulted_GetRef();
		Offer.ProductId = ProductId;
		Offer.DisplayName = FText::FromString(Name);
		Offer.EffectDescription = FText::FromString(Description);
		Offer.Price = Price;
		Offer.Upgrade = Upgrade;
	};
	AddOffer(TEXT("Shop.AttackPower"), TEXT("공격력 강화"), TEXT("공격력 +15%"), 20, EMDSLevelUpUpgrade::AttackPower);
	AddOffer(TEXT("Shop.FireRate"), TEXT("발사속도 강화"), TEXT("발사속도 +15%"), 20, EMDSLevelUpUpgrade::FireRate);
	AddOffer(TEXT("Shop.MoveSpeed"), TEXT("이동속도 강화"), TEXT("이동속도 +10%"), 15, EMDSLevelUpUpgrade::MoveSpeed);
	MDSGameState->SetActiveShopOffers(Offers);
}

void AMDSProjectGameMode::BeginRoundResultTracking(const int32 RoundIndex, const int32 TotalEnemyCount)
{
	RoundStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	AccumulatedLevelUpPauseSeconds = 0.0;
	bLevelUpPauseTimingActive = false;
	RoundTrackedEnemyCount = FMath::Max(0, TotalEnemyCount);
	RoundStartCastleHealth = 0.0f;

	for (TActorIterator<AMDSObjectiveActor> ObjectiveIt(GetWorld()); ObjectiveIt; ++ObjectiveIt)
	{
		RoundStartCastleHealth = ObjectiveIt->GetCurrentHealth();
		break;
	}

	if (const AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>())
	{
		for (APlayerState* BasePlayerState : MDSGameState->PlayerArray)
		{
			if (AMDSProjectPlayerState* PlayerState = Cast<AMDSProjectPlayerState>(BasePlayerState))
			{
				PlayerState->BeginRoundTracking();
			}
		}
	}

	UE_LOG(LogMDSGameMode, Log,
		TEXT("MDS RoundResult | TrackingStarted | Round=%d | Enemies=%d | CastleStartHP=%.1f."),
		RoundIndex,
		RoundTrackedEnemyCount,
		RoundStartCastleHealth);
}

void AMDSProjectGameMode::FinalizeRoundResults()
{
	AMDSProjectGameState* MDSGameState = GetGameState<AMDSProjectGameState>();
	if (!MDSGameState)
	{
		return;
	}

	float CastleHealthRemaining = 0.0f;
	float CastleMaxHealth = 0.0f;
	for (TActorIterator<AMDSObjectiveActor> ObjectiveIt(GetWorld()); ObjectiveIt; ++ObjectiveIt)
	{
		CastleHealthRemaining = ObjectiveIt->GetCurrentHealth();
		CastleMaxHealth = ObjectiveIt->GetMaxHealth();
		break;
	}

	FMDSRoundResult RoundResult;
	RoundResult.RoundIndex = MDSGameState->GetCurrentRoundIndex();
	const double CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : RoundStartTimeSeconds;
	const double ActiveLevelUpPauseSeconds = bLevelUpPauseTimingActive
		? FMath::Max(0.0, CurrentTimeSeconds - LevelUpPauseStartTimeSeconds)
		: 0.0;
	RoundResult.ClearTime = FMath::Max(0.0f, static_cast<float>(
		CurrentTimeSeconds - RoundStartTimeSeconds - AccumulatedLevelUpPauseSeconds - ActiveLevelUpPauseSeconds));
	RoundResult.TotalEnemyCount = RoundTrackedEnemyCount;
	RoundResult.CastleDamageTaken = FMath::Max(0.0f, RoundStartCastleHealth - CastleHealthRemaining);
	RoundResult.CastleHealthRemaining = CastleHealthRemaining;
	RoundResult.CastleHealthPercent = CastleMaxHealth > 0.0f
		? FMath::Clamp(CastleHealthRemaining / CastleMaxHealth, 0.0f, 1.0f)
		: 0.0f;
	MDSGameState->SetRoundResult(RoundResult);

	for (APlayerState* BasePlayerState : MDSGameState->PlayerArray)
	{
		if (AMDSProjectPlayerState* PlayerState = Cast<AMDSProjectPlayerState>(BasePlayerState))
		{
			PlayerState->FinalizeRoundResult();
		}
	}
}
