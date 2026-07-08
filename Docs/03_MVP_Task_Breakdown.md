# MVP 작업 분해

이 문서는 `MDSProject` MVP를 작은 작업 단위로 나눈 기록입니다.

모든 non-trivial 작업은 승인 기반 workflow를 따릅니다.

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 명시 승인 대기
5. 승인된 변경만 구현
6. 검증
7. approval report

## Phase Overview

| Phase | 이름 | 목적 |
| --- | --- | --- |
| 0 | Harness / Documentation | 작업 규칙과 범위 정의 |
| 0.5 | MDS v2 Structure Spec | 구현 전 전체 구조 명세 확정 |
| 1 | Dedicated Server Combat Baseline | server-authoritative combat 기준 정의 |
| 2 | Objective / Enemy HP | 서버 소유 HP와 replication 정의 |
| 3 | Wave Baseline | Wave 진행 상태 정의 |
| 4 | Replicated UI Baseline | replicated state 기반 UI 갱신 |
| 5 | Level Content Baseline | Objective Combat Demo 최소 레벨 구성 |
| 6 | Objective Damage Integration | server-authoritative objective damage 연결 |
| 7 | Debug UI / Logging | runtime state 출력 |
| 7.5 | Character Movement & Animation Readiness | CMC, Skeletal Mesh, AnimBP, Montage, Notify, Hit/Death 기준 정의 |
| 8 | Runtime Review / Verification Evidence | Dedicated Server combat loop 검증 증거 정리 |
| 9 | Final README / Interview Summary | 면접 설명 정리 |

## Phase 0.5: MDS v2 Structure Spec

구현 전에 v2 MVP 전체 구조를 먼저 확정합니다.

Objective:

- Dedicated Server Objective Combat Demo의 actor/system 책임과 authority boundary를 정의합니다.

Expected files or systems:

- `Docs/MDS_v2_Structure_Spec.md`
- `Docs/02_Architecture.md`
- `Docs/01_Scope_Constraints.md`

Acceptance criteria:

- Objective, Enemy, Player, Combat, Wave, UI, Animation 책임이 분리되어 있습니다.
- `AMDSObjectiveActor`는 Objective 전용이고 Enemy HP 용도로 재사용하지 않는다고 명시되어 있습니다.
- Enemy HP/death state는 별도 actor/component로 설계한다고 명시되어 있습니다.
- client/AnimNotify/UI가 authoritative HP를 직접 변경하지 않는다고 명시되어 있습니다.
- Future Extension과 MVP 구현 범위가 분리되어 있습니다.

Verification method:

- 문서 inspection
- 구조 키워드 검색
- `git diff -- Docs` 확인

Out of scope:

- C++ gameplay implementation
- Blueprint asset changes
- ObjectiveActor 리팩터링
- Enemy HP component 구현

## Phase 1: Dedicated Server Combat Baseline

Phase 1은 v2 MVP combat의 권한 경계를 정의합니다. 구현 목표는 full combat system이 아니라 후속 Phase에서 Enemy HP, Objective HP, Wave, UI, animation을 붙일 수 있는 최소 서버 권한 기준입니다.

### Task 1.1 Define combat authority boundary

Objective:

- combat state ownership과 server validation 기준을 정의합니다.

Expected files or systems:

- `Docs/Combat_Baseline_Design.md`
- player character or combat component 후보
- server attack request path 후보

Acceptance criteria:

- attack validation, damage, enemy HP, Objective HP, Wave progression이 서버 소유로 정의되어 있습니다.
- client input/request와 server result가 구분되어 있습니다.
- client-authoritative combat이 제외 범위로 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 dedicated server log에서 damage authority path 확인

Out of scope:

- full combat implementation
- final enemy HP component
- final Wave manager

### Task 1.2 Define attack request and validation flow

Objective:

- owning client input이 server validation으로 이어지는 최소 flow를 정의합니다.

Expected files or systems:

- PlayerController / Character input path
- Server RPC 후보
- target validation path 후보

Acceptance criteria:

- client request가 gameplay intent이며 damage result가 아니라고 명시되어 있습니다.
- server validation 항목이 requester, target, range, cooldown/lockout 기준으로 정리되어 있습니다.
- invalid request는 damage 없이 거절된다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 invalid request log 확인

Out of scope:

- production lag compensation
- advanced hitbox system
- combo system

### Task 1.3 Define animation timing rule for combat

Objective:

- Attack Montage와 AnimNotify timing이 combat authority와 어떻게 분리되는지 정의합니다.

Expected files or systems:

- Attack Montage
- AnimNotify marker
- combat request path

Acceptance criteria:

- montage/notify는 presentation 또는 timing marker로 정의되어 있습니다.
- AnimNotify 또는 client-side montage event가 authoritative damage를 직접 적용하지 않는다고 명시되어 있습니다.
- server가 damage 성공 여부를 결정한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 client-only animation event로 HP가 변경되지 않는지 확인

Out of scope:

- montage-only combat 판정
- client-authoritative damage

### Task 1.4 Define replicated combat state baseline

Objective:

- UI와 animation presentation이 읽을 replicated state 후보를 정의합니다.

Expected files or systems:

- replicated enemy HP
- replicated enemy alive/dead state
- replicated Objective HP
- replicated Wave state
- optional combat cue

Acceptance criteria:

- UI와 animation은 replicated state를 읽는다고 명시되어 있습니다.
- UI와 animation이 gameplay truth를 직접 쓰지 않는다고 명시되어 있습니다.
- Phase 2/3에서 세부 구현할 state와 Phase 1의 설계 기준이 구분되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 owning/simulated client에서 replicated state 관찰

Out of scope:

- final replicated property implementation
- full UI implementation
- full Wave implementation

### Task 1.5 Define combat debug/log baseline

Objective:

- dedicated server 검증과 면접 설명에 필요한 combat debug 기준을 정의합니다.

Expected files or systems:

- debug subsystem
- server combat log
- optional UI debug output

Acceptance criteria:

