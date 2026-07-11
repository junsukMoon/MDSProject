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
	void TryAutoStartWaveFromCommandLine();
	void CompleteWaveIfCleared();
};



