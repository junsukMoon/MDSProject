# MDSProject

MDSProject is a server-authoritative UE5 multiplayer defense sandbox built to demonstrate dedicated server gameplay, replication, Mass AI, debugging, profiling, and AI-assisted development workflow.

## Purpose

This project is a technical portfolio for Unreal Engine client/gameplay programming interviews.

The goal is to demonstrate practical engineering judgment around multiplayer gameplay, server authority, ownership boundaries, scalable enemy simulation, runtime debugging, profiling, and controlled AI-assisted development.

This is not a full game production project.

## Core Concept

Players run in a multiplayer or dedicated-server environment while AI enemies move toward a shared objective.

The Objective owns server-authoritative HP. When enemies validly arrive and apply damage, the server updates Objective HP and clients observe the result through approved replicated state, debug UI, or logs.

## Technical Focus

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
- Mass enemies move toward the Objective.
- Arrival can be detected.
- Objective HP decreases on valid server-authoritative arrival/damage logic.
- Debug UI or logs expose key runtime state.
- Profiling notes can later compare Actor-based and Mass-based approaches.

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
- Full production-quality game content

## Documentation Map

- `Docs/00_Project_Goal.md` - project goal, scope, target date, and interview value
- `Docs/01_Requirements.md` - MVP requirements and verification mapping
- `Docs/02_Architecture.md` - high-level architecture and system responsibilities
- `Docs/03_MVP_Task_Breakdown.md` - phased MVP task breakdown
- `Docs/AI_Harness.md` - AI-assisted development workflow
- `Docs/Task_Template.md` - reusable task request template
- `Docs/Approval_Report_Template.md` - reusable completion report template
- `Docs/Verification.md` - verification standards
- `Docs/Unreal_Rules.md` - Unreal C++ and multiplayer implementation rules
- `Docs/Mass_Rules.md` - Mass Entity / Mass AI working rules

## AI-Assisted Workflow

Non-trivial work follows an approval-based workflow:

1. Inspect relevant files.
2. Summarize current structure.
3. Propose a plan.
4. Wait for explicit approval.
5. Implement only approved changes.
6. Verify the result.
7. Provide an approval report.

AI assistance is used as a controlled accelerator. Human review owns goals, scope, architecture decisions, and final verification judgment.

## Verification

Tests or checks must not be reported as passed unless they were actually run.

Network-related changes require server/client verification notes. Mass-related changes should report entity count, spawn behavior, movement or processor behavior, arrival behavior, and performance impact when applicable.

Manual inspection is useful, but it is not the same as runtime verification.

## Target Completion Date

`2026-07-31`

