#include "HMI/Graphics/TextureAtlas.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/SlopeGeometry.h"
#include "HMI/Graphics/GraphicsLog.h"
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
    static const std::uint32_t palette[] = {
        // Ligne 0
        pack(200, 60, 60, 255), pack(60, 200, 60, 255), pack(60, 60, 200, 255),
        pack(200, 200, 60, 255), pack(0, 0, 0, 255),
        // Ligne 1
        pack(200, 60, 200, 255), pack(60, 200, 200, 255), pack(230, 140, 40, 255),
        pack(140, 40, 230, 255), pack(0, 0, 0, 255),
        // Ligne 2
        pack(120, 120, 120, 255), pack(80, 160, 120, 255), pack(160, 80, 120, 255),
        pack(120, 80, 160, 255), pack(0, 0, 0, 255),
        // Ligne 3
        pack(200, 200, 200, 255), pack(90, 90, 90, 255), pack(0, 0, 0, 255), pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
        // Ligne 4
        pack(0, 0, 0, 255), pack(0, 0, 0, 255), pack(0, 0, 0, 255), pack(0, 0, 0, 255),
        pack(0, 0, 0, 255),
    };
    const int count = static_cast<int>(std::size(palette));
    return palette[((tileIndex % count) + count) % count];
}

// Vrai si value est dans l'intervalle ferme [low, high] (bornes des zones du personnage).
bool inRange(int value, int low, int high) {
    return value >= low && value <= high;
}

// Les douze types a silhouette inclinee/courbe (pente/arrondi convexe ou concave de sol,
// suivables, EX-GP-003/EX-GP-004/EX-GP-007 ; ou de plafond, solides, EX-GP-006/EX-GP-007) dont la
// case d'atlas recoit un masque de forme (triangle/courbe) plutot qu'un carre plein — la seule
// liste a parcourir, TileType n'etant pas enumerable directement en C++.
constexpr core::TileType kSlopeTileTypes[] = {
    core::TileType::SlopeUpRight,     core::TileType::SlopeUpLeft,
    core::TileType::RoundedUpRight,   core::TileType::RoundedUpLeft,
    core::TileType::SlopeDownRight,   core::TileType::SlopeDownLeft,
    core::TileType::RoundedDownRight, core::TileType::RoundedDownLeft,
    core::TileType::ConcaveUpRight,   core::TileType::ConcaveUpLeft,
    core::TileType::ConcaveDownRight, core::TileType::ConcaveDownLeft,
};

