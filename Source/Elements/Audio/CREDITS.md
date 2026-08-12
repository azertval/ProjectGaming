# Crédits audio

Tous les bruitages de `Source/Elements/Audio/` sont issus de packs **Kenney** (www.kenney.nl),
publiés sous licence **CC0 1.0 Universal** (domaine public) :
<http://creativecommons.org/publicdomain/zero/1.0/>.

Le CC0 n'exige aucune attribution, mais ce lot (`LOT-60`) s'engage à créditer les auteurs malgré
tout. Chaque fichier est renommé (nom de l'événement, cf. `sounds.json`) ; le nom d'origine et le
pack source sont conservés ci-dessous pour la traçabilité.

| Fichier livré              | Nom d'origine              | Pack Kenney         | Auteur | Licence |
|-----------------------------|-----------------------------|----------------------|--------|---------|
| `saut.wav`                  | `phaseJump1.ogg`            | Digital Audio        | Kenney | CC0 1.0 |
| `atterrissage.wav`          | `impactSoft_medium_000.ogg` | Impact Sounds        | Kenney | CC0 1.0 |
| `dash.wav`                  | `zap1.ogg`                  | Digital Audio        | Kenney | CC0 1.0 |
| `interrupteur.wav`          | `metalClick.ogg`            | RPG Audio            | Kenney | CC0 1.0 |
| `porte.wav`                 | `doorOpen_1.ogg`            | RPG Audio            | Kenney | CC0 1.0 |
| `plaque_pression.wav`       | `impactPlate_medium_000.ogg`| Impact Sounds        | Kenney | CC0 1.0 |
| `ramassage.wav`             | `powerUp1.ogg`               | Digital Audio        | Kenney | CC0 1.0 |
| `mort.wav`                  | `lowDown.ogg`                | Digital Audio        | Kenney | CC0 1.0 |
| `victoire_tableau.wav`      | `confirmation_002.ogg`       | Interface Sounds     | Kenney | CC0 1.0 |
| `fin_sequence.wav`          | `powerUp12.ogg`              | Digital Audio        | Kenney | CC0 1.0 |
| `menu_deplacement.wav`      | `click_002.ogg`               | Interface Sounds     | Kenney | CC0 1.0 |
| `menu_validation.wav`       | `confirmation_001.ogg`       | Interface Sounds     | Kenney | CC0 1.0 |

Sources des packs :
- Digital Audio — <https://kenney.nl/assets/digital-audio>
- Impact Sounds — <https://kenney.nl/assets/impact-sounds>
- RPG Audio — <https://kenney.nl/assets/rpg-audio>
- Interface Sounds — <https://kenney.nl/assets/interface-sounds>

Conversion : fichier source `.ogg` (Vorbis) ré-encodé en `.wav` PCM 16 bits, sans autre
retouche — `QSoundEffect` (Qt Multimedia) ne lit pas de format compressé (cf. `LOT-60 TACHE-01`).
