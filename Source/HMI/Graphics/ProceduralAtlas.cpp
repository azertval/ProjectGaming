#include "HMI/Graphics/ProceduralAtlas.h"

#include <array>
#include <optional>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/SlopeMask.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {
// Assemble une couleur RVBA (octets) en un pixel `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

// Couleur opaque de base d'une tuile, selon son index dans la grille (déterministe). Rangée
// (`TextureAtlas::TILES_PER_SIDE` par ligne, actuellement `5`) : les quatre premières colonnes de
// chaque ligne reprennent **exactement** les couleurs historiques (`TILES_PER_SIDE == 4`, avant
// l'ajout des pentes/arrondis de plafond, `EX-GP-006`) — un simple agrandissement de la grille ne
// doit jamais redécaler silencieusement les couleurs des tuiles existantes. Les cases au-delà de
// la colonne 4 (ou de la ligne 4) sont soit réservées (damier de transparence, dernière case),
// soit remplacées par un masque de forme (`slopeShapePixel`) : leur couleur de base ici n'a pas
// d'importance, remplie de noir par convention.
std::uint32_t tileColor(int tileIndex) {
    static const std::array<std::uint32_t, 25> palette{
        // Ligne 0
        pack(200, 60, 60, 255),
        pack(60, 200, 60, 255),
        pack(60, 60, 200, 255),
        pack(200, 200, 60, 255),
        pack(0, 0, 0, 255),
        // Ligne 1
        pack(200, 60, 200, 255),
        pack(60, 200, 200, 255),
        pack(230, 140, 40, 255),
        pack(140, 40, 230, 255),
        pack(0, 0, 0, 255),
        // Ligne 2
        pack(120, 120, 120, 255),
        pack(80, 160, 120, 255),
        pack(160, 80, 120, 255),
        pack(120, 80, 160, 255),
        pack(0, 0, 0, 255),
        // Ligne 3
        pack(200, 200, 200, 255),
        pack(90, 90, 90, 255),
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        // Ligne 4
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
    };
    const int count = static_cast<int>(std::size(palette));
    return palette[((tileIndex % count) + count) % count];
}

// Vrai si value est dans l'intervalle ferme [low, high] (bornes des zones du personnage).
bool inRange(int value, int low, int high) {
    return value >= low && value <= high;
}

// Type de tuile a silhouette inclinee/courbe dont la case d'atlas est (tileColumn, tileRow), s'il
// y en a un. La liste des douze types vit dans SlopeMask.h, avec la silhouette elle-meme.
std::optional<core::TileType> slopeTypeAtGridPosition(int tileColumn, int tileRow) {
    for (const core::TileType type : SILHOUETTE_TILE_TYPES) {
        const std::optional<hmi::AtlasGridPosition> position = hmi::slopeTileGridPosition(type);
        if (position && position->column == tileColumn && position->row == tileRow) {
            return type;
        }
    }
    return std::nullopt;
}

// Couleur du pixel (localX, localY) d'une case a silhouette inclinee/courbe, 0-based dans la case
// 16x16 : plein (gris, meme couleur que Solid — ces tuiles restent un materiau de plateforme comme
// un autre, pas une famille de couleurs distinctes) d'un cote de la surface, transparent de
// l'autre — l'affichage reproduit ainsi la silhouette reelle plutot qu'un carre plein qui la
// masquerait.
//
// Pente/arrondi de SOL (EX-GP-003/EX-GP-004) : plein SOUS la surface suivie par la physique
// (core::slopeSurfaceHeight). Pente/arrondi de PLAFOND (EX-GP-006) : plein AU-DESSUS de la
// silhouette (core::ceilingSlopeHeight, miroir vertical de la variante de sol) — dans les deux
// cas, l'affichage correspond exactement a la hitbox reelle (core::resolveSlopeFollow /
// core::resolveCeilingSlopeFollow, pas de solidite statique via core::isSolid).
std::uint32_t slopeShapePixel(core::TileType type, int localX, int localY) {
    // Appartenance a la matiere deleguee a hmi::isInsideSilhouette (LOT-42), point de verite unique
    // partage avec le detourage des skins : deux implementations de la meme silhouette finiraient
    // par diverger, et un skin decoupe autrement que l'atlas se verrait immediatement a la bascule
    // Physique/Texture.
    if (!isInsideSilhouette(type, localX, localY, TextureAtlas::TILE_SIZE)) {
        return pack(0, 0, 0, 0);  // du cote vide de la silhouette : transparent
    }
    // Meme case que Solid (colonne 0, ligne 2 — TileVisuals.cpp::regionForTile) : gris, reutilise
    // ici comme indice de palette plutot que la couleur propre de `type`.
    return tileColor(2 * TextureAtlas::TILES_PER_SIDE);
}

