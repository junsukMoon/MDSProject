# 코딩 규약

이 문서는 `MDSProject` C++ 작업에서 따를 실무 코딩 규약입니다.

함께 참고할 문서:

- `AGENTS.md`
- `Docs/Unreal_Rules.md`
- `Docs/Mass_Rules.md`
- `Docs/Verification.md`

## 범위 관리

- 작고 리뷰 가능한 변경을 선호합니다.
- 승인된 파일만 수정합니다.
- 명시 요청 없이 broad refactor를 하지 않습니다.
- 명시 요청 없이 파일, 클래스, 함수, 변수, 폴더 이름을 바꾸지 않습니다.
- 큰 gameplay class를 바꾸기보다 작은 helper/component/subsystem을 우선 고려합니다.
- 프로젝트 범위는 dedicated server, replication, authority, Mass, debug, profiling, verification 중심으로 유지합니다.

## 파일과 클래스 구조

- 클래스는 하나의 명확한 책임에 집중합니다.
- 새 구조를 만들기 전에 기존 프로젝트 구조를 따릅니다.
- header는 public API, override, protected, private, property 순서를 선호합니다.
- `.cpp`는 include, log category/local helper, constructor, lifecycle, public method, private helper 순서를 선호합니다.
- helper는 실제 복잡도나 중복을 줄일 때만 추가합니다.

## 이름 규칙

- Unreal C++ prefix(`A`, `U`, `F`, `E`, `I`)를 따릅니다.
- bool은 `bHasArrived`처럼 `b` prefix를 사용합니다.
- 새 public project type은 `MDS` prefix를 유지합니다.
- CVar는 `mds.System.Feature` 형식을 사용합니다.
- command-line flag는 `-MDSFeature`, 비활성화 flag는 `-NoMDSFeature` 형식을 선호합니다.

## Include 규칙

- header에서는 가능한 forward declaration을 사용합니다.
- concrete include는 가능하면 `.cpp`에 둡니다.
- reflected header에서 `*.generated.h`는 마지막 include입니다.
- speculative include를 추가하지 않습니다.
- include coupling을 숨기기 위해 module dependency를 추가하지 않습니다.

## 함수 설계

- gameplay state를 바꾸는 함수는 초반에 authority check를 둡니다.
- invalid state는 early return으로 명확히 배제합니다.
- Tick, Mass processor, replication callback, frequent debug update에서는 allocation, expensive world search, log spam을 피합니다.
- 한 함수가 관련 없는 책임을 섞으면 private helper로 분리합니다.
- 추상화를 위한 추상화는 피합니다.

## Unreal Reflection

- `UPROPERTY`는 lifetime tracking, GC, serialization, replication, editor exposure, Blueprint exposure가 필요할 때 사용합니다.
- `UFUNCTION`은 RPC, delegate, reflection, Blueprint, editor/tooling이 필요할 때 사용합니다.
- Blueprint exposure는 의도적으로만 추가합니다.
- client가 server-owned gameplay state를 직접 수정할 수 있게 노출하지 않습니다.

## UObject Lifetime과 GC

- raw UObject pointer가 ownership을 의미한다고 가정하지 않습니다.
- GC 추적이 필요하면 `UPROPERTY` 또는 적절한 Unreal pointer type을 사용합니다.
- `TObjectPtr`, `TWeakObjectPtr`, raw pointer, reference는 lifetime 의도에 맞게 선택합니다.
- world teardown, level change, destruction 가능성이 있는 cached reference는 `TWeakObjectPtr`를 고려합니다.
- `NewObject`에는 적절한 Outer를 지정합니다.
- constructor에서는 world, subsystem, player, network state에 의존하지 않습니다.
- `BeginPlay` 순서를 가정하지 않고 필요한 object/subsystem 존재 여부를 확인합니다.
- timer/delegate는 owner lifetime을 고려하고 필요하면 `Deinitialize` 또는 `EndPlay`에서 정리합니다.
- async/deferred/next-tick callback에서는 UObject validity를 다시 확인합니다.
- `AddToRoot`는 명시 승인과 문서화된 사유 없이는 사용하지 않습니다.

