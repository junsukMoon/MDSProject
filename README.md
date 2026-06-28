# MDSProject

MDSProject는 UE5 기술 포트폴리오 프로젝트입니다. 서버 권위 기반의 멀티플레이어 디펜스 샌드박스를 통해 Replication, Mass Entity 기반 게임플레이 흐름, 런타임 디버그 출력, 프로파일링 기록, 승인 기반 AI-assisted 개발 워크플로우를 보여주는 것이 목적입니다.

이 프로젝트는 완성형 게임이 아닙니다. 면접에서 설명 가능한 기술 데모에 초점을 맞춰 의도적으로 범위를 제한합니다.

## Interview Demo

이 프로젝트의 첫 데모 목표는 dedicated server에서 Objective HP를 서버 권위로 변경하고, client가 replicated Objective HP를 같은 값으로 관찰하는 흐름을 보여주는 것입니다.

주요 확인 포인트:

- UE 5.8 source engine 기준 dedicated server binary build/cook/stage/runtime 검증
- Server-owned Objective HP와 `ReplicatedUsing=OnRep_CurrentHealth` 기반 client 표시
- Mass enemy spawn, movement, arrival detection, once-only objective damage
- Runtime debug line: `NetMode`, `ObjectiveHP`, Mass count, Actor baseline count
- Actor vs Mass 1000개 `MovementActive` phase-based profiling comparison
- 승인 기반 AI-assisted workflow와 검증 로그 중심의 작업 기록

현재 가장 짧은 설명:

```text
Dedicated server owns Objective HP. Mass entities damage the Objective on arrival. Clients observe the replicated HP. Actor and Mass baselines are profiled under the same MovementActive phase trigger for interview discussion, not final rendering performance claims.
```

## 현재 상태

구현 및 검증 완료:

- UE 5.8 source engine 업그레이드
- Dedicated Server target binary 빌드
- WindowsServer cook/stage
- staged dedicated server runtime 검증
- 서버 권위 기반 Objective HP Actor
- Replicated Objective HP 상태
- Mass Entity spawn-only 단계
- Objective를 향한 Mass 이동
- Mass 도착 감지
- 유효한 도착 시 서버 측 Objective damage 적용
- 런타임 debug state subsystem
- Standalone, editor server-mode, dedicated server binary 검증 로그
- 현재 Mass 시나리오에 대한 profiling comparison 문서
- 도착 후 debug visualization 성능 문제 수정
- Actor enemy baseline 구현 및 debug count 연동
- Actor vs Mass 1000개 phase-based profiling comparison 문서화

알려진 제한:

- Client viewport에서 replicated Objective HP를 직접 확인하는 검증은 아직 별도 단계로 남아 있습니다.
- Unreal Insights trace는 아직 캡처하지 않았습니다.
- `-NullRHI` profiling 결과는 로컬 비교에는 유용하지만, 최종 viewport 또는 GPU 성능을 대표하지 않습니다.

## 핵심 시나리오

Mass 적들이 Top Down 맵에 생성되고, 공유 Objective를 향해 이동합니다.

Mass Entity가 도착하면 서버가 도착을 기록하고 Objective damage를 한 번 적용합니다. Objective HP는 서버가 권위 있게 소유합니다. 런타임 debug output은 현재 NetMode, Objective HP, Mass spawn count, moved count, arrival count, damage count를 노출합니다.

현재 16 entity 시나리오에서 검증한 최종 dedicated server 상태:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

## 기술 초점

- Dedicated Server readiness
- Replication
- Authority / Ownership
- Objective gameplay
- Mass Entity / Mass AI
- Debug UI 및 로그
- Profiling
- AI-assisted development workflow

## 아키텍처 요약

이 프로젝트는 server-authoritative gameplay를 기본 규칙으로 사용합니다.

- Objective 상태는 서버가 소유합니다.
- Objective HP 변경은 서버 측 gameplay logic을 통해서만 적용됩니다.
- Mass processor는 확장 가능한 적 시뮬레이션 단계를 점진적으로 처리합니다.
- Arrival processing은 도착한 entity마다 damage를 한 번만 적용합니다.
- Debug state는 전용 debug subsystem에서 수집하고 출력합니다.

Mass 작업은 검토 가능한 작은 단계로 나눠 진행했습니다.

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection
5. Objective damage integration
6. Debug output
7. Profiling comparison
8. UE 5.8 source dedicated server verification

## Dedicated Server 검증

UE 5.8 source engine 기준으로 실제 dedicated server binary를 빌드하고 staged runtime까지 검증했습니다.

검증 흐름:

1. `MDSProjectServer Win64 Development` 빌드
2. `WindowsServer` cook
3. `Saved/StagedBuilds/WindowsServer` stage
4. staged `MDSProjectServer.exe` 실행
5. server log에서 map load, NetDriver listen, Objective/Mass final state 확인

UE 5.8 기본 cook은 `ZenStore`를 사용합니다. standalone staged server runtime 검증에는 loose cooked content가 필요했기 때문에 `-skipzenstore` cook을 사용했습니다.

검증 로그:

- Cook: `MDSProject/Saved/Logs/Phase10_Cook_WindowsServer_SkipZen.log`
- Runtime: `MDSProject/Saved/Logs/Phase10_StagedDedicatedServer.log`

확인된 runtime 상태:

- `Premade AssetRegistry loaded`
- `World NetMode = Dedicated Server`
- `IpNetDriver listening on port 7777`
- `ObjectiveHP=20/100`
- `Mass Spawned=16`
- `Moved=0`
- `Arrived=16`
- `Damage=16`

## 프로파일링 스냅샷

Mass objective 시나리오는 `-NullRHI`와 benchmark mode로 측정했습니다.

| Scenario | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Standalone Mass scenario, UE 5.6 installed build | `2571.43` | `0.39 ms` |
| Editor server-mode Mass scenario, UE 5.6 installed build | `4500.00` | `0.22 ms` |
| Staged dedicated server binary, UE 5.8 source build | `905.43` | `1.10 ms` |

도착 후 frame slowdown은 debug visualization에서 발견되었습니다. 원인은 entity가 도착한 뒤에도 매 프레임 debug sphere를 반복해서 그리는 것이었습니다. 수정 후 marker는 최초 도착 시 한 번만 그립니다.

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before debug draw fix | `248.76` | `4.02 ms` |
| After debug draw fix | `2163.46` | `0.46 ms` |

Actor vs Mass baseline은 UE 5.8에서 같은 `MovementActive` phase trigger와 1000개 count로 다시 측정했습니다.

| Scenario | Avg FrameTime | Avg TickActors | Notes |
| --- | ---: | ---: | --- |
| Mass 1000 MovementActive | `0.4567 ms` | `0.0997 ms` | `-NoMDSMassDebugDraw` |
| Actor 1000 MovementActive | `1.2473 ms` | `0.7907 ms` | `-NoMDSMassBaseline` |

이 수치는 local headless measurement로 설명해야 하며, production rendering benchmark나 GPU/viewport 성능 주장으로 해석하면 안 됩니다.

## 검증 상태

검증 완료:

- Standalone Mass objective 시나리오가 기대한 최종 Objective HP에 도달합니다.
- Editor server-mode 실행에서 `NetMode=DedicatedServer`를 보고합니다.
- UE 5.8 source engine에서 dedicated server binary를 빌드했습니다.
- `WindowsServer` cook/stage 후 staged dedicated server runtime이 실행됩니다.
- Dedicated server binary가 `IpNetDriver listening on port 7777` 상태에 도달합니다.
- Mass entity가 spawn, move, arrive하고, 도착 후 movement work가 중단됩니다.
- Objective damage event count가 arrival count와 일치합니다.
- Debug output이 기대한 runtime state를 노출합니다.
- Profiling log에서 debug visualization 수정 후 도착 이후 frame time이 개선된 것을 확인했습니다.
- Actor baseline과 Mass baseline을 같은 `MovementActive` phase trigger 조건으로 비교했습니다.

아직 미검증:

- Client viewport에서 replicated Objective HP 직접 확인.
- Unreal Insights trace capture.
- 전체 client viewport recording workflow.

## 문서 맵

- `Docs/00_Project_Goal.md` - 프로젝트 목표, 범위, 목표일, 면접 가치
- `Docs/01_Requirements.md` - MVP 요구사항 및 검증 매핑
- `Docs/02_Architecture.md` - high-level architecture 및 system responsibility
- `Docs/03_MVP_Task_Breakdown.md` - phased MVP task breakdown
- `Docs/07_Mass_Concept.md` - Mass implementation concept notes
- `Docs/08_Profiling_Comparison.md` - 현재 Mass profiling 결과 및 제한
- `Docs/AI_Harness.md` - AI-assisted development workflow
- `Docs/Task_Template.md` - 재사용 가능한 task request template
- `Docs/Approval_Report_Template.md` - 재사용 가능한 completion report template
- `Docs/Verification.md` - verification standards
- `Docs/Unreal_Rules.md` - Unreal C++ 및 multiplayer implementation rules
- `Docs/Mass_Rules.md` - Mass Entity / Mass AI working rules

## AI-assisted 워크플로우

비 trivial 작업은 승인 기반 workflow를 따릅니다.

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 명시적 승인 대기
5. 승인된 변경만 구현
6. 결과 검증
7. approval report 제공

AI assistance는 통제된 가속 도구로 사용합니다. 목표, 범위, 아키텍처 결정, 최종 검증 판단은 사람이 소유합니다.

## 면접 Discussion Points

- Objective HP를 server-authoritative로 둔 이유
- Replicated Objective state와 client-requested gameplay action의 차이
- Mass 작업을 spawn, movement, arrival, damage, debug, profiling 단계로 나눈 이유
- Debug visualization이 도착 이후 runtime cost를 만든 방식
- UE 5.8에서 `ZenStore` 기본 cook과 staged dedicated server 검증의 관계
- `-NullRHI` profiling이 비교에는 유용하지만 성능 주장으로는 제한되는 이유
- Actor-vs-Mass 비교를 같은 phase trigger와 debug draw off 조건으로 맞춘 이유

## 명시적 비범위

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

## 목표 완료일

`2026-07-31`
