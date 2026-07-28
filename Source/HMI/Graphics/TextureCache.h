#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <d3d11.h>

#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Graphics/TextureCache.h
 * @brief Registre de textures chargées à la demande par **nom logique** (`EX-REN-043`).
 */

namespace hmi {

/**
 * @brief Charge, valide et met en cache des textures Direct3D 11 désignées par leur nom de
 *        fichier logique.
 *
 * `hmi::TextureAtlas` ne connaît qu'**une** texture fixe ; le programme d'habillage (`LOT-42` →
 * `LOT-55`) a besoin d'un nombre variable de textures indépendantes (skins, fonds, objets, décors,
 * spritesheets). Ce registre les charge au **premier accès**, les valide contre le contrat de leur
 * famille (`EX-REN-007`) et les conserve : les accès suivants renvoient la même ressource, sans
 * relire le disque.
 *
 * Ne réimplémente ni le décodage ni l'upload : il compose `hmi::AssetPaths::resolve` et
 * `hmi::loadTextureFromFile` (`LOT-39`), exactement comme `hmi::TextureAtlas`. Toutes les
 * ressources Direct3D sont détenues en RAII (`Microsoft::WRL::ComPtr`) et libérées à la
 * destruction du cache (`EX-NFR-041`).
 *
 * Un asset absent, illisible ou non conforme est un cas **attendu** (`EX-NFR-040`) : `get`
 * renvoie `nullptr` sans exception, après un avertissement journalisé **une seule fois** par nom
 * — le résultat négatif est mémorisé pour ne pas retenter une lecture disque à chaque image, et
 * pour ne pas noyer le journal. Les appelants qui veulent une texture *quoi qu'il arrive* passent
 * par `hmi::resolveOrPlaceholder`, point de résolution unique du repli en damier (`LOT-40`
 * TACHE-03).
 *
 * **Aucune éviction** : le cache grossit avec les assets rencontrés et n'est jamais purgé de
 * lui-même (décision de cadrage `LOT-40` — les assets sont bornés). `invalidate` sert au
 * **rechargement** (à chaud, `LOT-43` ; aperçu de l'atelier, `LOT-54`), pas à une politique
 * mémoire.
 *
 * **Durée de vie des handles** : `get` renvoie un pointeur vers une entrée du cache, valide tant
 * que l'entrée n'est pas invalidée. `invalidate`/`invalidateAll` **détruisent** l'entrée, donc les
 * ressources Direct3D si personne d'autre ne les retient : ils ne doivent être appelés
 * qu'**entre deux images**, jamais pendant la composition d'une image. Un appelant qui souhaite
 * conserver une texture au-delà d'une image en copie le `ComPtr` — c'est le contrat explicite,
 * pas une invalidation différée cachée.
 */
class TextureCache {
public:
    /**
     * @brief Construit un cache vide pour un device et un dossier d'assets donnés.
     * @param device Device Direct3D 11 utilisé pour créer les textures (non possédé, doit
     *               survivre au cache).
     * @param paths  Résolveur de chemins d'assets (copié : le cache en garde sa propre copie).
     */
    TextureCache(ID3D11Device* device, AssetPaths paths);

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    /**
     * @brief Obtient la texture d'un asset, en la chargeant au premier accès.
     * @param fileName Nom logique du fichier, relatif au dossier d'assets (ex. « forest.png »).
     * @param family   Famille de l'asset, qui fixe les dimensions attendues (`EX-REN-007`).
     * @return La texture chargée (propriété du cache), ou `nullptr` si l'asset est absent,
     *         illisible ou non conforme — jamais d'exception (`EX-NFR-040`).
     */
    [[nodiscard]] const LoadedTexture* get(const std::string& fileName, AssetFamily family);

    /**
     * @brief Retire une entrée du cache, de sorte que le prochain `get` relise le fichier.
     *
     * Retire aussi un éventuel **échec** mémorisé : c'est ce qui permet à un asset créé après coup
     * d'être pris en compte sans redémarrer (rechargement à chaud, `LOT-43`).
     * @param fileName Nom logique du fichier à oublier (sans effet s'il n'est pas en cache).
     */
    void invalidate(const std::string& fileName);

    /// Retire **toutes** les entrées du cache (rechargement global, `LOT-43`/`LOT-54`).
    void invalidateAll();

    /// @return Le nombre d'entrées mémorisées (succès **et** échecs), pour le diagnostic.
    [[nodiscard]] std::size_t entryCount() const noexcept {
        return _entries.size();
    }

    /**
     * @brief Texture de repli en damier magenta, créée **une seule fois** à la demande.
     *
     * Partagée par tous les appelants : le damier ne dépend d'aucun asset, il n'y a donc aucune
     * raison d'en créer plusieurs (`LOT-40` TACHE-03).
     * @return La texture de repli, ou `nullptr` si même sa création GPU a échoué (cas extrême :
     *         device perdu — le rendu saute alors la primitive plutôt que de planter).
     */
    [[nodiscard]] const LoadedTexture* missingTexture();

    /// @return Le résolveur de chemins d'assets utilisé par ce cache.
    [[nodiscard]] const AssetPaths& assetPaths() const noexcept {
        return _paths;
    }

private:
    /// Charge et valide un asset depuis le disque, sans passer par le cache.
    [[nodiscard]] std::optional<LoadedTexture> load(const std::string& fileName,
                                                    AssetFamily family) const;

    ID3D11Device* _device;  // non possédé
    AssetPaths _paths;
    /// Nom logique → texture chargée ; `std::nullopt` mémorise un **échec** déjà journalisé.
    std::unordered_map<std::string, std::optional<LoadedTexture>> _entries;
    std::optional<LoadedTexture> _missingTexture;
};

/**
 * @brief Obtient la texture d'un asset, ou le **repli en damier magenta** à défaut.
 *
 * **Point de résolution unique** du repli : tous les lots d'habillage (`LOT-42`, `LOT-44`,
 * `LOT-45`, `LOT-49`) passent par ici plutôt que de réimplémenter chacun son propre comportement
 * de secours — c'est la seule façon de garantir qu'un asset manquant se voie toujours de la même
 * manière et soit toujours journalisé (`EX-NFR-040`, `EX-REN-007`).
 * @param cache    Cache interrogé (et propriétaire du damier partagé).
 * @param fileName Nom logique de l'asset attendu, repris dans l'avertissement journalisé.
 * @param family   Famille de l'asset, qui fixe les dimensions attendues.
 * @return La texture de l'asset si elle est disponible, sinon le damier de repli ; `nullptr`
 *         seulement si même le damier n'a pas pu être créé.
 */
[[nodiscard]] const LoadedTexture* resolveOrPlaceholder(TextureCache& cache,
                                                        const std::string& fileName,
                                                        AssetFamily family);

}  // namespace hmi
