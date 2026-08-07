#!/usr/bin/env python3
"""Genere un jeu de skins de TEST pour ProjectGaming (LOT-42).

Ces images sont volontairement schematiques : elles servent a verifier que le moteur
d'habillage fonctionne (assignation, raccords automatiques, detourage des pentes),
pas a habiller le jeu. Un artiste les remplacera par les vrais assets.

Le script n'a AUCUNE dependance externe (PNG ecrit avec zlib + struct) : il tourne sur
n'importe quel poste et en CI, sans rien installer.

Usage :
    python scripts/generate_test_skins.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Skins/.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une case, en pixels (hmi::TextureAtlas::TILE_SIZE)."""

SHEET_SIDE = 4
"""Cases par cote d'une planche a raccords (hmi::AUTOTILE_SHEET_SIDE)."""

# Bits du masque de voisinage (hmi::NeighborBits). Un bit A 1 = voisin solide = AUCUN bord de ce
# cote. La case d'un masque est (masque % 4, masque // 4) : c'est la table de hmi::autotileCell,
# reproduite ici pour que la planche generee soit lisible par le moteur sans ajustement.
UP, RIGHT, DOWN, LEFT = 1, 2, 4, 8

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


def hatch(image: Image, color: Color, step: int = 4) -> None:
    """Pose une trame diagonale discrete, pour que la repetition d'un motif se voie."""
    for y in range(image.height):
        for x in range(image.width):
            if (x + y) % step == 0:
                image.set(x, y, color)


def stone_cell(mask: int) -> Image:
    """Une case de la planche de pierre, bordee sur les cotes SANS voisin solide."""
    interior: Color = (104, 104, 118, 255)
    edge: Color = (58, 58, 70, 255)
    top_light: Color = (162, 168, 186, 255)
    speckle: Color = (88, 88, 102, 255)

    cell = Image(TILE, TILE, interior)
    hatch(cell, speckle, step=5)

    # Bord franc de 2 px sur chaque cote ouvert : c'est ce qui distingue visuellement un bord d'un
    # interieur, et donc ce que les raccords automatiques doivent faire apparaitre.
    if not mask & UP:
        cell.fill_rect(0, 0, TILE, 3, top_light)  # dessus eclaire (lecture de plateforme)
        cell.fill_rect(0, 0, TILE, 1, edge)
    if not mask & DOWN:
        cell.fill_rect(0, TILE - 2, TILE, 2, edge)
    if not mask & LEFT:
        cell.fill_rect(0, 0, 2, TILE, edge)
    if not mask & RIGHT:
        cell.fill_rect(TILE - 2, 0, 2, TILE, edge)
    return cell


def stone_sheet() -> Image:
    """Planche 4x4 a raccords : une case par configuration de voisinage."""
    sheet = Image(TILE * SHEET_SIDE, TILE * SHEET_SIDE)
    for mask in range(SHEET_SIDE * SHEET_SIDE):
        column, row = mask % SHEET_SIDE, mask // SHEET_SIDE
        sheet.blit(stone_cell(mask), column * TILE, row * TILE)
    return sheet


def crate() -> Image:
    """Caisse en bois : skin `single` pour un bloc poussable."""
    wood: Color = (150, 100, 52, 255)
    dark: Color = (92, 58, 28, 255)
    light: Color = (190, 138, 82, 255)

    cell = Image(TILE, TILE, wood)
    cell.fill_rect(0, 0, TILE, 1, light)
    cell.fill_rect(0, 0, 1, TILE, light)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    # Diagonales de renfort : rendent l'orientation de la caisse lisible d'un coup d'oeil.
    for i in range(TILE):
        cell.set(i, i, dark)
        cell.set(TILE - 1 - i, i, dark)
    return cell


def spikes() -> Image:
    """Pics : skin `single` pour un danger. Fond transparent, pointes vers le haut."""
    metal: Color = (206, 66, 66, 255)
    shade: Color = (140, 34, 34, 255)

    tooth_width = 4
    tooth_count = TILE // tooth_width
    base_height = 3
    tip_height = TILE - base_height

    cell = Image(TILE, TILE)
    cell.fill_rect(0, TILE - base_height, TILE, base_height, shade)  # socle
    for tooth in range(tooth_count):
        for y in range(tip_height):
            # Largeur croissante de 1 px (pointe, en haut) a `tooth_width` (base, en bas).
            width = 1 + (y * (tooth_width - 1)) // (tip_height - 1)
            start_x = tooth * tooth_width + (tooth_width - width) // 2
            cell.fill_rect(start_x, y, width, 1, metal)
    return cell


def entry_icon() -> Image:
    """Point d'entree : arche verte, fleche entrante -- skin `single` pour `Entry`."""
    frame: Color = (40, 90, 40, 255)
    glow: Color = (96, 200, 96, 255)
    arrow: Color = (210, 244, 210, 255)

    cell = Image(TILE, TILE, TRANSPARENT)
    cell.fill_rect(1, 1, 2, TILE - 2, frame)  # montant gauche
    cell.fill_rect(TILE - 3, 1, 2, TILE - 2, frame)  # montant droit
    cell.fill_rect(1, 1, TILE - 2, 2, frame)  # linteau
    cell.fill_rect(4, 4, TILE - 8, TILE - 6, glow)  # lueur du passage
    for row in range(4):  # fleche pointant vers l'interieur (bas), largeur croissante
        width = 4 + row
        cell.fill_rect(6 - row // 2, 6 + row, width, 1, arrow)
    return cell


def exit_icon() -> Image:
    """Point de sortie : fanion dore hisse -- skin `single` pour `Exit`."""
    pole: Color = (90, 74, 40, 255)
    flag_light: Color = (240, 200, 64, 255)
    flag_dark: Color = (196, 156, 32, 255)

    cell = Image(TILE, TILE, TRANSPARENT)
    cell.fill_rect(2, 1, 2, TILE - 2, pole)  # hampe
    for row in range(6):  # fanion triangulaire, raye
        width = 8 - row
        cell.fill_rect(4, 1 + row, width, 1, flag_light if row % 2 == 0 else flag_dark)
    cell.fill_rect(1, TILE - 3, 6, 2, pole)  # socle
    return cell


def slope_block() -> Image:
    """Carre PLEIN destine aux pentes et arrondis.

    L'artiste peint un carre : le moteur le detoure automatiquement a la silhouette exacte de la
    hitbox (hmi::applySilhouetteMask). La trame diagonale rend ce decoupage bien visible.
    """
    interior: Color = (94, 118, 96, 255)
    trace: Color = (128, 158, 128, 255)

    cell = Image(TILE, TILE, interior)
    hatch(cell, trace, step=3)
    return cell


SKINS = {
    "stone.png": stone_sheet,
    "crate.png": crate,
    "spikes.png": spikes,
    "slope_stone.png": slope_block,
    "entry.png": entry_icon,
    "exit.png": exit_icon,
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
        / "Skins",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, build in SKINS.items():
        image = build()
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")

    print(f"\n{len(SKINS)} skins de test ecrits dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
