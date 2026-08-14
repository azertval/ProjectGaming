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


DECORS = {
    "bush.png": bush,
    "branch.png": branch,
    "rock.png": rock,
    "cloud.png": cloud,
    "sign.png": sign,
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
