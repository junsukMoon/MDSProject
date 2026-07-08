# MDS v2 Structure Spec

이 문서는 `MDSProject` v2의 MVP 구조 명세서입니다.

구현은 이 문서를 기준으로 작은 단계로 진행합니다. 이 문서에 없는 시스템은 MVP 구현 범위로 간주하지 않습니다.

## MVP Definition

```text
MDS v2 = Dedicated Server Objective Combat Demo
```

MVP는 완성형 게임이 아니라 면접에서 설명 가능한 서버 권한 전투 데모입니다.

핵심 목표:

- Dedicated Server 환경에서 gameplay state를 서버가 소유합니다.
- 플레이어는 Objective를 방어합니다.
- 서버가 player attack, enemy HP, Objective HP, Wave 진행을 판단합니다.
- 클라이언트는 replicated state를 기반으로 UI와 animation presentation을 갱신합니다.
- 캐릭터 구현 기본기를 보여주기 위해 CMC, Skeletal Mesh, AnimBP, Montage, AnimNotify, Hit/Death animation을 최소 범위로 포함합니다.

## Core Loop

```text
Player Input
-> Server Combat Validation
-> Enemy HP / Death State
-> Wave Progress
-> Objective Threat / Objective HP
-> Replicated UI
-> Animation Presentation
```

MVP에서 중요한 것은 최종 콘텐츠 양이 아니라 이 흐름의 authority, replication, verification을 설명할 수 있는 구조입니다.

## Domain Terms

Objective:

- 플레이어가 방어해야 하는 목표물입니다.
- `AMDSObjectiveActor`가 담당합니다.
- Objective HP는 서버 소유입니다.

Enemy:

- 플레이어가 공격하는 전투 대상입니다.
- enemy HP와 death state는 서버 소유입니다.
- v2 enemy는 기존 Mass entity가 아니라 replicated combat target 기준으로 설계합니다.

Player:

- owning client가 입력을 생성하는 캐릭터입니다.
- movement는 CMC 기준입니다.
- attack input은 서버 검증 요청으로 이어집니다.

Combat:

- client input을 서버가 검증하고 damage를 적용하는 영역입니다.
- animation, UI, debug는 combat 결과를 표시할 뿐 gameplay truth를 쓰지 않습니다.

Wave:

- 적 생성과 진행 상태를 관리하는 서버 소유 흐름입니다.
- Phase 3에서 구체화합니다.

## Runtime Ownership

| System | Server Owns | Client Owns / Displays |
| --- | --- | --- |
| Player input | validation result | local input, local prediction/presentation |
| Player movement | authoritative movement state via CMC flow | input and movement presentation |
| Combat | attack validation, damage result | request intent, animation cue |
| Enemy | HP, alive/dead state | replicated HP/death presentation |
| Objective | HP, damage result | replicated Objective HP UI |
| Wave | wave index, spawn/progress state | replicated wave UI |
| UI | source gameplay state | presentation only |
| Animation | gameplay state result | montage/hit/death presentation |

## Proposed Class / System Responsibilities

### Objective

Primary class:

```text
AMDSObjectiveActor
```

Responsibilities:

- Objective HP 소유
- Objective damage 처리
- `CurrentHealth` replication
- server authority check
- debug state 갱신

Rules:

- Objective는 방어 목표물 전용입니다.
- Enemy HP 용도로 `AMDSObjectiveActor`를 재사용하지 않습니다.
- Objective HP 변경은 서버에서만 수행합니다.
- Objective HP는 기존 `AMDSObjectiveActor::CurrentHealth` replication path를 유지합니다.
- ObjectiveActor를 shared health component로 리팩터링하는 작업은 MVP 필수가 아닙니다.

### Enemy

Expected future class or component:

```text
AMDSCombatEnemyActor
or
UMDSHealthComponent + enemy actor
```

Responsibilities:

- enemy HP 소유
- death state 소유
- server-side damage application
- replicated HP/death state 제공
- hit/death presentation이 읽을 state 제공

