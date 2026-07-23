# Visible Demo Verification

이 문서는 `MDSProject`의 visible viewport replication check와 demo capture evidence를 기록합니다.

## Runtime

- Date: 2026-06-29
- Engine: UE 5.8 source build
- Server: staged `MDSProjectServer.exe`
- Clients: staged `MDSProject.exe` standalone clients
- Map: `/Game/TopDown/Lvl_TopDown`

## Launch Setup

Dedicated server:

```text
MDSProject/Saved/StagedBuilds/WindowsServer/MDSProject/Binaries/Win64/MDSProjectServer.exe
/Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -port=7777
```

Visible clients:

```text
MDSProject/Saved/StagedBuilds/Windows/MDSProject/Binaries/Win64/MDSProject.exe
127.0.0.1:7777 -windowed -ResX=960 -ResY=540 -nosound -NoSplash
```

## Log Evidence

서버 로그에서 확인한 항목:

- `IpNetDriver listening on port 7777`
- client login/join
- final debug state

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

클라이언트 로그에서 확인한 항목:

```text
MDS Debug | NetMode=Client | ObjectiveHP=20/100 | Mass Spawned=0 Moved=0 Arrived=0 Damage=0
```

## Visual Evidence

- `Docs/Verification/VisibleObjectiveHP_Client1.png`
- `Docs/Verification/VisibleObjectiveHP_Client2.png`
- `Docs/Verification/VisibleObjectiveHP_Demo.gif`

두 standalone client가 같은 server-owned Objective HP `20/100`을 표시하는 것을 확인했습니다.

## Latest Actor-Following World UI Evidence

- Date: 2026-07-17
- PR: #43 (`Make world UI follow owning actors`)
- Script: `Run_Verify_ReplicatedUIViewport.ps1 -Port 7779 -ActorEnemyCount 4 -ActorEnemyMoveSpeed 30`
- Server: staged `MDSProjectServer.exe`
- Client: staged `MDSProject.exe`
- Map: `/Game/TopDown/Lvl_TopDown`
- Visible screenshot:
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png`
- Logs:
  - `SavedVerifyLogs/MDS_GameplayUIAsset.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Server.log`
  - `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client.log`

Result observed from the verification run:

```text
REPLICATED UI VIEWPORT VERIFY RESULT: PASS
```

Visible evidence:

- The engine screenshot shows Match HUD/debug text, an Objective HP label near the objective, and four separated Enemy HP labels around the objective.
- Enemy HP labels are no longer captured as a single overlapped label at the screen center.
- Objective and Enemy World UI are screen-space widget components attached to their owning actors; no per-tick manual UI location update is required for following behavior.

Runtime log evidence:

```text
IpNetDriver listening on port 7779
Combat enemy wave spawn created 4/4 enemies around objective MDS_ActorObjectiveProbe
MDS Match HUD read GameState wave state: Wave=1 Remaining=0 Total=0 Active=false.
ObjectiveWorldUITrack Actor=... WidgetWorld=... Screen=... Projected=true
EnemyWorldUITrack Actor=... WidgetWorld=... Screen=... Projected=true
```

Notes:

- `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png` is the visible placement evidence for this run. `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_PrintWindow.png` was produced by the script but is not used as visual proof.
- Some initial tracking samples are `Projected=false` when actors are outside the current camera projection, but later samples include `Projected=true` for Objective UI and all four Enemy UI labels.
- The screenshot is visible layout evidence. The multiple Objective/probe context means it should not be treated as a single Objective HP state transition proof.

## Player Attack Runtime Evidence Reference

Player attack verification is tracked as runtime/log evidence rather than visible screenshot evidence.

- Script: `Run_Verify_PlayerAttack.ps1`
- Latest result: `PLAYER ATTACK VERIFY RESULT: PASS`
- Scenarios: `Valid`, valid directional miss (`OutOfRange` legacy scenario name), `Cooldown`, `InvalidDirection`, `InvalidDamage`, and `NoPawn`
- Logs: `SavedVerifyLogs/MDS_PlayerAttack_*`
- Detailed evidence: `Docs/11_Runtime_Review_Evidence.md`

This verifies owning-client attack intent, server validation, Enemy HP replication, HP-derived death handling, Wave remaining decrement, and directional-fire valid/reject scenarios. Animation presentation is verified by the later dedicated presentation sections.

## Combat Presentation Hook Evidence Reference

Combat presentation hook verification is tracked as runtime/log evidence rather than visible animation evidence.

- Script: `Run_Verify_CombatPresentationHooks.ps1`
- Evidence type: C++ hook/log ordering
- Detailed evidence: `Docs/11_Runtime_Review_Evidence.md`

This verifies local attack presentation hooks, replicated Enemy HP-driven hit/death presentation hooks, and a presentation-only negative path that does not send an attack RPC or apply Enemy HP damage. It does not verify authored Attack Montage playback, real AnimNotify asset firing, authored Hit Reaction, authored Death Animation, or viewport-visible animation pose changes.

## Combat Animation Asset Readiness Reference

Combat animation asset readiness is tracked as Editor-Cmd asset evidence rather than visible animation evidence.

- Script: `Run_Verify_CombatAnimationAssets.ps1`
- Evidence type: existing asset loadability and skeleton compatibility
- Latest result: `COMBAT ANIMATION ASSET VERIFY RESULT: PASS_WITH_INCOMPLETE_ITEMS`
- Log: `SavedVerifyLogs/MDS_CombatAnimationAssets.log`

This verifies that existing attack, hit reaction, and death animation candidate assets can be loaded and are compatible with the `BP_TopDownCharacter` skeletal mesh skeleton. The authored Notify, character lineage, simulated-client playback, and viewport pose changes are verified by later runtime passes.

`PASS_WITH_INCOMPLETE_ITEMS` is the historical result of this earlier read-only asset scan. Its lineage and Notify limitations were superseded by the later native-parent, persistent authored AnimNotify, runtime dispatch, simulated-client playback, and pose-delta verification passes.

## Combat Animation Playback Attempt Reference

Combat animation playback attempt verification is tracked as runtime/log evidence rather than visible pose-change evidence.

- Script: `Run_Verify_CombatPresentationHooks.ps1`
- Evidence type: staged client animation playback API attempt/acceptance logs
- Latest result: `COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS`
- Detailed evidence: `Docs/11_Runtime_Review_Evidence.md`

This verifies that the staged client accepts playback attempts for the existing attack montage, hit reaction sequence, and death sequence after the expected presentation triggers. It does not verify viewport-visible pose changes, real AnimNotify firing, or simulated-client attack montage replication.

## Combat Animation Visible Capture Reference

Combat animation visible capture verification is tracked as viewport screenshot evidence correlated with runtime playback logs.

- Script: `Run_Verify_CombatAnimationVisibleCapture.ps1`
- Evidence type: staged visible client engine screenshots requested after successful attack, hit, and death animation playback events
- Historical result: `COMBAT ANIMATION VISIBLE CAPTURE VERIFY RESULT: PASS_WITH_POSE_LIMITATION`
- Latest result: `COMBAT ANIMATION POSE DELTA VERIFY RESULT: PASS`
- Latest command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Run_Verify_CombatAnimationVisibleCapture.ps1 -Port 7922 -ClientWaitSeconds 26 -SkipBuild -SkipStage
```

