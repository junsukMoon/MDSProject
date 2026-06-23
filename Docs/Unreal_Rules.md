# Unreal Rules

This document defines Unreal Engine C++ and multiplayer implementation rules for `MDSProject`.

Server-authoritative gameplay is the default. Broad refactoring is not allowed unless explicitly requested and approved.

## Unreal C++ Class Structure

Keep classes focused on one responsibility.

Prefer small, reviewable additions over changing large existing gameplay classes. Add a new component or helper class when it keeps ownership and behavior clearer.

Class names, file names, and module placement must follow existing project conventions once source files exist.

Do not rename existing files, classes, functions, variables, or folders unless explicitly requested.

## UCLASS / USTRUCT / UENUM Usage

Use `UCLASS`, `USTRUCT`, and `UENUM` only when Unreal reflection, serialization, Blueprint exposure, replication, editor use, or asset integration requires it.

Rules:

- Keep reflected types minimal and intentional.
- Prefer `USTRUCT` for small data containers that need reflection or replication.
- Prefer `UENUM` for stable gameplay states exposed to Blueprints, logs, or debugging.
- Do not expose implementation-only types to reflection without a reason.
- Use clear names that make editor and Blueprint use understandable.

## UPROPERTY / UFUNCTION Usage

Use `UPROPERTY` when Unreal needs to track lifetime, expose values, serialize state, replicate state, or show data in the editor.

Use `UFUNCTION` when Unreal needs reflection, Blueprint access, RPC support, delegates, console integration, or editor tooling.

Rules:

- Keep Blueprint exposure deliberate.
- Use access specifiers and metadata to communicate intended use.
- Do not expose mutable gameplay state to clients unless the server remains the source of truth.
- Replicated properties must have a clear owner, update path, and verification plan.
- Avoid public mutable properties unless they are intentionally editor-tuned or Blueprint-facing.

## Actor Component Usage

Prefer Actor Components for reusable actor behavior with clear ownership.

Use components when behavior can be attached to multiple actors or when separating behavior improves reviewability.

Rules:

- The owning actor should remain responsible for authority-sensitive decisions unless ownership is explicitly delegated.
- Components that change gameplay state must respect server authority.
- Replicated components must clearly define what replicates and why.
- Avoid adding a component when a simple private helper function is enough.

## UObject Lifetime and Garbage Collection

Respect Unreal's object lifetime and garbage collection rules.

Rules:

- Store UObject references in `UPROPERTY` fields when they must be kept alive or tracked by GC.
- Avoid raw UObject pointers unless lifetime is guaranteed and documented by context.
- Do not create UObjects with unclear ownership.
- Do not assume constructors can safely access world, game mode, controller, replicated state, or runtime-only actors.
- Prefer initialization points that match the object lifecycle.

## BeginPlay / Tick Usage

Use `BeginPlay` for runtime initialization that requires the world or spawned actors.

Be careful with BeginPlay order. Do not assume all dependent actors, components, replicated state, or controllers are ready unless the code verifies that condition.

Tick should not be added casually.

Rules:

- Disable Tick by default unless per-frame work is required.
- Prefer timers, events, delegates, Mass processors, or explicit state changes before adding Tick.
- If Tick is required, explain why and keep the work bounded.
- Avoid expensive searches, allocations, logging spam, and replicated state changes in Tick.
- Profiling-sensitive Tick changes require performance verification notes.

## Replication

Replicated state must have a clear server-side source of truth.

Rules:

- The server owns gameplay state changes.
- Clients observe replicated results.
- Replicated properties must be listed in lifetime replication.
- Replication conditions must be intentional and explainable.
- Do not replicate derived state if clients can safely derive it from existing replicated data.
- Do not use replication as a substitute for clear ownership.

Network changes require server/client verification notes.

## Authority / Ownership

Authority and ownership must be explicit in gameplay code.

Rules:

