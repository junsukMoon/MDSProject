# 아키텍처

이 문서는 `MDSProject`의 high-level architecture와 system responsibility를 정의합니다.

아키텍처는 의도적으로 작게 유지합니다. 목표는 full game framework가 아니라 ownership, replication, Mass Entity boundary, verification discipline을 명확하게 보여주는 것입니다.

## 핵심 원칙

- 서버가 gameplay state의 source of truth입니다.
- client는 replicated result를 관찰합니다.
- Objective HP, damage, score, win/loss 같은 gameplay state는 client-owned가 되면 안 됩니다.
- Mass simulation은 scalable enemy behavior를 보여주기 위한 도구입니다.
- Debug output과 profiling은 설명과 검증을 돕는 보조 수단입니다.

## 주요 시스템

### Objective

책임:

- Objective HP 소유
- server-side damage application
- replicated HP 노출
- client-side `OnRep` 반응

규칙:

- Objective HP 변경은 서버에서만 발생합니다.
- client는 HP를 직접 감소시키면 안 됩니다.
- Objective state는 debug output과 logs로 확인 가능해야 합니다.

### MassAI

책임:

- Mass entity spawn
- Objective 방향 movement
- arrival detection
- server-side objective damage trigger
- debug/profiling state 제공

규칙:

- Mass 작업은 spawn, movement, arrival, damage로 나누어 진행합니다.
- Mass fragment/tag/processor는 작고 설명 가능해야 합니다.
- Mass presentation은 gameplay authority가 아닙니다.

### ActorAI Baseline

책임:

- Mass 비교용 최소 Actor enemy baseline 제공
- Actor spawn / movement / arrival damage 흐름 제공
- profiling comparison용 count와 tick behavior 제공

규칙:

- full AI, behavior tree, animation, combat system으로 확장하지 않습니다.
- Mass와 비교 가능한 최소 baseline에 집중합니다.

### Debug

책임:

- NetMode
- Objective HP
- Mass count
- Actor baseline count
- runtime verification에 필요한 상태 표시

규칙:

- Debug UI는 lightweight해야 합니다.
- debug output은 gameplay correctness의 필수 조건이 되면 안 됩니다.
- dedicated server logic은 viewport나 HUD에 의존하면 안 됩니다.

### Profiling

책임:

- profiling scenario 실행 보조
- CSV capture trigger
- Actor/Mass phase-based comparison
- performance notes 문서화

규칙:

- `-NullRHI` 결과는 local comparison으로만 설명합니다.
- viewport/GPU 성능 주장으로 확대하지 않습니다.
- 필요할 때만 deeper Unreal Insights analysis를 수행합니다.

## 네트워크 구조

- 서버가 Objective HP를 소유합니다.
- client는 action을 요청할 수 있지만 서버가 검증하고 적용합니다.
- replicated state에는 명확한 source of truth가 있어야 합니다.
- RPC는 ownership과 방향이 설명 가능해야 합니다.
- `OnRep`는 client-side presentation/cache update에 사용하고 authoritative gameplay source가 되면 안 됩니다.

## 코드 배치 원칙

- Objective code는 `Objective` 영역에 둡니다.
- Mass 관련 code는 `MassAI` 영역에 둡니다.
- Actor baseline은 `ActorAI` 영역에 둡니다.
- Debug state는 `Debug` 영역에 둡니다.
- Profiling helper는 `Profiling` 영역에 둡니다.
- player/controller 관련 확장은 기존 player/controller code와 가까운 곳에 둡니다.

## 검증 원칙

각 task는 다음 중 관련된 검증을 실제로 수행하고 보고해야 합니다.

- build / compile
- editor startup
- PIE
- dedicated server runtime
- server/client logs
- visible viewport evidence
- profiling CSV
- Unreal Insights trace

실행하지 않은 검증은 성공했다고 기록하지 않습니다.