// Type de tuile a silhouette inclinee/courbe dont la case d'atlas est (tileColumn, tileRow), s'il
// y en a un.
std::optional<core::TileType> slopeTypeAtGridPosition(int tileColumn, int tileRow) {
    for (const core::TileType type : kSlopeTileTypes) {
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
    // Échantillonne au bord des pixels de coin (0 et TILE_SIZE-1 atteignent exactement 0,0/1,0),
    // pas à leur centre (`(localX+0.5)/TILE_SIZE` ne dépasse jamais 0,969 pour le dernier pixel) :
    // sans ça, la colonne de pixels la plus proche du bord « plein » d'un arrondi CONCAVE (tangente
    // quasi verticale à cet endroit précis, EX-GP-007) manque la vraie valeur de bord par une marge
    // significative (~0,25 de hauteur de case) — visible comme une encoche grossière juste là où la
    // silhouette doit au contraire être quasiment pleine. Négligeable pour les pentes/arrondis
    // convexes existants (tangente raide du côté CREUX plutôt que du côté plein, donc jamais à un
    // bord critique pour le raccord entre deux cases).
    constexpr float LAST_INDEX = static_cast<float>(TextureAtlas::TILE_SIZE - 1);
    const float normalizedX = static_cast<float>(localX) / LAST_INDEX;
    const float normalizedY = static_cast<float>(localY) / LAST_INDEX;
    const bool isCeiling = core::isCeilingSlope(type);
    const std::optional<float> surfaceHeight = isCeiling
                                                    ? core::ceilingSlopeHeight(type, normalizedX)
                                                    : core::slopeSurfaceHeight(type, normalizedX);
    if (!surfaceHeight) {
        return pack(0, 0, 0, 0);
    }
    const bool solid =
        isCeiling ? normalizedY <= *surfaceHeight : normalizedY >= *surfaceHeight;
    if (solid) {
        // Meme case que Solid (colonne 0, ligne 2 — TileVisuals.cpp::regionForTile) : gris,
        // reutilise ici comme indice de palette plutot que la couleur propre de `type`.
        return tileColor(2 * TextureAtlas::TILES_PER_SIDE);
    }
    return pack(0, 0, 0, 0);  // du cote vide de la silhouette : transparent
}

// Largeur des bras : ecartes du corps ou resserres (variation de pose entre images d'un meme
// clip, LOT-18).
enum class ArmPose { Wide, Tucked };

// Position des jambes : neutre, ecartee (course), ou resserree/raccourcie (saut, jambes
// repliees en un seul bloc, pieds ne touchant pas la ligne du bas).
enum class LegPose { Neutral, Apart, Tucked };

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
        const bool armsShown = (arms == ArmPose::Wide) ? inRange(x, 2, 13) : inRange(x, 3, 12);
        return armsShown ? shirt : transparent;
    }
    if (y == 7) {
        const bool hand = (arms == ArmPose::Wide) ? (inRange(x, 2, 3) || inRange(x, 12, 13))
                                                   : (x == 3 || x == 12);
        if (hand) {
            return skin;
        }
        return inRange(x, 4, 11) ? shirt : transparent;
    }
    // Jambes (lignes 10-15) : pantalon puis chaussures, separees par un espace transparent.
    // Tucked (saut) est plus court (pieds repliees) et forme un seul bloc central.
    if (legs == LegPose::Tucked) {
        if (inRange(y, 10, 12)) {
            return inRange(x, 6, 9) ? pants : transparent;
        }
        if (y == 13) {
            return inRange(x, 6, 9) ? shoes : transparent;
        }
        return transparent;
    }
    const int leftMin = (legs == LegPose::Apart) ? 4 : 5;
    const int leftMax = (legs == LegPose::Apart) ? 6 : 7;
    const int rightMin = (legs == LegPose::Apart) ? 10 : 9;
    const int rightMax = (legs == LegPose::Apart) ? 12 : 11;
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

// Index a plat (0-based) d'une image dans la grille : Idle (0..IDLE_FRAME_COUNT-1), puis Run,
// puis Jump — seule source de vérité de l'ordre, partagée par la génération et par
// TextureAtlas::playerFrameRegion.
int flatFrameIndex(core::AnimationClip clip, int frameIndex) {
    switch (clip) {
        case core::AnimationClip::Idle:
            return frameIndex;
        case core::AnimationClip::Run:
            return core::IDLE_FRAME_COUNT + frameIndex;
        case core::AnimationClip::Jump:
            return core::IDLE_FRAME_COUNT + core::RUN_FRAME_COUNT + frameIndex;
    }
    return 0;
}

Pose poseFor(core::AnimationClip clip, int frameIndex) {
    switch (clip) {
        case core::AnimationClip::Idle:
            // Image 0 : bras relaches. Image 1 : legerement resserres (respiration/attente).
            return (frameIndex == 0) ? Pose{ArmPose::Wide, LegPose::Neutral}
                                     : Pose{ArmPose::Tucked, LegPose::Neutral};
        case core::AnimationClip::Run:
            // Alterne jambes ecartees (phase basse, bras relaches) et jambes neutres (phase
            // haute, bras resserres) : deux poses distinctes suffisent a lire un cycle de course.
            return (frameIndex % 2 == 0) ? Pose{ArmPose::Wide, LegPose::Apart}
                                         : Pose{ArmPose::Tucked, LegPose::Neutral};
        case core::AnimationClip::Jump:
            return Pose{ArmPose::Wide, LegPose::Tucked};
    }
    return Pose{ArmPose::Wide, LegPose::Neutral};
}
}  // namespace

