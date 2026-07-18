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

## Not Verified In This Pass

- Authored Widget Blueprint TextBlock placement.
- Authored Match HUD visual layout.
- Authored Objective World UI visual layout.
- Authored Enemy World UI visual layout.
- Enemy HP/death presentation.
- Attack Montage / AnimNotify negative test.
- Hit Reaction and Death Animation presentation.

These items require visual PIE/client checks, authored Widget Blueprint layout work, or animation-specific runtime scenarios.

## Manual Follow-Up

1. Add authored Widget Blueprint TextBlocks if a custom layout is needed.
2. Confirm displayed values match replicated GameState, ObjectiveActor, and debug snapshot sources during dedicated server/client play.
3. Use `MDS_ReplicatedUIViewport_Client_EngineShot.png` as the current replicated UI viewport screenshot evidence; authored UI polish can replace the fallback layout later.
