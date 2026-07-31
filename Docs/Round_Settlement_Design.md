# 라운드, 전투 중 레벨업 및 정산 설계

## 목적

이 문서는 Round와 Wave, 전투 중 레벨업 중단, 라운드 결과, 상점과 다음 라운드 준비 흐름을 정의합니다.

## 용어

라운드(Round):

- 전투 시작부터 내부 Wave, 라운드 정산, 상점과 다음 라운드 준비까지의 전체 단위입니다.

Wave:

- 한 라운드 안에서 생성되는 몬스터 묶음입니다.

MVP에서는 라운드당 Wave 하나를 사용할 수 있지만 다음 상태를 분리합니다.

- `CurrentRoundIndex`
- `CurrentWaveIndex`
- `EnemiesRemaining`
- `TotalEnemiesThisWave`
- `EMDSMatchPhase`

## Match Phase

`EMDSMatchPhase` 후보:

- `Waiting`
- `Combat`
- `RoundSettlement`
- `Finished`

전투 중 레벨업은 별도 Match Phase가 아니라 `Combat` 안의 하위 중단 상태로 우선 설계합니다. 이를 통해 같은 Round/Wave를 유지한 채 전투를 재개합니다.

전투 하위 상태 후보:

- `Running`
- `LevelUpTransitionIn`
- `LevelUpSelection`
- `LevelUpTransitionOut`

## 기본 라운드 흐름

```text
Waiting
-> Combat 시작
-> Wave 생성
-> 전투와 전투 중 레벨업 반복
-> 모든 적 사망
-> Round 결과 확정
-> RoundSettlement
-> 결과 + 상점
-> 준비 완료 또는 제한시간
-> 다음 Round Combat
-> 마지막 Round 정산
-> Finished
```

## 전투 중 레벨업 흐름

```text
경험치 지급
-> 서버가 레벨업과 미처리 횟수 확정
-> LevelUpTransitionIn
-> 짧은 감속
-> CombatSuspended
-> 3지선다 Modal
-> 서버 선택 검증
-> 강화 Gameplay Effect
-> 남은 선택이 있으면 다음 3지선다
-> 모든 대상의 모든 선택 완료
-> LevelUpTransitionOut
-> 짧은 복귀 감속
-> Combat 재개
```

레벨업은 라운드 정산에서 처리하지 않습니다.

## 전투 정지 범위

정지 대상:

- 플레이어 이동과 전투 입력
- 플레이어와 적 Gameplay Ability
- AI 판단과 이동
- Projectile 이동, 충돌, damage와 수명
- Objective 공격과 damage
- 적 생성
- enemy death 소비 이후 Wave 전환
- 전투 관련 Timer
- 전투 관련 Duration 및 Periodic Effect

계속 동작할 대상:

- UI
- 네트워크 연결
- Replication
- 선택 RPC
- 연결 해제 감지
- 레벨업 후보와 완료 상태 관리

Dedicated Server World Pause는 사용하지 않습니다.

## 감속과 완전 정지

진입·복귀 감속은 짧은 연출 구간입니다. 초기 튜닝 후보는 각각 `0.2~0.5초`지만 구현 시 별도 승인과 실제 테스트를 거칩니다.

클라이언트 시각적 Time Dilation만으로 authoritative simulation을 정지시키지 않습니다.

완전 정지는 명시적인 서버 상태와 각 gameplay 시스템의 중단 계약으로 보장합니다.

Projectile은 최소한 다음을 보존해야 합니다.

- 정지 직전 transform
- velocity
- collision 활성 상태
- 남은 수명
- 관련 Timer 남은 시간

재개 순간 누적 이동, 즉시 만료, 중복 충돌과 중복 damage가 발생하면 안 됩니다.

## 멀티플레이 레벨업 규칙

- 한 플레이어의 레벨업이 발생하면 Match 단위로 전투를 중단합니다.
- 모든 클라이언트가 같은 감속·정지·재개 상태를 관찰합니다.
- 동시에 여러 플레이어가 레벨업하면 각 플레이어가 자신의 후보를 처리합니다.
- 모든 필수 선택이 완료되어야 전투를 한 번만 재개합니다.
- 연결 해제된 플레이어는 대기 집합에서 제거하거나 서버 기본 선택을 적용해야 합니다.
- MVP 기본안은 연결 해제 플레이어를 대기 집합에서 제거하는 것입니다.

## 라운드 결과 데이터

팀 공통 `FMDSRoundResult` 후보:

- `RoundIndex`
- `ClearTime`
- `TotalEnemyCount`
- `CastleDamageTaken`
- `CastleHealthRemaining`
- `CastleHealthPercent`

플레이어별 `FMDSPlayerRoundResult` 후보:

- `KillCount`
- `CurrencyEarned`
- `ExperienceEarned`
- `CurrencySpent`
- `CurrentLevel`
- `CurrentCurrency`
- `SelectedUpgrades`
- `bReadyForNextRound`

팀 공통 데이터와 플레이어별 데이터는 별도 구조와 Replication 범위를 사용합니다.

## 데이터 소유권

GameMode:

- Round/Wave 시작·종료
- 적 전멸과 마지막 Round 판단
- 전투 중단·재개
- 결과 확정
- 준비 및 제한시간 평가

GameState:

- Match Phase와 전투 하위 상태
- Round/Wave 공통 상태
- 팀 공통 라운드 결과
- 정산 종료 서버 시간
- 레벨업 대상과 전체 완료 상태

PlayerState:

- 개인 결과
- 재화, 경험치와 레벨
- 미처리 레벨업 수
- 현재 후보와 선택 결과
- 구매 내역과 준비 상태

Objective:

- 성 HP와 파괴 상태

## 라운드 정산 UI

루트 후보:

```text
WBP_MDSRoundSettlement
```

구성:

```text
WBP_MDSRoundSettlement
- WBP_MDSRoundResultPanel
- WBP_MDSShopPanel
- NextRoundReadyButton
```

전투 중 레벨업 UI `WBP_MDSLevelUpChoiceModal`은 정산 루트의 처리 단계가 아닙니다. 전투 HUD 위에 독립 Modal로 표시합니다.

결과 패널:

- 완료 라운드
- 클리어 시간
- 개인 처치 수
- 개인 획득 재화와 경험치
- 성이 받은 피해
- 성 남은 HP와 비율
- 현재 플레이어 레벨
- 현재 보유 재화
- 해당 라운드에서 선택한 강화의 읽기 전용 목록

## 상점

표시 항목:

- 상품 이름과 효과
- 가격
- 구매 가능 여부
- 현재 재화
- 구매 버튼과 완료 상태

서버 검증:

- `RoundSettlement` 여부
- 요청자 소유권
- 상품 ID와 현재 상품 목록
- 가격과 현재 재화
- 중복 구매 정책
- 효과 대상 유효성

상품은 구매 즉시 효과를 적용하고 Inventory에 저장하지 않습니다.

예시:

- 플레이어 HP 회복
- 성 HP 수리
- 공격력 증가
- 발사속도 증가
- 이동속도 증가
- 탄창 크기 증가

## 다음 라운드 준비

- 플레이어별 준비 상태는 PlayerState가 소유합니다.
- 모든 연결된 유효 플레이어가 준비하면 서버가 다음 라운드를 시작합니다.
- 정산 제한시간이 끝나도 서버가 다음 라운드를 시작합니다.
- 연결 해제된 플레이어는 준비 집계에서 제외합니다.
- Phase 전환 후 준비 상태를 초기화합니다.

## 마지막 라운드

- 동일한 라운드 결과 UI를 표시합니다.
- 다음 라운드 버튼 대신 매치 종료 또는 최종 결과 확인 버튼을 표시합니다.
- 상점 구매 가능 여부는 마지막 라운드 정책으로 명시해야 합니다. MVP 기본안은 표시하되 구매는 비활성화하는 것입니다.
- 별도의 전체 매치 누적 결과 화면은 선택적 확장입니다.

## 명시적 제외

- 범용 Inventory
- 거대한 Skill Tree
- 영구 성장
- 근거 없는 종합 Score
- Dedicated Server World Pause
- 플레이어별 별도 시간축
- 정지 중 다른 플레이어만 전투를 계속하는 구조

## 검증 기준

- Round와 Wave 값이 분리되어 복제됩니다.
- 레벨업은 Combat 중 즉시 발생하고 RoundSettlement에서 발생하지 않습니다.
- 모든 클라이언트가 같은 중단 단계를 관찰합니다.
- 정지 중 actor, Projectile, HP, damage, spawn과 Wave 값이 변하지 않습니다.
- 모든 필수 선택 이후 한 번만 재개합니다.
- 정산 UI는 서버가 확정한 팀·개인 결과를 표시합니다.
- 구매와 준비 요청은 서버가 검증합니다.
- 연결 해제로 레벨업 또는 준비 상태가 교착되지 않습니다.