// Génère l'atlas procédural et crée la ressource Direct3D associée.
TextureAtlas::TextureAtlas(ID3D11Device* device) {
    const int gridSide = TILE_SIZE * TILES_PER_SIDE;
    // La grille d'images du personnage est ajoutee sous la grille de tuiles, dans la meme
    // texture (le rendu ne dessine qu'une seule texture par passe, cf. SpriteRenderer). Le
    // nombre total d'images (Idle + Run + Jump) determine le nombre de lignes necessaires.
    const int totalFrames = core::IDLE_FRAME_COUNT + core::RUN_FRAME_COUNT + core::JUMP_FRAME_COUNT;
    const int frameRows = (totalFrames + PLAYER_FRAME_COLUMNS - 1) / PLAYER_FRAME_COLUMNS;
    const int framesTop = gridSide;
    _width = gridSide;
    _height = gridSide + frameRows * PLAYER_FRAME_SIZE;
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(_width) *
                                      static_cast<std::size_t>(_height),
                                      pack(0, 0, 0, 0));

    // Dernière tuile réservée au test de transparence : damier opaque / transparent.
    const int transparentTileIndex = TILES_PER_SIDE * TILES_PER_SIDE - 1;

    for (int y = 0; y < gridSide; ++y) {
        for (int x = 0; x < _width; ++x) {
            const int tileColumn = x / TILE_SIZE;
            const int tileRow = y / TILE_SIZE;
            const int tileIndex = tileRow * TILES_PER_SIDE + tileColumn;

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
                color = slopeShapePixel(*slopeType, x % TILE_SIZE, y % TILE_SIZE);
            }
            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_width) +
                   static_cast<std::size_t>(x)] = color;
        }
    }

    // Chaque image (clip, index) occupe un bloc 16x16 de la grille sous les tuiles, dans
    // l'ordre Idle, Run, Jump (celui de core::AnimationClip — un seul ordre, partage avec
    // TextureAtlas::playerFrameRegion, pas de table de correspondance dupliquee).
    auto paintFrame = [&](core::AnimationClip clip, int frameIndex) {
        const Pose pose = poseFor(clip, frameIndex);
        const int flatIndex = flatFrameIndex(clip, frameIndex);
        const int column = flatIndex % PLAYER_FRAME_COLUMNS;
        const int row = flatIndex / PLAYER_FRAME_COLUMNS;
        const int originX = column * PLAYER_FRAME_SIZE;
        const int originY = framesTop + row * PLAYER_FRAME_SIZE;
        for (int localY = 0; localY < PLAYER_FRAME_SIZE; ++localY) {
            for (int localX = 0; localX < PLAYER_FRAME_SIZE; ++localX) {
                const std::uint32_t color = playerPixel(localX, localY, pose.arms, pose.legs);
                const int x = originX + localX;
                const int y = originY + localY;
                pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_width) +
                       static_cast<std::size_t>(x)] = color;
            }
        }
    };
    for (int frame = 0; frame < core::IDLE_FRAME_COUNT; ++frame) {
        paintFrame(core::AnimationClip::Idle, frame);
    }
    for (int frame = 0; frame < core::RUN_FRAME_COUNT; ++frame) {
        paintFrame(core::AnimationClip::Run, frame);
    }
    for (int frame = 0; frame < core::JUMP_FRAME_COUNT; ++frame) {
        paintFrame(core::AnimationClip::Jump, frame);
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(_width);
    description.Height = static_cast<UINT>(_height);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(_width) * sizeof(std::uint32_t);

    if (FAILED(device->CreateTexture2D(&description, &data, &_texture))) {
        throw std::runtime_error("Echec de creation de la texture d'atlas");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de la texture d'atlas");
    }
    GRAPHICS_LOG_TRACE("TextureAtlas : atlas procedural genere");
}

// Région (en pixels) de la tuile à une position de la grille.
// La région d'atlas correspondante, en pixels.
core::AtlasRegion TextureAtlas::tile(int column, int row) const {
    return core::AtlasRegion{column * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
}

// Région (en pixels) d'une image d'animation du personnage, sous la grille de tuiles.
core::AtlasRegion TextureAtlas::playerFrameRegion(core::AnimationClip clip, int frameIndex) const {
    const int flatIndex = flatFrameIndex(clip, frameIndex);
    const int column = flatIndex % PLAYER_FRAME_COLUMNS;
    const int row = flatIndex / PLAYER_FRAME_COLUMNS;
    return core::AtlasRegion{column * PLAYER_FRAME_SIZE,
                             TILE_SIZE * TILES_PER_SIDE + row * PLAYER_FRAME_SIZE,
                             PLAYER_FRAME_SIZE, PLAYER_FRAME_SIZE};
}

}  // namespace hmi
