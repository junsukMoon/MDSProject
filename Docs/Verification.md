# Verification Guide

이 문서는 `MDSProject`의 검증 기준을 정의합니다.

검증은 정확하고 구체적이어야 합니다. 실행하지 않은 build, compile, PIE, dedicated server run, log check, profiling pass를 성공했다고 보고하지 않습니다.

## Build / Compile Checks

C++, module, plugin, config, Unreal API 변경에는 build 또는 compile check를 사용합니다.

보고 항목:

- target
- configuration
- platform
- result
- relevant errors/warnings

`.Build.cs`가 바뀌면 각 module이 왜 필요한지 설명합니다.

## Editor Startup Checks

asset, config, module, plugin, game mode, map, subsystem에 영향을 줄 수 있으면 editor startup check를 사용합니다.

보고 항목:

- editor 실행 여부
- project/map load 상태
- startup warning/error/crash
- 현재 변경과 관련 있는 문제인지

## PIE Single-Player Checks

local gameplay flow, objective behavior, UI visibility, input handling, basic runtime error 확인에 사용합니다.

PIE single-player는 multiplayer authority/ownership/replication 검증을 대체하지 않습니다.

## PIE Listen-Server Checks

editor-hosted server와 client로 multiplayer behavior를 확인할 때 사용합니다.

보고 항목:

- player 수
- server instance
- client instance
- server-observed result
- client-observed result
- ownership/RPC/replication notes

## Dedicated Server Checks

dedicated server support, standalone client behavior, Objective state, replicated data 검증에 사용합니다.

보고 항목:

- server target 또는 launch method
- client launch method
- map/test context
- server log result
- client log result
- observed gameplay result

## Network Replication Checks

replicated property, RPC, authority check, ownership, damage, health, score, Objective HP, spawning, possession, client-visible gameplay state 변경에 사용합니다.

보고 항목:

- server authority path
- client request path
- client에서 관찰한 replicated data
- RPC ownership assumption
- lifetime replicated properties
- server/client result notes

Animation과 combat sync가 포함된 경우 다음도 확인합니다.

- attack montage가 owning client와 simulated client에서 의도한 방식으로 표시되는지
- damage가 server-authoritative path에서만 적용되는지
- Objective damage가 client-only animation event 또는 AnimNotify만으로 발생하지 않는지
- Hit Reaction이 server-confirmed damage 이후 표시되는지
- Death Animation이 replicated HP 기준 death 이후 표시되는지

## Objective Gameplay Checks

Objective HP, score, enemy arrival, win/loss, damage application, defense goal 변경에 사용합니다.

보고 항목:

- initial objective state
- state를 바꾸는 action
- server-side result
- client-visible result
- expected/observed final state

Objective gameplay state는 server-owned입니다.

## Objective Damage Integration Checks

Enemy-to-Objective damage, Objective HP reduction path, Objective UI replication 변경에 사용합니다.

보고 항목:

- Objective actor instance
- damage source
- server validation condition
- enemy/objective range or overlap result
- Objective HP before/after on server
- replicated Objective HP observed on client
- Objective World UI displayed value
- client-only animation/UI event negative test result

필수 확인:

- Objective damage is applied only on the server.
- Objective damage path uses `AMDSObjectiveActor` or its approved server authority function.
- Client, UI, AnimNotify, and montage events do not directly change Objective HP.
- Server rejects invalid range/overlap damage attempts.
- `CurrentHealth` replicates after valid server damage.
- `AMDSCombatEnemyActor` applies Objective damage only after server-side Objective arrival logic.
- each spawned combat enemy applies Objective arrival damage at most once.
- Objective World UI updates from replicated Objective HP only.
- Objective damage does not directly decrement `EnemiesRemaining`.
- Full win/loss flow is not required for MVP Objective damage integration.

## Objective / Enemy HP Checks

Objective HP, Enemy HP, replicated HP, death presentation 변경에 사용합니다.

보고 항목:

- Objective HP owner
- Enemy HP owner
- server damage path
- replicated `CurrentHealth`
- HP before/after
- client-observed HP
- death presentation trigger
- duplicate death handling prevention if implemented

필수 확인:

- `AMDSObjectiveActor` remains Objective-specific.
- Enemy HP does not reuse `AMDSObjectiveActor`.
- Objective HP and Enemy HP are separate paths.
- death is derived from `CurrentHealth <= 0.0f`.
- MVP does not require replicated `bIsDead`.
- client cannot directly change Objective HP or Enemy HP.
- UI and animation read replicated HP as presentation.
- Wave behavior is not verified as part of Phase 2 unless explicitly implemented later.

## Wave Baseline Checks

