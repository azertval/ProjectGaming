#!/usr/bin/env python3
"""Genere une spritesheet de personnage de TEST pour ProjectGaming (LOT-48, livree en LOT-65).

`Player/` ne contenait jusqu'ici aucun fichier : le personnage retombait systematiquement sur la
silhouette procedurale historique (`atlas.png`) faute d'un seul asset livre, alors que le contrat
(`hmi::AssetFamily::CharacterSheet`, `Documentation/Lot/LOT-48-personnage-texture/`) est complet
depuis longtemps. Ce script comble ce trou avec un personnage schematique, dans le meme esprit que
les autres assets de test du depot (generes par script, reproductibles, sans dependance externe).

Une seule spritesheet horizontale, une image par case de `TILE` pixels, accompagnee de
`player.anim.json` (`hmi::AnimationCatalog`) declarant les sept clips attendus
(`idle`, `run`, `jump`, `fall`, `land`, `wallslide`, `dash`) -- meme patron que
generate_mechanism_animations.py pour les clips multiples dans un seul fichier. Le personnage est
dessine dans le seul sens attendu (vers la DROITE) : le moteur retourne l'image pour l'autre sens.

Usage :
    python scripts/generate_test_player.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Player/.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une case, en pixels (hmi::TextureAtlas::TILE_SIZE) -- une image par case, comme les
autres skins animes de single case (LOT-46/LOT-47), plutot qu'un format dedie au personnage."""

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)

SKIN: Color = (232, 190, 150, 255)
HAIR: Color = (90, 60, 40, 255)
BODY: Color = (90, 140, 210, 255)
BODY_DARK: Color = (58, 96, 156, 255)
LEG: Color = (64, 74, 104, 255)
LEG_DARK: Color = (40, 48, 72, 255)
OUTLINE: Color = (30, 32, 42, 255)
TRAIL: Color = (200, 220, 245, 140)


class Image:
    """Bitmap RGBA simple, origine en haut a gauche (repris de generate_test_skins.py)."""

    def __init__(self, width: int, height: int, fill: Color = TRANSPARENT) -> None:
        self.width = width
        self.height = height
        self.pixels: list[Color] = [fill] * (width * height)

    def set(self, x: int, y: int, color: Color) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def fill_rect(self, x: int, y: int, width: int, height: int, color: Color) -> None:
        for row in range(y, y + height):
            for column in range(x, x + width):
                self.set(column, row, color)

    def blit(self, source: "Image", x: int, y: int) -> None:
        for row in range(source.height):
            for column in range(source.width):
                self.set(x + column, y + row, source.pixels[row * source.width + column])

    def to_png(self) -> bytes:
        """Encode l'image en PNG (RGBA 8 bits, sans filtrage)."""
        raw = bytearray()
        for row in range(self.height):
            raw.append(0)  # type de filtre 0 (aucun) : suffisant a cette taille
            for column in range(self.width):
                raw.extend(self.pixels[row * self.width + column])

        def chunk(tag: bytes, data: bytes) -> bytes:
            body = tag + data
            return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

        header = struct.pack(">IIBBBBB", self.width, self.height, 8, 6, 0, 0, 0)
        return (
            b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", header)
            + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
            + chunk(b"IEND", b"")
        )


def horizontal_sheet(frames: list[Image]) -> Image:
    """Assemble des images de meme taille en une spritesheet horizontale (un seul rang)."""
    sheet = Image(TILE * len(frames), TILE)
    for index, frame in enumerate(frames):
        sheet.blit(frame, index * TILE, 0)
    return sheet


def head_and_torso(image: Image, bob: int, lean: int) -> None:
    """Tete + buste, communs a la plupart des poses. `bob` decale verticalement (respiration,
    foulee), `lean` decale horizontalement (penche vers l'avant, dans le sens de la course)."""
    image.fill_rect(6 + lean, 1 + bob, 4, 4, SKIN)
    image.fill_rect(6 + lean, 1 + bob, 4, 1, HAIR)
    image.fill_rect(5 + lean, 5 + bob, 6, 6, BODY)
    image.fill_rect(5 + lean, 5 + bob, 1, 6, BODY_DARK)
    image.fill_rect(10 + lean, 5 + bob, 1, 6, BODY_DARK)


def idle_frame(phase: int) -> Image:
    """Respiration : le buste monte/descend d'un pixel, jambes jointes."""
    bob = -1 if phase == 1 else 0
    image = Image(TILE, TILE)
    head_and_torso(image, bob, 0)
    image.fill_rect(5, 11 + bob, 2, 4, LEG)
    image.fill_rect(9, 11 + bob, 2, 4, LEG)
    image.fill_rect(5, 14 + bob, 2, 1, LEG_DARK)
    image.fill_rect(9, 14 + bob, 2, 1, LEG_DARK)
    return image