- NetMode, Role, requester, target, validation result, damage, HP before/after를 debug 후보로 정의합니다.
- combat debug가 gameplay authority를 갖지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client log comparison

Out of scope:

- final debug UI layout
- profiling implementation

## Phase 2: Objective / Enemy HP

Phase 2는 Objective HP와 Enemy HP의 책임을 분리하고, 서버 권한 HP 계산과 replication 기준을 정의합니다.

### Task 2.1 Confirm Objective HP ownership

Objective:

- `AMDSObjectiveActor`가 Objective 전용 HP 소유자임을 확정합니다.

Expected files or systems:

- `AMDSObjectiveActor`
- `Docs/MDS_v2_Structure_Spec.md`

Acceptance criteria:

- Objective HP는 `AMDSObjectiveActor::CurrentHealth` replication path를 유지한다고 명시되어 있습니다.
- `AMDSObjectiveActor`를 Enemy HP 용도로 재사용하지 않는다고 명시되어 있습니다.
- ObjectiveActor 리팩터링은 Phase 2 범위가 아니라고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 Objective HP replication regression check

Out of scope:

- ObjectiveActor HealthComponent migration
- Objective gameplay redesign

### Task 2.2 Define Enemy HP ownership

Objective:

- v2 Enemy HP를 별도 enemy actor 또는 component가 소유하도록 정의합니다.

Expected files or systems:

- future `AMDSCombatEnemyActor`
- future enemy HP path
- optional future `UMDSHealthComponent`

Acceptance criteria:

- Enemy HP는 Objective HP와 별도 경로로 정의되어 있습니다.
- MVP 기본안은 small v2 enemy actor에 replicated `CurrentHealth`를 두는 방향입니다.
- `UMDSHealthComponent`는 future refactor 후보이며 즉시 구현하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- enemy actor implementation
- shared health component implementation

### Task 2.3 Define death state from HP

Objective:

- MVP death state를 `CurrentHealth <= 0.0f`로 파생한다고 정의합니다.

Expected files or systems:

- replicated enemy `CurrentHealth`
- optional server-only death handled flag

Acceptance criteria:

- `bIsDead`를 MVP에서 별도 replicate하지 않는다고 명시되어 있습니다.
- client death presentation은 replicated HP가 0 이하인지로 판단한다고 명시되어 있습니다.
- 서버 중복 death 처리는 server-only flag로 처리 가능하다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 HP replication과 death presentation 확인

Out of scope:

- replicated death bool
- downed/revive/ragdoll death phases

### Task 2.4 Define HP replication contract

Objective:

- Objective HP와 Enemy HP의 replication contract를 정의합니다.

Expected files or systems:

- Objective `CurrentHealth`
- Enemy `CurrentHealth`
- UI / animation presentation path

Acceptance criteria:

- Objective `CurrentHealth`와 Enemy `CurrentHealth`가 서버 truth로 정의되어 있습니다.
- UI와 animation은 replicated HP를 읽는 presentation이라고 명시되어 있습니다.
- UI, AnimNotify, client-only montage가 HP를 직접 변경하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client HP 관찰

Out of scope:

- final UI implementation
- combat cue replication

### Task 2.5 Define Wave relationship boundary

Objective:

- Enemy death와 Wave 진행의 경계를 정의합니다.

Expected files or systems:

- enemy death handled event
- future Wave system

Acceptance criteria:

- enemy HP가 0 이하가 되는 것과 Wave completion은 같은 개념이 아니라고 명시되어 있습니다.
- Wave는 Phase 3에서 enemy death/remain count를 소비한다고 명시되어 있습니다.
- Phase 2는 full Wave behavior를 구현하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- Wave manager implementation
- spawn table
- win/loss flow

## Phase 3: Wave Baseline

Phase 3는 Wave authority와 replicated display state를 정의합니다. MVP Wave는 game rule이며, 서버에서만 판단하고 클라이언트는 GameState를 통해 관찰합니다.

### Task 3.1 Define Wave owner classes

Objective:

- Wave authority와 replicated display state의 class 책임을 확정합니다.

Expected files or systems:

- `AMDSProjectGameMode`
- future `AMDSProjectGameState`
- `Docs/MDS_v2_Structure_Spec.md`

Acceptance criteria:

- Wave authority는 `AMDSProjectGameMode`가 소유한다고 명시되어 있습니다.
- replicated Wave display state는 `AMDSProjectGameState`가 소유한다고 명시되어 있습니다.
- WorldSubsystem 또는 standalone replicated actor는 MVP 기본안이 아니라고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 GameMode server-only / GameState client-visible state 확인

Out of scope:

- GameState C++ 구현
- Wave manager 구현
- enemy spawn 구현

### Task 3.2 Define replicated Wave state

Objective:

- 클라이언트 UI가 읽을 최소 Wave state를 정의합니다.

Expected files or systems:

- `CurrentWaveIndex`
- `EnemiesRemaining`
- `bWaveActive`
- optional `TotalEnemiesThisWave`

Acceptance criteria:

- Wave state는 서버가 변경하고 GameState가 replicate한다고 명시되어 있습니다.
- UI는 GameState를 읽는 presentation이라고 명시되어 있습니다.
- 클라이언트가 Wave progression을 직접 변경하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client GameState 값 비교

Out of scope:

- final UI layout
- win/loss UI

### Task 3.3 Define Enemy death consumption

Objective:

- Enemy death와 Wave remaining count의 연결 규칙을 정의합니다.

Expected files or systems:

- enemy server death handled event
- GameMode Wave handler
- GameState enemies remaining value

Acceptance criteria:

- Enemy HP 계산은 Wave가 하지 않는다고 명시되어 있습니다.
- Enemy HP가 0 이하가 되는 것과 Wave clear는 별도 개념이라고 명시되어 있습니다.
- 서버에서 death handled event가 한 번 발생하고 GameMode가 소비한다고 명시되어 있습니다.
- GameMode가 `EnemiesRemaining`을 감소시키고 GameState가 replicate한다고 명시되어 있습니다.
- Wave start 시 `UMDSActorEnemySpawnSubsystem`이 `AMDSCombatEnemyActor` spawn을 수행할 수 있다고 명시되어 있습니다.
- GameState의 enemy count는 실제 spawn 성공 수 기준이라고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 duplicate death event가 remaining count를 중복 감소시키지 않는지 확인

