# GAS 아키텍처

## 목적

이 문서는 `MDSProject`에서 게임플레이 어빌리티 시스템(Gameplay Ability System, GAS)이 담당하는 범위와 서버 권한 경계를 정의합니다.

GAS는 전투 행동의 실행 계층입니다. AI 의사결정, Round/Wave, 보상, 상점과 결과 통계를 모두 GAS로 옮기지 않습니다.

## AbilitySystemComponent 소유권

플레이어:

- 어빌리티 시스템 컴포넌트(Ability System Component, ASC)는 `AMDSProjectPlayerState` 소유를 우선 검토합니다.
- `AMDSProjectCharacter`는 Avatar Actor가 됩니다.
- 사망·재생성 이후에도 매치 전용 Attribute와 강화가 유지될 수 있어야 합니다.
- PlayerState와 Character 초기화 순서, possession, `OnRep_PlayerState`를 검증해야 합니다.

적:

- `AMDSCombatEnemyActor`가 ASC와 전투 Attribute를 소유하는 방향을 우선 검토합니다.
- AI는 Ability 활성화를 요청하고 서버가 실행 조건과 Effect 적용을 확정합니다.

Objective:

- 기존 `AMDSObjectiveActor::CurrentHealth`가 서버 truth입니다.
- Objective ASC를 즉시 추가하지 않습니다.
- 성 수리와 공통 Attribute 필요성이 명확해질 때 별도 승인 작업으로 검토합니다.
- ASC를 추가할 경우 기존 HP와 이중 truth를 만들면 안 됩니다.

## AttributeSet 후보

작은 책임 단위로 다음 후보를 검토합니다.

`UMDSHealthAttributeSet`:

- Health
- MaxHealth

`UMDSCombatAttributeSet`:

- AttackPower
- FireRate
- MoveSpeed
- MagazineSize

`UMDSPlayerProgressionAttributeSet` 또는 PlayerState 일반 replicated data:

- Experience
- Level
- Currency

경험치, 레벨과 재화는 Gameplay Effect 계산보다 보상·상점 규칙에서 직접 사용하므로 PlayerState 일반 replicated data로 유지하는 방안을 우선 비교합니다.

## Gameplay Ability 범위

플레이어:

- `GA_Player_Fire`
- `GA_Player_Reload`
- `GA_Player_Dash` 선택 사항

적:

- `GA_Enemy_MeleeAttack`
- `GA_Enemy_AttackCastle`

Gameplay Ability는 행동 실행을 담당합니다. 목표 선택, Round/Wave 전환과 보상 집계를 담당하지 않습니다.

## Gameplay Effect 범위

Instant:

- `GE_Damage`
- `GE_Heal`
- `GE_CastleRepair` 후보

Infinite:

- `GE_Upgrade_AttackPower`
- `GE_Upgrade_FireRate`
- `GE_Upgrade_MoveSpeed`
- `GE_Upgrade_MaxHealth`
- `GE_Upgrade_MagazineSize`

Duration/Periodic:

- 상태 이상
- 임시 강화 또는 약화

전투 중 레벨업 정지 동안 Duration과 Period가 흘러가면 안 된다는 것을 MVP 기본 규칙으로 합니다. 실제 구현 방식은 Timer와 ASC 동작을 검증한 뒤 결정합니다.

## Gameplay Tag

상태:

- `State.RoundSettlement`
- `State.ShopOpen`
- `State.LevelUpTransition`
- `State.LevelUpSelection`
- `State.CombatSuspended`
- `State.Dead`
- `State.Stunned`
- `State.Reloading`
- `State.Attacking`

Ability:

- `Ability.Player.Fire`
- `Ability.Player.Reload`
- `Ability.Player.Dash`
- `Ability.Enemy.MeleeAttack`
- `Ability.Enemy.AttackCastle`

`State.RoundSettlement`, `State.LevelUpSelection` 또는 `State.CombatSuspended` 동안 모든 전투 Ability를 차단합니다.

