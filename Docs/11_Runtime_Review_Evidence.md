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
- A standalone headless runtime reaches the debug state reporting path without a missing debug overlay class log, widget creation failure log, CommonUI viewport error, or fatal error.
- This check does not verify visual pixels, F1 input, or Widget Blueprint TextBlock layout.

## Verified

- Dedicated Server starts and listens on port `7777`.
- Server initializes and owns Wave display state.
- Client connects to the server.
- Client observes replicated Wave display state.
- Client observes replicated Objective HP.
- Existing CommonUI viewport client configuration error is removed after re-stage.
- Debug overlay Widget Blueprint asset compiles/saves and is resolved as `WBP_MDSDebugOverlay_C` in runtime configuration.

## Not Verified In This Pass

- Widget Blueprint visual overlay layout, including F1 toggle and TextBlock placement.
- Match HUD visual layout.
- Objective World UI visual layout.
- Enemy World UI visual layout.
- Enemy HP/death presentation.
- Attack Montage / AnimNotify negative test.
- Hit Reaction and Death Animation presentation.

These items require Widget Blueprint assets, visual PIE/client checks, or animation-specific runtime scenarios.

## Manual Follow-Up

1. Create Widget Blueprint assets for the debug overlay and gameplay UI surfaces.
2. Assign the debug overlay Widget Blueprint to `DebugOverlayWidgetClass`.
3. Run staged client or PIE and toggle the overlay with `F1`.
4. Confirm displayed values match replicated GameState, ObjectiveActor, and debug snapshot sources.
5. Capture screenshots or short video for Match HUD, Objective World UI, and Enemy World UI.
