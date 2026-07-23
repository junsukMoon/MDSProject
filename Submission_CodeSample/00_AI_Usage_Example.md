# AI 사용 예시

## 목적

이 문서는 `MDSProject_CodeSample.zip`에서 AI-assisted development를 어떻게 사용했는지 설명합니다.

이 제출물은 UE5 프로젝트 안에서 승인 기반 AI-assisted engineering workflow를 적용한 사례를 보여주는 것을 목적으로 합니다. AI는 관련 파일 확인, 현재 구조 요약, 변경 계획 작성, 승인된 범위의 구현, 검증 보조, 결과 보고에 사용했습니다.

작업 범위 결정, 승인 여부 판단, 기술적 판단, 검증 기준, 최종 수용 여부는 개발자가 담당했습니다.

## 실제 적용 Task

대표 사례:

```text
Replicated UI Viewport Verification
```

이 작업은 Match HUD, Objective World UI, Enemy World UI가 replicated gameplay state를 읽고 staged dedicated server/client runtime에서 실제 viewport에 표시되는지 확인하는 것을 목적으로 했습니다.

이 작업의 주요 목표는 다음과 같습니다.

- gameplay state를 server-authoritative 구조로 유지합니다.
- UI를 replicated state observer로 유지합니다.
- dedicated server/client 환경에서 runtime 동작을 검증합니다.
- build, staged runtime log, viewport screenshot 기반 evidence를 남깁니다.
- 실제로 실행한 검증 결과만 문서에 기록합니다.

## AI가 한 일

- 관련 C++ 파일, 프로젝트 문서, 검증 스크립트를 확인했습니다.
- 현재 runtime 구조와 data ownership 경계를 요약했습니다.
- 파일 수정 전에 구현 계획을 제안했습니다.
- 예상 변경 파일과 검증 절차를 구현 전에 정리했습니다.
- 승인된 범위 안에서 변경을 구현했습니다.
- runtime widget이 화면에 visible output을 만들도록 UI fallback rendering path를 조정했습니다.
- dedicated server/client log를 확인하는 검증 스크립트를 정리했습니다.
- blank screenshot 또는 title-bar-only screenshot을 실패로 처리하도록 visible-pixel validation을 추가했습니다.
- 작업 완료 후 프로젝트의 approval report 형식으로 결과를 보고했습니다.

## 개발자가 판단하고 수정한 일

- 프로젝트를 기술 포트폴리오 샘플로 정의했습니다.
- networking, replication, debug UI, verification behavior를 설명 가능한 범위로 유지했습니다.
- 제안된 변경이 server-authoritative gameplay 원칙을 지키는지 검토했습니다.
- UI widget을 replicated state의 presentation/read-only observer로 유지하는 방향을 선택했습니다.
- 구체적인 verification evidence가 있어야 결과를 수용하도록 기준을 정했습니다.
- 검증하지 않은 주장을 최종 문서에 포함하지 않도록 정리했습니다.
- 제출물의 설명 방향을 controlled AI-assisted development practice에 맞게 정리했습니다.

## 실제 검증 결과

최신 strict verification result:

```text
REPLICATED UI VIEWPORT VERIFY RESULT: PASS
```

검증된 항목:

- `MDSProjectEditor`, `MDSProject`, `MDSProjectServer` Development build가 성공했습니다.
- Win64 client/server cook and stage가 성공했습니다.
- staged dedicated server가 실행되고 combat enemy를 spawn했습니다.
- staged client가 Match HUD를 생성했습니다.
- Objective World UI가 replicated Objective HP를 읽었습니다.
- Enemy World UI가 replicated Enemy HP를 읽었습니다.
- runtime log에서 replicated UI source read가 확인되었습니다.
- engine screenshot capture에서 visible viewport pixel이 확인되었습니다.

주요 evidence 문서:

```text
Docs/11_Runtime_Review_Evidence.md
```

## 주요 문서 안내

```text
Docs/AI_Harness.md
Docs/Verification.md
Docs/11_Runtime_Review_Evidence.md
Docs/Coding_Standards.md
Docs/Unreal_Rules.md
Docs/Mass_Rules.md
```

문서 역할:

- `Docs/AI_Harness.md`: 승인 기반 AI-assisted workflow 기준을 설명합니다.
- `Docs/Verification.md`: 유효한 검증 기준을 설명합니다.
- `Docs/11_Runtime_Review_Evidence.md`: 실제 runtime verification evidence를 기록합니다.
- `Docs/Coding_Standards.md`: 프로젝트 C++ coding rule을 설명합니다.
- `Docs/Unreal_Rules.md`: Unreal-specific implementation rule을 설명합니다.
- `Docs/Mass_Rules.md`: Mass Entity 작업의 제약과 진행 순서를 설명합니다.

## 요약

이 제출물은 AI를 통제된 engineering assistant로 사용한 사례를 보여줍니다. AI는 project context 확인, scoped plan 제안, 승인된 변경 구현, verification evidence 정리를 담당했습니다. 개발자는 scope, approval, technical judgment, final acceptance를 관리했습니다.
