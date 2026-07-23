import sys

import unreal

unreal.load_module("AnimationBlueprintLibrary")
ANIMATION_LIBRARY = getattr(unreal, "AnimationBlueprintLibrary", None) or getattr(unreal, "AnimationLibrary", None)


ATTACK_MONTAGE = "/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage"
NOTIFY_CLASS = "/Script/MDSProject.MDSCombatTimingAnimNotify"
NOTIFY_TIME_SECONDS = 0.1


def log(message):
    unreal.log(f"MDSConfigureCombatNotify: {message}")


def fail(message):
    unreal.log_error(f"MDSConfigureCombatNotify: {message}")
    raise RuntimeError(message)


def get_notify_object(event):
    try:
        return event.get_editor_property("notify")
    except Exception:
        return None


def get_matching_notifies(montage, notify_class):
    try:
        events = ANIMATION_LIBRARY.get_animation_notify_events(montage)
    except Exception as exc:
        fail(f"Unable to read montage notifies: {exc}")

    matches = []
    for event in events:
        notify = get_notify_object(event)
        if notify is not None and notify.get_class().get_name() == notify_class.get_name():
            matches.append(notify)
    return matches


def main():
    if ANIMATION_LIBRARY is None:
        candidates = [name for name in dir(unreal) if "Animation" in name and "Library" in name]
        fail(f"Animation notify library is unavailable. Candidates={candidates}")

    montage = unreal.EditorAssetLibrary.load_asset(ATTACK_MONTAGE)
    if montage is None:
        fail(f"Attack montage not found: {ATTACK_MONTAGE}")

    notify_class = unreal.load_class(None, NOTIFY_CLASS)
    if notify_class is None:
        fail(f"Notify class not found: {NOTIFY_CLASS}")

    matches = get_matching_notifies(montage, notify_class)
    if len(matches) > 1:
        fail(f"Duplicate authored combat timing notifies found: {len(matches)}")

    if not matches:
        track_names = ANIMATION_LIBRARY.get_animation_notify_track_names(montage)
        if not track_names:
            fail("Attack montage has no notify track")

        created = ANIMATION_LIBRARY.add_animation_notify_event(
            montage,
            track_names[0],
            NOTIFY_TIME_SECONDS,
            notify_class,
        )
        if created is None:
            fail("AnimationBlueprintLibrary did not create the notify")

        if not unreal.EditorAssetLibrary.save_loaded_asset(montage, only_if_is_dirty=False):
            fail("Failed to save the attack montage")
        log(f"CREATED | Time={NOTIFY_TIME_SECONDS:.3f} | Track={track_names[0]}")
    else:
        log("EXISTING | Count=1 | No duplicate added")

    reloaded = unreal.EditorAssetLibrary.load_asset(ATTACK_MONTAGE)
    reloaded_matches = get_matching_notifies(reloaded, notify_class)
    if len(reloaded_matches) != 1:
        fail(f"Persistent notify count after save is {len(reloaded_matches)}, expected 1")

    play_length = ANIMATION_LIBRARY.get_sequence_length(reloaded)
    if NOTIFY_TIME_SECONDS < 0.0 or NOTIFY_TIME_SECONDS > play_length:
        fail(f"Notify time {NOTIFY_TIME_SECONDS:.3f} is outside montage length {play_length:.3f}")

    log(
        f"PASS | Montage={ATTACK_MONTAGE} | NotifyClass={NOTIFY_CLASS} | "
        f"Count=1 | ConfiguredTime={NOTIFY_TIME_SECONDS:.3f} | PlayLength={play_length:.3f}"
    )
    return 0


try:
    sys.exit(main())
except Exception as exc:
    unreal.log_error(f"MDSConfigureCombatNotify: FAILED | {exc}")
    sys.exit(1)