def run_frame(phase: int) -> Image:
    """Foulee de course : jambes et bras alternent, quatre phases pour un cycle complet."""
    image = Image(TILE, TILE)
    head_and_torso(image, 0, 1)
    stride = [(-2, 2), (0, 0), (2, -2), (0, 0)][phase % 4]
    front, back = stride
    image.fill_rect(6 + front, 11, 2, 4, LEG)
    image.fill_rect(6 + back, 11, 2, 4, LEG_DARK)
    image.fill_rect(4 + front // 2, 6, 2, 4, BODY_DARK)  # bras avant
    return image


def jump_frame() -> Image:
    """Envol : jambes repliees sous le buste, un bras leve."""
    image = Image(TILE, TILE)
    head_and_torso(image, -1, 0)
    image.fill_rect(5, 10, 2, 3, LEG)
    image.fill_rect(9, 10, 2, 3, LEG)
    image.fill_rect(10, 3, 2, 4, BODY_DARK)  # bras leve
    return image


def fall_frame() -> Image:
    """Chute : jambes et bras ecartes, silhouette en X, lisible en un coup d'oeil."""
    image = Image(TILE, TILE)
    head_and_torso(image, 0, 0)
    image.fill_rect(3, 11, 3, 2, LEG)
    image.fill_rect(10, 11, 3, 2, LEG)
    image.fill_rect(2, 5, 3, 2, BODY_DARK)
    image.fill_rect(11, 5, 3, 2, BODY_DARK)
    return image


def land_frame(phase: int) -> Image:
    """Reception : accroupi (phase 0) puis redresse (phase 1, identique a idle immobile)."""
    if phase == 0:
        image = Image(TILE, TILE)
        image.fill_rect(6, 4, 4, 4, SKIN)
        image.fill_rect(6, 4, 4, 1, HAIR)
        image.fill_rect(4, 8, 8, 5, BODY)
        image.fill_rect(4, 8, 1, 5, BODY_DARK)
        image.fill_rect(11, 8, 1, 5, BODY_DARK)
        image.fill_rect(4, 13, 3, 2, LEG)
        image.fill_rect(9, 13, 3, 2, LEG)
        return image
    return idle_frame(0)


def wallslide_frame() -> Image:
    """Glissade murale : corps plaque contre le bord droit, bras tendu vers la paroi."""
    image = Image(TILE, TILE)
    head_and_torso(image, 0, -1)
    image.fill_rect(4, 11, 2, 4, LEG)
    image.fill_rect(7, 12, 2, 3, LEG_DARK)
    image.fill_rect(11, 6, 3, 2, BODY_DARK)  # bras plaque a droite
    image.fill_rect(13, 4, 1, 10, OUTLINE)  # paroi, repere visuel
    return image


def dash_frame() -> Image:
    """Ruee : corps incline vers l'avant, trainee horizontale derriere (sens oppose au regard)."""
    image = Image(TILE, TILE)
    head_and_torso(image, 0, 2)
    image.fill_rect(7, 12, 2, 3, LEG)
    image.fill_rect(9, 12, 2, 3, LEG_DARK)
    for i, width in enumerate((6, 4, 2)):
        image.fill_rect(0, 6 + i * 2, width, 1, TRAIL)
    return image


def player_sheet() -> Image:
    return horizontal_sheet(
        [
            idle_frame(0),  # 0 : idle
            idle_frame(1),  # 1 : idle
            run_frame(0),  # 2 : run
            run_frame(1),  # 3 : run
            run_frame(2),  # 4 : run
            run_frame(3),  # 5 : run
            jump_frame(),  # 6 : jump
            fall_frame(),  # 7 : fall
            land_frame(0),  # 8 : land (accroupi)
            land_frame(1),  # 9 : land (redresse)
            wallslide_frame(),  # 10 : wallslide
            dash_frame(),  # 11 : dash
        ]
    )


PLAYER_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "idle": {"frames": [0, 1], "frameDuration": 0.5, "loop": True},
        "run": {"frames": [2, 3, 4, 5], "frameDuration": 0.1, "loop": True},
        "jump": {"frames": [6], "loop": True},
        "fall": {"frames": [7], "loop": True},
        "land": {"frames": [8, 9], "frameDuration": 0.08, "loop": False, "next": "idle"},
        "wallslide": {"frames": [10], "loop": True},
        "dash": {"frames": [11], "loop": True},
    },
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent
        / "Source"
        / "Elements"
        / "Assets"
        / "Player",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    image = player_sheet()
    (arguments.output / "player.png").write_bytes(image.to_png())
    (arguments.output / "player.anim.json").write_text(
        json.dumps(PLAYER_JSON, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    print(f"player.png : {image.width}x{image.height} + player.anim.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