## Gameplay Cue

Gameplay Cue는 다음 연출에 사용할 수 있습니다.

- 발사와 피격 VFX/SFX
- Heal과 강화 피드백
- 레벨업 진입·복귀 감속 연출
- 선택 완료 연출

Gameplay Cue가 HP, 경험치, 재화, Level, Round/Wave 또는 정지 상태를 직접 변경하면 안 됩니다.

## 공격 실행 흐름

플레이어:

```text
Owning Client Input
-> ASC Ability 요청
-> 서버 activation 조건 검증
-> Montage / Gameplay Event timing
-> 서버 hit/target 검증
-> GE_Damage 적용
-> HP 및 death 결과 Replication
-> UI / Hit Reaction / Death presentation
```

적:

```text
AI 목표와 행동 결정
-> Enemy ASC Ability 요청
-> 서버 attack 조건 검증
-> Montage / Gameplay Event timing
-> GE_Damage 또는 Objective damage 경계 호출
-> Replication과 presentation
```

AnimNotify와 Gameplay Event는 실행 타이밍을 전달할 수 있지만 authoritative damage를 직접 적용하지 않습니다.

## 전투 중 레벨업 연결

경험치와 레벨업 조건은 PlayerState와 서버 보상 계층이 소유합니다.

```text
Server Kill Confirmation
-> Reward Granted
-> Experience Threshold Reached
-> GameMode requests LevelUp suspension
-> State.LevelUpTransition
-> State.CombatSuspended + State.LevelUpSelection
-> Candidate selected and validated
-> Infinite upgrade GE applied
-> all pending selections complete
-> transition out
-> combat tags removed
```

실행 중 Ability는 중단 진입 시 다음 기준으로 처리합니다.

- 아직 gameplay effect를 적용하지 않은 공격은 취소
- 이미 서버가 확정한 Effect는 되돌리지 않음
- 정지 이후 새로운 Ability activation 금지
- 재개 순간 AnimNotify나 Effect가 중복 실행되지 않음

## 상점 연결

상점 서비스가 상품, 가격, 중복 구매와 재화를 검증합니다.

GAS는 검증이 끝난 상품 효과만 실행합니다.

- Heal과 수리: Instant Effect 또는 Objective 서버 수리 경계
- 전투 강화: Infinite Effect
- Inventory 생성 없음
- Gameplay Cue가 구매 결과를 확정하지 않음

## Build 및 플러그인 경계

실제 GAS 구현 시 `.Build.cs`와 `.uproject`의 모듈·플러그인 설정을 별도 첫 작업으로 처리합니다.

예상 모듈 후보:

- `GameplayAbilities`
- `GameplayTags`
- `GameplayTasks`

추가 시 각 모듈의 사용 이유와 target build 영향을 설명하고 Dedicated Server build를 검증합니다.

## 단계적 구현 순서

1. Build/module setup
2. PlayerState와 Player ASC
3. 최소 AttributeSet
4. 플레이어 Fire Ability
5. Reload와 선택적 Dash
6. Enemy ASC와 단일 공격 Ability
7. Enemy Objective attack
8. Gameplay Tag 차단
9. Upgrade Gameplay Effect
10. Gameplay Cue

위 단계를 한 작업에 묶지 않습니다.

## 검증 기준

- 서버만 authoritative Effect를 적용합니다.
- 소유권이 없는 클라이언트 RPC 또는 Ability 요청은 상태를 바꾸지 않습니다.
- Dedicated Server에는 cosmetic playback 의존성이 없습니다.
- 정산과 레벨업 정지 중 전투 Ability가 활성화되지 않습니다.
- 재생성 이후 ASC와 강화 상태가 설계한 수명에 맞게 유지됩니다.
- UI는 Attribute와 replicated data를 읽을 뿐 직접 수정하지 않습니다.
- 기존 Server RPC 전투 경로와 GAS 경로가 동시에 damage를 적용하지 않습니다.
