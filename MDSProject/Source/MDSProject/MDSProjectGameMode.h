// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MDSProjectGameMode.generated.h"

class AMDSProjectPlayerState;
enum class EMDSLevelUpUpgrade : uint8;

/**
 *  Simple Game Mode for a top-down perspective game
 *  Sets the default gameplay framework classes
 *  Check the Blueprint derived class for the set values
 */
UCLASS()
class AMDSProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AMDSProjectGameMode();

	void StartWave(int32 WaveIndex, int32 TotalEnemies);
	void HandleEnemyDeathForWave(AMDSProjectPlayerState* RewardRecipient);
	void SetCombatSuspended(bool bInCombatSuspended);
	void HandleLevelUpChoice(AMDSProjectPlayerState* PlayerState, EMDSLevelUpUpgrade Upgrade);

protected:
	virtual void BeginPlay() override;

private:
	void InitializeWaveDisplayState();
	void ConfigureWaveLoopFromCommandLine();
	void TryAutoStartWaveFromCommandLine();
	void ScheduleWaveStart(int32 WaveIndex, float DelaySeconds);
	void StartScheduledWave();
	void CompleteWaveIfCleared();
	void BeginRoundResultTracking(int32 RoundIndex, int32 TotalEnemyCount);
	void FinalizeRoundResults();
	void BeginLevelUpFlow();
	void EnterLevelUpSelection();
	void BeginLevelUpResume();
	void FinishLevelUpResume();
	bool DoAllPlayersHaveNoPendingLevelUpChoices() const;
	int32 GetEnemyCountForWave(int32 WaveIndex) const;

	FTimerHandle WaveStartTimerHandle;
	FTimerHandle CombatResumeVerificationTimerHandle;
	FTimerHandle LevelUpTransitionTimerHandle;
	int32 ScheduledWaveIndex = 0;
	int32 MaxWaveCount = 3;
	int32 InitialWaveEnemyCount = 3;
	int32 EnemyIncrementPerWave = 1;
	int32 KillCurrencyReward = 10;
	int32 KillExperienceReward = 25;
	float WaveIntermissionSeconds = 3.0f;
	double RoundStartTimeSeconds = 0.0;
	double LevelUpPauseStartTimeSeconds = 0.0;
	double AccumulatedLevelUpPauseSeconds = 0.0;
	float RoundStartCastleHealth = 0.0f;
	int32 RoundTrackedEnemyCount = 0;
	bool bContinuousWaveLoopEnabled = true;
	bool bLevelUpPauseTimingActive = false;
};



