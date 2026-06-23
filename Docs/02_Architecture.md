# MVP Architecture

## Architecture Overview

`MDSProject` is a server-authoritative multiplayer defense sandbox for Unreal Engine client/gameplay programming interviews.

The MVP architecture separates player input, combat requests, objective state, Mass-based enemy simulation, network authority, debug visibility, profiling, and AI-assisted documentation workflow.

The architecture is intentionally small. It should demonstrate clear ownership, replication reasoning, Mass Entity boundaries, and verification discipline without becoming a full game framework.

## Major Systems

- Player
- Combat
- Objective
- MassAI
- Network
- DebugUI
- Profiling
- Docs / AI Harness

## System Responsibilities

Player:

- Owns local input and local control.
- Provides the weapon/action request entry point.
- Sends gameplay requests through approved server-authoritative paths.
- Does not directly apply authoritative gameplay results on clients.

Combat:

- Defines the boundary between damage requests and damage application.
- Routes valid gameplay requests toward server-side validation.
- Keeps damage application separate from input and presentation.

Objective:

- Owns objective HP, damage handling, and destruction state.
- Treats objective HP as server-authoritative gameplay state.
- Exposes state through replication, logs, or debug UI as approved.

MassAI:

- Owns enemy entity data, spawn behavior, movement behavior, and arrival detection.
- Keeps spawn, movement, arrival detection, and objective damage integration in separate tasks unless explicitly approved.
- Handles large enemy simulation responsibilities without becoming a full AI system.

Network:

- Defines authority, replication, ownership, and server/client validation expectations.
- Ensures replicated state has a server-side source of truth.
- Requires server/client verification notes for network changes.

DebugUI:

- Visualizes key runtime state such as authority role, objective HP, enemy count, spawn state, movement state, arrival state, and damage events.
- Must remain lightweight and must not become a large UI framework.
- Must not be required for gameplay correctness.

Profiling:

- Defines measurement points and comparison data.
- Supports later Actor-based vs Mass-based comparison.
- Records FPS, frame time, GameThread impact, runtime mode, and entity or actor count when relevant.

Docs / AI Harness:

- Controls task planning, approval, verification, and report history.
- Keeps work small, reviewable, and interview-oriented.
- Prevents scope expansion into full game production.

## Data Flow

Primary gameplay flow:

1. Player input creates a local action or weapon request.
2. The request is routed to the server through an approved ownership path.
3. The server validates the request.
4. Combat applies valid gameplay results on the server.
5. Objective updates server-owned HP when valid damage is applied.
6. Replicated state, logs, or debug UI expose the result to clients and developers.

MassAI objective flow:

1. MassAI spawns enemy entities in an approved scenario.
2. MassAI moves entities toward the Objective.
3. MassAI detects arrival separately from damage application.
4. Arrival requests objective damage through a server-authoritative path.
5. Objective applies valid damage on the server.
6. Debug UI, logs, and profiling notes expose entity count, arrival behavior, objective HP, and performance impact.

## Networking / Authority Model

Server-authoritative gameplay is the default.

- The server owns HP, damage, score, objective damage, objective HP, and enemy arrival results.
- Clients may display state but must not directly apply authoritative gameplay results.
- Clients may request actions, but the server validates and applies results.
- Replicated state must have a clear server-side source of truth.
- RPCs must be justified by ownership and direction.
- Network changes require listen-server or dedicated-server verification notes.

Manual inspection alone is not runtime network verification.

## Mass Entity Architecture

MassAI is responsible for scalable enemy simulation, not full enemy gameplay production.

Mass work must follow the incremental order from `Docs/Mass_Rules.md`:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI integration
8. Profiling comparison

Mass boundaries:

- Fragments store focused entity data.
- Tags classify simple entity state.
- Processors perform one behavior at a time.
- Spawners create only the approved entity scenario.
- Representation stays separate from authoritative gameplay state.

