#pragma once

#include <cstdint>
#include <vector>

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

/**
 * @brief Pont ECS → écran : dessine chaque entité affichable, triée par couche.
 *
 * À chaque frame, parcourt les entités possédant un `core::Transform` **et** un
 * `core::Sprite`, résout la région d'atlas, construit le quad en unités monde et l'empile
 * dans le `SpriteBatch` en appliquant la projection de la caméra. Objet de **présentation**
 * (`HMI`) : il **lit** l'ECS sans jamais le muter (`EX-ARCH-012`). Ce n'est pas un
 * `core::ISystem` : le rendu est découplé de la simulation (`EX-REN-021`).
 */
class SpriteRenderer {
public:
    /**
     * @brief Construit le rendu de sprites.
     * @param batch Pipeline de quads texturés utilisé pour dessiner (non possédé).
     * @param atlas Atlas fournissant la texture et ses dimensions (non possédé).
     */
    SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas);

    /**
     * @brief Dessine toutes les entités affichables du monde, vues par la caméra.
     * @param world  Monde dont on lit les composants `Transform` et `Sprite`.
     * @param camera Caméra fournissant la projection monde → écran.
     */
    void render(core::World& world, const Camera2D& camera);

private:
    /// Un quad prêt à dessiner, associé à sa couche (pour le tri).
    struct LayeredQuad {
        std::int32_t layer;
        SpriteQuad quad;
    };

    SpriteBatch* _batch;         // non possédé
    const TextureAtlas* _atlas;  // non possédé
    std::vector<LayeredQuad> _items;
};

}  // namespace hmi
