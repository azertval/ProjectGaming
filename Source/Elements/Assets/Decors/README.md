# Elements/Assets/Decors/

Éléments de décor libres, posés sans contrainte de grille (`LOT-49`, `EX-DEC-001`).

Déposer un fichier ici suffit à le rendre sélectionnable dans le sélecteur de décors du panneau
« Outils » de l'éditeur (outil **Décor**) : la liste est peuplée par **balayage de ce dossier**,
jamais par saisie d'un chemin (même patron que `Skins/`/`Objects/`/`Backgrounds/`).

Dimensions **libres** (`hmi::AssetFamily::Decor`, `EX-REN-007`) : contrairement à un skin de tuile
ou un objet interactif, un décor n'est jamais découpé en grille — l'image entière est affichée à sa
taille réelle (mise à l'échelle par `core::Decor::scale`). Un fichier introuvable ou illisible
retombe sur le damier magenta, avec un avertissement journalisé nommant l'asset (`EX-NFR-040`) —
contrairement au fond de niveau, un décor **désigné** est toujours censé exister.

## Contenu actuel : décors de **test**

Cinq décors, **schématiques**, servent à vérifier le placement, la superposition par couche et la
traversabilité des décors, pas à habiller le jeu. Générés par script, donc reproductibles et
modifiables sans éditeur d'image :

```
python scripts/generate_test_decors.py
```

| Fichier | Dimensions | Couche prévue |
|---|---|---|
| `bush.png` | 22×18 | arrière-plan |
| `rock.png` | 20×14 | arrière-plan |
| `cloud.png` | 32×14 | arrière-plan |
| `branch.png` | 40×12 | premier plan |
| `sign.png` | 14×20 | premier plan |

Un artiste les remplacera par les vrais assets, sans toucher au code.

## Décors `kenney_*.png` : premiers décors réels, sous licence libre (`LOT-65`)

Quatre décors, à la différence des cinq ci-dessus, sont de véritables illustrations retouchées
depuis des sprites **Kenney** (www.kenney.nl) sous licence **CC0 1.0 Universal** — voir
`../CREDITS.md` pour le détail. Aucun jeu (`skins.json` ne structure pas `Decors/` en `jeux`) : ils
sont sélectionnables directement, au même titre que les décors de test.

| Fichier | Dimensions | Couche prévue |
|---|---|---|
| `kenney_fence.png` | 32×24 | premier plan |
| `kenney_torch.png` | 14×32 | premier plan |
| `kenney_chain.png` | 9×32 | premier plan |
| `kenney_ladder.png` | 16×16 | arrière-plan |
