# MDSProject

`MDSProject`는 UE5 기술 포트폴리오용 멀티플레이어 방어 샌드박스입니다.

목표는 완성형 게임을 만드는 것이 아니라, 면접에서 설명 가능한 기술 데모를 만드는 것입니다.

핵심 주제:

- Dedicated Server
- Replication
- Authority / Ownership
- Objective gameplay
- Server-authoritative combat
- Wave progression
- Character Movement / Animation baseline
- Debug output
- Runtime Review / Verification Evidence
- AI-assisted development workflow

## Interview Demo

The list below describes the target interview demo flow. The currently verified subset is called out separately in `Current Evidence Status`.

이 프로젝트의 핵심 데모는 다음 흐름입니다.

1. Dedicated server가 전투 판정, 적 HP, Objective HP, Wave 진행을 소유합니다.
2. 플레이어는 CMC 기반 Skeletal Mesh 캐릭터로 Objective를 방어합니다.
3. Attack Montage와 AnimNotify timing은 전투 연출과 타이밍 표시를 담당합니다.
4. 실제 damage 적용은 서버가 검증하고 처리합니다.
5. 클라이언트는 replicated state를 기반으로 UI, Hit Reaction, Death Animation을 갱신합니다.

가장 짧은 설명:

```text
Dedicated server owns Objective HP, combat enemy HP, Objective arrival damage, and Wave display state. Clients observe replicated state and update debug/gameplay UI. Character movement, Skeletal Mesh, AnimBP, Montage/AnimNotify, Hit Reaction, and Death Animation remain target presentation work until separately implemented and verified.
```

Current implementation note: the verified runtime path is a technical Objective Combat baseline with server-owned Objective HP, minimal replicated combat enemies, Wave display replication, and replicated UI evidence. It is not yet a complete player character or animation presentation demo.

## MDS v2 Positioning

Current verified scope is the server-authoritative Objective/Enemy/Wave state baseline plus replicated UI evidence. Character movement and animation presentation remain planned presentation work until separate runtime evidence is captured.

MDS v2는 Dedicated Server 환경에서 동작하는 Objective Combat Demo입니다.

서버 권한으로 전투 판정, 적 HP, Objective HP, Wave 진행을 처리하고, 클라이언트는 Replication된 상태를 기반으로 UI와 연출을 갱신하는 구조입니다.

캐릭터 구현 기본기인 CMC 이동, Skeletal Mesh, AnimBP State Machine, Attack Montage, AnimNotify 타이밍, Hit Reaction, Death Animation은 후속 presentation/evidence 범위로 남겨 둡니다.

Mover, Motion Matching, Mutable, Mass Entity는 MVP에 직접 구현하지 않고 추후 확장 가능한 기술 항목으로 문서화합니다.

## 현재 상태

기존 구현 및 검증 완료 항목:

- UE 5.8 source engine 기준 server/client target build
- Dedicated Server target
- WindowsServer cook/stage/runtime workflow
- 서버 권한 기반 Objective Actor
- replicated Objective HP
- minimal replicated combat enemy actor with server-owned HP
- server-side actor enemy spawn baseline
- server-side enemy approach and once-only Objective arrival damage
- GameState Wave display state replication
- Match HUD, Objective World UI, and Enemy World UI replicated-state presentation
- actor-attached Objective/Enemy World UI viewport evidence
- owning-client player attack intent, server validation, Enemy HP replication, and negative reject log evidence
- runtime debug state subsystem
- visible two-client Objective HP verification
- smoke verification script

Verified runtime evidence:

- Dedicated Server/client smoke verification: `SMOKE RESULT: PASS`
- Wave display state verification: `WAVE VERIFY RESULT: PASS`
- Debug Overlay viewport verification: `DEBUG OVERLAY VIEWPORT VERIFY RESULT: PASS`
- Replicated UI baseline verification: `REPLICATED UI BASELINE VERIFY RESULT: PASS`
- Replicated UI viewport verification: `REPLICATED UI VIEWPORT VERIFY RESULT: PASS`
- Player attack runtime verification: `PLAYER ATTACK VERIFY RESULT: PASS`
- Actor-following Objective/Enemy World UI evidence is recorded in `Docs/11_Runtime_Review_Evidence.md`.

v2에서 재정의할 MVP 항목:

The items below remain implementation or verification gaps, not completed runtime evidence:

- CMC-based player movement
- Skeletal Mesh character
- AnimBP State Machine
- Attack Montage / AnimNotify timing
- Hit Reaction / Death Animation
- authored Widget Blueprint visual polish
- Enemy death visual/animation presentation runtime evidence
- Attack Montage / AnimNotify negative test
- Hit Reaction / Death Animation runtime evidence

기존 Mass 실험은 v2 MVP 필수 구현이 아니라 future extension/reference 항목으로 유지합니다.

## 런타임 시나리오

플레이어는 Dedicated Server 환경에서 Objective를 방어합니다.

서버는 attack validation, enemy HP, Objective HP, Wave 진행을 처리합니다. 클라이언트는 복제된 결과를 기반으로 UI와 animation presentation을 갱신합니다.

