# 진행 로그

## 현재 상태

필수 MVP 구현과 주요 검증은 완료되었습니다.

현재 프로젝트는 면접에서 설명 가능한 기술 데모 상태입니다.

## 완료된 작업

문서:

- 프로젝트 목표
- 요구사항
- 아키텍처
- MVP 작업 분해
- Git workflow
- AI harness
- Unreal/Mass/Coding rules
- 검증 기준
- profiling 기록
- visible demo 검증 기록
- runtime review evidence 기록
- PR #33 merge 및 feature branch cleanup 완료

구현:

- server-authoritative Objective Actor
- replicated Objective HP
- Dedicated Server target
- Mass module setup
- Mass spawn
- Mass movement
- Mass arrival detection
- once-only Objective damage integration
- runtime debug state subsystem
- Combat enemy spawn baseline
- phase-based profiling harness
- smoke verification script
- CommonUI 기반 debug overlay C++ 골격
- Widget Blueprint 연동용 `BindWidgetOptional` 필드와 제작 가이드
- CommonUI viewport client config 정리
- UI 화면 검증은 Widget Blueprint asset 생성 후 진행 필요
- 제출용 코드 샘플 정리 및 VS2022 탐색용 프로젝트 패키징
- 원본 프로젝트 기준 MDS v2 구조 정합성 리뷰

검증:

- UE 5.8 source engine build
- `MDSProjectEditor` compile
- `MDSProjectServer` compile
- `MDSProject` client compile
- dedicated server listen 확인
- server final Objective/Mass state 확인
- client replicated Objective HP 확인
- staged client/server Wave display state 확인
- client CommonUI viewport 설정 오류 제거 확인

최근 smoke 결과:

```text
SMOKE RESULT: PASS
WAVE VERIFY RESULT: PASS
```

## 현재 브랜치

```text
main
```

## 다음 후보 작업

- Runtime Review / Verification Evidence 추가 정리
- Widget Blueprint asset 생성 후 UI 화면 검증
- future network extension으로 client prediction / server reconciliation / server rewind를 단계별 검토
- 문서 한글화
- 추가 profiling 반복 측정 또는 Unreal Insights deeper capture는 필요 시 future/reference 작업으로만 진행

## 최근 구조 리뷰 메모

- Wave authority는 `AMDSProjectGameMode`가 담당하고, replicated Wave display state는 `AMDSProjectGameState`가 담당합니다.
- Objective HP는 `AMDSObjectiveActor`가 서버 권한으로 계산하고 `CurrentHealth`를 복제합니다.
- Enemy HP는 `AMDSCombatEnemyActor`가 별도 경로로 계산하며, death 상태는 `CurrentHealth <= 0.0f`에서 파생됩니다.
- `UMDSActorEnemySpawnSubsystem`은 v2 경로에서 `AMDSCombatEnemyActor`를 생성합니다. 삭제된 `AMDSActorEnemy`는 MVP runtime path에 남아 있지 않습니다.
- Debug overlay와 `UMDSDebugStateSubsystem`은 관찰/검증 보조이며 gameplay truth source가 아닙니다.
- CommonUI debug overlay runtime을 위해 `GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient` 설정을 사용합니다.
- Mass와 profiling 자료는 MVP 필수 구현이 아니라 reference/future extension으로 유지합니다.
- `mds-v2-objective-combat-demo` 브랜치는 PR #33으로 `main`에 merge된 뒤 로컬/원격에서 정리되었습니다.

## 주의사항

- `-NullRHI` profiling은 viewport/GPU 성능 주장이 아닙니다.
- 프로젝트는 완성형 게임이 아니라 기술 샌드박스입니다.
- visible demo는 replicated Objective HP 검증에 초점을 둡니다.
- 최신 runtime review evidence는 `Docs/11_Runtime_Review_Evidence.md`에 정리되어 있습니다.

## Recent Debug Overlay Verification

