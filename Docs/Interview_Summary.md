# Interview Summary

이 문서는 MDS v2를 면접에서 짧게 설명하기 위한 요약입니다.

## 30초 설명

MDS v2는 Dedicated Server 환경에서 동작하는 Objective Combat Demo입니다. 서버가 전투 판정, Enemy HP, Objective HP, Wave 진행을 소유하고, 클라이언트는 replicated state를 기반으로 UI와 combat presentation을 갱신합니다. CMC 이동은 Authority/AutonomousProxy/SimulatedProxy에서 복제를 확인했고 persistent authored montage Notify도 staged client에서 검증했습니다. 프레임 단위 pose 변화는 후속 evidence 범위입니다. Mover, Motion Matching, Mutable, Mass Entity는 MVP 구현이 아니라 future extension으로 문서화했습니다.

Current evidence note: the verified runtime path covers Dedicated Server Objective/Enemy/Wave state, owning-client player attack intent, server attack validation, Enemy HP replication, replicated UI presentation, persistent authored AnimNotify firing, simulated-client attack presentation, and paired Attack/Hit/Death viewport pose deltas.

Presentation hook note: existing attack, hit reaction, and death assets have loadability, playback, persistent Notify, simulated-client, and paired viewport pose-delta evidence. Pixel deltas prove rendered change, not artistic quality.

## Core Flow

```text
Player input
-> optional local presentation
-> server attack validation
-> server updates Enemy HP / Objective HP / Wave state
-> replicated state reaches clients
-> gameplay/debug UI presentation update
-> client-only animation presentation update after replicated state / local intent
```

## Authority Boundary

- 서버가 combat validation, damage, Enemy HP, Objective HP, Wave progression을 소유합니다.
- 클라이언트 request는 gameplay intent이며 damage result가 아닙니다.
- UI, AnimNotify, client-side montage event는 authoritative damage를 직접 적용하지 않습니다.
- Objective HP는 `AMDSObjectiveActor`의 replicated `CurrentHealth` 경로를 기준으로 합니다.
- Enemy death presentation은 MVP에서 replicated HP가 `0` 이하인지로 파생합니다.

## MVP Includes

- Dedicated Server runtime path
- server-authoritative combat
- Objective defense loop
- Enemy HP / Objective HP separation
- Wave display state replication
- Match HUD, Objective World UI, Enemy World UI
- Runtime Review / Verification Evidence

## MVP Excludes

- formal performance profiling requirement
- Actor vs Mass benchmark requirement
- full Mover migration
- production Motion Matching implementation
- full Mutable customization pipeline
- Mass Entity implementation as required MVP scope
- inventory, quests, save system, lobby, matchmaking, skill tree
- full production-quality game content

## Future Extensions

- Mover: future movement-system comparison or migration topic
- Motion Matching: future locomotion replacement for State Machine/BlendSpace layer
- Mutable: future character customization pipeline topic
- Mass Entity: future scaling comparison and Actor vs Mass profiling topic

## Verified Runtime Evidence

- Dedicated Server/client launch and smoke verification.
- Objective HP replication observed on clients.
- Wave display state replication observed through `AMDSProjectGameState`.
- Debug Overlay runtime and viewport visibility verified as a debug snapshot, not gameplay truth.
- Match HUD reads replicated GameState Wave values.
- Objective World UI reads replicated `AMDSObjectiveActor` HP.
- Enemy World UI reads replicated `AMDSCombatEnemyActor` HP for multiple spawned combat enemies.
- Objective/Enemy World UI labels are actor-attached and verified in a staged viewport with separated actor-following labels.
- Left-mouse directional fire intent from the owning client reaches the server RPC path while WASD movement continues independently.
- The server owns directional range/hit-radius evaluation. Enemy-direction shots apply server-owned damage; empty-space shots resolve as valid misses with zero damage.
- Valid directional-fire damage replicates Enemy HP to the client, including HP-derived enemy death and Wave remaining decrement.
- Runtime checks verify valid misses do not apply damage and Cooldown rejects do not apply extra damage.
- Dedicated Server checks also verify InvalidDirection, InvalidDamage, and NoPawn requests are rejected exactly once with no valid resolution, damage, death, or Wave decrement.
- C++ combat presentation hooks can be triggered from local attack intent and replicated Enemy HP observation without mutating server-owned damage state.
- Existing attack, hit reaction, and death animation assets are accepted by staged-client playback APIs.
- Event-timed visible client screenshots are captured for attack, hit, and death playback moments with pose-proof limitations documented.
- One persistent authored `UMDSCombatTimingAnimNotify` fires through the staged-client montage path while the Dedicated Server observes zero callbacks; the presentation-only scenario sends no attack request and applies zero damage.
- Paired pre-combat/playback captures verify non-zero Attack/Hit/Death pose deltas (`75 / 58 / 803` changed center-region samples).
- Authored Widget Blueprint TextBlocks use a cyan Match HUD, gold Objective HP, and red Enemy HP treatment while preserving replicated-state data sources and actor-attached placement.

## Evidence Status

- Dedicated-server CMC movement replication is verified (`Authority` / `AutonomousProxy` / `SimulatedProxy`, maximum distance/speed `1620.5 / 600` on all three views).
- `BP_TopDownCharacter` uses `AMDSProjectCharacter`; normalized W/A/S/D input feeds the shared CMC movement path without breaking combat regressions.
- Physical WASD, diagonal movement, and observer-visible locomotion are verified in a visible Dedicated Server + two-client session.
- Normal facing follows movement direction. Fire temporarily faces the cursor for the attack montage duration, then returns to movement-direction facing; mover and observer presentation were manually confirmed.
- The observer client's `SimulatedProxy` receives the server-confirmed fire direction and successfully plays the remote attack montage. Four attacks produced four remote receipts/playbacks with zero owner duplicate and zero Dedicated Server animation playback.

Remaining optional evidence/polish:

- Production-level panel/art polish remains optional; the authored high-contrast TextBlock styling is viewport-verified.
- Paired pre-combat and playback-frame viewport captures verify non-zero Attack/Hit/Death pose deltas; artistic quality review remains optional.

## Runtime Evidence

MVP completion should be supported by runtime correctness evidence, not performance claims.

Evidence to collect:

- Dedicated Server and client launch result
- server combat validation log
- Objective HP before/after and client replicated observation
- Wave index / EnemiesRemaining replication
- Enemy death presentation
- Match HUD / Objective World UI / Enemy World UI screenshot or short video
- negative test showing AnimNotify or client-only event cannot apply authoritative damage
- Debug Overlay showing state as a snapshot, not gameplay truth

Current evidence document:

- `Docs/11_Runtime_Review_Evidence.md`

## Remaining Implementation Direction

Completed baseline order:

1. Confirm existing `AMDSObjectiveActor` authority and `CurrentHealth` replication.
2. Add minimal GameState replicated Wave display state.
3. Add minimal combat enemy actor with server-owned HP.
4. Connect server-side combat validation and Objective damage.
5. Add replicated UI presentation for Match HUD, Objective HP, and Enemy HP.
6. Add owning-client player attack request, server validation, Enemy HP replication, and valid/negative runtime evidence.
7. Add combat animation presentation hooks, existing asset playback attempts, and event-timed visible capture evidence.

Remaining optional implementation/evidence order:

1. Add production-level Widget Blueprint panels/art only if additional visual polish is needed.
2. Capture a short portfolio video or manually review animation quality if additional presentation polish is needed.
