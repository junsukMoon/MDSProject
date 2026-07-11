# Character Customization Notes

이 문서는 MDS v2의 modular character parts와 Mutable future extension 범위를 정리합니다.

## MVP Position

MDS v2는 character customization demo가 아닙니다. 캐릭터 파츠 구조는 면접에서 skeletal character 구조와 component attachment 기본기를 설명할 수 있는 최소 범위로만 다룹니다.

## Simple Modular Slots

MVP에서 구현한다면 다음 정도로 제한합니다.

- Head mesh slot
- Body mesh slot
- Weapon mesh slot
- component-based attachment
- socket-based weapon attachment

목표는 캐릭터 구조를 설명하는 것입니다. 런타임 character creator, 복잡한 파츠 조합, 대규모 cosmetic pipeline은 포함하지 않습니다.

## Mutable Future Extension

Mutable은 향후 character customization 확장 후보로만 문서화합니다.

Future extension에서 검토할 항목:

- runtime customization data model
- skeletal mesh part combination
- material/texture variation
- cook and asset pipeline impact
- replication requirements for selected appearance
- UI requirements for customization selection

## Out of Scope

MVP에서는 다음을 구현하지 않습니다.

- full Mutable pipeline
- complex character creator UI
- advanced skeletal mesh merge/cook pipeline
- large cosmetic inventory
- production character customization workflow

MDS v2는 Objective Combat Demo이며, customization은 핵심 데모를 보조하는 future extension 주제입니다.
