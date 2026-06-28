# AI Task Commands

이 문서는 `MDSProject`에서 AI-assisted workflow를 반복적으로 실행하기 위한 command pattern을 정리합니다.

Codex는 기본적으로 한국어로 답합니다. 단, code, command, log, Unreal API 이름은 원문을 유지합니다.

## Orchestrator Command

목적:

- 다음 task를 고르고 scope를 좁힙니다.
- 관련 파일을 읽고 plan을 제안합니다.
- 승인 전에는 파일을 수정하지 않습니다.

사용 예:

```text
다음 오케스트레이터 진행해줘
```

필수 동작:

- 현재 branch와 worktree 확인
- 관련 문서/코드 확인
- 현재 구조 요약
- 변경 계획 제안
- approval 필요 여부 명시

## Execute Command

목적:

- 승인된 plan만 구현합니다.

사용 예:

```text
승인
```

필수 동작:

- approved scope 안에서만 파일 수정
- 필요 시 branch 생성
- build/test/runtime verification 실행
- 결과 보고

## PR Preparation Command

목적:

- 완료된 task를 commit하고 PR로 올립니다.

필수 동작:

- `git diff --check`
- 변경 파일 확인
- commit 생성
- push
- PR 생성
- PR 본문은 한국어로 작성

현재 대화 규칙:

- PR 생성, PR 본문 확인/수정, PR 상태 확인, PR checks 확인은 사용자에게 다시 묻지 않고 진행합니다.
- 사용자가 승인한 PR은 merge, 원격 branch 삭제, merge 결과 확인, 로컬 상태 확인까지 바로 진행합니다.

## Merge Command

목적:

- 승인된 PR을 `main`에 통합합니다.

필수 동작:

- PR merge
- remote branch 삭제
- merge 결과 확인
- local `main` 상태 확인
- PR 작업 branch 정리

주의:

- `git reset --hard`, untracked 파일 삭제, 백업 branch 삭제, 사용자 변경 되돌리기는 별도 확인이 필요합니다.

## Failure Handling Command

실패 시 보고해야 하는 내용:

- 실패한 명령
- exit code
- 핵심 error/warning
- 영향 범위
- 가장 작은 다음 diagnostic step

실패 후에는 scope를 넓히지 않습니다.

## Approval Report Format

완료 보고에는 다음을 포함합니다.

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

## Working Rules

- 각 AI task는 별도 branch를 사용합니다.
- 승인 전 파일 수정 금지.
- 승인된 파일/범위만 수정.
- verification을 실행하지 않았으면 실행하지 않았다고 말합니다.
- PR body에는 approval report 또는 핵심 검증 내용을 포함합니다.
- commit message, PR title/body는 한국어를 기본으로 합니다.
