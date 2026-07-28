#pragma once

#include <DirectXMath.h>

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/SpriteBatch.h"

/**
 * @file HMI/Graphics/SpriteRenderer.h
 * @brief Rend les entités de l'ECS possédant un `Transform` et un `Sprite`.
 */

namespace core {
class World;
}

namespace hmi {

class Camera2D;
class TextureAtlas;
class TextureCache;

/**
 * @brief Soumet une scène composée au pipeline de dessin, une passe par groupe de texture.
 *
 * Seul endroit du rendu qui reconvertit une `hmi::TextureHandle` en ressource Direct3D : c'est la
 * **frontière** entre la composition (pure, testable sans GPU) et la soumission. Émet un
 * `SpriteBatch::begin/end` par groupe **contigu** de même texture, dans l'ordre de la scène — donc
 * dans l'ordre des calques, que `ComposedScene::sort()` a rendu prioritaire (`EX-REN-043`). Le
 * contrat public de `hmi::SpriteBatch` est strictement inchangé.
 * @param batch      Pipeline de quads texturés (non possédé).
 * @param projection Matrice de projection monde → clip (fournie par la caméra).
 * @param scene      Scène **déjà triée** (`ComposedScene::sort()`).
 */
void submitComposedScene(SpriteBatch& batch, const DirectX::XMFLOAT4X4& projection,
                         const ComposedScene& scene);

/**
 * @brief Textures liables par la composition d'une scène, atlas **et** damier de repli.
 *
 * Point d'assemblage unique, partagé par le jeu et l'éditeur (`LOT-41`) : le damier est résolu
 * **à la demande** auprès du cache, donc créé seulement si un rendu en mode Texture a lieu.
 * @param atlas Atlas du jeu.
 * @param cache Cache de textures, propriétaire du damier partagé.
 * @return Les deux textures et leurs dimensions, prêtes pour `hmi::composeWorldSprites`.
 */
[[nodiscard]] SceneTextures sceneTextures(const TextureAtlas& atlas, TextureCache& cache);

/**
 * @brief Pont ECS → écran : dessine chaque entité affichable, triée par calque puis par texture.
 *
 * À chaque frame, **compose** la scène (`hmi::composeWorldSprites` : parcours des entités
 * `core::Transform` + `core::Sprite`, régions d'atlas, interpolation, culling — logique pure) puis
 * la **soumet** au `SpriteBatch`. Objet de **présentation** (`HMI`) : il **lit** l'ECS sans jamais
 * le muter (`EX-ARCH-012`). Ce n'est pas un `core::ISystem` : le rendu est découplé de la
 * simulation (`EX-REN-021`).
 *
 * La séparation composition/soumission (`LOT-40`) est ce qui rend l'ordre de dessin assertable
 * sans GPU : `lastScene()` expose la liste exacte des primitives de la dernière image
 * (`EX-NFR-004`).
 */
class SpriteRenderer {
public:
    /**
     * @brief Construit le rendu de sprites.
     * @param batch Pipeline de quads texturés utilisé pour dessiner (non possédé).
     * @param atlas Atlas fournissant la texture et ses dimensions (non possédé).
     * @param cache Cache de textures, propriétaire du damier de repli lié en mode Texture
     *              (non possédé, `LOT-41`).
     */
    SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache);

    /**
     * @brief Dessine toutes les entités affichables du monde, vues par la caméra.
     *
     * Seules les entités intersectant le cadrage de la caméra (marge comprise) sont soumises
     * (`EX-NFR-005`) ; une entité écartée reste simulée normalement (`EX-ARCH-012`).
     * @param world  Monde dont on lit les composants `Transform` et `Sprite`.
     * @param camera Caméra fournissant la projection monde → écran et le cadrage visible.
     * @param mode   Mode de rendu courant (`EX-REN-046`) : bascule purement **visuelle**, sans
     *               effet sur la scène ECS ni sur la simulation.
     * @param interpolationAlpha Facteur d'interpolation `[0, 1[` entre le pas de simulation
     *        précédent et le pas courant (`EX-ARCH-031`,
     * `core::FixedTimestep::interpolationAlpha`). Une entité portant un `hmi::PreviousPosition` est
     * dessinée à `lerp(précédente, courante, alpha)` (mouvement lisse) ; une entité sans ce
     * composant (tuiles fixes) est dessinée à sa position courante. `0` reproduit le comportement
     * non interpolé.
     */
    void render(core::World& world, const Camera2D& camera, RenderMode mode,
                float interpolationAlpha);

    /// @return La scène composée à la dernière image (primitives soumises et compteurs).
    [[nodiscard]] const ComposedScene& lastScene() const noexcept {
        return _scene;
    }

private:
    /// Journalise les compteurs de l'image **seulement** s'ils ont changé depuis la précédente :
    /// le volume soumis reste observable (`EX-NFR-005`) sans écrire une ligne par image, ce que
    /// `GraphicsLog` proscrit sur un chemin de dessin.
    void logStatisticsIfChanged();

    SpriteBatch* _batch;         // non possédé
    const TextureAtlas* _atlas;  // non possédé
    TextureCache* _cache;        // non possédé
    ComposedScene _scene;
    SceneStatistics _loggedStatistics;
};

}  // namespace hmi
