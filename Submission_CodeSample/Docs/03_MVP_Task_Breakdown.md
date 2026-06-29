# MVP 작업 분해

이 문서는 `MDSProject` MVP를 작은 작업 단위로 나눈 기록입니다.

모든 non-trivial 작업은 승인 기반 workflow를 따릅니다.

1. 관련 파일 확인
2. 현재 구조 요약
3. 계획 제안
4. 명시 승인 대기
5. 승인된 변경만 구현
6. 검증
7. approval report

## Phase Overview

| Phase | 이름 | 목적 |
| --- | --- | --- |
| 0 | Harness / Documentation | 작업 규칙과 범위 정의 |
| 1 | Mass Concept Study | Mass 사용 목적과 설계 방향 정의 |
| 2 | Mass Module Setup | 필요한 module dependency 준비 |
| 3 | Mass Spawn Only | entity 생성 증명 |
| 4 | Mass Movement Only | Objective 방향 이동 증명 |
| 5 | Arrival Detection Only | 도착 감지 증명 |
| 6 | Objective Damage Integration | server-authoritative damage 연결 |
| 7 | Debug UI / Logging | runtime state 출력 |
| 8 | Profiling Comparison | Actor vs Mass 비교 |
| 9 | Final README / Interview Summary | 면접 설명 정리 |

## Mass 작업 순서

Mass 작업은 다음 순서를 지킵니다.

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection only
5. Objective damage integration
6. Debug/profiling

명시 승인 없이 여러 단계를 한 번에 합치지 않습니다.

## 완료 기준

각 단계는 실제 검증 결과를 남겨야 합니다.

- build/compile
- PIE 또는 server runtime
- dedicated server log
- client log
- debug output
- profiling data

실행하지 않은 검증은 성공으로 기록하지 않습니다.
