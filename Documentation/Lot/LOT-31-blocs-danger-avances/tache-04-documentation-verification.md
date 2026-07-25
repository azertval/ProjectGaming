# TACHE-04 — Documentation et vérification {#lot-31-tache-04-documentation-verification}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `Documentation` · **Statut :** ⬜

## Contexte
Dernière tâche : aligne la documentation sur le comportement réellement livré par
TACHE-01/02/03 — conformément au principe du projet, la documentation (`Guide`, format de niveau)
ne décrit que du code déjà implémenté, jamais en avance sur lui.

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : retirer les marqueurs ⚠️ « non implémenté » posés
  sur `EX-GP-050`–`EX-GP-053` et la sous-section « Dangers avancés » lors du cadrage de ce lot,
  une fois le comportement effectivement livré.
- **`Documentation/Specification/niveaux.md`** : section « Format retenu » étendue aux sept
  nouveaux types (`dangerUp`/`dangerDown`/`dangerLeft`/`dangerRight`/`dangerMover`/
  `dangerSwitched`/`dangerBlink`), champs optionnels par type, exemple JSON si pertinent.
- **`Documentation/Guide/guide-niveaux.md`** : nouvelle section sur les dangers avancés —
  `core::dangerHitbox`, généralisation de `core::Mechanism`/`MechanismController` (ou nouveau
  `DangerController`, selon la décision prise en TACHE-02), même niveau de détail que les sections
  existantes sur les mécanismes/blocs poussables.
- **`Documentation/Specification/editeur-niveaux.md`** : mention de la restructuration de la
  catégorie « Piège » (analogue à la section 8 sur la palette par catégories, `LOT-27`) et du geste
  de portée pour `DangerMover`.
- **`Source/Elements/Levels/README.md`** : liste des types de tuiles étendue aux sept nouveaux.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-31`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot.
- **Doxygen**, **Cahier de test**, **lint des exigences** : lancés localement (`python
  scripts/lint_exigences.py`, `python scripts/generate_cahier_test.py`,
  `python scripts/check_demo_sequence.py`), et Doxygen **vérifié avec le binaire 1.9.8 exact de la
  CI** avant tout push (mémoire `project_ci_local_reproduction`) — attention particulière au piège
  `` `fichier.cpp::Nom` `` dans un span (mémoire `project_doxygen_colon_link_pitfall`).

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`, `editeur-niveaux.md`.
- `Documentation/Guide/guide-niveaux.md`.
- `Source/Elements/Levels/README.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `python scripts/check_demo_sequence.py` retourne un code de sortie `0`.
- Doxygen (binaire 1.9.8 local, voir ci-dessus) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.
- Build complet `/W4 /WX` et suite de tests complète, sans régression.

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
`EX-GP-050`, `EX-GP-051`, `EX-GP-052`, `EX-GP-053`, `EX-LVL-002`, `EX-LVL-004`, `EX-EDIT-001`,
`EX-EDIT-002`, `EX-EDIT-003`.