## Authority와 Replication Checklist

replicated gameplay state를 추가하거나 변경할 때 확인합니다.

- server source of truth가 명확한가
- client mutation path가 차단되거나 server request로 변환되는가
- `UPROPERTY` metadata가 적절한가
- `GetLifetimeReplicatedProps`가 갱신됐는가
- `OnRep`가 client presentation/cache/local reaction 용도만 수행하는가
- server state change가 client observation보다 먼저 발생하는가
- RPC ownership/direction이 계획 또는 보고에 명시됐는가
- listen server 또는 dedicated server 검증이 포함됐는가

## RPC 규칙

- Server RPC는 client request를 server가 validate/apply해야 할 때만 사용합니다.
- Client RPC는 owning client 대상 메시지에만 사용합니다.
- NetMulticast RPC는 sparingly 사용합니다.
- RPC로 replicated state ownership을 우회하지 않습니다.
- high-frequency reliable RPC를 피합니다.
- durable gameplay result는 replicated state를 선호합니다.

## Logging

- project-specific log category를 선호합니다.
- authority-sensitive event는 server/client 진단에 도움이 되도록 기록합니다.
- actor, role/net mode, state, result를 포함합니다.
- per-frame log spam을 피합니다.
- `Warning`/`Error`는 actionable problem, invalid configuration, rejected unsafe path에 사용합니다.

## CVar와 Command-Line Flag

- debug/profiling behavior는 CVar 또는 command-line flag로 끌 수 있어야 합니다.
- 비활성화 flag는 `-NoMDS...` 형식을 선호합니다.
- runtime behavior flag는 기본값과 목적이 명확해야 합니다.
- 중요한 profiling/debug flag는 작업 보고나 문서에 기록합니다.
- 호환되지 않는 profiling flag는 초기에 검증하고 명확한 log를 남깁니다.

## Debug Draw와 Debug UI

- debug draw는 gameplay correctness에 필요하면 안 됩니다.
- dedicated server logic은 visual debug system에 의존하면 안 됩니다.
- profiling 시 debug draw를 끌 수 있어야 합니다.
- 반복 debug draw는 lifetime, count, frequency를 제한합니다.
- debug UI는 상태를 보고할 뿐 gameplay authority가 되면 안 됩니다.
- 명시 요청 없이 큰 UI framework를 추가하지 않습니다.

## Subsystem 사용 기준

- world-scoped demo state, runtime harness, debug/profiling 지원에는 `UWorldSubsystem`을 선호합니다.
- subsystem이 client, listen server, dedicated server, standalone 중 어디서 실행돼야 하는지 확인합니다.
- gameplay authority와 debug/reporting 책임을 섞지 않습니다.
- subsystem이 Actor/UObject를 cache하면 world teardown과 stale reference를 고려합니다.
- teardown에서 timer와 외부 binding을 정리합니다.

## 성능 민감 코드

다음은 성능 민감 코드로 봅니다.

- Tick
- Mass processor
- spawning
- replication update
- debug output
- profiling harness
- runtime logging

규칙:

- hot path allocation을 피합니다.
- 반복 world search를 피합니다.
- 불필요한 replication을 피합니다.
- high-frequency reliable RPC를 피합니다.
- debug/profiling overhead는 guard합니다.
- 성능 주장을 할 때는 측정 context를 포함합니다.

## 검증 기대값

- C++ 변경은 build/compile, runtime check, log check 중 최소 하나를 보고합니다.
- network 변경은 server/client 관찰 결과가 필요합니다.
- replication 변경은 client-visible result check가 필요합니다.
- Mass 변경은 해당 behavior(spawn, movement, arrival, damage, debug, profiling)를 검증합니다.
- 문서 변경만이면 build/runtime 검증을 생략할 수 있지만 이유를 보고합니다.

## Markdown과 인코딩

- 새 Markdown 문서는 UTF-8로 작성합니다.
- 공개 문서와 `.private` 개인 문서를 구분합니다.
- 깨진 인코딩 문서는 필요한 경우 한글 UTF-8 문서로 정리합니다.
