# Modules de code source
#
# Core     : logique / moteur (bibliothèque)
# HMI      : rendu Direct3D 11 réutilisable (HmiRuntime) + exécutable historique (ProjectGaming)
# Editor   : application/éditeur Qt (ProjectGamingEditor) — LOT-34, optionnelle (nécessite Qt6)
# Elements : assets statiques (non compilés)

add_subdirectory(Core)
add_subdirectory(HMI)
add_subdirectory(Editor)
