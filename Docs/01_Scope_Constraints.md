# 범위 제한

## MVP 정의

```text
MDSProject MVP = GAS 기반 Dedicated Server 성 방어 로그라이트 슈터 버티컬 슬라이스
```

MVP는 상용 완성형 게임이 아니라 전투, 전투 중 레벨업, 라운드 정산, 상점과 다음 라운드 시작까지 실제로 이어지는 작은 플레이 흐름입니다.

## MVP 포함

- Dedicated Server
- 서버 권한 전투와 Replication
- 플레이어 및 적 Gameplay Ability System
- 성 Objective
- Round와 Wave
- Actor 기반 적
- 매치 전용 재화, 경험치와 레벨
- 전투 중 즉시 레벨업
- 진입·복귀 슬로우 모션과 서버 권한 전투 시뮬레이션 정지
- 3지선다와 강화 Gameplay Effect
- 매 라운드 정산 UI
- 즉시 적용형 라운드 상점
- 다음 라운드 준비와 정산 제한시간
- CMC 기반 이동
- 기본 AnimBP, Attack Montage, AnimNotify 또는 Gameplay Event
- Hit Reaction과 Death Animation
- Gameplay Tag 기반 행동 차단
- Debug output, Runtime Review, 실행 검증과 필요한 범위의 profiling

## 제한된 허용 범위

상점:

- 상품은 Inventory에 저장하지 않습니다.
- 구매 즉시 플레이어 또는 Objective에 효과를 적용합니다.
- 장비 슬롯, 아이템 드래그와 범용 소유 목록을 만들지 않습니다.

레벨업:

- 전투 중 매치 전용 3지선다입니다.
- 영구 성장, 노드 그래프와 대규모 Skill Tree를 만들지 않습니다.

라운드 결과:

- 서버가 측정한 라운드 및 개인 통계만 표시합니다.
- 근거 없는 가중치나 종합 Score 공식을 만들지 않습니다.

GAS:

- 전투 행동, Effect와 Tag 차단에 사용합니다.
- AI 의사결정, Round/Wave, 보상 집계, 상점과 통계까지 GAS에 넣지 않습니다.

## MVP 제외

- 범용 Inventory와 장비 슬롯
- Quest, Crafting, SaveGame
- 영구 성장과 계정 재화
- 백엔드 DB
- Matchmaking과 Lobby
- 거대한 Skill Tree
- 근거 없는 종합 Score
- 대형 UI framework
- 전체 production 콘텐츠
- 전체 Mover migration
- custom network prediction movement system
- production Motion Matching
- 전체 Mutable pipeline
- 전체 Mass Entity 적 전환
- 고급 lag compensation과 server rewind
- MVP 책임을 넘어서는 Full GAS expansion

## 향후 확장

- Mass Entity와 Actor 기반 적 비교
- Actor vs Mass profiling
- Mover
- Motion Matching
- Mutable
- CSV 및 Unreal Insights 심화 분석
- 선택적인 전체 매치 누적 결과 화면

## 권한 제한

서버가 damage, HP, 사망, 처치 귀속, 재화, 경험치, 레벨, 강화 적용, Round/Wave와 구매 결과를 소유합니다.

UI, Gameplay Cue, AnimNotify, Montage와 client-only event는 gameplay state를 직접 변경하지 않습니다.

Dedicated Server World Pause는 사용하지 않습니다. 전투 중 레벨업은 명시적인 서버 권한 `CombatSuspended` 상태로 gameplay simulation만 정지하고 UI, 네트워크, Replication과 선택 RPC는 유지합니다.
