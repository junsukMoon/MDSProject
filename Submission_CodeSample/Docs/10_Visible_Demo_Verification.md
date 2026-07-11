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
