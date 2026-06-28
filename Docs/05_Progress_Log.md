# Progress Log

## Current Status

portfolio demo scope는 documentation, runtime log, visible viewport, profiling, smoke trace 수준에서 구현 및 검증되었습니다.

현재 focus:

- public documentation을 검증 증거와 맞게 유지
- 면접에서 설명하기 쉬운 project status 유지
- future work는 필수 MVP가 아니라 optional refinement로 취급

## Current Task

Task:

- visible demo verification merge 이후 final project status documentation 유지

Task type:

- Documentation

Allowed scope:

- `Docs/05_Progress_Log.md`

Status:

- 완료된 milestone 중심 progress log

## Completed Tasks

완료된 documentation setup:

- AI harness docs
- Project goal
- Requirements
- Architecture
- MVP task breakdown
- Git workflow
- Progress log
- PR template
- AI task commands
- Verification standards
- Unreal and Mass working rules

완료된 gameplay/system work:

- server-authoritative Objective Actor
- client debug visibility를 포함한 replicated Objective HP
- Dedicated Server target 및 staged runtime workflow
- Mass module setup
- Mass spawn
- Mass movement toward Objective
- Mass arrival detection
- server-side once-only Objective damage integration
- runtime debug state와 debug line
- Mass comparison용 Actor enemy baseline
- phase-based gameplay CSV profiling harness
- Actor vs Mass `MovementActive` phase profiling comparison
- Mass debug draw profiling guard
- dedicated server console launch batch file

완료된 verification evidence:

- UE 5.8 source engine build/runtime verification
- WindowsServer cook/stage/runtime verification
- dedicated server log verification
- two standalone client log verification
- visible two-client Objective HP screenshot verification
- 2-minute visible demo GIF
- short Unreal Insights smoke trace

## Active Branches

현재 알려진 active branch:

- `main` - integration branch

현재 active gameplay implementation branch는 없습니다.

## Blocked Tasks

없음.

## Next Recommended Task

이전 checklist scope 기준 필수 MVP task는 남아 있지 않습니다.

Optional future refinements:

1. processor-level analysis가 필요하면 deeper Unreal Insights session 캡처
2. 더 엄밀한 variance data가 필요하면 repeated profiling capture 실행
3. viewport performance claim이 필요할 때만 visible `stat unit` / `stat fps` 기록
4. 특정 면접/portfolio page를 위해 demo presentation material polish

## Verification Status

현재 verification status:

- Documentation inspection: public project docs 전반 완료
- C++ build checks: gameplay/profiling 변경 당시 수행
- Dedicated server runtime: staged server binary로 검증
- Network replication: two standalone client logs와 visible client windows로 검증
- Objective gameplay: server-owned HP가 `20/100`에 도달하는 것으로 검증
- Mass spawn/movement/arrival/damage: runtime debug state와 logs로 검증
- Actor baseline: 구현 및 Mass와 phase-profiled
- Profiling: Mass runtime snapshots와 Actor vs Mass phase capture 기록
- Visible demo evidence: screenshots와 2-minute GIF 기록
- Unreal Insights: smoke trace 캡처

Verification evidence references:

- `Docs/08_Profiling_Comparison.md`
- `Docs/10_Visible_Demo_Verification.md`
- `Docs/Verification/VisibleObjectiveHP_Client1.png`
- `Docs/Verification/VisibleObjectiveHP_Client2.png`
- `Docs/Verification/VisibleObjectiveHP_Demo.gif`
- `Docs/Verification/MDS_Insights_TraceSmoke.utrace`

## Merge History

Milestone merge history:

- initial project documentation and UE baseline
- workflow and AI harness documentation
- Mass concept and Mass working rules
- Objective HP implementation and replication verification
- dedicated server build/cook/stage/runtime support
- Mass spawn, movement, arrival, damage integration
- debug output and replicated Objective HP client debug fixes
- Actor enemy baseline
- phase-based profiling harness
- Actor vs Mass phase profiling comparison
- README Interview Demo update
- visible client demo verification, GIF, Unreal Insights smoke trace
- final README/profiling status cleanup

## Learning Review Status

Learning review expectations:

- non-trivial task는 what changed, what was verified, what was not verified, remaining risks, portfolio value, next improvement를 설명해야 합니다.

현재 상태:

- 프로젝트는 면접 discussion에 필요한 implementation과 verification evidence를 갖추었습니다.
- 남은 improvement는 missing MVP가 아니라 optional depth work입니다.

## Risks / Notes

- `-NullRHI` profiling result는 local comparison에는 유용하지만 final viewport/GPU performance로 주장하면 안 됩니다.
- Unreal Insights trace는 smoke capture이며 full performance investigation이 아닙니다.
- 이 프로젝트는 full game이 아니라 technical portfolio sandbox입니다.
- 이 log는 작은 PR마다 stale해지지 않도록 milestone 중심으로 유지합니다.
