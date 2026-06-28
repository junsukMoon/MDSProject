# 프로젝트 목표

`MDSProject`는 UE5 기술 포트폴리오 프로젝트입니다.

목표는 완성형 게임을 만드는 것이 아니라, 면접에서 설명 가능한 **서버 권위 기반 멀티플레이어 디펜스 샌드박스**를 만드는 것입니다.

## 핵심 목표

- Dedicated Server 실행 가능성 증명
- 서버 권위 기반 Objective HP
- Replication과 client 관찰 흐름 검증
- Mass Entity 기반 enemy simulation
- Debug output으로 runtime state 확인
- Actor baseline과 Mass 비교 profiling
- AI-assisted development workflow 기록

## 범위

이 프로젝트는 작은 기술 데모입니다.

포함 범위:

- Objective Actor와 server-owned HP
- Mass enemy spawn / movement / arrival / objective damage
- client가 replicated Objective HP를 관찰하는 흐름
- runtime debug line
- profiling / verification 문서
- dedicated server build/cook/stage/runtime 검증

비범위:

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

## 면접 가치

이 프로젝트는 다음 질문에 답할 수 있어야 합니다.

- 왜 Objective HP를 서버가 소유해야 하는가?
- client는 어떤 상태를 관찰하고, 어떤 상태를 직접 바꾸면 안 되는가?
- Mass Entity를 actor-only 방식과 비교할 때 어떤 장점과 tradeoff가 있는가?
- spawn, movement, arrival, damage를 왜 나누어 구현했는가?
- dedicated server에서 visual/debug system에 의존하지 않는 구조인가?
- profiling 결과를 어디까지 주장할 수 있고, 어디부터는 제한해야 하는가?

## 개발 원칙

- 작은 task 단위로 작업합니다.
- non-trivial 작업은 계획을 먼저 제안하고 승인 후 구현합니다.
- server-authoritative gameplay를 기본값으로 둡니다.
- debug/profiling은 설명을 돕기 위한 도구이며 gameplay correctness의 필수 조건이 되면 안 됩니다.
- 검증하지 않은 build, runtime, replication, profiling 결과는 성공했다고 말하지 않습니다.