Out of scope:

- enemy HP implementation
- kill reward
- score system

### Task 3.4 Define Wave progression rule

Objective:

- MVP Wave start/clear progression 기준을 정의합니다.

Expected files or systems:

- GameMode Wave start function
- GameMode Wave clear condition
- GameState replicated values

Acceptance criteria:

- Wave start는 서버에서만 수행됩니다.
- Wave clear는 `EnemiesRemaining <= 0` 기준으로 정의됩니다.
- 다음 Wave 시작 방식은 MVP에서 수동 또는 단순 delay로 제한됩니다.
- spawn table, 난이도 scaling, complex win/loss는 제외됩니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server log와 client GameState observation

Out of scope:

- production wave director
- spawn table editor workflow
- difficulty scaling
- complex win/loss flow

### Task 3.5 Define existing baseline boundary

Objective:

- 기존 Actor/Mass baseline과 v2 Wave architecture의 관계를 정리합니다.

Expected files or systems:

- `UMDSActorEnemySpawnSubsystem`
- MassAI future extension
- v2 GameMode/GameState Wave path

Acceptance criteria:

- 기존 `UMDSActorEnemySpawnSubsystem`은 v2 combat enemy spawn baseline으로 재사용할 수 있다고 명시되어 있습니다.
- spawn 대상 actor는 `AMDSCombatEnemyActor`라고 명시되어 있습니다.
- Mass spawn system은 future extension이라고 명시되어 있습니다.
- v2 Wave baseline은 GameMode/GameState 기준이라고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- deleting old baseline systems
- Mass migration
- full spawn subsystem rename/refactor

## Phase 4: Replicated UI Baseline

Phase 4는 v2 gameplay UI와 debug overlay의 책임을 분리합니다. UI는 모두 presentation이며 gameplay authority를 갖지 않습니다.

### Task 4.1 Define Match / Wave HUD

Objective:

- Wave 진행 상태를 표시하는 screen-space HUD 책임을 정의합니다.

Expected files or systems:

- `AMDSProjectGameState`
- Match HUD Widget
- Wave UI text/bar

Acceptance criteria:

- Match HUD는 `AMDSProjectGameState`의 replicated state를 읽는다고 명시되어 있습니다.
- 표시 후보는 `CurrentWaveIndex`, `EnemiesRemaining`, `bWaveActive`입니다.
- Match HUD는 Wave progression을 직접 변경하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client GameState UI 값 비교

Out of scope:

- final HUD layout
- win/loss UI
- menu UI

### Task 4.2 Define Objective World UI

Objective:

- Objective Actor 위에 표시되는 Objective HP UI 책임을 정의합니다.

Expected files or systems:

- `AMDSObjectiveActor`
- Objective World Widget
- widget component or attached widget actor 후보

Acceptance criteria:

- Objective World UI는 `AMDSObjectiveActor::CurrentHealth` replicated value를 읽는다고 명시되어 있습니다.
- Objective World UI는 Objective HP를 직접 변경하지 않는다고 명시되어 있습니다.
- Objective가 방어 목표물임을 시각적으로 보여주는 역할이라고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 client에서 replicated Objective HP world UI 확인

Out of scope:

- final art/layout
- ObjectiveActor HealthComponent migration

### Task 4.3 Define Enemy World UI

Objective:

- 전투 중인 Enemy 위에 표시되는 Enemy HP UI 책임을 정의합니다.

Expected files or systems:

- v2 combat enemy actor
- Enemy World Widget
- replicated enemy `CurrentHealth`

Acceptance criteria:

- Enemy World UI는 enemy actor의 replicated `CurrentHealth`를 읽는다고 명시되어 있습니다.
- 단일 target만 표시하는 구조가 아니라, 전투 중인 여러 enemy에 표시 가능하다고 명시되어 있습니다.
- Enemy World UI는 damage나 death를 직접 처리하지 않는다고 명시되어 있습니다.
- HP가 0 이하이면 death presentation으로 전환 가능하다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 여러 enemy HP UI가 replicated HP 기준으로 표시되는지 확인

Out of scope:

- complex visibility system
- targeting UI
- threat indicator system

### Task 4.4 Define Enemy HP UI visibility baseline

Objective:

- MVP Enemy World UI visibility 기준을 최소 범위로 정의합니다.

Expected files or systems:

- Enemy World Widget
- combat enemy actor

Acceptance criteria:

- MVP에서는 모든 spawned combat enemy가 HP UI를 가질 수 있다고 명시되어 있습니다.
- 또는 최근 damage/combat engagement 후 일정 시간 표시하는 방식도 허용된다고 명시되어 있습니다.
- visibility rule은 presentation이며 gameplay authority에 영향을 주지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- occlusion-aware UI
- distance LOD UI
- advanced targeting filter

### Task 4.5 Separate Debug Overlay from Gameplay UI

Objective:

- 기존 `UMDSDebugOverlayWidget`의 역할을 v2 gameplay UI와 분리합니다.

Expected files or systems:

- `UMDSDebugOverlayWidget`
- `UMDSDebugStateSubsystem`
- `Docs/UI_Widget_Blueprint_Guide.md`

Acceptance criteria:

- Debug Overlay는 `UMDSDebugStateSubsystem` snapshot을 읽는 검증/면접 보조 UI라고 명시되어 있습니다.
- Debug Overlay는 gameplay truth source가 아니라고 명시되어 있습니다.
- 기존 Mass summary는 future/reference debug라고 명시되어 있습니다.
- Actor spawn summary는 `AMDSCombatEnemyActor` spawn observation일 때만 v2 debug로 사용할 수 있다고 명시되어 있습니다.
- v2 gameplay UI 필수 항목은 Match HUD, Objective World UI, Enemy World UI로 구분되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- debug overlay C++ modification
- gameplay widget implementation
- CommonUI layout work

