# Character Movement & Animation Readiness

이 문서는 MDS v2 MVP의 캐릭터 이동/애니메이션 기준을 정의합니다.

## MVP Position

MDS v2는 Dedicated Server Objective Combat Demo입니다. 캐릭터/애니메이션 레이어는 전투 데모를 설명 가능하게 만드는 최소 범위로 포함합니다.

MVP 캐릭터 레이어:

- CMC-based movement
- Skeletal Mesh player character
- Animation Blueprint State Machine
- Idle / Walk / Run locomotion
- Attack Montage
- AnimNotify timing markers
- Hit Reaction
- Death Animation

## CMC Baseline

MVP의 기본 movement 구현은 Unreal의 `CharacterMovementComponent`입니다.

CMC를 사용하는 이유:

- Unreal Character 기본 replication 흐름과 잘 맞습니다.
- Dedicated Server / client movement correction 흐름을 설명하기 좋습니다.
- MVP가 movement system 연구가 아니라 server-authoritative combat demo에 집중할 수 있습니다.
- 면접에서 ownership, role, prediction, correction, replicated movement를 기본기 중심으로 설명할 수 있습니다.

## Authority and Replication

Movement input은 owning client에서 시작될 수 있지만, 최종 gameplay state는 서버 권한을 기준으로 해석합니다.

전투 판정과 damage는 movement/animation presentation과 분리합니다.

- 클라이언트는 입력과 animation playback을 요청하거나 표시할 수 있습니다.
- 서버는 attack 가능 여부, range, target validity, damage, enemy HP, Objective HP를 판단합니다.
- replicated combat state가 UI, hit reaction, death animation을 구동합니다.

## Animation Baseline

MVP AnimBP는 State Machine + Montage 구조를 기본으로 합니다.

- locomotion: Idle / Walk / Run State Machine
- attack: Attack Montage
- attack timing: AnimNotify marker 사용
- hit: server-confirmed damage 이후 Hit Reaction
- death: replicated death state 이후 Death Animation

AnimNotify는 authoritative damage를 직접 적용하지 않습니다. Notify는 visual timing, local cue, server request marker, debug marker로만 사용합니다.

## Future Mover Extension

Mover로 전환하려면 다음 항목을 별도 작업으로 검토해야 합니다.

- movement component 교체 범위
- input production and simulation path
- network prediction model
- movement replication and correction behavior
- ability/combat code가 movement state를 읽는 방식
- debug output for movement authority and role state
- dedicated server 검증 기준

Full Mover migration은 MVP 범위가 아닙니다. MVP에서는 CMC를 기준으로 동작 가능한 Objective Combat Demo를 먼저 완성합니다.
