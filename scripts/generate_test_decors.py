#!/usr/bin/env python3
"""Genere des textures de decor de TEST pour ProjectGaming (LOT-49).

Ces images sont volontairement schematiques : elles servent a verifier le placement, la
superposition par couche et la traversabilite des decors, pas a habiller le jeu. Un artiste les
remplacera par les vrais assets. Meme mecanique d'encodage PNG que generate_test_objects.py/
generate_test_backgrounds.py (aucune dependance externe). Contrairement aux objets/skins, un decor
n'est jamais contraint a une grille de cases (`hmi::AssetFamily::Decor`, dimensions libres,
`EX-REN-007`) : les dimensions choisies ci-dessous le montrent delibrement.

Usage :
    python scripts/generate_test_decors.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Decors/.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import zlib

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)


class Image:
    """Bitmap RGBA simple, origine en haut a gauche."""

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


def bush() -> Image:
    """Buisson : decor d'ARRIERE-PLAN (derriere les tuiles), dimensions libres (22x18)."""
    leaves: Color = (46, 120, 58, 255)
    outline: Color = (24, 70, 32, 255)
    highlight: Color = (90, 170, 100, 255)

    image = Image(22, 18, TRANSPARENT)
    image.fill_rect(1, 4, 20, 13, leaves)
    image.fill_rect(3, 1, 16, 5, leaves)
    image.fill_rect(0, 6, 22, 1, outline)
    image.fill_rect(4, 2, 10, 2, highlight)
    return image


def branch() -> Image:
    """Branche : decor de PREMIER PLAN (devant le personnage), dimensions libres (40x12)."""
    wood: Color = (92, 62, 34, 255)
    dark: Color = (54, 34, 16, 255)
    leaf: Color = (58, 140, 70, 255)

    image = Image(40, 12, TRANSPARENT)
    image.fill_rect(0, 4, 40, 3, wood)
    image.fill_rect(0, 6, 40, 1, dark)
    for x in (6, 16, 28):
        image.fill_rect(x, 0, 5, 5, leaf)
        image.fill_rect(x + 1, 7, 4, 4, leaf)
    return image


def rock() -> Image:
    """Rocher : decor d'ARRIERE-PLAN, amas de blocs gris, dimensions libres (20x14)."""
    stone: Color = (110, 108, 118, 255)
    shadow: Color = (72, 70, 80, 255)
    highlight: Color = (150, 148, 158, 255)

    image = Image(20, 14, TRANSPARENT)
    image.fill_rect(2, 6, 16, 8, stone)
    image.fill_rect(0, 9, 20, 5, stone)
    image.fill_rect(0, 12, 20, 2, shadow)
    image.fill_rect(3, 6, 6, 2, highlight)
    return image


def cloud() -> Image:
    """Nuage : decor d'ARRIERE-PLAN, silhouette bombee claire, dimensions libres (32x14)."""
    body: Color = (240, 244, 250, 220)
    shadow: Color = (206, 214, 226, 200)

    image = Image(32, 14, TRANSPARENT)
    image.fill_rect(4, 6, 24, 6, body)
    image.fill_rect(8, 2, 12, 6, body)
    image.fill_rect(18, 3, 10, 6, body)
    image.fill_rect(4, 10, 24, 2, shadow)
    return image


def sign() -> Image:
    """Panneau : decor de PREMIER PLAN, poteau + planche, dimensions libres (14x20)."""
    post: Color = (92, 62, 34, 255)
    board: Color = (176, 138, 84, 255)
    board_dark: Color = (128, 96, 56, 255)

    image = Image(14, 20, TRANSPARENT)
    image.fill_rect(6, 8, 2, 12, post)
    image.fill_rect(0, 0, 14, 9, board)
    image.fill_rect(0, 0, 14, 2, board_dark)
    image.fill_rect(0, 7, 14, 2, board_dark)
    return image


def mushroom() -> Image:
    """Champignon : decor d'ARRIERE-PLAN pour les tableaux souterrains (16x18)."""
    cap: Color = (178, 62, 58, 255)
    cap_dark: Color = (120, 36, 34, 255)
    spot: Color = (238, 226, 210, 255)
    stem: Color = (222, 210, 188, 255)
    stem_dark: Color = (170, 158, 138, 255)

    image = Image(16, 18, TRANSPARENT)
    image.fill_rect(6, 8, 4, 10, stem)
    image.fill_rect(9, 8, 1, 10, stem_dark)
    image.fill_rect(1, 4, 14, 5, cap)
    image.fill_rect(3, 1, 10, 4, cap)
    image.fill_rect(1, 8, 14, 1, cap_dark)
    image.fill_rect(4, 3, 3, 2, spot)
    image.fill_rect(10, 5, 2, 2, spot)
    return image


def crystal() -> Image:
    """Cristal : decor d'ARRIERE-PLAN, prisme clair pour grottes (14x22)."""
    body: Color = (108, 176, 214, 235)
    bright: Color = (176, 224, 246, 235)
    dark: Color = (56, 104, 148, 235)

    image = Image(14, 22, TRANSPARENT)
    # Prisme central : largeur croissante puis decroissante, en bandes horizontales.
    for y in range(22):
        half = 1 + min(y, 21 - y) * 6 // 11
        image.fill_rect(7 - half, y, half * 2, 1, body)
    image.fill_rect(5, 4, 2, 14, bright)
    image.fill_rect(8, 6, 2, 12, dark)
    return image


