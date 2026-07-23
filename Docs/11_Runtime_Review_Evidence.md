# Runtime Review Evidence

이 문서는 MDS v2 runtime correctness evidence를 정리합니다.

MDS v2 MVP에서 이 문서는 formal profiling report가 아닙니다. 목적은 Dedicated Server 환경에서 server-owned state가 client에 복제되고, debug/runtime UI 기반 검증을 방해하던 CommonUI viewport 설정 문제가 제거되었음을 기록하는 것입니다.

## Runtime

- Date: 2026-07-10
- Engine: UE 5.8 source build
- Server: staged `MDSProjectServer.exe`
- Client: staged `MDSProject.exe`
- Map: `/Game/TopDown/Lvl_TopDown`
- Script: `Run_Verify_WaveDisplayState.ps1`
- Logs:
  - `SavedVerifyLogs/MDS_WaveVerify_Server.log`
  - `SavedVerifyLogs/MDS_WaveVerify_Client.log`

## Launch Setup

Dedicated server:

```text
MDSProject/Saved/StagedBuilds/WindowsServer/MDSProject/Binaries/Win64/MDSProjectServer.exe
/Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -port=7777
```

Client:

```text
MDSProject/Saved/StagedBuilds/Windows/MDSProject/Binaries/Win64/MDSProject.exe
127.0.0.1:7777 -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush
```

## Result

```text
WAVE VERIFY RESULT: PASS
```

## Server Evidence

Server listen:

```text
IpNetDriver listening on port 7777
```

Wave display state initialized and set on the server:

```text
Wave state set on server: Wave=1 Remaining=0 Total=0 Active=false.
Initialized wave display state on server: Wave=1 Remaining=0 Total=0 Active=false.
```

Final server-observed debug state:

```text
MDS Debug | NetMode=DedicatedServer | Wave=1 Active=false Remaining=0/0 | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16 | Actor Spawned=0 ActiveTicks=0 Arrived=0 Damage=0
```

Client join:

```text
Join succeeded
```

## Client Evidence

Client-observed replicated state:

```text
MDS Debug | NetMode=Client | Wave=1 Active=false Remaining=0/0 | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0 | Actor Spawned=0 ActiveTicks=0 Arrived=0 Damage=0
```

Interpretation:

- Client observes replicated Wave display state as `Wave=1 Active=false Remaining=0/0`.
- Client observes replicated Objective HP as `20/100`.
- Client Mass counters remain `0` because those counters are local debug/reference values, not client gameplay authority.

## CommonUI Check

Before the viewport config fix, the staged client log reported:

```text
Using CommonUI without a CommonGameViewportClient derived game viewport client.
```

After adding this project config and re-staging the client, that error no longer appears in the latest client verification log:

```ini
GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient
```

Remaining client log entries such as missing `aqProf.dll`, `VtuneApi.dll`, and `WinPixGpuCapturer.dll` are environment/tooling load messages observed during headless runtime and are not treated as gameplay replication failures.

## Debug Overlay Widget Check

- Date: 2026-07-12
- Script: `Run_Verify_DebugOverlayWidget.ps1`
- Logs:
  - `SavedVerifyLogs/MDS_DebugOverlayAsset.log`
  - `SavedVerifyLogs/MDS_DebugOverlayRuntime.log`

Result:

```text
DEBUG OVERLAY VERIFY RESULT: PASS
```

Evidence:

```text
MDSDebugOverlayAsset: Loaded existing widget blueprint /Game/MDS/UI/WBP_MDSDebugOverlay
MDSDebugOverlayAsset: Compiled and saved widget blueprint
MDSDebugOverlayAsset: Done
Debug overlay widget class configured as WBP_MDSDebugOverlay_C.
MDS Debug | NetMode=Standalone | Wave=1 Active=false Remaining=0/0 | ObjectiveHP=20/100
```

Interpretation:

- The debug overlay Widget Blueprint asset exists and can be compiled/saved by the editor script.
- `AMDSProjectPlayerController` resolves `WBP_MDSDebugOverlay_C` as its default debug overlay widget class.
- `UMDSDebugOverlayWidget` now builds a C++ fallback text layout when a Widget Blueprint has no bound debug TextBlocks.
- A standalone headless runtime reaches the debug state reporting path without a missing debug overlay class log, widget creation failure log, CommonUI viewport error, or fatal error.
- This check verifies build/runtime configuration and fallback layout code compilation. It does not verify visual pixels or F1 input.

## Debug Overlay Viewport Check

- Date: 2026-07-12
- Script: `Run_Verify_DebugOverlayViewport.ps1`
- Logs:
  - `SavedVerifyLogs/MDS_DebugOverlayControllerConfig.log`
  - `SavedVerifyLogs/MDS_DebugOverlayViewport_Client.log`
- Screenshot:
  - `SavedVerifyLogs/MDS_DebugOverlayViewport_Client_PrintWindow.png`

Result:

```text
DEBUG OVERLAY VIEWPORT VERIFY RESULT: PASS
```

Evidence:

```text
MDSOverlayControllerConfig: /Game/TopDown/Blueprints/BP_TopDownController already uses /Script/MDSProject.MDSProjectPlayerController
Debug overlay widget class configured as WBP_MDSDebugOverlay_C.
Debug overlay fallback layout initialized on WBP_MDSDebugOverlay_C
Debug overlay widget created on BP_TopDownController_C using WBP_MDSDebugOverlay_C.
MDS Debug | NetMode=Standalone | Wave=1 Active=false Remaining=0/0 | ObjectiveHP=20/100
```

Interpretation:

