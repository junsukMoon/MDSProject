# Phase 8-10 Profiling Comparison

This document records profiling notes for the current Mass-based objective scenario.

The goal is to support technical interview discussion with measured context, not to claim production-grade benchmarking.

## Scenario

- Project: `MDSProject`
- Map: `/Game/TopDown/Lvl_TopDown`
- Runtime dates:
  - 2026-06-27 for UE 5.6 standalone/editor server-mode measurements
  - 2026-06-28 for UE 5.8 source dedicated server binary measurement
- Engines used for measurement:
  - Epic Launcher UE 5.6 installed build
  - Source-built UE 5.8
- Scenario type: Mass-based objective interaction
- Mass entities: 16
- Movement target: spawned Objective probe actor
- Expected final state:
  - Objective HP: `20 / 100`
  - Mass spawned: `16`
  - Mass arrived: `16`
  - Objective damage events: `16`
  - Moved count after arrival: `0`

## Measurement Method

Commands were run with `UnrealEditor-Cmd.exe` or staged `MDSProjectServer.exe`, `-NullRHI`, and benchmark mode.

The effective FPS and average frame time below were calculated from Unreal log timestamps and frame counters after all Mass entities had arrived.

This method is useful for consistent local before/after comparisons, but it does not fully represent viewport rendering, GPU cost, or PIE/editor overhead.

## Mass Runtime Results

### Standalone NullRHI

- Log: `MDSProject/Saved/Logs/Phase8_Mass_Standalone.log`
- Runtime mode: `-game -NullRHI -BENCHMARK -BENCHMARKSECONDS=20`
- Final debug line:
  - `MDS Debug | NetMode=Standalone | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16`
- Sampled post-arrival frames: `450`
- Sampled seconds: `0.175`
- Effective FPS: `2571.43`
- Average frame time: `0.39 ms`

### Editor Server-Mode NullRHI

- Log: `MDSProject/Saved/Logs/Phase8_Mass_ServerMode.log`
- Runtime mode: `-server -NullRHI -BENCHMARK -BENCHMARKSECONDS=20`
- NetDriver:
  - `IpNetDriver listening on port 7777`
- Final debug line:
  - `MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16`
- Sampled post-arrival frames: `450`
- Sampled seconds: `0.100`
- Effective FPS: `4500.00`
- Average frame time: `0.22 ms`

### Staged Dedicated Server Binary NullRHI

- Log: `MDSProject/Saved/Logs/Phase10_StagedDedicatedServer.log`
- Engine: source-built UE 5.8
- Build target: `MDSProjectServer Win64 Development`
- Cook:
  - Platform: `WindowsServer`
  - Log: `MDSProject/Saved/Logs/Phase10_Cook_WindowsServer_SkipZen.log`
  - Result: `Success - 0 error(s), 0 warning(s)`
- Stage:
  - `BuildCookRun -skipbuild -skipcook -stage`
  - Result: `BUILD SUCCESSFUL`
- Runtime mode:
  - Staged `MDSProjectServer.exe`
  - `-NullRHI -BENCHMARK -BENCHMARKSECONDS=20`
- Runtime verification:
  - `Premade AssetRegistry loaded`
  - `World NetMode = Dedicated Server`
  - `IpNetDriver listening on port 7777`
- Final debug line:
  - `MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16`
- Sampled post-arrival frames: `450`
- Sampled seconds: `0.497`
- Effective FPS: `905.43`
- Average frame time: `1.10 ms`

Important cook/stage note:

- UE 5.8 defaults to `bUseZenStore=True`.
- The first cook/stage attempt produced a Zen project store marker but did not stage loose UFS content needed for this standalone server runtime check.
- Re-cooking with `-skipzenstore` produced loose cooked content, after which staging copied UFS files and the dedicated server runtime check succeeded.

### Dedicated Server Two-Client Replication Check

- Runtime date: 2026-06-29
- Engine: source-built UE 5.8
- Server:
  - Staged `MDSProjectServer.exe`
  - Log: `C:\Temp\MDS_Dedicated_TwoClients_Server.log`
- Clients:
  - Staged `MDSProject.exe`
  - Runtime mode: `127.0.0.1:7777 -log -unattended -nosound -NullRHI`
  - Client 1 log: `C:\Temp\MDS_Dedicated_TwoClients_Client1.log`
  - Client 2 log: `C:\Temp\MDS_Dedicated_TwoClients_Client2.log`
- Server connection verification:
  - `IpNetDriver listening on port 7777`
  - Two client `Login request` entries
  - Two client `Join succeeded` entries
- Server final debug line:
  - `MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16`
- Client replication verification:
  - Client 1 logged `MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0` 72 times.
  - Client 2 logged `MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0` 72 times.

Result:

- Both standalone clients observed the same replicated server-owned Objective HP result: `20/100`.

Important limitation:

- This was a headless `-NullRHI` log verification. Visible two-client screenshot/GIF evidence was captured later and is recorded in `Docs/10_Visible_Demo_Verification.md`.
- Both clients emitted repeated `LogNetPlayerMovement: Warning: CreateSavedMove: Hit limit of 96 saved moves` warnings under the headless run. The Objective HP replication result remained stable.

## Debug Draw Fix Measurement

Before Phase 8, a post-arrival slowdown was investigated and fixed in PR #11.