## Phase 5: Level Content Baseline

Phase 5는 Objective Combat Demo를 검증할 최소 레벨 구성을 정의합니다. 레벨은 production environment가 아니라 server-authoritative combat, Wave, replication, UI 검증용 공간입니다.

### Task 5.1 Define v2 Objective Combat map

Objective:

- MDS v2 전용 Objective Combat 검증 맵 기준을 정의합니다.

Expected files or systems:

- future map `/Game/MDS/Maps/L_MDS_ObjectiveCombat`
- existing map `/Game/TopDown/Lvl_TopDown`
- Project Settings default map, if implementation later changes map selection

Acceptance criteria:

- v2 MVP는 dedicated Objective Combat map을 기준으로 한다고 명시되어 있습니다.
- 기존 TopDown map은 prototype/reference verification map으로 유지한다고 명시되어 있습니다.
- 실제 `.umap` 생성은 별도 구현 승인 작업으로 분리되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 map load와 dedicated server launch 확인

Out of scope:

- `.umap` asset creation
- level art pass
- existing map migration

### Task 5.2 Define central Objective layout

Objective:

- Objective를 맵 중앙의 방어 대상으로 배치하는 기준을 정의합니다.

Expected files or systems:

- `AMDSObjectiveActor`
- Objective World UI
- map center placement marker or actor transform

Acceptance criteria:

- Objective는 map center에 배치한다고 명시되어 있습니다.
- Objective는 Player가 방어해야 하는 protected target이라고 명시되어 있습니다.
- Objective World UI는 중앙 Objective 위에서 식별 가능해야 한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 editor placement 확인
- 향후 구현 시 client에서 Objective HP UI visibility 확인

Out of scope:

- Objective art polish
- destructible environment
- multiple Objectives

### Task 5.3 Define PlayerStart near Objective

Objective:

- Player가 Objective 방어 역할을 즉시 수행할 수 있도록 spawn 기준을 정의합니다.

Expected files or systems:

- PlayerStart
- player character spawn path
- Objective location

Acceptance criteria:

- PlayerStart는 Objective 근처에 배치한다고 명시되어 있습니다.
- Player spawn은 Objective와 겹치지 않고 combat space를 보존한다고 명시되어 있습니다.
- Player가 시작 직후 Objective 위치와 역할을 파악할 수 있어야 한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 PIE 또는 dedicated server client spawn 위치 확인

Out of scope:

- team spawn system
- respawn rules
- spawn selection algorithm

### Task 5.4 Define four-direction enemy spawn areas

Objective:

- Enemy가 Objective를 향해 상하좌우 4방향에서 접근하는 spawn 기준을 정의합니다.

Expected files or systems:

- North spawn area
- South spawn area
- East spawn area
- West spawn area
- future GameMode Wave spawn logic

Acceptance criteria:

- Enemy spawn area가 North/South/East/West 4방향으로 구분된다고 명시되어 있습니다.
- Enemy는 각 방향에서 Objective를 향해 접근한다고 명시되어 있습니다.
- spawn layout은 Wave 검증을 위한 단순 구조로 유지한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server spawn log와 client replicated enemy 위치 확인

Out of scope:

- procedural spawn layout
- complex lane scripting
- spawn weighting table
- director AI

### Task 5.5 Define combat space and UI sightlines

Objective:

- Objective 주변 combat space와 UI 검증 가능한 시야 기준을 정의합니다.

Expected files or systems:

- combat area around Objective
- Match HUD
- Objective World UI
- Enemy World UI

Acceptance criteria:

- Objective 주변에 combat space가 필요하다고 명시되어 있습니다.
- Objective World UI는 중앙에서 보일 수 있어야 한다고 명시되어 있습니다.
- Enemy World UI는 여러 방향에서 접근하는 enemy 위에 보일 수 있어야 한다고 명시되어 있습니다.
- Match HUD는 Wave와 EnemiesRemaining 검증에 사용된다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 client viewport에서 UI visibility 확인

Out of scope:

- cinematic camera pass
- occlusion-aware UI
- advanced level readability pass

### Task 5.6 Define level content exclusions

Objective:

- MVP 레벨 작업이 게임 콘텐츠 제작으로 확장되지 않도록 제외 범위를 정의합니다.

Expected files or systems:

- `Docs/01_Scope_Constraints.md`
- `Docs/MDS_v2_Structure_Spec.md`
- `Docs/Verification.md`

Acceptance criteria:

- complex lanes, maze/path design, cinematic dressing, large environment art pass가 제외 범위로 명시되어 있습니다.
- navmesh-heavy level design은 MVP 목표가 아니라고 명시되어 있습니다.
- 레벨은 combat loop 검증용 공간이라고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- production environment
- complex navigation level
- procedural generation

## Phase 6: Objective Damage Integration

Phase 6은 Enemy가 Objective에 피해를 주는 서버 권한 경로를 정의합니다. 이 단계의 문서 목표는 실제 Enemy AI나 full combat 구현이 아니라, 이후 구현 시 Objective HP가 오직 서버 검증 경로로만 감소하도록 contract를 확정하는 것입니다.

### Task 6.1 Define Objective damage authority path

Objective:

- Objective HP 감소가 서버 권한 경로에서만 발생한다는 기준을 정의합니다.

Expected files or systems:

- `AMDSObjectiveActor`
- `AMDSObjectiveActor::ApplyObjectiveDamage`
- future enemy damage source
- future server validation path

Acceptance criteria:

- Objective HP는 `AMDSObjectiveActor`가 소유한다고 명시되어 있습니다.
- Objective damage는 서버에서만 적용된다고 명시되어 있습니다.
- client, UI, AnimNotify, montage event가 Objective HP를 직접 변경하지 않는다고 명시되어 있습니다.
- 구현 시 서버가 damage 조건을 검증한 뒤 Objective damage path를 호출한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 dedicated server log에서 Objective HP 변경 path 확인
- 향후 구현 시 client-only animation event로 Objective HP가 변경되지 않는지 확인

