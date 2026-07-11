# Scope Constraints

이 문서는 `MDSProject` v2의 MVP 범위와 제외 범위를 정의합니다.

## MVP Core

MDS v2는 Dedicated Server 환경에서 동작하는 Objective Combat Demo입니다.

MVP는 다음 흐름을 우선합니다.

```text
Server-authoritative combat -> Enemy HP / Objective HP -> Wave progress -> Replicated UI / presentation
```

상세 구조 명세는 다음 문서를 기준으로 합니다.

```text
Docs/MDS_v2_Structure_Spec.md
```

## MVP Includes

- Dedicated Server runtime path
- server-authoritative combat validation and damage application
- server-owned enemy HP
- server-owned Objective HP
- Objective defense loop
- Wave progression
- client UI driven by replicated state
- minimal level content required to demonstrate the loop
- CMC-based character movement
- Skeletal Mesh player character
- basic Animation Blueprint State Machine
- Idle / Walk / Run locomotion states
- Attack Montage
- AnimNotify timing for visual/combat sync markers
- Hit Reaction
- Death Animation
- server-authoritative combat/animation synchronization rules
- optional simple modular character mesh slots such as Head / Body / Weapon
- Runtime Review / Verification Evidence for the dedicated server combat loop

## MVP Excludes

- formal performance profiling as a required MVP deliverable
- Actor vs Mass benchmark requirement
- CSV profiling requirement
- Unreal Insights trace requirement
- large enemy count stress test
- full Mover migration
- custom network prediction movement system
- production Motion Matching implementation
- full Pose Search database tuning
- full Mutable character customization pipeline
- complex character creator UI
- advanced skeletal mesh merge/cook pipeline
- Mass Entity implementation as a required v2 MVP feature
- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Large UI framework
- Full GAS expansion
- full production-quality game content

## Future Extensions

The following systems are documented as future extension topics, not required MVP implementation.

- Mover
- Motion Matching
- Mutable
- Mass Entity
- Actor vs Mass profiling
- CSV / Unreal Insights profiling

## Authority Constraint

Animation events are presentation and synchronization aids. AnimNotify or client-side montage events must not directly apply authoritative damage.

Damage, enemy HP, Objective HP, and Wave progression remain server-owned. Clients may request actions and play predicted or replicated presentation, but the server validates and applies gameplay state changes.
