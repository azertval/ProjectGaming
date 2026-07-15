# SPEC 00 — Vision & périmètre

> Statut : **brouillon**. Les points marqués ⚠️ sont des propositions à valider.

## Concept
Jeu **2D de plateforme et puzzle**, en vue de côté, à base de niveaux discrets. Le joueur dirige un personnage qui traverse des tableaux en combinant **agilité de plateforme** (sauter, courir) et **résolution d'énigmes** (interrupteurs, blocs, portes, dangers).

- **Genre** : plateforme / puzzle.
- **Perspective** : 2D, vue de côté, décor en tuiles.
- **Session type** : niveaux courts (1 à 3 minutes), rejouables.
- **Public** : joueurs appréciant la réflexion et la précision, tous âges.
- **Plateforme** : Windows (bureau), rendu DirectX.

### ⚠️ Mécanique signature (à définir)
Le socle repose sur la boîte à outils classique du genre (blocs, interrupteurs, clés/portes, dangers, plateformes mobiles). Une **mécanique signature** distinctive reste à choisir. Proposition par défaut : **bascule de gravité** (le joueur inverse le sens de la gravité pour marcher au plafond). Alternatives : rembobinage du temps, création/suppression de blocs, dédoublement du personnage.

## Boucle de gameplay
1. Le joueur entre dans un niveau (état initial défini).
2. Il observe l'agencement (plateformes, obstacles, mécanismes).
3. Il agit : déplacement, saut, interaction avec les mécanismes.
4. Il atteint la **sortie** du niveau (condition de victoire) — ou échoue (danger / chute) et **recommence** immédiatement.
5. Progression vers le niveau suivant.

## Objectifs (MVP)
- **EX-VIS-001** — Le jeu doit proposer un personnage jouable se déplaçant et sautant dans un niveau en tuiles.
- **EX-VIS-002** — Le jeu doit permettre de terminer un niveau en atteignant une sortie.
- **EX-VIS-003** — Le jeu doit intégrer au moins un mécanisme de puzzle (interrupteur ↔ porte).
- **EX-VIS-004** — Le jeu doit gérer l'échec (danger/chute) et le redémarrage du niveau.
- **EX-VIS-005** — Le jeu doit enchaîner au moins **3 niveaux** de démonstration.

## Hors périmètre (MVP)
- Multijoueur, réseau.
- Sauvegarde de progression persistante (au-delà du niveau courant).
- Éditeur de niveaux intégré au jeu (les niveaux sont des fichiers de données).
- Bande-son musicale élaborée (bruitages simples suffisent au MVP).
- Portabilité hors Windows.

## Traçabilité
Ces objectifs sont détaillés dans [`01-gameplay.md`](01-gameplay.md), [`02-controles.md`](02-controles.md), [`03-rendu-technique.md`](03-rendu-technique.md), [`04-niveaux.md`](04-niveaux.md) et [`05-exigences-non-fonctionnelles.md`](05-exigences-non-fonctionnelles.md). Chaque lot de `../Lot/` référence les exigences `EX-…` qu'il couvre.
