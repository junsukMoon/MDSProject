# Profiling Comparison

이 문서는 Mass objective scenario와 Actor baseline 비교 기록입니다.

목표는 production benchmark가 아니라 면접에서 설명 가능한 측정 context를 남기는 것입니다.

## Scenario

- Project: `MDSProject`
- Map: `/Game/TopDown/Lvl_TopDown`
- Engine: UE 5.8 source build
- 기본 Mass scenario:
  - Mass entities: `16`
  - Objective HP: `100`
  - damage per arrival: `5`
  - expected final HP: `20/100`

## 측정 방식

주요 profiling은 `-NullRHI`와 benchmark/headless runtime에서 수행했습니다.

주의:

- `-NullRHI`는 rendering/GPU cost를 대표하지 않습니다.
- 로컬 CPU/gameplay 비교에는 유용하지만 최종 viewport 성능 주장으로 사용하면 안 됩니다.

## Mass Runtime 결과

| Scenario | Runtime | Effective FPS | Average frame time |
| --- | --- | ---: | ---: |
| Standalone Mass scenario | UE 5.6 installed build, `-game -NullRHI` | `2571.43` | `0.39 ms` |
| Editor server-mode Mass scenario | UE 5.6 installed build, `-server -NullRHI` | `4500.00` | `0.22 ms` |
| Staged dedicated server binary | UE 5.8 source build, `-NullRHI` | `905.43` | `1.10 ms` |

## Debug Draw Fix

문제:

- arrival 이후 debug sphere가 반복적으로 그려져 runtime overhead가 커졌습니다.

수정:

- arrival marker는 entity가 처음 도착했을 때 한 번만 그리도록 변경했습니다.

결과:

| State | Effective FPS | Average frame time |
| --- | ---: | ---: |
| Before fix | `248.76` | `4.02 ms` |
| After fix | `2163.46` | `0.46 ms` |

## Actor vs Mass Phase Capture

조건:

- Engine: UE 5.8 source build
- Runtime: commandlet/headless style
- Mode: `-game -NullRHI -nosound -unattended`
- Capture frames: `600`
- Trigger: `MovementActive`
- Stable frames: `3`
- Expected count: `1000`

결과:

| Scenario | Samples | Avg FrameTime | P95 FrameTime | Max FrameTime | Avg TickActors |
| --- | ---: | ---: | ---: | ---: | ---: |
| Mass 1000 MovementActive | `600` | `0.4567 ms` | `0.4898 ms` | `7.9245 ms` | `0.0997 ms` |
| Actor 1000 MovementActive | `600` | `1.2473 ms` | `1.4643 ms` | `7.0531 ms` | `0.7907 ms` |

해석:

- 같은 `MovementActive` 조건에서 Actor baseline이 Mass path보다 높은 TickActors cost를 보였습니다.
- 이 결과는 headless local comparison입니다.
- final rendering performance 또는 GPU 성능 결론으로 해석하면 안 됩니다.

## Known Limitations

- visible client evidence는 replicated Objective HP 검증이지 viewport performance 검증이 아닙니다.
- Unreal Insights trace는 smoke capture입니다.
- 반복 측정이나 variance 분석은 추가 작업입니다.
