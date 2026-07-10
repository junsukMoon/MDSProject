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
- Combat enemy spawn baseline
- phase-based profiling harness
- smoke verification script
- CommonUI 기반 debug overlay C++ 골격
- Widget Blueprint 연동용 `BindWidgetOptional` 필드와 제작 가이드
- CommonUI viewport client config 정리
- UI 화면 검증은 Widget Blueprint asset 생성 후 진행 필요
- 제출용 코드 샘플 정리 및 VS2022 탐색용 프로젝트 패키징
- 원본 프로젝트 기준 MDS v2 구조 정합성 리뷰

검증:

- UE 5.8 source engine build
- `MDSProjectEditor` compile
- `MDSProjectServer` compile
- `MDSProject` client compile
- dedicated server listen 확인
- server final Objective/Mass state 확인
- client replicated Objective HP 확인
- staged client/server Wave display state 확인
- client CommonUI viewport 설정 오류 제거 확인

최근 smoke 결과:

```text
SMOKE RESULT: PASS
WAVE VERIFY RESULT: PASS
```

## 현재 브랜치

```text
mds-v2-objective-combat-demo
```

## 다음 후보 작업

- Runtime Review / Verification Evidence 추가 정리
- Widget Blueprint asset 생성 후 UI 화면 검증
- 문서 한글화
- 추가 profiling 반복 측정 또는 Unreal Insights deeper capture는 필요 시 future/reference 작업으로만 진행

## 최근 구조 리뷰 메모

- Wave authority는 `AMDSProjectGameMode`가 담당하고, replicated Wave display state는 `AMDSProjectGameState`가 담당합니다.
- Objective HP는 `AMDSObjectiveActor`가 서버 권한으로 계산하고 `CurrentHealth`를 복제합니다.
- Enemy HP는 `AMDSCombatEnemyActor`가 별도 경로로 계산하며, death 상태는 `CurrentHealth <= 0.0f`에서 파생됩니다.
- `UMDSActorEnemySpawnSubsystem`은 v2 경로에서 `AMDSCombatEnemyActor`를 생성합니다. 삭제된 `AMDSActorEnemy`는 MVP runtime path에 남아 있지 않습니다.
- Debug overlay와 `UMDSDebugStateSubsystem`은 관찰/검증 보조이며 gameplay truth source가 아닙니다.
- CommonUI debug overlay runtime을 위해 `GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient` 설정을 사용합니다.
- Mass와 profiling 자료는 MVP 필수 구현이 아니라 reference/future extension으로 유지합니다.

## 주의사항

- `-NullRHI` profiling은 viewport/GPU 성능 주장이 아닙니다.
- 프로젝트는 완성형 게임이 아니라 기술 샌드박스입니다.
- visible demo는 replicated Objective HP 검증에 초점을 둡니다.