- `BP_TopDownController` now derives from `AMDSProjectPlayerController`, so the debug overlay runtime path is active in the staged TopDown map.
- The staged Win64 client creates the debug overlay widget, initializes the C++ fallback text layout, accepts the `F1` toggle path, and renders visible viewport pixels.
- `PrintWindow` captured the staged client window with the green `MDS Debug` overlay text visible in the viewport.
- The controller now has C++ default references for the TopDown input mapping context, click action, touch action, and cursor FX, and the configure script writes the same defaults to `BP_TopDownController`.
- A generic `LogEnhancedInput: Warning: Called AddMappingContext with a null Mapping Context!` line still appears from the existing TopDown Blueprint input path. No project-side missing input warnings are emitted, and this warning is not treated as a debug overlay viewport failure.

## Replicated UI Baseline Check

- Date: 2026-07-12
- Script: `Run_Verify_ReplicatedUIBaseline.ps1`
- Logs:
  - `SavedVerifyLogs/MDS_ReplicatedUI_Server.log`
  - `SavedVerifyLogs/MDS_ReplicatedUI_Client.log`

Result:

```text
REPLICATED UI BASELINE VERIFY RESULT: PASS
```

Evidence:

```text
Combat enemy wave spawn created 4/4 enemies around objective MDS_ActorObjectiveProbe
MDS Match HUD read GameState wave state: Wave=1 Remaining=0 Total=0 Active=false.
MDS Objective World UI read ObjectiveActor health: 20.0 / 100.0
MDS Enemy World UI read CombatEnemy health: 100.0 / 100.0
Objective HP replicated on client: 20.0 / 100.0.
```

Interpretation:

- Match HUD reads replicated Wave display state from `AMDSProjectGameState`.
- Objective World UI reads replicated Objective HP from each `AMDSObjectiveActor`.
- Enemy World UI reads replicated Enemy HP from each `AMDSCombatEnemyActor`; the verification used four combat enemies to avoid a single-target-only path.
- These UI widgets only observe state and do not mutate HP, damage, Wave, or combat state.
- `AMDSProjectGameMode` now uses `AMDSProjectPlayerController` directly, removing the staged client dependency on loading `BP_TopDownController` during GameMode CDO construction.

## Replicated UI Viewport Check

- Date: 2026-07-13
- Script: `Run_Verify_ReplicatedUIViewport.ps1`
- Logs:
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Server.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client.log`
- Screenshot:
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_PrintWindow.png`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png`

Result:

```text
REPLICATED UI VIEWPORT VERIFY RESULT: PASS
```

Evidence:

```text
Combat enemy wave spawn created 4/4 enemies around objective MDS_ActorObjectiveProbe
MDS Match HUD read GameState wave state: Wave=1 Remaining=0 Total=0 Active=false.
MDS Match HUD widget created on MDSProjectPlayerController
MDS Objective World UI read ObjectiveActor health: 20.0 / 100.0
MDS Enemy World UI read CombatEnemy health: 100.0 / 100.0
Objective HP replicated on client: 80.0 / 100.0.
MDS replicated UI viewport screenshot requested: .../SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png
Engine screenshot has visible pixels: True
```

Interpretation:

- `MDSProjectEditor`, `MDSProject`, and `MDSProjectServer` Development builds succeeded.
- Win64 client/server cook and stage succeeded.
- The staged dedicated server spawned four combat enemies for the actor baseline source.
- The staged client created the Match HUD and initialized Objective/Enemy world UI widgets.
- The staged client read replicated Wave, Objective HP, and Enemy HP sources.
- The viewport verification script now requests an engine screenshot from the staged client and requires visible pixels from that screenshot.
- `MDS_ReplicatedUIViewport_Client_EngineShot.png` shows the Match HUD fallback text plus Objective World UI and Enemy World UI fallback text in the connected staged client.
- Fallback UI root widgets are now created during `RebuildWidget`, before Slate builds the widget tree, so the C++ fallback UI is actually rendered.

## Actor-Following World UI Viewport Check

- Date: 2026-07-17
- PR: #43 (`Make world UI follow owning actors`)
- Script: `Run_Verify_ReplicatedUIViewport.ps1 -Port 7779 -ActorEnemyCount 4 -ActorEnemyMoveSpeed 30`
- Logs:
  - `SavedVerifyLogs/MDS_GameplayUIAsset.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Server.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client.log`
- Screenshot:
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png`

Result:

```text
REPLICATED UI VIEWPORT VERIFY RESULT: PASS
```

Evidence:

```text
IpNetDriver listening on port 7779
Combat enemy wave spawn created 4/4 enemies around objective MDS_ActorObjectiveProbe at V(0). Total spawned=4.
Login request
Join succeeded
MDS Match HUD read GameState wave state: Wave=1 Remaining=0 Total=0 Active=false.
Objective World UI widget initialized ... using WBP_MDSObjectiveWorldUI_C.
Enemy World UI widget initialized ... using WBP_MDSEnemyWorldUI_C.
ObjectiveWorldUITrack Actor=... ActorWorld=... WidgetWorld=... Screen=... Projected=true WidgetClass=WBP_MDSObjectiveWorldUI_C.
EnemyWorldUITrack Actor=... ActorWorld=... WidgetWorld=... Screen=... Projected=true WidgetClass=WBP_MDSEnemyWorldUI_C.
MDS replicated UI viewport screenshot requested: .../SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png
```

Widget Blueprint asset evidence:

