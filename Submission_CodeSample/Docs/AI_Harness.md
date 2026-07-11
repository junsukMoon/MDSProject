# AI Harness

이 문서는 `MDSProject`에서 AI-assisted development를 통제된 방식으로 사용하기 위한 규칙입니다.

AI는 project owner가 아니라 development assistant입니다. 목표, 범위, architecture decision, 최종 검증 판단은 사람이 소유합니다.

## 기본 원칙

- 작은 변경을 선호합니다.
- non-trivial task는 승인 전 파일을 수정하지 않습니다.
- 구현 전 관련 파일을 읽습니다.
- 승인된 범위 밖으로 확장하지 않습니다.
- 검증하지 않은 결과를 성공으로 말하지 않습니다.
- 완료 후 approval report를 제공합니다.

## 필수 Workflow

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 명시 승인 대기
5. 승인된 변경만 구현
6. 결과 검증
7. approval report 제공

## Plan Requirements

계획에는 다음이 포함되어야 합니다.

- Objective
- Files Read
- Current Structure Observed
- Proposed Changes
- Files Expected to Change
- Risks
- Verification Plan
- Approval Needed

## Implementation Rules

- 승인된 파일과 범위만 수정합니다.
- 관련 없는 refactor를 하지 않습니다.
- 명시 요청 없이 rename하지 않습니다.
- Mass 작업은 incremental order를 따릅니다.
- server-authoritative gameplay를 기본값으로 둡니다.
- debug/profiling code가 gameplay correctness의 필수 경로가 되면 안 됩니다.

## Verification Rules

코드 변경은 관련 검증을 포함해야 합니다.

예시:

- build / compile
- PIE
- dedicated server run
- server/client logs
- visible viewport evidence
- profiling CSV
- Unreal Insights trace

문서 변경만이면 build를 생략할 수 있지만 생략 이유를 보고해야 합니다.

## Failure Handling

실패하면 범위를 넓히지 말고 다음을 보고합니다.

- 무엇이 실패했는가
- 어떤 error/warning이 있었는가
- 현재 영향 범위
- 가장 작은 다음 diagnostic step

## Portfolio Value

AI-assisted workflow 자체도 포트폴리오 설명 대상입니다.

강조할 점:

- task를 작게 나눴다
- 승인 전 plan을 만들었다
- 검증 증거를 남겼다
- AI를 architecture owner가 아니라 controlled assistant로 사용했다
