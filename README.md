# MDSProject

MDSProject는 UE5 기술 포트폴리오 프로젝트입니다. 서버 권위 기반의 멀티플레이어 디펜스 샌드박스를 통해 Replication, Mass Entity 기반 게임플레이 흐름, 런타임 디버그 출력, 프로파일링 기록, 승인 기반 AI-assisted 개발 워크플로우를 보여주는 것이 목적입니다.

이 프로젝트는 완성형 게임이 아닙니다. 면접에서 설명 가능한 기술 데모에 초점을 맞춰 의도적으로 범위를 제한합니다.

## 현재 상태

구현 및 검증 완료:

- 서버 권위 기반 Objective HP Actor
- Replicated Objective HP 상태
- Mass Entity spawn-only 단계
- Objective를 향한 Mass 이동
- Mass 도착 감지
- 유효한 도착 시 서버 측 Objective damage 적용
- 런타임 debug state subsystem
- Standalone 및 editor server-mode 검증 로그
- 현재 Mass 시나리오에 대한 profiling comparison 문서
- 도착 후 debug visualization 성능 문제 수정

알려진 제한:

- Dedicated Server target binary 빌드는 현재 Epic Launcher 설치 버전 UE 5.6 제한으로 막혀 있습니다.
- 실제 dedicated server binary 빌드/프로파일링 검증은 source-built UE 5.6 설치 후 진행해야 합니다.
- Actor 기반 baseline benchmark는 아직 구현하거나 측정하지 않았습니다.
- `-NullRHI` profiling 결과는 로컬 비교에는 유용하지만, 최종 viewport 또는 GPU 성능을 대표하지 않습니다.

## 핵심 시나리오

Mass 적들이 Top Down 맵에 생성되고, 공유 Objective를 향해 이동합니다.

Mass Entity가 도착하면 서버가 도착을 기록하고 Objective damage를 한 번 적용합니다. Objective HP는 서버가 권위 있게 소유합니다. 런타임 debug output은 현재 NetMode, Objective HP, Mass spawn count, moved count, arrival count, damage count를 노출합니다.

현재 16 entity 시나리오에서 기대하고 검증한 최종 상태:

```text
MDS Debug | NetMode=Standalone | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

Editor server-mode 검증에서도 다음 상태에 도달했습니다.

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

## 프로파일링 스냅샷

현재 Mass 시나리오는 2026-06-27에 `UnrealEditor-Cmd.exe`, `-NullRHI`, benchmark mode로 측정했습니다.

| Scenario | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Standalone Mass scenario | `2571.43` | `0.39 ms` |
| Editor server-mode Mass scenario | `4500.00` | `0.22 ms` |

도착 후 frame slowdown은 debug visualization에서 발견되었습니다. 원인은 entity가 도착한 뒤에도 매 프레임 debug sphere를 반복해서 그리는 것이었습니다. 수정 후 marker는 최초 도착 시 한 번만 그립니다.

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before debug draw fix | `248.76` | `4.02 ms` |
| After debug draw fix | `2163.46` | `0.46 ms` |

이 수치는 local headless measurement로 설명해야 하며, production rendering benchmark로 주장하면 안 됩니다.

## 검증 상태

검증 완료:

- Standalone Mass objective 시나리오가 기대한 최종 Objective HP에 도달합니다.
- Editor server-mode 실행에서 `NetMode=DedicatedServer`를 보고합니다.
- Mass entity가 spawn, move, arrive하고, 도착 후 movement work가 중단됩니다.
- Objective damage event count가 arrival count와 일치합니다.
- Debug output이 기대한 runtime state를 노출합니다.
- Profiling log에서 debug visualization 수정 후 도착 이후 frame time이 개선된 것을 확인했습니다.

아직 미검증:

- Dedicated Server target binary build. 현재 installed engine distribution 제한으로 불가합니다.
- Source-built UE dedicated server 실행.
- Actor 기반 enemy baseline 비교.
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
- `-NullRHI` profiling이 비교에는 유용하지만 성능 주장으로는 제한되는 이유
- true dedicated server binary support를 주장하기 전에 남은 작업
- Actor-vs-Mass 주장을 하기 전에 Actor baseline을 어떻게 설계해야 하는지

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
