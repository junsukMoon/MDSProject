# AI Task Commands

## Purpose

This document defines standard commands for running the AI task branch workflow in `MDSProject`.

These commands are written as reusable prompts that the user can give to Codex. They are intended to keep planning, execution, learning review, PR preparation, and merge work separated.

Unless the user explicitly requests otherwise, Codex should answer in Korean.

## Approval Chain Rule

When the user approves an Orchestrator plan, that approval allows Codex to continue through Execute, Learning Review, and PR Preparation for the approved task scope.

This chained approval applies only while all of the following remain true:

- The task objective is unchanged.
- The branch matches the approved task.
- Changed files stay within the approved scope.
- Verification follows the approved plan.
- The PR is prepared for review, not merged.

If Codex needs to expand scope, modify unapproved files, change the task objective, or skip required verification, Codex must stop and ask for additional approval.

Merge is never included in Orchestrator approval. Merge requires a separate explicit user approval for the specific PR.

## Orchestrator Command

Use this command to select and scope the next task before implementation.

```text
Read AGENTS.md, Docs/AI_Harness.md, Docs/04_Git_Workflow.md, Docs/05_Progress_Log.md, and any task-specific documents.

Role:
Act as Orchestrator.

Task:
Select and scope the next AI task branch.

Requirements:
- Identify the linked task from Docs/03_MVP_Task_Breakdown.md.
- Identify required context files to inspect.
- Identify allowed files to modify.
- Identify forbidden changes.
- Identify verification requirements.
- Identify branch name.
- Do not modify files.
- Provide a Plan using the AGENTS.md required format.
- Wait for explicit user approval before implementation.
```

Expected output:

- Current structure summary.
- Proposed task scope.
- Expected changed files.
- Verification plan.
- Approval request.

## Execute Command

Use this command after the Orchestrator plan has been approved.

```text
Read AGENTS.md, Docs/AI_Harness.md, Docs/04_Git_Workflow.md, Docs/05_Progress_Log.md, and the approved task plan.

Role:
Act as Execute.

Task:
Implement only the approved task scope on the approved task branch.

Requirements:
- Create or switch to the approved task branch.
- Modify only approved files.
- Do not commit directly to main.
- Do not broaden scope.
- If scope expansion is required, stop and ask for approval.
- Run the approved verification steps.
- Do not claim unrun tests passed.
- Provide an Approval Report using the AGENTS.md required format.
- Do not merge to main.
```

Expected output:

- Changed files.
- Implementation summary.
- Verification results.
- Manual test steps.
- Risks and notes.
- Next suggested task.

## Learning Review Command

Use this command after implementation and verification, before PR merge.

```text
Read AGENTS.md, Docs/AI_Harness.md, Docs/04_Git_Workflow.md, Docs/05_Progress_Log.md, and the Approval Report for the task.

Role:
Act as Learning Review.

Task:
Prepare a concise learning review for the completed task.

Requirements:
- Explain what changed.
- Explain why each file changed.
- Explain what was verified.
- Explain what was not verified.
- Explain remaining risks.
- Explain key Unreal concepts involved, if any.
- Explain networking or authority implications, if any.
- Generate interview questions and short expected answers.
- Do not invent verification that was not run.
```

Expected output:

```text
What changed:
Why each file changed:
What was verified:
What was not verified:
Risks remaining:
Key Unreal concepts:
Networking / authority implications:
Generated interview questions:
```

## PR Preparation Command

Use this command when the task branch is ready to become a pull request.

```text
Read AGENTS.md, Docs/04_Git_Workflow.md, Docs/05_Progress_Log.md, .github/pull_request_template.md, and the Approval Report for the task.

Role:
Act as PR Preparation.

Task:
Prepare a PR-ready branch and PR description.

Requirements:
- Confirm the current branch is not main.
- Confirm the branch maps to one task from Docs/03_MVP_Task_Breakdown.md.
- Confirm changed files match the approved scope.
- Confirm the Approval Report is complete.
- Confirm the Learning Review is complete.
- Confirm verification results are accurate.
- Use a Korean commit message.
- Use a Korean PR title and Korean PR body.
- Include the Approval Report in the PR body.
- Include the Learning Review in the PR body.
- Push the task branch, not main.
- Do not merge without explicit user approval.
```

Expected output:

- Branch name.
- Commit message.
- PR title.
- PR body.
- Verification summary.
- Merge readiness summary.

## Merge Command

Use this command only after the user explicitly approves merge.

```text
Read AGENTS.md, Docs/04_Git_Workflow.md, Docs/05_Progress_Log.md, and the target PR.

Role:
Act as Merge.

Task:
Merge the approved PR and synchronize local main.

Requirements:
- Confirm the user explicitly approved merge.
- Confirm the PR matches the approved task.
- Confirm the Approval Report is present.
- Confirm the Learning Review is present.
- Confirm verification results are stated accurately.
- Confirm remaining risks are acceptable to the user.
- Merge only the approved PR.
- Do not bypass branch protection.
- After merge, switch to main.
- Pull origin/main with fast-forward only.
- Confirm working tree is clean.
- Delete the local task branch if appropriate.
- Delete the remote task branch if appropriate.
- Report the merge commit and final status.
```

Expected output:

- PR number.
- Merge result.
- Merge commit.
- Local `main` sync result.
- Branch cleanup result.
- Final `git status`.
- Next suggested task.

## When To Use Each Command

Use Orchestrator when:

- Choosing the next task.
- Turning a rough idea into a scoped task.
- Defining allowed files and forbidden changes.
- Preparing a task plan before implementation.

Use Execute when:

- The plan has been approved.
- The branch exists or should be created.
- Files need to be changed.
- Verification needs to be run.

Use Learning Review when:

- The implementation is complete.
- The Approval Report exists.
- The PR needs interview-ready explanation.
- The user wants to understand what was learned.

Use PR Preparation when:

- The task branch is ready for review.
- The Approval Report and Learning Review should be placed into a PR.
- The branch should be pushed to GitHub.

Use Merge when:

- The PR is open and reviewed.
- The user explicitly approves merge.
- Remaining risks are understood.
- The branch should be integrated into `main`.

## Safety Rules

Safety rules:

- Codex must never commit directly to `main`.
- Codex must never push directly to `main`.
- Each AI task must use a separate task branch.
- Each task branch must map to one task from `Docs/03_MVP_Task_Breakdown.md`.
- Codex must inspect relevant files before proposing a plan.
- Codex must wait for explicit user approval before modifying files.
- After Orchestrator approval, Codex may continue through Execute, Learning Review, and PR Preparation for the approved scope.
- Codex must modify only approved files.
- Codex must stop and ask for approval if scope expansion is required.
- Codex must not claim unrun tests passed.
- PRs must include an Approval Report.
- PRs must include a Learning Review.
- Commit messages, PR titles, and PR bodies should be written in Korean.
- Merge must not happen until the user explicitly approves it.

## Main Merge Approval Rule

`main` merge requires explicit user approval.

Rules:

- Opening a PR is not merge approval.
- A completed Approval Report is not merge approval.
- A completed Learning Review is not merge approval.
- A clean PR state is not merge approval.
- Passing checks are not merge approval.
- Codex must ask before merging.
- Codex may merge only after the user clearly approves merge for that PR.

Accepted merge approval examples:

```text
PR #3 merge 승인.
이 PR merge 진행해.
검토 완료, merge 해줘.
```

Not accepted as merge approval:

```text
확인해줘.
PR 상태 봐줘.
다음 작업 가자.
```
