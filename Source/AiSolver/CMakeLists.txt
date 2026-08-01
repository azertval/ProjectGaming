# Bibliothèque AiSolver : calcul numérique maison (tenseurs, RNG) pour le programme annexe d'IA de
# résolution autonome (@ref lots-annexe) — aucune dépendance à un framework de calcul tiers.
add_library(AiSolver STATIC
    Math/Rng.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet depuis la racine
# (ex. "AiSolver/Math/Rng.h"), comme Core/HMI.
target_include_directories(AiSolver PUBLIC ${PROJECT_SOURCE_DIR}/Source)

target_link_libraries(AiSolver PRIVATE Core project_warnings project_options)
