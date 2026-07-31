# 아키텍처

## 개요

```text
Player / AI Intent
-> Gameplay Ability
-> Server Validation
-> Gameplay Effect
-> HP / Death / Reward
-> Round / Wave
-> Replicated UI / Animation
```

상세 기준:

- `Docs/MDS_v2_Structure_Spec.md`
- `Docs/GAS_Architecture.md`
- `Docs/Round_Settlement_Design.md`

## Gameplay Framework

### GameMode

`AMDSProjectGameMode`는 서버 전용 규칙 소유자입니다.

- Match Phase 전환
- Round와 Wave 시작·종료
- 실제 생성된 적 수와 사망 이벤트 소비
- 전투 중 레벨업 중단·재개 조정
- 라운드 결과 확정
- 준비 완료와 정산 제한시간 집계
- 마지막 라운드와 매치 종료 판단

### GameState

`AMDSProjectGameState`는 모든 클라이언트가 관찰할 공통 상태를 복제합니다.

- `EMDSMatchPhase`
- `CurrentRoundIndex`
- `CurrentWaveIndex`
- `EnemiesRemaining`
- `TotalEnemiesThisWave`
- 팀 공통 `FMDSRoundResult`
- 정산 종료 서버 시간
- 전투 중 레벨업 전환 단계와 대상 집합

### PlayerState

신규 `AMDSProjectPlayerState`를 플레이어별 지속 상태의 우선 소유자로 검토합니다.

- 개인 라운드 결과
- 매치 전용 재화
- 경험치와 레벨
- 미처리 레벨업 수와 현재 후보
- 구매와 강화 결과
- 다음 라운드 준비 상태
- 플레이어 AbilitySystemComponent의 우선 소유 위치

### PlayerController

`AMDSProjectPlayerController`는 owning client의 입력과 UI를 연결합니다.

- Gameplay Ability 입력 전달
- 라운드 정산, 상점과 레벨업 Modal 생성
- 구매, 강화 선택과 준비 요청
- 입력 모드와 UI focus 전환

권한 결과를 직접 확정하지 않습니다.

## GAS 계층

게임플레이 어빌리티 시스템(Gameplay Ability System, GAS)은 플레이어와 적 전투 행동의 실행 계층입니다.

- Player ASC: PlayerState 소유, Character를 Avatar Actor로 사용하는 방향 우선 검토
- Enemy ASC: `AMDSCombatEnemyActor` 소유 방향 우선 검토
- Objective ASC: 기존 `CurrentHealth`와 truth 중복 위험 때문에 별도 승인 전까지 미결정
- AttributeSet: Health, combat attributes, player progression 관련 책임을 작은 세트로 분리
- Gameplay Ability: Fire, Reload, optional Dash, Enemy Melee, Enemy Attack Castle
- Gameplay Effect: Damage, Heal, Castle Repair 후보, 전투 강화와 상태 이상
- Gameplay Tag: 상태, 행동과 차단 규칙
- Gameplay Cue: VFX, SFX와 UI 연출

AI 목표 선택, 이동 경로, Round/Wave, 보상, 상점 상품과 통계는 GAS 밖에 둡니다.

## 전투 중 레벨업 중단

레벨업은 `Combat` 안의 일시적 하위 상태입니다.

```text
Experience Threshold
-> LevelUpTransitionIn
-> 약 2초 실시간 슬로우 모션
-> CombatSuspended
-> LevelUpSelection
-> Upgrade Applied
-> LevelUpTransitionOut
-> Combat Resumed
```

World Pause를 사용하지 않습니다.

서버는 플레이어, 적, AI, Projectile, damage, Objective 공격, 적 생성과 Wave 전환을 정지합니다. UI, 네트워크, Replication과 선택 RPC는 계속 동작합니다.

여러 플레이어 또는 여러 레벨의 필수 선택이 모두 끝나야 한 번만 재개합니다.

## Objective

`AMDSObjectiveActor`는 성 HP와 파괴 상태의 서버 권한 소유자입니다.

- 기존 `CurrentHealth` Replication 유지
- 서버 전용 damage와 repair 경계
- UI는 Replication된 HP만 읽음
- Objective ASC 도입 전까지 GAS Attribute와 중복 truth를 만들지 않음

## Enemy와 AI

`AMDSCombatEnemyActor`와 `UMDSActorEnemySpawnSubsystem`의 Actor 기반 경로를 MVP 기반선으로 재사용합니다.

- Enemy ASC가 공격 Ability를 실행
- AI는 목표와 행동 의도를 결정
- 이동은 Actor/Character 경로
- death event는 GameMode와 보상 계층에 서버에서 한 번 전달
- MassAI는 향후 확장 및 비교 학습

## Round와 Wave

- Round: 전투, 정산, 상점과 준비를 포함
- Wave: Round 안의 몬스터 묶음
- MVP는 라운드당 Wave 하나를 허용
- 상태와 데이터 이름은 분리
- 마지막 적 처치 후 `RoundEnding`에서 약 2초 실시간 슬로우 모션을 거쳐 `RoundSettlement` 진입
- `RoundSettlement` 중 생성과 전투 Ability 금지

## UI

기존:

- `UMDSMatchHUDWidget`
- `UMDSObjectiveWorldWidget`
- `UMDSEnemyWorldWidget`
- `UMDSDebugOverlayWidget`

신규 후보:

- `UMDSRoundSettlementWidget` / `WBP_MDSRoundSettlement`
- `WBP_MDSRoundResultPanel`
- `WBP_MDSShopPanel`
- `WBP_MDSLevelUpChoiceModal`

UI는 Replication된 데이터를 표시하고 소유 클라이언트 요청을 전달할 뿐 gameplay truth를 직접 수정하지 않습니다.

## Animation

- CMC 기반 이동
- 기본 AnimBP locomotion
- Attack/Reload Montage
- AnimNotify 또는 Gameplay Event 공격 타이밍
- server-confirmed Hit Reaction과 Death Animation
- Gameplay Cue 기반 연출

정지·재개 경계에서 Montage, Notify와 Timer의 중복 처리를 방지해야 합니다.

## Debug와 Profiling

Debug UI는 gameplay truth가 아닙니다. GameState, PlayerState, Objective와 ASC의 서버 상태를 읽기 전용으로 관찰합니다.

Dedicated Server와 다중 클라이언트에서 다음을 검증합니다.

- 권한과 Replication
- Round/Wave
- Ability 차단과 Effect 적용
- 전투 중 감속·정지·선택·재개
- 정산·상점·준비
- Projectile, Timer와 periodic effect 보존

Mass, Mover, Motion Matching, Mutable와 Actor vs Mass profiling은 향후 확장 트랙입니다.