Rules:

- `AMDSActorEnemy` is removed from the v2 MVP path.
- `UMDSActorEnemySpawnSubsystem` may be reused as the server-side spawn baseline, but it spawns `AMDSCombatEnemyActor`.
- v2 enemy HP 구조는 `AMDSCombatEnemyActor` 또는 별도 component로 작게 추가합니다.
- full AI behavior는 Phase 2 범위가 아닙니다.
- MVP 기본안은 small v2 enemy actor에 replicated `CurrentHealth`를 두는 방향입니다.
- Death state는 `CurrentHealth <= 0.0f`로 파생합니다.
- MVP에서는 별도 replicated `bIsDead`를 두지 않습니다.
- 서버 중복 death 처리는 optional server-only flag로 처리할 수 있습니다.

### Health

Potential shared component:

```text
UMDSHealthComponent
```

Purpose:

- MaxHealth
- CurrentHealth replication
- IsDead helper derived from `CurrentHealth <= 0.0f`
- server-only damage function
- OnRep hooks for UI/animation presentation
- optional server-only death handled flag

Decision:

- MVP 초반에는 ObjectiveActor를 바로 리팩터링하지 않습니다.
- Enemy HP에 먼저 작은 server-owned health path를 만들고, 공통화 필요가 명확해지면 `UMDSHealthComponent`를 별도 승인 작업으로 추가합니다.
- ObjectiveActor를 HealthComponent로 이전하는 작업은 MVP 필수가 아닙니다.
- `UMDSHealthComponent`는 Phase 2 즉시 구현 대상이 아닙니다.

### Player Character / Combat Request

Expected class area:

```text
MDSProjectCharacter
or
player combat component
```

Responsibilities:

- attack input 처리
- optional local montage presentation 요청
- owning client에서 server attack request 전송
- server-side validation 호출

Rules:

- client request는 intent입니다.
- damage success는 server validation 결과입니다.
- Server RPC는 owning client가 소유한 actor/component 경로에서 시작합니다.

### Combat Service / Component

Expected class area:

```text
UMDSCombatComponent
or
server-side helper on player/enemy
```

Responsibilities:

- attack validation
- target validation
- range/cooldown/lockout check
- damage application dispatch
- debug log

Rules:

- Combat logic must not depend on UI.
- Combat logic must not depend on client animation notify for authority.

### Wave

MVP class decision:

```text
AMDSProjectGameMode
AMDSProjectGameState
```

Responsibilities:

`AMDSProjectGameMode`:

- server-only Wave rules
- wave start/end decision
- enemy spawn request
- enemy death handled event consumption
- next wave decision

`AMDSProjectGameState`:

- replicated current wave index
- replicated enemies remaining
- replicated wave active state
- optional replicated total enemies for current wave

Rules:

- Wave state is server-owned.
- GameMode owns authority.
- GameState carries replicated display state to clients.
- Clients observe GameState and update UI.
- Full Wave implementation is Phase 3, not Phase 2.

### UI

Expected class area:

```text
Match HUD Widget
Objective World Widget
Enemy World Widget
UMDSDebugOverlayWidget for debug only
```

Responsibilities:

- Match HUD displays replicated Wave state from `AMDSProjectGameState`.
- Objective World UI displays replicated Objective HP from `AMDSObjectiveActor`.
- Enemy World UI displays replicated Enemy HP from combat enemy actors.
- Debug Overlay displays local/debug snapshots from `UMDSDebugStateSubsystem`.

Rules:

- Gameplay UI does not modify gameplay state.
- Gameplay UI reads replicated gameplay state.
- Debug Overlay is for development and interview verification only.
- Debug Overlay is not the source of gameplay truth.
- Existing Mass debug summaries are future/reference debug, not required v2 gameplay UI.
- Actor enemy spawn debug is valid only when it represents spawned `AMDSCombatEnemyActor` instances.

### Animation

Expected assets/systems:

