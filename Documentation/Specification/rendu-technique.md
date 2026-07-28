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
- \anchor EX-REN-013 **EX-REN-013** — Une **caméra 2D** doit cadrer le niveau en jeu : elle reste
  bornée aux limites de ce qu'elle cadre, et pour un contenu plus grand que la fenêtre, elle
  **zoome pour l'englober entièrement** plutôt que de suivre le personnage — aucune zone ne doit
  rester invisible. Pour un niveau qui **tient dans une seule salle** (`RoomGrid`, `EX-REN-015`),
  ce contenu est le **niveau entier** (précisé en LOT-16 ; la formulation initiale « suivre le
  personnage » ne correspondait déjà plus à l'implémentation, une caméra fixe cadrant le tableau
  depuis LOT-08) ; au-delà d'une salle, `EX-REN-015` prend le relais et ce contenu devient la
  **salle courante**.
- \anchor EX-REN-015 **EX-REN-015** — Pour un niveau plus grand qu'une **salle** (constante de
  taille fixe, `LOT-32`), la caméra ne cadre plus le niveau entier mais la **salle** contenant le
  personnage, au zoom pixel art natif (`EX-REN-013`, appliqué au rectangle de la salle plutôt qu'au
  niveau) : elle bascule **nettement** sur la salle voisine dès que le personnage en franchit la
  frontière, sans jamais suivre le personnage en continu ni rapetisser le rendu quelle que soit la
  taille totale du niveau. Le niveau reste une **grille de tuiles unique** (aucun format, aucune
  nouvelle tuile) : une salle a « plusieurs entrées/sorties » simplement parce qu'un couloir reste
  ouvert sur plusieurs de ses bords vers des salles voisines — propriété géométrique, pas un
  mécanisme.
- \anchor EX-REN-014 **EX-REN-014** — Le rendu doit gérer un ordre de dessin par **couches** (fond, décor, entités, interface).
- \anchor EX-REN-041 **EX-REN-041** — Le rendu doit pouvoir **charger ses textures depuis des
  fichiers image** (PNG au minimum), décodés en pixels RGBA puis créés en texture Direct3D 11, en plus
  de la génération procédurale historique. Le filtrage reste *nearest* (pixel art, `EX-ARCH-022`).
  Concrétisé en `LOT-39`.
- \anchor EX-REN-042 **EX-REN-042** — Les **assets graphiques** (atlas de tuiles) doivent être
  **externalisés en fichiers éditables hors code** (remplacer le fichier suffit à changer l'apparence),
  copiés à côté de l'exécutable comme les niveaux et la localisation, avec **repli procédural** si un
  asset est absent/illisible (`EX-NFR-040`). Concrétisé en `LOT-39`.
- \anchor EX-REN-043 **EX-REN-043** — Le rendu doit pouvoir dessiner, en une seule frame, des
  entités provenant de **plusieurs textures distinctes** (au-delà de l'atlas unique historique), sans
  changer le contrat public de `SpriteBatch`/`SpriteRenderer`, et selon un **ordonnancement de calques
  explicite et unique** (fond, tuiles, objets, aides d'édition). Concrétisé en `LOT-40`.
- \anchor EX-REN-044 **EX-REN-044** — Un niveau doit pouvoir afficher une **image de fond**
  optionnelle, en dessous de toutes les tuiles, en mode Texture uniquement ; l'absence de fond
  configuré est un état normal (pas de repli visible), un fond référencé mais introuvable déclenche
  le repli en damier (`EX-NFR-040`). Concrétisé en `LOT-43`.
- \anchor EX-REN-045 **EX-REN-045** — Les tuiles **solides** doivent pouvoir projeter une **ombre**
  portée, purement visuelle, sur le fond du niveau en mode Texture, pour distinguer visuellement le
  physique du décor sans aucun effet sur le gameplay (`EX-ARCH-012`). Concrétisé en `LOT-46`.
- \anchor EX-REN-046 **EX-REN-046** — Le jeu doit permettre de basculer, par une commande **fixe et
  non remappable** (`F8`), entre le rendu **Physique** (couleur plate par type de tuile, accès direct
  à la lecture des collisions) et le rendu **Texture** (habillage complet — fond, skin, objets
  interactifs) — disponible aussi bien en édition qu'en jeu réel. Le rendu par défaut dépend de la
  configuration de build (`core::kDeveloperBuild`) : Physique en Debug, Texture en Release.
  Concrétisé en `LOT-41`.

## 3. Boucle & temps
- \anchor EX-REN-020 **EX-REN-020** — Le jeu doit tourner à **60 images/seconde** cible.
- \anchor EX-REN-021 **EX-REN-021** — La logique doit être mise à jour à **pas de temps fixe** (simulation déterministe), le rendu pouvant être découplé.
- \anchor EX-REN-022 **EX-REN-022** — Le rendu doit synchroniser la présentation (V-Sync activable) pour éviter le *tearing*.
- \anchor EX-REN-004 **EX-REN-004** — La présentation doit utiliser le **modèle flip** de DXGI
  (`DXGI_SWAP_EFFECT_FLIP_DISCARD`, au moins deux back buffers) plutôt que l'ancien modèle *blt*
  (`DISCARD`) : sous Windows 10/11, le flip model présente le back buffer **sans copie
  supplémentaire** (compositeur DWM), ce qui réduit la latence entrée → image et régularise la
  cadence, V-Sync activée comprise (`EX-REN-022`). Concrétisé en `LOT-33`.

## 4. Interface (HMI)
- \anchor EX-REN-030 **EX-REN-030** — Le jeu doit afficher un **menu principal** (Jouer, Quitter).
- \anchor EX-REN-031 **EX-REN-031** (⚠️ non implémenté) — Le jeu doit afficher un écran de **pause** et un écran de **fin de niveau**. En l'état, Échap **quitte directement** vers le menu (pas d'écran de pause dédié) et l'enchaînement de niveaux à la réussite ne passe par aucun écran intermédiaire.
- \anchor EX-REN-032 **EX-REN-032** — Le jeu doit afficher du **texte** (titres, indications) via une police bitmap ou vectorielle.
- \anchor EX-REN-033 **EX-REN-033** — Tout **texte affiché** doit passer par un **catalogue de traduction** : le code référence des **clés** stables, résolues vers une chaîne selon la **langue active**, chargée depuis un **fichier par langue** (français par défaut). Aucun libellé d'interface n'est codé en dur, afin de rendre l'ajout d'une langue trivial (un fichier de plus, sans modification du code). Une clé ou un fichier de langue manquant est traité comme une **erreur récupérable** (repli déterministe), cf. `EX-NFR-040`.

## 5. Audio (⚠️ minimal MVP)
- \anchor EX-REN-040 **EX-REN-040** (⚠️ souhaité) — Le jeu devrait jouer des **bruitages** (saut, interrupteur, victoire, échec).

## Traçabilité
Tout ce qui touche fenêtre, rendu, entrées et interface relève de `Source/HMI` ; la logique de simulation reste dans `Source/Core`. Contraintes de performance : [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md).
