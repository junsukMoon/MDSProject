# Visible Demo Verification

이 문서는 `MDSProject`의 visible viewport replication check와 demo capture evidence를 기록합니다.

## Runtime

- Date: 2026-06-29
- Engine: source-built UE 5.8
- Server: staged `MDSProjectServer.exe`
- Clients: staged `MDSProject.exe` standalone client 2개
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
- `Login request` 2개
- `Join succeeded` 2개
- repeated final state:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

Client logs confirmed:

- Client 1은 `ObjectiveHP=20/100`을 400회 관찰했습니다.
- Client 2는 `ObjectiveHP=20/100`을 400회 관찰했습니다.
- visible client 2개 모두 다음 debug line을 표시했습니다.

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## Visual Evidence

- Client 1 screenshot: `Docs/Verification/VisibleObjectiveHP_Client1.png`
- Client 2 screenshot: `Docs/Verification/VisibleObjectiveHP_Client2.png`
- 2-minute demo GIF: `Docs/Verification/VisibleObjectiveHP_Demo.gif`

GIF는 visible standalone client 2개를 side by side로 캡처합니다. 두 client 모두 replicated server-owned Objective HP `20/100`을 표시합니다.

## Unreal Insights Trace

visible demo evidence 이후 짧은 Unreal Insights smoke trace를 캡처했습니다.

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

- visible viewport replicated Objective HP verification: 완료
- two-client same server-owned Objective HP screenshot/GIF evidence: 완료
- 2-minute demo GIF: 완료
- Unreal Insights trace smoke capture: 완료
