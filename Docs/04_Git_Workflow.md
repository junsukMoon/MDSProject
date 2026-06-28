# Git Workflow

Git workflow는 포트폴리오 프로세스의 일부입니다. 목적은 빠른 변경이 아니라, task selection, approval gate, verification, review discipline을 보여주는 것입니다.

## 기본 규칙

- `main`에는 직접 commit하지 않습니다.
- `main`에는 직접 push하지 않습니다.
- `main`은 PR merge된 work만 포함해야 합니다.
- 각 task는 별도 branch에서 진행합니다.
- task branch는 최신 `origin/main`에서 시작합니다.
- PR title/body는 이 프로젝트에서는 한국어를 사용합니다.

## Branch 규칙

예시:

- `docs/...`
- `feature/...`
- `fix/...`
- `verify/...`
- `profile/...`

작업 전 `main`에 있다면 새 task branch를 만듭니다.

## Approval Workflow

non-trivial task는 다음 순서를 따릅니다.

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 사용자 승인 대기
5. 승인된 변경만 구현
6. 검증
7. approval report 제공

승인 전에는 파일을 수정하지 않습니다.

## PR Workflow

PR은 작고 task-specific이어야 합니다.

PR body에는 다음이 포함되어야 합니다.

- 요약
- 변경 파일
- 검증 결과
- 제한 / notes

문서 변경만 있으면 build를 생략할 수 있지만, 그 사실을 명확히 적습니다.

## Merge Workflow

일반 원칙:

- 사용자가 merge를 승인한 PR만 merge합니다.
- merge 후 원격 branch를 삭제합니다.
- 로컬 `main` 상태를 확인합니다.
- 로컬 PR 작업 branch가 남아 있으면 정리합니다.

현재 대화 규칙:

- PR 생성, PR 본문 확인/수정, PR 상태 확인, PR checks 확인은 별도 승인 질문 없이 진행합니다.
- 사용자가 승인한 PR은 merge, 원격 branch 삭제, merge 결과 확인, 로컬 상태 확인까지 별도 재확인 없이 진행합니다.
- `git reset --hard`, untracked 파일 삭제, 백업 branch 삭제, 사용자 변경 되돌리기 같은 손실 가능 작업은 별도 확인을 받습니다.

## Verification Rule

실제로 실행하지 않은 검증은 성공했다고 쓰지 않습니다.

예:

- build를 실행하지 않았다면 “build 실행 안 함”이라고 적습니다.
- runtime verification을 하지 않았다면 manual inspection과 구분합니다.
- profiling은 runtime mode, scenario, entity/actor count, limitation을 기록합니다.

## Failure Handling

task가 실패하면 scope를 넓히지 말고 다음을 보고합니다.

- 실패한 명령 또는 검증
- 관찰된 error/warning
- 영향 범위
- 가장 작은 다음 diagnostic step

실패를 숨기거나 unrelated system을 크게 고쳐 우회하지 않습니다.
