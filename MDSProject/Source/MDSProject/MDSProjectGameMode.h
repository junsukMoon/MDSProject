// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MDSProjectGameMode.generated.h"

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
	void HandleEnemyDeathForWave();

protected:
	virtual void BeginPlay() override;

private:
	void InitializeWaveDisplayState();
	void ConfigureWaveLoopFromCommandLine();
	void TryAutoStartWaveFromCommandLine();
	void ScheduleWaveStart(int32 WaveIndex, float DelaySeconds);
	void StartScheduledWave();
	void CompleteWaveIfCleared();
	int32 GetEnemyCountForWave(int32 WaveIndex) const;

	FTimerHandle WaveStartTimerHandle;
	int32 ScheduledWaveIndex = 0;
	int32 MaxWaveCount = 3;
	int32 InitialWaveEnemyCount = 3;
	int32 EnemyIncrementPerWave = 1;
	float WaveIntermissionSeconds = 3.0f;
	bool bContinuousWaveLoopEnabled = true;
};



