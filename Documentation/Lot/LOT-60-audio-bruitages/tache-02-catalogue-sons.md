# TACHE-02 — Catalogue de sons et repli silencieux {#lot-60-tache-02-catalogue-sons}

**Lot :** [LOT-60](epic.md) · **Emplacement :** `Source/HMI/Audio`, `Source/Elements/Audio` ·
**Statut :** non commencé

## Contexte
Le projet a un patron bien rodé pour associer un nom logique à un fichier d'asset : `SkinCatalog`
(`LOT-42`), `AnimationCatalog` (`LOT-46`) et `PixelPalette` (`LOT-54`) lisent tous un JSON, traitent
le fichier absent comme un catalogue vide et **ignorent une entrée malformée plutôt que d'abandonner
le catalogue entier**. Le catalogue de sons suit ce patron ; l'écrire autrement serait la seule
erreur possible ici.

Le repli, en revanche, diffère. Pour une texture manquante, `LOT-40` produit un damier magenta :
visible, donc signalant. Pour un son manquant, l'équivalent — un bip de remplacement — serait
insupportable. Le repli d'un son absent est le **silence**, plus une entrée de journal.

## Travail à réaliser
- **`hmi::SoundCatalog`** : nom logique d'événement → fichier de `Source/Elements/Audio`, lu depuis
  un `sounds.json`, sur le patron exact de `hmi::SkinCatalog`.
- **Préchargement au démarrage** de tous les échantillons référencés, pour éviter le premier
  déclenchement muet (cf. `TACHE-01`).
- **Repli silencieux** : événement sans entrée, fichier absent, fichier illisible → aucun son,
  un avertissement journalisé **une seule fois** par asset, jamais à chaque déclenchement.
- **Assets de bruitages** dans `Source/Elements/Audio/` : WAV PCM courts, couvrant les événements
  de `EX-REN-040` et ceux de la `TACHE-03`. Sons **libres de droit** sélectionnés dans un dépôt
  réputé (licence CC0 ou CC-BY), pas de génération procédurale — un vrai bruitage vaut mieux qu'un
  bip synthétique pour un jeu destiné à être joué, pas seulement testé.
- **Licence et attribution des sons** documentées dans `Source/Elements/Audio/CREDITS.md` : pour
  chaque fichier, l'auteur, la source et la licence — sur le modèle de la police Inter
  (`Source/Elements/Assets/Fonts/Inter-LICENSE.txt`), et **obligatoire** même pour du CC0 dès lors
  que ce lot s'engage à créditer les auteurs.
- **Politique de recouvrement** : un même événement déclenché en rafale ne doit ni saturer ni
  couper le son précédent de façon audible — limiter le nombre d'instances simultanées par
  événement, avec une constante nommée.

## Fichiers impactés
- `Source/HMI/Audio/SoundCatalog.{h,cpp}` (nouveau).
- `Source/Elements/Audio/sounds.json` et les fichiers WAV (nouveaux).
- `Source/Elements/Audio/README.md`.
- `Source/Elements/Audio/CREDITS.md` (nouveau) — auteur, source et licence par fichier.
- `Source/HMI/CMakeLists.txt` — copie du dossier `Audio/` à côté de l'exécutable, comme `Levels/` et
  `Localization/`.
- `Source/Test/Unit/HMI/Audio/test_sound_catalog.cpp` (nouveau), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Catalogue **absent** → catalogue vide, aucune erreur.
- Entrée **malformée** → cette entrée seule est ignorée, les autres restent chargées (c'est la
  garantie que donne déjà `SkinCatalog`, et le test qui la vérifie).
- Événement **inconnu** → aucun son, aucune exception.
- Le catalogue **livré** référence des fichiers qui existent tous — le test qui attrape une faute de
  frappe dans le contenu.
- Tests **purs**, sans Qt Multimedia ni périphérique : le catalogue est une résolution de chemins,
  la lecture appartient à `AudioEngine`.

## Points d'attention
- **Ne pas journaliser à chaque déclenchement.** Un son manquant sur un événement fréquent (le pas,
  le saut) inonderait le journal et masquerait tout le reste. Une fois par asset, comme le fait déjà
  `hmi::SpriteRenderer` pour ses statistiques.
- **Séparer résolution et lecture** : le catalogue reste testable sans périphérique. C'est la même
  séparation que `SkinCatalog` (pur) vs `TextureCache` (GPU), et elle a déjà prouvé sa valeur.
- Attention au piège de chemin déjà rencontré avec `TextureCache` : si un asset est rangé dans un
  sous-dossier, **toutes** les fonctions qui le désignent doivent recevoir le même chemin, préfixe
  compris. Les tests « sans périphérique » ne couvrent pas cette glu.
- Le dossier `Audio/` doit être copié à côté de l'exécutable par le `POST_BUILD`, sans quoi la
  release est muette alors que le build local est sonore — panne classique et tardive.
- Des WAV, même courts, sont des binaires versionnés : garder les fichiers petits, et noter que
  `Source/Elements/Audio/README.md` les évoquait déjà comme candidats à Git LFS.
- Vérifier la licence de **chaque** son avant de l'intégrer : le CC-BY exige l'attribution
  (`CREDITS.md` suffit), certaines licences interdisent la redistribution ou l'usage commercial —
  écarter ces dernières plutôt que de les intégrer avec une réserve.

## Définition de fait (DoD)
- Un catalogue JSON associe événements et fichiers sur le patron des catalogues existants, les
  échantillons sont préchargés, tout manque se traduit par un silence journalisé une fois, les
  assets sont présents, générables et licenciés, le dossier est déployé ; `/W4 /WX` propre.

## Exigences
`EX-REN-047` (socle audio), `EX-REN-048` (absence de son en erreur récupérable) ; lève `EX-REN-040`
pour sa partie contenu ; réutilise `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable sans
périphérique), `EX-NFR-031` (assets et licences).