Out of scope:

- ObjectiveActor 리팩터링
- generic HealthComponent 도입
- client-authoritative Objective damage

### Task 6.2 Define enemy-to-objective damage condition

Objective:

- Enemy가 Objective에 피해를 줄 수 있는 최소 조건을 정의합니다.

Expected files or systems:

- future combat enemy actor
- Objective reference or query path
- attack range / overlap condition
- server timer or attack cadence if implemented

Acceptance criteria:

- Enemy가 Objective attack range 또는 overlap 조건을 만족해야 damage 후보가 된다고 명시되어 있습니다.
- 조건 판단은 서버에서 수행한다고 명시되어 있습니다.
- Enemy movement, navigation, spawn 구현은 별도 Phase로 분리한다고 명시되어 있습니다.
- MVP 구현 순서는 `spawn -> approach -> server range check -> Objective damage`로 정리되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server log에서 enemy/objective distance 또는 overlap validation 확인

Out of scope:

- full enemy AI behavior tree
- production hitbox system
- navmesh-heavy arrival logic
- Mass arrival detection implementation

### Task 6.3 Define Objective damage replication result

Objective:

- 서버에서 감소한 Objective HP가 클라이언트 UI와 debug presentation으로 전달되는 방식을 정의합니다.

Expected files or systems:

- `AMDSObjectiveActor::CurrentHealth`
- Objective World UI
- Debug Overlay
- `OnRep` presentation path if implemented

Acceptance criteria:

- Objective HP 변경 후 `CurrentHealth`가 replicate된다고 명시되어 있습니다.
- 클라이언트는 replicated Objective HP를 읽어 UI와 debug만 갱신한다고 명시되어 있습니다.
- `OnRep` 또는 widget update는 gameplay decision을 만들지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client Objective HP 값 비교
- 향후 구현 시 Objective World UI가 replicated HP 기준으로 갱신되는지 확인

Out of scope:

- final Objective UI art
- client-side prediction for Objective HP
- Objective HP rollback/prediction

### Task 6.4 Define Wave and loss boundary

Objective:

- Objective damage와 Wave 진행/loss condition의 책임 경계를 정의합니다.

Expected files or systems:

- `AMDSProjectGameMode`
- `AMDSProjectGameState`
- `AMDSObjectiveActor`
- future loss condition handler

Acceptance criteria:

- Objective damage는 Wave 진행을 직접 소유하지 않는다고 명시되어 있습니다.
- Wave authority는 GameMode가 소유한다고 명시되어 있습니다.
- Objective HP가 0 이하가 되면 loss condition 후보가 될 수 있지만, MVP에서는 복잡한 win/loss flow를 구현하지 않는다고 명시되어 있습니다.
- Enemy death와 Objective damage는 별도 server event로 유지한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 Objective HP 감소가 `EnemiesRemaining`을 직접 변경하지 않는지 확인

Out of scope:

- full win/loss UI
- match reset flow
- score/reward system
- complex Wave director

### Task 6.5 Define implementation sequence after documentation

Objective:

- Phase 6 문서 이후 실제 구현을 어떤 순서로 진행할지 명시합니다.

Expected files or systems:

- existing `AMDSObjectiveActor`
- future combat enemy actor or damage source
- future server validation function
- Objective World UI verification path

Acceptance criteria:

- 먼저 기존 `AMDSObjectiveActor`의 server authority, `CurrentHealth`, replication path를 확인한다고 명시되어 있습니다.
- 다음으로 최소 Enemy actor 또는 server-only damage source를 만든다고 명시되어 있습니다.
- 다음으로 server range/overlap validation 후 Objective damage를 호출한다고 명시되어 있습니다.
- 다음으로 client UI가 replicated HP만 읽는지 검증한다고 명시되어 있습니다.
- 마지막으로 dedicated server에서 client-only event로 Objective HP가 감소하지 않는지 검증한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 PR/작업 계획에서 위 순서를 체크리스트로 사용

Out of scope:

- 이번 문서 작업에서 gameplay code 구현
- Blueprint asset creation
- `.umap` placement

## Phase 7: Debug UI / Logging

Phase 7은 Dedicated Server Objective Combat Demo를 검증하기 위한 debug surface와 log 기준을 정의합니다. Debug UI는 gameplay UI가 아니며, gameplay truth source도 아닙니다. Debug UI와 log는 서버 권한 전투, Objective HP, Wave, UI/animation presentation을 설명하고 검증하기 위한 관찰 도구입니다.

### Task 7.1 Define v2 debug data ownership

Objective:

- `UMDSDebugStateSubsystem`의 역할을 gameplay truth가 아닌 snapshot/cache로 정의합니다.

Expected files or systems:

- `UMDSDebugStateSubsystem`
- `FMDSDebugStateSnapshot`
- `UMDSDebugOverlayWidget`
- server-owned gameplay systems

Acceptance criteria:

- DebugStateSubsystem은 authoritative gameplay owner가 아니라고 명시되어 있습니다.
- DebugStateSubsystem은 server/client runtime state를 관찰 가능한 snapshot으로 보관한다고 명시되어 있습니다.
- HP, damage, Wave, combat state의 실제 소유자는 ObjectiveActor, GameMode, GameState, combat/enemy system이라고 명시되어 있습니다.
- Debug UI는 gameplay state 변경 함수를 호출하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 Debug UI에서 HP/Wave/Combat 변경 경로가 없는지 확인

Out of scope:

- DebugStateSubsystem을 gameplay manager로 승격
- Debug UI에서 gameplay command 실행
- cheat/debug command framework

### Task 7.2 Define v2 debug display fields

Objective:

- v2 MVP 검증에 필요한 debug 표시 후보를 정의합니다.

Expected files or systems:

- `UMDSDebugOverlayWidget`
- `UMDSDebugStateSubsystem`
- Match HUD / Objective World UI / Enemy World UI와 비교 가능한 debug text

Acceptance criteria:

- v2 debug 표시 후보에 NetMode, LocalRole/RemoteRole, Objective HP, Wave index, EnemiesRemaining, Enemy count 또는 Enemy HP summary가 포함되어 있습니다.
- combat debug 후보에 last attack validation result, last damage result, last Objective damage event가 포함되어 있습니다.
- animation/combat sync debug marker는 선택 항목으로 정의되어 있습니다.
- 기존 Mass/Actor counters는 v1/reference debug로 유지한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 dedicated server와 client overlay/log에서 표시 항목 비교

Out of scope:

- final debug UI layout
- large dashboard UI
- production telemetry system

### Task 7.3 Define server logging baseline

Objective:

- Dedicated Server에서 combat/objective/wave 검증에 필요한 log 기준을 정의합니다.

Expected files or systems:

- combat validation log category
- Objective damage log
- Wave progression log
- replicated state observation log if needed

Acceptance criteria:

- server combat validation log는 requester, target, validation result, damage amount, HP before/after를 포함한다고 명시되어 있습니다.
- Objective damage log는 damage source, validation condition, Objective HP before/after를 포함한다고 명시되어 있습니다.
- Wave log는 wave start, enemies remaining change, wave clear 후보를 포함한다고 명시되어 있습니다.
- client log는 replicated state observation용이며 authoritative result로 간주하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 dedicated server log와 client log 비교

Out of scope:

- structured telemetry backend
- analytics pipeline
- verbose per-frame logging

### Task 7.4 Define gameplay UI vs debug UI boundary

Objective:

- Match HUD, Objective World UI, Enemy World UI와 Debug Overlay의 책임을 분리합니다.

Expected files or systems:

- Match HUD
- Objective World UI
- Enemy World UI
- `UMDSDebugOverlayWidget`
- `Docs/UI_Widget_Blueprint_Guide.md`

Acceptance criteria:

- gameplay UI는 replicated gameplay state presentation이라고 명시되어 있습니다.
- Debug Overlay는 snapshot/log verification용이라고 명시되어 있습니다.
- Debug Overlay가 Match HUD 또는 World UI를 대체하지 않는다고 명시되어 있습니다.
- Debug Overlay는 HP, damage, Wave, combat state를 직접 변경하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 UI data source와 interaction path 확인

Out of scope:

- CommonUI screen framework expansion
- menu UI
- complex UX flow

### Task 7.5 Define debug implementation sequence

Objective:

- Phase 7 문서 이후 실제 debug/log 구현을 어떤 순서로 진행할지 명시합니다.

Expected files or systems:

- existing `UMDSDebugStateSubsystem`
- existing `UMDSDebugOverlayWidget`
- future v2 combat/wave/objective debug fields
- future log categories

Acceptance criteria:

- 먼저 기존 DebugStateSubsystem과 DebugOverlayWidget을 유지한다고 명시되어 있습니다.
- 다음으로 v1 Mass/Actor counters를 reference debug로 라벨링한다고 명시되어 있습니다.
- 다음으로 v2 Objective/Wave/Combat debug fields를 작은 단위로 추가한다고 명시되어 있습니다.
- 다음으로 server log category를 combat, objective damage, wave progression 기준으로 정리한다고 명시되어 있습니다.
- 마지막으로 Debug UI가 gameplay state를 변경하지 않는 negative test를 수행한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 PR/작업 계획에서 위 순서를 체크리스트로 사용

Out of scope:

- 이번 문서 작업에서 C++ debug field 추가
- Widget Blueprint asset 변경
- profiling dashboard 구현

## Phase 7.5: Character Movement & Animation Readiness

MDS v2는 Dedicated Server Objective Combat Demo입니다. Character / Animation layer는 전투 데모를 설명 가능하게 만드는 최소 범위로만 포함합니다.

### Task 7.5.1 Define CMC-based movement baseline

Objective:

- MVP movement 구현을 CharacterMovementComponent 기준으로 정의합니다.

Expected files or systems:

- Character class
- PlayerController input path
- movement debug output
- `Docs/Character_Movement_Animation_Readiness.md`

Acceptance criteria:

- MVP가 CMC 기반 movement를 사용한다고 문서화되어 있습니다.
- movement authority, ownership, replication 설명이 포함되어 있습니다.
- full Mover migration이 MVP 제외 범위로 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 listen server 또는 dedicated server에서 role/authority movement debug 확인

Out of scope:

- full Mover migration
- custom network prediction movement system

### Task 7.5.2 Add animation state requirements

Objective:

- Skeletal Mesh player character와 AnimBP baseline 요구사항을 정의합니다.

Expected files or systems:

- Skeletal Mesh character
- Animation Blueprint
- Idle / Walk / Run State Machine

Acceptance criteria:

- MVP animation baseline이 Idle / Walk / Run State Machine으로 정의되어 있습니다.
- animation state가 필요한 경우 gameplay state 또는 replicated state를 기준으로 구동된다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 owning client와 simulated client에서 locomotion state 확인

Out of scope:

- production animation system
- complex locomotion layering

### Task 7.5.3 Define attack montage and notify timing rules

Objective:

- Attack Montage와 AnimNotify timing의 역할을 정의합니다.

Expected files or systems:

- Attack Montage
- AnimNotify marker
- server attack validation path

Acceptance criteria:

- Attack Montage는 combat presentation으로 정의되어 있습니다.
- AnimNotify는 visual timing 또는 sync marker로만 사용됩니다.
- AnimNotify 또는 client-side montage event가 authoritative damage를 직접 적용하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server log에서 damage authority path 확인
- client-only animation event로 Objective damage가 발생하지 않는지 확인

Out of scope:

- client-authoritative damage
- montage-only combat 판정

### Task 7.5.4 Define hit/death animation handling

Objective:

- Hit Reaction과 Death Animation의 gameplay state 연동 방식을 정의합니다.

