# MDS v2 구조 명세

## 문서 위치

이 문서는 `MDSProject` MVP의 상위 구조 명세입니다.

세부 설계:

- GAS: `Docs/GAS_Architecture.md`
- Round, 전투 중 레벨업, 정산과 상점: `Docs/Round_Settlement_Design.md`
- 검증: `Docs/Verification.md`

## MVP 정의

```text
MDS v2 = GAS 기반 Dedicated Server 성 방어 로그라이트 슈터 버티컬 슬라이스
```

플레이어가 CMC 기반 캐릭터를 직접 조작하고 총으로 Actor 기반 몬스터를 처치해 성 Objective를 방어합니다.

MVP는 상용 완성형 게임이 아닙니다. Dedicated Server에서 전투, 전투 중 레벨업, 라운드 정산·상점과 다음 라운드 시작까지 실제로 플레이 가능한 작은 흐름입니다.

## 핵심 흐름

```text
Match Start
-> Round Combat
-> Wave Spawn
-> Player / Enemy GAS Combat
-> Server Kill / Reward
-> In-Combat Level Up Suspension when needed
-> Combat Resume
-> Round Clear
-> Round Settlement + Shop
-> Ready / Timeout
-> Next Round or Finished
```

## 용어와 상태

Round:

- 전투 시작부터 정산, 상점과 준비까지의 전체 단위

Wave:

- Round 안에서 생성되는 몬스터 묶음

`EMDSMatchPhase`:

- `Waiting`
- `Combat`
- `RoundSettlement`
- `Finished`

전투 중 레벨업 하위 상태:

- `Running`
- `LevelUpTransitionIn`
- `LevelUpSelection`
- `LevelUpTransitionOut`

MVP에서 라운드당 Wave 하나를 사용할 수 있지만 Round와 Wave의 상태와 데이터 이름을 분리합니다.

## Runtime Ownership

| 시스템 | 서버 소유 | 클라이언트 역할 |
| --- | --- | --- |
| 입력 | Ability 허용과 결과 | 로컬 입력과 요청 |
| 이동 | CMC 권한·보정 결과 | 입력, prediction과 presentation |
| 전투 | activation, target, damage, HP와 death | 요청과 연출 |
| Reward | 처치 귀속, 재화, 경험치와 Level | Replication 관찰 |
| Level Up | 중단, 후보, 선택 검증과 강화 | 자신의 Modal과 선택 요청 |
| Objective | HP, damage, repair와 파괴 | HP UI |
| Round/Wave | 시작, 종료, 생성과 전환 | GameState UI |
| Shop | 상품, 가격, 구매와 Effect | 구매 요청과 결과 UI |
| UI | source state | 표시와 소유 요청 전달 |

## Gameplay Framework 책임

### `AMDSProjectGameMode`

- 서버 전용 Match Phase
- Round와 Wave 시작·종료
- 적 spawn 요청과 death event 소비
- 전투 중 레벨업 중단·재개
- 라운드 결과 확정
- 준비 완료, 정산 timeout과 연결 해제 처리
- 마지막 Round와 `Finished`

### `AMDSProjectGameState`

- Match Phase와 전투 하위 상태 Replication
- `CurrentRoundIndex`
- `CurrentWaveIndex`
- `EnemiesRemaining`
- `TotalEnemiesThisWave`
- 팀 공통 `FMDSRoundResult`
- 정산 종료 서버 시간
- 레벨업 처리 대상과 전체 완료 상태

### 신규 `AMDSProjectPlayerState`

- 플레이어 ASC 우선 소유 후보
- 개인 `FMDSPlayerRoundResult`
- 재화, 경험치와 Level
- 미처리 레벨업 수와 후보
- 적용 강화와 구매 내역
- 준비 상태

### `AMDSProjectPlayerController`

- owning client 입력
- Gameplay Ability 입력 전달
- Match HUD, 레벨업 Modal, 정산과 상점 UI
- 강화 선택, 구매와 준비 Server 요청

권한 결과는 확정하지 않습니다.

### `AMDSProjectCharacter`

- PlayerState ASC의 Avatar Actor 후보
- CMC 이동
- 캐릭터 animation presentation
- 입력과 Ability 연결

## GAS 책임

GAS 담당:

- Player Fire, Reload와 선택적 Dash
- Enemy Melee와 Attack Castle
- Damage, Heal, 강화와 상태 이상
- Gameplay Tag 차단
- Montage, AnimNotify 또는 Gameplay Event 공격 timing
- Gameplay Cue 연출

GAS 비담당:

- AI 목표와 path
- Round/Wave
- 처치 귀속과 Reward
- 후보와 상점 상품 생성
- 결과 통계와 다음 Round

Player ASC는 PlayerState, Enemy ASC는 `AMDSCombatEnemyActor` 소유를 우선 검토합니다.

Objective ASC는 기존 `CurrentHealth`와 중복 truth가 생기지 않도록 별도 승인 전까지 도입하지 않습니다.

## 전투 중 레벨업

레벨업은 경험치 충족 즉시 Combat 중에 발생합니다.

```text
server reward
-> threshold reached
-> transition-in slow motion
-> explicit CombatSuspended
-> three-choice Modal
-> server validates selection
-> Infinite upgrade GE
-> all pending selections complete
-> transition-out slow motion
-> same Round/Wave resumes
```

정지:

- 플레이어·적·AI
- 전투 Ability
- Projectile 이동·충돌·damage·수명
- Objective 공격
- spawn과 Wave 전환
- 전투 Timer와 Duration/Periodic Effect

유지:

- UI
- 네트워크
- Replication
- 선택 RPC
- 연결 해제 감지

Dedicated Server World Pause는 사용하지 않습니다.

