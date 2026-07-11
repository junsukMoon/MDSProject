import sys

import unreal


ASSET_PATH = "/Game/MDS/UI"
ASSET_NAME = "WBP_MDSDebugOverlay"
FULL_ASSET_PATH = f"{ASSET_PATH}/{ASSET_NAME}"


def log(message):
    unreal.log(f"MDSDebugOverlayAsset: {message}")


def load_required_object(path):
    obj = unreal.load_object(None, path)
    if obj is None:
        raise RuntimeError(f"Unable to load required object: {path}")
    return obj


def ensure_directory(path):
    editor_asset_lib = unreal.EditorAssetLibrary
    if not editor_asset_lib.does_directory_exist(path):
        if not editor_asset_lib.make_directory(path):
            raise RuntimeError(f"Unable to create content directory: {path}")


def create_or_load_widget_blueprint():
    existing_asset = unreal.EditorAssetLibrary.load_asset(FULL_ASSET_PATH)
    if existing_asset:
        log(f"Loaded existing widget blueprint {FULL_ASSET_PATH}")
        return existing_asset

    ensure_directory(ASSET_PATH)

    parent_class = load_required_object("/Script/MDSProject.MDSDebugOverlayWidget")
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    widget_blueprint = asset_tools.create_asset(
        ASSET_NAME,
        ASSET_PATH,
        unreal.WidgetBlueprint,
        factory,
    )
    if widget_blueprint is None:
        raise RuntimeError(f"Unable to create widget blueprint: {FULL_ASSET_PATH}")

    log(f"Created widget blueprint {FULL_ASSET_PATH}")
    return widget_blueprint


def compile_and_save_widget_blueprint(widget_blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_asset(FULL_ASSET_PATH, only_if_is_dirty=False)
    log("Compiled and saved widget blueprint")


def main():
    widget_blueprint = create_or_load_widget_blueprint()
    compile_and_save_widget_blueprint(widget_blueprint)
    log("Done")


try:
    main()
except Exception as exc:
    unreal.log_error(f"MDSDebugOverlayAsset failed: {exc}")
    sys.exit(1)
