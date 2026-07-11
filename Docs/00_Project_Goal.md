# 프로젝트 목표

`MDSProject`는 UE5 기술 포트폴리오용 멀티플레이어 방어 샌드박스입니다.

## 핵심 목표

다음 기술을 작고 검증 가능한 형태로 보여줍니다.

- Dedicated Server
- Replication
- Authority / Ownership
- Objective gameplay
- Server-authoritative combat
- Wave progression
- UI driven by replicated state
- Character Movement / Animation baseline
- Debug output
- Runtime Review / Verification Evidence
- AI-assisted development workflow

## 범위

이 프로젝트는 완성형 게임이 아닙니다. 면접에서 설명 가능한 기술 데모가 목적입니다.

포함 범위:

- 서버가 전투 판정, 적 HP, Objective HP, Wave 진행을 소유하는 gameplay loop
- Objective defense combat loop
- client-visible replicated Objective HP
- replicated state 기반 UI 갱신
- CMC 기반 player movement
- Skeletal Mesh character setup
- AnimBP State Machine
- Attack Montage와 AnimNotify timing
- Hit Reaction과 Death Animation
- debug state 출력
- runtime review와 smoke verification evidence

제외 범위:

- Inventory
- Quest
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- 대형 UI framework
- Full GAS
- 완성형 콘텐츠 제작
- full Mover migration
- production Motion Matching implementation
- full Mutable character customization pipeline
- Mass Entity as a required v2 MVP feature
- formal profiling as an MVP requirement

## Future Extension Notes

다음 항목은 MVP에 직접 구현하지 않고 추후 확장 가능한 기술 항목으로 문서화합니다.

- Mover
- Motion Matching
- Mutable
- Mass Entity

기존 profiling, Actor vs Mass 비교, CSV/Insights trace는 future extension/reference 자료로 유지합니다.

## 포트폴리오 가치

이 프로젝트는 “게임 하나를 완성했다”보다 “Dedicated Server 환경에서 서버 권한 전투, Objective 방어, Replication, UI, Wave, 캐릭터/애니메이션 기본기를 어떻게 설계하고 설명할 수 있는가”에 초점을 둡니다.