- Date: 2026-07-12
- Branch/PR scope: debug overlay runtime configuration verification
- Added `Run_Verify_DebugOverlayWidget.ps1`.
- Verified `WBP_MDSDebugOverlay` can be loaded, compiled, and saved by the editor script.
- Verified runtime configuration resolves `WBP_MDSDebugOverlay_C`.
- Verified standalone headless runtime reaches `MDS Debug | NetMode=Standalone` without a missing debug overlay class log, widget creation failure log, CommonUI viewport error, or fatal error.
- Added C++ fallback text layout for `UMDSDebugOverlayWidget` so an otherwise empty debug overlay Widget Blueprint still has visible debug text when constructed.
- Verified `Run_Verify_DebugOverlayWidget.ps1` result: `DEBUG OVERLAY VERIFY RESULT: PASS`.
- Verified `Run_Verify_WaveDisplayState.ps1` result: `WAVE VERIFY RESULT: PASS`.
- Added `Run_Verify_DebugOverlayViewport.ps1` to build, configure `BP_TopDownController`, cook/stage, launch the staged client, toggle `F1`, capture a viewport screenshot, and scan runtime logs.
- Reparented `BP_TopDownController` to `AMDSProjectPlayerController` so the TopDown map uses the project debug overlay runtime path.
- Added C++ default TopDown input asset references on `AMDSProjectPlayerController` and configure-script defaults on `BP_TopDownController`.
- Verified staged client viewport pixels with visible green `MDS Debug` overlay text in `SavedVerifyLogs/MDS_DebugOverlayViewport_Client_PrintWindow.png`.
- Verified `Run_Verify_DebugOverlayViewport.ps1` result: `DEBUG OVERLAY VIEWPORT VERIFY RESULT: PASS`.
- Known note: a generic `LogEnhancedInput: Warning: Called AddMappingContext with a null Mapping Context!` line remains from the existing TopDown Blueprint input path, but project-side missing input warnings are not emitted and the overlay viewport verification passes.
- Authored Widget Blueprint TextBlock layout remains a future polish task; the C++ fallback layout is verified visible.

## Recent Replicated UI Baseline

- Date: 2026-07-12
- Branch/PR scope: Match HUD / Objective World UI / Enemy World UI replicated UI baseline
- Added `UMDSMatchHUDWidget`, `UMDSObjectiveWorldWidget`, and `UMDSEnemyWorldWidget`.
- Match HUD reads `AMDSProjectGameState` Wave state directly.
- Objective World UI reads `AMDSObjectiveActor` replicated HP directly.
- Enemy World UI reads each `AMDSCombatEnemyActor` replicated HP directly and supports multiple spawned combat enemies.
- Added `Run_Verify_ReplicatedUIBaseline.ps1` to build client/server targets, cook/stage, launch dedicated server plus windowed client, enable actor enemy baseline, and scan replicated UI source logs.
- Changed `AMDSProjectGameMode` to use `AMDSProjectPlayerController` directly, removing the staged runtime dependency on loading `BP_TopDownController` as the controller class.
- Verified `Run_Verify_ReplicatedUIBaseline.ps1` result: `REPLICATED UI BASELINE VERIFY RESULT: PASS`.
- Verified `Run_Verify_WaveDisplayState.ps1` result: `WAVE VERIFY RESULT: PASS`.
- Verified `Run_Verify_DebugOverlayWidget.ps1` result: `DEBUG OVERLAY VERIFY RESULT: PASS`.

## Recent Replicated UI Viewport Verification

