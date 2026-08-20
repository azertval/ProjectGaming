// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <optional>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "HMI/Editor/PathGesture.h"

/**
 * @file HMI/Editor/PropertiesPanel.h
 * @brief Panneau « Propriétés » : réglages de gameplay de l'élément sélectionné et du tableau
 *        (LOT-67, `EX-EDIT-033`).
 */

namespace Ui {
class PropertiesPanel;
}

namespace core {
class LevelDraft;
}

namespace hmi {

class Localization;

/**
 * @brief Réglages **de gameplay** éditables : timings des éléments mobiles, règles du tableau.
 *
 * **Vue du modèle**, pas un état : `refresh` repeuple les champs depuis un `core::LevelDraft` et la
 * sélection courante, aucune copie n'est conservée entre deux appels. Les champs n'écrivent rien :
 * ils émettent un signal typé, l'appelant (`hmi::MainWindow` → `hmi::GameViewport`) applique la
 * mutation au brouillon, seul propriétaire — même patron que la section « Cadrage » du panneau
 * Textures.
 *
 * Les champs numériques émettent sur `editingFinished`, **jamais** sur `valueChanged` : taper
 * « 120 » dans une période produirait sinon trois mutations (`1`, `12`, `120`), donc trois pas
 * d'annulation pour un seul geste utilisateur.
 *
 * Deux notions distinctes cohabitent dans les règles du tableau, séparées en deux groupes
 * explicitement libellés : un **budget** (`EX-GP-024`) se consomme une fois pour toutes sur tout le
 * tableau, une **capacité** (`EX-GP-055`) se recharge à chaque contact avec le sol.
 */
class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);
    ~PropertiesPanel() override;

    /**
     * @brief Repeuple le panneau depuis @p draft et la sélection @p selection.
     *
     * @param draft     Brouillon courant (lecture seule).
     * @param selection Parcours sélectionné par l'outil « Parcours », si un l'est.
     * @param blinkCell Case d'un danger temporisé sélectionné, si l'un l'est — les dangers
     *                  temporisés n'ont pas de trajectoire, donc pas de `PathSelection` ; ils sont
     *                  désignés par leur case.
     */
    void refresh(const core::LevelDraft& draft, std::optional<PathSelection> selection,
                 std::optional<core::GridPosition> blinkCell);

    /// Applique la langue active (titres de groupes, libellés, entrées de listes déroulantes).
    void retranslateUi(const Localization& loc);

signals:
    /// La vitesse de la plateforme en @p position vient d'être changée (`EX-GP-026`).
    void platformSpeedChanged(core::GridPosition position, float speed);
    /// Le déphasage de la plateforme en @p position vient d'être changé, en pas fixes.
    void platformPhaseChanged(core::GridPosition position, int phase);
    /// Le mode de bouclage de la plateforme en @p position vient d'être changé (`EX-GP-054`).
    void platformModeChanged(core::GridPosition position, core::PlatformPathMode mode);
    /// L'axe et la portée du danger mobile en @p position viennent d'être changés (`EX-GP-051`).
    void moverConfigChanged(core::GridPosition position, core::DangerMoverAxis axis, int range);
    /// La période, le déphasage et la durée active du danger temporisé en @p position viennent
    /// d'être changés (`EX-GP-053`).
    void blinkConfigChanged(core::GridPosition position, int period, int phase, int activeDuration);
    /// Le budget de sauts du tableau vient d'être changé (`EX-GP-024`) ; -1 = illimité.
    void jumpBudgetChanged(int jumpBudget);
    /// Le budget de dashs du tableau vient d'être changé (`EX-GP-024`) ; -1 = illimité.
    void dashBudgetChanged(int dashBudget);
    /// Les sauts aériens accordés par le tableau viennent d'être changés (`EX-GP-055`) ; absent
    /// pour s'en remettre au réglage du moteur.
    void airJumpsChanged(std::optional<int> airJumps);
    /// Les charges de dash accordées par le tableau viennent d'être changées (`EX-GP-055`).
    void dashChargesChanged(std::optional<int> dashCharges);

private:
    /// Rang des pages de `selectionStack`, dans l'ordre du fichier `.ui`.
    enum Page { PageEmpty = 0, PagePlatform = 1, PageMover = 2, PageBlink = 3 };

    void connectFields();

    Ui::PropertiesPanel* _ui;
    const Localization* _loc = nullptr;
    /// Case de l'élément actuellement affiché, si un l'est : c'est **elle** que portent les
    /// signaux, jamais un rang de vecteur — un rang se périme dès qu'une configuration est
    /// ajoutée ou retirée, une case non.
    std::optional<core::GridPosition> _target;
    /// Vrai pendant `refresh` : empêche les signaux de champs de reboucler en mutations.
    bool _updating = false;
};

}  // namespace hmi
