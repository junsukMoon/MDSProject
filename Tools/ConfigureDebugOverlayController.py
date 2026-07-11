import sys

import unreal


CONTROLLER_PATH = "/Game/TopDown/Blueprints/BP_TopDownController"
CONTROLLER_PARENT_PATH = "/Script/MDSProject.MDSProjectPlayerController"
INPUT_DEFAULTS = (
    ("ShortPressThreshold", 0.3),
    ("FXCursor", "/Game/TopDown/Cursor/FX_Cursor_Success"),
    ("DefaultMappingContext", "/Game/TopDown/Input/IMC_Default"),
    ("SetDestinationClickAction", "/Game/TopDown/Input/Actions/IA_SetDestination_Click"),
    ("SetDestinationTouchAction", "/Game/TopDown/Input/Actions/IA_SetDestination_Touch"),
)


def log(message):
    unreal.log(f"MDSOverlayControllerConfig: {message}")


def load_required_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Unable to load asset: {path}")
    return asset


def load_required_object(path):
    obj = unreal.load_object(None, path)
    if obj is None:
        raise RuntimeError(f"Unable to load object: {path}")
    return obj


def get_generated_class(blueprint):
    generated_class = blueprint.generated_class
    return generated_class() if callable(generated_class) else generated_class


def configure_input_defaults(controller):
    generated_class = get_generated_class(controller)
    default_object = unreal.get_default_object(generated_class)

    for property_name, value in INPUT_DEFAULTS:
        if isinstance(value, str):
            value = load_required_asset(value)

        default_object.set_editor_property(property_name, value)
        log(f"Set {property_name} default to {value}")


def main():
    controller = load_required_asset(CONTROLLER_PATH)
    desired_parent = load_required_object(CONTROLLER_PARENT_PATH)
    current_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(controller)

    log(f"Current parent: {current_parent}")
    if current_parent != desired_parent:
        unreal.BlueprintEditorLibrary.reparent_blueprint(controller, desired_parent)
        log(f"Reparented {CONTROLLER_PATH} to {CONTROLLER_PARENT_PATH}")
    else:
        log(f"{CONTROLLER_PATH} already uses {CONTROLLER_PARENT_PATH}")

    unreal.BlueprintEditorLibrary.compile_blueprint(controller)
    configure_input_defaults(controller)
    unreal.BlueprintEditorLibrary.compile_blueprint(controller)
    unreal.EditorAssetLibrary.save_asset(CONTROLLER_PATH, only_if_is_dirty=False)
    log("Compiled and saved BP_TopDownController")


try:
    main()
except Exception as exc:
    unreal.log_error(f"MDSOverlayControllerConfig failed: {exc}")
    sys.exit(1)
