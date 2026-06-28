# Remaining Work Checklist

이 문서는 Phase 10 이후 남은 작업을 빌드 완료 후 순차적으로 처리하기 위한 체크리스트입니다.

상태 기준:

- 완료: 코드 또는 문서가 있고 실제 검증 로그가 존재합니다.
- 진행 중: 현재 작업 중이지만 아직 검증이 끝나지 않았습니다.
- 부족: 일부 구현은 있으나 데모/검증 기준에 부족합니다.
- 미완료: 아직 구현 또는 검증이 없습니다.

## Objective HP

상태: 완료

- Objective Actor 생성: 완료
  - `AMDSObjectiveActor` 구현 완료.
- HP Replication: 완료
  - `CurrentHealth`는 `ReplicatedUsing=OnRep_CurrentHealth`로 복제됩니다.
- Server-only damage 함수: 완료
  - `ApplyObjectiveDamage()`는 `HasAuthority()`를 검사하고 non-authority damage request를 거부합니다.
- Debug log: 완료
  - 서버 초기화, Objective damage, client `OnRep_CurrentHealth()` 로그가 있습니다.

추가 확인:

- dedicated server에 standalone client 2개를 붙여 client-side Objective HP replication을 로그 기준으로 확인했습니다.
  - Client 1/2 모두 `LogMDSDebugState`에서 `NetMode=Client | ObjectiveHP=20/100` 확인.
  - 실제 viewport 영상 검증은 Debug UI / Logs 항목에 남겨 둡니다.

## Dedicated Server 검증

상태: 진행 중

완료:

- Server Target 빌드 완료.
- UE 5.8 기준 `MDSProject Win64 Development` client/game target 빌드 완료.
- UE 5.8 source engine 기준 staged dedicated server runtime 검증 완료.
- Server log에서 다음 상태 확인 완료:
  - `World NetMode = Dedicated Server`
  - `IpNetDriver listening on port 7777`
  - `ObjectiveHP=20/100`
  - `Mass Spawned=16`
  - `Arrived=16`
  - `Damage=16`
- README에 dedicated server build/cook/stage/runtime 검증 흐름 문서화 완료.
- standalone client 2개를 dedicated server에 접속 완료.
- client log에서 replicated Objective HP 확인 완료.
  - Client 1: `ObjectiveHP=20/100` 72회 확인.
  - Client 2: `ObjectiveHP=20/100` 72회 확인.
- client 2개가 같은 server-owned Objective HP 결과를 보는지 로그 기준 확인 완료.

미완료:

- visible viewport에서 replicated Objective HP 변화 관찰.
- client 2개가 같은 server-owned Objective HP 결과를 보는 영상 또는 스크린샷 검증.

다음 순서:

1. visible viewport client 2개로 같은 Objective HP 표시를 확인.
2. server/client 로그와 viewport 결과를 함께 보관.
3. 필요한 경우에만 debug field를 최소 추가.

## Actor Enemy Baseline

상태: 진행 중

완료:

- Actor enemy spawn.
- Actor enemy `MoveTo Objective`.
- Actor arrival damage.
- Actor enemy count log.
- Actor enemy debug state count.
  - `Actor Spawned`
  - `Actor Arrived`
  - `Actor Damage`
- Actor baseline profiling.
  - 최신 gameplay CSV harness 기준 Actor 1000 vs Mass 1000 headless profile 기록 완료.

미완료:

- visible viewport 기준 Actor vs Mass 확인.

주의:

- Actor baseline은 Mass 비교 목적의 최소 구현이어야 합니다.
- full AI, behavior tree 확장, animation, combat system 확장은 범위 밖입니다.

다음 순서:

1. visible viewport 기준으로 필요 시 Actor vs Mass profile을 재확인.
2. README `Interview Demo` 섹션에 현재 검증 결과를 정리.

## Mass Spawn + Movement

상태: 완료

완료:

- Mass module setup 완료.
- Mass spawn subsystem 구현 완료.
- Mass movement processor 구현 완료.
- debug state에 Mass spawned/moved count 연동 완료.
- dedicated server runtime에서 Mass spawned/moved/arrived/damage final state 확인 완료.

주의:

- 작업 순서는 spawn-only, movement-only, arrival, damage integration으로 나눠 진행했습니다.
- 현재 최종 코드에서는 arrival detect와 damage integration이 연결된 상태입니다.

## Mass Arrival + Damage

상태: 완료

완료:

- Arrival detect 구현 완료.
- Objective damage integration 구현 완료.
- `bHasAppliedObjectiveDamage`로 once-only damage 처리.
- Objective damage는 서버 권위 path인 `AMDSObjectiveActor::ApplyObjectiveDamage()`를 통해 적용됩니다.
- dedicated server log에서 `Arrived=16`, `Damage=16`, `ObjectiveHP=20/100` 확인 완료.

추가 확인:

- standalone client 2개에서 replicated HP 결과를 로그 기준으로 확인했습니다.
  - Client 1/2 모두 `ObjectiveHP=20/100`.
  - visible viewport 영상 검증은 Debug UI / Logs 항목에 남겨 둡니다.

## Debug UI / Logs

상태: 부족

완료:

- `NetMode` 표시.
- `ObjectiveHP` 표시.
- `Mass Spawned` 표시.
- `Moved` 표시.
- `Arrived` 표시.
- `Damage` 표시.
- dedicated server가 아닌 world에서는 on-screen debug message 출력.

부족:

- `Role` 또는 local/remote role은 debug output에 아직 없습니다.
- client 2개 viewport에서 같은 replicated Objective HP를 보는 영상 검증은 아직 없습니다.

추가 완료:

- Actor enemy baseline count를 debug state/debug line에 추가했습니다.
  - `Actor Spawned`
  - `Actor Arrived`
  - `Actor Damage`
- UE 5.8 runtime log에서 Actor baseline debug counts 갱신을 확인했습니다.
  - 최종 debug line: `Actor Spawned=16 Arrived=16 Damage=16`

다음 순서:

1. visible viewport 검증에서 `Role` 표시가 필요하다고 판단되면 debug line에 최소 추가.
2. Actor baseline profiling 시 추가 counter가 필요하면 별도 검토.

## Profiling + Interview README

상태: 부족

완료:

- Mass standalone profiling 기록.
- editor server-mode Mass profiling 기록.
- staged dedicated server binary profiling 기록.
- README에 server authority, Mass workflow, dedicated server 검증, profiling snapshot 정리.

부족:

- README 상단에 명시적인 `Interview Demo` 섹션은 아직 없습니다.
- 2~3분 영상 또는 GIF가 아직 없습니다.
- Unreal Insights trace는 아직 없습니다.

추가 완료:

- Actor vs Mass headless CSV smoke profile을 최신 gameplay CSV harness 기준으로 기록했습니다.

다음 순서:

1. README 상단에 `Interview Demo` 섹션 추가.
2. dedicated server + clients + debug output 기준 2~3분 영상 또는 GIF 제작.
3. 필요 시 Unreal Insights trace 캡처.

## Recommended Order After Current Build

1. Update README with `Interview Demo`.
2. Record 2-3 minute demo video or GIF.
