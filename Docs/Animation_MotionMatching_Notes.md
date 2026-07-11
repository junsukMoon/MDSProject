# Animation / Motion Matching Notes

이 문서는 MDS v2의 MVP animation 접근과 Motion Matching future extension 방향을 정리합니다.

## Current MVP Animation Approach

MVP는 production locomotion system이 아니라 전투 데모에 필요한 기본기를 보여주는 것을 목표로 합니다.

현재 기준:

- Animation Blueprint State Machine
- Idle / Walk / Run locomotion
- Attack Montage
- AnimNotify timing marker
- Hit Reaction
- Death Animation

Gameplay state가 필요한 animation은 replicated state 또는 server-confirmed state를 기준으로 구동합니다.

## Motion Matching Concept

Motion Matching은 고정된 State Machine transition만으로 pose를 고르는 방식이 아니라, 현재 캐릭터 상태와 원하는 trajectory에 가장 잘 맞는 pose를 animation database에서 검색해 선택하는 locomotion 접근입니다.

개념적으로 다음 요소가 중요합니다.

- Pose Search Database: 검색 대상 animation pose 모음
- Pose Search Schema: 어떤 feature를 비교할지 정의하는 기준
- Pose History: 최근 pose와 movement context
- trajectory query: 앞으로 이동하려는 방향과 속도에 대한 질의

## MVP Scope Decision

Full Motion Matching implementation은 MVP 범위가 아닙니다.

이유:

- MDS v2의 핵심은 Dedicated Server Objective Combat Demo입니다.
- Motion Matching은 animation asset 준비, database 구성, schema tuning, trajectory tuning이 필요합니다.
- 잘못 넣으면 전투/복제/서버 권한 데모보다 animation pipeline 작업이 중심이 됩니다.

## Future Replacement Point

Motion Matching은 향후 locomotion layer를 대체할 수 있습니다.

대체 가능 영역:

- Idle / Walk / Run locomotion State Machine
- BlendSpace 기반 속도/방향 locomotion

계속 gameplay-driven으로 남겨야 하는 영역:

- attack intent and validation
- Attack Montage or attack action presentation
- AnimNotify timing as visual/sync marker only
- Hit Reaction after server-confirmed damage
- Death Animation after replicated death state
- server-authoritative damage timing and calculation

## Interview Notes

State Machine:

- 명확한 상태와 transition rule을 정의하기 쉽습니다.
- MVP locomotion 기본기 설명에 적합합니다.

BlendSpace:

- speed/direction 같은 연속 값을 기반으로 animation을 blending합니다.
- State Machine 내부 locomotion state에서 자주 사용됩니다.

Montage:

- attack, reload, hit, death처럼 gameplay action과 묶인 one-shot animation에 적합합니다.
- section, notify, slot을 통해 action timing을 제어할 수 있습니다.

Motion Matching:

- database search 기반으로 현재/미래 movement intent에 맞는 pose를 고릅니다.
- 자연스러운 locomotion 품질을 목표로 하지만 MVP에는 과합니다.

MDS v2에서는 State Machine + Montage + AnimNotify로 기본기를 보여주고, Motion Matching은 future extension으로 설명합니다.