```text
Compiled and saved widget blueprint /Game/MDS/UI/WBP_MDSMatchHUD
Compiled and saved widget blueprint /Game/MDS/UI/WBP_MDSObjectiveWorldUI
Compiled and saved widget blueprint /Game/MDS/UI/WBP_MDSEnemyWorldUI
```

Interpretation:

- Objective World UI and Enemy World UI remain presentation-only `UWidgetComponent` paths attached to their owning actors.
- The staged client logged actor world position, widget world position, and projected screen position for Objective and Enemy world UI.
- Enemy tracking samples show moving enemy actor positions with matching widget component positions, for example `ActorWorld=V(Y=652.00, Z=65.00)` and `WidgetWorld=V(Y=652.00, Z=185.00)`.
- Later tracking samples include `Projected=true` for Objective UI and all four spawned Enemy UI labels.
- `MDS_ReplicatedUIViewport_Client_EngineShot.png` shows Match HUD/debug text, an Objective HP label near the objective, and four separated Enemy HP labels around the objective instead of a single center overlap.
- The tracking log is gated by `-MDSWorldUITrackingLog`; it is verification evidence and not an always-on runtime UI position update path.
- UI continues to read replicated GameState, ObjectiveActor, and CombatEnemy state. It does not mutate HP, damage, Wave, or combat state.

Caveats:

- The saved log files contain the runtime evidence lines above; the script summary PASS line was observed in the verification console output rather than persisted inside the runtime logs.
- `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png` is the visible placement evidence for this run. `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_PrintWindow.png` was produced by the script but is not used as the visual proof.
- Initial projection samples can be `Projected=false` for actors outside the current camera projection; later samples show `Projected=true`.
- The screenshot includes multiple Objective/probe contexts, so it should be used as UI placement evidence rather than as a single-objective HP-before/after proof.

## Player Attack Runtime Verification

- Date: 2026-07-19
- Scope: owning-client player attack intent, server validation, Enemy HP replication, and negative reject checks.
- Script: `Run_Verify_PlayerAttack.ps1`
- Runtime command used for the latest scenario sweep:

```powershell
powershell -ExecutionPolicy Bypass -File .\Run_Verify_PlayerAttack.ps1 -Port 7790 -SkipBuild -SkipStage
```

Result:

```text
PLAYER ATTACK VERIFY RESULT: PASS
```

Logs:

- `SavedVerifyLogs/MDS_PlayerAttack_Valid_Server.log`
- `SavedVerifyLogs/MDS_PlayerAttack_Valid_Client.log`
- `SavedVerifyLogs/MDS_PlayerAttack_OutOfRange_Server.log`
- `SavedVerifyLogs/MDS_PlayerAttack_OutOfRange_Client.log`
- `SavedVerifyLogs/MDS_PlayerAttack_Cooldown_Server.log`
- `SavedVerifyLogs/MDS_PlayerAttack_Cooldown_Client.log`

Valid scenario evidence:

```text
MDS Combat | AutoAttackIntent | ... | Distance=1214.7
Enemy damage applied by PlayerAttack: 25.0 damage, HP 100.0 -> 75.0.
Enemy damage applied by PlayerAttack: 25.0 damage, HP 75.0 -> 50.0.
Enemy damage applied by PlayerAttack: 25.0 damage, HP 50.0 -> 25.0.
Enemy damage applied by PlayerAttack: 25.0 damage, HP 25.0 -> 0.0.
MDS Combat | ServerAttackResolved | ... | Valid=true | ... | EnemyHP=25.0->0.0.
Enemy death handled on server from PlayerAttack.
Wave enemy death consumed on server: Wave=1 Remaining=1 -> 0.
Wave cleared on server: Wave=1.
Enemy HP replicated on client: 75.0 / 100.0. Dead=false.
Enemy HP replicated on client: 50.0 / 100.0. Dead=false.
Enemy HP replicated on client: 25.0 / 100.0. Dead=false.
Enemy HP replicated on client: 0.0 / 100.0. Dead=true.
```

OutOfRange negative evidence:

```text
MDS Combat | AutoAttackIntent | ... | Distance=1214.7
MDS Combat | ServerAttackRejected | Reason=OutOfRange | ... | Distance=1214.7 | Range=100.0.
Valid attack count: 0
PlayerAttack damage count: 0
Client Enemy HP replication count: 0
```

Cooldown negative evidence:

```text
MDS Combat | AutoAttackIntent | ... | AttemptsRemaining=1
Enemy damage applied by PlayerAttack: 10.0 damage, HP 100.0 -> 90.0.
MDS Combat | ServerAttackResolved | ... | Valid=true | ... | EnemyHP=100.0->90.0.
MDS Combat | AutoAttackIntent | ... | AttemptsRemaining=0
MDS Combat | ServerAttackRejected | Reason=Cooldown | ... | CooldownRemaining=9.84.
Valid attack count: 1
PlayerAttack damage count: 1
Client Enemy HP replication count: 1
Cooldown reject count: 1
```

Interpretation:

- The auto-attack harness is command-line gated and uses the same owning-client `ServerRequestAttack` RPC path as manual attack input.
- The client selects a replicated enemy actor only as request intent; the server remains authoritative for target validation, range, cooldown, damage, HP, death handling, and Wave remaining decrement.
- OutOfRange and Cooldown runtime checks verify rejected attack requests do not apply extra Enemy HP damage.
- This is log/runtime evidence, not visible animation evidence.

Caveats:

- The latest scenario sweep used `-SkipBuild -SkipStage` because the same player attack code had already been built, cooked, and staged in the immediately preceding verification pass.
- Verified death is HP-derived death handling and Wave consumption. Enemy death visual/animation presentation remains unverified.
- Additional reject branches such as InvalidTarget, InvalidDamage, DeadTarget, and NoPawn are not covered by this pass.