- Server-authoritative gameplay is the default.
- Clients may request gameplay actions, but the server validates and applies results.
- Client-owned actors may send valid owner-routed requests to the server.
- The server must reject or ignore invalid requests.
- Gameplay state such as health, damage, score, and objective HP must not be client-owned.
- Use authority checks where gameplay state can change.

## RPC Usage

RPCs must be justified by ownership and direction.

Rules:

- Use Server RPCs for client requests that must be validated and applied by the server.
- Use Client RPCs only when a specific owning client needs a targeted message.
- Use NetMulticast RPCs sparingly and only for server-initiated events that must reach relevant clients.
- Do not use RPCs to bypass replicated state ownership.
- Avoid reliable RPCs for high-frequency or spam-prone events.
- Document the caller, target, validation expectation, and ownership assumption in the plan or approval report.

## OnRep Usage

Use `OnRep` functions for client-side responses to replicated state changes.

Rules:

- `OnRep` should update presentation, cached derived values, or local reactions.
- `OnRep` should not become the authoritative gameplay source.
- Server-side state changes should happen before replication.
- Keep `OnRep` functions deterministic and lightweight.
- Avoid triggering new server gameplay decisions from `OnRep`.

## Health / Damage / Objective HP Ownership

Health, damage, and objective HP are server-owned gameplay state.

Rules:

- Damage is requested or triggered by gameplay, then validated and applied on the server.
- Health and objective HP changes happen on the server.
- Clients receive the result through replication or approved server-to-client messaging.
- Objective damage from enemies, Mass agents, or player actions must not be applied only on a client.
- Score or win/loss state derived from objective HP must also be server-owned.

## Dedicated Server Considerations

Dedicated server support is a core project goal.

Rules:

- Do not rely on local player, viewport, HUD, input, audio, or visual-only systems for server gameplay decisions.
- Guard client-only presentation code from dedicated server execution when needed.
- Server logic must run without a rendered viewport.
- Logs should make server-side behavior diagnosable.
- Networked changes should include listen-server or dedicated-server verification notes.

## Build.cs Dependency Changes

`.Build.cs` changes require a reason for each added, removed, or changed module.

Rules:

- Add only modules required by the approved task.
- Explain why each module is needed in the plan or approval report.
- Do not add broad dependency sets speculatively.
- Prefer the narrowest module dependency that supports the implementation.
- If a dependency is only for editor code, keep it separated from runtime modules when applicable.

## Logging Conventions

Use logging to support debugging and interview discussion without creating noise.

Rules:

- Prefer project-specific log categories once they exist.
- Log authority-sensitive events from the server path when useful.
- Avoid per-frame log spam.
- Include enough context to diagnose actor, role, state, and result.
- Do not use logs as the only verification for gameplay behavior unless the task is specifically about logs.

## Debug-Only Code

Debug-only code must not become required for gameplay correctness.

Rules:

- Keep debug UI, debug drawing, and profiling helpers separate from authoritative gameplay logic.
- Guard debug-only behavior when appropriate.
- Ensure dedicated server behavior does not depend on visual debug systems.
- Debug UI should reveal server/client state clearly when networked.

## Performance-Sensitive Code

Treat Tick, spawning, Mass processing, replication frequency, debug UI updates, and logging as performance-sensitive.

Rules:

- Avoid unnecessary allocations in hot paths.
- Avoid expensive world searches during frequent updates.
- Avoid high-frequency reliable RPCs.
- Avoid replicating data more often than necessary.
- Keep Mass-related work incremental and measurable.
- Profiling checks should record FPS, frame time, and GameThread impact when relevant.

## What Must Be Verified After Network Changes

After network or replication-related changes, verify and report:

- Server-side source of truth
- Client request path, if any
- Server validation and application of results
- Client-visible replicated result
- RPC ownership and direction
- `OnRep` behavior, if used
- Listen-server or dedicated-server result
- Relevant server and client log output

Manual inspection alone is not runtime network verification. If runtime verification cannot be executed, state the reason and list what remains unverified.