- Date: 2026-07-13
- Branch/PR scope: stricter staged viewport evidence for replicated UI baseline
- Added `Run_Verify_ReplicatedUIViewport.ps1` to build client/server/editor targets, cook/stage Win64 client and server, launch a dedicated server plus visible client, capture the client window, and scan replicated UI logs.
- The script now rejects title-bar-only or blank captures by checking visible pixels inside the client content area.
- Added explicit white fallback text with black shadow for Match HUD, Objective World UI, and Enemy World UI fallback widgets.
- Added explicit Match HUD viewport position and size so the fallback HUD is not dependent on implicit placement.
- Verified `MDSProjectEditor`, `MDSProject`, and `MDSProjectServer` Development builds succeeded.
- Verified Win64 client/server cook and stage succeeded.
- Verified runtime logs show Match HUD, Objective World UI, and Enemy World UI reading replicated GameState/Object/Enemy HP sources.
- Fixed C++ fallback UI root creation by moving fallback setup into `RebuildWidget` for Match HUD, Objective World UI, and Enemy World UI.
- Added a verification-only staged client engine screenshot request via `-MDSReplicatedUIViewportShot`.
- Verified `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png` shows Match HUD fallback text plus Objective World UI and Enemy World UI fallback text.
- Latest strict result: `REPLICATED UI VIEWPORT VERIFY RESULT: PASS`.

## Recent Actor-Following World UI Verification

- Date: 2026-07-17
- Branch/PR scope: PR #43, Objective/Enemy World UI follows owning actors.
- Kept Objective World UI and Enemy World UI as actor-attached screen-space `UWidgetComponent` presentation paths.
- Set explicit draw size and pivot so Objective and Enemy HP labels no longer collapse into a single center overlap in the staged viewport evidence.
- Added `-MDSWorldUITrackingLog` verification logging for short actor/widget/screen projection samples without enabling always-on runtime tracking logs.
- Added `MDSActorBaselineMoveSpeed` verification override so the staged viewport capture can show separated enemies before they converge on the objective.
- Verified `Run_Verify_ReplicatedUIViewport.ps1 -Port 7779 -ActorEnemyCount 4 -ActorEnemyMoveSpeed 30` result: `REPLICATED UI VIEWPORT VERIFY RESULT: PASS`.
- Verified latest logs show dedicated server listen on port `7779`, client join, `4/4` actor combat enemy spawn, Match HUD GameState reads, Objective World UI initialization, Enemy World UI initialization, and later `Projected=true` tracking samples for Objective and four Enemy UI labels.
- Verified `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png` shows Match HUD/debug text, an Objective HP label near the objective, and four separated Enemy HP labels around the objective.
- Notes: this is visible placement and replicated UI evidence, not authored UI polish, animation/death presentation, Mass profiling, or performance evidence.

## Recent Player Attack Runtime Verification

- Date: 2026-07-19
- Branch/PR scope: player attack request, server validation, Enemy HP replication, and negative reject verification.
- Added owning-client `ServerRequestAttack` flow on `AMDSProjectPlayerController`.
- Added command-line gated `-MDSAutoAttackNearestEnemy` verification harness that selects a replicated enemy on the client and still uses the same server RPC path.
- Added `Run_Verify_PlayerAttack.ps1`.
- Verified staged dedicated server/client runtime with `Run_Verify_PlayerAttack.ps1`.
- Verified `Valid` scenario: four accepted player attacks apply server-owned Enemy HP damage `100 -> 75 -> 50 -> 25 -> 0`, replicate Enemy HP to the client, handle HP-derived enemy death, and decrement Wave remaining `1 -> 0`.
- Verified `OutOfRange` negative scenario: server rejects the request with `Reason=OutOfRange`, with zero valid attacks and zero `PlayerAttack` damage.
- Verified `Cooldown` negative scenario: one valid damage applies `100 -> 90`, the next request is rejected with `Reason=Cooldown`, and no extra damage is applied.
- Verified latest script result: `PLAYER ATTACK VERIFY RESULT: PASS`.
- Notes: this is runtime/log evidence, not Attack Montage, AnimNotify, Hit Reaction, or Death Animation presentation evidence. Additional reject branches such as InvalidTarget, InvalidDamage, DeadTarget, and NoPawn remain unverified.

## Recent Combat Presentation Hook Verification

