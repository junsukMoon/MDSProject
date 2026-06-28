# Task Template

이 템플릿은 새 task를 요청하거나 계획할 때 사용합니다.

## Task Request

```text
Objective:
<작업 목표>

Context:
<왜 필요한지, 관련 배경>

Allowed Files:
<수정 가능한 파일 또는 폴더>

Forbidden Scope:
<하지 말아야 할 것>

Acceptance Criteria:
<완료 판단 기준>

Verification:
<실행해야 할 검증>
```

## Plan Format

구현 전 계획은 다음 형식을 사용합니다.

```text
Plan

Objective:
Files Read:
Current Structure Observed:
Proposed Changes:
Files Expected to Change:
Risks:
Verification Plan:
Approval Needed:
```

## Approval Report Format

구현 후 보고는 다음 형식을 사용합니다.

```text
Approval Report

Objective:
Plan Executed:
Changed Files:
Implementation Summary:
Verification:
Manual Test Steps:
Risks / Notes:
Next Suggested Task:
```

## Acceptance Criteria 작성 규칙

- review, build, test, PIE, dedicated server, logs, manual inspection 등으로 관찰 가능해야 합니다.
- “잘 동작한다”처럼 모호하게 쓰지 않습니다.
- network task는 server/client result를 포함해야 합니다.
- profiling task는 scenario, runtime mode, count, metric을 포함해야 합니다.

## Verification 작성 규칙

- 실행할 검증을 구체적으로 씁니다.
- 실행하지 못할 수 있는 검증은 대체 검증과 남은 gap을 명시합니다.
- 실행하지 않은 test를 passed로 쓰면 안 됩니다.
