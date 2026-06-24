# Mass Concept Study

## Purpose

This document defines how Mass Entity will be used in `MDSProject` before any Mass implementation code is added.

The goal is to keep Mass work incremental, server-authoritative, and useful for interview discussion. Mass should demonstrate large-scale AI behavior without turning the project into a full game system.

## Why Mass Is Used

Mass Entity is used to represent and update many lightweight enemy agents more efficiently than traditional Actor-heavy gameplay.

For this project, Mass is intended to demonstrate:

- Data-oriented AI representation
- Batch-style movement and processing
- Scalable enemy simulation
- Clear separation between simulation data and gameplay authority
- Profiling and debugging opportunities for technical discussion

Mass is not being used to replace all gameplay Actors. Objective gameplay, scoring, replicated state, and authoritative damage rules remain owned by server-side gameplay systems.

## MVP Mass Scope

The MVP Mass scope is limited to enemy-style agents moving toward an objective.

In scope:

- Define Mass agent concepts
- Spawn Mass entities in a controlled way
- Move entities toward a target
- Detect arrival at an objective
- Apply objective damage through server-authoritative gameplay code
- Add debug and profiling visibility after core behavior works

Out of scope:

- Inventory
- Quest systems
- Matchmaking or lobby systems
- Complex animation systems
- Full GAS expansion
- Advanced perception or behavior trees for Mass agents
- Client-authoritative damage or scoring

## Server Authority Rule

The server owns gameplay state.

Mass may simulate enemy movement and arrival data, but gameplay results must be applied by server-authoritative systems. Clients may observe replicated outcomes, but they must not decide objective HP, score, damage, wave completion, or win/loss state.

Server-owned gameplay state includes:

- Objective HP
- Enemy damage application
- Score or kill credit
- Wave state
- Match state

Mass-owned or Mass-related simulation data may include:

- Entity position or target direction
- Movement speed
- Agent tags
- Arrival candidate state
- Lightweight debug state

## Expected Mass Concepts

### Entity

A Mass Entity represents a lightweight enemy agent. It should be treated as simulation data, not as a full replicated gameplay Actor.

### Fragment

Fragments store the data needed by Mass processors. Expected fragments may include movement data, target data, objective approach data, and lightweight state used for arrival checks.

Fragments should remain focused. Gameplay state such as objective HP does not belong in Mass fragments.

### Tag

Tags classify entities for processor selection or state transitions. Example uses include spawned, moving, arrived, or pending-damage states.

Tags should not become a replacement for authoritative gameplay rules.

### Processor

Processors update Mass entities in focused passes. Each processor should do one clear job, such as spawn setup, movement, arrival detection, or debug collection.

Processors must not combine spawn, movement, arrival detection, and objective damage in one task.

### Spawner

The spawner creates Mass entities in a controlled server-side flow. The first Mass implementation task should prove spawn only, without movement or objective damage.

### Representation

Representation is used only when visual feedback is needed. It should not be the source of gameplay truth.

Visual representation can be added after entity creation and movement are understood.

## Responsibility Split

Mass responsibilities:

- Represent many lightweight agents
- Store agent simulation data
- Run batch movement logic
- Mark arrival candidates
- Provide data for debug and profiling

Gameplay responsibilities:

- Validate gameplay effects
- Apply objective damage
- Own replicated objective state
- Own scoring and match state
- Expose player-visible authoritative results

## Incremental Task Order

Mass work must proceed in this order:

1. Concept document
2. Build/module setup
3. Spawn only
4. Movement only
5. Arrival detection only
6. Objective damage integration
7. Debug UI integration
8. Profiling comparison

Each step should be its own task branch and PR.

## Branch and PR Expectations

Each Mass task branch must map to one task from `Docs/03_MVP_Task_Breakdown.md`.

Each PR must include:

- Approval Report
- Verification result
- Scope check
- Learning Review

Merging to `main` requires explicit user approval.

## Verification Approach

This concept document is verified by manual inspection against:

- `Docs/Mass_Rules.md`
- `Docs/03_MVP_Task_Breakdown.md`
- `Docs/04_Git_Workflow.md`

No Build, PIE, or Dedicated Server test is required for this document-only task.

Future implementation tasks must include the relevant Unreal verification, such as build, PIE, dedicated server, log inspection, or manual in-editor checks.

## Risks and Notes

Mass implementation can easily expand in scope. If a task needs to add behavior outside the approved step, Codex must stop and ask for user approval before continuing.

Objective damage must not be added during spawn, movement, or arrival-only tasks.

Mass should be used to demonstrate scalable simulation, not to hide gameplay authority inside data processors.

## Learning Review Questions

- Why is Mass useful for this project?
- What gameplay state must remain server-authoritative?
- What is the difference between a Mass Entity and an Actor?
- What type of data belongs in a Fragment?
- When should a Tag be used?
- Why should spawn, movement, arrival, and objective damage be separate tasks?
- What should clients observe, and what should the server decide?
- Why should visual representation not become gameplay authority?
