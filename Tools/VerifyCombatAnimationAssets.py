import sys

import unreal

unreal.load_module("AnimationBlueprintLibrary")
ANIMATION_LIBRARY = getattr(unreal, "AnimationBlueprintLibrary", None) or getattr(unreal, "AnimationLibrary", None)


CHARACTER_BLUEPRINT = "/Game/TopDown/Blueprints/BP_TopDownCharacter"
CHARACTER_CLASS = "/Script/MDSProject.MDSProjectCharacter"
ANIM_BLUEPRINT = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
AUTHORED_NOTIFY_MONTAGE = "/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage"
AUTHORED_NOTIFY_CLASS = "/Script/MDSProject.MDSCombatTimingAnimNotify"
AUTHORED_NOTIFY_CONFIGURED_TIME = 0.1

SKELETAL_MESH_CANDIDATES = [
    "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
    "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
    "/Game/Characters/Mannequins/Meshes/SK_Mannequin",
]

ATTACK_CANDIDATES = [
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_ChargedAttack",
    "/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage",
]

HIT_REACT_CANDIDATES = [
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Back_Med_01",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Hvy_01",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_01",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_02",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_03",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_04",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Med_01",
    "/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Med_02",
]

DEATH_CANDIDATES = [
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Back_01",
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01",
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_02",
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_03",
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Left_01",
    "/Game/Characters/Mannequins/Anims/Death/MM_Death_Right_01",
]


def log(message):
    unreal.log(f"MDSCombatAnimationAssets: {message}")


def warn(message):
    unreal.log_warning(f"MDSCombatAnimationAssets: {message}")


def error(message):
    unreal.log_error(f"MDSCombatAnimationAssets: {message}")


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        warn(f"MISSING | {path}")
    else:
        log(f"LOADED | {path} | Class={asset.get_class().get_name()}")
    return asset


def load_class(path):
    loaded_class = unreal.load_class(None, path)
    if loaded_class is None:
        warn(f"MISSING_CLASS | {path}")
    else:
        log(f"LOADED_CLASS | {path} | Class={loaded_class.get_name()}")
    return loaded_class


def get_parent_chain_names(loaded_class):
    names = []
    current = loaded_class
    while current is not None:
        names.append(current.get_name())
        try:
            current = current.get_super_class()
        except Exception:
            break
    return names


def is_class_child_of(loaded_class, expected_parent):
    if loaded_class is None or expected_parent is None:
        return False

    current = loaded_class
    while current is not None:
        if current == expected_parent or current.get_name() == expected_parent.get_name():
            return True
        try:
            current = current.get_super_class()
        except Exception:
            return False
    return False


def has_class_name(asset_or_class, expected_name):
    if asset_or_class is None:
        return False

    try:
        current = asset_or_class if isinstance(asset_or_class, unreal.Class) else asset_or_class.get_class()
    except Exception:
        return False

    while current is not None:
        if current.get_name() == expected_name:
            return True
        try:
            current = current.get_super_class()
        except Exception:
            return False
    return False


def get_generated_class(blueprint_asset, class_path):
    generated_class = None
    try:
        generated_class = blueprint_asset.get_editor_property("generated_class")
    except Exception:
        generated_class = None

    if generated_class is None:
        generated_class = load_class(class_path)
    return generated_class


def get_blueprint_parent_class(blueprint_asset):
    if blueprint_asset is None:
        return None

    for prop_name in ("parent_class", "ParentClass"):
        try:
            parent_class = blueprint_asset.get_editor_property(prop_name)
            if parent_class is not None:
                log(f"CHARACTER_BLUEPRINT_PARENT | {parent_class.get_name()}")
                return parent_class
        except Exception:
            pass

    return None


def get_asset_skeleton(asset):
    if asset is None:
        return None

    for prop_name in ("skeleton", "target_skeleton"):
        try:
            skeleton = asset.get_editor_property(prop_name)
            if skeleton is not None:
                return skeleton
        except Exception:
            pass

    try:
        if hasattr(asset, "get_preview_mesh"):
            preview_mesh = asset.get_preview_mesh()
            if preview_mesh is not None:
                return get_asset_skeleton(preview_mesh)
    except Exception:
        pass

    return None


def get_anim_notify_count(asset):
    if asset is None:
        return 0

    try:
        return len(ANIMATION_LIBRARY.get_animation_notify_events(asset))
    except Exception:
        pass

    return 0


