# Project Goal

## Project Name

`MDSProject`

## One-line Summary

MDSProject is a server-authoritative UE5 multiplayer defense sandbox built to demonstrate dedicated server gameplay, replication, Mass AI, debugging, profiling, and AI-assisted development workflow.

## Portfolio Purpose

This project is a technical portfolio for Unreal Engine client/gameplay programming interviews.

It is designed to show practical engineering judgment around multiplayer gameplay, authority boundaries, scalable AI simulation, verification, and focused documentation.

## Core Gameplay Concept

Players run in a multiplayer or dedicated-server environment while AI enemies move toward a shared objective.

The objective has server-authoritative HP. When enemies validly arrive and apply damage, the server updates objective HP and clients observe the result through approved replicated state or debug output.

## Technical Goals

- Dedicated Server
- Replication
- Authority / Ownership
- Objective gameplay
- Mass Entity / Mass AI
- Debug UI
- Profiling
- AI-assisted development workflow

## MVP Scope

- Player can run in a multiplayer/dedicated-server environment.
- Objective has server-authoritative HP.
- Mass-based enemies can be spawned.
- Mass enemies move toward the objective.
- Arrival can be detected.
- Objective HP decreases on valid arrival/damage.
- Debug UI or logs expose key state.
- Profiling document can compare Actor-based and Mass-based approaches later.

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
- Full game production content

## AI-Assisted Development Goal

AI assistance is used as a controlled development workflow, not as a replacement for engineering judgment.

The human owns project direction, task approval, architecture decisions, final review, and verification judgment. AI assists with file inspection, planning, scoped implementation, risk identification, verification checklists, and documentation.

Every non-trivial task should remain small, reviewable, approved before implementation, and reported after verification.

## Interview Value

The project should support clear interview discussion about:

- Designing server-authoritative gameplay in UE5
- Separating client requests from server-owned results
- Using replication and ownership deliberately
- Building objective gameplay with verifiable state changes
- Integrating Mass Entity incrementally
- Comparing Actor-based and Mass-based approaches through profiling
- Exposing runtime state through debug UI or logs
- Using AI assistance with explicit approval gates and verification discipline

## Target Completion Date

`2026-07-31`

## Success Criteria

The project is successful when it can demonstrate:

- A multiplayer or dedicated-server run path.
- Server-owned objective HP that changes only through validated gameplay.
- Mass-based enemy spawn, movement, arrival detection, and objective damage flow.
- Debug UI or logs that make authority, objective state, and Mass behavior inspectable.
- Verification notes for build/compile, runtime behavior, network behavior, and logs where applicable.
- Profiling notes that can compare Actor-based and Mass-based approaches.
- Documentation that explains project scope, implementation rules, Mass rules, verification standards, and AI-assisted workflow.
- A concise interview narrative focused on Unreal Engine client/gameplay programming rather than full game production.