```text
Skeletal Mesh Character
Animation Blueprint
Idle / Walk / Run State Machine
Attack Montage
AnimNotify timing marker
Hit Reaction
Death Animation
```

Responsibilities:

- movement presentation
- attack presentation
- hit reaction after server-confirmed damage
- death animation after replicated HP-derived death state

Rules:

- AnimNotify can mark timing.
- AnimNotify cannot directly apply authoritative damage.
- Montage playback success is not damage success.

## Server Authority Boundaries

Server-owned gameplay state:

- enemy HP
- enemy alive/dead state
- Objective HP
- Wave progression
- damage result
- attack validation result

Client-owned or client-local presentation:

- input generation
- local animation playback
- local UI rendering
- cosmetic VFX/SFX

Forbidden authority paths:

- client directly setting enemy HP
- client directly setting Objective HP
- UI changing gameplay state
- AnimNotify applying authoritative damage
- simulated client montage event applying damage

## Replication Contract

MVP replicated state candidates:

- Objective current HP
- enemy current HP
- current wave index
- enemies remaining
- wave active state
- optional combat cue for simulated client presentation

Replication rules:

- Replication carries server truth to clients.
- OnRep handlers may update presentation/debug state.
- OnRep handlers must not create new authoritative gameplay decisions.
- Client-only prediction must be corrected by replicated state.
- Enemy death presentation is derived from replicated enemy HP reaching zero.
- MVP does not require replicated `bIsDead`.

## Combat Flow

```text
1. Owning client presses attack.
2. Character may play local attack presentation.
3. Owning client sends attack request to server.
4. Server validates requester, target, range, and attack state.
5. Server applies damage if valid.
6. Server updates enemy HP/death state.
7. Replicated state reaches clients.
8. UI and animation update from replicated state.
```

Invalid request:

```text
1. Server rejects request.
2. HP does not change.
3. Server may log validation failure.
4. Client presentation is corrected by lack of replicated damage/death result.
```

## Enemy HP / Death Flow

```text
Server ApplyEnemyDamage
-> CurrentHealth changes on server
-> server handles death once if CurrentHealth reaches zero
-> replicated HP/death state reaches clients
-> UI updates
-> Hit Reaction or Death Animation plays from replicated state
```

Design rule:

- Enemy HP is not Objective HP.
- Enemy death is not Wave completion by itself; Wave consumes enemy death/remain count later.
- MVP death state is derived from `CurrentHealth <= 0.0f`.
- MVP does not require replicated `bIsDead`.

## Objective HP Flow

```text
Enemy reaches Objective attack range
-> server validates Objective damage condition
-> server calls Objective damage path
-> AMDSObjectiveActor::ApplyObjectiveDamage
-> CurrentHealth changes on server
-> CurrentHealth replicates
-> clients update Objective HP UI/debug
```

Design rule:

- Objective HP remains inside `AMDSObjectiveActor`.
- ObjectiveActor is not reused as generic health actor.
- Objective damage path is separate from enemy damage path.
- Objective damage is applied only by server-authoritative validation.
- MVP baseline enemy movement may use server-side straight-line movement before future AI/navigation work.
- Client, UI, AnimNotify, and montage events must not directly damage Objective.
- Client presentation observes replicated Objective HP only.
- Objective HP reaching zero may feed a future loss condition, but full win/loss flow is not MVP scope.

Future implementation sequence:

```text
1. Confirm existing AMDSObjectiveActor server authority and CurrentHealth replication.
2. Add minimal enemy actor or server-only damage source.
3. Validate enemy/objective range or overlap on the server.
4. Call Objective damage path only after server validation succeeds.
5. Replicate CurrentHealth to clients.
6. Update Objective World UI/debug from replicated state.
7. Verify client-only animation/UI events cannot reduce Objective HP.
```

## Wave Flow

Phase 3 owns final Wave design.

MVP expected flow:

