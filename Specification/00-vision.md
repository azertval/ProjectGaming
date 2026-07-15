# SPEC 00 — Vision & périmètre

> Statut : **brouillon**. Les points marqués ⚠️ sont des propositions à valider.

## Concept
Jeu **2D de plateforme et puzzle**, en vue de côté, à base de niveaux discrets. Le joueur dirige un personnage qui traverse des tableaux en combinant **agilité de plateforme** (sauter, courir) et **résolution d'énigmes** (interrupteurs, blocs, portes, dangers).

- **Genre** : plateforme / puzzle.
- **Perspective** : 2D, vue de côté, décor en tuiles.
- **Session type** : niveaux courts (1 à 3 minutes), rejouables.
- **Public** : joueurs appréciant la réflexion et la précision, tous âges.
- **Plateforme** : Windows (bureau), rendu DirectX.

### Mécanique de jeu (décidée)
Le jeu repose sur la **boîte à outils classique** du genre plateforme/puzzle : interrupteurs, portes, clés, blocs poussables, dangers et (plus tard) plateformes mobiles. **Pas de mécanique signature exotique** pour le MVP : la difficulté vient de l'agencement des mécanismes, pas d'un gimmick. Une mécanique distinctive pourra être introduite ultérieurement une fois le socle validé.

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

## Objectifs produit (au-delà du moteur)
- **EX-VIS-006** — Le projet doit fournir un **éditeur de niveaux** permettant à des membres non-développeurs de l'équipe (level/game design) de créer du contenu sans coder. Détaillé dans [`06-editeur-niveaux.md`](06-editeur-niveaux.md). Livré **après** le chargement de niveaux dans le moteur.

## Hors périmètre (MVP)
- Multijoueur, réseau.
- Sauvegarde de progression persistante (au-delà du niveau courant).
- Édition collaborative en temps réel dans l'éditeur (plusieurs éditeurs simultanés sur un même niveau).
- Bande-son musicale élaborée (bruitages simples suffisent au MVP).
- Portabilité hors Windows.

## Traçabilité
Ces objectifs sont détaillés dans [`01-gameplay.md`](01-gameplay.md), [`02-controles.md`](02-controles.md), [`03-rendu-technique.md`](03-rendu-technique.md), [`04-niveaux.md`](04-niveaux.md), [`05-exigences-non-fonctionnelles.md`](05-exigences-non-fonctionnelles.md) et [`06-editeur-niveaux.md`](06-editeur-niveaux.md). Chaque lot de `../Lot/` référence les exigences `EX-…` qu'il couvre.
