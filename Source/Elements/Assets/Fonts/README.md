# Elements/Assets/Fonts/

Police bitmap utilisée pour le texte affiché **dans la scène rendue** (HUD, `LOT-52`,
`EX-REN-032`) : `hmi::BitmapFont` cherche ici, au démarrage, un atlas de glyphes accompagné de
ses métriques.

## Fichiers attendus

- `font.png` — atlas de glyphes (`hmi::AssetFamily::Font`, dimensions libres, découpées par les
  métriques plutôt que par une grille de cases).
- `font.json` — métriques : la région (en pixels) et l'avance horizontale de chaque caractère
  couvert.

```json
{
  "version": 1,
  "lineHeight": 10,
  "replacement": "?",
  "glyphs": [
    { "char": "A", "x": 0, "y": 0, "width": 6, "height": 10, "advance": 6 }
  ]
}
```

`"replacement"` est optionnel (`?` par défaut) : c'est le glyphe substitué à un caractère non
couvert (`EX-NFR-040`), jamais un trou silencieux ni un plantage. Chaque entrée de `"glyphs"`
désigne exactement **un** caractère (`"char"`), en UTF-8 — les lettres accentuées du français
(`é è à ç ù ê î ô û`, minuscules et majuscules) doivent être couvertes, faute de quoi le HUD en
français serait illisible.

## Absence d'asset : repli procédural

Aucun fichier n'est déposé dans ce dossier pour l'instant : `hmi::BitmapFont` retombe sans
plantage sur une police procédurale minimale (`hmi::buildProceduralFont`, glyphes 5×7 pixels,
ASCII imprimable + accents français), sur le modèle de `atlas.png`/`hmi::buildProceduralAtlasImage`
(`LOT-39`). Le jeu reste lisible même sans aucun asset de police (`EX-NFR-040`).

Un artiste pourra déposer `font.png` + `font.json` ici sans toucher au code : dès que les deux
fichiers sont présents, valides (contrat d'asset, `EX-REN-007`) et cohérents entre eux, ils
remplacent le repli procédural au démarrage suivant.

## Police de l'interface hors-jeu (`LOT-56`)

`Inter-Regular.ttf` et `Inter-Bold.ttf` sont la police **embarquée de l'IHM Qt** (menus, panneaux,
boîtes de dialogue) — sans rapport avec la police bitmap du HUD ci-dessus, décrite plus haut : deux
mécanismes distincts (`hmi::ApplicationTheme` pour l'un, `hmi::BitmapFont`/`hmi::ProceduralFont`
pour l'autre), deux jeux de fichiers distincts. Enregistrées auprès de `QFontDatabase` au démarrage
(`hmi::ApplicationTheme::applyEditorTheme`) ; famille couvrant les caractères accentués français du
catalogue de traduction (`EX-REN-033`) et lisible aux petites tailles des libellés de panneaux.

**Repli** : si l'un des deux fichiers est absent ou refusé par Qt, l'application retombe sur une
famille générique demandée à Qt (jamais un second nom de police codé en dur), en journalisant un
avertissement — même garantie que le repli procédural du HUD (`EX-NFR-040`).

**Licence** : [Inter](https://github.com/rsms/inter) est distribuée sous licence
[SIL Open Font License 1.1](https://scripts.sil.org/OFL), redistribuable avec l'application ;
texte complet dans `Inter-LICENSE.txt`, à côté des fichiers.