Objective damage should be integrated only after spawn, movement, and arrival detection are implemented separately, unless explicitly approved.

## Objective System Architecture

The Objective system is the authoritative owner of objective HP.

Responsibilities:

- Store objective HP on the server.
- Accept damage only through validated server-side logic.
- Track destruction state if implemented.
- Expose current state through replication, logs, or debug UI.
- Keep objective damage separate from arrival detection until integration is explicitly approved.

Clients may observe objective state but must not own HP changes.

## Debug UI / Logging Architecture

Debug UI and logs expose runtime state for development and interview discussion.

They should show, when relevant:

- Authority role
- Objective HP
- Enemy count
- Spawn state
- Movement state
- Arrival state
- Damage events
- Server/client differences
- Relevant Mass state

Debug UI must stay lightweight. Logs must avoid per-frame spam. Neither debug UI nor logs should become required for gameplay correctness.

## Profiling Architecture

Profiling supports later Actor-based vs Mass-based comparison.

Profiling notes should capture, when relevant:

- Scenario or map context
- Runtime mode
- FPS
- Frame time
- GameThread impact
- Entity count or actor count
- Baseline comparison

Profiling should support technical explanation and decision-making, not scope expansion.

## AI-Assisted Development Boundaries

AI assistance is part of the architecture only as a controlled workflow.

Rules:

- Human approval is required before non-trivial file changes.
- AI must inspect relevant files before proposing implementation.
- AI must implement only approved changes.
- AI must verify results or state why verification was not possible.
- AI must provide an approval report after implementation.
- AI must not broaden scope, perform broad refactors, or create full-game systems unless explicitly requested.

## Initial Folder / File Direction

Exact source paths should be chosen during approved implementation tasks after inspecting the project structure.

Initial direction:

- Player-related code should stay near existing player or pawn/controller code once present.
- Combat code should stay near the damage request/application boundary.
- Objective code should be isolated enough to make server-owned HP easy to review.
- MassAI code should be grouped so fragments, tags, processors, spawners, and representation remain understandable.
- DebugUI code should remain lightweight and separate from authoritative gameplay logic.
- Profiling notes should live in documentation unless an approved implementation task creates runtime profiling helpers.

Do not create folders, files, classes, or modules from this architecture document alone.

## Implementation Order

Recommended MVP implementation order:

1. Confirm project build and baseline startup.
2. Establish multiplayer or dedicated-server test path.
3. Add server-authoritative Objective HP.
4. Add minimal debug logs or debug UI for authority and objective HP.
5. Add Mass concept and module setup.
6. Add Mass spawn only.
7. Add Mass movement only.
8. Add Mass arrival detection only.
9. Integrate server-authoritative objective damage.
10. Expand debug UI/logging for Mass and objective state.
11. Add profiling comparison notes for Actor-based vs Mass-based approaches.

Each step should be planned, approved, implemented, verified, and reported separately unless explicitly approved otherwise.

## Risks and Constraints

- Overbuilding combat, UI, AI, or animation would weaken the technical portfolio focus.
- Combining Mass spawn, movement, arrival detection, and damage can hide bugs and make verification weak.
- Client-side authority mistakes can invalidate multiplayer behavior.
- Tick, logging, debug UI, and Mass processors can create performance issues if added casually.
- Dedicated server behavior can break if gameplay relies on viewport, HUD, local player, or visual-only systems.
- Source structure is not defined by this document; implementation tasks must inspect existing files first.

## Interview Explanation Points

This architecture should help explain:

- Why the project is a technical sandbox, not a full game.
- How server authority protects gameplay state.
- How clients request actions and observe results.
- Why objective HP is owned by the server.
- Why MassAI is split into spawn, movement, arrival, and damage integration steps.
- How debug UI and logs expose runtime state without becoming a large UI framework.
- How profiling supports Actor-based vs Mass-based comparison.
- How AI-assisted development is controlled through approval gates and verification reports.

