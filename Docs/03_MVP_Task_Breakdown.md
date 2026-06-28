# MVP Task Breakdown

이 문서는 `MDSProject` MVP를 작은 review 가능한 task로 나눈 원래 계획 문서입니다.

현재 구현/검증 상태는 `README.md`, `Docs/05_Progress_Log.md`, `Docs/08_Profiling_Comparison.md`, `Docs/10_Visible_Demo_Verification.md`를 기준으로 봅니다. 이 문서는 작업 순서와 원칙을 보존하기 위한 기록입니다.

모든 non-trivial task는 AI harness workflow를 따라야 합니다.

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 명시적 승인 대기
5. 승인된 변경만 구현
6. 검증
7. approval report 제공

Mass spawn, movement, arrival detection, objective damage는 명시적으로 승인되지 않는 한 분리된 task여야 합니다.

## MVP Phase Overview

| Phase | Name | Task Type | Priority | Purpose |
| --- | --- | --- | --- | --- |
| 0 | Harness / Documentation | Documentation | Critical | workflow, scope, project rules 수립 |
| 1 | Mass Concept Study | Documentation | Critical | 구현 전 Mass 사용 방식 정의 |
| 2 | Mass Module Setup | Setup | Critical | 필요한 Mass dependency 준비 |
| 3 | Mass Spawn Only | Implementation | Critical | entity spawn 증명 |
| 4 | Mass Movement Only | Implementation | Critical | Objective 방향 movement 증명 |
| 5 | Arrival Detection Only | Implementation | Critical | damage와 분리된 arrival 감지 |
| 6 | Objective Damage Integration | Implementation | Critical | server-authoritative objective damage 적용 |
| 7 | Debug UI / Logging | Debug | High | 핵심 runtime state 노출 |
| 8 | Profiling Comparison | Profiling | High | Actor vs Mass 비교 데이터 기록 |
| 9 | Final README / Interview Summary | Documentation | High | 면접 설명용 패키징 |

## Phase 0: Harness / Documentation

목표:

- project goal, requirements, architecture, AI workflow, verification rules, Unreal rules, Mass rules를 문서화합니다.

검증:

- 문서 수동 확인
- implementation code가 추가되지 않았는지 확인

## Phase 1: Mass Concept Study

목표:

- Mass code를 추가하기 전에 Mass Entity 사용 목적과 server authority boundary를 정의합니다.

검증:

- `Docs/Mass_Rules.md` 기준 수동 확인

## Phase 2: Mass Module Setup

목표:

- 첫 Mass implementation step에 필요한 최소 module dependency만 추가합니다.

검증:

- build 또는 compile check
- `.Build.cs` 변경 이유 설명
- spawn/movement/arrival/damage logic이 추가되지 않았는지 확인

## Phase 3: Mass Spawn Only

목표:

- Mass enemy entity를 spawn하는 최소 경로를 구현합니다.

검증:

- entity count 확인
- spawn behavior 확인
- movement/arrival/damage가 포함되지 않았는지 확인

## Phase 4: Mass Movement Only

목표:

- spawn된 Mass entity가 Objective를 향해 이동하도록 합니다.

검증:

- movement behavior 확인
- arrival/damage가 포함되지 않았는지 확인
- 필요 시 profiling note 기록

## Phase 5: Arrival Detection Only

목표:

- Mass entity가 Objective area에 도착했는지 감지합니다.

검증:

- arrival count/log/debug 확인
- Objective HP가 이 단계에서 변경되지 않는지 확인

## Phase 6: Objective Damage Integration

목표:

- validated Mass arrival을 server-authoritative Objective HP damage와 연결합니다.

검증:

- server-side damage application
- client-visible replicated result
- objective gameplay check
- network replication check

## Phase 7: Debug UI / Logging

목표:

- authority, Objective HP, Mass spawn/movement/arrival/damage state를 debug output 또는 logs로 노출합니다.

검증:

- runtime debug output 확인
- server/client 차이 확인
- debug output이 gameplay correctness에 필수가 아닌지 확인

## Phase 8: Profiling Comparison

목표:

- Actor-based vs Mass-based comparison을 위한 profiling notes를 기록합니다.

검증:

- runtime mode
- scenario context
- entity/actor count
- FPS / frame time / GameThread impact
- limitation 명시

## Phase 9: Final README / Interview Summary

목표:

- completed MVP, verification evidence, server authority, Mass workflow, profiling result를 면접 설명용으로 정리합니다.

검증:

- README와 문서가 실제 검증 결과와 일치하는지 확인

## Dependency Summary

- Phase 0은 모든 작업의 기반입니다.
- Phase 1은 Mass setup/implementation 전에 필요합니다.
- Phase 2는 Mass spawn 전에 필요합니다.
- Phase 3은 movement 전에 필요합니다.
- Phase 4는 arrival detection 전에 필요합니다.
- Phase 5는 objective damage integration 전에 필요합니다.
- Phase 6은 final gameplay summary 전에 필요합니다.
- Phase 7은 표시할 runtime state에 의존합니다.
- Phase 8은 profiling할 behavior에 의존합니다.
- Phase 9는 완료/검증된 prior phase에 의존합니다.

## Task Type Rules

Documentation tasks:

- 승인된 Docs 또는 README만 수정합니다.
- implementation code를 만들지 않습니다.

Setup tasks:

- 승인된 build/module/config만 수정합니다.
- 모든 module dependency 변경 이유를 설명합니다.
- 별도 승인 없이 gameplay behavior를 추가하지 않습니다.

Implementation tasks:

- 작고 behavior-specific해야 합니다.
- server-authoritative gameplay를 보존해야 합니다.
- runtime verification을 포함해야 합니다.

Debug tasks:

- approved DebugUI/logs로 runtime state를 노출합니다.
- 큰 UI framework가 되면 안 됩니다.
- gameplay correctness에 필수가 되면 안 됩니다.

Profiling tasks:

- measurement context와 relevant metrics를 기록합니다.
- Actor vs Mass comparison을 지원합니다.
- 명시적 승인 없이 broad optimization/refactor가 되면 안 됩니다.