- Date: 2026-07-19
- Branch/PR scope: minimal C++ combat presentation hooks for attack, hit, and death trigger evidence.
- Added presentation-only local attack hook on `AMDSProjectCharacter`.
- `AMDSProjectPlayerController` requests local attack presentation for manual and auto attack intent without changing the server-authoritative damage path.
- Added command-line gated `-MDSPresentationOnlyAttackMarker` negative path that triggers presentation markers without sending a server attack RPC.
- `AMDSCombatEnemyActor` now uses replicated `CurrentHealth` previous value in `OnRep_CurrentHealth` to request client-only hit/death presentation hooks.
- Death presentation uses a client-only transient guard separate from server Wave death handling.
- Added `Run_Verify_CombatPresentationHooks.ps1`.
- Verified `Run_Verify_CombatPresentationHooks.ps1 -Port 7798` result: `COMBAT PRESENTATION VERIFY RESULT: PASS`.
- Re-verified `Run_Verify_PlayerAttack.ps1 -Port 7800 -SkipBuild -SkipStage` result: `PLAYER ATTACK VERIFY RESULT: PASS`.
- Notes: this is C++ hook/log evidence only. Real Attack Montage playback, real AnimNotify asset firing, authored Hit Reaction, authored Death Animation, and viewport-visible animation pose changes remain unverified.

## Recent Combat Animation Asset Readiness Verification

- Date: 2026-07-20
- Branch/PR scope: read-only verification for reusing existing attack, hit reaction, and death animation assets.
- Added `Tools/VerifyCombatAnimationAssets.py`.
- Added `Run_Verify_CombatAnimationAssets.ps1`.
- Fixed Unity build helper-name collisions by making combat presentation log helpers file-specific.
- Verified `MDSProjectEditor Win64 Development` build succeeded; UBT reported log path `C:\UnrealEngine\Engine\Programs\UnrealBuildTool\Log.txt`.
- Verified `Run_Verify_CombatAnimationAssets.ps1 -SkipBuild` result: `COMBAT ANIMATION ASSET VERIFY RESULT: PASS_WITH_INCOMPLETE_ITEMS`.
- Re-verified `Run_Verify_CombatPresentationHooks.ps1 -Port 7804 -SkipBuild -SkipStage -ClientWaitSeconds 28` result: `COMBAT PRESENTATION VERIFY RESULT: PASS`.
- Existing `BP_TopDownCharacter` uses `SKM_Manny_Simple`, `ABP_Unarmed_C`, and `SK_Mannequin`.
- Existing attack candidates include four unarmed `AnimSequence` assets and one pistol `AnimMontage`, all compatible with `SK_Mannequin`.
- Existing hit reaction and death candidates load as `AnimSequence` assets and are compatible with `SK_Mannequin`.
- Notes: this is asset loadability/skeleton compatibility evidence only. Attack notify readiness remains incomplete because no authored/readable attack notify was found in the checked candidates. `BP_TopDownCharacter` parent lineage was not proven by the checked Editor Python APIs. This is not a full animation readiness pass. Runtime animation playback and viewport-visible pose changes remain unverified.

## Recent Combat Animation Playback Attempt Verification

- Date: 2026-07-20
- Branch/PR scope: existing asset playback API attempt/acceptance evidence for combat presentation.
- Added client-side attack montage playback attempt logging from `AMDSProjectPlayerController`.
- Added a visual-only skeletal mesh presentation component to `AMDSCombatEnemyActor` using existing mannequin mesh/AnimBP assets.
- Added client-side hit reaction and death animation playback attempt logging from replicated Enemy HP presentation hooks.
- Added `/Game/Characters/Mannequins/Anims` to packaging always-cook directories so existing animation assets are available in staged clients.
- Extended `Run_Verify_CombatPresentationHooks.ps1` to require attack/hit/death animation playback attempt success counts and to assert zero server animation playback attempts.
- Verified `Run_Verify_CombatPresentationHooks.ps1 -Port 7808 -ClientWaitSeconds 28` result: `COMBAT PRESENTATION VERIFY RESULT: PASS` and `COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS`.
- Re-verified `Run_Verify_CombatPresentationHooks.ps1 -Port 7815 -SkipBuild -SkipStage -ClientWaitSeconds 28` after enabling `-MDSCombatPresentationLog` on the server launch and tightening exact count / success count / rejected RPC checks; result remained `COMBAT PRESENTATION VERIFY RESULT: PASS` and `COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS`.