Expected files or systems:

- replicated health/death state
- Hit Reaction animation
- Death Animation

Acceptance criteria:

- Hit Reaction은 server-confirmed damage 이후 표시됩니다.
- Death Animation은 replicated HP 기준 death 이후 표시됩니다.
- animation presentation과 gameplay authority가 분리되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client에서 damage, death state, animation 표시 순서 확인

Out of scope:

- ragdoll system
- complex death pipeline

### Task 7.5.5 Document server-authoritative damage vs client visual animation

Objective:

- combat-to-animation synchronization 규칙을 문서화합니다.

Expected files or systems:

- combat component or character combat path
- replicated combat state
- UI / animation presentation path

Acceptance criteria:

- damage, enemy HP, Objective HP, Wave 진행은 서버 소유로 정의되어 있습니다.
- 클라이언트는 replicated state를 기반으로 UI와 연출을 갱신한다고 명시되어 있습니다.
- client visual animation이 authoritative gameplay state를 직접 변경하지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 server/client log comparison

Out of scope:

- client-side authoritative combat

### Task 7.5.6 Add Motion Matching future extension notes

Objective:

- Motion Matching을 MVP 구현이 아닌 future extension으로 정리합니다.

Expected files or systems:

- `Docs/Animation_MotionMatching_Notes.md`

Acceptance criteria:

- State Machine + Montage + AnimNotify 기반 MVP 접근이 설명되어 있습니다.
- Pose Search Database, Pose Search Schema, Pose History, trajectory query 개념이 설명되어 있습니다.
- Motion Matching이 locomotion layer를 대체할 수 있는 future extension이라고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- production Motion Matching implementation
- full Pose Search database tuning

### Task 7.5.7 Add Mover future extension notes

Objective:

- Mover를 MVP 구현이 아닌 future movement extension으로 정리합니다.

Expected files or systems:

- `Docs/Character_Movement_Animation_Readiness.md`

Acceptance criteria:

- CMC가 MVP default movement라고 명시되어 있습니다.
- Mover migration 시 검토할 항목이 정리되어 있습니다.
- full Mover migration이 MVP 제외 범위로 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- Mover plugin migration
- custom movement prediction rewrite

### Task 7.5.8 Add Mutable future extension notes

Objective:

- Mutable과 modular character parts를 future customization extension으로 정리합니다.

Expected files or systems:

- `Docs/Character_Customization_Notes.md`
- optional Head / Body / Weapon mesh slots

Acceptance criteria:

- simple modular mesh slots는 선택적 MVP 범위로 제한되어 있습니다.
- Mutable은 future extension으로 문서화되어 있습니다.
- project가 character customization demo가 아니라고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- full Mutable pipeline
- complex character creator UI
- advanced skeletal mesh merge/cook pipeline

## Phase 8: Runtime Review / Verification Evidence

Phase 8은 formal profiling 단계가 아니라 v2 MVP가 의도대로 만들어지고 있는지 검증 증거를 정리하는 단계입니다. MDS v2의 핵심은 성능 수치가 아니라 Dedicated Server 환경에서 server-authoritative combat, Objective HP, Wave, Replication, UI/Animation sync가 올바르게 동작한다는 runtime evidence입니다.

### Task 8.1 Define runtime review scope

Objective:

- MVP Phase 8의 범위를 profiling이 아니라 runtime verification evidence로 정의합니다.

Expected files or systems:

- `Docs/Verification.md`
- `Docs/08_Profiling_Comparison.md`
- dedicated server run notes
- client observation notes

Acceptance criteria:

- Phase 8은 Runtime Review / Verification Evidence라고 명시되어 있습니다.
- formal performance profiling은 MVP 필수가 아니라고 명시되어 있습니다.
- Actor vs Mass benchmark, CSV profiling, Unreal Insights trace는 future extension/reference로 분리되어 있습니다.
- runtime review는 서버 권한 gameplay가 의도대로 동작하는지 확인하는 단계라고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 evidence checklist 작성

Out of scope:

- performance optimization pass
- benchmark report 작성
- Mass scaling comparison

### Task 8.2 Define MVP verification evidence checklist

Objective:

- v2 MVP 완료 판단에 필요한 runtime evidence 항목을 정의합니다.

Expected files or systems:

- Dedicated Server log
- client log
- screenshots or short GIF/video
- Match HUD
- Objective World UI
- Enemy World UI
- Debug Overlay

Acceptance criteria:

- evidence 항목에 Dedicated Server 실행 결과, server/client log review, Objective HP replication, Wave state replication, Enemy HP/death presentation이 포함되어 있습니다.
- UI가 replicated state만 읽는지 확인하는 항목이 포함되어 있습니다.
- AnimNotify 또는 client-only event가 damage를 직접 만들지 않는 negative test가 포함되어 있습니다.
- Debug Overlay가 gameplay truth가 아니라는 확인 항목이 포함되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 manual test report와 screenshot/log 첨부

Out of scope:

- automated test framework 구축
- full QA test matrix
- platform compatibility matrix

### Task 8.3 Define runtime review scenario

Objective:

- 반복 가능한 v2 MVP runtime review scenario를 정의합니다.

Expected files or systems:

- `/Game/MDS/Maps/L_MDS_ObjectiveCombat`
- Dedicated Server launch path
- one or more clients
- Objective actor
- combat enemy actor
- Wave state

Acceptance criteria:

- runtime scenario는 Dedicated Server + client 기준이라고 명시되어 있습니다.
- Objective는 map center, PlayerStart는 Objective 근처, enemy는 4방향 spawn area에서 접근한다고 명시되어 있습니다.
- 검증 흐름은 `spawn -> approach -> server combat/objective damage -> replication -> UI/animation presentation`이라고 명시되어 있습니다.
- scenario context는 map, server/client count, enemy count, wave index와 함께 기록한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 구현 시 동일 scenario로 반복 실행

Out of scope:

- large enemy count stress test
- long soak test
- performance benchmark scenario