## Combat Presentation Hook Verification

- Date: 2026-07-19
- Scope: C++ presentation hook/log evidence for local attack presentation, replicated Enemy HP hit presentation, and HP-derived death presentation.
- Script: `Run_Verify_CombatPresentationHooks.ps1`
- Runtime command:

```powershell
powershell -ExecutionPolicy Bypass -File .\Run_Verify_CombatPresentationHooks.ps1 -Port 7798
```

Result:

```text
COMBAT PRESENTATION VERIFY RESULT: PASS
```

Evidence:

- Valid attack scenario:
  - local client logs `AttackPresentationRequested` and `AttackTimingMarker` before sending the existing server attack request.
  - server applies `PlayerAttack` damage through the authoritative path.
  - client observes replicated Enemy HP in `OnRep_CurrentHealth`.
  - client logs `EnemyHitPresentationRequested` after non-death HP replication.
  - client logs one `EnemyDeathPresentationRequested` after replicated HP reaches `0`.
- Presentation-only negative scenario:
  - client logs `PresentationOnlyAttackMarker`, `AttackPresentationRequested`, and `AttackTimingMarker`.
  - `ServerAttackResolved` count remains `0`.
  - `Enemy damage applied by PlayerAttack` count remains `0`.
  - client `Enemy HP replicated on client` count remains `0`.

Interpretation:

- Attack, hit, and death presentation hooks are presentation-only C++ extension points.
- Hit/death presentation is driven by replicated Enemy HP observation on clients.
- The presentation-only marker does not apply Enemy HP damage and does not send the server attack RPC.
- This verifies hook/log ordering only; it is not visible animation playback evidence.

Caveats:

- Real Attack Montage playback is not verified.
- Real AnimNotify asset firing is not verified.
- Authored Hit Reaction and Death Animation asset playback are not verified.
- Viewport-visible animation pose changes and frame-accurate animation/combat timing are not verified.

## Combat Animation Asset Readiness Verification

- Date: 2026-07-20
- Scope: read-only Editor-Cmd asset readiness check for existing combat animation assets.
- Script: `Run_Verify_CombatAnimationAssets.ps1`
- Log: `SavedVerifyLogs/MDS_CombatAnimationAssets.log`
- Build evidence:
  - `MDSProjectEditor Win64 Development` succeeded before the Editor-Cmd asset verification pass.
  - UBT log path reported by the build: `C:\UnrealEngine\Engine\Programs\UnrealBuildTool\Log.txt`
- Runtime command:

```powershell
powershell -ExecutionPolicy Bypass -File .\Run_Verify_CombatAnimationAssets.ps1 -SkipBuild
```

Result:

```text
COMBAT ANIMATION ASSET VERIFY RESULT: PASS_WITH_INCOMPLETE_ITEMS
BP_TopDownCharacter lineage proven: False
Attack notify authored/readable: False
Attack notify readiness note: INCOMPLETE
Character lineage readiness note: INCOMPLETE
```

Evidence:

- `BP_TopDownCharacter` loads and resolves a character mesh/AnimBP candidate. The checked Editor Python class APIs did not prove the `AMDSProjectCharacter` lineage in this pass.
- The character mesh is `SKM_Manny_Simple`, uses `ABP_Unarmed_C`, and resolves skeleton `SK_Mannequin`.
- `ABP_Unarmed` loads as an `AnimBlueprint` and is compatible with the character skeleton.
- Skeletal mesh candidates `SKM_Manny_Simple` and `SKM_Quinn_Simple` load and use `SK_Mannequin`.
- Attack candidates load: four unarmed `AnimSequence` assets and one pistol `AnimMontage`.
- Hit reaction candidates load as `AnimSequence` assets and are compatible with `SK_Mannequin`.
- Death candidates load as `AnimSequence` assets and are compatible with `SK_Mannequin`.

Interpretation:

- Existing project assets are sufficient as candidates for attack, hit reaction, and death presentation integration.
- The check is read-only and does not create, save, compile, or modify Blueprint/content assets.
- Attack timing notify readiness remains incomplete because no authored/readable notify was found in the checked attack candidates.
- `BP_TopDownCharacter` lineage readiness remains incomplete because the checked Editor Python APIs did not prove the parent chain.
- The script reports this as `PASS_WITH_INCOMPLETE_ITEMS`, not a full animation readiness pass.

Caveats:

- This is Editor asset loadability and skeleton compatibility evidence only.
- Real Attack Montage playback is not verified.
- Real AnimNotify firing is not verified.
- Authored Hit Reaction and Death Animation playback are not verified.
- Viewport-visible animation pose changes and combat timing alignment are not verified.

Regression:

- `Run_Verify_CombatPresentationHooks.ps1 -Port 7804 -SkipBuild -SkipStage -ClientWaitSeconds 28` returned `COMBAT PRESENTATION VERIFY RESULT: PASS` after rebuilding/staging the same source with `Run_Verify_CombatPresentationHooks.ps1 -Port 7802`.

## Combat Animation Playback Attempt Verification

- Date: 2026-07-20
- Scope: existing asset playback API attempt/acceptance logs for attack, hit reaction, and death presentation.
- Script: `Run_Verify_CombatPresentationHooks.ps1`
- Runtime command:

```powershell
powershell -ExecutionPolicy Bypass -File .\Run_Verify_CombatPresentationHooks.ps1 -Port 7815 -SkipBuild -SkipStage -ClientWaitSeconds 28
```

Result:

