# Unreal Rules

이 문서는 `MDSProject`의 Unreal Engine C++ 및 multiplayer implementation rules를 정의합니다.

server-authoritative gameplay가 기본값입니다. broad refactor는 명시적으로 요청/승인된 경우에만 허용합니다.

## Class Structure

- class는 한 가지 책임에 집중합니다.
- 기존 project convention을 따릅니다.
- 명시적 요청 없이는 file/class/function/variable/folder rename을 하지 않습니다.
- 큰 gameplay class를 수정하기보다 필요한 경우 작은 component/helper를 추가합니다.

## UCLASS / USTRUCT / UENUM

Unreal reflection, serialization, Blueprint exposure, replication, editor integration이 필요할 때만 사용합니다.

규칙:

- reflected type은 최소화합니다.
- 작은 data container에는 `USTRUCT`를 선호합니다.
- stable gameplay state에는 `UENUM`을 사용할 수 있습니다.
- client가 mutable gameplay state를 직접 바꾸게 노출하지 않습니다.

## UPROPERTY / UFUNCTION

`UPROPERTY`는 lifetime tracking, serialization, replication, editor exposure가 필요할 때 사용합니다.

`UFUNCTION`은 reflection, Blueprint access, RPC, delegate, console/editor tooling이 필요할 때 사용합니다.

규칙:

- Blueprint exposure는 의도적으로만 합니다.
- replicated property는 owner, update path, verification plan이 명확해야 합니다.
- public mutable property는 신중하게 사용합니다.

## BeginPlay / Tick

- runtime world/actor 접근은 BeginPlay 이후가 안전합니다.
- BeginPlay order를 가정하지 말고 필요한 조건을 확인합니다.
- Tick은 필요할 때만 추가합니다.
- Tick work는 bounded해야 하며 allocation, expensive search, log spam을 피합니다.
- Tick 관련 변경은 profiling-sensitive로 봅니다.

## Replication

- server가 gameplay state를 소유합니다.
- client는 replicated result를 관찰합니다.
- replicated property는 lifetime replication에 등록해야 합니다.
- replication condition은 의도적으로 사용합니다.
- derived state를 불필요하게 replicate하지 않습니다.
- replication으로 ownership 불명확성을 덮지 않습니다.

## Authority / Ownership

- server-authoritative gameplay가 기본값입니다.
- client는 request를 보낼 수 있지만 서버가 validate/apply해야 합니다.
- health, damage, score, Objective HP는 client-owned가 되면 안 됩니다.
- gameplay state 변경 지점에는 authority check가 필요합니다.

## RPC Usage

- Server RPC는 client request를 서버가 validate/apply해야 할 때 사용합니다.
- Client RPC는 특정 owning client에게 targeted message가 필요할 때만 사용합니다.
- NetMulticast RPC는 sparingly 사용합니다.
- RPC로 replicated state ownership을 우회하지 않습니다.
- high-frequency reliable RPC를 피합니다.

## OnRep Usage

- `OnRep`는 client-side presentation/cache/local reaction에 사용합니다.
- `OnRep`가 authoritative gameplay source가 되면 안 됩니다.
- server-side state change가 replication보다 먼저 발생해야 합니다.
- `OnRep`에서 새 server gameplay decision을 만들지 않습니다.

## Health / Damage / Objective HP

- health, damage, Objective HP는 server-owned gameplay state입니다.
- damage는 gameplay에서 request/trigger되고 서버가 validate/apply합니다.
- client는 replication 또는 approved server-to-client messaging으로 결과를 봅니다.
- Objective damage는 client-only path에서 적용되면 안 됩니다.
- Objective HP에서 파생되는 score/win/loss도 server-owned여야 합니다.

## Dedicated Server

- server logic은 local player, viewport, HUD, input, audio, visual-only system에 의존하면 안 됩니다.
- client-only presentation code는 dedicated server에서 실행되지 않도록 guard합니다.
- logs는 server-side behavior를 진단할 수 있어야 합니다.
- network 변경은 listen-server 또는 dedicated-server verification notes를 포함합니다.

## Build.cs

`.Build.cs` 변경은 각 module 이유를 설명해야 합니다.

- 승인된 task에 필요한 module만 추가합니다.
- runtime/editor dependency를 구분합니다.
- broad dependency set을 speculative하게 추가하지 않습니다.

## Logging

- project-specific log category를 선호합니다.
- authority-sensitive event는 server path에서 useful하게 log합니다.
- per-frame spam을 피합니다.
- actor, role, state, result를 진단할 수 있는 context를 포함합니다.

## Debug-Only Code

- debug UI/draw/profiling helper는 authoritative gameplay logic과 분리합니다.
- debug-only behavior는 필요할 때 guard합니다.
- dedicated server behavior가 visual debug system에 의존하면 안 됩니다.

## Performance-Sensitive Code

다음은 performance-sensitive로 봅니다.

- Tick
- spawning
- Mass processing
- replication frequency
- debug UI updates
- logging

규칙:

- hot path allocation을 피합니다.
- frequent update에서 expensive world search를 피합니다.
- high-frequency reliable RPC를 피합니다.
- 불필요한 replication을 피합니다.
- profiling checks는 필요할 때 FPS, frame time, GameThread impact를 기록합니다.

## Network Change Verification

network/replication 변경 후 보고할 것:

- server-side source of truth
- client request path
- server validation/application result
- client-visible replicated result
- RPC ownership/direction
- `OnRep` behavior
- listen-server 또는 dedicated-server result
- relevant server/client logs

manual inspection만으로 runtime network verification을 대체하지 않습니다.
