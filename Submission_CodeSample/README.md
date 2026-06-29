# MDSProject

`MDSProject`는 UE5 기술 포트폴리오용 멀티플레이어 방어 샌드박스입니다.

목표는 완성형 게임을 만드는 것이 아니라, 면접에서 설명 가능한 기술 데모를 만드는 것입니다.

핵심 주제:

- Dedicated Server
- Replication
- Authority / Ownership
- Objective gameplay
- Mass Entity / Mass AI
- Debug output
- Profiling
- AI-assisted development workflow

## Interview Demo

이 프로젝트의 핵심 데모는 다음 흐름입니다.

1. Dedicated server가 Objective HP를 소유합니다.
2. Mass entity가 Objective 방향으로 이동합니다.
3. Entity가 도착하면 서버가 Objective damage를 한 번만 적용합니다.
4. 클라이언트는 replicated Objective HP를 관찰합니다.

가장 짧은 설명:

```text
Dedicated server owns Objective HP. Mass entities damage the Objective on arrival. Clients observe the replicated HP. Actor and Mass baselines are profiled under the same MovementActive phase trigger for interview discussion, not final rendering performance claims.
```

## 현재 상태

구현 및 검증 완료 항목:

- UE 5.8 source engine 기준 server/client target build
- Dedicated Server target
- WindowsServer cook/stage/runtime workflow
- 서버 권한 기반 Objective Actor
- replicated Objective HP
- Mass entity spawn
- Mass movement
- Mass arrival detection
- once-only Objective damage integration
- runtime debug state subsystem
- Actor enemy baseline
- Actor vs Mass phase-based profiling comparison
- visible two-client Objective HP verification
- smoke verification script

## 런타임 시나리오

Mass entity는 Top Down map에서 생성되고 Objective 방향으로 이동합니다.

도착한 entity는 서버에서 Objective damage를 한 번만 적용합니다. Objective HP는 서버가 소유하고 클라이언트는 복제된 결과만 관찰합니다.

서버 최종 상태 예시:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

클라이언트 관찰 상태 예시:

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## 아키텍처 요약

서버 권한 경계:

- `AMDSObjectiveActor::ApplyObjectiveDamage`
- `CurrentHealth`는 `ReplicatedUsing=OnRep_CurrentHealth`
- 클라이언트는 Objective HP를 직접 수정하지 않음

Mass 흐름:

1. `UMDSMassSpawnSubsystem`
2. `UMDSMassMovementProcessor`
3. `UMDSMassArrivalProcessor`
4. `AMDSObjectiveActor::ApplyObjectiveDamage`

Debug / profiling:

- `UMDSDebugStateSubsystem`
- `UMDSGameplayProfileSubsystem`
- `Run_Smoke_DedicatedServer_WithClient.ps1`

## 주요 코드

```text
MDSProject/Source/MDSProject/Objective
MDSProject/Source/MDSProject/MassAI
MDSProject/Source/MDSProject/ActorAI
MDSProject/Source/MDSProject/Debug
MDSProject/Source/MDSProject/Profiling
```

## Dedicated Server 검증

검증 스크립트:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Smoke_DedicatedServer_WithClient.ps1
```

최근 검증 결과:

```text
SMOKE RESULT: PASS
```

검증 내용:

- server binary 실행
- UDP `7777` listen
- staged client 접속
- server final Objective/Mass state 확인
- client replicated Objective HP 확인

## Profiling 요약

Actor baseline과 Mass baseline은 같은 `MovementActive` phase trigger 기준으로 비교했습니다.

주의:

- `-NullRHI` 결과는 로컬 CPU/gameplay 비교용입니다.
- viewport/GPU 최종 성능 주장으로 해석하면 안 됩니다.
- Unreal Insights trace는 smoke capture입니다.

상세 내용:

```text
Docs/08_Profiling_Comparison.md
Docs/10_Visible_Demo_Verification.md
```

## 문서

```text
Docs/Coding_Standards.md
Docs/Unreal_Rules.md
Docs/Mass_Rules.md
Docs/Verification.md
Docs/AI_Harness.md
Docs/08_Profiling_Comparison.md
Docs/10_Visible_Demo_Verification.md
```

## 제외 범위

이 프로젝트는 기술 데모이므로 다음 시스템은 의도적으로 제외합니다.

- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Large UI framework
- Complex animation system
- Full GAS expansion
- Full production-quality game content
