# TACHE-10 — Contenu, documentation et référentiel {#lot-69-tache-10-contenu-documentation}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Documentation` ·
**Statut :** non commencé

## Contexte
`demo-final.json` est le **seul** niveau livré à porter des décors — sept entrées. Il faut préserver
son intention visuelle sans prétendre à une conversion mécanique : un assemblage de sprites ne
devient pas une fresque peinte par transformation automatique.

Le volet référentiel est contraint par `scripts/lint_exigences.py` : chaque exigence déclarée
**exactement une fois**, aucune référence orpheline, et toute exigence déclarée référencée quelque
part.

## Travail à réaliser
- **Migrer `demo-final.json`** : retirer `decors`, déclarer deux plans — un fond à densité 8 avec un
  facteur inférieur à 1, un plan de devant à densité native avec un facteur supérieur à 1 — et le
  drapeau de parallaxe.
- **Générer les PNG** par un script (`scripts/generate_demo_plans.py`) qui **compose les assets de
  décor retirés à leurs anciennes positions** dans une image aux dimensions du niveau. L'intention
  visuelle est préservée et le résultat est **reproductible** (`LOT-66`), ce qu'une image dessinée à
  la main et commitée ne serait pas. Puis supprimer `Source/Elements/Assets/Decors/` et le générateur
  d'assets de décors.
- Vérifier `check_demo_sequence.py` et la séquence : aucun autre niveau ne porte de décors, mais la
  **présence et les dimensions** des PNG référencés doivent être contrôlées.
- **Référentiel** — la stratégie est déjà en place depuis le cadrage et doit être **maintenue** :
  aucune ancre supprimée, aucune renumérotation ; les exigences retirées vivent dans la section
  dédiée de [`decors.md`](@ref spec-decors), leur texte d'origine intact, chacune précédée du motif
  du retrait. Les dossiers `LOT-49`/`LOT-50`/`LOT-51` et le `CHANGELOG` continuent de s'y référer
  sans être réécrits — réécrire un lot livré falsifierait son histoire.
- Mettre à jour les **guides** utilisateur (niveaux, éditeur, rendu, atelier pixel art) : la notion
  de décor y disparaît au profit des plans, et le mode création y est décrit — y compris ce qu'il
  **n'est pas** (la référence est un repère géométrique, pas un aperçu).
- Figer les **statuts** : chapeau de `decors.md`, tableau de découpage de l'epic, `lots.md`.
- `CHANGELOG.md` sous `[Non publié]` : compléter la section du lot, en mentionnant explicitement le
  **retrait** du système de décors et l'**inversion** de la décision du `LOT-64` sur la parallaxe en
  mode suivi — deux changements visibles qui ne doivent pas se découvrir à l'usage.
- Régénérer le cahier de test et relancer les quatre linters.

## Fichiers impactés
`Source/Elements/Levels/demo-final.json`, `Source/Elements/Levels/Plans/*` (nouveaux),
`scripts/generate_demo_plans.py` (nouveau), `Documentation/Specification/*`,
`Documentation/Guide/*`, `Documentation/Lot/lots.md`,
`Documentation/Lot/LOT-69-plans-picturaux/*`, `CHANGELOG.md`.

## Tests (obligatoires)
- `test_parcours_complet.cpp` / `test_couverture_mecaniques.cpp` : `demo-final` charge, se joue, et
  ses deux plans sont présents et résolus.
- Test système : **aucun niveau du dépôt ne contient `decors`**.
- Test système : **tout plan référencé existe et a les dimensions attendues** pour sa densité.

## Points d'attention
La stratégie « exigences retirées » doit être **validée en exécutant le lint**, pas supposée : c'est
le point où le volet documentaire peut casser silencieusement. Elle est préférable à un ajout dans la
liste d'exceptions du linter, dont le contrat documenté ne couvre que les invariants transverses et
le post-MVP — l'élargir à « retirée » modifierait ce contrat pour rien.

Générer les plans par script plutôt que commiter des images dessinées est un choix de
**reproductibilité**, dans la ligne du `LOT-66`. Il a une limite à assumer : le résultat est un
report fidèle de l'ancien habillage, pas un décor peint qui exploiterait vraiment sept plans. Ce
dernier est un acte de *level design*, hors de ce lot d'outillage.

## Definition de fait (DoD)
Aucun niveau ne porte plus de décors, tous les plans référencés existent, les quatre linters passent,
le cahier de test est régénéré, les statuts sont figés. `ctest` à 100 %.

## Exigences
`EX-LVL-009`, `EX-LVL-012`, `EX-LVL-013`, `EX-DEC-040`, `EX-DEC-032`, `EX-NFR-031`.
