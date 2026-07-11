# AI 작업 명령 예시

이 문서는 Codex/AI에게 작업을 요청할 때 사용할 수 있는 예시입니다.

## 기본 요청 형식

```text
목표:
허용 파일:
금지 범위:
검증 방법:
승인 전에는 수정하지 말 것:
```

## 예시: 문서 수정

```text
Docs/08_Profiling_Comparison.md를 현재 검증 결과와 맞게 정리해줘.
코드는 수정하지 말고, 계획을 먼저 제안해줘.
```

## 예시: C++ 변경

```text
Objective HP replication 관련 코드를 점검하고,
server authority 규칙에 맞지 않는 부분이 있으면 계획을 제안해줘.
승인 전에는 수정하지 마.
```

## 예시: Mass 작업

```text
Mass arrival detection만 추가하는 계획을 세워줘.
spawn/movement/damage는 변경하지 마.
Docs/Mass_Rules.md를 읽고 따라줘.
```

## 예시: 검증

```text
Run_Smoke_DedicatedServer_WithClient.ps1를 실행해서
server final ObjectiveHP와 client replicated ObjectiveHP를 확인해줘.
실행한 검증만 성공으로 보고해줘.
```

## 주의사항

- non-trivial 작업은 승인 기반 workflow를 따릅니다.
- 검증하지 않은 결과를 성공으로 보고하지 않습니다.
- 관련 없는 refactor를 섞지 않습니다.
