# MDSProject

MDSProject is a UE5 technical portfolio project that demonstrates a server-authoritative multiplayer defense sandbox with replication, Mass Entity gameplay flow, runtime debug output, profiling notes, and an approval-based AI-assisted development workflow.

This is not a full game. The project is intentionally scoped as an interview-ready technical demonstration.

## Current Status

Implemented and verified:

- Server-authoritative Objective HP actor
- Replicated Objective HP state
- Mass entity spawn-only phase
- Mass movement toward an Objective
- Mass arrival detection
- Server-side Objective damage on valid arrival
- Runtime debug state subsystem
- Standalone and editor server-mode verification logs
- Profiling comparison document for the current Mass scenario
- Post-arrival debug visualization performance fix

Known limitations:

- Dedicated Server target binary build is currently blocked by the Epic Launcher installed UE 5.6 build.
- A source-built UE 5.6 engine install is required before true dedicated server binary build/profiling can be verified.
- Actor-based baseline benchmarking has not been implemented or measured yet.
- `-NullRHI` profiling results are useful for local comparison, but do not represent final viewport or GPU performance.

## Core Scenario

Mass enemies spawn in the Top Down map and move toward a shared Objective.

When a Mass entity arrives, the server records the arrival and applies Objective damage once. The Objective owns authoritative HP. Runtime debug output exposes the current network mode, Objective HP, Mass spawn count, moved count, arrival count, and damage count.

Expected verified final state for the current 16-entity scenario:

```text
MDS Debug | NetMode=Standalone | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

Editor server-mode verification also reached:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

## Technical Focus

- Dedicated Server readiness
- Replication
- Authority / Ownership
- Objective gameplay
- Mass Entity / Mass AI
- Debug UI and logs
- Profiling
- AI-assisted development workflow

## Architecture Summary

The project uses server-authoritative gameplay as the default rule.

- Objective state is owned by the server.
- Objective HP changes are applied only through server-side gameplay logic.
- Mass processors handle scalable enemy simulation phases incrementally.
- Arrival processing applies damage once per arrived entity.
- Debug state is collected and reported through a dedicated debug subsystem.

The Mass work was intentionally split into reviewable phases:

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection
5. Objective damage integration
6. Debug output
7. Profiling comparison

## Profiling Snapshot

The current Mass scenario was measured with `UnrealEditor-Cmd.exe`, `-NullRHI`, and benchmark mode on 2026-06-27.

| Scenario | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Standalone Mass scenario | `2571.43` | `0.39 ms` |
| Editor server-mode Mass scenario | `4500.00` | `0.22 ms` |

A post-arrival slowdown was found in debug visualization. The root cause was repeated per-frame debug sphere drawing after entities had arrived. The fix changed the marker to draw once on first arrival.

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before debug draw fix | `248.76` | `4.02 ms` |
| After debug draw fix | `2163.46` | `0.46 ms` |

These numbers should be discussed as local headless measurements, not production rendering benchmarks.

## Verification Status

Verified:

- Standalone Mass objective scenario reaches expected final Objective HP.
- Editor server-mode run reports `NetMode=DedicatedServer`.
- Mass entities spawn, move, arrive, and stop contributing movement work after arrival.
- Objective damage event count matches arrival count.
- Debug output exposes the expected runtime state.
- Profiling logs show the debug visualization fix improved post-arrival frame time.

Not yet verified:

- Dedicated Server target binary build, due to installed engine distribution limitation.
- Source-built UE dedicated server run.
- Actor-based enemy baseline comparison.
- Unreal Insights trace capture.
- Full client viewport recording workflow.

## Documentation Map

- `Docs/00_Project_Goal.md` - project goal, scope, target date, and interview value
- `Docs/01_Requirements.md` - MVP requirements and verification mapping
- `Docs/02_Architecture.md` - high-level architecture and system responsibilities
- `Docs/03_MVP_Task_Breakdown.md` - phased MVP task breakdown
- `Docs/07_Mass_Concept.md` - Mass implementation concept notes
- `Docs/08_Profiling_Comparison.md` - current Mass profiling results and limitations
- `Docs/AI_Harness.md` - AI-assisted development workflow
- `Docs/Task_Template.md` - reusable task request template
- `Docs/Approval_Report_Template.md` - reusable completion report template
- `Docs/Verification.md` - verification standards
- `Docs/Unreal_Rules.md` - Unreal C++ and multiplayer implementation rules
- `Docs/Mass_Rules.md` - Mass Entity / Mass AI working rules

## AI-Assisted Workflow

Non-trivial work follows an approval-based workflow:

1. Inspect relevant files.
2. Summarize current structure.
3. Propose a plan.
4. Wait for explicit approval.
5. Implement only approved changes.
6. Verify the result.
7. Provide an approval report.

AI assistance is used as a controlled accelerator. Human review owns goals, scope, architecture decisions, and final verification judgment.

## Interview Discussion Points

- Why Objective HP is server-authoritative.
- How replicated Objective state differs from client-requested gameplay actions.
- Why Mass work was split into spawn, movement, arrival, damage, debug, and profiling phases.
- How debug visualization caused measurable runtime cost after arrival.
- Why `-NullRHI` profiling is useful for comparison but limited as a performance claim.
- What remains before claiming true dedicated server binary support.
- How an Actor baseline should be designed before making Actor-vs-Mass claims.

## Explicit Non-Scope

- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Large UI framework
- Complex animation system
- Full GAS expansion
- Full production-quality game content

## Target Completion Date

`2026-07-31`