```text
COMBAT PRESENTATION VERIFY RESULT: PASS
COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS
```

Evidence:

- `MDSProject`, `MDSProjectServer`, client cook/stage, and server cook/stage succeeded in the immediately preceding `Run_Verify_CombatPresentationHooks.ps1 -Port 7808 -ClientWaitSeconds 28` run.
- The latest runtime replay used the same staged build with `-MDSCombatPresentationLog` enabled on both server and client.
- The latest script version requires exact playback/presentation counts, equal attempt/success counts, and zero rejected attacks in the presentation-only no-RPC scenario.
- Valid attack scenario:
  - server animation playback count: `0`.
  - attack montage playback attempts: `4`, success count: `4`.
  - hit animation playback attempts: `3`, success count: `3`.
  - death animation playback attempts: `1`, success count: `1`.
  - client Enemy HP replication count: `4`.
  - first hit animation playback occurs after replicated Enemy HP `100 -> 75`.
  - death animation playback occurs after replicated Enemy HP reaches `0`.
- Presentation-only scenario:
  - server animation playback count: `0`.
  - attack montage playback attempts: `1`, success count: `1`.
  - hit/death animation playback attempts: `0`.
  - valid attack count, PlayerAttack damage count, and client Enemy HP replication count remain `0`.

Representative client logs:

```text
MDS CombatAnimationPlayback | AttackMontagePlaybackAttempted | ... | Asset=MM_Pistol_Fire_Montage | ... | Duration=0.667 | PlaybackSucceeded=true | ... | GameplayDamage=false.
MDS CombatAnimationPlayback | EnemyHitAnimationPlaybackAttempted | ... | Asset=MM_HitReact_Front_Lgt_01 | ... | PlaybackSucceeded=true | ... | GameplayDamage=false.
MDS CombatAnimationPlayback | EnemyDeathAnimationPlaybackAttempted | ... | Asset=MM_Death_Front_01 | ... | PlaybackSucceeded=true | ... | GameplayDamage=false.
```

Interpretation:

- Existing attack, hit reaction, and death assets can be accepted by Unreal animation playback APIs in the staged client.
- Enemy presentation now has a visual-only skeletal mesh component using existing mannequin mesh/AnimBP assets.
- Animation playback attempts are client presentation only and do not apply gameplay damage.
- Dedicated server logs show no animation playback attempts.

Caveats:

- This is playback API attempt/acceptance evidence, not viewport-visible pose-change evidence.
- At this earlier capture pass, real authored AnimNotify firing was not yet verified; see the later persistent authored Notify evidence.
- At this earlier pass, simulated-client attack montage replication was not verified; the later Dedicated Server + two-client verification supersedes this limitation.
- Listen-server host enemy presentation is not covered by this dedicated server/client check.

Regression:

- `Run_Verify_PlayerAttack.ps1 -Port 7810 -SkipBuild -SkipStage` returned `PLAYER ATTACK VERIFY RESULT: PASS`.
- `git diff --check` passed with CRLF warnings only.

## Combat Animation Visible Capture Verification

- Date: 2026-07-20
- Scope: visible staged client screenshots correlated with attack, hit, and death animation playback events.
- Script: `Run_Verify_CombatAnimationVisibleCapture.ps1`
- Runtime command:

```powershell
powershell -ExecutionPolicy Bypass -File .\Run_Verify_CombatAnimationVisibleCapture.ps1 -Port 7822 -SkipBuild -SkipStage -ClientWaitSeconds 24
```

Result:

```text
COMBAT ANIMATION VISIBLE CAPTURE VERIFY RESULT: PASS_WITH_POSE_LIMITATION (historical; superseded by paired pose-delta PASS)
```

Evidence:

- `MDSProject Win64 Development` and `MDSProjectServer Win64 Development` built successfully in the immediately preceding full visible capture run.
- Client and server cook/stage succeeded in the immediately preceding full visible capture run.
- The successful runtime replay used the same staged build with a visible `MDSProject.exe` client and `-MDSCombatAnimationVisibleShot`.
- Dedicated server logs show zero `MDS CombatAnimationPlayback` and zero `MDS CombatAnimationVisibleCapture` entries.
- Server applied four valid `PlayerAttack` damage events and handled death once.
- Client observed four Enemy HP replication updates.
- Client logged four successful attack montage playback attempts, three successful hit animation playback attempts, and one successful death animation playback attempt.
- Client requested event-timed screenshots:
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Attack.png`
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Hit.png`
  - `SavedVerifyLogs/MDS_CombatAnimationVisible_Death.png`
- The script verified that all three screenshot files exist, are non-empty, and contain visible pixels.

Interpretation:

- A visible staged client viewport can be captured at attack, hit, and death playback moments using existing project assets.
- Capture hooks are command-line gated and presentation-only; they do not apply gameplay damage.
- Hit/death captures occur after replicated Enemy HP observation in the client log.

Caveats:

- Historical limitation: these original single screenshots did not prove pose delta; the later paired `Before`/`Pose` pass supersedes it.
- At this earlier visible-capture pass, authored AnimNotify firing remained unverified; the later staged runtime test supersedes this limitation.
- Simulated-client attack montage presentation is verified by the later Dedicated Server + two-client pass.

## Verified

