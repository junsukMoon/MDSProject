# Unreal 규칙

이 문서는 `MDSProject`의 Unreal Engine C++ 및 multiplayer 구현 규칙을 정의합니다.

기본 원칙은 server-authoritative gameplay입니다. broad refactor는 명시적으로 요청되고 승인된 경우에만 허용합니다.

## 클래스 구조

- 클래스는 하나의 명확한 책임에 집중합니다.
- 새 구조를 만들기 전에 기존 프로젝트 관례를 따릅니다.
- 명시 요청 없이 파일, 클래스, 함수, 변수, 폴더 이름을 바꾸지 않습니다.
- 큰 gameplay class를 수정하기보다 작은 helper, component, subsystem을 우선 고려합니다.
- 기술 포트폴리오 범위를 유지하고 완성형 게임 시스템으로 확장하지 않습니다.

## UCLASS / USTRUCT / UENUM

Unreal reflection은 serialization, editor integration, replication, Blueprint exposure, delegate, tooling이 필요할 때만 사용합니다.

- reflected type은 최소화합니다.
- 작은 data container에는 `USTRUCT`를 선호합니다.
- 안정적인 gameplay state에는 필요 시 `UENUM`을 사용합니다.
- server-owned gameplay state를 client가 직접 수정할 수 있게 노출하지 않습니다.
- Blueprint exposure는 작업에 필요할 때만 추가합니다.

## UPROPERTY / UFUNCTION

- `UPROPERTY`는 lifetime tracking, GC, serialization, editor exposure, replication이 필요할 때 사용합니다.
- `UFUNCTION`은 RPC, delegate, Blueprint, editor/tooling, reflection이 필요할 때 사용합니다.
- replicated property는 owner, update path, verification plan이 명확해야 합니다.
- public mutable property는 신중하게 사용합니다.
- gameplay state 변경은 명시적인 함수로 처리하는 것을 선호합니다.

## BeginPlay / Tick

- constructor에서는 world, player, network, subsystem, runtime actor에 의존하지 않습니다.
- world/actor 접근은 `BeginPlay` 이후가 더 안전합니다.
- `BeginPlay` 순서를 가정하지 않습니다.
- Tick은 필요한 경우에만 추가합니다.
- Tick work는 bounded해야 합니다.
- Tick에서 allocation, expensive search, log spam을 피합니다.
- Tick 변경은 performance-sensitive로 보고 검증합니다.

## Replication

- 서버가 gameplay state를 소유합니다.
- 클라이언트는 replicated result를 관찰합니다.
- replicated property는 `GetLifetimeReplicatedProps`에 등록합니다.
- replication condition은 의도적으로 사용합니다.
- derived state를 불필요하게 replicate하지 않습니다.
- ownership이 불명확한 상태를 replication으로 덮지 않습니다.
- client가 server-owned replicated state를 직접 수정하면 안 됩니다.

## Authority / Ownership

- server-authoritative gameplay가 기본입니다.
- 클라이언트는 request를 보낼 수 있지만 서버가 validate/apply합니다.
- health, damage, score, Objective HP는 client-owned가 되면 안 됩니다.
- gameplay state 변경 지점에는 authority check가 필요합니다.
- RPC나 client/server interaction을 추가할 때 ownership 가정을 명시합니다.

## RPC 사용

- Server RPC는 client request를 서버가 validate/apply해야 할 때만 사용합니다.
- Client RPC는 owning client 대상 메시지에만 사용합니다.
- NetMulticast RPC는 sparingly 사용합니다.
- RPC로 replicated state ownership을 우회하지 않습니다.
- high-frequency reliable RPC를 피합니다.
- durable gameplay outcome은 replicated state를 선호합니다.

## OnRep 사용

- `OnRep`는 client-side presentation, cache update, local reaction 용도로 사용합니다.
- `OnRep`가 authoritative gameplay source가 되면 안 됩니다.
- server-side state change가 client observation보다 먼저 발생해야 합니다.
- `OnRep`에서 server gameplay decision을 만들지 않습니다.

## Health / Damage / Objective HP

- health, damage, Objective HP는 server-owned gameplay state입니다.
- damage는 gameplay에서 request/trigger될 수 있지만 서버가 적용합니다.
- 클라이언트는 replication 또는 승인된 server-to-client messaging으로 결과를 봅니다.
- Objective damage는 client-only path에서 적용되면 안 됩니다.
- Objective HP에서 파생되는 score, win/loss, objective state도 server-owned여야 합니다.

## Dedicated Server

- server logic은 local player, viewport, HUD, input, audio, visual-only system에 의존하면 안 됩니다.
- client-only presentation code는 dedicated server에서 실행되지 않도록 guard합니다.
- log는 server-side behavior를 진단할 수 있어야 합니다.
- network 변경은 listen server 또는 dedicated server verification notes를 포함합니다.
- dedicated server 검증은 launch method, map, server log, client log, observed gameplay result를 보고합니다.

## Build.cs

`.Build.cs` 변경 시 추가 module이 왜 필요한지 설명합니다.

- 승인된 task에 필요한 module만 추가합니다.
- runtime dependency와 editor dependency를 구분합니다.
- broad dependency set을 speculative하게 추가하지 않습니다.
- include coupling을 숨기기 위해 dependency를 추가하지 않습니다.

## Logging

- project-specific log category를 선호합니다.
- authority-sensitive event는 server/client 진단에 도움이 되도록 기록합니다.
- per-frame log spam을 피합니다.
- actor, role/net mode, state, result 같은 context를 포함합니다.
- `Warning`/`Error`는 actionable problem이나 invalid/rejected path에 사용합니다.

## Debug-Only Code

- debug UI, debug draw, profiling helper는 authoritative gameplay logic과 분리합니다.
- debug-only behavior는 필요 시 guard합니다.
- dedicated server behavior가 visual debug system에 의존하면 안 됩니다.
- debug output은 gameplay state를 만들지 않고 상태만 보고해야 합니다.

## 성능 민감 코드

다음은 성능 민감 코드입니다.

- Tick
- spawning
- Mass processing
- replication frequency
- debug UI update
- logging
- profiling harness

규칙:

- hot path allocation을 피합니다.
- frequent update에서 expensive world search를 피합니다.
- high-frequency reliable RPC를 피합니다.
- 불필요한 replication을 피합니다.
- 성능 주장을 할 때는 FPS, frame time, GameThread impact 등 context를 기록합니다.

## Network 변경 검증

network 또는 replication 변경 후 보고할 항목:

- server-side source of truth
- client request path
- server validation/application result
- client-visible replicated result
- RPC ownership/direction
- `OnRep` behavior
- listen server 또는 dedicated server result
- relevant server/client logs

behavior가 바뀌는 network 변경은 manual inspection만으로 runtime verification을 대체하지 않습니다.
