# Profiling Comparison

이 문서는 현재 Mass objective scenario와 Actor baseline 비교 profiling 기록을 정리합니다.

목표는 production benchmark가 아니라, 면접에서 설명 가능한 측정 맥락을 남기는 것입니다.

## Scenario

- Project: `MDSProject`
- Map: `/Game/TopDown/Lvl_TopDown`
- Engine:
  - Epic Launcher UE 5.6 installed build
  - source-built UE 5.8
- 기본 Mass scenario:
  - Mass entities: `16`
  - Objective HP: `100`
  - damage per arrival: `5`
  - expected final HP: `20/100`

## Measurement Method

초기 Mass runtime 측정은 `UnrealEditor-Cmd.exe` 또는 staged `MDSProjectServer.exe`, `-NullRHI`, benchmark mode로 실행했습니다.

FPS와 average frame time은 log timestamp와 frame counter를 기준으로 계산했습니다.

이 방식은 local before/after 비교에는 유용하지만, viewport rendering, GPU cost, PIE/editor overhead를 대표하지 않습니다.

## Mass Runtime Results

| Scenario | Runtime | Effective FPS | Average frame time |
| --- | --- | ---: | ---: |
| Standalone Mass scenario | UE 5.6 installed build, `-game -NullRHI` | `2571.43` | `0.39 ms` |
| Editor server-mode Mass scenario | UE 5.6 installed build, `-server -NullRHI` | `4500.00` | `0.22 ms` |
| Staged dedicated server binary | UE 5.8 source build, `-NullRHI` | `905.43` | `1.10 ms` |

Staged dedicated server verification:

- `Premade AssetRegistry loaded`
- `World NetMode = Dedicated Server`
- `IpNetDriver listening on port 7777`
- final debug line:

```text
MDS Debug | NetMode=DedicatedServer | ObjectiveHP=20/100 | Mass Spawned=16 Moved=0 Arrived=16 Damage=16
```

## Dedicated Server Two-Client Replication Check

Headless log verification:

- Server log: `C:\Temp\MDS_Dedicated_TwoClients_Server.log`
- Client 1 log: `C:\Temp\MDS_Dedicated_TwoClients_Client1.log`
- Client 2 log: `C:\Temp\MDS_Dedicated_TwoClients_Client2.log`

확인 결과:

- server는 port `7777`에서 listen
- client 2개 login/join 성공
- client 1/2 모두 replicated `ObjectiveHP=20/100` 확인

이후 visible two-client screenshot/GIF evidence는 `Docs/10_Visible_Demo_Verification.md`에 기록했습니다.

## Debug Draw Fix Measurement

PR #11 이전에는 post-arrival slowdown이 있었습니다.

원인:

- arrived entity마다 매 프레임 5초짜리 cyan debug sphere를 생성했습니다.
- 모든 entity가 도착한 뒤에도 debug primitive가 계속 누적되었습니다.

수정:

- arrival marker는 entity가 처음 도착했을 때 한 번만 그립니다.

측정 결과:

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before fix | `248.76` | `4.02 ms` |
| After fix | `2163.46` | `0.46 ms` |

제한:

- 이 결과는 `-NullRHI` 기준입니다.
- 실제 viewport debug draw cost는 다를 수 있습니다.

## Actor vs Mass Phase Capture

Runtime date: 2026-06-29

조건:

- Engine: source-built UE 5.8
- Runtime: `MDSProjectEditor-Cmd.exe`
- Mode: `-game -NullRHI -nosound -unattended`
- Capture frames: `600`
- Trigger: `MovementActive`
- Stable frames: `3`
- Expected count: `1000`

Mass 조건:

- `-NoMDSMassDebugDraw`
- `-MDSMassBaselineCount=1000`
- `-MDSGameplayProfileSubject=Mass`

Actor 조건:

- `-NoMDSMassBaseline`
- `-MDSActorBaseline`
- `-MDSActorBaselineCount=1000`
- `-MDSGameplayProfileSubject=Actor`

CSV results:

| Scenario | Numeric samples | Avg FrameTime | P95 FrameTime | Max FrameTime | Avg TickActors | P95 TickActors | TotalActorCount | Ticks/Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Mass 1000 MovementActive | `600` | `0.4567 ms` | `0.4898 ms` | `7.9245 ms` | `0.0997 ms` | `0.1172 ms` | `87` | `38` |
| Actor 1000 MovementActive | `600` | `1.2473 ms` | `1.4643 ms` | `7.0531 ms` | `0.7907 ms` | `0.9467 ms` | `1087` | `1038` |

Measured delta:

- Actor average `FrameTime`은 Mass 대비 약 `2.73x`였습니다.
- Actor average `Exclusive/GameThread/TickActors`는 Mass 대비 약 `7.93x`였습니다.
- Actor total tick count는 1000개 Actor enemy spawn과 맞게 증가했습니다.

제한:

- 이 결과는 headless `MovementActive` capture 기준입니다.
- viewport/GPU 성능이나 엔진 전체 성능 결론으로 말하면 안 됩니다.

## Findings

- Mass scenario는 standalone, editor server-mode, staged dedicated server binary에서 expected objective state에 도달했습니다.
- staged dedicated server는 final Objective HP를 standalone client 2개에 replicate했습니다.
- post-arrival movement work는 `0` moved entities로 떨어집니다.
- 가장 큰 runtime issue는 Mass movement 자체가 아니라 debug visualization overhead였습니다.
- dedicated server binary support는 source-built UE 5.8 기준 staged runtime/log 수준에서 검증되었습니다.
- visible two-client Objective HP evidence와 Unreal Insights smoke trace는 `Docs/10_Visible_Demo_Verification.md`에 기록되어 있습니다.

## Known Limitations

- `-NullRHI` 결과는 final rendering performance로 주장하면 안 됩니다.
- visible client evidence는 replicated Objective HP display 검증이지 final viewport performance 검증이 아닙니다.
- Unreal Insights trace는 smoke capture이며 full performance investigation이 아닙니다.
- UE 5.8 `ZenStore` cook output은 별도 고려가 필요했고, staged server runtime 검증에는 `-skipzenstore`를 사용했습니다.

## Optional Future Profiling Refinements

1. 더 엄밀한 variance data가 필요하면 repeated capture를 수행합니다.
2. processor-level analysis가 필요하면 deeper Unreal Insights session을 캡처합니다.
3. viewport performance claim이 필요할 때만 visible `stat unit` / `stat fps`를 기록합니다.