- Dedicated Server starts and listens on port `7777`.
- Server initializes and owns Wave display state.
- Client connects to the server.
- Client observes replicated Wave display state.
- Client observes replicated Objective HP.
- Existing CommonUI viewport client configuration error is removed after re-stage.
- Debug overlay Widget Blueprint asset compiles/saves and is resolved as `WBP_MDSDebugOverlay_C` in runtime configuration.
- Debug overlay fallback layout code compiles and preserves existing standalone and dedicated server/client runtime verification paths.
- Debug overlay viewport pixels are visible in a staged Win64 client after `F1`.
- `BP_TopDownController` derives from `AMDSProjectPlayerController`, activating the project debug overlay path in the TopDown map.
- Match HUD, Objective World UI, and Enemy World UI baseline widgets read replicated gameplay sources on a staged dedicated server/client run.
- Replicated UI viewport verification now fails closed when the client content screenshot is visually blank.
- Replicated UI viewport pixels are visible in an engine-captured staged client screenshot.
- Objective and Enemy World UI labels can be captured in a staged client viewport while following actor-attached widget component positions.
- Player attack intent from the owning client reaches server validation and applies Enemy HP damage only through the server path.
- Valid player attack damage replicates Enemy HP changes to the client.
- HP-derived enemy death is handled once by the server and Wave remaining is decremented from `1` to `0`.
- OutOfRange and Cooldown player attack requests are rejected without extra Enemy HP damage.
- Combat presentation hooks can be triggered as local/client presentation without mutating server-owned damage state.
- Client hit/death presentation hook logs occur after replicated Enemy HP observation.
- Existing attack, hit reaction, and death animation candidate assets load in Editor-Cmd and are compatible with the character skeleton.
- Existing attack montage, hit reaction, and death animation assets are accepted by client-side animation playback APIs during the staged dedicated server/client presentation scenario.
- Visible staged client screenshots are captured at attack, hit, and death animation playback moments and pass nonblank pixel checks.

## Runtime AnimNotify Dispatch Verification

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_CombatAnimNotify.ps1 -Port 7832 -ClientWaitSeconds 24 -SkipBuild -SkipStage
```

Result:

```text
COMBAT PRESENTATION VERIFY RESULT: PASS
COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS
COMBAT ANIMNOTIFY VERIFY RESULT: PASS
```

Observed evidence:

- Valid scenario: client Notify callbacks `4`, dedicated server Notify callbacks `0`, valid server attacks `4`, PlayerAttack damage events `4`, client Enemy HP observations `4`.
- Presentation-only scenario: client Notify callbacks `1`, dedicated server Notify callbacks `0`, valid server attacks `0`, rejected server attacks `0`, PlayerAttack damage events `0`, client Enemy HP observations `0`.
- Notify logs explicitly record `GameplayDamage=false` and `ServerRequestSent=false`.
- Historical note: this pass used transient injection. It is superseded by the persistent authored Notify resolution below.

## Persistent Authored Attack AnimNotify Evidence

- Asset: `/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage`
- Notify: one `UMDSCombatTimingAnimNotify`, configured at `0.100` seconds in a `0.667` second montage.
- Idempotency: a second Editor-Cmd configuration run found exactly one existing Notify and added no duplicate.
- Transient runtime injection was removed from `AMDSProjectPlayerController` before build/cook/stage.
- Asset verification reports `ATTACK_NOTIFY_READINESS | PASS=True` and authored/readable `True`.
- Valid runtime scenario: client callbacks `4`, Dedicated Server callbacks `0`, server-authoritative damage events `4`.
- Presentation-only scenario: client callback `1`, Dedicated Server callbacks `0`, attack RPC/damage/Enemy HP replication `0`.
- Result: `COMBAT ANIMNOTIFY VERIFY RESULT: PASS`.
- Notify logs explicitly report `GameplayDamage=false` and `ServerRequestSent=false`.
- UE Python does not expose stored `FAnimNotifyEvent.LinkValue`; configured-time range is checked during authoring and actual in-playback firing is proven by the staged runtime test.

Regression results:

```text
COMBAT PRESENTATION VERIFY RESULT: PASS
PLAYER ATTACK VERIFY RESULT: PASS
```

## Character Movement Role and Replication Evidence

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_CharacterMovementReplication.ps1 -Port 7849 -ClientWaitSeconds 30 -SkipBuild -SkipStage
```

Observed:

- Dedicated server accepted both clients (`4` login/join markers).
- Mover pawn: `NetMode=Client`, `LocalRole=AutonomousProxy`, `RemoteRole=Authority`, `LocallyControlled=true`.
- Observer view of the remote pawn: `LocalRole=SimulatedProxy`, `RemoteRole=Authority`, `LocallyControlled=false`.
- Server pawns: `NetMode=DedicatedServer`, `LocalRole=Authority`.
- Automated move start and finish markers were emitted without fatal errors.
- Maximum movement distance and speed remained `0 / 0` for mover, server, and observer.

Result:

```text
CHARACTER MOVEMENT REPLICATION VERIFY RESULT: INCOMPLETE
```

Interpretation: network role topology is proven, but actual CMC movement and simulated-proxy motion are not. This is a fail-closed evidence result, not a movement-completion claim.

Follow-up diagnostic evidence:

- Pawn class: `BP_TopDownCharacter_C`; native parent: engine `Character`.
- CMC: active, component Tick enabled, updated component `CollisionCylinder`, `MaxWalkSpeed=600`, `MaxAcceleration=1000`.
- Input injection: Pending Input changes from zero to `Y=1` immediately after forced injection.
- Input consumption: later AutonomousProxy snapshots keep Last Input, velocity, and distance at zero.
- Navigation: packaged client reports `NavSystem=None`, `NavData=None`, `PathFollowing=None`, and the engine logs the corresponding `SimpleMove` navigation warning.

This isolates two configuration gaps: the Blueprint is not based on `AMDSProjectCharacter`, and the packaged network client cannot use the navigation-dependent TopDown simple-move path in the current configuration.

