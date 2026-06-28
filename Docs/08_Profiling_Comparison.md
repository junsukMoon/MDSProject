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

An equivalent Actor-based enemy/objective benchmark has not been implemented yet.

For that reason, this document does not claim an Actor-vs-Mass performance win.

Current verified comparison:

- Mass scenario: measured.
- Actor scenario: not measured.

To create a valid Actor baseline later, the Actor scenario should match:

- Same map.
- Same entity count: `16`.
- Same spawn grid.
- Same movement target.
- Same objective HP: `100`.
- Same damage per arrival: `5`.
- Same final damage events: `16`.
- Same runtime mode and measurement method.

## Findings

- The Mass-based scenario currently reaches the expected objective state in standalone, editor server-mode, and staged dedicated server binary runs.
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
- Client viewport replication has not been verified in this phase.
- No Actor baseline has been implemented or measured yet.
- No Unreal Insights trace was captured.
- UE 5.8 `ZenStore` cook output needs separate handling; this verification used `-skipzenstore` for loose staged server content.

## Recommended Next Profiling Steps

1. Re-run the same scenario in a visible viewport and record `stat fps` / `stat unit`.
2. Capture Unreal Insights trace once a stable recording workflow is available.
3. Build a minimal Actor-based baseline only if an actual Actor-vs-Mass comparison is needed.
4. Add a client connection check against the staged dedicated server and verify replicated Objective HP from the client view.
