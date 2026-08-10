# TACHE-07 — Documentation et vérification {#lot-59-tache-07-documentation-verification}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/Test`, `Documentation` ·
**Statut :** non commencé

## Contexte
Ce lot est le premier depuis longtemps à modifier ce que **voit le joueur** plutôt que ce que
manipule le level designer. Deux documents s'en trouvent faux dès la première tâche : le manuel
(« Quitter vers le menu — Échap ») et les spécifications, dont deux exigences portent une mention
⚠️ que ce lot lève.

## Travail à réaliser
- **Lever les mentions ⚠️** devenues fausses :
  - `EX-REN-031` (`Specification/rendu-technique.md`) — décrire l'écran de pause et l'écran de fin
    de niveau livrés, retirer « non implémenté » et la description du comportement d'avant.
  - `EX-GP-040` (`Specification/gameplay.md`) — les états `Pause` et `NiveauTermine` existent ;
    retirer « partiellement implémenté » et mettre à jour l'énumération de `hmi::ScreenId`.
  - `vision.md` — la sauvegarde de progression n'est plus « hors périmètre » : renvoyer vers
    `EX-LVL-014`.
- **Manuel utilisateur** (`Documentation/Manuel/jouer.md`) : Échap met en pause (et non plus quitte),
  écrans de fin de niveau et de fin de séquence, Continuer / Nouvelle partie / Choisir un niveau,
  et le fait que la progression est conservée. Le tableau des contrôles est à corriger.
- **Guide du développeur** : une section « boucle de jeu et écrans » dans `Documentation/Guide/` —
  machine à états, suspension du pas fixe et son piège de réarmement d'horloge, format de séquence,
  format de progression. Documenter **ce qui est livré**, pas l'intention.
- **Cahier de test** : régénérer via `python scripts/generate_cahier_test.py` ; chaque nouveau
  `TEST()` porte son bloc `\castest{}`, écrit **en même temps** que le test.
- **Vérification manuelle**, au moment prévu par le lot et pas avant : jouer trois tableaux, mettre
  en pause, reprendre, quitter l'application, la relancer, vérifier *Continuer* ; terminer la
  séquence ; jouer un niveau personnel.

## Fichiers impactés
- `Documentation/Specification/{rendu-technique,gameplay,vision}.md`.
- `Documentation/Manuel/jouer.md`.
- `Documentation/Guide/guide-boucle.md` (étendu) ou nouveau `guide-ecrans.md` selon le découpage
  existant.
- `Documentation/CahierTest.md` (régénéré, jamais édité à la main).
- `CHANGELOG.md` (section *Non publié*).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — les quatre nouvelles exigences sont déclarées **une fois** et
  référencées ; aucune référence orpheline.
- `python scripts/generate_cahier_test.py --check` — cahier à jour.
- `python scripts/check_demo_sequence.py` — adapté par la `TACHE-04`, et vert.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml` (`DOXYGEN_VERSION`),
  pas celle du poste.
- `ctest --preset vs` à 100 %.

## Points d'attention
- **Ne documenter que le livré.** Si une tâche a été rognée, le guide et le manuel décrivent ce qui
  existe, pas ce qui était prévu.
- Le cahier de test se **régénère** ; l'éditer à la main est la première cause d'échec du garde-fou
  de complétude.
- Dans la documentation Doxygen, ne jamais écrire `` `fichier.cpp::Nom` `` : le `::` dans un span
  casse la génération sur la version épinglée de la CI sans rien dire en local.
- Vérifier la CI **complète** avant d'ouvrir la PR (`gh pr checks`), pas seulement le lint
  d'exigences.

## Définition de fait (DoD)
- Les deux mentions ⚠️ sont levées, le manuel décrit les contrôles réels, le guide documente la
  machine à états et les deux nouveaux formats, le cahier est régénéré, tous les scripts de
  vérification et la CI complète sont verts, l'essai manuel est fait.

## Exigences
Réutilise `EX-REN-031`, `EX-GP-040` (levées), `EX-IHM-004`, `EX-IHM-005`, `EX-LVL-013`,
`EX-LVL-014`, `EX-NFR-012` (conventions), `EX-NFR-020` (tests), `EX-NFR-022` (CI verte).
