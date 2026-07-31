# 프로젝트 목표

## 프로젝트 정의

```text
MDSProject = GAS 기반 Dedicated Server 성 방어 로그라이트 슈터
```

`MDSProject`는 UE5 기술 포트폴리오를 위한 소규모 플레이 가능한 버티컬 슬라이스입니다. 플레이어가 캐릭터를 직접 조작하고 총을 사용해 라운드마다 성을 공격하는 몬스터를 막습니다.

상용 완성형 게임을 목표로 하지 않습니다. 전투에서 라운드 정산과 다음 라운드 준비까지 이어지는 작은 플레이 흐름 안에서 서버 권한, Replication, 게임플레이 어빌리티 시스템(Gameplay Ability System, GAS), UI 동기화와 실행 검증을 설명하는 것이 목적입니다.

## 핵심 기술 목표

- Dedicated Server
- 서버 권한 전투
- Gameplay Ability System
- 성 Objective 방어
- 라운드 및 Wave 진행
- 플레이어와 Actor 기반 몬스터의 전투 행동
- 매치 전용 재화와 경험치
- 전투 중 레벨업 3지선다
- 라운드 사이 상점
- 라운드 정산 UI
- CMC 기반 이동과 기본 애니메이션
- Replication과 UI 동기화
- Debug output, 실행 검증과 프로파일링
- AI-assisted development workflow

## 핵심 플레이 흐름

```text
매치 시작
-> 서버가 라운드 전투 시작
-> Wave 단위 몬스터 생성
-> 플레이어 이동과 사격
-> 서버가 피해, 사망, 처치 귀속, 경험치와 재화 확정
-> 경험치 충족 시 전투 중 레벨업 중단과 3지선다
-> 같은 라운드 전투 재개
-> 모든 적 사망
-> 라운드 정산과 상점
-> 플레이어 준비 완료 또는 정산 제한시간
-> 다음 라운드
```

## 용어

라운드(Round):

- 전투 시작부터 전투 종료, 정산, 상점, 다음 라운드 준비까지 포함하는 전체 진행 단위입니다.

Wave:

- 한 라운드 안에서 특정 시점에 생성되는 몬스터 묶음입니다.
- MVP에서는 라운드당 Wave 하나를 사용할 수 있지만 상태와 데이터 명칭은 분리합니다.

전투 중 레벨업:

- 경험치 조건을 충족한 즉시 서버가 시작하는 매치 단위 전투 중단입니다.
- 진입 감속, 전투 시뮬레이션 정지, 3지선다, 강화 적용, 복귀 감속을 거쳐 같은 라운드를 재개합니다.
- Dedicated Server의 World Pause는 사용하지 않습니다.

## MVP 포함 범위

- Dedicated Server와 서버 권한 전투
- 플레이어 및 적 GAS
- 성 Objective
- Round와 Wave
- Actor 기반 적
- 매치 전용 재화, 경험치와 레벨
- 전투 중 레벨업 3지선다
- 매 라운드 정산 UI
- 정산 UI와 동시에 표시되는 즉시 적용형 상점
- 다음 라운드 준비와 정산 제한시간
- CMC, 기본 AnimBP, Attack Montage, AnimNotify 또는 Gameplay Event
- Hit Reaction과 Death Animation
- Gameplay Tag 기반 행동 차단
- Replication, Debug UI, 검증과 프로파일링

## 명시적 제외 범위

- 범용 Inventory와 장비 슬롯
- Quest, Crafting, SaveGame
- 영구 성장, 계정 재화, 백엔드 DB
- Matchmaking과 Lobby
- 거대한 Skill Tree
- 근거가 검증되지 않은 종합 Score 공식
- 대형 UI framework
- 상용 수준의 전체 콘텐츠와 복잡한 애니메이션 시스템
- 전체 Mover 전환
- 전체 Motion Matching 구축
- 전체 Mutable 시스템
- 전체 Mass Entity 적 전환
- 고급 lag compensation과 server rewind
- MVP 책임을 넘어서는 Full GAS expansion

즉시 적용형 상점은 Inventory를 허용하지 않습니다. 전투 중 3지선다는 Skill Tree 또는 영구 성장이 아닙니다. 라운드 정산 통계는 임의의 종합 Score가 아닙니다.

## 향후 확장 및 비교 학습

- Mass Entity와 Actor 기반 적의 확장성 비교
- Mover 이동 구조 비교
- Motion Matching locomotion 교체
- Mutable 캐릭터 구성
- Actor vs Mass profiling
- CSV 및 Unreal Insights 심화 분석

이 항목들은 MVP 완료 조건이 아닙니다.

## 포트폴리오 가치

면접에서 다음을 코드, 로그, UI와 실행 증거로 연결해 설명할 수 있어야 합니다.

- 서버가 어떤 gameplay state를 소유하는가
- 클라이언트 요청을 서버가 어떻게 검증하는가
- GAS와 AI, 라운드, 상점 책임을 왜 분리했는가
- 전투 중 레벨업에서 왜 World Pause 대신 명시적 시뮬레이션 중단을 사용하는가
- 팀 공통 결과와 플레이어별 결과를 어떻게 복제하는가
- 과거에 검증된 기반선과 새로 검증해야 할 기능을 어떻게 구분하는가
