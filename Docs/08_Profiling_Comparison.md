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

- This was a headless `-NullRHI` log verification. Visible viewport or recorded-video verification is still pending.
- Both clients emitted repeated `LogNetPlayerMovement: Warning: CreateSavedMove: Hit limit of 96 saved moves` warnings under the headless run. The Objective HP replication result remained stable.

### Actor vs Mass 1000 Latest Harness

- Runtime date: 2026-06-29
- Engine: source-built UE 5.8
- Runtime mode: `MDSProjectEditor-Cmd.exe -game -NullRHI -nosound -unattended`
- CSV capture method:
  - `-MDSGameplayProfile`
  - `-MDSGameplayProfileFrames=600`
  - First 10 CSV rows excluded from summary calculations.
- Mass command flags:
  - `-MDSMassBaselineCount=1000`
  - `-MDSGameplayProfileName=Mass1000_LatestHarness`
- Actor command flags:
  - `-NoMDSMassBaseline`
  - `-MDSActorBaseline`
  - `-MDSActorBaselineCount=1000`
  - `-MDSGameplayProfileName=Actor1000_LatestHarness`
- Logs:
  - Mass: `C:\Temp\MDS_Profile_Mass1000_LatestHarness.log`
  - Actor: `C:\Temp\MDS_Profile_Actor1000_LatestHarness.log`
- CSV files:
  - Mass: `MDSProject/Saved/Profiling/CSV/Mass1000_LatestHarness.csv`
  - Actor: `MDSProject/Saved/Profiling/CSV/Actor1000_LatestHarness.csv`

Runtime verification:

- Mass spawned 1000 entities and started a 600-frame gameplay CSV capture.
- Actor run disabled Mass baseline, spawned 1000 Actor enemies, and started a 600-frame gameplay CSV capture.
- Mass CSV wrote 600 frames with captured duration `23.944997` seconds.
- Actor CSV wrote 600 frames with captured duration `0.714544` seconds.

CSV summary:

| Scenario | Sampled frames | Avg FrameTime | P95 FrameTime | Max FrameTime | Avg TickActors | P95 TickActors | Workload signal |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Mass 1000 | 590 | `39.99 ms` | `43.90 ms` | `182.38 ms` | `38.98 ms` | `42.79 ms` | `Ticks/MassProcessingPhase` avg `5.995` |
| Actor 1000 | 591 | `1.17 ms` | `1.31 ms` | `2.87 ms` | `0.68 ms` | `0.77 ms` | `Ticks/MDSActorEnemy` avg `1000` |

Additional counters:

| Scenario | ActorCount/MDSActorEnemy | ActorCount/TotalActorCount | Basic/TicksQueued | MassActors/NumSpawned |
| --- | ---: | ---: | ---: | ---: |
| Mass 1000 | not present | avg `87` | avg `38.01` | avg `0` |
| Actor 1000 | avg `1000` | avg `1087` | avg `1038` | avg `0` |

Important limitations:

- This is a headless `-NullRHI` smoke profile, not a final gameplay FPS benchmark.
- The Mass run was much slower per frame, so the same 600-frame window covered more wall-clock time than the Actor run.
- Objective HP is still `100`, so in 1000-enemy runs objective damage clamps at 20 successful damage events. This profile is useful for movement/tick cost comparison, not full 1000-arrival damage throughput.
- `MassActors/NumSpawned` remained `0`; Mass count was verified from the runtime log.
- Repeated Editor-Cmd startup warnings were observed and are consistent with earlier runs.

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

## Actor-Based Baseline Status

An equivalent minimal Actor-based enemy/objective benchmark is implemented.

The first latest-harness Actor-vs-Mass smoke profile has been captured, but it should not be treated as a final performance claim.

Current verified comparison:

- Mass scenario: measured.
- Actor scenario: measured.

For a stricter interview-grade Actor baseline, the scenario should still be validated in a visible viewport and, if needed, with objective health tuned so all arrivals can apply damage:

- Same map.
- Same entity count.
- Same spawn grid.
- Same movement target.
- Comparable objective HP and damage policy.
- Same damage per arrival: `5`.
- Same runtime mode and measurement method.

## Findings

- The Mass-based scenario currently reaches the expected objective state in standalone, editor server-mode, and staged dedicated server binary runs.
- The staged dedicated server also replicated the final Objective HP result to two standalone clients in a headless log verification.
- The latest-harness Actor 1000 vs Mass 1000 headless smoke profile is captured and documented.
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

## Known Limitations

- `-NullRHI` results should not be presented as final rendering performance.
- Client Objective HP replication has been verified through two standalone client logs, but visible viewport or recorded-video verification is still pending.
- Actor-vs-Mass profiling is currently a headless smoke comparison. It is not a visible viewport or GPU benchmark.
- No Unreal Insights trace was captured.
- UE 5.8 `ZenStore` cook output needs separate handling; this verification used `-skipzenstore` for loose staged server content.

## Recommended Next Profiling Steps

1. Re-run the same scenario in a visible viewport and record `stat fps` / `stat unit`.
2. Capture Unreal Insights trace once a stable recording workflow is available.
3. Record visible client viewport evidence showing both clients display the same replicated Objective HP.