def stalactite() -> Image:
    """Stalactite : decor de PREMIER PLAN, pointe suspendue au plafond (12x26)."""
    stone: Color = (96, 92, 104, 255)
    shadow: Color = (60, 58, 68, 255)
    highlight: Color = (136, 132, 146, 255)

    image = Image(12, 26, TRANSPARENT)
    for y in range(26):
        half = max(1, 6 - y * 6 // 25)
        image.fill_rect(6 - half, y, half * 2, 1, stone)
    image.fill_rect(3, 0, 2, 16, highlight)
    image.fill_rect(7, 2, 2, 14, shadow)
    return image


def vine() -> Image:
    """Liane : decor de PREMIER PLAN, retombee verticale feuillue (12x34)."""
    stalk: Color = (58, 108, 52, 255)
    leaf: Color = (82, 148, 72, 255)
    leaf_dark: Color = (44, 88, 42, 255)

    image = Image(12, 34, TRANSPARENT)
    image.fill_rect(5, 0, 2, 34, stalk)
    for y in range(3, 32, 7):
        image.fill_rect(1, y, 4, 3, leaf)
        image.fill_rect(1, y + 2, 4, 1, leaf_dark)
        image.fill_rect(7, y + 3, 4, 3, leaf)
        image.fill_rect(7, y + 5, 4, 1, leaf_dark)
    return image


def grass_tuft() -> Image:
    """Touffe d'herbe : decor d'ARRIERE-PLAN, petit remplissage de sol (18x10)."""
    blade: Color = (74, 148, 64, 255)
    blade_dark: Color = (46, 104, 44, 255)

    image = Image(18, 10, TRANSPARENT)
    for index, x in enumerate((1, 4, 7, 10, 13, 16)):
        height = 6 + (index % 3) * 2
        image.fill_rect(x, 10 - height, 2, height, blade if index % 2 == 0 else blade_dark)
    image.fill_rect(0, 9, 18, 1, blade_dark)
    return image


def lantern() -> Image:
    """Lanterne : decor de PREMIER PLAN, suspendue, halo clair (14x24)."""
    metal: Color = (86, 80, 70, 255)
    glass: Color = (250, 224, 150, 255)
    glow: Color = (250, 236, 190, 120)

    image = Image(14, 24, TRANSPARENT)
    image.fill_rect(6, 0, 2, 7, metal)     # chaine
    image.fill_rect(3, 7, 8, 2, metal)     # chapeau
    image.fill_rect(4, 9, 6, 10, glass)    # verre
    image.fill_rect(3, 9, 1, 10, metal)    # montants
    image.fill_rect(10, 9, 1, 10, metal)
    image.fill_rect(3, 19, 8, 2, metal)    # socle
    image.fill_rect(2, 11, 10, 6, glow)    # halo, volontairement semi-transparent
    return image


def pillar() -> Image:
    """Pilier : decor d'ARRIERE-PLAN, colonne de pierre (20x40)."""
    stone: Color = (128, 124, 132, 255)
    dark: Color = (86, 82, 90, 255)
    light: Color = (162, 158, 168, 255)

    image = Image(20, 40, TRANSPARENT)
    image.fill_rect(4, 4, 12, 32, stone)
    image.fill_rect(1, 0, 18, 4, stone)     # chapiteau
    image.fill_rect(1, 36, 18, 4, stone)    # base
    image.fill_rect(4, 4, 3, 32, light)
    image.fill_rect(13, 4, 3, 32, dark)
    for y in range(8, 36, 7):
        image.fill_rect(4, y, 12, 1, dark)  # joints
    return image


def gear() -> Image:
    """Engrenage : decor d'ARRIERE-PLAN, note industrielle (24x24)."""
    metal: Color = (128, 118, 106, 255)
    dark: Color = (82, 74, 66, 255)

    image = Image(24, 24, TRANSPARENT)
    for y in range(24):
        for x in range(24):
            distance = (x - 11.5) ** 2 + (y - 11.5) ** 2
            if distance <= 9.5**2:
                image.set(x, y, metal)
            if distance <= 3.5**2:
                image.set(x, y, TRANSPARENT)
    for x, y in ((10, 0), (10, 20), (0, 10), (20, 10)):
        image.fill_rect(x, y, 4, 4, metal)
    for y in range(24):
        for x in range(24):
            distance = (x - 11.5) ** 2 + (y - 11.5) ** 2
            if 8.0**2 <= distance <= 9.5**2:
                image.set(x, y, dark)
    return image


DECORS = {
    "bush.png": bush,
    "branch.png": branch,
    "rock.png": rock,
    "cloud.png": cloud,
    "sign.png": sign,
    "mushroom.png": mushroom,
    "crystal.png": crystal,
    "stalactite.png": stalactite,
    "vine.png": vine,
    "grass_tuft.png": grass_tuft,
    "lantern.png": lantern,
    "pillar.png": pillar,
    "gear.png": gear,
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
        / "Decors",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, build in DECORS.items():
        image = build()
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")

    print(f"\n{len(DECORS)} textures de decor de test ecrites dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