서버 최종 상태 예시:

```text
MDS Debug | NetMode=DedicatedServer | Wave=2 | ObjectiveHP=70/100 | EnemyAlive=3 | DamageApplied=12
```

클라이언트 관찰 상태 예시:

```text
MDS Debug | NetMode=Client | Wave=2 | ObjectiveHP=70/100 | EnemyAlive=3 | LocalAnim=Attack
```

## 아키텍처 요약

서버 권한 경계:

- `AMDSObjectiveActor::ApplyObjectiveDamage`
- `CurrentHealth`는 `ReplicatedUsing=OnRep_CurrentHealth`
- 클라이언트는 Objective HP를 직접 수정하지 않음
- attack montage 또는 client-side AnimNotify는 authoritative damage를 직접 적용하지 않음

MDS v2 MVP 흐름:

1. CMC 기반 player movement
2. server-authoritative attack validation
3. enemy HP / Objective HP update on server
4. replicated state update
5. UI / hit / death presentation update on clients

Current verified flow:

1. server-authoritative Objective and combat enemy state
2. enemy HP / Objective HP update on server
3. Wave display state update on server
4. replicated state update
5. gameplay/debug UI presentation update on clients

Player movement and animation presentation remain separate future evidence items.

Debug / runtime evidence:

- `UMDSDebugStateSubsystem`
- `Run_Smoke_DedicatedServer_WithClient.ps1`
- `Run_Verify_WaveDisplayState.ps1`
- `Docs/11_Runtime_Review_Evidence.md`
- `UMDSGameplayProfileSubsystem`은 future profiling/reference 용도로 유지

## 주요 코드

```text
MDSProject/Source/MDSProject/Objective
MDSProject/Source/MDSProject/ActorAI
MDSProject/Source/MDSProject/Debug
MDSProject/Source/MDSProject/Profiling
```

## Dedicated Server 검증

검증 스크립트:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Smoke_DedicatedServer_WithClient.ps1
```

최근 검증 결과:

```text
SMOKE RESULT: PASS
```

검증 내용:

- server binary 실행
- UDP `7777` listen
- staged client 접속
- server final Objective/Mass state 확인
- client replicated Objective HP 확인

## Current Evidence Status

Verified:

- Dedicated Server/client launch
- Objective HP replication
- Wave display state replication
- Debug Overlay runtime and viewport visibility
- Match HUD / Objective World UI / Enemy World UI replicated-state reads
- Objective/Enemy World UI actor-following viewport placement
- Player attack server validation, Enemy HP replication, HP-derived enemy death, and Wave remaining decrement
- Player attack negative validation for OutOfRange and Cooldown rejects

Not yet verified:

- authored Widget Blueprint visual polish
- Enemy death visual/animation presentation
- Attack Montage / AnimNotify negative test
- Hit Reaction and Death Animation runtime presentation

## Runtime Review / Profiling Reference

v2 MVP의 핵심 검증 대상은 성능 수치가 아니라 Dedicated Server Objective Combat loop가 의도대로 동작하는지 보여주는 runtime evidence입니다.

MVP 검증 증거:

- Dedicated Server 실행 결과
- server/client log review
- Objective HP replication 확인
- Wave state replication 확인
- Enemy HP/death presentation 확인
- Match HUD / Objective World UI / Enemy World UI 확인
- Attack Montage / AnimNotify가 authoritative damage를 직접 만들지 않는 negative test
- Debug Overlay가 gameplay truth가 아님을 확인

기존 Actor/Mass baseline 비교는 future extension/reference 자료로 유지합니다.

주의:

- `-NullRHI` 결과는 로컬 CPU/gameplay 비교용입니다.
- viewport/GPU 최종 성능 주장으로 해석하면 안 됩니다.
- Unreal Insights trace는 smoke capture입니다.
- Mass profiling 결과는 v2 MVP 필수 성능 주장으로 사용하지 않습니다.

상세 내용:

```text
Docs/08_Profiling_Comparison.md
Docs/10_Visible_Demo_Verification.md
Docs/11_Runtime_Review_Evidence.md
```

## 문서

```text
Docs/01_Scope_Constraints.md
Docs/Character_Movement_Animation_Readiness.md
Docs/Animation_MotionMatching_Notes.md
Docs/Character_Customization_Notes.md
Docs/Coding_Standards.md
Docs/Unreal_Rules.md
Docs/Verification.md
Docs/Interview_Summary.md
Docs/AI_Harness.md
Docs/08_Profiling_Comparison.md
Docs/10_Visible_Demo_Verification.md
Docs/11_Runtime_Review_Evidence.md
```

## 제외 범위

이 프로젝트는 기술 데모이므로 다음 시스템은 의도적으로 제외합니다.

- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Large UI framework
- Complex animation system
- Full GAS expansion
- Full production-quality game content
- Full Mover migration
- Production Motion Matching implementation
- Full Mutable character customization pipeline
- Mass Entity as a required v2 MVP feature