```text
GameMode starts wave on server
-> GameMode requests combat enemy spawn from UMDSActorEnemySpawnSubsystem
-> UMDSActorEnemySpawnSubsystem spawns AMDSCombatEnemyActor instances
-> GameMode sets GameState wave values
-> Enemy HP reaches zero on server
-> enemy death handled event is consumed by GameMode
-> GameMode decrements enemies remaining in GameState
-> GameState replicates wave values
-> UI observes GameState
-> GameMode advances wave when remaining count reaches zero
```

Wave ownership decision:

- `AMDSProjectGameMode` owns server-only Wave authority.
- `AMDSProjectGameState` owns replicated Wave display state.
- `UMDSActorEnemySpawnSubsystem` may perform the server-side `AMDSCombatEnemyActor` spawn request for Wave start.
- GameState `EnemiesRemaining` and `TotalEnemiesThisWave` should use actual spawned combat enemy count, not only requested count.
- Optional dedicated server verification may use `-MDSAutoStartWave MDSWaveEnemyCount=<N>` to trigger the first wave.
- Wave does not calculate enemy HP.
- Wave consumes server death handled events or server-maintained remaining count.
- Enemy HP reaching zero and Wave completion are separate concepts.
- Existing Actor/Mass baseline spawn systems are reference/future paths, not required v2 Wave architecture.

## UI Flow

MDS v2 separates gameplay UI from debug UI.

Gameplay UI:

```text
AMDSProjectGameState
-> Match HUD
-> WaveIndex / EnemiesRemaining / WaveActive
```

```text
AMDSObjectiveActor::CurrentHealth
-> Objective World UI
-> Objective HP bar/text above Objective
```

```text
Combat Enemy CurrentHealth
-> Enemy World UI
-> Enemy HP bar/text above combat enemies
```

Debug UI:

```text
UMDSDebugStateSubsystem snapshot
-> UMDSDebugOverlayWidget
-> NetMode / debug counters / verification text
```

Rules:

- Match HUD reads GameState.
- Objective World UI reads ObjectiveActor replicated HP.
- Enemy World UI reads Enemy replicated HP.
- Debug Overlay reads DebugStateSubsystem.
- UI never calls damage functions.
- UI never owns combat truth.
- UI visibility rules are presentation only.

MVP Enemy World UI visibility:

- Enemy HP UI may be visible for all spawned combat enemies.
- Alternatively, enemy HP UI may be shown only after recent damage or combat engagement.
- The exact visibility rule is an implementation detail and must not affect gameplay authority.

## Level Content Flow

MDS v2 uses a purpose-built Objective Combat test map.

Recommended map path:

```text
/Game/MDS/Maps/L_MDS_ObjectiveCombat
```

Existing map status:

- `/Game/TopDown/Lvl_TopDown` remains a prototype/reference verification map.
- v2 MVP should use a dedicated Objective Combat map when implementation begins.

MVP layout:

```text
                 [North Spawn]
                      |
                      v

[West Spawn] -> [ Objective ] <- [East Spawn]
                    [PlayerStart]
                      ^
                      |
                 [South Spawn]
```

Required level elements:

- Objective at map center
- PlayerStart near Objective
- North enemy spawn area
- South enemy spawn area
- East enemy spawn area
- West enemy spawn area
- combat space around Objective
- sightline for Objective World UI
- sightline for Enemy World UI as enemies approach
- camera-friendly space for Match HUD verification

Rules:

- Objective is the protected center of the map.
- Player starts near Objective because defending Objective is the player role.
- Enemies approach from four cardinal directions.
- Enemy goal is Objective location or Objective attack range.
- Level content exists to verify server-authoritative combat, Wave, replication, and UI.
- Level art polish is not an MVP goal.
- Complex lanes, maze design, procedural spawn layout, and cinematic dressing are out of scope.

Verification focus:

- Match HUD shows Wave and EnemiesRemaining.
- Objective World UI is visible near map center.
- Enemy World UI is visible on approaching enemies from multiple directions.
- Dedicated Server logs and clients agree on Objective/Wave/Enemy state.

