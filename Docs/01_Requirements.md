# MVP Requirements

## Overview

This document defines the MVP requirements for `MDSProject`.

The MVP is a technical portfolio milestone for Unreal Engine client/gameplay programming interviews. It should demonstrate server-authoritative multiplayer gameplay, objective state, Mass Entity enemies, debug visibility, profiling readiness, and a controlled AI-assisted development workflow.

## MVP Functional Requirements

- The project must support multiplayer testing.
- The project should be compatible with a dedicated-server style workflow.
- The server must own authoritative gameplay state.
- The Objective must have server-authoritative HP.
- Mass-based enemies must be spawnable.
- Mass-based enemies must move toward the Objective.
- Arrival at the Objective must be detectable.
- Objective HP must decrease only from valid server-authoritative arrival/damage logic.
- Debug UI or logs must expose key runtime state.
- Profiling data must later support Actor-based vs Mass-based comparison.

## Non-Functional Requirements

- Keep systems small and reviewable.
- Prefer incremental implementation.
- Avoid broad refactoring.
- Keep the project suitable for technical interview explanation.
- Keep implementation scope realistic for completion by `2026-07-31`.
- Keep gameplay scope focused on technical demonstration, not full game production.
- Document assumptions, verification gaps, and risks after each non-trivial task.

## Networking / Authority Requirements

- The server is the source of truth for HP, damage, score, objective damage, and enemy arrival results.
- Clients may display state but must not directly apply authoritative gameplay results.
- Clients may request gameplay actions, but the server must validate and apply results.
- Replicated state must have clear verification steps.
- Replicated state must have a clear server-side source of truth.
- RPCs must be justified by ownership and direction.
- Network-related changes must include server/client verification notes.

## Mass Entity Requirements

- Mass work must follow the order defined in `Docs/Mass_Rules.md`.
- Spawn, movement, arrival detection, and objective damage integration must be separate tasks unless explicitly approved.
- Each Mass task must explain Entity, Fragment, Processor, Spawner, and Representation responsibilities before implementation.
- Mass spawn tasks must report entity count and spawn behavior.
- Mass movement tasks must report movement or processor behavior.
- Mass arrival tasks must report arrival detection behavior.
- Mass objective integration tasks must preserve server-authoritative objective damage.
- Mass profiling tasks must report performance impact when applicable.

## Objective Gameplay Requirements

- The Objective must expose HP as server-owned gameplay state.
- Objective HP must change only through validated server gameplay logic.
- Enemy arrival must be distinguishable from objective damage application.
- Objective damage must be measurable through debug UI, logs, or verification notes.
- Clients may observe objective state but must not own objective HP changes.
- Win/loss or score state derived from objective HP must also be server-owned if implemented.

## Debug UI / Logging Requirements

- Debug UI or logs must expose key runtime state relevant to the current task.
- Key state may include authority role, objective HP, enemy count, spawn state, movement state, arrival state, damage events, and relevant Mass state.
- Debug output must not become required for gameplay correctness.
- Debug UI should make server/client differences clear when networked.
- Logging should support diagnosis without per-frame spam.

## Profiling Requirements

- Profiling work should support future Actor-based vs Mass-based comparison.
- Profiling notes should record FPS, frame time, and GameThread impact when relevant.
- Profiling notes should include scenario context, entity or actor count, and runtime mode.
- Mass profiling must identify entity count and performance impact when applicable.
- Profiling should be used to support interview discussion, not to overbuild the project.

## Explicit Non-Requirements

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

## Acceptance Criteria

The MVP is acceptable when:

- Multiplayer testing can be performed through an approved workflow.
- A dedicated-server style workflow is supported or documented.
- Objective HP is owned by the server.
- Clients can observe relevant objective state without directly applying authoritative changes.
- Mass-based enemies can be spawned in a controlled scenario.
- Mass-based enemies can move toward the Objective.
- Arrival at the Objective can be detected separately from damage application.
- Objective HP decreases only through valid server-authoritative arrival/damage logic.
- Debug UI or logs expose enough runtime state to inspect authority, objective state, and Mass behavior.
- Profiling notes can support later Actor-based vs Mass-based comparison.
- Verification notes identify what was actually run and what remains unverified.
- The result remains focused on technical portfolio value rather than full game production.

## Verification Mapping

Use `Docs/Verification.md` as the detailed verification guide.

- Multiplayer testing: PIE listen-server checks or dedicated server checks.
- Dedicated-server style workflow: dedicated server checks, server/client logs, or documented verification limitation.
- Server-authoritative gameplay state: network replication checks and server/client verification notes.
- Objective HP: objective gameplay checks, replication checks when networked, and log or debug review.
- Mass enemy spawn: Mass Entity checks with entity count and spawn behavior.
- Mass enemy movement: Mass Entity checks with movement or processor behavior.
- Arrival detection: Mass Entity checks and objective gameplay checks.
- Objective damage: objective gameplay checks, server authority checks, and client-visible result checks when networked.
- Debug UI or logs: Debug UI checks and log review.
- Profiling comparison readiness: profiling checks with FPS, frame time, GameThread impact, scenario context, and entity or actor count when relevant.
- Non-functional scope control: manual inspection against this document, `Docs/Mass_Rules.md`, and `Docs/Unreal_Rules.md`.

Do not report any verification step as passed unless it was actually run. If verification cannot be executed, state the reason and list what remains unverified.