// Largeur des bras : ecartes du corps ou resserres (variation de pose entre images d'un meme
// clip, LOT-18).
enum class ArmPose { WIDE, TUCKED };

// Position des jambes : neutre, ecartee (course), ou resserree/raccourcie (saut, jambes
// repliees en un seul bloc, pieds ne touchant pas la ligne du bas).
enum class LegPose { NEUTRAL, APART, TUCKED };

// Couleur du pixel (x, y) de la silhouette du personnage pour une pose donnee, (0,0) = coin
// haut-gauche de la region 16x16 (EX-REN-011). Silhouette humanoide par blocs rectangulaires :
// cheveux, peau, chemise/manches, mains, pantalon, chaussures. Transparent hors silhouette.
//
// La region est CARREE (16x16), comme une tuile : le rendu (SpriteRenderer) multiplie ses
// dimensions par Transform::scale, qui vaut core::playerSize() (0,4 x 0,8, cf. GameSession) —
// c'est ce facteur d'echelle, deja non uniforme, qui donne au personnage sa silhouette deux
// fois plus haute que large a l'ecran. Une region deja non carree doublerait cet effet (bug de
// LOT-17, corrige) : chaque image est donc dessinee compressee de moitie en hauteur ici, pour
// retrouver ses proportions une fois etiree par l'echelle.
std::uint32_t playerPixel(int x, int y, ArmPose arms, LegPose legs) {
    const std::uint32_t hair = pack(90, 60, 40, 255);
    const std::uint32_t skin = pack(230, 190, 150, 255);
    const std::uint32_t shirt = pack(50, 110, 200, 255);
    const std::uint32_t pants = pack(60, 60, 70, 255);
    const std::uint32_t shoes = pack(30, 30, 35, 255);
    const std::uint32_t transparent = pack(0, 0, 0, 0);

    // Tete (lignes 0-3) : cheveux, puis peau avec cheveux sur les cotes, puis nuque. Fixe :
    // aucune pose ne fait bouger la tete (LOT-18 se limite aux bras/jambes).
    if (y == 0) {
        return inRange(x, 5, 10) ? hair : transparent;
    }
    if (y == 1) {
        if (x == 5 || x == 10) {
            return hair;
        }
        return inRange(x, 6, 9) ? skin : transparent;
    }
    if (y == 2) {
        return inRange(x, 5, 10) ? skin : transparent;
    }
    if (y == 3) {
        return inRange(x, 6, 9) ? skin : transparent;
    }
    // Torse (lignes 4-9) : epaules (largeur fixe), puis bras+torse (largeur selon ArmPose),
    // mains aux extremites des bras, puis torse seul.
    if (y == 4 || inRange(y, 8, 9)) {
        return inRange(x, 4, 11) ? shirt : transparent;
    }
    if (inRange(y, 5, 6)) {
        const bool armsShown = (arms == ArmPose::WIDE) ? inRange(x, 2, 13) : inRange(x, 3, 12);
        return armsShown ? shirt : transparent;
    }
    if (y == 7) {
        const bool hand = (arms == ArmPose::WIDE) ? (inRange(x, 2, 3) || inRange(x, 12, 13))
                                                  : (x == 3 || x == 12);
        if (hand) {
            return skin;
        }
        return inRange(x, 4, 11) ? shirt : transparent;
    }
    // Jambes (lignes 10-15) : pantalon puis chaussures, separees par un espace transparent.
    // Tucked (saut) est plus court (pieds repliees) et forme un seul bloc central.
    if (legs == LegPose::TUCKED) {
        if (inRange(y, 10, 12)) {
            return inRange(x, 6, 9) ? pants : transparent;
        }
        if (y == 13) {
            return inRange(x, 6, 9) ? shoes : transparent;
        }
        return transparent;
    }
    const int leftMin = (legs == LegPose::APART) ? 4 : 5;
    const int leftMax = (legs == LegPose::APART) ? 6 : 7;
    const int rightMin = (legs == LegPose::APART) ? 10 : 9;
    const int rightMax = (legs == LegPose::APART) ? 12 : 11;
    const bool onLeg = inRange(x, leftMin, leftMax) || inRange(x, rightMin, rightMax);
    if (inRange(y, 10, 12)) {
        return onLeg ? pants : transparent;
    }
    if (inRange(y, 13, 15)) {
        return onLeg ? shoes : transparent;
    }
    return transparent;
}

