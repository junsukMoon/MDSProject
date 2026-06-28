# Visible Demo Verification

This document records the visible viewport replication check and demo capture evidence for `MDSProject`.

## Runtime

- Date: 2026-06-29
- Engine: source-built UE 5.8
- Server: staged `MDSProjectServer.exe`
- Clients: two staged `MDSProject.exe` standalone clients
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

Logs:

- Server: `C:\Temp\MDS_VisibleDemo_Server.log`
- Client 1: `C:\Temp\MDS_VisibleDemo_Client1.log`
- Client 2: `C:\Temp\MDS_VisibleDemo_Client2.log`

Server log confirmed:

- `IpNetDriver listening on port 7777`
- two `Login request` entries
- two `Join succeeded` entries
- final repeated state:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

Client logs confirmed:

- Client 1 observed `ObjectiveHP=20/100` 400 times.
- Client 2 observed `ObjectiveHP=20/100` 400 times.
- Both visible clients displayed:

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## Visual Evidence

- Client 1 screenshot: `Docs/Verification/VisibleObjectiveHP_Client1.png`
- Client 2 screenshot: `Docs/Verification/VisibleObjectiveHP_Client2.png`
- 2-minute demo GIF: `Docs/Verification/VisibleObjectiveHP_Demo.gif`

The GIF captures both visible standalone client windows side by side. Both clients show the replicated server-owned Objective HP as `20/100`.

## Unreal Insights Trace

A short Unreal Insights smoke trace was captured after the visible demo evidence.

- Trace: `Docs/Verification/MDS_Insights_TraceSmoke.utrace`
- Log: `C:\Temp\MDS_Insights_TraceSmoke.log`
- Trace channels: `cpu,frame,bookmark,log`
- Runtime mode: `-game -NullRHI -nosound -unattended`
- Capture driver: `-MDSGameplayProfile -MDSGameplayProfileFrames=300`

Log confirmation:

```text
Trace started (writing to file "Docs/Verification/MDS_Insights_TraceSmoke.utrace")
Capture Starting
Capture Ended
```

## Result

- Visible viewport replicated Objective HP verification: complete.
- Two-client same server-owned Objective HP screenshot/GIF evidence: complete.
- 2-minute demo GIF: complete.
- Unreal Insights trace smoke capture: complete.
