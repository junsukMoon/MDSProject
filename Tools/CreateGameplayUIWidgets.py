import sys

import unreal


ASSET_PATH = "/Game/MDS/UI"

WIDGET_SPECS = [
    {
        "name": "WBP_MDSMatchHUD",
        "parent": "/Script/MDSProject.MDSMatchHUDWidget",
        "texts": [
            ("WaveTextBlock", "Wave: -"),
            ("EnemiesTextBlock", "Enemies: -"),
        ],
    },
    {
        "name": "WBP_MDSObjectiveWorldUI",
        "parent": "/Script/MDSProject.MDSObjectiveWorldWidget",
        "texts": [
            ("ObjectiveHealthTextBlock", "Objective HP: -"),
        ],
    },
    {
        "name": "WBP_MDSEnemyWorldUI",
        "parent": "/Script/MDSProject.MDSEnemyWorldWidget",
        "texts": [
            ("EnemyHealthTextBlock", "Enemy HP: -"),
        ],
    },
]


def log(message):
    unreal.log(f"MDSGameplayUIAsset: {message}")


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


def create_or_load_widget_blueprint(spec):
    full_asset_path = f"{ASSET_PATH}/{spec['name']}"
    existing_asset = unreal.EditorAssetLibrary.load_asset(full_asset_path)
    if existing_asset:
        log(f"Loaded existing widget blueprint {full_asset_path}")
        return existing_asset

    ensure_directory(ASSET_PATH)

    parent_class = load_required_object(spec["parent"])
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    widget_blueprint = asset_tools.create_asset(
        spec["name"],
        ASSET_PATH,
        unreal.WidgetBlueprint,
        factory,
    )
    if widget_blueprint is None:
        raise RuntimeError(f"Unable to create widget blueprint: {full_asset_path}")

    log(f"Created widget blueprint {full_asset_path}")
    return widget_blueprint


def set_text_block_defaults(text_block, label):
    text_block.set_editor_property("text", unreal.Text(label))
    text_block.set_editor_property("color_and_opacity", unreal.SlateColor(unreal.LinearColor.WHITE))
    text_block.set_editor_property("shadow_offset", unreal.Vector2D(1.0, 1.0))
    text_block.set_editor_property("shadow_color_and_opacity", unreal.LinearColor.BLACK)


def ensure_widget_tree(widget_blueprint, spec):
    root = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, "Root")
    if root is None:
        root = unreal.EditorUtilityLibrary.add_source_widget(
            widget_blueprint,
            unreal.VerticalBox,
            "Root",
            "",
        )
        if root is None:
            raise RuntimeError(f"Unable to add Root widget to {spec['name']}")
        log(f"Added Root VerticalBox to {spec['name']}")

    for text_name, label in spec["texts"]:
        text_block = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, text_name)
        if text_block is None:
            text_block = unreal.EditorUtilityLibrary.add_source_widget(
                widget_blueprint,
                unreal.TextBlock,
                text_name,
                "Root",
            )
            if text_block is None:
                raise RuntimeError(f"Unable to add TextBlock {text_name} to {spec['name']}")
            log(f"Added TextBlock {text_name} to {spec['name']}")
        else:
            log(f"Found TextBlock {text_name} in {spec['name']}")

        set_text_block_defaults(text_block, label)


def compile_and_save_widget_blueprint(widget_blueprint, spec):
    ensure_widget_tree(widget_blueprint, spec)
    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_asset(f"{ASSET_PATH}/{spec['name']}", only_if_is_dirty=False)
    log(f"Compiled and saved widget blueprint {ASSET_PATH}/{spec['name']}")


def main():
    for spec in WIDGET_SPECS:
        widget_blueprint = create_or_load_widget_blueprint(spec)
        compile_and_save_widget_blueprint(widget_blueprint, spec)
    log("Done")


try:
    main()
except Exception as exc:
    unreal.log_error(f"MDSGameplayUIAsset failed: {exc}")
    sys.exit(1)