// Pose (bras, jambes) d'une image donnee d'un clip. L'ordre des images dans la grille de
// l'atlas (2 Idle, 4 Run, 1 Jump) suit celui de core::AnimationClip (voir epic LOT-18).
struct Pose {
    ArmPose arms;
    LegPose legs;
};

Pose poseFor(PlayerClipKind clip, int frameIndex) {
    switch (clip) {
        case PlayerClipKind::Idle:
            // Image 0 : bras relaches. Image 1 : legerement resserres (respiration/attente).
            return (frameIndex == 0) ? Pose{.arms = ArmPose::WIDE, .legs = LegPose::NEUTRAL}
                                     : Pose{.arms = ArmPose::TUCKED, .legs = LegPose::NEUTRAL};
        case PlayerClipKind::Run:
            // Alterne jambes ecartees (phase basse, bras relaches) et jambes neutres (phase
            // haute, bras resserres) : deux poses distinctes suffisent a lire un cycle de course.
            return (frameIndex % 2 == 0) ? Pose{.arms = ArmPose::WIDE, .legs = LegPose::APART}
                                         : Pose{.arms = ArmPose::TUCKED, .legs = LegPose::NEUTRAL};
        case PlayerClipKind::Jump:
            return Pose{.arms = ArmPose::WIDE, .legs = LegPose::TUCKED};
    }
    return Pose{.arms = ArmPose::WIDE, .legs = LegPose::NEUTRAL};
}
}  // namespace

// Index à plat (0-based) d'une image dans la grille sous les tuiles.
int flatPlayerFrameIndex(PlayerClipKind clip, int frameIndex) {
    switch (clip) {
        case PlayerClipKind::Idle:
            return frameIndex;
        case PlayerClipKind::Run:
            return PLAYER_IDLE_FRAME_COUNT + frameIndex;
        case PlayerClipKind::Jump:
            return PLAYER_IDLE_FRAME_COUNT + PLAYER_RUN_FRAME_COUNT + frameIndex;
    }
    return 0;
}

