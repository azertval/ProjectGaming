# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Modules de code source
#
# Core     : logique / moteur (bibliothèque)
# HMI      : application/éditeur Qt (ProjectGaming) + rendu Direct3D 11 — nécessite Qt6
# Elements : assets statiques non compilés (niveaux, localisation, UI Qt, thèmes, textures…)
# AiSolver : calcul numérique maison (tenseurs, RNG) pour l'IA de résolution autonome (programme
#            annexe, @ref lots-annexe), sans dépendance à un framework de calcul tiers

add_subdirectory(Core)
add_subdirectory(HMI)
add_subdirectory(AiSolver)
