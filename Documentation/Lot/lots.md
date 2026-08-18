# Lots {#lots}

Le travail est découpé en **lots** (un incrément livrable par lot), chacun dans
un sous-dossier `LOT-XX-nom/` contenant un `epic.md` (objectif, périmètre,
critères d'acceptation) et des fichiers `tache-NN.md` (une unité de travail
chacun). Les lots référencent les [spécifications](@ref specifications) via les
identifiants d'exigences `EX-…`.

Contrairement aux spécifications, les lots **conservent** leur numéro (`LOT-XX`) :
c'est un identifiant stable, jamais réordonné.

> **Exception actée — programme d'habillage `LOT-40` → `LOT-55`.** La règle ci-dessus protège les
> identifiants **livrés** : un lot terminé n'est jamais renuméroté, sous peine de rendre fausses
> toutes les références du CHANGELOG, des commits et des spécifications. Les lots `LOT-40` à
> `LOT-48`, **tous non commencés**, ont été renumérotés **une seule fois**, lors du recadrage du
> programme d'habillage, afin que le numéro corresponde à l'ordre d'implémentation réel. Cette
> exception ne se reproduira pas : à partir de ce recadrage, tout lot planifié conserve son numéro
> même si son ordre d'exécution change.

## Lots

- @subpage lot-01
- @subpage lot-02
- @subpage lot-03
- @subpage lot-04
- @subpage lot-05
- @subpage lot-06
- @subpage lot-07
- @subpage lot-08
- @subpage lot-09
- @subpage lot-10
- @subpage lot-11
- @subpage lot-12
- @subpage lot-13
- @subpage lot-14
- @subpage lot-15
- @subpage lot-16
- @subpage lot-17
- @subpage lot-18
- @subpage lot-19
- @subpage lot-20
- @subpage lot-21
- @subpage lot-22
- @subpage lot-23
- @subpage lot-24
- @subpage lot-25
- @subpage lot-26
- @subpage lot-27
- @subpage lot-28
- @subpage lot-29
- @subpage lot-30
- @subpage lot-31
- @subpage lot-32
- @subpage lot-33
- @subpage lot-34
- @subpage lot-35
- @subpage lot-36
- @subpage lot-37
- @subpage lot-38
- @subpage lot-39
- @subpage lot-40
- @subpage lot-41
- @subpage lot-42
- @subpage lot-43
- @subpage lot-44
- @subpage lot-45
- @subpage lot-46
- @subpage lot-47
- @subpage lot-48
- @subpage lot-49
- @subpage lot-50
- @subpage lot-51
- @subpage lot-52
- @subpage lot-53
- @subpage lot-54
- @subpage lot-55
- @subpage lot-56
- @subpage lot-57
- @subpage lot-58
- @subpage lot-59
- @subpage lot-60
- @subpage lot-61
- @subpage lot-62
- @subpage lot-63
- @subpage lot-64
- @subpage lot-65
- @subpage lot-66
- @subpage lot-67

## Apres le programme `0.1.0`

Le [LOT-67](@ref lot-67) ouvre la suite : il ne fait partie d'aucun programme cadre, et repond a un
manque constate a l'usage de l'editeur — les trajectoires des elements mobiles et les regles de
mobilite d'un tableau n'etaient editables qu'en modifiant le JSON a la main.

## Programme `0.1.0`

Les lots `LOT-58` à `LOT-66`, avec le `LOT-53` (cadré de longue date et resté non commencé),
forment le programme de la version **`0.1.0`** : le passage d'un moteur complet à un **jeu**
distribuable. Deux familles s'y répondent — la **complétude produit** (boucle de jeu, son, effets,
mécanismes manquants, cadrage de caméra, refonte des niveaux) et le **durcissement d'ingénierie**
(vérification en Release, sanitizer, analyse statique, diagnostics d'une version publiée, budget de
rendu mesuré).

Ces lots sont numérotés **dans leur ordre d'exécution**, ce que la règle générale ci-dessus permet
puisqu'aucun n'était encore livré au moment de leur cadrage. Seul le `LOT-53`, cadré de longue date
et déjà publié sous ce numéro, conserve le sien et s'exécute entre le `LOT-60` et le `LOT-61` :
c'est précisément le cas que la règle protège.

| Rang | Lot | Pourquoi à cette place |
|:----:|-----|------------------------|
| 1 | [LOT-58](@ref lot-58) | Le durcissement précède le contenu qu'il doit protéger. |
| 2 | [LOT-59](@ref lot-59) | Tous les autres lots produit se voient à travers ses écrans. |
| 3 | [LOT-60](@ref lot-60) | Le son a besoin d'un écran de fin de niveau où exister. |
| 4 | [LOT-53](@ref lot-53) | Réutilise les déclencheurs d'événements posés par le `LOT-60`. |
| 5 | [LOT-61](@ref lot-61) | Indépendant ; requis avant qu'un tiers n'exécute le jeu. |
| 6 | [LOT-62](@ref lot-62) | Mesure le budget une fois tous les émetteurs livrés. |
| 7 | [LOT-63](@ref lot-63) | **Découpable** : le lot qu'on rogne si le calendrier se tend. |
| 8 | [LOT-64](@ref lot-64) | Le cadrage doit exister avant qu'on refasse les niveaux. |
| 9 | [LOT-65](@ref lot-65) | Dernier lot de contenu : exploite tout ce qui précède. |
| 10 | [LOT-66](@ref lot-66) | Clôt le programme ; les statuts ne se figent qu'à la fin. |
