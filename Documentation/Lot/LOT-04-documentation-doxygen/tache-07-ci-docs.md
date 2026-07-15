# TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)

**Lot :** [LOT-04](epic.md) · **Emplacement :** `.github/workflows/docs.yml`, `Documentation/Doxyfile` · **Statut :** à faire

## Contexte
La CI documentaire (`docs.yml`) génère la Doxygen sur `main` et la publie sur `gh-pages`. Une fois toutes les pages en place et propres, on ajoute un **garde-fou qualité** : la génération échoue au moindre avertissement, dans l'esprit du `/W4 /WX` côté code.

## Travail à réaliser
- `Documentation/Doxyfile` : `WARN_AS_ERROR = YES` (ou `FAIL_ON_WARNINGS`), `QUIET = YES`, `WARN_NO_PARAMDOC = YES` selon le niveau d'exigence souhaité.
- `docs.yml` : s'assurer que l'échec de génération **casse** le job ; publier `Documentation/generated/html` (inchangé) sur `gh-pages`.
- Corriger les éventuels avertissements résiduels révélés par le garde-fou (références, ancres, symboles non documentés).
- Vérifier que le site publié contient bien accueil + spécifications + lots + manuel + référence de code.

## Fichiers impactés
- `Documentation/Doxyfile`.
- `.github/workflows/docs.yml`.

## Vérifications (obligatoires)
- En local, `doxygen Doxyfile` avec `WARN_AS_ERROR` retourne un **code de sortie non nul** en présence d'un avertissement (test négatif), et **0** une fois tout propre.
- Le job `docs.yml` échoue si la génération avertit.
- Après merge sur `main`, `gh-pages` sert un site complet et navigable.

## Points d'attention
- Activer `WARN_AS_ERROR` **en dernier**, une fois TACHE-02→06 terminées, pour ne pas bloquer le travail intermédiaire.
- Distinguer avertissements légitimes (à corriger) et bruit éventuel (ex. fichiers `Source/**/README.md` non voulus comme pages) — ajuster `INPUT`/`EXCLUDE` plutôt que de baisser le niveau d'exigence.

## Définition de fait (DoD)
- Génération sans avertissement, garde-fou actif en CI, site complet publié sur `gh-pages`.

## Exigences
`EX-NFR-012`, `EX-NFR-013` (esprit « zéro avertissement »).
