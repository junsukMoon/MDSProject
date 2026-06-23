# Git Workflow

## Purpose

This document defines the Git-based AI task workflow for `MDSProject`.

The workflow keeps AI-assisted work isolated, reviewable, and aligned with the approved MVP task breakdown. Codex works on a separate task branch or git worktree, verifies the approved change, prepares PR-ready results, and waits for user approval before anything is merged to `main`.

Git workflow is part of the portfolio process. It should demonstrate disciplined task selection, approval gates, verification, and learning review rather than fast unreviewed changes.

## Main Branch Rule

`main` is the protected integration branch.

Rules:

- Codex must never commit directly to `main`.
- Codex must never push directly to `main`.
- `main` should contain only reviewed, PR-merged work.
- `main` should remain buildable or have known documented verification limitations.
- Each change merged to `main` must map back to an approved task and approval report.

If Codex finds itself on `main` before making a change, it must create or switch to an approved task branch before editing files.

## Branch Strategy

Each AI task must use a separate branch.

Rules:

- One task branch maps to one task from `Docs/03_MVP_Task_Breakdown.md`.
- Do not combine unrelated MVP phases in one branch.
- Do not combine Mass spawn, movement, arrival detection, and objective damage in one branch unless explicitly approved.
- Keep each branch small enough to review in one pass.
- Delete task branches after merge unless the user asks to keep them.

Task branches should start from the latest `origin/main`.

Recommended setup:

```text
git fetch origin
git switch main
git pull --ff-only origin main
git switch -c <task-branch>
```

## Worktree Strategy

Git worktrees may be used when multiple tasks need to remain available at the same time.

Use a worktree when:

- A task is blocked but should be preserved.
- A second independent task must start without disturbing the blocked branch.
- The user wants separate folders for Orchestrator, Execute, or review work.

Rules:

- Each worktree must still use one task branch.
- Do not edit the same task from multiple worktrees at the same time.
- Keep worktree folder names clear and task-specific.
- Remove completed worktrees after merge.

Example:

```text
git worktree add ../MDSProject-task-mass-study task/mass-concept-study
```

## Task Branch Naming Convention

Use short, descriptive branch names that identify the task type and MVP phase when possible.

Format:

```text
<type>/<phase-or-area>-<short-task-name>
```

Recommended types:

- `docs/`
- `setup/`
- `feature/`
- `debug/`
- `profile/`
- `smoke/`
- `fix/`

Examples:

```text
docs/phase-1-mass-concept-study
setup/phase-2-mass-modules
feature/phase-3-mass-spawn-only
debug/phase-7-objective-logs
smoke/readme-workflow-test
```

## Task Selection Workflow

Task selection starts from `Docs/03_MVP_Task_Breakdown.md`.

Workflow:

1. Select the next approved MVP task.
2. Confirm dependencies are complete.
3. Confirm the task is small and reviewable.
4. Identify required context documents.
5. Identify allowed files to modify.
6. Identify forbidden scope.
7. Ask Codex to inspect files and produce a plan.

Mass tasks must follow the incremental order in `Docs/Mass_Rules.md` unless the user explicitly approves a different sequence.

## Codex Plan / Approval / Implementation Workflow

For every non-trivial task, Codex must follow the required approval workflow:

1. Inspect relevant existing files.
2. Summarize current structure.
3. Provide a plan using the required `AGENTS.md` plan format.
4. Wait for explicit user approval.
5. Create or switch to the task branch.
6. Implement only the approved changes.
7. Verify the result.
8. Provide an Approval Report.
9. Prepare PR-ready output.

Codex must not modify files before approval.

Codex must modify only the files allowed by the approved task. If the approved task allows only one file, only that file may change.

## Verification Before PR

Before opening or preparing a PR, Codex must verify the task according to `Docs/Verification.md`.

Minimum verification for documentation-only changes:

- Manual inspection of the changed document.
- Confirm changed files match the approved scope.
- Confirm no source or config files changed.

Minimum verification for code or config changes:

- Build or compile check when available.
- Relevant Unreal runtime check when behavior changed.
- Listen-server or dedicated-server notes for networked gameplay changes.
- Log review when logs are relevant.
- Clear statement of anything not run.

Codex must not claim that a test, build, compile, PIE run, dedicated server run, or log check passed unless it was actually run.

## PR Title and Description Format

Each PR should be small, task-specific, and reviewable.

