# 요구사항

이 문서는 `MDSProject`의 기술 포트폴리오 MVP 요구사항을 정의합니다.

## 목적

MVP는 Unreal Engine client/gameplay programming 면접을 위한 기술 milestone입니다.

보여줘야 하는 핵심은 다음과 같습니다.

- server-authoritative multiplayer gameplay
- Objective state와 HP replication
- Mass Entity enemy flow
- debug visibility
- profiling readiness
- controlled AI-assisted development workflow

## 핵심 요구사항

- 프로젝트는 multiplayer testing을 지원해야 합니다.
- dedicated-server style workflow와 호환되어야 합니다.
- 서버가 authoritative gameplay state를 소유해야 합니다.
- Objective HP는 server-authoritative여야 합니다.
- Mass enemy를 spawn할 수 있어야 합니다.
- Mass enemy는 Objective를 향해 이동해야 합니다.
- Objective 도착을 감지할 수 있어야 합니다.
- Objective HP는 유효한 server-authoritative arrival/damage logic으로만 감소해야 합니다.
- Debug UI 또는 logs는 핵심 runtime state를 노출해야 합니다.
- Profiling data는 Actor 기반 baseline과 Mass 기반 scenario 비교를 지원해야 합니다.

## 네트워크 요구사항

- client는 gameplay result를 직접 적용하면 안 됩니다.
- client는 gameplay action을 요청할 수 있지만, 서버가 검증하고 적용해야 합니다.
- replicated state는 명확한 server-side source of truth를 가져야 합니다.
- network 관련 변경은 server/client 검증 노트를 포함해야 합니다.
- RPC는 ownership과 방향성이 설명 가능해야 합니다.

## Mass Entity 요구사항

- Mass 작업은 `Docs/Mass_Rules.md`의 순서를 따라야 합니다.
- spawn, movement, arrival detection, objective damage integration은 명시적으로 승인되지 않는 한 분리된 작업이어야 합니다.
- Mass task는 Entity, Fragment, Processor, Spawner, Representation 책임을 설명해야 합니다.
- Mass spawn task는 entity count와 spawn behavior를 보고해야 합니다.
- Mass movement task는 movement 또는 processor behavior를 보고해야 합니다.
- Mass arrival task는 arrival detection behavior를 보고해야 합니다.
- Mass objective integration은 server-authoritative objective damage를 보존해야 합니다.
- Mass profiling task는 성능 영향을 기록해야 합니다.

## Objective Gameplay 요구사항

- Objective는 HP를 server-owned gameplay state로 노출해야 합니다.
- Objective HP는 validated server gameplay logic으로만 변경되어야 합니다.
- enemy arrival과 objective damage application은 구분되어야 합니다.
- Objective damage는 debug UI, logs, verification notes 중 하나로 측정 가능해야 합니다.
- client는 Objective state를 관찰할 수 있지만 Objective HP를 소유하면 안 됩니다.
- objective HP에서 파생되는 score, win/loss state가 추가된다면 그것도 server-owned여야 합니다.

## Debug / Logs 요구사항

- Debug UI 또는 logs는 현재 task와 관련된 key runtime state를 노출해야 합니다.
- Debug output은 gameplay correctness에 필수이면 안 됩니다.
- network 상황에서는 server/client 차이가 명확해야 합니다.
- logging은 진단을 도와야 하며 per-frame spam이 되면 안 됩니다.

## Profiling 요구사항

- profiling은 Actor-based vs Mass-based comparison을 지원해야 합니다.
- profiling notes는 FPS, frame time, GameThread impact를 필요할 때 기록해야 합니다.
- scenario context, entity/actor count, runtime mode를 포함해야 합니다.
- Mass profiling은 entity count와 performance impact를 명시해야 합니다.
- profiling은 면접 설명을 돕기 위한 것이며 프로젝트를 production optimization으로 확장하기 위한 목적이 아닙니다.
