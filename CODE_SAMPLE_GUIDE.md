# MDSProject 코드 샘플 안내

## 요약

`MDSProject`는 UE5 기술 포트폴리오용 멀티플레이어 방어 샌드박스입니다.

이 샘플의 목적은 완성형 게임을 보여주는 것이 아니라 다음 기술을 작고 검증 가능한 형태로 보여주는 것입니다.

- Dedicated Server
- 서버 권한 기반 Objective HP
- Replication
- Mass Entity 기반 적 시뮬레이션
- Actor baseline 비교
- 런타임 Debug 상태 출력
- Profiling 기록
- AI-assisted 개발 워크플로우

## 먼저 볼 파일

평가자는 아래 파일부터 보면 전체 구조를 빠르게 파악할 수 있습니다.

VS2022에서 코드 구조를 보고 싶다면 다음 솔루션 파일을 열면 됩니다.

```text
MDSProject/MDSProject.sln
```

이 솔루션은 제출 샘플 안의 핵심 C++ 파일을 직접 연결한 코드 탐색용 솔루션입니다. Unreal Build Tool로 빌드하는 전체 프로젝트 솔루션이 아닙니다.

이 패키지는 코드 리뷰용 샘플입니다. 실행 가능한 전체 UE 프로젝트 패키지가 아니며 `Content`, `Binaries`, `Saved`, `Intermediate`는 제외했습니다.

```text
MDSProject/Source/MDSProject/Objective/MDSObjectiveActor.h
MDSProject/Source/MDSProject/Objective/MDSObjectiveActor.cpp
MDSProject/Source/MDSProject/MassAI/MDSMassEnemyFragments.h
MDSProject/Source/MDSProject/MassAI/MDSMassSpawnSubsystem.cpp
MDSProject/Source/MDSProject/MassAI/MDSMassMovementProcessor.cpp
MDSProject/Source/MDSProject/MassAI/MDSMassArrivalProcessor.cpp
MDSProject/Source/MDSProject/Debug/MDSDebugStateSubsystem.cpp
MDSProject/Source/MDSProject/Profiling/MDSGameplayProfileSubsystem.cpp
Run_Smoke_DedicatedServer_WithClient.ps1
```

## 핵심 흐름

1. Dedicated server가 Objective HP를 소유합니다.
2. Mass entity가 생성됩니다.
3. Mass movement processor가 entity를 Objective 방향으로 이동시킵니다.
4. Mass arrival processor가 도착을 감지합니다.
5. 도착한 entity는 Objective damage를 한 번만 적용합니다.
6. 클라이언트는 replicated Objective HP를 관찰합니다.

서버 최종 상태 예시:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

클라이언트 관찰 상태 예시:

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## 서버 권한과 Replication

관련 파일:

```text
MDSProject/Source/MDSProject/Objective/MDSObjectiveActor.h
MDSProject/Source/MDSProject/Objective/MDSObjectiveActor.cpp
```

핵심 포인트:

- `AMDSObjectiveActor`가 Objective HP를 소유합니다.
- `CurrentHealth`는 `ReplicatedUsing=OnRep_CurrentHealth`로 복제됩니다.
- `ApplyObjectiveDamage`가 서버 권한 경계입니다.
- 서버 권한이 없는 damage 요청은 거부됩니다.
- 클라이언트는 `OnRep_CurrentHealth`를 통해 로컬 debug 상태를 갱신합니다.

중요한 설계 의도는 클라이언트가 Objective HP를 직접 바꾸지 않고 서버가 소유한 결과만 관찰한다는 점입니다.

## Mass Entity 흐름

관련 파일:

```text
MDSProject/Source/MDSProject/MassAI/MDSMassEnemyFragments.h
MDSProject/Source/MDSProject/MassAI/MDSMassSpawnSubsystem.h
MDSProject/Source/MDSProject/MassAI/MDSMassSpawnSubsystem.cpp
MDSProject/Source/MDSProject/MassAI/MDSMassMovementProcessor.h
MDSProject/Source/MDSProject/MassAI/MDSMassMovementProcessor.cpp
MDSProject/Source/MDSProject/MassAI/MDSMassArrivalProcessor.h
MDSProject/Source/MDSProject/MassAI/MDSMassArrivalProcessor.cpp
```