Reparent follow-up:

- `BP_TopDownCharacter` was successfully reparented from engine `Character` to `/Script/MDSProject.MDSProjectCharacter`, then compiled and saved through Editor-Cmd.
- The recooked staged client reports that the native parent is no longer engine `Character`.
- Navigation-independent input was tested along the X axis to avoid the adjacent player capsule.
- Pending Input was still accepted but not observed as Last Input, and mover/server/observer did not exceed the required distance or speed thresholds.
- A temporary `RequestDirectMove` probe also failed the thresholds and was removed; it is not part of the retained implementation.
- That diagnostic pass remained `INCOMPLETE`; the retained direct-move workaround was not used.

Resolution evidence (2026-07-21):

- `BP_TopDownCharacter` was reparented to `AMDSProjectCharacter` and an Enhanced Input `IA_Move` Axis2D action was mapped to W/A/S/D.
- Automated movement now enters through the shared character movement helper during the owning controller's `PlayerTick`, matching the CMC input-consumption window.
- Mover `AutonomousProxy`, server `Authority`, and observer `SimulatedProxy` each reported maximum distance/speed `1620.5 / 600`.
- Movement input accepted/consumed: `True / True`; fatal error: `False`.
- Final result: `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS`.
- This proves packaged CMC movement replication, not physical keyboard-event injection or frame-accurate AnimBP locomotion pose changes.

## Not Verified In This Pass

- Authored Widget Blueprint TextBlock placement.
- Authored Match HUD visual layout.
- Authored Objective World UI visual layout.
- Authored Enemy World UI visual layout.
- Frame-accurate Attack Montage pose delta.
- Frame-accurate viewport confirmation of the authored attack Notify cue.
- Frame-accurate Hit Reaction and Death Animation pose deltas.
- Additional player attack reject branches outside the directional-fire contract, such as InvalidTarget and DeadTarget.

These items require visual PIE/client checks, authored Widget Blueprint layout work, or animation-specific runtime scenarios.

## Directional Movement and Fire-Facing Evidence

Date: 2026-07-22

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_CharacterMovementVisible.ps1 -Port 7874 -ManualInputSeconds 90
```

Observed:

- Dedicated server accepted two clients (`4` login/join markers).
- Mover `AutonomousProxy` maximum distance/speed: `1585.5 / 600`.
- Server `Authority` maximum distance/speed: `1588.7 / 600`.
- Observer `SimulatedProxy` maximum distance/speed: `1629 / 600.4`.
- Directional fire resolved `19` valid shots: `3` hits and `16` misses.
- Fatal error: `False`.
- User manual viewport review confirmed W/S/A/D cardinal movement and diagonal input.
- Normal facing followed movement direction on mover and observer.
- Left-mouse fire temporarily faced the cursor direction during the attack montage and then returned to movement-direction facing while movement continued.
- Empty-space fire produced presentation and a valid miss without damage.
- Enemy-direction fire produced server-authoritative hits and replicated HP changes.

Result:

```text
CHARACTER MOVEMENT VISIBLE VERIFY RESULT: PASS_WITH_MANUAL_POSE_REVIEW
```

Authority note: cursor direction is client attack intent. The server owns directional range/hit evaluation, cooldown, damage application, Enemy HP, death handling, and Wave progression. Montage playback and AnimNotify remain presentation-only.

## Directional Fire Reject Verification

Date: 2026-07-22

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_PlayerAttack.ps1 -Scenario Rejects -Port 7900 -SkipBuild -SkipStage
```

Observed:

- `InvalidDirection`: zero-vector owning-client RPC intent produced exactly one `Reason=InvalidDirection` server rejection.
- `InvalidDamage`: server attack damage `0.0` produced exactly one `Reason=InvalidDamage` rejection.
- `NoPawn`: the command-line-gated server verification path unpossessed the requester's Pawn immediately before validation and produced exactly one `Reason=NoPawn` rejection.
- Every scenario recorded zero valid attacks, zero `PlayerAttack` damage, zero Enemy HP replication, zero enemy deaths, and zero Wave enemy-death consumption.
- All three client connections succeeded and no fatal error was found.
- Normal attack regression passed with four valid hits, four replicated Enemy HP updates, HP `100 -> 0`, one server death, and one Wave consumption.
- CharacterMovement regression passed after extending the collection window: mover, server, and observer each reached `1620.5 / 600`, with input accepted/consumed `True / True`.

Result:

```text
PLAYER ATTACK InvalidDirection VERIFY RESULT: PASS
PLAYER ATTACK InvalidDamage VERIFY RESULT: PASS
PLAYER ATTACK NoPawn VERIFY RESULT: PASS
PLAYER ATTACK VERIFY RESULT: PASS
CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS
```

The rejection harness is inactive unless `MDSAutoAttackReject=<Scenario>` is explicitly supplied. Normal manual and automated fire continue to use the owning-client `ServerRequestAttack` RPC and server-owned validation path.

## Simulated Client Attack Presentation Verification

