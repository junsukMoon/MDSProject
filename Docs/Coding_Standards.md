# Coding Standards

This document defines practical coding standards for `MDSProject` C++ work.

It complements:

- `AGENTS.md`
- `Docs/Unreal_Rules.md`
- `Docs/Mass_Rules.md`
- `Docs/Verification.md`

Use this document when writing or reviewing code. Keep the project focused on a technical portfolio sandbox, not a full game.

## Scope Control

- Prefer small, reviewable changes.
- Modify only the files approved for the task.
- Do not perform broad refactors unless explicitly requested.
- Do not rename files, classes, functions, variables, or folders unless explicitly requested.
- Prefer adding focused helpers/components over changing broad gameplay classes.
- Keep implementation scope tied to dedicated server, replication, authority, Mass, debug, profiling, and verification goals.

## File And Class Layout

- Keep each class focused on one clear responsibility.
- Follow existing project layout before introducing a new structure.
- In headers, prefer this order:
  1. public API
  2. overrides
  3. protected methods
  4. private methods
  5. properties and cached state
- In `.cpp` files, prefer this order:
  1. includes
  2. log category and local helpers
  3. constructor
  4. lifecycle methods
  5. public methods
  6. private helpers
- Add a helper only when it reduces real complexity, meaningful duplication, or clarifies an ownership boundary.

## Naming

- Follow Unreal C++ type prefixes: `A`, `U`, `F`, `E`, and `I`.
- Use `b` prefix for booleans, for example `bHasArrived`.
- Keep project-specific gameplay names under the `MDS` prefix when adding new public types.
- CVar names should use the `mds.System.Feature` style.
- Command-line flags should use `-MDSFeature` for enable flags and `-NoMDSFeature` for disable flags.
- Do not change existing names unless the task explicitly approves a rename.

## Includes

- Prefer forward declarations in headers when possible.
- Put concrete includes in `.cpp` files when the header only needs a pointer or reference declaration.
- Keep `*.generated.h` as the last include in a reflected header.
- Avoid speculative includes.
- Do not add module dependencies just to hide avoidable include coupling.

## Function Design

- Put authority checks near the start of functions that mutate gameplay state.
- Use early returns to reject invalid state clearly.
- Keep hot path functions bounded and easy to reason about.
- Avoid allocation, expensive world searches, and log spam in Tick, Mass processors, replication callbacks, and frequent debug updates.
- Split helper functions when a function mixes unrelated responsibilities.
- Do not split code solely to create abstraction; the helper should make the code easier to verify.
- Prefer explicit state transitions over hidden side effects.

## Unreal Reflection

- Use `UPROPERTY` when lifetime tracking, GC visibility, serialization, replication, editor exposure, or Blueprint exposure is needed.
- Use `UFUNCTION` when reflection, RPC, delegates, Blueprint access, console/editor tooling, or serialization hooks are needed.
- Add Blueprint exposure only when the task requires it.
- Keep reflected data minimal and intentional.
- Do not expose client-mutable access to server-owned gameplay state.

## UObject Lifetime And GC

- Do not assume raw UObject pointers imply ownership.
- If a UObject or Actor reference must be tracked by GC, use `UPROPERTY` or the appropriate Unreal pointer type.
- Choose `TObjectPtr`, `TWeakObjectPtr`, raw pointer, or reference intentionally based on ownership and lifetime.
- Use `TWeakObjectPtr` for cached Actor/UObject references that may disappear during world teardown, level changes, or destruction.
- Give `NewObject` calls an appropriate Outer.
- Be able to explain the lifetime of any runtime-created UObject.
- Separate constructor-time setup from world-dependent runtime logic.
- Do not use world, subsystem, player, or network state in constructors.
- Do not assume `BeginPlay` order; check required actors, components, subsystems, and world state before use.
- Actor/component creation should be clearly tied to constructor setup, `BeginPlay`, or runtime spawn.
- Use `IsValid()` or null checks at boundaries where destruction or missing objects are possible.
- Clear timers in `Deinitialize`, `EndPlay`, or the appropriate owner teardown path.
- Unbind delegates when the binding can outlive the receiver or when the owner lifecycle is unclear.
- Re-check UObject validity in async, deferred, timer, or next-tick callbacks.
- Avoid `AddToRoot`; do not use it without explicit task approval and a documented reason.

