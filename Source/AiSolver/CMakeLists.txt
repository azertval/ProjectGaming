# Bibliothèque AiSolver : calcul numérique maison (tenseurs, RNG) pour le programme annexe d'IA de
# résolution autonome (@ref lots-annexe) — aucune dépendance à un framework de calcul tiers.
add_library(AiSolver STATIC
    Math/Rng.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet depuis la racine
# (ex. "AiSolver/Math/Rng.h"), comme Core/HMI.
target_include_directories(AiSolver PUBLIC ${PROJECT_SOURCE_DIR}/Source)

# PUBLIC (pas PRIVATE) : les en-têtes header-only (Tensor.h, TensorOps.h, Matmul.h) utilisent
# PROJECTGAMING_ASSERT (Core/Diagnostics/Assert.h) et sont inclus directement par les consommateurs
# d'AiSolver -- il leur faut donc le chemin d'inclusion et l'édition de liens de Core, pas
# seulement Rng.cpp (seul fichier compilé dans cette bibliothèque).
target_link_libraries(AiSolver PUBLIC Core project_warnings project_options)
