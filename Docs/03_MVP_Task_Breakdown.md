# MVP Task Breakdown

This document breaks the `MDSProject` MVP into small, reviewable tasks suitable for Codex-driven implementation.

Every non-trivial task must follow the AI harness workflow: inspect relevant files, summarize current structure, propose a plan, wait for approval, implement only approved changes, verify, and provide an approval report.

Mass spawn, movement, arrival detection, and objective damage must be separate tasks unless explicitly approved.

## MVP Phase Overview

| Phase | Name | Task Type | Priority | Purpose |
| --- | --- | --- | --- | --- |
| 0 | Harness / Documentation | Documentation | Critical | Establish workflow, scope, and project rules. |
| 1 | Mass Concept Study | Documentation | Critical | Define Mass usage before implementation. |
| 2 | Mass Module Setup | Setup | Critical | Prepare required Mass dependencies. |
| 3 | Mass Spawn Only | Implementation | Critical | Prove entities can be spawned. |
| 4 | Mass Movement Only | Implementation | Critical | Prove entities can move toward the Objective. |
| 5 | Arrival Detection Only | Implementation | Critical | Detect arrival separately from damage. |
| 6 | Objective Damage Integration | Implementation | Critical | Apply server-authoritative objective damage. |
| 7 | Debug UI / Logging | Debug | High | Expose key runtime state. |
| 8 | Profiling Comparison | Profiling | High | Capture Actor-based vs Mass-based comparison data. |
| 9 | Final README / Interview Summary | Documentation | High | Package the project for interview discussion. |

Priority is assigned for completion by `2026-07-31`.

## Task List by Phase

## Phase 0: Harness / Documentation

Task Type: Documentation

Objective:

- Establish project goals, requirements, architecture, AI workflow, verification rules, Unreal rules, and Mass rules.

Allowed Modification Scope:

- Documentation files approved by each task.
- No Source or Config changes unless a separate approved implementation task allows them.

Dependencies:

- None.

Acceptance Criteria:

- Project goal, requirements, architecture, AI harness, verification, Unreal, and Mass rules are documented.
- Documentation clearly states scope and non-scope.
- Required plan and approval report formats are available.

Verification Method:

- Manual inspection of created documentation.
- Confirm no implementation code was created.

Priority:

- Critical.

## Phase 1: Mass Concept Study

Task Type: Documentation

Objective:

- Define how Mass Entity will be used for scalable enemy simulation before adding Mass code.

Allowed Modification Scope:

- A Mass concept document or approved Mass planning document.
- No Source or Config changes.

Dependencies:

- Phase 0.

Acceptance Criteria:

- The document explains why Mass is used.
- It identifies expected Mass concepts: Entity, Fragment, Tag, Processor, Spawner, and Representation.
- It states what will remain server-authoritative.
- It confirms spawn, movement, arrival, and objective damage will be separate tasks.

Verification Method:

- Manual inspection against `Docs/Mass_Rules.md`.

Priority:

- Critical.

## Phase 2: Mass Module Setup

Task Type: Setup

Objective:

- Add only the required Mass-related build or module dependencies needed for the first approved Mass implementation step.

Allowed Modification Scope:

- Approved `.Build.cs` file.
- Approved config files only if the task explicitly requires them.
- No gameplay implementation beyond dependency/setup work.

Dependencies:

- Phase 1.

Acceptance Criteria:

- Required module dependencies are added.
- Each added module has a documented reason.
- Project still builds or compile status is reported.
- No spawn, movement, arrival, or objective damage logic is added.

Verification Method:

- Build or compile check when available.
- Manual inspection of module dependency changes.
- Report any verification that could not be executed.

Priority:

- Critical.

## Phase 3: Mass Spawn Only

Task Type: Implementation

Objective:

- Implement the smallest approved Mass spawn path for enemy entities.

Allowed Modification Scope:

- Approved MassAI source files for spawn-only behavior.
- Approved build files only if missing dependencies are discovered and approved.
- Debug logs only if approved for spawn verification.

Dependencies:

- Phase 2.

Acceptance Criteria:

- Mass entities can be spawned in an approved test scenario.
- Entity count can be observed through logs, debug output, or Mass verification.
- No movement, arrival detection, or objective damage is implemented.

Verification Method:

- Build or compile check if code changes.
- PIE or runtime check if available.
- Mass Entity check with entity count and spawn behavior.
- Log review for relevant warnings or errors.

Priority:

- Critical.

## Phase 4: Mass Movement Only

Task Type: Implementation

Objective:

- Move previously spawned Mass entities toward the Objective without applying arrival or damage behavior.

Allowed Modification Scope:

- Approved MassAI movement-related source files.
- Approved debug logs only for movement verification.
- No objective damage changes.

Dependencies:

- Phase 3.

Acceptance Criteria:

- Spawned Mass entities move toward an approved Objective target or target location.
- Movement behavior is observable.
- Arrival detection and objective damage remain unimplemented in this task.

Verification Method:

- Build or compile check if code changes.
- PIE or runtime check if available.
- Mass Entity check with movement or processor behavior.
- Profiling note if movement introduces meaningful per-frame work.

Priority:

- Critical.

## Phase 5: Arrival Detection Only

Task Type: Implementation

Objective:

- Detect when Mass entities reach the Objective area without applying objective damage.

