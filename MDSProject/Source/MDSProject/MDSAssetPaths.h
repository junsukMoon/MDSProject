#pragma once

#include "CoreMinimal.h"

namespace MDSAssetPaths
{
inline constexpr TCHAR MoveAction[] = TEXT("/Game/TopDown/Input/Actions/IA_Move.IA_Move");
inline constexpr TCHAR DefaultMappingContext[] = TEXT("/Game/TopDown/Input/IMC_Default.IMC_Default");

inline constexpr TCHAR AttackPresentationMontage[] = TEXT("/Game/Characters/Mannequins/Anims/Pistol/MDS_Pistol_Fire_Montage.MDS_Pistol_Fire_Montage");
inline constexpr TCHAR EnemyPresentationMesh[] = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple");
inline constexpr TCHAR EnemyPresentationAnimClass[] = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C");
inline constexpr TCHAR HitReactionAnimation[] = TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_01.MM_HitReact_Front_Lgt_01");
inline constexpr TCHAR DeathAnimation[] = TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01.MM_Death_Front_01");

inline constexpr TCHAR DebugOverlayWidgetClass[] = TEXT("/Game/MDS/UI/WBP_MDSDebugOverlay.WBP_MDSDebugOverlay_C");
inline constexpr TCHAR MatchHUDWidgetClass[] = TEXT("/Game/MDS/UI/WBP_MDSMatchHUD.WBP_MDSMatchHUD_C");
inline constexpr TCHAR ObjectiveWorldWidgetClass[] = TEXT("/Game/MDS/UI/WBP_MDSObjectiveWorldUI.WBP_MDSObjectiveWorldUI_C");
inline constexpr TCHAR EnemyWorldWidgetClass[] = TEXT("/Game/MDS/UI/WBP_MDSEnemyWorldUI.WBP_MDSEnemyWorldUI_C");
}