Wave authority, GameState replication, enemies remaining, wave UI 변경에 사용합니다.

보고 항목:

- Wave authority owner
- replicated Wave state owner
- current wave index
- enemies remaining
- wave active state
- server death handled event source
- server-side remaining count change
- client-observed GameState values
- command-line trigger if used, such as `-MDSAutoStartWave MDSWaveEnemyCount=4`

필수 확인:

- Wave authority is owned by GameMode.
- replicated Wave display state is owned by GameState.
- Wave start spawns `AMDSCombatEnemyActor` through the approved server-side spawn path.
- `EnemiesRemaining` and `TotalEnemiesThisWave` match actual spawned combat enemy count.
- command-line auto-start runs only on server authority and only when explicitly requested.
- clients do not directly change Wave progression.
- Enemy HP calculation is not performed by Wave.
- enemy death handled event decrements remaining count once.
- Wave clear is based on `EnemiesRemaining <= 0`.
- UI reads replicated GameState values.
- existing Actor/Mass baseline systems are not required v2 Wave architecture.

## Replicated UI Baseline Checks

Match HUD, Objective World UI, Enemy World UI, Debug Overlay 변경에 사용합니다.

보고 항목:

- UI type
- data source
- replicated state read
- client-observed value
- gameplay authority boundary
- visibility rule if relevant

필수 확인:

- Match HUD reads replicated Wave state from GameState.
- Objective World UI reads replicated Objective HP from ObjectiveActor.
- Enemy World UI reads replicated Enemy HP from combat enemy actors.
- Enemy World UI can support multiple combat enemies, not only a single target.
- UI does not directly change HP, damage, Wave, or combat state.
- Debug Overlay reads DebugStateSubsystem snapshot.
- Debug Overlay is not gameplay truth.
- Mass debug summary is future/reference debug, not required v2 gameplay UI.
- Actor enemy spawn debug is valid only when it represents spawned `AMDSCombatEnemyActor` instances.

## Level Content Baseline Checks

MVP Objective Combat map, Objective placement, PlayerStart, enemy spawn area, UI sightline 변경에 사용합니다.

보고 항목:

- map name
- Objective location
- PlayerStart location
- North/South/East/West spawn area locations
- combat space around Objective
- Objective World UI visibility
- Enemy World UI visibility
- Match HUD visibility
- dedicated server/client map load result if implemented

필수 확인:

- Objective is placed at map center.
- PlayerStart is near Objective.
- Enemy spawn areas exist in four cardinal directions.
- Enemies approach toward Objective.
- Objective World UI is visible near center.
- Enemy World UI is visible as enemies approach from multiple directions.
- Match HUD can show Wave and EnemiesRemaining.
- Level content remains a minimal verification space, not a production environment.
- Existing TopDown map remains prototype/reference unless explicitly migrated.
- Initial C++ baseline may use Objective-relative North/South/East/West spawn positions before a dedicated map provides authored spawn markers.

## Combat Baseline Checks

server-authoritative combat, attack request, damage validation, enemy HP, combat cue 변경에 사용합니다.

보고 항목:

- requester and owning connection
- server attack request path
- validation checks performed
- target and damageable state
- damage amount
- HP before/after
- replicated result observed by clients
- animation or UI presentation triggered by replicated state

필수 확인:

- client request is intent, not damage result.
- server validates and applies damage.
- enemy HP, Objective HP, and Wave progression remain server-owned.
- Attack Montage and AnimNotify do not directly apply authoritative damage.
- invalid request does not change HP.
- simulated clients update presentation from replicated state.

## Character Movement / Animation Checks

CMC movement, Skeletal Mesh character, Animation Blueprint, montage, AnimNotify, hit reaction, death animation 변경에 사용합니다.

보고 항목:

- movement mode and authority/role state
- owning client movement input result
- server-observed movement state
- simulated client movement presentation
- AnimBP locomotion state
- Attack Montage playback path
- AnimNotify role and timing
- server damage validation path
- Hit Reaction trigger source
- Death Animation trigger source

필수 확인:

- MVP movement는 CharacterMovementComponent 기준입니다.
- attack montage는 owning client와 simulated client에서 의도한 방식으로 표시됩니다.
- server remains authoritative for damage.
- Objective damage cannot be caused by client-only animation events.
- hit reaction appears after server-confirmed damage.
- death animation appears after replicated HP-derived death state.
- movement debug shows expected authority/role state.
- no MVP feature depends on Motion Matching, Mover, Mutable, or Mass Entity.

## Mass Entity Checks

Mass spawn, movement, arrival detection, objective damage integration, debug visualization, profiling에 사용합니다.

