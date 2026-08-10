#pragma once

#include <QWidget>
#include <cstddef>

#include "HMI/Editor/PixelHistory.h"

class QListWidget;

/**
 * @file HMI/Editor/PixelHistoryPanel.h
 * @brief Panneau d'historique visuel de l'atelier pixel art (`LOT-54` TACHE-04, `EX-IHM-060`).
 */

namespace hmi {

class Localization;

/**
 * @brief Liste ordonnée des opérations nommées de l'historique du canevas pixel art actif.
 *
 * **Vue du modèle**, pas un état : `refresh` reconstruit la liste depuis un `hmi::PixelHistory`
 * (`hmi::pixelOperationTranslationKey`), même patron que `hmi::LinkPanel`. Double-cliquer une
 * entrée émet `jumpRequested` (le retour effectif passe par `PixelCanvas::jumpHistoryTo`, seul
 * propriétaire de l'historique) ; la dernière entrée reste toujours sélectionnée après un
 * rafraîchissement, indiquant la position courante dans la liste.
 */
class PixelHistoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit PixelHistoryPanel(QWidget* parent = nullptr);

    /// Reconstruit la liste depuis @p history (après toute mutation : geste, annuler, refaire).
    void refresh(const PixelHistory& history);

    /// Applique la langue active (libellés des opérations déjà affichées).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis quand l'utilisateur double-clique une entrée : retour à l'état immédiatement après
    /// celle-ci (`hmi::PixelHistory::jumpTo`).
    void jumpRequested(std::size_t index);

private:
    void onItemActivated();

    QListWidget* _list;
    const Localization* _loc = nullptr;
    std::vector<PixelHistoryEntry> _entries;  ///< Dernier contenu affiché (retranslateUi le relit).
};

}  // namespace hmi