def verify_authored_attack_notify():
    montage = load_asset(AUTHORED_NOTIFY_MONTAGE)
    notify_class = load_class(AUTHORED_NOTIFY_CLASS)
    if montage is None or notify_class is None:
        return False

    try:
        events = ANIMATION_LIBRARY.get_animation_notify_events(montage)
    except Exception as exc:
        warn(f"AUTHORED_ATTACK_NOTIFY | INCOMPLETE | Unable to read notifies: {exc}")
        return False

    play_length = ANIMATION_LIBRARY.get_sequence_length(montage)
    matches = []
    for event in events:
        try:
            notify = event.get_editor_property("notify")
        except Exception:
            notify = None
        if notify is None or notify.get_class().get_name() != notify_class.get_name():
            continue

        configured_time_in_range = 0.0 <= AUTHORED_NOTIFY_CONFIGURED_TIME <= play_length
        matches.append(notify)
        log(
            f"AUTHORED_ATTACK_NOTIFY | Class={notify.get_class().get_name()} | "
            f"ConfiguredTime={AUTHORED_NOTIFY_CONFIGURED_TIME:.3f} | PlayLength={play_length:.3f} | "
            f"ConfiguredTimeInRange={configured_time_in_range} | StoredTimeReadableFromPython=False"
        )

    ready = len(matches) == 1 and 0.0 <= AUTHORED_NOTIFY_CONFIGURED_TIME <= play_length
    log(f"AUTHORED_ATTACK_NOTIFY_READINESS | Count={len(matches)} | PASS={ready}")
    return ready


def summarize_animation_group(name, candidate_paths, compatible_skeleton):
    loaded = []
    montage_count = 0
    sequence_base_count = 0
    compatible_count = 0
    notify_count = 0

    for path in candidate_paths:
        asset = load_asset(path)
        if asset is None:
            continue

        loaded.append(asset)
        if has_class_name(asset, "AnimMontage"):
            montage_count += 1
        if (
            has_class_name(asset, "AnimSequenceBase")
            or has_class_name(asset, "AnimSequence")
            or has_class_name(asset, "AnimMontage")
        ):
            sequence_base_count += 1

        skeleton = get_asset_skeleton(asset)
        skeleton_name = skeleton.get_name() if skeleton else "None"
        is_compatible = compatible_skeleton is not None and skeleton == compatible_skeleton
        if is_compatible:
            compatible_count += 1

        asset_notify_count = get_anim_notify_count(asset)
        notify_count += asset_notify_count
        log(
            f"{name} | Path={path} | Class={asset.get_class().get_name()} | "
            f"Skeleton={skeleton_name} | CompatibleWithCharacter={is_compatible} | "
            f"NotifyCount={asset_notify_count}"
        )

    log(
        f"{name} SUMMARY | Loaded={len(loaded)} | AnimMontage={montage_count} | "
        f"AnimSequenceBase={sequence_base_count} | Compatible={compatible_count} | "
        f"NotifyCount={notify_count}"
    )

    return {
        "loaded": len(loaded),
        "montage_count": montage_count,
        "sequence_base_count": sequence_base_count,
        "compatible_count": compatible_count,
        "notify_count": notify_count,
    }


def find_character_mesh_skeleton(character_class):
    if character_class is None:
        return None

    try:
        default_object = unreal.get_default_object(character_class)
    except Exception as exc:
        warn(f"Unable to read BP_TopDownCharacter CDO: {exc}")
        return None

    mesh_component = None
    try:
        mesh_component = default_object.get_component_by_class(unreal.SkeletalMeshComponent)
    except Exception:
        mesh_component = None

    if mesh_component is None:
        for prop_name in ("mesh", "Mesh"):
            try:
                candidate = default_object.get_editor_property(prop_name)
                if candidate is not None:
                    mesh_component = candidate
                    break
            except Exception:
                pass

    if mesh_component is None:
        warn("CHARACTER_MESH | INCOMPLETE | Unable to resolve SkeletalMeshComponent from BP_TopDownCharacter CDO")
        return None

    skeletal_mesh = None
    anim_class = None
    try:
        skeletal_mesh = mesh_component.get_editor_property("skeletal_mesh")
    except Exception:
        pass
    try:
        anim_class = mesh_component.get_editor_property("anim_class")
    except Exception:
        pass

    mesh_name = skeletal_mesh.get_name() if skeletal_mesh else "None"
    anim_class_name = anim_class.get_name() if anim_class else "None"
    skeleton = get_asset_skeleton(skeletal_mesh)
    skeleton_name = skeleton.get_name() if skeleton else "None"
    log(
        f"CHARACTER_MESH | SkeletalMesh={mesh_name} | AnimClass={anim_class_name} | "
        f"Skeleton={skeleton_name}"
    )
    return skeleton