Allowed Modification Scope:

- Approved MassAI arrival-detection source files.
- Approved debug logs only for arrival verification.
- No objective HP or damage application changes.

Dependencies:

- Phase 4.

Acceptance Criteria:

- Arrival can be detected separately from movement.
- Arrival state or count can be observed through logs or debug output.
- Objective HP does not change in this task.

Verification Method:

- Build or compile check if code changes.
- PIE or runtime check if available.
- Mass Entity check with arrival behavior.
- Log review for arrival events and warnings.

Priority:

- Critical.

## Phase 6: Objective Damage Integration

Task Type: Implementation

Objective:

- Connect validated Mass arrival to server-authoritative Objective HP damage.

Allowed Modification Scope:

- Approved Objective source files.
- Approved MassAI integration source files.
- Approved network/replication code only as required by the approved plan.
- No unrelated combat, UI, or AI expansion.

Dependencies:

- Phase 5.

Acceptance Criteria:

- Objective HP is server-owned.
- Valid Mass arrival can trigger objective damage on the server.
- Clients can observe the resulting state through approved replication, logs, or debug UI.
- Invalid or client-only paths do not directly apply authoritative objective damage.

Verification Method:

- Build or compile check if code changes.
- PIE listen-server or dedicated-server verification when networked.
- Objective gameplay check.
- Network replication check with server/client notes.
- Log review for objective damage events.

Priority:

- Critical.

## Phase 7: Debug UI / Logging

Task Type: Debug

Objective:

- Expose key runtime state for authority, objective HP, Mass entity count, spawn state, movement state, arrival state, and damage events.

Allowed Modification Scope:

- Approved DebugUI source files or approved logging changes.
- Approved Objective or MassAI read-only reporting hooks if required.
- No large UI framework.

Dependencies:

- Phase 3 for spawn state.
- Phase 4 for movement state.
- Phase 5 for arrival state.
- Phase 6 for objective damage state.

Acceptance Criteria:

- Debug UI or logs expose the approved key state.
- Debug output does not become required for gameplay correctness.
- Server/client differences are clear when networked.

Verification Method:

- PIE or runtime check if UI/log behavior is runtime.
- Debug UI check or log review.
- Server/client notes if networked state is displayed.

Priority:

- High.

## Phase 8: Profiling Comparison

Task Type: Profiling

Objective:

- Capture profiling notes that support later Actor-based vs Mass-based comparison.

Allowed Modification Scope:

- Approved profiling documentation.
- Approved profiling helper source files only if a separate implementation plan allows them.
- No broad performance refactor.

Dependencies:

- Phase 3 at minimum.
- Phase 4 or later for movement-related profiling.
- Phase 6 if objective integration performance is being measured.

Acceptance Criteria:

- Profiling notes include runtime mode, scenario context, entity or actor count, FPS, frame time, and GameThread impact when relevant.
- The comparison is framed for technical interview discussion.
- Profiling does not expand the project into production optimization work.

Verification Method:

- Profiling check with recorded metrics when available.
- Manual inspection of profiling notes.
- State any metrics that could not be captured.

Priority:

- High.

## Phase 9: Final README / Interview Summary

Task Type: Documentation

Objective:

- Summarize the completed MVP, how it was verified, and how it should be discussed in interviews.

Allowed Modification Scope:

- Approved README or interview summary documentation.
- No Source or Config changes.

Dependencies:

- Phase 6 for core gameplay summary.
- Phase 7 for debug visibility summary.
- Phase 8 for profiling summary.

Acceptance Criteria:

- Summary explains the project goal, architecture, server authority, Mass workflow, verification results, and remaining limitations.
- It distinguishes completed work from planned or unverified work.
- It stays focused on Unreal Engine client/gameplay programming interviews.

Verification Method:

- Manual inspection of final documentation.
- Confirm claims match actual verification reports.

Priority:

- High.

## Dependency Summary

- Phase 0 is the base for all later work.
- Phase 1 must happen before Mass setup or implementation.
- Phase 2 must happen before Mass spawn.
- Phase 3 must happen before Mass movement.
- Phase 4 must happen before arrival detection.
- Phase 5 must happen before objective damage integration.
- Phase 6 must happen before final gameplay summary.
- Phase 7 depends on whichever runtime state it displays.
- Phase 8 depends on the behavior being profiled.
- Phase 9 depends on completed and verified prior phases.

## Task Type Rules

Documentation tasks:

- May create or update approved Docs or README files only.
- Must not create implementation code.

Setup tasks:

- May adjust approved build/module/config setup only when explicitly allowed.
- Must explain every `.Build.cs` or module dependency change.
- Must not add gameplay behavior unless separately approved.

Implementation tasks:

- Must be small and behavior-specific.
- Must preserve server-authoritative gameplay.
- Must include runtime verification when behavior changes.

Debug tasks:

- May expose runtime state through approved DebugUI or logs.
- Must not become a large UI framework.
- Must not be required for gameplay correctness.

Profiling tasks:

- Must record measurement context and relevant metrics.
- Should support Actor-based vs Mass-based comparison.
- Must not become broad optimization or refactor work unless explicitly approved.

## Mass Separation Rule

Mass spawn, movement, arrival detection, and objective damage are separate tasks.

Do not implement more than one of these in a single task unless the user explicitly approves that combined scope in the task request and plan.

