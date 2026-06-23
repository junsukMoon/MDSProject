# Approval Report Template

Use this template after completing an approved Codex task for `MDSProject`.

The report must describe what was done, what changed, how it was verified, and whether the implementation stayed inside the approved scope.

## Objective

State the approved task objective.

`<Objective from the approved plan>`

## Plan Executed

Summarize the approved plan that was implemented.

- `<Approved plan item 1>`
- `<Approved plan item 2>`
- `<Approved plan item 3>`

If the final implementation differed from the approved plan, describe that in `Deviations From Plan`.

## Changed Files

List every file that was created, modified, moved, or deleted.

- `<Changed file path>` - `<Created / Modified / Moved / Deleted>`

If any file was changed outside the approved scope, report it explicitly:

- `Out-of-scope change:` `<File path>` - `<Reason and impact>`

Do not omit accidental or unexpected file changes.

## Implementation Summary

Describe the completed work in reviewable terms.

- `<What was added or changed>`
- `<How it fits the approved objective>`
- `<Important behavior or workflow impact>`

For `.Build.cs` or module dependency changes, include the reason for each added, removed, or changed module:

- `<Module name>` - `<Reason required>`

## Verification

State exactly what verification was performed and the result.

- `<Build / compile / PIE / dedicated server / log check / manual inspection>` - `<Result>`

Do not claim any test, build, compile, PIE run, dedicated server run, or log check passed unless it was actually run.

If verification was not possible, state the reason:

- `Not run:` `<Verification step>` - `<Reason>`

For network or replication-related changes, include server/client verification notes:

- `Server behavior:` `<Observed or expected result>`
- `Client behavior:` `<Observed or expected result>`
- `Authority / ownership notes:` `<Relevant replication or RPC notes>`

## Manual Test Steps

List the steps a human can use to verify the result.

1. `<Manual step 1>`
2. `<Manual step 2>`
3. `<Expected result>`

Manual steps should be concrete enough to reproduce in the editor, a listen server, a dedicated server, logs, or file review.

## Risks / Notes

List remaining risks, known limitations, or review notes.

- `<Risk or note>`

Separate current-task risks from pre-existing issues when possible.

## Deviations From Plan

State whether the implementation deviated from the approved plan.

- `None`

If there were deviations, replace `None` with:

- `<Deviation>` - `<Reason>` - `<Impact>`

Any file changed outside the approved scope must also be listed in `Changed Files`.

## Assumptions Made

List assumptions used during implementation or verification.

- `<Assumption>`

If no assumptions were made, write:

- `None`

## Next Suggested Task

Suggest one focused next task that follows from the completed work.

`<Next small, reviewable task>`