def is_character_cdo_compatible(character_class, expected_character_class):
    if character_class is None or expected_character_class is None:
        return False

    try:
        default_object = unreal.get_default_object(character_class)
        return default_object.is_a(expected_character_class)
    except Exception:
        return False


def main():
    character_bp = load_asset(CHARACTER_BLUEPRINT)
    expected_character_class = load_class(CHARACTER_CLASS)
    character_generated_class = get_generated_class(
        character_bp,
        f"{CHARACTER_BLUEPRINT}.{CHARACTER_BLUEPRINT.rsplit('/', 1)[-1]}_C",
    ) if character_bp is not None else None

    character_parent_class = get_blueprint_parent_class(character_bp)
    character_lineage_ok = (
        is_class_child_of(character_generated_class, expected_character_class)
        or is_character_cdo_compatible(character_generated_class, expected_character_class)
        or is_class_child_of(character_parent_class, expected_character_class)
    )
    if character_generated_class is not None:
        log(f"CHARACTER_CLASS_CHAIN | {' -> '.join(get_parent_chain_names(character_generated_class))}")
    log(f"CHARACTER_CLASS | IsMDSProjectCharacter={character_lineage_ok}")

    character_skeleton = find_character_mesh_skeleton(character_generated_class)

    anim_blueprint = load_asset(ANIM_BLUEPRINT)
    anim_blueprint_skeleton = get_asset_skeleton(anim_blueprint)
    anim_blueprint_skeleton_name = anim_blueprint_skeleton.get_name() if anim_blueprint_skeleton else "None"
    anim_blueprint_compatible = character_skeleton is not None and anim_blueprint_skeleton == character_skeleton
    log(
        f"ANIM_BLUEPRINT | Path={ANIM_BLUEPRINT} | Skeleton={anim_blueprint_skeleton_name} | "
        f"CompatibleWithCharacter={anim_blueprint_compatible}"
    )

    mesh_loaded_count = 0
    mesh_compatible_count = 0
    for path in SKELETAL_MESH_CANDIDATES:
        mesh = load_asset(path)
        if mesh is None:
            continue
        mesh_loaded_count += 1
        mesh_skeleton = get_asset_skeleton(mesh)
        mesh_skeleton_name = mesh_skeleton.get_name() if mesh_skeleton else "None"
        mesh_compatible = character_skeleton is not None and mesh_skeleton == character_skeleton
        if mesh_compatible:
            mesh_compatible_count += 1
        log(
            f"SKELETAL_MESH_CANDIDATE | Path={path} | Skeleton={mesh_skeleton_name} | "
            f"CompatibleWithCharacter={mesh_compatible}"
        )

    attack = summarize_animation_group("ATTACK_CANDIDATE", ATTACK_CANDIDATES, character_skeleton)
    hit_react = summarize_animation_group("HIT_REACT_CANDIDATE", HIT_REACT_CANDIDATES, character_skeleton)
    death = summarize_animation_group("DEATH_CANDIDATE", DEATH_CANDIDATES, character_skeleton)

    core_ready = (
        character_bp is not None
        and character_skeleton is not None
        and anim_blueprint is not None
        and anim_blueprint_compatible
        and mesh_loaded_count >= 1
        and mesh_compatible_count >= 1
        and attack["loaded"] >= 1
        and attack["montage_count"] >= 1
        and attack["compatible_count"] >= 1
        and hit_react["loaded"] >= 1
        and hit_react["sequence_base_count"] >= 1
        and hit_react["compatible_count"] >= 1
        and death["loaded"] >= 1
        and death["sequence_base_count"] >= 1
        and death["compatible_count"] >= 1
    )
    notify_ready = verify_authored_attack_notify()

    log(f"CORE_READINESS | PASS={core_ready}")
    log(f"CHARACTER_LINEAGE_READINESS | PASS={character_lineage_ok}")
    log(f"ATTACK_NOTIFY_READINESS | PASS={notify_ready}")

    if core_ready:
        if notify_ready:
            log("PASS | Existing combat animation assets are loadable and skeleton-compatible, including at least one attack notify.")
        else:
            log("PASS_WITH_INCOMPLETE_ITEMS | Existing combat animation assets are loadable and skeleton-compatible.")
            log("INCOMPLETE | No authored attack notify was found/readable in the checked attack candidates.")
        if not character_lineage_ok:
            log("INCOMPLETE | BP_TopDownCharacter lineage could not be proven from the checked Editor Python APIs.")
        return 0

    error("INCOMPLETE | Existing combat animation asset readiness requirements were not fully met.")
    return 2


try:
    sys.exit(main())
except Exception as exc:
    error(f"FAILED | {exc}")
    sys.exit(1)
