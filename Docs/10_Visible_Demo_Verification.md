# Visible Demo Verification

이 문서는 `MDSProject`의 visible viewport replication check와 demo capture evidence를 기록합니다.

## Runtime

- Date: 2026-06-29
- Engine: UE 5.8 source build
- Server: staged `MDSProjectServer.exe`
- Clients: staged `MDSProject.exe` standalone clients
- Map: `/Game/TopDown/Lvl_TopDown`

## Launch Setup

Dedicated server:

```text
MDSProject/Saved/StagedBuilds/WindowsServer/MDSProject/Binaries/Win64/MDSProjectServer.exe
/Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -port=7777
```

Visible clients:

```text
MDSProject/Saved/StagedBuilds/Windows/MDSProject/Binaries/Win64/MDSProject.exe
127.0.0.1:7777 -windowed -ResX=960 -ResY=540 -nosound -NoSplash
```

## Log Evidence

서버 로그에서 확인한 항목:

- `IpNetDriver listening on port 7777`
- client login/join
- final debug state

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

클라이언트 로그에서 확인한 항목:

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## Visual Evidence

- `Docs/Verification/VisibleObjectiveHP_Client1.png`
- `Docs/Verification/VisibleObjectiveHP_Client2.png`
- `Docs/Verification/VisibleObjectiveHP_Demo.gif`

두 standalone client가 같은 server-owned Objective HP `20/100`을 표시하는 것을 확인했습니다.

## Latest Actor-Following World UI Evidence

- Date: 2026-07-17
- PR: #43 (`Make world UI follow owning actors`)
- Script: `Run_Verify_ReplicatedUIViewport.ps1 -Port 7779 -ActorEnemyCount 4 -ActorEnemyMoveSpeed 30`
- Server: staged `MDSProjectServer.exe`
- Client: staged `MDSProject.exe`
- Map: `/Game/TopDown/Lvl_TopDown`
- Visible screenshot:
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png`
- Logs:
  - `SavedVerifyLogs/MDS_GameplayUIAsset.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Server.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client.log`

Result observed from the verification run:

```text
REPLICATED UI VIEWPORT VERIFY RESULT: PASS
```

Visible evidence:

- The engine screenshot shows Match HUD/debug text, an Objective HP label near the objective, and four separated Enemy HP labels around the objective.
- Enemy HP labels are no longer captured as a single overlapped label at the screen center.
- Objective and Enemy World UI are screen-space widget components attached to their owning actors; no per-tick manual UI location update is required for following behavior.

Runtime log evidence:

```text
IpNetDriver listening on port 7779
Combat enemy wave spawn created 4/4 enemies around objective MDS_ActorObjectiveProbe
MDS Match HUD read GameState wave state: Wave=1 Remaining=0 Total=0 Active=false.
ObjectiveWorldUITrack Actor=... WidgetWorld=... Screen=... Projected=true
EnemyWorldUITrack Actor=... WidgetWorld=... Screen=... Projected=true
```

Notes:

- `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png` is the visible placement evidence for this run. `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_PrintWindow.png` was produced by the script but is not used as visual proof.
- Some initial tracking samples are `Projected=false` when actors are outside the current camera projection, but later samples include `Projected=true` for Objective UI and all four Enemy UI labels.
- The screenshot is visible layout evidence. The multiple Objective/probe context means it should not be treated as a single Objective HP state transition proof.

## Unreal Insights Trace

Smoke trace:

```text
Docs/Verification/MDS_Insights_TraceSmoke.utrace
```

Trace channels:

```text
cpu, frame, bookmark, log
```

이 trace는 smoke capture이며 full performance investigation은 아닙니다.

## Result

- visible viewport replicated Objective HP verification: 완료
- two-client same server-owned Objective HP evidence: 완료
- demo GIF: 완료
- Unreal Insights smoke trace: 완료