## Authority And Replication Checklist

When adding or changing replicated gameplay state, verify:

- The server source of truth is clear.
- Client mutation paths are blocked or converted into validated server requests.
- The replicated property has the correct `UPROPERTY` metadata.
- `GetLifetimeReplicatedProps` is updated when needed.
- `OnRep` is used only for client-side presentation, local cache updates, or visual reactions.
- Server-side gameplay state changes happen before client observation.
- Ownership and RPC direction are documented in the plan or approval report.
- Listen-server or dedicated-server verification is included when runtime behavior changes.

## RPC Rules

- Use Server RPCs only when a client request must be validated and applied by the server.
- Use Client RPCs only for messages targeted at the owning client.
- Use NetMulticast sparingly.
- Do not use RPCs to bypass replicated state ownership.
- Avoid high-frequency reliable RPCs.
- Prefer replicated state for durable gameplay results.

## Logging

- Prefer project-specific log categories.
- Log authority-sensitive events where they are useful for server/client diagnosis.
- Include enough context to identify actor, role, state, and result.
- Avoid per-frame log spam.
- Use `Warning` or `Error` only for actionable problems, invalid configuration, failed verification conditions, or rejected unsafe paths.

## CVar And Command-Line Flags

- Debug and profiling behavior should be disableable with a CVar or command-line flag.
- Prefer `-NoMDS...` for disable flags.
- Runtime behavior flags must have clear defaults.
- Document important profiling/debug flags in task reports or relevant docs.
- Validate incompatible profiling flags early and fail with a useful log message.

## Debug Draw And Debug UI

- Debug draw must not be required for gameplay correctness.
- Dedicated server logic must not depend on visual debug systems.
- Debug draw should be guardable for profiling.
- Repeated debug draw should have bounded lifetime, count, or frequency.
- Debug UI should report state; it should not become a gameplay authority path.
- Avoid large UI frameworks unless explicitly requested.

## Subsystem Usage

- Use `UWorldSubsystem` for world-scoped demo state, runtime harnesses, and debug/profiling support.
- Before using a subsystem, confirm whether it should run on client, listen server, dedicated server, or standalone worlds.
- Keep gameplay authority separate from debug/reporting responsibilities.
- When caching Actors or UObjects in a subsystem, consider world teardown and stale references.
- Clear timers and release external bindings during subsystem teardown.
- Do not use a subsystem as a global bucket for unrelated behavior.

## Performance-Sensitive Code

Treat these areas as performance-sensitive:

- Tick
- Mass processors
- spawning
- replication updates
- debug output
- profiling harnesses
- runtime logging

Rules:

- Avoid hot path allocation.
- Avoid repeated expensive world searches.
- Avoid unnecessary replication.
- Avoid high-frequency reliable RPCs.
- Guard debug/profiling overhead.
- Include measurement context when making performance claims.

## Verification Expectations

- C++ changes need at least a build/compile check, runtime check, log check, or an explicit reason verification could not run.
- Network changes need server/client observation, not only manual inspection.
- Replication changes need a client-visible result check.
- Mass changes need verification for the specific behavior touched: spawn, movement, arrival, damage, debug, or profiling.
- Profiling changes need runtime mode, entity/actor count, map/scenario, and metric context.
- Documentation-only changes may skip build/runtime verification, but the approval report must say why.

## Markdown And Encoding

- Write new Markdown as UTF-8.
- Prefer ASCII unless the document already intentionally uses another character set.
- Do not broadly rewrite existing corrupted text unless the task explicitly asks for cleanup.
- Keep public project docs separate from private notes under `.private/`.