Date: 2026-07-22

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_SimulatedClientAttackPresentation.ps1 -Port 7910 -SkipBuild -SkipStage
```

Observed:

- Dedicated Server accepted observer and attacker clients (`4` login/join markers).
- Server resolved four valid directional hits and applied four `PlayerAttack` damage events.
- Observer received four `RemoteAttackPresentationReceived` events on `LocalRole=SimulatedProxy`, including the server-confirmed direction and presentation duration.
- Observer remote montage playback succeeded `4/4`, and the character presentation hook ran `4/4`.
- Observer and attacker each observed four replicated Enemy HP changes.
- Owning attacker remote montage playback count was `0`; its local intent path remained the sole owning-client montage path.
- Dedicated Server animation playback count was `0`.
- No fatal error or ensure was found.

Result:

```text
SIMULATED CLIENT ATTACK PRESENTATION VERIFY RESULT: PASS
PLAYER ATTACK VERIFY RESULT: PASS
CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS
```

Authority note: the server confirms the directional attack before issuing the multicast presentation. The multicast changes facing and presentation only; damage, Enemy HP, death, and Wave progression remain server-owned.

## Continuous Wave Loop Verification

Date: 2026-07-23

Command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_ContinuousWaveLoop.ps1
```

Observed:

- Dedicated Server spawned Wave 1/2/3 with exactly `3/4/5` combat enemies.
- The owning client issued server-validated attacks; the server consumed exactly `12` enemy deaths.
- Each wave cleared exactly once, and `Demo wave loop completed on server: FinalWave=3` appeared exactly once.
- The client replication log observed active Wave 1, Wave 2, and Wave 3 state.
- No fatal error or ensure was found.
- `MDSWaveIntermission=1` and zero enemy movement were verification-only overrides; normal defaults remain three seconds and movement speed `100`.

Result:

```text
CONTINUOUS WAVE LOOP VERIFY RESULT: PASS
PLAYER ATTACK Valid VERIFY RESULT: PASS
SIMULATED CLIENT ATTACK PRESENTATION VERIFY RESULT: PASS
CHARACTER MOVEMENT REPLICATION VERIFY RESULT: INCOMPLETE
```

The movement result was incomplete because the existing auto-move trigger did not start; server and two-client connection succeeded and no fatal error occurred. It is not counted as a passing movement re-verification.

## Enemy Presentation and Mass Default-Off Verification

Date: 2026-07-23

Observed:

- Client and Server Development builds and both staged packages succeeded.
- Three Wave runtime spawned and consumed `12` Character-based enemies without changing authoritative Wave counts.
- Server and client each logged `12` death-fade starts after the two-second body hold.
- Normal server startup logged `Mass baseline disabled` and zero Mass initialization events.
- Explicit `-MDSMassBaseline MDSMassBaselineCount=2` initialized exactly two Mass entities, preserving the opt-in technical probe.
- Dedicated Server animation playback remained zero.
- Simulated observer received all four server-confirmed attack presentations and four replicated Enemy HP updates.
- No fatal error or ensure was found.

Visual verification scope:

- Enemy slope/step walking and locomotion pose.
- Immediate owning-client yellow shot tracer and pistol montage.
- Pawn-overlap enemy crowd behavior.
- Two-second death hold followed by one-second sink/fade.

Material note: the code sets optional `Opacity` and `Fade` scalar parameters, but the current mannequin material may not expose them. The downward sink and final visibility removal remain the guaranteed fallback.

## 3D Aim and Enemy Movement Correction

Date: 2026-07-23

Observed:

- Attack intent carries the cursor's 3D Visibility impact point instead of a flattened XY direction.
- Server clamps the shot endpoint to range `5000` and performs a 3D line-segment proximity test against live enemies.
- Local tracer start/end presentation uses the actual cursor collision endpoint.
- Three controller-free enemies entered `MOVE_Walking` with velocity `100`, valid collision components, and controller-free physics enabled.
- All three reached the Objective at `Z=307.96`; movement was no longer stationary.
- Continuous Wave verification passed with `12` deaths and three Wave clears.
- Client logs recorded `12` death-pose freezes before `12` fade starts.
- InvalidDirection verification produced exactly one server rejection and zero damage.
- No fatal error or ensure was found.

## Directional Target and Hit Pause Verification

Date: 2026-07-23

Observed:

- Cursor impact selects direction; it no longer truncates the server target search.
- The nearest alive enemy in the full range corridor becomes both the damage target and tracer endpoint.
- If no directional enemy exists, the terrain cursor impact remains the tracer endpoint.
- Four valid attacks applied four damage events and four replicated HP updates.
- Three nonlethal hits each produced a movement resume after the configured `0.35` second pause.
- The fourth hit killed the enemy and did not resume movement.
- Continuous Wave and PlayerAttack Valid passed without fatal/ensure.

## Manual Follow-Up

Authored gameplay UI style update (2026-07-23): `WBP_MDSMatchHUD`, `WBP_MDSObjectiveWorldUI`, and `WBP_MDSEnemyWorldUI` were compiled/saved twice through the idempotent Editor Python script. The latest replicated UI EngineShot shows cyan Match HUD text, gold Objective HP, and red Enemy HP labels at four actor-attached positions. All underlying viewport PASS checks were true and fatal/CommonUI errors were false. Production panel/art polish remains optional.

Pose-delta evidence update (2026-07-22): `Run_Verify_CombatAnimationVisibleCapture.ps1 -Port 7922 -ClientWaitSeconds 26 -SkipBuild -SkipStage` produced six paired captures and `COMBAT ANIMATION POSE DELTA VERIFY RESULT: PASS`. Center-region changed samples were Attack `75`, Hit `58`, and Death `803`. This verifies rendered pose change, not artistic animation quality.

1. Add authored Widget Blueprint TextBlocks if a custom layout is needed.
2. Confirm displayed values match replicated GameState, ObjectiveActor, and debug snapshot sources during dedicated server/client play.
3. Use `MDS_ReplicatedUIViewport_Client_EngineShot.png` as the current replicated UI viewport screenshot evidence; authored UI polish can replace the fallback layout later.
