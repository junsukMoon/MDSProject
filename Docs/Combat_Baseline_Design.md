# Combat Baseline Design

이 문서는 MDS v2 `Phase 1: Dedicated Server Combat Baseline`의 설계 기준을 정의합니다.

## Objective

MDS v2는 Dedicated Server Objective Combat Demo입니다.

Phase 1의 목표는 full combat system을 만드는 것이 아니라, 이후 Enemy HP, Objective HP, Wave, UI, animation 연동이 따를 수 있는 서버 권한 전투 기준을 확정하는 것입니다.

## MVP Combat Scope

포함 범위:

- player attack request path
- server-side attack validation
- server-side damage application rule
- enemy HP / Objective HP / Wave progression이 서버 소유라는 경계
- replicated state가 UI와 animation presentation을 구동한다는 원칙
- Attack Montage와 AnimNotify timing의 역할 정의
- combat debug/log 기준

제외 범위:

- inventory weapon system
- skill tree
- full GAS expansion
- advanced combo system
- client-authoritative combat
- animation notify 직접 damage
- production hitbox/lag compensation system
- final enemy AI behavior
- full Wave implementation

## Authority Model

Combat state is server-owned.

서버가 소유하는 상태:

- attack validation result
- damage application
- enemy HP
- Objective HP
- enemy death state
- Wave progression

클라이언트가 할 수 있는 일:

- input 생성
- attack request 전송
- local presentation 재생
- replicated state 기반 UI 갱신
- replicated state 기반 Hit Reaction / Death Animation 표시

클라이언트가 하면 안 되는 일:

- enemy HP 직접 변경
- Objective HP 직접 변경
- Wave state 직접 변경
- AnimNotify 또는 montage event만으로 authoritative damage 적용

## Minimal Combat Flow

```text
Owning Client Input
-> optional local Attack Montage presentation
-> Server Attack Request
-> Server validates attack state / range / target / cooldown
-> Server applies damage
-> Server updates replicated combat state
-> Clients update UI and animation presentation
```

Phase 1에서는 이 flow를 기준으로 설계합니다. 실제 enemy HP component, replicated death state, Wave manager는 후속 Phase에서 작게 나눠 구현합니다.

## Objective Damage Integration Rule

Objective damage follows the same server-authoritative rule as enemy damage.

```text
Enemy reaches Objective attack range
-> server validates range / overlap / attack cadence
-> server calls Objective damage path
-> AMDSObjectiveActor updates CurrentHealth on server
-> CurrentHealth replicates
-> clients update Objective UI and debug presentation
```

규칙:

- Objective HP는 `AMDSObjectiveActor`가 소유합니다.
- Objective damage는 서버 검증 이후에만 적용합니다.
- client, UI, AnimNotify, montage event는 Objective HP를 직접 변경하지 않습니다.
- Wave는 Objective damage를 직접 소유하지 않습니다.
- Objective HP가 0 이하가 되는 상황은 future loss condition 후보이며, full win/loss flow는 MVP 범위가 아닙니다.

구현 순서 기준:

1. 기존 ObjectiveActor의 authority와 `CurrentHealth` replication을 먼저 확인합니다.
2. 최소 Enemy actor 또는 server-only damage source를 추가합니다.
3. 서버에서 Objective attack range 또는 overlap을 검증합니다.
4. 검증 성공 시 Objective damage path를 호출합니다.
5. 클라이언트 UI가 replicated Objective HP만 읽는지 확인합니다.
6. client-only event로 Objective HP가 감소하지 않는지 dedicated server에서 확인합니다.

## Attack Request Rule

Client request는 gameplay intent입니다. 결과가 아닙니다.

Server validation should check:

- requester is valid and possessed
- requester has authority path through owning connection
- target is valid
- target is damageable
- attack is allowed by server-side state
- range or overlap condition is valid
- cooldown or attack lockout is valid if implemented

Validation 실패 시:

- damage를 적용하지 않습니다.
- 필요한 경우 debug log만 남깁니다.
- client presentation은 replicated result로 정정됩니다.

## Animation Timing Rule

Attack Montage와 AnimNotify는 combat presentation과 timing marker입니다.

Allowed uses:

- montage playback cue
- local swing timing marker
- server request timing marker
- debug marker
- cosmetic sound/VFX trigger

Forbidden uses:

- AnimNotify가 enemy HP를 직접 감소
- AnimNotify가 Objective HP를 직접 감소
- simulated client montage event가 damage 적용
- montage playback 성공을 damage 성공으로 간주

면접 설명 기준:

```text
Animation can express intent and timing, but the server decides whether damage actually happens.
```

## Replication Baseline

Phase 1에서 replication 대상은 구체 class 구현 전 기준만 정의합니다.

Replicated state 후보:

- enemy current HP
- enemy alive/dead state
- Objective current HP
- current Wave index
- enemies remaining
- current attack or combat cue if needed for simulated client presentation

주의:

- replicated state는 gameplay truth를 클라이언트에 전달하는 경로입니다.
- UI와 animation은 replicated state를 읽습니다.
- UI와 animation은 gameplay truth를 직접 쓰지 않습니다.

## Expected Files or Systems

후속 구현에서 예상되는 최소 시스템:

- player character or combat component
- server attack request function
- damageable enemy actor or component
- Objective damage interface/path
- replicated combat state
- debug output for attack validation and damage result

Phase 1 문서화 작업에서는 위 시스템을 구현하지 않습니다.

## RPC Ownership Assumption

Server RPC가 필요할 경우 owning client가 소유한 PlayerController, Pawn, Character, 또는 그 하위 replicated component에서 시작합니다.

규칙:

- Server RPC는 client request를 서버가 validate/apply해야 할 때만 사용합니다.
- RPC는 damage 결과를 확정하지 않습니다.
- 서버가 validation 후 state를 변경합니다.
- 변경된 state는 replicated property 또는 server-authoritative event로 클라이언트에 전달합니다.
- high-frequency reliable RPC는 피합니다.

## Debug / Log Baseline

combat debug는 면접 설명과 dedicated server 검증에 도움이 되어야 합니다.

권장 debug 항목:

- NetMode
- LocalRole / RemoteRole
- attack requester
- target
- validation result
- damage amount
- target HP before/after
- Objective HP before/after if affected
- Wave index if relevant

예시:

```text
MDS Combat | NetMode=DedicatedServer | Requester=P1 | Target=Enemy_03 | Valid=true | Damage=10 | EnemyHP=30->20
```

## Verification Criteria

Phase 1 문서 기준:

- combat state ownership이 서버로 정의되어 있습니다.
- client request와 server validation이 분리되어 있습니다.
- AnimNotify가 authoritative damage를 직접 적용하지 않는다고 명시되어 있습니다.
- replicated state가 UI/animation presentation을 구동한다고 명시되어 있습니다.
- Phase 2/3 구현 범위를 침범하지 않습니다.

향후 구현 검증:

- dedicated server log에서 damage가 server path에서만 발생합니다.
- client-only animation event로 enemy HP 또는 Objective HP가 바뀌지 않습니다.
- owning client input이 server request로 전달됩니다.
- simulated client는 replicated state 기반으로 presentation을 갱신합니다.
- invalid target/range/cooldown request는 damage 없이 거절됩니다.
