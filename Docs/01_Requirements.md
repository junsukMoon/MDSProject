# 요구사항

## 프로젝트 요구사항

1. `MDSProject`는 GAS 기반 Dedicated Server 성 방어 로그라이트 슈터의 소규모 플레이 가능한 버티컬 슬라이스여야 합니다.
2. 서버는 전투 판정, 피해, HP, 사망, 처치 귀속, 재화, 경험치, 레벨, 라운드와 Wave 진행을 소유해야 합니다.
3. 클라이언트는 행동을 요청할 수 있지만 서버가 검증하고 적용해야 합니다.
4. UI, Gameplay Cue, AnimNotify와 `OnRep`는 gameplay truth를 직접 변경하면 안 됩니다.
5. 클라이언트는 Replication된 상태를 기반으로 UI와 animation presentation을 갱신해야 합니다.

## 라운드 및 Wave 요구사항

1. 라운드는 전투 시작부터 정산, 상점과 다음 라운드 준비까지의 전체 단위입니다.
2. Wave는 한 라운드 안에서 생성되는 몬스터 묶음입니다.
3. MVP에서 라운드당 Wave 하나를 사용하더라도 `RoundIndex`와 `WaveIndex`를 분리해야 합니다.
4. Match Phase는 `Waiting`, `Combat`, `RoundSettlement`, `Finished`를 우선 사용합니다.
5. `RoundSettlement`에서는 새 적 생성과 전투 Ability를 차단해야 합니다.
6. 모든 유효 플레이어가 준비하거나 정산 제한시간이 끝나면 서버가 다음 라운드를 시작해야 합니다.
7. 연결 해제된 플레이어가 준비 완료 집계를 막으면 안 됩니다.

## 전투 중 레벨업 요구사항

1. 서버가 경험치 지급과 레벨업 조건 충족을 확정해야 합니다.
2. 레벨업은 라운드 종료가 아니라 전투 중 경험치 충족 즉시 시작해야 합니다.
3. 진입 시 짧은 슬로우 모션을 거쳐 매치 단위 전투 시뮬레이션을 정지해야 합니다.
4. 플레이어, 적, AI 판단, Projectile 이동·충돌·수명, 피해, Objective 공격, 적 생성과 Wave 전환이 정지해야 합니다.
5. UI, 네트워크, Replication과 선택 RPC는 계속 동작해야 합니다.
6. Dedicated Server World Pause를 사용하면 안 됩니다.
7. 3개 후보 중 선택을 서버가 검증하고 선택한 Gameplay Effect를 적용해야 합니다.
8. 여러 레벨이 누적되면 모든 선택을 순차 처리한 뒤 전투를 재개해야 합니다.
9. 선택 후 복귀 슬로우 모션을 거쳐 같은 라운드와 Wave를 재개해야 합니다.
10. 여러 플레이어가 동시에 레벨업하면 모든 필수 선택이 완료된 뒤 한 번만 재개해야 합니다.
11. 선택 대상의 연결 해제로 매치가 교착되면 안 됩니다.

## GAS 요구사항

GAS 담당:

- 플레이어 발사와 재장전
- 선택적 플레이어 대시
- 적 근접 공격과 성 공격
- Damage, Heal, 전투 능력치 강화와 상태 이상
- Gameplay Tag 기반 행동 차단
- Montage와 AnimNotify 또는 Gameplay Event 기반 공격 타이밍
- Gameplay Cue 기반 연출

GAS 비담당:

- AI 목표 선택과 이동 경로
- 라운드와 Wave 진행
- 상점 상품과 레벨업 후보 생성
- 처치 귀속, 재화와 경험치 집계
- 결과 통계와 다음 라운드 전환

## 정산 및 상점 요구사항

1. 모든 라운드가 끝날 때 라운드 정산과 상점을 한 화면에 동시에 표시해야 합니다.
2. 정산에는 라운드 번호, 시간, 처치, 재화, 경험치, 성 피해와 남은 HP, 플레이어 레벨과 보유 재화를 표시해야 합니다.
3. 상점 상품은 Inventory 아이템이 아니며 구매 즉시 플레이어 또는 성에 효과를 적용해야 합니다.
4. 구매는 서버가 Phase, 상품, 가격, 재화, 중복 여부와 대상을 검증해야 합니다.
5. 마지막 라운드도 같은 정산 UI를 사용하고 다음 라운드 버튼 대신 종료 버튼을 표시해야 합니다.

## 데이터 및 소유권 요구사항

- GameMode: 서버 권한 라운드·Wave·중단·정산·재개 규칙
- GameState: Match Phase, Round/Wave 공통 상태, 팀 결과, 정산 시간, 전투 중단 단계
- PlayerState: 개인 결과, 재화, 경험치, 레벨, 레벨업 후보·선택, 구매와 준비 상태
- Objective: 성 HP와 파괴 상태
- UI: Replication된 데이터의 표시와 소유 클라이언트 요청 전달

팀 공통 라운드 결과와 플레이어별 라운드 결과는 별도 구조로 정의해야 합니다.

## 이동 및 애니메이션 요구사항

- MVP 이동은 CharacterMovementComponent를 사용합니다.
- 기본 AnimBP locomotion을 제공합니다.
- 공격은 Montage와 AnimNotify 또는 Gameplay Event를 사용할 수 있습니다.
- Hit Reaction과 Death Animation은 서버가 확정한 상태를 기반으로 표시합니다.
- AnimNotify가 authoritative damage를 직접 적용하면 안 됩니다.

## 검증 요구사항

- C++ 변경은 build 또는 compile 결과를 보고합니다.
- Replication 변경은 Dedicated Server와 다중 클라이언트 관찰 결과가 필요합니다.
- 전투 중 레벨업은 감속, 완전 정지, 선택, 복귀 감속과 재개 순서를 검증해야 합니다.
- 정지 중 위치, Projectile, HP, 피해, 적 생성과 Wave 값이 변하지 않는지 확인해야 합니다.
- 과거 기반선의 검증과 새 기능의 미검증 상태를 구분해야 합니다.
- profiling 결과에는 측정 조건과 한계를 기록해야 합니다.

## 비기능 요구사항

- 변경은 작고 리뷰 가능해야 합니다.
- AI-assisted workflow는 승인 기반으로 통제합니다.
- 문서와 Codex 보고서는 가능한 한 한글로 작성하고 UTF-8을 유지합니다.
- 문서는 면접에서 실제 코드와 검증 증거로 설명 가능한 수준을 유지합니다.

## 향후 확장

Mass Entity, Mover, Motion Matching, Mutable은 MVP 필수 구현이 아니라 향후 확장 및 비교 학습 대상으로 유지합니다.
