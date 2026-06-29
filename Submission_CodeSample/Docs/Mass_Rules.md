# Mass 규칙

이 문서는 `MDSProject`의 Mass Entity / Mass AI 작업 규칙을 정의합니다.

Mass 작업은 incremental하고 측정 가능해야 하며, 기술 포트폴리오 가치에 집중해야 합니다.

## 목적

Mass Entity는 multiplayer defense sandbox에서 scalable AI-style simulation을 보여주기 위해 사용합니다.

목표는 완전한 enemy system을 한 번에 만드는 것이 아닙니다. controlled spawning, movement, arrival detection, objective interaction, debugging, profiling을 설명 가능하고 검증 가능하게 만드는 것입니다.

## 허용 범위

Mass 작업에 포함될 수 있는 항목:

- concept documentation
- module/build setup
- fragments and tags
- entity spawning
- processor-driven movement
- arrival detection
- server-authoritative objective damage integration
- debug UI/log integration
- profiling and Actor-vs-Mass comparison notes

각 task는 승인된 범위에만 집중합니다.

## 금지 범위

명시 승인 없이 다음을 추가하지 않습니다.

- spawn, movement, arrival detection, damage를 하나의 task에 결합
- full combat AI
- behavior tree replacement
- complex animation integration
- crowd avoidance expansion
- dynamic formations
- large debug UI framework
- broad gameplay refactor
- client-authoritative Mass gameplay state
- technical demo를 넘어서는 production enemy content

## Incremental Task 순서

권장 순서:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI/log integration
8. Profiling comparison

명시 승인 없이 단계를 결합하지 않습니다.

## Mass 개념

### Fragments

- Mass data의 가장 작은 useful unit을 저장합니다.
- data layout은 단순하고 설명 가능해야 합니다.
- Objective HP 같은 server-owned gameplay state를 fragment에 중복 저장하지 않습니다.
- spawn, movement, arrival flag 같은 entity-local state를 저장합니다.

### Tags

- simple state classification에 사용합니다.
- 측정, debug, replication이 필요한 data의 대체물로 사용하지 않습니다.
- tag는 좁고 읽기 쉬워야 합니다.

### Processors

- processor는 하나의 behavior에 집중합니다.
- 명시 승인 없이 spawn, movement, arrival, damage를 한 processor에 섞지 않습니다.
- processor work는 bounded하고 performance-aware해야 합니다.
- frequent update에서 expensive world search를 피합니다.
- debug draw와 profiling-sensitive work는 guard합니다.

### Spawners

- 승인된 scenario/entity type만 spawn합니다.
- entity count와 spawn behavior를 보고합니다.
- spawn-only task에서 movement, arrival, damage를 추가하지 않습니다.
- profiling을 위해 count/enablement가 바뀌어야 하면 CVar 또는 command-line flag를 사용합니다.

### Representation

- representation은 visual feedback이며 gameplay authority가 아닙니다.
- dedicated server logic은 Mass visual representation에 의존하면 안 됩니다.
- client presentation이 objective damage, score, win/loss의 source가 되면 안 됩니다.

## 서버 권한

- gameplay result에 영향을 주는 Mass simulation은 서버가 소유하거나 검증해야 합니다.
- 클라이언트는 Mass-related result를 관찰할 수 있지만 objective damage, score, win/loss의 source가 되면 안 됩니다.
- replicated state는 명확한 server-side source of truth를 가져야 합니다.
- client-only Mass presentation은 gameplay damage를 적용하면 안 됩니다.

## Objective 연동

- Objective HP와 objective damage는 server-owned입니다.
- Mass arrival은 objective damage request의 근거가 될 수 있지만 서버가 실제 결과를 적용합니다.
- Objective damage는 log, debug state, verification evidence로 측정 가능해야 합니다.
- 별도 정의가 없다면 damage는 valid arrival당 한 번만 적용합니다.
- 반복 processing 가능성이 있으면 arrival state와 damage-applied state를 분리합니다.

## Debug와 Profiling

Debug reporting에 포함할 수 있는 항목:

- entity count
- spawn state
- movement state
- arrival state
- objective interaction state
- server/client visibility

Profiling notes에 포함할 항목:

- FPS 또는 frame time
- GameThread impact
- entity/actor count
- map과 scenario context
- runtime mode
- baseline comparison 조건

Debug UI, debug draw, log가 misleading runtime overhead를 만들지 않도록 주의합니다. Profiling run에서는 debug draw를 끌 수 있어야 합니다.

## 검증 Checklist

Mass task마다 실제 실행한 검증만 보고합니다.

- files manually inspected
- build or compile result
- editor startup result
- PIE result
- listen server 또는 dedicated server result
- entity count
- spawn behavior
- movement behavior
- arrival behavior
- objective damage behavior
- performance impact
- relevant logs, warnings, errors

실제로 확인하지 않은 spawn, movement, arrival, damage를 verified로 보고하지 않습니다.

## 면접 설명 포인트

설명할 수 있어야 하는 항목:

- 왜 Mass를 사용했는가
- 왜 작은 단계로 나눴는가
- fragment, tag, processor, spawner, representation을 어떻게 분리했는가
- server-authoritative state는 무엇인가
- client는 무엇을 관찰하고 무엇을 결정하지 않는가
- objective damage는 어떻게 적용되는가
- profiling은 어떤 조건에서 측정됐는가
- profiling 수치가 무엇을 증명하고 무엇을 증명하지 않는가
