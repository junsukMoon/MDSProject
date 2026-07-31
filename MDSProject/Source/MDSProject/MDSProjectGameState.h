#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Progression/MDSRoundResultTypes.h"
#include "Shop/MDSShopTypes.h"
#include "MDSProjectGameState.generated.h"

UENUM(BlueprintType)
enum class EMDSMatchPhase : uint8
{
	Waiting,
	Combat,
	RoundEnding,
	RoundSettlement,
	Finished
};

UENUM(BlueprintType)
enum class EMDSLevelUpFlowState : uint8
{
	None,
	TransitionIn,
	Selection,
	TransitionOut
};

UCLASS()
class MDSPROJECT_API AMDSProjectGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMDSProjectGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EMDSMatchPhase GetMatchPhase() const { return MatchPhase; }
	int32 GetCurrentRoundIndex() const { return CurrentRoundIndex; }
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }
	int32 GetEnemiesRemaining() const { return EnemiesRemaining; }
	int32 GetTotalEnemiesThisWave() const { return TotalEnemiesThisWave; }
	bool IsWaveActive() const { return bWaveActive; }
	bool IsCombatSuspended() const { return bCombatSuspended; }
	EMDSLevelUpFlowState GetLevelUpFlowState() const { return LevelUpFlowState; }
	const FMDSRoundResult& GetLastRoundResult() const { return LastRoundResult; }
	const TArray<FMDSShopOffer>& GetActiveShopOffers() const { return ActiveShopOffers; }
	bool IsFinalRoundSettlement() const { return bFinalRoundSettlement; }
	double GetSettlementEndServerTime() const { return SettlementEndServerTime; }

	void SetMatchState(EMDSMatchPhase InMatchPhase, int32 InCurrentRoundIndex);
	void SetCombatSuspended(bool bInCombatSuspended);
	void SetLevelUpFlowState(EMDSLevelUpFlowState InFlowState);
	void SetRoundResult(const FMDSRoundResult& InRoundResult);
	void SetActiveShopOffers(const TArray<FMDSShopOffer>& InShopOffers);
	void SetSettlementState(bool bInFinalRoundSettlement, double InSettlementEndServerTime);
	void SetWaveState(int32 InCurrentWaveIndex, int32 InEnemiesRemaining, bool bInWaveActive, int32 InTotalEnemiesThisWave);
	void SetEnemiesRemaining(int32 InEnemiesRemaining);
	void SetWaveActive(bool bInWaveActive);

protected:
	UFUNCTION()
	void OnRep_WaveState();

	UFUNCTION()
	void OnRep_MatchState();

private:
	bool HasWaveStateAuthority(const TCHAR* Context) const;
	void ApplyCurrentTimeDilation();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	EMDSMatchPhase MatchPhase = EMDSMatchPhase::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	int32 CurrentRoundIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	bool bCombatSuspended = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	EMDSLevelUpFlowState LevelUpFlowState = EMDSLevelUpFlowState::None;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	FMDSRoundResult LastRoundResult;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Shop")
	TArray<FMDSShopOffer> ActiveShopOffers;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	bool bFinalRoundSettlement = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Match")
	double SettlementEndServerTime = 0.0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 EnemiesRemaining = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	int32 TotalEnemiesThisWave = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, VisibleInstanceOnly, Category = "Wave")
	bool bWaveActive = false;
};