Initial C++ spawn/movement baseline:

- `UMDSActorEnemySpawnSubsystem` spawns `AMDSCombatEnemyActor` around the Objective in North/South/East/West round-robin order.
- `AMDSCombatEnemyActor` uses bounded server tick for straight-line movement toward the Objective.
- On first Objective arrival, the enemy calls the server-authoritative Objective damage path once.
- This baseline does not use NavMesh, AIController, Behavior Tree, pathfinding, repeated attacks, or final animation presentation.

## Animation Flow

Attack:

```text
Input
-> optional local montage
-> server request
-> replicated combat result/cue
-> simulated clients present result
```

Hit:

```text
server-confirmed damage
-> replicated HP/combat cue
-> Hit Reaction
```

Death:

```text
server death state
-> replicated CurrentHealth <= 0.0f
-> Death Animation
```

AnimNotify:

- allowed for visual timing
- allowed for debug marker
- allowed to request server action if routed through validation
- forbidden as direct damage authority

## Debug / Verification Flow

Debug is a verification surface, not gameplay truth.

Authoritative owners:

- Objective HP: `AMDSObjectiveActor`
- Wave authority: `AMDSProjectGameMode`
- replicated Wave display state: `AMDSProjectGameState`
- Enemy HP/death: combat enemy actor or component
- combat validation: server combat path

Debug snapshot source:

```text
server/client gameplay state
-> UMDSDebugStateSubsystem snapshot/cache
-> UMDSDebugOverlayWidget text
-> local debug overlay or log output
```

Debug may expose:

- NetMode
- LocalRole / RemoteRole
- Objective HP
- Enemy HP/death state
- Wave index
- enemies remaining
- wave active state
- last attack validation result
- last damage result
- last Objective damage event
- animation/combat sync marker if useful
- v1/reference Mass and Actor counters

Verification must include:

- dedicated server owns damage
- client cannot directly change enemy HP
- client cannot directly change Objective HP
- AnimNotify alone cannot apply damage
- replicated HP is visible on clients
- death animation follows replicated HP-derived death state
- Wave UI follows replicated wave state
- Debug Overlay does not change HP, damage, Wave, or combat state
- v1/reference debug counters are not treated as v2 gameplay truth

Future implementation sequence:

```text
1. Keep existing UMDSDebugStateSubsystem and UMDSDebugOverlayWidget as read-only debug surfaces.
2. Label existing Mass counters as future/reference debug and actor spawn counters as `AMDSCombatEnemyActor` spawn observations.
3. Add v2 Objective/Wave/Combat debug fields incrementally.
4. Add server log categories for combat validation, Objective damage, and Wave progression.
5. Compare server logs, replicated UI, and Debug Overlay during dedicated server verification.
6. Verify Debug UI cannot invoke gameplay state changes.
```

## Out of Scope for MVP

- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Full GAS expansion
- full Mover migration
- production Motion Matching
- full Mutable pipeline
- Mass Entity as required MVP implementation
- advanced lag compensation
- production hitbox system
- complex animation system
- complex character creator UI

## Future Extensions

Mover:

- future movement architecture study
- not part of MVP movement implementation

Motion Matching:

- future locomotion layer replacement
- not required for MVP AnimBP baseline

Mutable:

- future character customization pipeline
- not required for MVP modular slots

Mass Entity:

- future scalable AI simulation/reference track
- not required for v2 MVP Objective Combat Demo

## Suggested Implementation Order

1. Confirm this structure spec.
2. Define Objective / Enemy HP design.
3. Add minimal replicated enemy HP/death state.
4. Add server attack request/validation.
5. Connect valid attack to enemy damage.
6. Add Wave baseline.
7. Add replicated UI baseline.
8. Add character animation baseline.
9. Connect hit/death presentation to replicated state.
10. Add debug verification output.
11. Run listen server or dedicated server verification.

Each step requires its own plan and approval before implementation.
