# Actor vs Mass Baseline Profile

Date: 2026-06-29

Engine: UE 5.8 source build at `C:\UnrealEngine`

Mode: `MDSProjectEditor-Cmd.exe -game -NullRHI -nosound -unattended`

Map: `/Game/TopDown/Lvl_TopDown`

## Purpose

Capture an initial Actor enemy baseline versus Mass baseline comparison using the same movement target, movement speed, arrival distance, and objective damage values.

This is a CPU-oriented headless smoke profile, not a final gameplay FPS benchmark.

## Runs

### Mass 1000

Command:

```text
MDSProjectEditor-Cmd.exe MDSProject.uproject -game -NullRHI -nosound -unattended -MDSMassBaselineCount=1000 -csvCaptureFrames=600 -ExitAfterCsvProfiling -ABSLOG=C:\Temp\MDS_Profile_Mass1000.log
```

Observed log:

```text
Mass objective-damage probe initialized 1000 entities ... Requested count: 1000.
Capture Ended. Writing CSV to file : ../../../MDSProject/Saved/Profiling/CSV/Profile(20260629_015852).csv
Frames : 600
```

### Actor 1000

Command:

```text
MDSProjectEditor-Cmd.exe MDSProject.uproject -game -NullRHI -nosound -unattended -MDSMassBaselineCount=1 -MDSActorBaseline -MDSActorBaselineCount=1000 -csvCaptureFrames=600 -ExitAfterCsvProfiling -ABSLOG=C:\Temp\MDS_Profile_Actor1000.log
```

Observed log:

```text
Mass objective-damage probe initialized 1 entities ... Requested count: 1.
Actor enemy baseline spawned 1000 enemies ... Requested count: 1000.
Capture Ended. Writing CSV to file : ../../../MDSProject/Saved/Profiling/CSV/Profile(20260629_015953).csv
Frames : 600
```

## CSV Summary

Values are in milliseconds unless noted. The first 10 filtered frames were dropped before calculating averages and p95.

| Scenario | Sampled frames | Avg FrameTime | P95 FrameTime | Avg TickActors | P95 TickActors | Tick count signal |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Mass 1000 | 591 | 37.60 | 41.08 | 36.64 | 40.06 | `Ticks/MassProcessingPhase` avg 5.99 |
| Actor 1000 | 589 | 1.20 | 1.48 | 0.79 | 1.02 | `Ticks/MDSActorEnemy` avg 1000 |

Additional observed CSV counters:

| Scenario | ActorCount/MDSActorEnemy | ActorCount/TotalActorCount | Basic/TicksQueued |
| --- | ---: | ---: | ---: |
| Mass 1000 | not present | avg 86.85 | avg 37.94 |
| Actor 1000 | avg 1000.00 | avg 1088.00 | avg 1038.00 |

## Notes

- `MassActors/NumSpawned` remained `0` in CSV, so the Mass entity count was verified from the runtime log instead of that CSV counter.
- The Actor run used `-MDSMassBaselineCount=1` because Mass baseline currently has no disable flag. That leaves one Mass entity in the Actor run.
- `GameThreadTime`, `RenderThreadTime`, and `GPUTime` were `0` in these NullRHI CSV captures, so this document uses `FrameTime` and `Exclusive/GameThread/TickActors`.
- The CSV capture starts at boot with `-csvCaptureFrames=600`; a gameplay-only `CsvProfile STARTFILE=... FRAMES=600` command did not auto-exit in this environment and was not used for the final numbers.
- Repeated Editor-Cmd warnings were observed, including `LogAutomationTest: Error: Saving and loading a serialized object containing FText properties failed...`, `LogEditorDataStorageUI` widget factory warnings, and a RecastNavMesh serialized tile count warning. No Actor/Mass runtime crash was observed.

## Follow-up

For a stricter interview-grade comparison, add a tiny profiling harness or console command that starts CSV capture after `OnWorldBeginPlay` and exits after a fixed gameplay-frame window. Also add a Mass baseline enable/disable CVar so the Actor run can be fully isolated without the `-MDSMassBaselineCount=1` workaround.
