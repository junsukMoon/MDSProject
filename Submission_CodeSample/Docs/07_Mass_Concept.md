# Mass Concept

## 목적

Mass Entity는 많은 적을 Actor tick 기반으로 처리하는 방식과 비교하기 위한 data-oriented simulation 예시입니다.

이 프로젝트에서 Mass는 완전한 적 AI 시스템이 아니라 다음을 보여주기 위한 기술 데모입니다.

- entity data 구성
- processor 기반 movement
- arrival detection
- server-authoritative objective damage
- debug/profiling integration

## 구성 요소

### Fragment

entity-local data를 저장합니다.

- spawn index/location
- current location
- target location
- move speed
- arrival distance
- objective damage amount
- arrival/damage flags

### Tag

processor 대상 entity를 구분합니다.

### Spawn Subsystem

world begin play 시 Mass entity를 생성하고 fragment를 초기화합니다.

### Movement Processor

도착하지 않은 entity를 target 방향으로 이동시킵니다.

### Arrival Processor

도착 여부를 감지하고 Objective damage를 한 번만 적용합니다.

## 서버 권한

Mass simulation이 Objective HP에 영향을 주는 경로는 서버에서 실행되어야 합니다.

클라이언트는 Mass 결과를 직접 결정하지 않고 replicated Objective HP를 관찰합니다.

## 검증 포인트

- entity count
- movement state
- arrival count
- damage count
- Objective HP final state
- server/client debug output

## 한계

- crowd avoidance나 formation은 포함하지 않습니다.
- Mass representation은 gameplay authority가 아닙니다.
- client에서 Mass entity 자체를 완전하게 replicate하는 시스템은 아닙니다.