Visible screenshots:

- `SavedVerifyLogs/MDS_CombatAnimationVisible_AttackBefore.png`
- `SavedVerifyLogs/MDS_CombatAnimationVisible_AttackPose.png`
- `SavedVerifyLogs/MDS_CombatAnimationVisible_HitBefore.png`
- `SavedVerifyLogs/MDS_CombatAnimationVisible_HitPose.png`
- `SavedVerifyLogs/MDS_CombatAnimationVisible_DeathBefore.png`
- `SavedVerifyLogs/MDS_CombatAnimationVisible_DeathPose.png`

All six images exist, are nonempty/visible, and have compatible dimensions. Center-gameplay comparison measured Attack `75`, Hit `58`, and Death `803` changed samples. This proves rendered pose change; artistic quality and the exact visual frame of the authored Notify remain optional review items.

## Unreal Insights Trace

Smoke trace:

```text
Docs/Verification/MDS_Insights_TraceSmoke.utrace
```

Trace channels:

```text
cpu, frame, bookmark, log
```

이 trace는 smoke capture이며 full performance investigation은 아닙니다.

## Runtime AnimNotify Verification

- Runtime: UE 5.8 staged dedicated server plus staged `-NullRHI` client.
- Verification flag: `-MDSCombatAnimNotifyVerify`.
- Latest result: `COMBAT ANIMNOTIFY VERIFY RESULT: PASS`.
- Valid scenario observed four client `UAnimNotify::Notify` callbacks, zero server callbacks, and four server-authoritative PlayerAttack damage applications.
- Presentation-only scenario observed one client Notify callback, zero server callbacks, zero server attack resolutions, zero PlayerAttack damage, and zero Enemy HP replication.
- Latest resolution: `MM_Pistol_Fire_Montage` contains one persistent authored `MDSCombatTimingAnimNotify` at configured time `0.100` within its `0.667` second duration. Runtime transient injection has been removed.
- `COMBAT ANIMNOTIFY VERIFY RESULT: PASS`: valid attacks fired `4` client callbacks and presentation-only fired `1`; Dedicated Server callbacks remained `0` and presentation-only gameplay damage/RPC remained `0`.
- This is persistent asset and headless runtime callback evidence. Frame-accurate viewport pose/cue timing remains a separate visual check.

## Character Movement Replication Status

- Script: `Run_Verify_CharacterMovementReplication.ps1`
- Topology observed: dedicated server `Authority`, mover client `AutonomousProxy`, observer client `SimulatedProxy`.
- Current result: `CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS`.
- Mover `AutonomousProxy`, server `Authority`, and observer `SimulatedProxy` each reached maximum distance/speed `1620.5 / 600`; movement input was accepted and consumed.
- This is packaged runtime/log evidence for CMC replication. Capture physical WASD input and observer-visible locomotion pose separately for portfolio footage.

## Authored Gameplay UI Style Verification

- Date: 2026-07-23
- Evidence: `SavedVerifyLogs/MDS_ReplicatedUIViewport_Client_EngineShot.png`
- Match HUD: cyan Wave and Enemies text in the upper-left viewport.
- Objective World UI: gold HP text attached to the center Objective.
- Enemy World UI: red HP text attached to four enemies around the Objective.
- Asset compile/save, four-enemy spawn, WBP runtime classes, replicated-state reads, actor projection, connection, screenshot request/file, and visible pixels all passed.
- No CommonUI viewport error or fatal error was found.

## Final Standalone Enemy Presentation Review

- Date: 2026-07-24
- Runtime: Editor Standalone PIE using the newly built `MDSProjectEditor Win64 Development` target.
- Manual review confirmed enemies use locomotion animation while moving with Character Movement.
- Manual review confirmed a nonlethal server-owned hit briefly pauses movement and shows the hit reaction.
- Manual review confirmed a dead enemy remains in its death pose and then fades after the configured two-second hold.
- This is manual viewport evidence. It complements, but does not replace, the automated Dedicated Server/client authority and replication evidence recorded above.

## Result

- visible viewport replicated Objective HP verification: 완료
- two-client same server-owned Objective HP evidence: 완료
- demo GIF: 완료
- Unreal Insights smoke trace: 완료
