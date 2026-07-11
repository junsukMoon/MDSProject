# Interview Summary

이 문서는 MDS v2를 면접에서 짧게 설명하기 위한 요약입니다.

## 30초 설명

MDS v2는 Dedicated Server 환경에서 동작하는 Objective Combat Demo입니다. 서버가 전투 판정, Enemy HP, Objective HP, Wave 진행을 소유하고, 클라이언트는 replicated state를 기반으로 UI와 animation presentation을 갱신합니다. 캐릭터 레이어는 CMC 이동, Skeletal Mesh, AnimBP State Machine, Attack Montage, AnimNotify timing, Hit Reaction, Death Animation을 최소 범위로 포함합니다. Mover, Motion Matching, Mutable, Mass Entity는 MVP 구현이 아니라 future extension으로 문서화했습니다.

## Core Flow

```text
Player input
-> optional local presentation
-> server attack validation
-> server updates Enemy HP / Objective HP / Wave state
-> replicated state reaches clients
-> UI and animation presentation update
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
- CMC-based movement
- Skeletal Mesh character
- AnimBP State Machine
- Attack Montage and AnimNotify timing
- Hit Reaction and Death Animation
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

## Runtime Evidence

MVP completion should be supported by runtime correctness evidence, not performance claims.

Evidence to collect:

- Dedicated Server and client launch result
- server combat validation log
- Objective HP before/after and client replicated observation
- Wave index / EnemiesRemaining replication
- Enemy HP/death presentation
- Match HUD / Objective World UI / Enemy World UI screenshot or short video
- negative test showing AnimNotify or client-only event cannot apply authoritative damage
- Debug Overlay showing state as a snapshot, not gameplay truth

Current evidence document:

- `Docs/11_Runtime_Review_Evidence.md`

## Next Implementation Direction

Recommended implementation order:

1. Confirm existing `AMDSObjectiveActor` authority and `CurrentHealth` replication.
2. Add minimal GameState replicated Wave display state.
3. Add minimal combat enemy actor with server-owned HP.
4. Connect server-side combat validation and Objective damage.
5. Add replicated UI presentation for Match HUD, Objective HP, and Enemy HP.
6. Add minimal animation presentation hooks for attack, hit reaction, and death.
7. Capture runtime review evidence on Dedicated Server and client.
