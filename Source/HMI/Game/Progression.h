// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>

/**
 * @file HMI/Game/Progression.h
 * @brief Progression de partie persistée entre deux lancements (`EX-LVL-014`, `LOT-59` TACHE-05).
 */

namespace hmi {

/**
 * @brief Progression dans une séquence de niveaux : tableaux **terminés** et tableau **atteint**
 *        (celui où reprendre), identifiés par **nom de fichier**, jamais par indice.
 *
 * Un indice devient faux dès qu'un tableau est inséré au milieu de la séquence (`TACHE-04` rend
 * cela facile) ; le nom, lui, reste valide même après un réordonnancement. Logique **pure**, sans
 * Qt ni GPU (`EX-NFR-010`) -- même patron de persistance que `GameKeyBindings`
 * (`HMI/Input/GameKeyBindings.h`) : construction par défaut = partie neuve, `save`/`load` en
 * membre/statique, lecture tolérante (fichier absent/vide/malformé -> valeurs par défaut, jamais
 * bloquant, `EX-NFR-040`). L'écriture, elle, est **atomique** (fichier temporaire puis
 * remplacement, comme `hmi::encodeImageFile`, `LOT-54`) : une fermeture brutale ne doit jamais
 * laisser un fichier à demi écrit.
 */
class Progression {
public:
    Progression() = default;

    /// @return L'identifiant de la séquence en cours (nom de fichier, ex. `"sequence-demo.json"`),
    ///         vide si aucune partie n'a encore été jouée.
    [[nodiscard]] const std::string& sequenceId() const noexcept {
        return _sequenceId;
    }
    void setSequenceId(std::string sequenceId) {
        _sequenceId = std::move(sequenceId);
    }

    /// @return Le nom du tableau où reprendre (« Continuer »), vide si aucun (partie neuve, ou
    ///         séquence entièrement terminée).
    [[nodiscard]] const std::string& currentLevel() const noexcept {
        return _currentLevel;
    }
    void setCurrentLevel(std::string levelName) {
        _currentLevel = std::move(levelName);
    }

    /// @return L'ensemble des tableaux terminés (noms de fichiers), dans un ordre stable
    ///         (`std::set`), pas nécessairement celui de la séquence.
    [[nodiscard]] const std::set<std::string>& completedLevels() const noexcept {
        return _completedLevels;
    }
    [[nodiscard]] bool isCompleted(const std::string& levelName) const {
        return _completedLevels.contains(levelName);
    }
    /// Marque @p levelName terminé. Idempotent (`std::set`) : un tableau rejoué puis re-terminé ne
    /// change rien à l'ensemble -- couvre le cas « marqué une seule fois » sans logique dédiée.
    void markCompleted(const std::string& levelName) {
        _completedLevels.insert(levelName);
    }

    /// Efface toute la progression (« Nouvelle partie ») : retour à l'état de construction.
    void reset() {
        *this = Progression{};
    }

    /**
     * @brief Sauvegarde atomiquement dans @p path (fichier temporaire dans le même dossier, puis
     *        remplacement en une seule opération).
     * @return Faux si l'écriture a échoué (dossier non créable/en lecture seule, etc.) --
     *         récupérable : la partie continue, seule la persistance est perdue (journalisé par
     *         l'appelant).
     */
    [[nodiscard]] bool save(const std::filesystem::path& path) const;

    /**
     * @brief Charge depuis @p path.
     * @return La progression lue ; `Progression{}` (partie neuve) si le fichier est
     *         absent/vide/malformé, ou si une entrée n'est pas du type attendu -- jamais bloquant.
     */
    [[nodiscard]] static Progression load(const std::filesystem::path& path);

private:
    std::string _sequenceId;
    std::string _currentLevel;
    std::set<std::string> _completedLevels;
};

/**
 * @brief Règle de déverrouillage (`LOT-59` TACHE-06, `EX-IHM-005`) : fonction **pure**, seule
 *        source de vérité sur ce qui est jouable -- un écran ne fait que l'afficher, jamais sa
 *        propre condition.
 *
 * Jouable : tout tableau déjà **terminé** (`progression.isCompleted`), plus le **premier** tableau
 * non terminé de @p sequence, dans son ordre. Les suivants restent verrouillés -- sinon la
 * progression ne signifierait rien. Une @p sequence entièrement terminée rend tout jouable (aucun
 * tableau non terminé à borner).
 *
 * @param progression Progression courante (tableaux terminés).
 * @param sequence Séquence de niveaux, dans son ordre de jeu.
 * @param levelName Nom de fichier (jamais un indice, comme le reste de la progression).
 */
[[nodiscard]] bool isLevelUnlocked(const Progression& progression,
                                   const std::vector<std::string>& sequence,
                                   const std::string& levelName);

/**
 * @brief Variante utilisée par l'IHM (affichage de l'écran de sélection ET garde au lancement,
 *        `LOT-70`) : identique à `isLevelUnlocked`, sauf en build de **développement**
 *        (`core::DEVELOPER_BUILD`, `Core/BuildConfig.h`) où elle retourne toujours vrai -- pour
 *        tester un tableau donné sans rejouer toute la séquence depuis le début à chaque
 *        lancement. `isLevelUnlocked` elle-même reste inchangée (ses tests couvrent la règle de
 *        progression indépendamment du type de build) ; en Release les deux fonctions coïncident.
 */
[[nodiscard]] bool isLevelPlayable(const Progression& progression,
                                   const std::vector<std::string>& sequence,
                                   const std::string& levelName);

}  // namespace hmi
