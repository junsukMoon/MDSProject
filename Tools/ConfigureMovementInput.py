import sys

import unreal


ACTION_PATH = "/Game/TopDown/Input/Actions/IA_Move"
ACTION_FOLDER = "/Game/TopDown/Input/Actions"
CONTEXT_PATH = "/Game/TopDown/Input/IMC_Default"


def log(message):
    unreal.log(f"MDSMovementInputConfig: {message}")


def make_modifier(modifier_class, outer):
    return unreal.new_object(modifier_class, outer=outer)


def configure_mapping(context, action, key_name, negate=False, swizzle=False):
    key = unreal.Key()
    key.set_editor_property("key_name", key_name)
    mapping = context.map_key(action, key)
    modifiers = []
    if negate:
        modifiers.append(make_modifier(unreal.InputModifierNegate, context))
    if swizzle:
        modifier = make_modifier(unreal.InputModifierSwizzleAxis, context)
        modifier.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
        modifiers.append(modifier)
    mapping.set_editor_property("modifiers", modifiers)
    log(f"Mapped {key_name} | Negate={negate} | Swizzle={swizzle}")


def main():
    action = unreal.EditorAssetLibrary.load_asset(ACTION_PATH)
    if action is None:
        factory = unreal.InputAction_Factory()
        action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "IA_Move", ACTION_FOLDER, unreal.InputAction, factory
        )
    if action is None:
        raise RuntimeError(f"Unable to create action: {ACTION_PATH}")

    action.set_editor_property("value_type", unreal.InputActionValueType.AXIS2D)
    context = unreal.EditorAssetLibrary.load_asset(CONTEXT_PATH)
    if context is None:
        raise RuntimeError(f"Unable to load mapping context: {CONTEXT_PATH}")

    context.unmap_all_keys_from_action(action)
    configure_mapping(context, action, "W", swizzle=True)
    configure_mapping(context, action, "S", negate=True, swizzle=True)
    configure_mapping(context, action, "A", negate=True)
    configure_mapping(context, action, "D")

    if not unreal.EditorAssetLibrary.save_asset(ACTION_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Unable to save action: {ACTION_PATH}")
    if not unreal.EditorAssetLibrary.save_asset(CONTEXT_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Unable to save mapping context: {CONTEXT_PATH}")
    log("Configured and saved IA_Move plus IMC_Default WASD mappings")


try:
    main()
except Exception as exc:
    unreal.log_error(f"MDSMovementInputConfig failed: {exc}")
    sys.exit(1)
