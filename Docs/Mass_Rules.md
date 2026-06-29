# Mass Rules

This document defines Mass Entity / Mass AI working rules for `MDSProject`.

Mass work must be incremental, measurable, and focused on technical portfolio value.

## Purpose

Mass Entity is used in this multiplayer defense sandbox to demonstrate scalable AI-style simulation.

The goal is not to build a complete enemy system all at once. The goal is to make controlled spawning, movement, arrival detection, objective interaction, debugging, and profiling explainable and verifiable.

## Allowed Scope

Mass work may include:

- concept documentation
- module/build setup
- fragments and tags
- entity spawning
- processor-driven movement
- arrival detection
- server-authoritative objective damage integration
- debug UI/log integration
- profiling and Actor-vs-Mass comparison notes

Each task must focus on the approved slice only.

## Forbidden Scope

Do not add the following without explicit approval:

- spawn, movement, arrival detection, and damage in one combined task
- full combat AI
- behavior tree replacement
- complex animation integration
- crowd avoidance expansion
- dynamic formations
- large debug UI framework
- broad gameplay refactor
- client-authoritative Mass gameplay state
- production enemy content beyond the technical demo

## Incremental Task Order

Preferred Mass task order:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI/log integration
8. Profiling comparison

Do not combine steps unless the user explicitly approves that scope.

## Mass Concepts

### Fragments

- Store the smallest useful units of Mass data.
- Keep data layout simple and explainable.
- Do not duplicate server-owned gameplay state such as Objective HP in Mass fragments.
- Use fragments to represent entity-local state such as spawn, movement, and arrival flags.

### Tags

- Use tags for simple state classification.
- Do not use tags as a replacement for data that must be measured, debugged, or replicated.
- Keep tags narrow and readable.

### Processors

- Keep each processor focused on one behavior.
- Do not combine spawn, movement, arrival, and damage into one processor unless explicitly approved.
- Processor work must be bounded and performance-aware.
- Avoid expensive world searches in frequent updates.
- Guard debug draw and profiling-sensitive work.

### Spawners

- Spawn only the approved scenario/entity type.
- Report entity count and spawn behavior.
- Do not add movement, arrival, or damage in a spawn-only task.
- Use command-line/CVar controls when count or enablement needs to vary for profiling.

### Representation

- Representation is visual feedback, not gameplay authority.
- Dedicated server logic must not depend on Mass visual representation.
- Client presentation must not become the source of objective damage, score, or win/loss state.

## Server Authority

- Any Mass simulation that affects gameplay results must be owned or validated by the server.
- Clients may observe Mass-related results, but they must not be the source of objective damage, score, or win/loss.
- Replicated state must have a clear server-side source of truth.
- Client-only Mass presentation must not apply gameplay damage.

## Objective Integration

- Objective HP and objective damage are server-owned.
- Mass arrival can be the reason for an objective damage request, but the server applies the result.
- Objective damage must be measurable through logs, debug state, and verification evidence.
- Damage should be applied once per valid arrival unless a task explicitly defines different behavior.
- Arrival state and damage-applied state should be represented separately when repeated processing is possible.

## Debug And Profiling

Debug reporting should include relevant state such as:

- entity count
- spawn state
- movement state
- arrival state
- objective interaction state
- server/client visibility

Profiling notes should include:

- FPS or frame time when available
- GameThread impact when available
- entity/actor count
- map and scenario context
- runtime mode
- baseline comparison conditions

Debug UI, debug draw, and logs must not create misleading runtime overhead. Profiling runs should be able to disable debug draw.

## Verification Checklist

For each Mass task, report only checks that were actually run.

Use relevant items from this checklist:

- files manually inspected
- build or compile result
- editor startup result
- PIE result
- listen-server or dedicated-server result
- entity count
- spawn behavior
- movement behavior
- arrival behavior
- objective damage behavior
- performance impact
- relevant logs, warnings, and errors

Do not claim spawn, movement, arrival, or damage is verified unless that behavior was actually tested.

## Interview Explanation Points

Be ready to explain:

- why Mass was used
- why Mass work was split into small phases
- how fragments, tags, processors, spawners, and representation are separated
- what state is server-authoritative
- what clients observe and what clients do not decide
- how objective damage is applied
- what profiling scenario was measured
- what the profiling numbers do and do not prove