Root cause:

- Arrival debug visualization created a 5-second cyan debug sphere every frame for every arrived entity.
- After all entities arrived, debug primitives continued to accumulate.

Fix:

- The arrival marker is now drawn once, only when an entity first arrives.

Measured same-condition post-arrival result:

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before fix | `248.76` | `4.02 ms` |
| After fix | `2163.46` | `0.46 ms` |

Delta:

- FPS: `+1914.70`
- FPS percentage: `+769.70%`
- Average frame time: `-3.56 ms`

Important limitation:

- This was measured in `-NullRHI`.
- The actual viewport cost of repeated debug draw may be worse than the measured headless cost.

## Actor vs Mass Phase Capture

Runtime date: 2026-06-29

Engine:

- Source-built UE 5.8

Runtime mode:

- `MDSProjectEditor-Cmd.exe`
- `-game -NullRHI -nosound -unattended`
- CSV profiler capture length: `600` frames
- Trigger: `MovementActive`
- Stable frames: `3`
- Expected count: `1000`

Mass command contract:

- `-NoMDSMassDebugDraw`
- `-MDSMassBaselineCount=1000`
- `-MDSGameplayProfileSubject=Mass`
- `-MDSGameplayProfileTrigger=MovementActive`
- `-MDSGameplayProfileExpectedCount=1000`
- CSV: `MDSProject/Saved/Profiling/CSV/Mass1000_MovementActive_Phase.csv`
- Log: `C:\Temp\MDS_Profile_Mass1000_MovementActive_Phase.log`

Actor command contract:

- `-NoMDSMassBaseline`
- `-MDSActorBaseline`
- `-MDSActorBaselineCount=1000`
- `-MDSGameplayProfileSubject=Actor`
- `-MDSGameplayProfileTrigger=MovementActive`
- `-MDSGameplayProfileExpectedCount=1000`
- CSV: `MDSProject/Saved/Profiling/CSV/Actor1000_MovementActive_Phase.csv`
- Log: `C:\Temp\MDS_Profile_Actor1000_MovementActive_Phase.log`

Runtime verification:

- Mass log confirmed `Gameplay CSV profile configured` with `Trigger=MovementActive`, `Subject=Mass`, and `ExpectedCount=1000`.
- Mass log confirmed `Mass debug draw disabled for profiling`.
- Mass log confirmed `Gameplay profile trigger condition met after 3 stable frames`.
- Mass log confirmed CSV capture start/end.
- Actor log confirmed `Gameplay CSV profile configured` with `Trigger=MovementActive`, `Subject=Actor`, and `ExpectedCount=1000`.
- Actor log confirmed `Mass baseline disabled`.
- Actor log confirmed `Actor enemy baseline spawned 1000 enemies`.
- Actor log confirmed `Gameplay profile trigger condition met after 3 stable frames`.
- Actor log confirmed CSV capture start/end.

CSV results:

| Scenario | Numeric samples | Avg FrameTime | P95 FrameTime | Max FrameTime | Avg TickActors | P95 TickActors | TotalActorCount | Ticks/Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Mass 1000 MovementActive | `600` | `0.4567 ms` | `0.4898 ms` | `7.9245 ms` | `0.0997 ms` | `0.1172 ms` | `87` | `38` |
| Actor 1000 MovementActive | `600` | `1.2473 ms` | `1.4643 ms` | `7.0531 ms` | `0.7907 ms` | `0.9467 ms` | `1087` | `1038` |

Measured delta:

- Actor average `FrameTime` was about `2.73x` the Mass average in this headless `MovementActive` capture.
- Actor average `Exclusive/GameThread/TickActors` was about `7.93x` the Mass average.
- Actor total tick count increased by `1000`, matching the 1000 spawned Actor enemies.
- This measurement validates the expected Actor tick overhead in this local scenario, but it should not be presented as viewport or GPU performance.

## Findings

- The Mass-based scenario currently reaches the expected objective state in standalone, editor server-mode, and staged dedicated server binary runs.
- The staged dedicated server also replicated the final Objective HP result to two standalone clients in a headless log verification.
- Post-arrival movement work drops to `0` moved entities.
- Debug output now exposes runtime state clearly enough for recording:
  - NetMode
  - Objective HP
  - Mass spawned count
  - Last moved count
  - Arrival count
  - Damage count
- The most visible measured performance issue so far was debug visualization overhead, not Mass movement or objective damage logic.
- Dedicated server binary support is now verified at the staged server runtime/log level with source-built UE 5.8.
- Visible two-client Objective HP replication evidence and a short Unreal Insights smoke trace are recorded in `Docs/10_Visible_Demo_Verification.md`.

## Known Limitations

- `-NullRHI` results should not be presented as final rendering performance.
- The visible client evidence verifies replicated Objective HP display, not final viewport performance.
- The Unreal Insights trace is a smoke capture, not a full performance investigation.
- UE 5.8 `ZenStore` cook output needs separate handling; this verification used `-skipzenstore` for loose staged server content.

## Optional Future Profiling Refinements

1. Run repeated captures if tighter variance data is needed.
2. Capture a deeper Unreal Insights session if processor-level analysis becomes necessary.
3. Record `stat unit` / `stat fps` in a visible viewport only if viewport performance claims are needed.
