# Elements/Assets/Player/

Spritesheet du personnage (`LOT-48`) : voir la section « `Player/` — spritesheet du personnage »
du `README.md` parent pour le format attendu (`player.png` + `player.anim.json`), l'ancrage
centre-bas et la liste des clips.

## Contenu actuel : un personnage de **test** (`LOT-65`)

`player.png` (192×16, douze cases de 16×16) est **schématique** — une silhouette bleue simplifiée,
tournée vers la droite comme l'exige le contrat — et sert à vérifier que les sept clips attendus
s'enchaînent correctement, pas à habiller le jeu. Générée par script, donc reproductible et
modifiable sans éditeur d'image :

```
python scripts/generate_test_player.py
```

| Clip | Cases | Sert à vérifier |
|---|---|---|
| `idle` | 0–1 | respiration en boucle |
| `run` | 2–5 | foulée de course, quatre phases |
| `jump` | 6 | pose d'envol |
| `fall` | 7 | pose de chute, distincte du saut |
| `land` | 8–9 | transition jouée une fois (accroupi → redressé), puis `next: idle` |
| `wallslide` | 10 | glissade murale |
| `dash` | 11 | ruée, trainée horizontale |

Avant ce lot, aucun fichier n'était livré ici et le jeu retombait sur la silhouette procédurale
historique (`atlas.png`). Ce repli reste actif si le fichier est absent ou invalide — un artiste
remplacera `player.png` par le vrai personnage, sans toucher au code, exactement comme pour
`Skins/`/`Backgrounds/`/`Objects/`.
