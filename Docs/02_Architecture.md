# 아키텍처

## 개요

`MDSProject` v2의 핵심 구조는 Dedicated Server 기반 Objective Combat Demo입니다.

```text
Player Attack -> Server Combat Validation -> Enemy HP / Objective HP -> Wave Progress -> Replicated UI / Animation Presentation
```

상세 구조 명세:

```text
Docs/MDS_v2_Structure_Spec.md
```

## 주요 모듈

### Objective

위치:

```text
MDSProject/Source/MDSProject/Objective
```

책임:

- Objective HP 소유
- server authority check
- damage 적용
- `CurrentHealth` replication
- client `OnRep` 처리
- Enemy HP 용도로 재사용하지 않음

### Combat / Wave

위치:

```text
MDSProject/Source/MDSProject
```

설계 기준:

```text
Docs/Combat_Baseline_Design.md
```

책임:

- server-authoritative attack validation
- enemy HP 변경
- Objective damage 연결
- `AMDSProjectGameMode`에서 Wave authority 소유
- `AMDSProjectGameState`에서 replicated Wave display state 소유
- replicated combat state를 UI와 animation presentation에 전달

### Character / Animation

위치:

```text
MDSProject/Source/MDSProject
MDSProject/Content/Characters
```

책임:

- CharacterMovementComponent 기반 이동
- Skeletal Mesh character 구성
- AnimBP State Machine 기준 locomotion
- Attack Montage playback
- AnimNotify timing marker 처리
- server-confirmed damage 이후 Hit Reaction 표시
- replicated HP 기준 death 이후 Death Animation 표시

### MassAI Future Extension

위치:

```text
MDSProject/Source/MDSProject/MassAI
```

책임:

- v2 MVP 필수 구현이 아닌 future extension/reference
- Mass fragments/tags 정의
- entity spawn/movement/arrival 실험
- scalable AI-style simulation 비교

### ActorAI

위치:

```text
MDSProject/Source/MDSProject/ActorAI
```

책임:

- v2 MVP enemy 또는 기존 Actor baseline 제공
- server-authoritative combat target 역할
- Mass future extension과 비교 가능한 reference 역할
- v2 replicated enemy HP/death 구조는 별도 actor/component로 분리 가능
- MVP death state는 별도 replicated `bIsDead`가 아니라 `CurrentHealth <= 0.0f`에서 파생

### Debug

위치:

```text
MDSProject/Source/MDSProject/Debug
```

책임:

- runtime state 집계
- NetMode, Role, Wave, Objective HP, enemy state, combat/animation state 출력

### UI

위치:

```text
MDSProject/Source/MDSProject/UI
```

책임:

- Match HUD는 `AMDSProjectGameState`의 replicated Wave state 표시
- Objective World UI는 `AMDSObjectiveActor`의 replicated Objective HP 표시
- Enemy World UI는 combat enemy actor의 replicated Enemy HP 표시
- CommonUI 기반 debug overlay 골격 제공
- `UMDSDebugStateSubsystem` snapshot을 UI 표시용 text로 변환
- gameplay authority와 분리된 client presentation 경로 유지
- Debug Overlay와 gameplay UI 책임 분리
- Widget Blueprint presentation은 `Docs/UI_Widget_Blueprint_Guide.md` 절차를 따름

### Level Content

위치:

```text
MDSProject/Content
```

책임:

- v2 Objective Combat Demo 검증 맵 제공
- Objective를 맵 중앙에 배치
- PlayerStart를 Objective 근처에 배치
- North/South/East/West 4방향 enemy spawn area 제공
- Objective 주변 combat space 제공
- Match HUD, Objective World UI, Enemy World UI 검증 가능한 시야 제공

기준 맵 후보:

```text
/Game/MDS/Maps/L_MDS_ObjectiveCombat
```

기존 `/Game/TopDown/Lvl_TopDown`은 prototype/reference verification map으로 유지합니다.

### Profiling

위치:

```text
MDSProject/Source/MDSProject/Profiling
```

책임:

- v2 MVP 필수 경로가 아닌 future extension/reference
- phase-based CSV capture
- `MovementActive` / `ArrivalsComplete` trigger
- Actor vs Mass 비교 조건 제어
- Runtime Review / Verification Evidence를 대체하지 않음

## 서버 권한 경계

`AMDSObjectiveActor::ApplyObjectiveDamage`가 Objective HP 변경의 권한 경계입니다.

클라이언트는 Objective HP를 직접 변경하지 않습니다. 클라이언트는 replicated `CurrentHealth`를 관찰합니다.

## Dedicated Server

Dedicated server는 gameplay state를 소유하고 client presentation에 의존하지 않습니다.

검증은 server log, client log, smoke script로 수행합니다.
