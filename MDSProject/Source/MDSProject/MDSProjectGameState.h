#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MDSProjectGameState.generated.h"

UCLASS()
class MDSPROJECT_API AMDSProjectGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMDSProjectGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }
	int32 GetEnemiesRemaining() const { return EnemiesRemaining; }
	int32 GetTotalEnemiesThisWave() const { return TotalEnemiesThisWave; }
	bool IsWaveActive() const { return bWaveActive; }

	void SetWaveState(int32 InCurrentWaveIndex, int32 InEnemiesRemaining, bool bInWaveActive, int32 InTotalEnemiesThisWave);
	void SetEnemiesRemaining(int32 InEnemiesRemaining);
	void SetWaveActive(bool bInWaveActive);

protected:
	UFUNCTION()
	void OnRep_WaveState();

private:
	bool HasWaveStateAuthority(const TCHAR* Context) const;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 EnemiesRemaining = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 TotalEnemiesThisWave = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	bool bWaveActive = false;
};