// Génère, en mémoire, l'atlas procédural historique.
ProceduralAtlasImage buildProceduralAtlasImage() {
    const int gridSide = TextureAtlas::TILE_SIZE * TextureAtlas::TILES_PER_SIDE;
    // La grille d'images du personnage est ajoutee sous la grille de tuiles, dans la meme
    // texture (le rendu ne dessine qu'une seule texture par passe, cf. SpriteRenderer). Le
    // nombre total d'images (Idle + Run + Jump) determine le nombre de lignes necessaires.
    const int totalFrames =
        PLAYER_IDLE_FRAME_COUNT + PLAYER_RUN_FRAME_COUNT + PLAYER_JUMP_FRAME_COUNT;
    const int frameRows =
        (totalFrames + TextureAtlas::PLAYER_FRAME_COLUMNS - 1) / TextureAtlas::PLAYER_FRAME_COLUMNS;
    const int framesTop = gridSide;

    ProceduralAtlasImage image;
    image.width = gridSide;
    image.height = gridSide + (frameRows * TextureAtlas::PLAYER_FRAME_SIZE);
    image.pixels.assign(
        static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height),
        pack(0, 0, 0, 0));

    // Dernière tuile réservée au test de transparence : damier opaque / transparent.
    const int transparentTileIndex =
        (TextureAtlas::TILES_PER_SIDE * TextureAtlas::TILES_PER_SIDE) - 1;

    for (int y = 0; y < gridSide; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const int tileColumn = x / TextureAtlas::TILE_SIZE;
            const int tileRow = y / TextureAtlas::TILE_SIZE;
            const int tileIndex = (tileRow * TextureAtlas::TILES_PER_SIDE) + tileColumn;

            std::uint32_t color = tileColor(tileIndex);
            if (tileIndex == transparentTileIndex) {
                // Un damier 4×4 pixels : une case sur deux est entièrement transparente.
                const bool transparent = (((x / 4) + (y / 4)) % 2) == 0;
                color = transparent ? pack(0, 0, 0, 0) : pack(240, 240, 240, 255);
            } else if (const std::optional<core::TileType> slopeType =
                           slopeTypeAtGridPosition(tileColumn, tileRow)) {
                // Pente/arrondi (EX-GP-003/EX-GP-004) : masque de forme (gris, comme Solid)
                // plutot qu'un carre plein d'une couleur distincte, pour que l'affichage
                // corresponde a la hitbox reelle (core::slopeSurfaceHeight).
                color = slopeShapePixel(*slopeType, x % TextureAtlas::TILE_SIZE,
                                        y % TextureAtlas::TILE_SIZE);
            }
            image.pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width)) +
                         static_cast<std::size_t>(x)] = color;
        }
    }

    // Chaque image (clip, index) occupe un bloc 16x16 de la grille sous les tuiles, dans
    // l'ordre Idle, Run, Jump (celui de hmi::PlayerClipKind — un seul ordre, partage avec
    // TextureAtlas::playerFrameRegion, pas de table de correspondance dupliquee).
    auto paintFrame = [&](PlayerClipKind clip, int frameIndex) {
        const Pose pose = poseFor(clip, frameIndex);
        const int flatIndex = flatPlayerFrameIndex(clip, frameIndex);
        const int column = flatIndex % TextureAtlas::PLAYER_FRAME_COLUMNS;
        const int row = flatIndex / TextureAtlas::PLAYER_FRAME_COLUMNS;
        const int originX = column * TextureAtlas::PLAYER_FRAME_SIZE;
        const int originY = framesTop + (row * TextureAtlas::PLAYER_FRAME_SIZE);
        for (int localY = 0; localY < TextureAtlas::PLAYER_FRAME_SIZE; ++localY) {
            for (int localX = 0; localX < TextureAtlas::PLAYER_FRAME_SIZE; ++localX) {
                const std::uint32_t color = playerPixel(localX, localY, pose.arms, pose.legs);
                const int x = originX + localX;
                const int y = originY + localY;
                image.pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width)) +
                             static_cast<std::size_t>(x)] = color;
            }
        }
    };
    for (int frame = 0; frame < PLAYER_IDLE_FRAME_COUNT; ++frame) {
        paintFrame(PlayerClipKind::Idle, frame);
    }
    for (int frame = 0; frame < PLAYER_RUN_FRAME_COUNT; ++frame) {
        paintFrame(PlayerClipKind::Run, frame);
    }
    for (int frame = 0; frame < PLAYER_JUMP_FRAME_COUNT; ++frame) {
        paintFrame(PlayerClipKind::Jump, frame);
    }

    return image;
}

}  // namespace hmi