## Recent Combat Animation Visible Capture Verification

- Added verification-only `-MDSCombatAnimationVisibleShot` capture hooks for the first successful client attack, hit, and death animation playback events.
- Added `Run_Verify_CombatAnimationVisibleCapture.ps1` to launch a staged dedicated server plus visible staged client, run the existing auto attack flow, request event-timed engine screenshots, and fail closed on missing/blank screenshots.
- Verified `Run_Verify_CombatAnimationVisibleCapture.ps1 -Port 7822 -SkipBuild -SkipStage -ClientWaitSeconds 24` result: `COMBAT ANIMATION VISIBLE CAPTURE VERIFY RESULT: PASS_WITH_POSE_LIMITATION`.
- Screenshot artifacts:
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Attack.png`
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Hit.png`
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Death.png`
- This is viewport capture evidence correlated with playback logs, not frame-accurate pose-delta proof or authored AnimNotify verification.
- Historical note: the later persistent AnimNotify and paired pose-delta passes supersede these limitations.
- Re-verified `Run_Verify_PlayerAttack.ps1 -Port 7810 -SkipBuild -SkipStage` result: `PLAYER ATTACK VERIFY RESULT: PASS`.
- Verified `git diff --check` passed with CRLF warnings only.
- Valid scenario observed attack playback `4/4`, hit playback `3/3`, and death playback `1/1` on the client, all with `GameplayDamage=false`.
- Presentation-only scenario observed one attack playback attempt with zero server valid attacks, zero PlayerAttack damage, and zero Enemy HP replication.
- Notes: this is playback API attempt/acceptance evidence, not viewport-visible pose-change evidence or real AnimNotify firing evidence.

## Recent Runtime AnimNotify Verification

- Date: 2026-07-21
- Added presentation-only `UMDSCombatTimingAnimNotify`.
- Added command-line gated `-MDSCombatAnimNotifyVerify` transient montage Notify injection so normal runtime assets and behavior remain unchanged.
- Added `Run_Verify_CombatAnimNotify.ps1` and fail-closed client/server Notify count checks.
- Verified `MDSProjectEditor`, `MDSProject`, and `MDSProjectServer` Win64 Development builds succeeded.
- Verified Win64 client and WindowsServer cook/stage succeeded.
- Verified `Run_Verify_CombatAnimNotify.ps1 -Port 7832 -ClientWaitSeconds 24 -SkipBuild -SkipStage` result: `COMBAT ANIMNOTIFY VERIFY RESULT: PASS`.
- Valid scenario: client runtime Notify fired `4/4`, server Notify fired `0`, and server-owned damage applied `4/4` through the existing RPC path.
- Presentation-only scenario: client runtime Notify fired `1/1`, server Notify fired `0`, server valid attacks `0`, PlayerAttack damage `0`, and Enemy HP replication `0`.
- Re-verified `Run_Verify_CombatPresentationHooks.ps1 -Port 7834 -ClientWaitSeconds 24 -SkipBuild -SkipStage`: `COMBAT PRESENTATION VERIFY RESULT: PASS`.
- Re-verified `Run_Verify_PlayerAttack.ps1 -Port 7839 -SkipBuild -SkipStage`: `PLAYER ATTACK VERIFY RESULT: PASS`.
- This verifies actual runtime Notify dispatch and authority isolation. It does not prove a persistent authored Notify stored in the source montage asset.

## Character Movement Replication Verification Attempt

- Date: 2026-07-21
- Added command-line gated movement snapshots and automated TopDown movement input.
- Added `Run_Verify_CharacterMovementReplication.ps1` for one dedicated server, one observer client, and one mover client.
- Verified two clients connected (`4` server login/join markers) with no fatal error.
- Verified role topology: server `Authority`, mover `AutonomousProxy`, observer `SimulatedProxy`.
- Result: `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: INCOMPLETE`.
- Blocking evidence: mover, server, and observer each reported maximum `DistanceFromStart=0` and `Speed2D=0`; both direct movement input and the existing TopDown `SimpleMoveToLocation` path failed to move the packaged verification pawn.
- Regression verification remained green: `COMBAT PRESENTATION VERIFY RESULT: PASS` and `PLAYER ATTACK VERIFY RESULT: PASS`.
- Next investigation is the pawn/CharacterMovement input-consumption and navigation/path-following configuration. AnimBP locomotion pose evidence remains a separate later task.
- Follow-up diagnostics confirmed `BP_TopDownCharacter_C` reports native parent `Character`, not `MDSProjectCharacter`. Its CMC is active and ticking with `MaxWalkSpeed=600` and `MaxAcceleration=1000`; forced input reaches Pending Input (`Y=1`) but is not observed as consumed Last Input. The packaged client also reports no NavigationSystem, NavData, or PathFollowingComponent, so its `SimpleMoveToLocation` request cannot run.
- `BP_TopDownCharacter` was reparented, compiled, and saved with `AMDSProjectCharacter` as its native parent. Client/server build and cook/stage succeeded afterward. Navigation-independent `AddMovementInput` still remained unconsumed in the packaged AutonomousProxy, and a verification-only `RequestDirectMove` probe also failed to exceed `0.1` client distance, so it was removed rather than retained as a workaround. Phase A-1 movement remains fail-closed and incomplete.

## Character Movement Replication Verification Resolution

- Date: 2026-07-21
- Added `IA_Move` as an Enhanced Input Axis2D action and mapped W/A/S/D in `IMC_Default`; existing click/touch movement remains intact.
- Bound the action in `AMDSProjectCharacter` and routed both runtime input and command-line verification through the same CMC movement helper.
- Moved automated input injection to the owning controller's `PlayerTick`, before `Super::PlayerTick`, so CMC consumes input during the normal movement tick window.
- Result: `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS`.
- Dedicated-server evidence: mover `AutonomousProxy`, server `Authority`, and observer `SimulatedProxy` all reached maximum distance/speed `1620.5 / 600`; input accepted/consumed was `True / True`, with no fatal error.
- Regression results: `COMBAT PRESENTATION VERIFY RESULT: PASS`, `COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS`, and `PLAYER ATTACK VERIFY RESULT: PASS`.
- Scope note: this proves CMC input consumption and network movement replication. Physical keyboard-event and AnimBP locomotion-pose viewport evidence remain manual follow-up checks.

## Persistent Authored Attack AnimNotify Resolution

- Date: 2026-07-21
- Authored one `UMDSCombatTimingAnimNotify` into `MM_Pistol_Fire_Montage` at configured time `0.100` seconds; montage play length is `0.667` seconds.
- Added idempotent `Tools/ConfigureCombatAnimationNotify.py`; a second independent run reported `EXISTING | Count=1` and did not add a duplicate.
- Removed the command-line transient montage Notify injection from `AMDSProjectPlayerController`. The verification flag now gates diagnostic logging only.
- Asset evidence: authored Notify class/readability `True`, count `1`, configured time in range `True`. UE Python does not expose the stored `FAnimNotifyEvent.LinkValue`, so runtime firing supplies the independent timing evidence.
- Runtime result: `COMBAT ANIMNOTIFY VERIFY RESULT: PASS`. Valid scenario fired client Notify `4/4`, server `0`; presentation-only scenario fired client Notify `1/1`, server `0`, with server request/damage/HP replication all `0`.
- Regression results: `COMBAT PRESENTATION VERIFY RESULT: PASS`, `COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS`, and `PLAYER ATTACK VERIFY RESULT: PASS`.
- The earlier transient/incomplete entries above are retained as historical evidence and are superseded by this result.

## Directional Movement and Fire-Facing Verification

- Date: 2026-07-22
- Replaced desktop click-to-move/touch movement bindings with independent WASD movement and left-mouse directional fire.
- W/S/A/D now produce top/down/left/right world-plane movement; diagonal input is normalized before entering the existing CMC helper.
- Directional fire is valid without a target. The server resolves a miss with zero damage or applies damage to the closest alive enemy inside the approved directional hitscan radius and range.
- Enemy HP, cooldown, death, and Wave progression remain server-authoritative.
- Normal character facing follows CMC movement direction. Left-mouse fire temporarily faces the cursor direction for the attack montage duration, then restores movement-direction facing.
- Dedicated Server automated results: `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS` and `PLAYER ATTACK VERIFY RESULT: PASS`.
- Visible Dedicated Server + two-client result: mover/server/observer maximum distance was `1585.5 / 1588.7 / 1629.0`; maximum speed was `600 / 600 / 600.4`; fatal error was `False`.
- Visible directional fire recorded `19` valid shots: `3` hits and `16` misses.
- Manual viewport review confirmed W/S/A/D direction, diagonal movement, movement-direction facing, temporary cursor-facing during fire, return to movement-direction facing, and observer-visible locomotion/fire presentation.

## Directional Fire Reject Verification

- Date: 2026-07-22
- Added command-line-gated `InvalidDirection`, `InvalidDamage`, and `NoPawn` scenarios to `Run_Verify_PlayerAttack.ps1` without changing normal input or server attack resolution behavior.
- Client and Dedicated Server Development builds succeeded; Windows and WindowsServer cook/stage succeeded.
- `Run_Verify_PlayerAttack.ps1 -Scenario Rejects -Port 7900 -SkipBuild -SkipStage` result: all three scenarios and the aggregate result passed.
- Each scenario produced exactly one expected server rejection and zero valid attacks, damage events, Enemy HP replication, enemy deaths, or Wave death consumption. Connections succeeded and fatal error was `False`.
- Normal attack regression passed with four valid hits and four client HP observations; CharacterMovement replication regression passed at `1620.5 / 600` on mover, server, and observer.

## Simulated Client Attack Presentation Verification

- Date: 2026-07-22
- Added a `-MDSCombatPresentationLog`-gated remote receipt log containing NetMode, local/remote role, direction, and presentation duration.
- Added `Run_Verify_SimulatedClientAttackPresentation.ps1` for Dedicated Server, observer, and attacker execution.
- Result: `SIMULATED CLIENT ATTACK PRESENTATION VERIFY RESULT: PASS`.
- Four server-confirmed hits produced four observer `SimulatedProxy` direction receipts, four successful remote montage playbacks, four presentation hooks, and four replicated HP observations.
- Owning-client remote playback and Dedicated Server animation playback were both `0`; server damage count was `4` and fatal/ensure was `False`.
- Regression results: `PLAYER ATTACK VERIFY RESULT: PASS` and `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS`.

## Combat Animation Pose Delta Verification

- Date: 2026-07-22
- Added command-line-gated paired `Before`/`Pose` viewport captures for Attack, Hit, and Death.
- Final result: `COMBAT ANIMATION POSE DELTA VERIFY RESULT: PASS`.
- Changed center-gameplay samples: Attack `75`, Hit `58`, Death `803`; all six images existed, were nonempty/visible, and shared compatible dimensions.
- Server animation playback remained `0`; valid attacks/damage/HP replication were `4/4/4` with no fatal error.

## Authored Gameplay UI Styling

- Date: 2026-07-23
- Updated `Tools/CreateGameplayUIWidgets.py` to apply idempotent font, color, shadow, alignment, and padding styles to the existing bound TextBlocks.
- Match HUD uses cyan 28/24pt text, Objective HP uses gold 22pt text, and Enemy HP uses red 20pt text.
- Two Editor-Cmd passes reused the existing widgets and compiled/saved all three Widget Blueprints without duplicates or Python errors.
- The full viewport harness exceeded the tool wrapper timeout during its final wait, but completed cook/stage and generated the latest EngineShot. Every underlying PASS condition was independently reconstructed as true; CommonUI/fatal errors were false.
- Manual image review confirmed readable color separation and actor-following placement for the Objective and four Enemy labels.

## Continuous Wave Demo Loop

- Date: 2026-07-23
- Normal server play now schedules and runs three server-authoritative waves with `3 -> 4 -> 5` real combat enemies and a three-second intermission.
- Enemy deaths decrement the replicated GameState count once; clearing a wave schedules the next wave, and Wave 3 emits the final demo-complete event.
- `-MDSAutoStartWave` retains one-shot verification precedence. `-NoMDSWaveLoop` isolates unrelated verification scenarios.
- Default combat-enemy movement speed is `100`; verification may still override it with `MDSActorBaselineMoveSpeed`.
- Client and Server Development builds and Windows/WindowsServer cook-stage succeeded.
- Dedicated Server verification passed: 12 spawned enemies were killed, three waves cleared, final completion occurred once, and the client observed active Wave 1, 2, and 3 state.
- Regression results: PlayerAttack Valid `PASS`; simulated-client attack presentation `PASS`. CharacterMovement replication connected both clients without fatal errors but was `INCOMPLETE` because its command-line auto-move trigger did not start during that run.

## Enemy Movement, Fire Feedback, Death Lifecycle, and Mass Isolation

- Date: 2026-07-23
- Converted `AMDSCombatEnemyActor` from collision-free `AActor` translation to `ACharacter` walking movement.
- Enemy capsules block world geometry and overlap Pawn capsules, allowing terrain walking while enemies can overlap one another.
- Enemy locomotion uses the existing `ABP_Unarmed`; movement orientation follows velocity.
- Owning-client fire now immediately plays facing/montage presentation and a short yellow directional tracer before server confirmation.
- Damage, Enemy HP, hit/death presentation triggers, and Wave progression remain server-authoritative.
- Dead enemies stop movement and collision immediately, hold the death pose for two seconds, then sink/fade for one second before server removal.
- Mass baseline now defaults off and requires explicit `-MDSMassBaseline` or CVar enablement.
- Editor, Client, and Server Development builds succeeded; Windows and WindowsServer cook-stage succeeded.
- Continuous Wave regression passed with 12 deaths, 12 server/client fade starts, three Wave clears, and no fatal/ensure.
- Normal execution produced zero Mass initialization events; explicit `-MDSMassBaseline MDSMassBaselineCount=2` produced exactly two entities.
- Simulated-client attack presentation regression passed.

## 3D Aim, Enemy Movement, and Persistent Death Pose Fixes

- Date: 2026-07-23
- Replaced horizontal direction-only attack requests with a quantized 3D cursor AimPoint.
- Server reconstructs and clamps the 3D shot segment to the default `5000` range and evaluates enemy distance to that segment, allowing high-to-low shots.
- Owning-client tracer now ends at the actual cursor Visibility impact point; remote presentation uses the server-clamped endpoint.
- Controller-free enemy movement now uses swept `CharacterMovementComponent` movement with collision-normal projection for terrain following.
- Runtime movement verification initialized three enemies at speed `100`; all three reached the Objective after approximately 10.4 seconds at terrain height `Z=307.96`.
- Death montage is paused near its final pose before the two-second body hold. Continuous Wave verification observed `12/12` client death-pose freezes and `12/12` fade starts.
- Client/Server builds, Windows/WindowsServer stages, three-wave progression, and InvalidDirection rejection passed without fatal/ensure.

## Directional Target Priority and Hit Pause

- Date: 2026-07-23
- Cursor impact now defines the XY fire direction rather than limiting shot length.
- Owning client and server search the full `5000` range and select the nearest alive enemy inside the directional corridor, including enemies beyond the clicked point.
- Tracer ends at the predicted/confirmed enemy when one exists; otherwise it ends at the clicked terrain impact.
- Nonlethal hits pause server enemy movement for `0.35` seconds; the timer resets on repeated hits.
- Player attack montage playback is routed through the Character presentation path for both owning and simulated clients.
- Client/Server build and stage succeeded; Continuous Wave and PlayerAttack Valid passed.
- Four valid attacks produced four damage/HP replication events and three movement resumes after the three nonlethal hits.