MDS v2에서는 Mass Entity를 MVP 필수 구현이 아니라 future extension으로 분류합니다. Mass 작업을 다시 진행할 때만 이 검증을 적용합니다.

보고 항목:

- entity count
- spawn behavior
- movement / processor behavior
- arrival / damage behavior
- performance impact
- Mass warnings/errors

실제로 테스트하지 않은 behavior를 verified로 적지 않습니다.

## Debug UI Checks

runtime status display, counters, overlays, logs, developer feedback 변경에 사용합니다.

보고 항목:

- display location
- 표시 값
- 값 생성 방식
- runtime update 여부
- server/client visibility
- source system for each displayed value
- whether value is gameplay truth or debug snapshot
- v1/reference counter label if Mass/Actor counters are displayed
- negative test result for debug UI gameplay mutation

필수 확인:

- Debug UI reads `UMDSDebugStateSubsystem` snapshot/cache.
- Debug UI is not gameplay truth.
- Debug UI does not directly change HP, damage, Wave, or combat state.
- Gameplay UI and Debug Overlay have separate responsibilities.
- Match HUD reads GameState, not DebugStateSubsystem.
- Objective World UI reads ObjectiveActor replicated HP, not DebugStateSubsystem.
- Enemy World UI reads combat enemy replicated HP, not DebugStateSubsystem.
- Existing Mass debug counters are labeled or understood as future/reference debug.
- Actor spawn counters represent spawned `AMDSCombatEnemyActor` instances only.
- Dedicated Server and client output make NetMode/state differences clear.

## Debug Logging Checks

server combat validation log, Objective damage log, Wave log, replicated state observation log 변경에 사용합니다.

보고 항목:

- log category
- NetMode
- LocalRole / RemoteRole if relevant
- requester / target / damage source
- validation result
- damage amount
- HP before/after
- Wave index / EnemiesRemaining if relevant
- server log vs client observation

필수 확인:

- server log is the authoritative gameplay event record.
- client log is treated as replicated state observation.
- combat validation logs include enough context to explain accepted/rejected damage.
- Objective damage logs include source, validation condition, and Objective HP before/after.
- Wave logs include wave start, remaining count change, and wave clear if implemented.
- logs are not emitted per-frame unless explicitly needed for temporary debugging.

## Log Review

build, editor startup, PIE, dedicated server, client run, Mass warnings, replication warnings, crash/ensure 확인에 사용합니다.

보고 항목:

- log source
- relevant warnings
- relevant errors
- crash/ensure
- 기존 문제인지 신규 문제인지

## Runtime Review / Verification Evidence Checks

MDS v2 MVP 완료 검증, dedicated server runtime review, evidence 정리에 사용합니다.

보고 항목:

- map/scenario
- dedicated server launch method
- client count
- enemy count
- Wave index / EnemiesRemaining
- Objective HP before/after
- Enemy HP/death presentation
- Match HUD observed values
- Objective World UI observed values
- Enemy World UI observed values
- Attack Montage / AnimNotify negative test result
- server log evidence
- client replicated state observation
- screenshot/GIF/video/log snippet path if captured

필수 확인:

- Dedicated Server owns combat, Objective HP, Enemy HP, and Wave state.
- clients observe replicated state and update UI/presentation.
- Objective HP replication is visible on clients.
- Wave state replication is visible on clients.
- Enemy HP/death presentation follows replicated state.
- AnimNotify or client-only montage event does not directly apply authoritative damage.
- Debug Overlay is not gameplay truth.
- runtime evidence includes enough context to reproduce the scenario.

## Future Profiling Checks

performance, Tick cost, Mass processing, entity/actor count, debug UI overhead, spawning, movement, server load 변경에 사용합니다.

MDS v2 MVP에서는 formal profiling을 필수 검증으로 요구하지 않습니다. 이 섹션은 Mass Entity, Actor vs Mass, large enemy count, CSV/Insights 기반 future extension 작업에만 적용합니다.

보고 항목:

- FPS
- frame time
- GameThread impact
- entity/actor count
- map/scenario
- runtime mode

Profiling number는 context와 함께 기록해야 합니다.
Profiling number는 runtime correctness evidence를 대체하지 않습니다.

## Manual Test Steps

manual test step은 재현 가능해야 합니다.

포함 항목:

1. setup 또는 map
2. player/server mode
3. 실행 action
4. expected result
5. observed result

manual inspection과 runtime check를 구분해서 기록합니다.

## 검증을 실행할 수 없을 때

다음을 보고합니다.

- 어떤 검증을 실행하지 못했는가
- 이유
- 대신 확인한 것
- 남은 unverified 항목
- 추천 manual check

manual inspection을 runtime verification처럼 표현하지 않습니다.
