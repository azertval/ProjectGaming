#include "HMI/Editor/DecorListModel.h"

#include <algorithm>

namespace hmi {

namespace {
// Rang d'affichage d'une couche (ordre de declaration de core::DecorLayer : Background < Decor <
// Foreground), pour le tri primaire de buildDecorListRows.
int layerDisplayRank(core::DecorLayer layer) noexcept {
    switch (layer) {
        case core::DecorLayer::Background:
            return 0;
        case core::DecorLayer::Decor:
            return 1;
        case core::DecorLayer::Foreground:
            return 2;
    }
    return 1;
}
}  // namespace

std::vector<DecorListRow> buildDecorListRows(const std::vector<core::Decor>& decors,
                                             const std::vector<std::string>& availableAssets) {
    std::vector<DecorListRow> rows;
    rows.reserve(decors.size());
    for (std::size_t index = 0; index < decors.size(); ++index) {
        const core::Decor& decor = decors[index];
        const bool missing =
            std::ranges::find(availableAssets, decor.assetName) == availableAssets.end();
        rows.push_back(DecorListRow{.index = index,
                                    .assetName = decor.assetName,
                                    .layer = decor.layer,
                                    .assetMissing = missing});
    }
    // Tri STABLE par couche uniquement : les lignes arrivent deja dans l'ordre de superposition
    // (index croissant de core::LevelDraft::decors()), un tri stable le preserve donc a couche
    // egale, sans cle secondaire explicite.
    std::ranges::stable_sort(rows, [](const DecorListRow& a, const DecorListRow& b) {
        return layerDisplayRank(a.layer) < layerDisplayRank(b.layer);
    });
    return rows;
}

}  // namespace hmi