### Task 8.4 Define profiling as future extension

Objective:

- 기존 profiling/Actor vs Mass 자료를 MVP 필수가 아닌 future extension/reference로 재분류합니다.

Expected files or systems:

- `Docs/08_Profiling_Comparison.md`
- `MDSGameplayProfileSubsystem`
- Mass future extension notes

Acceptance criteria:

- 기존 CSV profiling harness는 유지하되 MVP completion requirement가 아니라고 명시되어 있습니다.
- Actor vs Mass comparison은 Mass Entity future extension에서 사용할 reference라고 명시되어 있습니다.
- Unreal Insights trace는 smoke/reference artifact이며 MVP 필수 산출물이 아니라고 명시되어 있습니다.
- Phase 8 MVP evidence는 profiling number보다 runtime correctness를 우선한다고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- deleting profiling subsystem
- running CSV capture
- new Insights trace capture

### Task 8.5 Define runtime review implementation sequence

Objective:

- Phase 8 문서 이후 실제 runtime review를 어떤 순서로 진행할지 명시합니다.

Expected files or systems:

- dedicated server launch setup
- v2 Objective Combat map
- logs
- screenshots/GIF/video evidence
- verification report

Acceptance criteria:

- 먼저 dedicated server와 client 실행 경로를 정리한다고 명시되어 있습니다.
- 다음으로 Objective/Wave/Enemy/UI/Animation check를 하나의 repeatable scenario로 묶는다고 명시되어 있습니다.
- 다음으로 server log와 client observed replicated state를 비교한다고 명시되어 있습니다.
- 다음으로 screenshot/GIF/log snippets를 evidence로 저장한다고 명시되어 있습니다.
- 마지막으로 profiling은 필요해졌을 때 future extension으로 별도 실행한다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- 향후 final verification report에서 checklist로 사용

Out of scope:

- 이번 문서 작업에서 dedicated server 실행
- formal profiling run
- performance optimization

## Phase 9: Final README / Interview Summary

Phase 9는 MDS v2를 면접과 포트폴리오에서 설명 가능한 형태로 압축합니다. 목표는 구현 범위를 키우는 것이 아니라, MVP 의도, authority boundary, replication 흐름, character/animation baseline, future extension 위치를 명확하게 정리하는 것입니다.

### Task 9.1 Update README positioning

Objective:

- README를 MDS v2의 첫 진입 문서로 정리합니다.

Expected files or systems:

- `README.md`
- `Docs/00_Project_Goal.md`
- `Docs/01_Scope_Constraints.md`

Acceptance criteria:

- README가 MDS v2를 Dedicated Server Objective Combat Demo로 설명합니다.
- 핵심 주제가 server-authoritative combat, Objective defense, Replication, Wave, UI/Animation presentation, Runtime Review로 정리되어 있습니다.
- Profiling은 MVP 필수가 아니라 future extension/reference로 분리되어 있습니다.
- Mover, Motion Matching, Mutable, Mass Entity는 future extension으로 명시되어 있습니다.

Verification method:

- 문서 inspection
- keyword search

Out of scope:

- README를 긴 기술 문서로 확장
- gameplay code 구현

### Task 9.2 Create interview summary

Objective:

- 면접에서 바로 말할 수 있는 짧은 설명과 구조 요약을 문서화합니다.

Expected files or systems:

- `Docs/Interview_Summary.md`

Acceptance criteria:

- 30초 설명이 포함되어 있습니다.
- 핵심 architecture flow가 포함되어 있습니다.
- MVP includes/excludes가 간결하게 정리되어 있습니다.
- Future Extension 항목이 Mover, Motion Matching, Mutable, Mass Entity로 정리되어 있습니다.
- Runtime Review evidence 기준이 포함되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- 발표 대본 전체 작성
- 포트폴리오 웹페이지 제작

### Task 9.3 Align MVP scope and exclusions

Objective:

- README, project goal, scope constraints의 MVP 포함/제외 범위를 일관되게 맞춥니다.

Expected files or systems:

- `README.md`
- `Docs/00_Project_Goal.md`
- `Docs/01_Scope_Constraints.md`

Acceptance criteria:

- Runtime Review / Verification Evidence가 MVP 검증 기준으로 명시되어 있습니다.
- formal profiling, Actor vs Mass benchmark, CSV/Insights trace는 MVP 제외로 명시되어 있습니다.
- 기존 profiling 자료는 reference/future extension으로 유지된다고 명시되어 있습니다.

Verification method:

- 문서 inspection
- `rg` keyword check

Out of scope:

- profiling subsystem 삭제
- CSV/Insights capture 실행

### Task 9.4 Define final implementation direction

Objective:

- 문서 정리 이후 실제 구현을 어떤 순서로 진행할지 요약합니다.

Expected files or systems:

- `Docs/Interview_Summary.md`
- `Docs/03_MVP_Task_Breakdown.md`

Acceptance criteria:

- 다음 구현 순서가 ObjectiveActor 확인, GameState/Wave display state, minimal combat enemy, Objective damage, UI/animation presentation, runtime evidence 순으로 정리되어 있습니다.
- 구현 범위가 MDS v2 MVP를 벗어나지 않는다고 명시되어 있습니다.

Verification method:

- 문서 inspection

Out of scope:

- 이번 문서 작업에서 implementation 시작
- full animation system
- Mass migration

## Future Extension: Mass 작업 순서

Mass 작업은 v2 MVP 필수 구현이 아니라 future extension으로 재분류합니다. Mass를 다시 진행할 경우 다음 순서를 지킵니다.

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection only
5. Objective damage integration
6. Debug/profiling

명시 승인 없이 여러 단계를 한 번에 합치지 않습니다.

## 완료 기준

각 단계는 실제 검증 결과를 남겨야 합니다.

- build/compile
- PIE 또는 server runtime
- dedicated server log
- client log
- debug output
- runtime verification evidence

실행하지 않은 검증은 성공으로 기록하지 않습니다.
