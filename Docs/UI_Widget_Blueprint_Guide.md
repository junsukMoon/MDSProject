# UI Widget Blueprint Guide

이 문서는 `UMDSDebugOverlayWidget`을 부모 클래스로 하는 Widget Blueprint 제작 절차를 기록합니다.

목표는 C++ gameplay/debug state와 Blueprint presentation을 분리하는 것입니다.

## v2 UI Scope Note

`UMDSDebugOverlayWidget`은 debug overlay입니다. MDS v2 gameplay UI와는 책임이 다릅니다.

v2 gameplay UI 구분:

- Match HUD: `AMDSProjectGameState`의 Wave state 표시
- Objective World UI: `AMDSObjectiveActor`의 Objective HP 표시
- Enemy World UI: combat enemy actor의 Enemy HP 표시

Debug overlay:

- `UMDSDebugStateSubsystem` snapshot 표시
- NetMode, Objective HP, reference debug counter 확인
- v2 Objective/Wave/Combat debug 후보 표시
- 면접/검증 보조용

Debug overlay는 gameplay truth source가 아니며, HP / Damage / Wave state를 직접 변경하지 않습니다.
Match HUD, Objective World UI, Enemy World UI를 대체하지 않습니다.

## 생성 절차

1. Unreal Editor에서 Widget Blueprint를 생성합니다.
2. Parent Class를 `MDSDebugOverlayWidget`으로 설정합니다.
3. 원하는 레이아웃에 TextBlock을 배치합니다.
4. TextBlock 변수 이름을 아래와 정확히 맞춥니다.

```text
NetModeTextBlock
ObjectiveHealthTextBlock
WaveSummaryTextBlock
MassSummaryTextBlock
ActorSummaryTextBlock
```

5. 각 TextBlock은 `Is Variable`을 활성화합니다.
6. PlayerController Blueprint에서 `DebugOverlayWidgetClass`를 새 Widget Blueprint로 지정합니다.
7. PIE 또는 client 실행 중 `F1` 키로 overlay를 토글합니다.

## TextBlock 바인딩

1. Widget Blueprint 안에 TextBlock 4개를 배치합니다.
2. TextBlock 변수 이름을 아래와 정확히 맞춥니다.

```text
NetModeTextBlock
ObjectiveHealthTextBlock
WaveSummaryTextBlock
MassSummaryTextBlock
ActorSummaryTextBlock
```

3. 각 TextBlock은 `Is Variable`을 활성화합니다.

`WaveSummaryTextBlock`은 v2 Wave display state 확인용입니다. 기존 Widget Blueprint에 이 TextBlock이 없어도 `BindWidgetOptional` 때문에 위젯 생성은 실패하지 않습니다.

## PlayerController 연결

Widget Blueprint를 만든 뒤:

1. PlayerController Blueprint를 엽니다.
2. `DebugOverlayWidgetClass`를 새 Widget Blueprint로 지정합니다.
3. PIE 또는 client 실행 중 `F1` 키로 overlay를 토글합니다.

## C++ 연결 방식

`UMDSDebugOverlayWidget`은 `BindWidgetOptional`을 사용합니다.

Widget Blueprint에 위 이름의 TextBlock이 있으면 자동으로 연결됩니다. TextBlock이 없더라도 C++ getter는 계속 동작하며, 위젯 생성이 실패하지 않습니다.

## 표시 데이터

- NetMode
- Objective HP
- Wave index / EnemiesRemaining 후보
- last combat validation result 후보
- last Objective damage event 후보
- Mass Spawned / Moved / Arrived / Damage
- Actor Spawned / ActiveTicks / Arrived / Damage

데이터 원본은 `UMDSDebugStateSubsystem`입니다. UI는 gameplay state를 변경하지 않고 관찰만 합니다.
Mass / Actor counters는 v1/reference debug로 취급합니다. v2 gameplay UI의 truth source는 GameState, ObjectiveActor, combat enemy actor의 replicated state입니다.

## 면접 설명 포인트

- server-authoritative gameplay state를 UI가 직접 수정하지 않도록 분리했습니다.
- Debug overlay는 local player controller에서만 생성되며 dedicated server 경로에는 의존하지 않습니다.
- C++은 데이터 공급과 refresh lifecycle을 담당하고, Widget Blueprint는 presentation을 담당합니다.
- `BindWidgetOptional`을 사용해 코드 샘플과 실제 UI asset 제작 흐름을 느슨하게 결합했습니다.
