# Task Template

Use this template when requesting Codex work for `MDSProject`.

Copy the sections below into a new task request and fill in the placeholders.

## Task Name

`<Short task name>`

## Objective

Describe the exact outcome wanted from this task.

Keep the objective focused on one reviewable change.

Example:

`Create a documentation template for future Codex tasks.`

## Context Files to Read First

List the files Codex must inspect before proposing a plan.

Required defaults:

- `AGENTS.md`

Add task-specific files when relevant:

- `<Relevant source file>`
- `<Relevant config file>`
- `<Relevant documentation file>`

For Mass Entity work, include:

- `Docs/Mass_Rules.md`

For verification-heavy work, include:

- `Docs/Verification.md`

## Allowed Files to Modify

List every file Codex is allowed to create or edit.

Codex must not modify files outside this list.

- `<Allowed file path>`
- `<Allowed file path>`

## Forbidden Changes

State changes that are explicitly out of scope.

Recommended defaults:

- Do not modify `AGENTS.md` unless explicitly requested.
- Do not modify unrelated Source files.
- Do not modify unrelated Config files.
- Do not perform broad refactoring.
- Do not rename existing files, classes, functions, variables, or folders unless explicitly requested.
- Do not create implementation code unless this task specifically requests implementation.
- Do not add full-game systems such as inventory, quests, save systems, matchmaking, lobby, crafting, skill trees, large UI frameworks, complex animation systems, or full GAS expansion unless explicitly requested.

## Implementation Requirements

Describe what the implementation must include.

Use concrete bullets:

- `<Requirement 1>`
- `<Requirement 2>`
- `<Requirement 3>`

For Unreal C++ work, include any requirements for:

- Replication
- Server authority
- RPC ownership
- Lifetime replicated properties
- BeginPlay order
- Tick cost
- UObject lifetime
- Build.cs module dependencies

When changing `.Build.cs`, require an explanation for each added module.

## Acceptance Criteria

Define what must be true for the task to be accepted.

- `<Expected result 1>`
- `<Expected result 2>`
- `<Expected result 3>`

Acceptance criteria should be observable through review, build, test, PIE, dedicated server testing, logs, or manual inspection.

## Verification Steps

List the verification Codex should run or describe.

- `<Build, compile, test, PIE, dedicated server, log, or manual inspection step>`
- `<Expected verification result>`

Codex must not claim a test passed unless it was actually run.

If verification cannot be run, Codex must clearly state why and list the remaining manual checks.

For networked gameplay changes, include a listen server or dedicated server verification plan.

## Required Plan Format Reminder

Before modifying files, Codex must respond with this plan format and wait for explicit approval:

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

## Required Approval Report Reminder

After implementation, Codex must respond with this approval report format:

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

