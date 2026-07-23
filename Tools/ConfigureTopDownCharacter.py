import sys

import unreal


CHARACTER_PATH = "/Game/TopDown/Blueprints/BP_TopDownCharacter"
CHARACTER_PARENT_PATH = "/Script/MDSProject.MDSProjectCharacter"


def log(message):
    unreal.log(f"MDSCharacterConfig: {message}")


def main():
    character = unreal.EditorAssetLibrary.load_asset(CHARACTER_PATH)
    if character is None:
        raise RuntimeError(f"Unable to load asset: {CHARACTER_PATH}")

    desired_parent = unreal.load_object(None, CHARACTER_PARENT_PATH)
    if desired_parent is None:
        raise RuntimeError(f"Unable to load class: {CHARACTER_PARENT_PATH}")

    current_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(character)
    log(f"Current parent: {current_parent}")
    if current_parent != desired_parent:
        unreal.BlueprintEditorLibrary.reparent_blueprint(character, desired_parent)
        log(f"Reparented {CHARACTER_PATH} to {CHARACTER_PARENT_PATH}")
    else:
        log(f"{CHARACTER_PATH} already uses {CHARACTER_PARENT_PATH}")

    unreal.BlueprintEditorLibrary.compile_blueprint(character)
    new_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(character)
    if new_parent != desired_parent:
        raise RuntimeError(f"Unexpected parent after reparent: {new_parent}")

    if not unreal.EditorAssetLibrary.save_asset(CHARACTER_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Unable to save asset: {CHARACTER_PATH}")

    log(f"Compiled and saved {CHARACTER_PATH} with parent {new_parent}")


try:
    main()
except Exception as exc:
    unreal.log_error(f"MDSCharacterConfig failed: {exc}")
    sys.exit(1)
