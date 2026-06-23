# AGENTS.md

## Project

This is a UE5 technical portfolio project.

Project name: `MDSProject`

Primary goal:

Build a server-authoritative multiplayer defense sandbox that demonstrates:

* Dedicated Server
* Replication
* Authority / Ownership
* Objective gameplay
* Mass Entity / Mass AI
* Debug UI
* Profiling
* AI-assisted development workflow

This is not a full game. Keep the scope focused on technical demonstration and interview discussion.

---

## Core Workflow

For every non-trivial task, follow this sequence:

1. Inspect relevant existing files.
2. Summarize the current structure.
3. Propose a plan.
4. Wait for explicit approval.
5. Implement only the approved changes.
6. Verify the result.
7. Provide an approval report.

Do not modify files before the plan is approved.

If the task is ambiguous, state assumptions before implementation.

---

## Change Policy

Prefer small, reviewable changes.

Do not perform broad refactoring unless explicitly requested.

Do not rename existing files, classes, functions, variables, or folders unless explicitly requested.

Do not modify unrelated systems.

Modify only the files allowed by the task.

Prefer adding new components/classes over changing existing gameplay classes when possible.

---

## Unreal Engine Rules

Follow Unreal Engine C++ conventions.

Be careful with:

* Replication
* Server authority
* RPC ownership
* Lifetime replicated properties
* BeginPlay order
* Tick cost
* UObject lifetime
* Garbage collection
* Build.cs module dependencies

When changing `.Build.cs`, explain why each added module is required.

---

## Network Rules

Server-authoritative gameplay is the default.

Gameplay state changes such as health, damage, score, and objective HP must be owned by the server.

Clients may request actions, but the server validates and applies gameplay results.

When adding replicated data, include a verification plan for listen server or dedicated server.

---

## Mass Entity Rules

Mass-related work must be incremental.

Do not combine Mass spawn, movement, arrival detection, and objective damage in one task unless explicitly requested.

Preferred task order:

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection
5. Objective damage integration
6. Debug/profiling

For Mass-specific details, read:

`Docs/Mass_Rules.md`

---

## Required Plan Format

Before implementation, respond with:

```text
Plan

Objective:
Files Read:
Current Structure Observed:
Proposed Changes:
Files Expected to Change:
Risks:
Verification Plan:
Approval Needed:
```

---

## Required Approval Report Format

After implementation, respond with:

```text
Approval Report

Objective:
Plan Executed:
Changed Files:
Implementation Summary:
Verification:
Manual Test Steps:
Risks / Notes:
Next Suggested Task:
```

---

## Verification Rules

Do not claim a test passed unless it was actually run.

For code changes, report at least one of:

* Build result
* Compile result
* PIE test result
* Dedicated server test result
* Log output check
* Manual inspection result

If verification cannot be executed, clearly state why.

For detailed verification rules, read:

`Docs/Verification.md`

---

## Forbidden Scope Unless Explicitly Requested

Do not add:

* Inventory
* Quest system
* Save system
* Matchmaking
* Lobby
* Crafting
* Skill tree
* Large UI framework
* Complex animation system
* Full GAS expansion

Do not expand this project into a full game.

---

## Additional Project Documents

When relevant, read:

* `Docs/AI_Harness.md`
* `Docs/Task_Template.md`
* `Docs/Approval_Report_Template.md`
* `Docs/Verification.md`
* `Docs/Unreal_Rules.md`
* `Docs/Mass_Rules.md`
* `Docs/Requirements.md`

