# 아키텍처

## 개요

`MDSProject`의 핵심 구조는 server-authoritative Objective와 Mass Entity 기반 simulation입니다.

```text
Mass Spawn -> Mass Movement -> Mass Arrival -> Objective Damage -> Replicated Objective HP
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

### MassAI

위치:

```text
MDSProject/Source/MDSProject/MassAI
```

책임:

- Mass fragments/tags 정의
- entity spawn
- movement processing
- arrival detection
- once-only objective damage trigger

### ActorAI

위치:

```text
MDSProject/Source/MDSProject/ActorAI
```

책임:

- Actor baseline 제공
- Actor tick 기반 movement/damage 비교 대상

### Debug

위치:

```text
MDSProject/Source/MDSProject/Debug
```

책임:

- runtime state 집계
- NetMode, Objective HP, Mass count, Actor count 출력

### Profiling

위치:

```text
MDSProject/Source/MDSProject/Profiling
```

책임:

- phase-based CSV capture
- `MovementActive` / `ArrivalsComplete` trigger
- Actor vs Mass 비교 조건 제어

## 서버 권한 경계

`AMDSObjectiveActor::ApplyObjectiveDamage`가 Objective HP 변경의 권한 경계입니다.

클라이언트는 Objective HP를 직접 변경하지 않습니다. 클라이언트는 replicated `CurrentHealth`를 관찰합니다.

## Dedicated Server

Dedicated server는 gameplay state를 소유하고 client presentation에 의존하지 않습니다.

검증은 server log, client log, smoke script로 수행합니다.
