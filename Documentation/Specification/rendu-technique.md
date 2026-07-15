# Rendu & cible technique {#spec-rendu-technique}

> Statut : **brouillon**. Dépend de [`vision.md`](vision.md).

## 1. Cible technique
- \anchor EX-REN-001 **EX-REN-001** — Le jeu doit fonctionner sous **Windows 10/11 (x64)**.
- \anchor EX-REN-002 **EX-REN-002** — Le rendu doit s'appuyer sur **Direct3D 11** (bon compromis simplicité/capacités pour de la 2D ; Direct3D 12 écarté car surdimensionné au MVP).
- \anchor EX-REN-003 **EX-REN-003** — La fenêtre doit être créée via l'API Win32, redimensionnable, avec titre et icône.

## 2. Rendu 2D
- \anchor EX-REN-010 **EX-REN-010** — Le rendu doit afficher une grille de tuiles à partir d'un **atlas de textures** (spritesheet).
- \anchor EX-REN-011 **EX-REN-011** — Le rendu doit afficher des **sprites** pour le personnage et les mécanismes, avec transparence.
- \anchor EX-REN-012 **EX-REN-012** — Le rendu doit supporter des **animations** par séquence d'images (personnage : repos, course, saut).
- \anchor EX-REN-013 **EX-REN-013** — Une **caméra 2D** doit suivre le personnage et rester bornée aux limites du niveau.
- \anchor EX-REN-014 **EX-REN-014** — Le rendu doit gérer un ordre de dessin par **couches** (fond, décor, entités, interface).

## 3. Boucle & temps
- \anchor EX-REN-020 **EX-REN-020** — Le jeu doit tourner à **60 images/seconde** cible.
- \anchor EX-REN-021 **EX-REN-021** — La logique doit être mise à jour à **pas de temps fixe** (simulation déterministe), le rendu pouvant être découplé.
- \anchor EX-REN-022 **EX-REN-022** — Le rendu doit synchroniser la présentation (V-Sync activable) pour éviter le *tearing*.

## 4. Interface (HMI)
- \anchor EX-REN-030 **EX-REN-030** — Le jeu doit afficher un **menu principal** (Jouer, Quitterter).
- \anchor EX-REN-031 **EX-REN-031** — Le jeu doit afficher un écran de **pause** et un écran de **fin de niveau**.
- \anchor EX-REN-032 **EX-REN-032** — Le jeu doit afficher du **texte** (titres, indications) via une police bitmap ou vectorielle.

## 5. Audio (⚠️ minimal MVP)
- \anchor EX-REN-040 **EX-REN-040** (⚠️ souhaité) — Le jeu devrait jouer des **bruitages** (saut, interrupteur, victoire, échec).

## Traçabilité
Tout ce qui touche fenêtre, rendu, entrées et interface relève de `Source/HMI` ; la logique de simulation reste dans `Source/Core`. Contraintes de performance : [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md).