Commit messages, PR titles, and PR descriptions should be written in Korean for this project workflow.

Recommended commit message format:

```text
[Type] Korean summary
```

Example:

```text
[문서] Git workflow와 PR 템플릿 추가
```

Recommended PR title format:

```text
<Type>: <short task summary>
```

Examples:

```text
Docs: Add Git workflow
Setup: Add Mass module dependencies
Feature: Add Mass spawn-only path
```

Required PR description sections:

```text
Objective:
Linked MVP Task:
Changed Files:
Approval Report:
Verification:
Learning Review:
Risks / Notes:
```

The PR description must include the Approval Report or a link to the exact Approval Report.

The PR description must include a Learning Review before merge.

## Merge Approval Rule

Merging is user-owned.

Rules:

- Codex may prepare a PR-ready branch and PR.
- Codex may summarize merge readiness.
- The user must approve before merge.
- Codex must not merge without explicit user approval.
- If branch protection blocks merge, Codex must report the blocker and avoid bypassing protection.

After merge:

1. Sync local `main`.
2. Confirm the merge commit or squash commit exists on `origin/main`.
3. Confirm the working tree is clean.
4. Delete the task branch if appropriate.
5. Update the next suggested task.

## Failure Handling

When a task fails, Codex must stop broadening the scope and report the exact failure.

Failure report should include:

- What command or verification step failed.
- Whether the failure appears current-task, pre-existing, environment-related, or unknown.
- The smallest next diagnostic step.
- What remains unverified.

Codex must not hide verification gaps.

Codex must not rewrite unrelated systems to work around a failure.

## Scope Expansion Rule

If a task requires scope expansion, Codex must stop and ask for approval.

Scope expansion includes:

- Modifying files outside the allowed list.
- Adding new Source or Config files during a documentation task.
- Adding modules or plugin dependencies not in the approved plan.
- Combining multiple MVP phases in one task.
- Adding gameplay systems outside the approved task.
- Renaming existing files, classes, functions, variables, or folders.
- Changing replication, RPC, authority, or Objective HP behavior outside the approved plan.

The user must explicitly approve the expanded scope before Codex continues.

## Learning Review Before Merge

Every PR must include a Learning Review before merge.

Purpose:

- Capture what was learned from the task.
- Support interview discussion.
- Preserve verification and tradeoff context.
- Improve the next task plan.

Recommended Learning Review format:

```text
What changed:
What was verified:
What was not verified:
What risks remain:
What this demonstrates for the portfolio:
What should be improved next:
```

Learning Review should be concise and factual. It should not claim runtime behavior that was not actually tested.

## Daily Workflow

Recommended daily workflow:

1. Start on clean `main`.
2. Pull latest `origin/main`.
3. Select one task from `Docs/03_MVP_Task_Breakdown.md`.
4. Ask Codex to inspect required files and propose a plan.
5. Approve or revise the plan.
6. Create a task branch or worktree.
7. Implement the approved scope.
8. Run verification.
9. Produce an Approval Report.
10. Create or prepare the PR.
11. Add the Learning Review.
12. Review and approve merge.
13. Merge to `main`.
14. Sync local `main`.
15. Record progress in the progress log when that document exists.

Do not start a second implementation task while the current task has unreported verification results.

## Orchestrator / Execute / Merge Command Roles

The workflow separates three command roles.

### Orchestrator

The Orchestrator role selects and scopes work.

Responsibilities:

- Choose the next task from `Docs/03_MVP_Task_Breakdown.md`.
- Identify required context files.
- Define allowed files and forbidden changes.
- Ask Codex for a plan.
- Approve, reject, or revise the plan.
- Decide whether scope expansion is allowed.

The Orchestrator does not merge unverified work.

### Execute

The Execute role implements the approved task.

Responsibilities:

- Work on the approved task branch or worktree.
- Modify only approved files.
- Keep changes small and reviewable.
- Run required verification.
- Produce the Approval Report.
- Prepare PR-ready output.
- Stop when scope expansion is required.

The Execute role does not merge to `main`.

### Merge

The Merge role reviews and integrates completed work.

Responsibilities:

- Confirm the PR matches the approved task.
- Confirm the Approval Report is present.
- Confirm the Learning Review is present.
- Confirm verification results are stated accurately.
- Confirm remaining risks are acceptable.
- Merge only after user approval.
- Sync local `main` after merge.

The Merge role protects `main` from unapproved or unverified work.
