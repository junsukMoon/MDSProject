# AGENTS.md

## Project

이 프로젝트는 UE5 기술 포트폴리오 프로젝트입니다.

프로젝트 이름: `MDSProject`

목표:

- Dedicated Server
- Replication
- Authority / Ownership
- Objective gameplay
- Mass Entity / Mass AI
- Debug UI / Debug output
- Profiling
- AI-assisted development workflow

이 프로젝트는 완성형 게임이 아닙니다. 기술 시연과 면접 설명에 필요한 범위로 유지합니다.

---

## Core Workflow

non-trivial 작업은 항상 다음 순서를 따릅니다.

1. 관련 파일을 확인합니다.
2. 현재 구조를 요약합니다.
3. 계획을 제안합니다.
4. 명시적 승인을 기다립니다.
5. 승인된 변경만 구현합니다.
6. 결과를 검증합니다.
7. approval report를 제공합니다.

승인 전에는 파일을 수정하지 않습니다.

작업이 모호하면 구현 전에 가정을 명시합니다.

---

## Change Policy

- 작고 리뷰 가능한 변경을 선호합니다.
- 명시 요청 없이 broad refactor를 하지 않습니다.
- 명시 요청 없이 파일, 클래스, 함수, 변수, 폴더 이름을 바꾸지 않습니다.
- 관련 없는 시스템을 수정하지 않습니다.
- 작업에서 허용된 파일만 수정합니다.
- 가능하면 기존 gameplay class를 크게 바꾸기보다 작은 component/helper/class를 추가합니다.

---

## Unreal Engine Rules

Unreal Engine C++ 관례를 따릅니다.

특히 주의할 항목:

- Replication
- Server authority
- RPC ownership
- Lifetime replicated properties
- BeginPlay order
- Tick cost
- UObject lifetime
- Garbage collection
- Build.cs module dependencies

`.Build.cs`를 변경할 때는 추가한 module이 왜 필요한지 설명합니다.

---

## Coding Standards

C++ 변경 시 다음 문서를 따릅니다.

- `Docs/Coding_Standards.md`
- `Docs/Unreal_Rules.md`
- Mass 코드를 수정할 때는 `Docs/Mass_Rules.md`

---

## Network Rules

- server-authoritative gameplay가 기본입니다.
- health, damage, score, Objective HP 같은 gameplay state는 서버가 소유합니다.
- 클라이언트는 action을 요청할 수 있지만, 서버가 validate/apply합니다.
- replicated data를 추가하면 listen server 또는 dedicated server 검증 계획을 포함합니다.

---

## Mass Entity Rules

Mass 작업은 incremental하게 진행합니다.

명시 승인 없이 Mass spawn, movement, arrival detection, objective damage를 한 작업에 섞지 않습니다.

권장 순서:

1. Build/module setup
2. Spawn only
3. Movement only
4. Arrival detection
5. Objective damage integration
6. Debug/profiling

자세한 규칙은 `Docs/Mass_Rules.md`를 따릅니다.

---

## Required Plan Format

구현 전 계획은 다음 형식을 사용합니다.

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

---

## Required Approval Report Format

구현 후 보고는 다음 형식을 사용합니다.

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

---

## Verification Rules

실제로 실행하지 않은 검증을 성공했다고 말하지 않습니다.

코드 변경은 최소 하나 이상의 검증 결과를 보고합니다.

- Build result
- Compile result
- PIE test result
- Dedicated server test result
- Log output check
- Manual inspection result

검증을 실행할 수 없으면 이유를 명확히 설명합니다.

자세한 기준은 `Docs/Verification.md`를 따릅니다.

---

## Forbidden Scope Unless Explicitly Requested

명시 요청 없이는 추가하지 않습니다.

- Inventory
- Quest system
- Save system
- Matchmaking
- Lobby
- Crafting
- Skill tree
- Large UI framework
- Complex animation system
- Full GAS expansion

이 프로젝트를 완성형 게임으로 확장하지 않습니다.

---

## Additional Project Documents

관련 작업 시 다음 문서를 확인합니다.

- `Docs/AI_Harness.md`
- `Docs/Task_Template.md`
- `Docs/Approval_Report_Template.md`
- `Docs/Verification.md`
- `Docs/Coding_Standards.md`
- `Docs/Unreal_Rules.md`
- `Docs/Mass_Rules.md`
- `Docs/Requirements.md`
