# Elements/Assets/Player/

Spritesheet du personnage (`LOT-48`) : voir la section « `Player/` — spritesheet du personnage »
du `README.md` parent pour le format attendu (`player.png` + `player.anim.json`), l'ancrage
centre-bas et la liste des clips.

## Contenu actuel : un personnage de **test** (`LOT-65`)

`player.png` (240×16, quinze cases de 16×16) est **schématique**, tournée vers la droite comme
l'exige le contrat, et sert à vérifier que les sept clips attendus s'enchaînent correctement.
Générée par script, donc reproductible et modifiable sans éditeur d'image :

```
python scripts/generate_test_player.py
```

| Clip | Cases | Sert à vérifier |
|---|---|---|
| `idle` | 0–2 | respiration en boucle, plus un clignement |
| `run` | 3–8 | foulée de course, six phases distinctes |
| `jump` | 9 | pose d'envol, silhouette étirée |
| `fall` | 10 | pose de chute, distincte du saut |
| `land` | 11–12 | transition jouée une fois (écrasé → redressé), puis `next: idle` |
| `wallslide` | 13 | glissade murale, étincelles de frottement |
| `dash` | 14 | ruée, corps horizontal et traînée |

### Refonte du second temps du `LOT-65`

La première version était illisible dès qu'elle passait devant une tuile de teinte voisine : des
aplats sans cerne, sans visage, et un cycle de course de quatre images dont **deux identiques** —
la foulée paraissait donc saccadée. La version actuelle ajoute :

- un **cerne** d'un pixel, calculé depuis la silhouette (`outline()`) plutôt que dessiné à la main,
  ce qui le garde juste quelle que soit la pose ;
- un **visage** (chevelure, œil orienté à droite, menton ombré), qui rend le sens de marche lisible
  sans attendre le déplacement ;
- un **ombrage** systématique (côté éclairé / côté à l'ombre) sur la tunique, les jambes et la tête ;
- des **bottes** et une **ceinture**, qui séparent les masses au lieu d'un bloc de couleur unique ;
- six phases de course réellement distinctes, les bras balançant en **hauteur** — un balancement
  horizontal les noyait dans le buste, et trois images sur six se retrouvaient sans bras ;
- de l'**écrasement/étirement** : silhouette étirée à l'envol, écrasée à la réception.

Avant ce lot, aucun fichier n'était livré ici et le jeu retombait sur la silhouette procédurale
historique (`atlas.png`). Ce repli reste actif si le fichier est absent ou invalide — un artiste
remplacera `player.png` par le vrai personnage, sans toucher au code, exactement comme pour
`Skins/`/`Backgrounds/`/`Objects/`.
