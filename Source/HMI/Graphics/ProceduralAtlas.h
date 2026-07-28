#pragma once

#include <cstdint>
#include <vector>

#include "Core/Ecs/Components/Animation.h"

/**
 * @file HMI/Graphics/ProceduralAtlas.h
 * @brief Génération procédurale (CPU, sans GPU) des pixels de l'atlas de repli.
 */

namespace hmi {

/**
 * @brief Index à plat (0-based) d'une image dans la grille sous les tuiles : `Idle`
 *        (0..IDLE_FRAME_COUNT-1), puis `Run`, puis `Jump`.
 *
 * **Seule source de vérité** de cet ordre, partagée par `buildProceduralAtlasImage` (placement
 * des images) et par `TextureAtlas::playerFrameRegion` (résolution de la même région) — les
 * dupliquer aux deux endroits risquerait de les laisser diverger silencieusement.
 * @param clip       Clip d'animation (`EX-REN-012`).
 * @param frameIndex Index de l'image dans le clip (0-based).
 * @return L'index à plat de cette image dans la grille.
 */
[[nodiscard]] int flatPlayerFrameIndex(core::AnimationClip clip, int frameIndex);

/// Pixels d'un atlas généré en mémoire, au format `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
struct ProceduralAtlasImage {
    int width = 0;
    int height = 0;
    /// Taille `width * height`, ligne par ligne (haut en bas), pixel `pack(r,g,b,a)`.
    std::vector<std::uint32_t> pixels;
};

/**
 * @brief Génère, en mémoire, l'atlas procédural historique (grille de tuiles + images du
 *        personnage), de façon **déterministe**.
 *
 * Logique pure (`EX-NFR-010`), sans dépendance Direct3D/Qt : utilisée à la fois par
 * `hmi::TextureAtlas` (repli si l'asset fichier est absent/illisible, `EX-NFR-040`) et par l'export
 * ponctuel de l'atlas de base en fichier (`--export-atlas`, cf. `main.cpp`), pour que les deux
 * chemins restent l'unique source de vérité de ce contenu.
 * @return L'image générée (dimensions et pixels).
 */
[[nodiscard]] ProceduralAtlasImage buildProceduralAtlasImage();

}  // namespace hmi
