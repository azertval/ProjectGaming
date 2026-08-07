#!/usr/bin/env python3
"""Genere des textures d'objets interactifs de TEST pour ProjectGaming (LOT-45).

Ces images sont volontairement schematiques : elles servent a verifier que la surcharge de
texture par instance fonctionne (assignation, priorite sur le skin), pas a habiller le jeu. Un
artiste les remplacera par les vrais assets. Meme mecanique d'encodage PNG que
generate_test_skins.py/generate_test_backgrounds.py (aucune dependance externe).

Usage :
    python scripts/generate_test_objects.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Objects/.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une case, en pixels (hmi::TextureAtlas::TILE_SIZE)."""

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


def door_red() -> Image:
    """Porte rouge : surcharge distincte de tout skin existant (LOT-42), bien reconnaissable."""
    panel: Color = (176, 40, 40, 255)
    frame: Color = (96, 16, 16, 255)
    handle: Color = (230, 200, 90, 255)

    cell = Image(TILE, TILE, panel)
    cell.fill_rect(0, 0, TILE, 2, frame)
    cell.fill_rect(0, TILE - 2, TILE, 2, frame)
    cell.fill_rect(0, 0, 2, TILE, frame)
    cell.fill_rect(TILE - 2, 0, 2, TILE, frame)
    cell.fill_rect(TILE - 5, TILE // 2 - 1, 2, 2, handle)
    return cell


def crate_blue() -> Image:
    """Caisse bleue : second exemple, pour verifier plusieurs surcharges distinctes cote a cote."""
    wood: Color = (60, 90, 150, 255)
    dark: Color = (30, 48, 84, 255)
    light: Color = (110, 150, 210, 255)

    cell = Image(TILE, TILE, wood)
    cell.fill_rect(0, 0, TILE, 1, light)
    cell.fill_rect(0, 0, 1, TILE, light)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    for i in range(TILE):
        cell.set(i, i, dark)
        cell.set(TILE - 1 - i, i, dark)
    return cell


OBJECTS = {
    "door_red.png": door_red,
    "crate_blue.png": crate_blue,
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
        / "Objects",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, build in OBJECTS.items():
        image = build()
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")

    print(f"\n{len(OBJECTS)} textures d'objets de test ecrites dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
