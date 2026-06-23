# Mass Rules

This document defines Mass Entity / Mass AI working rules for `MDSProject`.

Mass work must be incremental, measurable, and focused on technical portfolio value.

## Purpose of Mass Entity Usage in This Project

Mass Entity is used to demonstrate scalable AI-style simulation for a multiplayer defense sandbox.

The goal is not to build a full enemy system in one pass. The goal is to show that the project can use Mass Entity for controlled spawning, movement, arrival detection, objective interaction, debugging, and profiling.

Mass work should support interview discussion around:

- Why Mass was chosen
- What problem it solves compared with actor-only AI
- How server authority is preserved
- How performance impact is measured
- How Mass behavior integrates with objective gameplay

## Scope of Mass Work

Mass work may include:

- Concept documentation
- Module and build setup
- Entity fragments and tags
- Entity spawning
- Processor-driven movement
- Arrival detection
- Server-authoritative objective damage integration
- Debug UI integration
- Profiling and comparison notes

Each task must focus on one approved slice.

## Forbidden Mass Scope

Do not add full AI gameplay systems unless explicitly requested.

Forbidden unless explicitly approved:

- Combining spawn, movement, arrival detection, and objective damage in one task
- Full combat AI
- Behavior tree replacement work
- Complex animation integration
- Crowd avoidance beyond the approved task
- Dynamic formations
- Large debug UI frameworks
- Broad refactors of existing gameplay systems
- Client-authoritative Mass gameplay state

Mass spawn, movement, arrival detection, and objective damage must not be implemented in a single task unless explicitly approved.

## Required Incremental Task Order

Mass work must follow this order unless the approved task explicitly says otherwise:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI integration
8. Profiling comparison

Do not skip ahead from setup directly to objective damage. Do not combine multiple behavior steps just because they are nearby in code.

## Required Explanation Before Each Mass Implementation

Before implementing Mass work, explain:

- Which incremental step is being implemented
- Why Mass is appropriate for that step
- Which files are expected to change
- Which Mass concepts are involved
- What gameplay state is server-owned
- What will not be implemented in this task
- How the result will be verified

If the task affects networking, include server/client verification notes in the plan.

## Required Build.cs / Module Dependency Explanation

Any `.Build.cs` or module dependency change must include a reason for each module.

For each added, removed, or changed module, report:

- Module name
- Why the module is required
- Whether it is runtime or editor-only
- Which approved task needs it

Do not add broad Mass dependency sets speculatively.

## Fragment / Tag / Processor / Spawner / Representation Rules

Mass types must have clear responsibilities.

Fragments:

- Store the smallest useful unit of Mass data.
- Avoid duplicating state that should remain owned by gameplay actors or the server.
- Keep data layout simple enough to profile and explain.

Tags:

- Use tags for simple state classification.
- Do not use tags as a substitute for data that must be measured or replicated.

Processors:

- Keep processors focused on one behavior.
- Avoid combining spawn, movement, arrival detection, and damage in one processor.
- Keep processor work bounded and performance-aware.
- Avoid per-entity expensive world searches when a shared query or cached reference is appropriate.

Spawners:

- Spawn only the approved entity type or scenario for the task.
- Report entity count and spawn behavior during verification.
- Do not add movement or damage behavior during a spawn-only task.

Representation:

- Keep visual representation separate from authoritative gameplay state.
- Do not require visual representation for dedicated server logic.
- Debug representation must not become required for gameplay correctness.

## Server Authority and Networking Considerations

Server-authoritative gameplay remains the default for Mass work.

Rules:

- Mass simulation that affects gameplay results should be owned or validated by the server.
- Clients may observe Mass-related results but must not be the source of objective damage, score, or win/loss state.
- Replicated state must have a clear server-side source of truth.
- If clients need visual feedback, separate presentation from authoritative state.
- Network changes require server/client verification notes.

Do not assume Mass behavior is verified for multiplayer unless listen-server or dedicated-server behavior was actually tested.

## Objective Integration Rules

Objective integration must happen only after spawn, movement, and arrival detection are already established or explicitly approved for the same task.

Rules:

- Objective HP and objective damage are server-owned.
- Mass arrival may request objective damage, but the server applies the result.
- Objective damage must be measurable and logged or visible through approved debug output.
- Do not apply objective damage from client-only Mass presentation.
- Do not combine first-time arrival detection and objective damage unless explicitly approved.

## Debug and Profiling Requirements

Mass debug and profiling work should make behavior explainable.

Debug reporting should include when relevant:

- Entity count
- Spawn state
- Movement state
- Arrival state
- Objective interaction state
- Server/client visibility

Profiling should include when relevant:

- FPS
- Frame time
- GameThread impact
- Entity count
- Scenario or map context
- Baseline compared against

Avoid debug UI or logging that creates meaningful runtime overhead during normal play.

## Verification Checklist for Each Mass Task

For every Mass task, report which checks were actually run.

Minimum checklist:

- Files manually inspected
- Build or compile result, if code or modules changed
- Editor startup result, if modules, plugins, assets, or config are affected
- PIE single-player result, if runtime behavior was changed
- PIE listen-server or dedicated-server result, if gameplay state is networked
- Entity count, when entities are spawned
- Spawn behavior, when spawning is changed
- Movement behavior, when movement is changed
- Arrival behavior, when arrival detection is changed
- Objective damage behavior, when objective integration is changed
- Performance impact, when entity count, processors, Tick, debug UI, or spawning changes
- Relevant logs, warnings, or errors

Do not report spawn, movement, arrival detection, or objective damage as verified unless each behavior was actually tested.

If verification cannot be executed, state the reason and list what remains unverified.

## Interview Explanation Points

Mass work should be explainable in an interview.

Be prepared to explain:

- Why the work was split into small steps
- Why Mass Entity was used instead of actor-only AI
- How fragments, tags, processors, spawners, and representation were separated
- What remains server-authoritative
- How clients observe results
- How objective damage is protected from client authority
- How performance was measured
- What tradeoffs were made for portfolio scope

The expected message is that Mass was integrated deliberately and verified incrementally, not that a large AI system was generated all at once.

