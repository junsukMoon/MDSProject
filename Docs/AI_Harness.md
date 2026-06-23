# AI Harness

## Purpose

The AI harness defines how AI assistance is used on `MDSProject`.

The goal is to make AI useful for planning, implementation support, verification, and documentation while keeping the project human-directed, server-authoritative, and focused on technical portfolio value.

AI is a development assistant, not the project owner. The workflow must produce small, reviewable changes that can be explained in an interview.

## Human Role

The human owns:

- Project goals and priorities
- Task approval and scope control
- Gameplay and technical design decisions
- Final code review
- Final verification judgment
- Interview narrative and portfolio framing

The human must approve non-trivial implementation plans before files are modified.

## AI Role

The AI assists by:

- Inspecting existing files before proposing changes
- Summarizing the current project structure
- Producing scoped implementation plans
- Implementing only approved changes
- Identifying Unreal Engine, networking, Mass Entity, and verification risks
- Suggesting focused tests or manual checks
- Producing approval reports after implementation

The AI must not broaden the task, rename unrelated symbols, refactor unrelated systems, or create full-game features unless explicitly requested.

## Standard Workflow

For every non-trivial task:

1. Inspect relevant files.
2. Summarize the current structure.
3. Propose a plan using the required plan format.
4. Wait for explicit human approval.
5. Implement only the approved changes.
6. Verify the result.
7. Provide an approval report using the required report format.

If the task is ambiguous, the AI must state assumptions before implementation. If assumptions materially affect scope or architecture, the AI must ask for clarification before changing files.

## Approval Gates

Approval is required before:

- Creating or modifying source files
- Creating or modifying config files
- Adding Unreal modules or plugin dependencies
- Changing replicated data or RPC behavior
- Adding Mass Entity systems or processors
- Adding gameplay systems beyond the approved task
- Performing broad refactors or renames

Approval reports are required after implementation and must include changed files, verification results, manual test steps, risks, and the next suggested task.

## Task Size Rules

Tasks should be small enough to review in one pass.

Preferred task shape:

- One feature slice
- One system boundary
- One verification path
- Minimal changed files

Avoid combining unrelated work. Mass Entity work must remain incremental and should follow this order unless explicitly approved otherwise:

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection
5. Objective damage integration
6. Debug/profiling

Do not combine Mass spawn, movement, arrival detection, and objective damage in one task unless explicitly requested.

## Verification Rules

The AI must not claim a test passed unless it was actually run.

For code changes, verification should include at least one relevant result:

- Build result
- Compile result
- PIE test result
- Dedicated server test result
- Log output check
- Manual inspection result

Networked gameplay changes must include a listen server or dedicated server verification plan. Replicated gameplay state must be checked for server authority, client visibility, and ownership rules.

If verification cannot be run, the approval report must state why and list the remaining manual checks.

## Failure Handling

When a task fails, the AI must:

- Stop broadening the scope.
- Report the exact failure or uncertainty.
- Identify the smallest next diagnostic step.
- Avoid hiding unverified assumptions.
- Avoid rewriting unrelated systems to work around the failure.

If a build, compile, or test fails, the AI should separate:

- Failures caused by the current change
- Pre-existing failures
- Environment or tooling failures
- Unknown failures requiring human review

The next plan should address only the smallest confirmed cause.

## Weekly Usage Pattern

A practical weekly workflow:

- Start the week by selecting one portfolio-relevant technical goal.
- Break the goal into small approved tasks.
- Use AI for file inspection, planning, focused implementation, and verification checklists.
- Keep each task reviewable and explainable.
- End the week by documenting what was built, how it was verified, and what interview topic it demonstrates.

Weekly progress should favor demonstrable multiplayer systems, server authority, Mass AI behavior, debug UI, profiling evidence, and clear technical writeups.

## Interview Value

The AI harness itself is part of the portfolio story.

It demonstrates that the project was built with:

- Human-owned technical direction
- Explicit approval gates
- Controlled AI assistance
- Small reviewable changes
- Verification discipline
- Clear server-authoritative reasoning
- Documentation suitable for interview discussion

The expected interview message is not that AI replaced engineering judgment. The message is that AI was used as a controlled accelerator while the human maintained ownership of architecture, correctness, verification, and tradeoffs.

