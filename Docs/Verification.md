# Verification Guide

This document defines verification standards for `MDSProject`.

Verification must be accurate and specific. Do not report a test, build, compile, PIE session, dedicated server run, log check, or profiling pass as successful unless it was actually run.

Manual inspection is useful, but it is not the same as runtime verification.

## Build / Compile Checks

Use build or compile checks for C++, module, plugin, config, and Unreal API changes.

Report:

- Target built or compiled
- Build configuration
- Platform
- Result
- Relevant errors or warnings

If `.Build.cs` or module dependencies changed, report why each added, removed, or changed module was required.

Do not claim a successful build if only files were inspected.

## Editor Startup Checks

Use editor startup checks when changes may affect assets, config loading, modules, plugins, game modes, maps, subsystems, or editor-facing APIs.

Report:

- Whether the editor opened successfully
- Project or map loaded
- Any startup warnings, errors, crashes, missing modules, or asset load failures
- Whether the issue appears related to the current change

If the editor cannot be started, state the reason.

## PIE Single-Player Checks

Use PIE single-player checks for local gameplay flow, objective behavior, UI visibility, input handling, and basic runtime errors.

Report:

- Map or test context used
- Number of PIE players
- What action was performed
- Expected result
- Observed result
- Runtime warnings or errors

PIE single-player checks do not verify multiplayer authority, ownership, or replication.

## PIE Listen-Server Checks

Use PIE listen-server checks for multiplayer behavior that can be validated with an editor-hosted server and one or more clients.

Report:

- Number of players
- Which instance acted as server
- Which instance acted as client
- Server-observed result
- Client-observed result
- Any ownership, RPC, or replication notes

Network-related changes require server/client verification notes. If listen-server verification was not run, state why.

## Dedicated Server Checks

Use dedicated server checks for server-authoritative gameplay, standalone client behavior, objective state, replicated data, and changes intended to demonstrate dedicated server support.

Report:

- Dedicated server target or launch method
- Client launch method
- Map or test context
- Server log result
- Client log result
- Observed gameplay result

If a dedicated server check is required but cannot be executed, state the reason and list the closest available substitute, such as PIE listen-server testing.

## Network Replication Checks

Use network replication checks for any change involving replicated properties, RPCs, authority checks, ownership, damage, health, score, objective HP, spawning, possession, or client-visible gameplay state.

Report:

- Server authority path
- Client request path, if any
- Replicated data observed on clients
- RPC ownership assumptions
- Lifetime replicated properties affected
- Server/client result notes

Server-authoritative gameplay is the default. Clients may request actions, but the server must validate and apply gameplay results.

## Objective Gameplay Checks

Use objective gameplay checks for changes involving base health, objective HP, score, enemy arrival, win/loss conditions, damage application, or defense goals.

Report:

- Initial objective state
- Action that changed the objective state
- Server-side result
- Client-visible result, when networked
- Expected and observed final state

Objective gameplay state must be owned by the server unless the approved task explicitly states otherwise.

## Mass Entity Checks

Use Mass Entity checks for Mass spawn, movement, arrival detection, objective damage integration, debug visualization, or profiling work.

Report when applicable:

- Entity count
- Spawn behavior
- Movement or processor behavior
- Arrival or damage behavior
- Performance impact
- Relevant Mass warnings or errors

Mass-related work must be incremental. Do not treat spawn, movement, arrival detection, and objective damage as verified together unless all of those behaviors were actually tested.

## Debug UI Checks

Use Debug UI checks for changes involving runtime status display, debug panels, counters, toggles, overlays, logs, or developer-facing feedback.

Report:

- UI entry point or display location
- Values shown
- How values were produced
- Whether values update at runtime
- Server/client visibility, if networked

Debug UI should support technical understanding and interview discussion without hiding authority or replication behavior.

## Log Review

Use log review for builds, editor startup, PIE, dedicated server, client runs, Mass warnings, replication warnings, runtime ensures, and crashes.

Report:

- Log source
- Relevant warnings
- Relevant errors
- Crashes, ensures, or failed loads
- Whether findings are new, pre-existing, or unknown

Log review can support verification, but log review alone does not prove gameplay behavior unless the task only required log behavior.

## Profiling Checks

Use profiling checks for changes that may affect performance, Tick cost, Mass processing, entity counts, debug UI overhead, spawning, movement, or server load.

Report when relevant:

- FPS
- Frame time
- GameThread impact
- Entity count or actor count
- Test map or scenario
- Whether measurement was editor, PIE, standalone, client, or server

Profiling numbers should include enough context to be useful in an interview or future comparison.

## Manual Test Steps

Manual test steps must be concrete and reproducible.

Include:

1. Setup or map to open
2. Number of players or server mode
3. Actions to perform
4. Expected result
5. Observed result, if already tested

Manual inspection should be labeled as manual inspection. Runtime checks should be labeled with the runtime mode used, such as PIE single-player, PIE listen-server, standalone, or dedicated server.

## When Verification Cannot Be Executed

If verification cannot be executed, state:

- Which verification step was not run
- Why it could not be run
- What was checked instead
- What remains unverified
- Recommended next manual check

Acceptable reasons may include missing project files, unavailable editor, missing build target, environment limitations, dependency failure, or insufficient task scope.

Do not replace a required runtime check with manual inspection without clearly stating the limitation.

