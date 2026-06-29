# Verification Guide

이 문서는 `MDSProject`의 검증 기준을 정의합니다.

검증은 정확하고 구체적이어야 합니다. 실행하지 않은 build, compile, PIE, dedicated server run, log check, profiling pass를 성공했다고 보고하지 않습니다.

## Build / Compile Checks

C++, module, plugin, config, Unreal API 변경에는 build 또는 compile check를 사용합니다.

보고 항목:

- target
- configuration
- platform
- result
- relevant errors/warnings

`.Build.cs`가 바뀌면 각 module이 왜 필요한지 설명합니다.

## Editor Startup Checks

asset, config, module, plugin, game mode, map, subsystem에 영향을 줄 수 있으면 editor startup check를 사용합니다.

보고 항목:

- editor 실행 여부
- project/map load 상태
- startup warning/error/crash
- 현재 변경과 관련 있는 문제인지

## PIE Single-Player Checks

local gameplay flow, objective behavior, UI visibility, input handling, basic runtime error 확인에 사용합니다.

PIE single-player는 multiplayer authority/ownership/replication 검증을 대체하지 않습니다.

## PIE Listen-Server Checks

editor-hosted server와 client로 multiplayer behavior를 확인할 때 사용합니다.

보고 항목:

- player 수
- server instance
- client instance
- server-observed result
- client-observed result
- ownership/RPC/replication notes

## Dedicated Server Checks

dedicated server support, standalone client behavior, Objective state, replicated data 검증에 사용합니다.

보고 항목:

- server target 또는 launch method
- client launch method
- map/test context
- server log result
- client log result
- observed gameplay result

## Network Replication Checks

replicated property, RPC, authority check, ownership, damage, health, score, Objective HP, spawning, possession, client-visible gameplay state 변경에 사용합니다.

보고 항목:

- server authority path
- client request path
- client에서 관찰한 replicated data
- RPC ownership assumption
- lifetime replicated properties
- server/client result notes

## Objective Gameplay Checks

Objective HP, score, enemy arrival, win/loss, damage application, defense goal 변경에 사용합니다.

보고 항목:

- initial objective state
- state를 바꾸는 action
- server-side result
- client-visible result
- expected/observed final state

Objective gameplay state는 server-owned입니다.

## Mass Entity Checks

Mass spawn, movement, arrival detection, objective damage integration, debug visualization, profiling에 사용합니다.

보고 항목:

- entity count
- spawn behavior
- movement / processor behavior
- arrival / damage behavior
- performance impact
- Mass warnings/errors

실제로 테스트하지 않은 behavior를 verified로 적지 않습니다.

## Debug UI Checks

runtime status display, counters, overlays, logs, developer feedback 변경에 사용합니다.

보고 항목:

- display location
- 표시 값
- 값 생성 방식
- runtime update 여부
- server/client visibility

## Log Review

build, editor startup, PIE, dedicated server, client run, Mass warnings, replication warnings, crash/ensure 확인에 사용합니다.

보고 항목:

- log source
- relevant warnings
- relevant errors
- crash/ensure
- 기존 문제인지 신규 문제인지

## Profiling Checks

performance, Tick cost, Mass processing, entity/actor count, debug UI overhead, spawning, movement, server load 변경에 사용합니다.

보고 항목:

- FPS
- frame time
- GameThread impact
- entity/actor count
- map/scenario
- runtime mode

Profiling number는 context와 함께 기록해야 합니다.

## Manual Test Steps

manual test step은 재현 가능해야 합니다.

포함 항목:

1. setup 또는 map
2. player/server mode
3. 실행 action
4. expected result
5. observed result

manual inspection과 runtime check를 구분해서 기록합니다.

## 검증을 실행할 수 없을 때

다음을 보고합니다.

- 어떤 검증을 실행하지 못했는가
- 이유
- 대신 확인한 것
- 남은 unverified 항목
- 추천 manual check

manual inspection을 runtime verification처럼 표현하지 않습니다.
