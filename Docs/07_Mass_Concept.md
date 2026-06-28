# Mass Concept

이 문서는 `MDSProject`에서 Mass Entity를 어떤 목적으로 사용할지 정리합니다.

목표는 큰 AI 시스템을 한 번에 만드는 것이 아니라, server-authoritative defense sandbox 안에서 Mass가 spawn, movement, arrival, objective interaction, debug, profiling에 어떻게 쓰이는지 설명 가능하게 만드는 것입니다.

## 왜 Mass를 사용하는가

Mass Entity는 많은 수의 lightweight agent를 data-oriented 방식으로 처리하기 위한 UE 시스템입니다.

이 프로젝트에서 Mass는 다음을 보여줍니다.

- actor-only enemy와 비교되는 scalable simulation
- fragment/tag/processor 기반 data flow
- dedicated server에서도 동작하는 gameplay simulation
- objective gameplay와 server authority를 유지하는 integration
- profiling 비교 근거

## Server Authority

Mass simulation이 gameplay result에 영향을 준다면 서버가 소유하거나 검증해야 합니다.

규칙:

- Objective HP는 Mass fragment에 저장하지 않습니다.
- client는 objective damage를 결정하지 않습니다.
- Mass arrival은 damage request의 근거가 될 수 있지만, 실제 damage application은 server-owned Objective에서 수행합니다.
- client는 replicated outcome을 관찰합니다.

## 주요 Mass 개념

### Entity

Mass Entity는 lightweight enemy agent입니다.

이 프로젝트에서는 full replicated Actor가 아니라 simulation data로 취급합니다.

### Fragment

Fragment는 entity에 붙는 작은 data unit입니다.

예:

- location / target
- movement speed
- arrival state
- damage-applied marker

Objective HP 같은 authoritative gameplay state는 fragment가 아니라 gameplay actor가 소유해야 합니다.

### Tag

Tag는 entity 상태 분류에 사용합니다.

예:

- spawned
- moving
- arrived
- pending-damage

Tag는 authoritative gameplay rule의 대체물이 아닙니다.

### Processor

Processor는 focused pass로 entity를 업데이트합니다.

각 processor는 한 가지 책임에 집중합니다.

- movement
- arrival check
- debug collection
- damage trigger

spawn, movement, arrival, damage를 하나의 processor/task에 섞지 않습니다.

### Spawner

Spawner는 controlled server-side flow에서 entity를 생성합니다.

첫 Mass task는 spawn만 증명하고 movement/damage를 포함하지 않아야 합니다.

### Representation

Representation은 visual feedback을 위한 것입니다.

dedicated server gameplay correctness는 representation에 의존하면 안 됩니다.

## Incremental Task Order

Mass 작업 순서:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI integration
8. Profiling comparison

각 단계는 별도 task branch와 PR로 진행하는 것이 원칙입니다.

## 검증 기준

Mass task는 실제로 실행한 검증만 보고해야 합니다.

확인 항목:

- build / compile
- entity count
- spawn behavior
- movement behavior
- arrival behavior
- objective damage behavior
- server/client visibility
- profiling impact
- relevant logs/warnings/errors

## 면접 설명 포인트

- 왜 Mass를 사용했는가?
- 왜 spawn/movement/arrival/damage를 나누었는가?
- fragment와 tag를 어떻게 구분했는가?
- server authority는 어디에 있는가?
- client는 무엇을 관찰하고 무엇을 결정하지 않는가?
- profiling은 어떤 조건에서 측정했는가?