Mass 작업은 작고 검증 가능한 단계로 나뉩니다.

- fragment/tag로 entity 데이터를 정의합니다.
- spawn subsystem이 entity를 생성하고 fragment를 초기화합니다.
- movement processor가 entity 위치를 갱신합니다.
- arrival processor가 도착을 감지하고 Objective damage를 한 번만 적용합니다.

도착 상태와 damage 적용 상태는 분리되어 있습니다.

- `bHasArrived`
- `bHasAppliedObjectiveDamage`

이 분리 덕분에 entity가 도착 상태를 유지해도 damage가 반복 적용되지 않습니다.

## Actor Baseline

관련 파일:

```text
MDSProject/Source/MDSProject/ActorAI/MDSActorEnemy.h
MDSProject/Source/MDSProject/ActorAI/MDSActorEnemy.cpp
MDSProject/Source/MDSProject/ActorAI/MDSActorEnemySpawnSubsystem.h
MDSProject/Source/MDSProject/ActorAI/MDSActorEnemySpawnSubsystem.cpp
```

Actor baseline은 Mass 방식과 비교하기 위한 기준 구현입니다. 많은 적을 Actor tick 기반으로 처리했을 때의 비용을 Mass 방식과 비교하기 위해 사용합니다.

## Debug와 Profiling

Debug 상태:

```text
MDSProject/Source/MDSProject/Debug/MDSDebugStateSubsystem.h
MDSProject/Source/MDSProject/Debug/MDSDebugStateSubsystem.cpp
```

Profiling harness:

```text
MDSProject/Source/MDSProject/Profiling/MDSGameplayProfileSubsystem.h
MDSProject/Source/MDSProject/Profiling/MDSGameplayProfileSubsystem.cpp
```

Debug subsystem은 NetMode, Objective HP, Mass count, Actor baseline count를 출력합니다.

Profiling subsystem은 `MovementActive` 같은 phase trigger를 기준으로 CSV capture를 수행합니다.

## 검증 방법

검증 스크립트:

```text
Run_Smoke_DedicatedServer_WithClient.ps1
```

스크립트는 다음을 실행합니다.

- 현재 빌드된 `MDSProjectServer.exe`
- staged standalone `MDSProject.exe` client

검증 항목:

- 서버가 port `7777`에서 listen하는지
- 서버 최종 Objective/Mass 상태가 기대값에 도달하는지
- 클라이언트가 replicated Objective HP `20/100`을 관찰하는지

최근 smoke 검증 결과:

```text
SMOKE RESULT: PASS
```

## 관련 문서

```text
Docs/Coding_Standards.md
Docs/Unreal_Rules.md
Docs/Mass_Rules.md
Docs/08_Profiling_Comparison.md
Docs/10_Visible_Demo_Verification.md
```

## 한계

- 이 프로젝트는 완성형 게임이 아닙니다.
- Inventory, quest, matchmaking, lobby, save system, 대형 UI framework는 의도적으로 제외했습니다.
- Mass entity 자체를 완전한 replicated enemy actor처럼 보여주는 구조는 아닙니다.
- 클라이언트 검증의 핵심은 replicated Objective HP입니다.
- `-NullRHI` profiling은 로컬 CPU/gameplay 비교 근거이며 viewport/GPU 최종 성능 벤치마크가 아닙니다.
- Unreal Insights trace는 smoke capture이며 깊은 성능 분석 결과는 아닙니다.

## 면접 설명용 한 문장

```text
이 UE5 샘플은 Dedicated Server가 Objective HP를 소유하고, Mass entity가 서버에서 이동/도착/damage를 처리하며, standalone client가 replicated Objective HP를 관찰하는 서버 권한 기반 멀티플레이어 기술 데모입니다.
```
