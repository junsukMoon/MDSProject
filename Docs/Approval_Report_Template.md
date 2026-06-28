# Approval Report Template

이 템플릿은 task 완료 후 결과를 보고할 때 사용합니다.

보고서는 무엇을 했고, 무엇이 바뀌었고, 어떻게 검증했으며, 승인된 범위를 지켰는지 설명해야 합니다.

```text
Approval Report

Objective:
<이번 작업의 목표>

Plan Executed:
<승인된 계획을 어떻게 실행했는지>

Changed Files:
<변경된 파일 목록>

Implementation Summary:
<핵심 구현 요약>

Verification:
<실제로 실행한 검증 결과>

Manual Test Steps:
<재현 가능한 수동 테스트 단계>

Risks / Notes:
<남은 위험, 제한, 주의 사항>

Next Suggested Task:
<다음으로 할 만한 작고 review 가능한 작업>
```

## 작성 규칙

- 실행하지 않은 검증을 성공했다고 쓰지 않습니다.
- build를 실행하지 않았으면 “문서 변경만이라 build는 실행하지 않음”처럼 명확히 씁니다.
- runtime behavior는 실제 runtime/log/viewport/profiling 증거가 있을 때만 성공으로 기록합니다.
- 변경 파일이 승인 범위를 벗어났다면 반드시 명시합니다.
- Next Suggested Task는 너무 큰 scope가 아니라 작은 다음 작업이어야 합니다.