한 플레이어의 레벨업도 Match 단위 중단을 발생시킵니다. 동시에 여러 플레이어 또는 여러 레벨이 대기 중이면 모든 필수 선택 이후 한 번만 재개합니다.

## Combat

```text
Player / AI Intent
-> Gameplay Ability
-> Server Validation
-> Montage / Gameplay Event timing
-> Gameplay Effect or Objective authority boundary
-> replicated result
-> UI / animation / Gameplay Cue
```

AnimNotify, Gameplay Cue와 UI는 authoritative damage를 적용하지 않습니다.

기존 `AMDSProjectPlayerController`의 직접 Server RPC 공격 경로는 검증된 기반선입니다. GAS 전환 시 이 경로와 새 Ability가 동시에 damage를 적용하지 않도록 단계적으로 대체합니다.

## Enemy

`AMDSCombatEnemyActor`의 다음 기반선을 재사용합니다.

- server-owned HP와 death-once
- Objective 접근
- Wave death notification
- replicated attack/hit/death presentation

목표 구조:

- AI가 목표와 행동 의도 결정
- Enemy ASC가 Ability 실행
- 서버가 Gameplay Effect 또는 Objective damage를 확정

Mass Entity는 MVP 적 경로가 아닙니다.

## Objective

`AMDSObjectiveActor`가 성 전용 HP를 소유합니다.

- `CurrentHealth`는 서버 truth
- damage와 repair는 서버에서만 적용
- Enemy HP에 재사용하지 않음
- UI는 replicated HP만 표시
- Gameplay Cue와 UI가 HP를 변경하지 않음

## Round 결과

`FMDSRoundResult` 팀 공통 후보:

- RoundIndex
- ClearTime
- TotalEnemyCount
- CastleDamageTaken
- CastleHealthRemaining
- CastleHealthPercent

`FMDSPlayerRoundResult` 개인 후보:

- KillCount
- CurrencyEarned
- ExperienceEarned
- CurrencySpent
- CurrentLevel
- CurrentCurrency
- SelectedUpgrades
- bReadyForNextRound

팀 공통 결과와 개인 결과를 한 구조에 섞지 않습니다.

## UI

기존 gameplay UI:

- `UMDSMatchHUDWidget`
- `UMDSObjectiveWorldWidget`
- `UMDSEnemyWorldWidget`

신규 후보:

- `UMDSRoundSettlementWidget`
- `WBP_MDSRoundSettlement`
- `WBP_MDSRoundResultPanel`
- `WBP_MDSShopPanel`
- `WBP_MDSLevelUpChoiceModal`

레벨업 Modal은 Combat HUD 위에 표시합니다. 정산 화면의 하위 단계가 아닙니다.

정산 화면은 왼쪽 결과, 오른쪽 상점을 동시에 표시합니다.

마지막 Round도 같은 결과 UI를 사용하며 다음 Round 버튼을 종료 버튼으로 교체합니다.

## 상점

- `RoundSettlement`에서만 구매 가능
- 서버가 Phase, 상품, 가격, 재화, 중복과 대상을 검증
- 구매 즉시 Effect 적용
- Inventory에 저장하지 않음
- Heal/Repair는 Instant
- 전투 Attribute 강화는 Infinite

마지막 Round의 MVP 기본안은 상점을 표시하되 구매를 비활성화하는 것입니다.

## 준비와 다음 Round

- PlayerState에 준비 상태 보관
- 모든 유효 플레이어가 준비하면 시작
- 또는 정산 제한시간 종료 시 시작
- 연결 해제 플레이어는 집계에서 제외
- 다음 Round 진입 시 준비 상태 초기화

## Animation

- CMC locomotion
- 기본 AnimBP
- Fire/Reload/Enemy Attack Montage
- AnimNotify 또는 Gameplay Event
- server-confirmed Hit Reaction과 Death
- Gameplay Cue VFX/SFX

전투 정지 경계에서 Notify, Montage와 Timer의 중복 실행을 방지합니다.

## Debug와 Verification

Debug는 gameplay truth가 아닙니다.

검증 대상:

- Dedicated Server 권한
- ASC ownership과 Effect 적용
- Round/Wave 분리와 Replication
- 전투 중 감속·정지·선택·복귀
- 정지 중 actor, Projectile, HP, spawn과 Wave 불변
- 다중 플레이어 동시 레벨업
- 정산, 구매, 준비와 timeout
- 마지막 Round

과거 Objective Combat 기반선의 검증 증거를 새 기능의 완료 증거로 사용하지 않습니다.

## MVP 제외

- Inventory, 장비 슬롯과 아이템 드래그
- Quest, Crafting과 SaveGame
- 영구 성장과 계정 재화
- Matchmaking과 Lobby
- 거대한 Skill Tree
- 근거 없는 종합 Score
- Dedicated Server World Pause
- 플레이어별 별도 시간축
- 전체 Mass Entity 적 전환
- 전체 Mover, Motion Matching과 Mutable
- 고급 lag compensation
- 상용 수준의 전체 콘텐츠

## 권장 구현 순서

1. 문서와 용어
2. GAS Build/module setup
3. PlayerState와 Player ASC
4. Player Fire
5. Enemy ASC와 단일 공격
6. Gameplay Tag 차단
7. Round/Wave 분리
8. Reward, Experience와 Level
9. 전투 중단 계약
10. 감속과 레벨업 Modal
11. Upgrade Effect
12. 라운드 결과
13. 정산 UI
14. 상점
15. 준비와 마지막 Round
16. Dedicated Server 다중 클라이언트 검증

각 단계는 별도 계획, 승인, 구현, 검증과 승인 보고서를 사용합니다.
