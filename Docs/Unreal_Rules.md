# Unreal Rules

This document defines Unreal Engine C++ and multiplayer implementation rules for `MDSProject`.

Server-authoritative gameplay is the default. Broad refactors are allowed only when explicitly requested and approved.

## Class Structure

- Keep each class focused on one clear responsibility.
- Follow existing project conventions before adding new structure.
- Do not rename files, classes, functions, variables, or folders unless explicitly requested.
- Prefer small focused helpers, components, or subsystems over broad gameplay class changes.
- Keep technical portfolio value in mind; do not expand the project into a full game system.

## UCLASS / USTRUCT / UENUM

Use Unreal reflection only when it supports serialization, editor integration, replication, Blueprint exposure, delegates, or tooling.

Rules:

- Keep reflected types minimal.
- Prefer `USTRUCT` for small data containers.
- Use `UENUM` for stable gameplay state when it improves readability or tooling.
- Do not expose mutable client access to server-owned gameplay state.
- Do not add Blueprint exposure unless the task needs it.

## UPROPERTY / UFUNCTION

Use `UPROPERTY` when Unreal needs to track lifetime, GC visibility, serialization, editor exposure, or replication.

Use `UFUNCTION` when Unreal reflection is needed for RPCs, delegates, Blueprint access, editor tooling, or console/tool integration.

Rules:

- Add Blueprint access intentionally, not by default.
- Replicated properties must have a clear owner, update path, and verification plan.
- Avoid public mutable properties unless the class is specifically a data container or Unreal tooling requires it.
- Prefer explicit functions for gameplay state changes.

## BeginPlay / Tick

- Constructor logic must not depend on world state, players, networking, subsystems, or runtime actors.
- Runtime world/actor access is safer in or after `BeginPlay`.
- Do not assume `BeginPlay` order; check required actors, components, and subsystems before use.
- Add Tick only when necessary.
- Tick work must be bounded.
- Avoid allocation, expensive searches, and log spam in Tick.
- Treat Tick changes as performance-sensitive and verify accordingly.

## Replication

- The server owns gameplay state.
- Clients observe replicated results.
- Register replicated properties in `GetLifetimeReplicatedProps`.
- Use replication conditions intentionally.
- Do not replicate derived state unless there is a clear reason.
- Do not use replication to hide unclear ownership.
- Do not let clients directly mutate replicated server-owned gameplay state.

## Authority / Ownership

- Server-authoritative gameplay is the default.
- Clients may request actions, but the server validates and applies gameplay results.
- Health, damage, score, and Objective HP must not be client-owned.
- Authority checks are required at gameplay state mutation points.
- Ownership assumptions must be stated when adding RPCs or client/server interaction.

## RPC Usage

- Use Server RPCs only for client requests that the server validates and applies.
- Use Client RPCs only for targeted messages to an owning client.
- Use NetMulticast RPCs sparingly.
- Do not use RPCs to bypass replicated state ownership.
- Avoid high-frequency reliable RPCs.
- Prefer replicated state for durable gameplay outcomes.

## OnRep Usage

- Use `OnRep` for client-side presentation, cache updates, or local reactions to replicated state.
- `OnRep` must not become an authoritative gameplay source.
- Server-side state changes should happen before client observation.
- Do not make server gameplay decisions inside `OnRep`.

## Health / Damage / Objective HP

- Health, damage, and Objective HP are server-owned gameplay state.
- Damage may be requested or triggered by gameplay, but the server applies the result.
- Clients observe results through replication or approved server-to-client messaging.
- Objective damage must not be applied from client-only paths.
- Any score, win/loss, or objective state derived from Objective HP must remain server-owned.

## Dedicated Server

- Server logic must not depend on local player, viewport, HUD, input, audio, or visual-only systems.
- Guard client-only presentation code so it does not run on dedicated servers.
- Logs must be useful for diagnosing server-side behavior.
- Network changes require listen-server or dedicated-server verification notes.
- Dedicated server checks should report launch method, map, server log result, client log result, and observed gameplay result.

## Build.cs

When changing `.Build.cs`, explain why each added module is required.

Rules:

- Add only modules required by the approved task.
- Separate runtime and editor dependencies.
- Do not add broad dependency sets speculatively.
- Prefer include cleanup over adding dependencies to hide coupling.

## Logging

- Prefer project-specific log categories.
- Log authority-sensitive events where server/client diagnosis benefits from them.
- Avoid per-frame log spam.
- Include useful context such as actor, role/net mode, state, and result.
- Use `Warning` or `Error` for actionable problems or invalid/rejected paths.

## Debug-Only Code

- Keep debug UI, debug draw, and profiling helpers separate from authoritative gameplay logic.
- Guard debug-only behavior when needed.
- Dedicated server behavior must not depend on visual debug systems.
- Debug output should report state rather than create gameplay state.

## Performance-Sensitive Code

Treat these areas as performance-sensitive:

- Tick
- spawning
- Mass processing
- replication frequency
- debug UI updates
- logging
- profiling harnesses

Rules:

- Avoid hot path allocation.
- Avoid expensive world searches in frequent updates.
- Avoid high-frequency reliable RPCs.
- Avoid unnecessary replication.
- Record FPS, frame time, GameThread impact, or relevant context when making performance claims.

## Network Change Verification

Network or replication changes should report:

- server-side source of truth
- client request path, if any
- server validation and application result
- client-visible replicated result
- RPC ownership and direction
- `OnRep` behavior
- listen-server or dedicated-server result
- relevant server/client logs

Manual inspection alone does not replace runtime network verification when behavior changes.
