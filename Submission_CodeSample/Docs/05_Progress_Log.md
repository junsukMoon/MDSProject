# 진행 로그

## 현재 상태

필수 MVP 구현과 주요 검증은 완료되었습니다.

현재 프로젝트는 면접에서 설명 가능한 기술 데모 상태입니다.

## 완료된 작업

문서:

- 프로젝트 목표
- 요구사항
- 아키텍처
- MVP 작업 분해
- Git workflow
- AI harness
- Unreal/Mass/Coding rules
- 검증 기준
- profiling 기록
- visible demo 검증 기록

구현:

- server-authoritative Objective Actor
- replicated Objective HP
- Dedicated Server target
- Mass module setup
- Mass spawn
- Mass movement
- Mass arrival detection
- once-only Objective damage integration
- runtime debug state subsystem
- Actor enemy baseline
- phase-based profiling harness
- smoke verification script

검증:

- UE 5.8 source engine build
- `MDSProjectEditor` compile
- `MDSProjectServer` compile
- `MDSProject` client compile
- dedicated server listen 확인
- server final Objective/Mass state 확인
- client replicated Objective HP 확인

최근 smoke 결과:

```text
SMOKE RESULT: PASS
```

## 현재 브랜치

```text
main
```

## 다음 후보 작업

- 제출용 코드 샘플 정리
- 문서 한글화
- 추가 profiling 반복 측정
- Unreal Insights deeper capture

## 주의사항

- `-NullRHI` profiling은 viewport/GPU 성능 주장이 아닙니다.
- 프로젝트는 완성형 게임이 아니라 기술 샌드박스입니다.
- visible demo는 replicated Objective HP 검증에 초점을 둡니다.
