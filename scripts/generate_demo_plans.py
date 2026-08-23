#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Genere les PLANS PICTURAUX des niveaux livres (LOT-69 TACHE-10, etendu au LOT-70).

Pourquoi un script plutot que des images dessinees et commitees : la reproductibilite (LOT-66). Le
resultat se reconstruit a l'identique depuis les sources du depot, et une correction de motif se
rejoue sur les vingt-deux niveaux d'un seul appel.

Ce que le script produit -- et ce qu'il ne pretend pas produire :

- Il peint pour chaque niveau un **fond** derive du theme qu'il declare (son image de fond), a
  densite 8.
- Il y **reporte** les anciens decors-sprites du niveau, a leurs positions d'origine : ceux
  d'arriere-plan dans le fond, ceux de premier plan dans un second plan a densite native, cree
  seulement pour les niveaux qui en avaient.
- Pour les deux seuls niveaux ou la parallaxe est active (`FAR_BACKDROP_LEVELS`, LOT-70), il peint
  en plus un plan **lointain** a densite 4, plus lent que le fond -- une vraie troisieme
  profondeur, chacune a son propre facteur de defilement (`EX-DEC-043`), plutot qu'un jeu de
  teintes empile sur un seul plan.

L'intention visuelle de chaque tableau reste preservee ; seuls les deux niveaux a parallaxe active
gagnent une fresque a trois profondeurs (LOT-70). Les vingt autres restent le report fidele de
l'ancien habillage decrit par le LOT-69 -- les repeindre serait un acte de level design distinct,
hors de ce lot.

Aucune dependance externe (meme encodage PNG que les generateurs d'assets du projet). Les motifs
proceduraux viennent de l'ancien `generate_test_decors.py`, retire avec le systeme de decors : ils
survivent ici parce qu'ils sont desormais la matiere des plans, plus des assets a poser.

Usage :
    python scripts/generate_demo_plans.py [--output <dossier>] [--check]

`--check` ne reecrit rien et echoue si un fichier attendu manque ou differe : c'est la forme
utilisable en CI.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import random
import struct
import zlib

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)

ROOT = pathlib.Path(__file__).resolve().parent.parent
LEVELS_DIR = ROOT / "Source" / "Elements" / "Levels"
MOTIFS_DIR = pathlib.Path(__file__).resolve().parent / "motifs"

# Densite native d'un plan, en pixels par unite monde : meme valeur que
# core::PLANE_NATIVE_PIXELS_PER_UNIT et hmi::Camera2D::PIXELS_PER_UNIT.
NATIVE_PIXELS_PER_UNIT = 16

# Densite des FONDS peints : moitie du natif. Un fond est vu de loin et derriere tout le reste --
# la moitie de la definition y est invisible, et le quart de memoire economise l est pour de bon
# (EX-DEC-041, EX-NFR-043). Les plans de DEVANT restent natifs : eux passent devant le personnage.
BACKDROP_PIXELS_PER_UNIT = 8

# Opacite du voile de ciel d'un fond peint, sur 255. Assez pour teinter et unifier, assez peu
# pour que l'image de fond du niveau reste visible dessous.
SKY_VEIL_ALPHA = 90

# Densite du plan LOINTAIN (LOT-70) : la moitie de celle du fond, pour la meme raison que
# BACKDROP_PIXELS_PER_UNIT -- il est vu de plus loin encore, et masque sous l'horizon du plan
# fond (opaque), donc invisible sur l'essentiel de sa surface.
FAR_BACKDROP_PIXELS_PER_UNIT = 4

# Voile de ciel du plan lointain : plus transparent que SKY_VEIL_ALPHA -- la profondeur se lit
# aussi a l'estompage atmospherique, pas seulement au facteur de parallaxe.
FAR_SKY_VEIL_ALPHA = 55

# Fraction de l'horizon du plan fond (0.62, voir paint_backdrop) a laquelle placer la crete du
# plan lointain : strictement AU-DESSUS, jamais en dessous -- le fond y est opaque et la
# recouvrirait entierement, du travail invisible a l'ecran (LOT-70).
FAR_HORIZON_RATIO = 0.82


class Image:
    """Bitmap RGBA simple, origine en haut a gauche."""

    def __init__(self, width: int, height: int, fill: Color = TRANSPARENT) -> None:
        self.width = width
        self.height = height
        self.pixels: list[Color] = [fill] * (width * height)

    def get(self, x: int, y: int) -> Color:
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.pixels[y * self.width + x]
        return TRANSPARENT

    def set(self, x: int, y: int, color: Color) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def blend(self, x: int, y: int, color: Color) -> None:
        """Compose `color` PAR-DESSUS le pixel courant (alpha-over, alpha non premultiplie).

        Meme regle que `hmi::composeOver` (PlaneReference.h) : un plan se peint par couches, et
        ecraser au lieu de composer ferait disparaitre tout ce qu'un motif semi-transparent
        recouvre (les halos de lanterne, notamment).
        """
        if color[3] == 0:
            return
        if color[3] == 255:
            self.set(x, y, color)
            return
        under = self.get(x, y)
        alpha = color[3] / 255.0
        under_alpha = under[3] / 255.0
        out_alpha = alpha + under_alpha * (1.0 - alpha)
        if out_alpha <= 0.0:
            self.set(x, y, TRANSPARENT)
            return
        channels = tuple(
            int(round((color[i] * alpha + under[i] * under_alpha * (1.0 - alpha)) / out_alpha))
            for i in range(3)
        )
        self.set(x, y, (channels[0], channels[1], channels[2], int(round(out_alpha * 255))))

    def fill_rect(self, x: int, y: int, width: int, height: int, color: Color) -> None:
        for row in range(y, y + height):
            for column in range(x, x + width):
                self.set(column, row, color)

    def blend_rect(self, x: int, y: int, width: int, height: int, color: Color) -> None:
        for row in range(y, y + height):
            for column in range(x, x + width):
                self.blend(column, row, color)

    def to_png(self) -> bytes:
        """Encode l'image en PNG (RGBA 8 bits, sans filtrage)."""
        raw = bytearray()
        for row in range(self.height):
            raw.append(0)  # type de filtre 0 (aucun)
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

    @staticmethod
    def from_png(path: pathlib.Path) -> "Image":
        """Decode un PNG RGBA 8 bits non entrelace (le seul format que le projet livre)."""
        data = path.read_bytes()
        if data[:8] != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"{path} n'est pas un PNG")
        position = 8
        width = height = 0
        compressed = bytearray()
        while position < len(data):
            length = struct.unpack(">I", data[position : position + 4])[0]
            tag = data[position + 4 : position + 8]
            body = data[position + 8 : position + 8 + length]
            position += 12 + length
            if tag == b"IHDR":
                width, height, depth, color_type, _, _, interlace = struct.unpack(
                    ">IIBBBBB", body
                )
                if depth != 8 or color_type != 6 or interlace != 0:
                    raise ValueError(f"{path} : seul le RGBA 8 bits non entrelace est lu")
            elif tag == b"IDAT":
                compressed.extend(body)
            elif tag == b"IEND":
                break

        raw = zlib.decompress(bytes(compressed))
        image = Image(width, height)
        stride = width * 4
        previous = bytearray(stride)
        offset = 0
        for row in range(height):
            filter_type = raw[offset]
            offset += 1
            line = bytearray(raw[offset : offset + stride])
            offset += stride
            for index in range(stride):
                left = line[index - 4] if index >= 4 else 0
                up = previous[index]
                up_left = previous[index - 4] if index >= 4 else 0
                if filter_type == 1:
                    line[index] = (line[index] + left) & 0xFF
                elif filter_type == 2:
                    line[index] = (line[index] + up) & 0xFF
                elif filter_type == 3:
                    line[index] = (line[index] + (left + up) // 2) & 0xFF
                elif filter_type == 4:
                    estimate = left + up - up_left
                    distance_left = abs(estimate - left)
                    distance_up = abs(estimate - up)
                    distance_up_left = abs(estimate - up_left)
                    if distance_left <= distance_up and distance_left <= distance_up_left:
                        predictor = left
                    elif distance_up <= distance_up_left:
                        predictor = up
                    else:
                        predictor = up_left
                    line[index] = (line[index] + predictor) & 0xFF
                elif filter_type != 0:
                    raise ValueError(f"{path} : filtre PNG inconnu {filter_type}")
            for column in range(width):
                base = column * 4
                image.set(
                    column,
                    row,
                    (line[base], line[base + 1], line[base + 2], line[base + 3]),
                )
            previous = line
        return image


def blit(destination: Image, source: Image, x: int, y: int, divisor: int = 1) -> None:
    """Compose `source` sur `destination`, coin haut-gauche en (x, y).

    `divisor` reduit le motif par un ratio ENTIER, en gardant un pixel sur `divisor` -- jamais
    d'interpolation, exactement comme `hmi::resamplePlane` : moyenner des couleurs voisines
    introduirait des teintes que personne n'a posees, ce qu'un pixel art ne pardonne pas.
    """
    for row in range(0, source.height // divisor):
        for column in range(0, source.width // divisor):
            destination.blend(
                x + column, y + row, source.get(column * divisor, row * divisor)
            )


# --------------------------------------------------------------------------------------------
# Motifs proceduraux (repris de l'ancien generate_test_decors.py, retire avec les decors).
# --------------------------------------------------------------------------------------------


def bush() -> Image:
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
    body: Color = (240, 244, 250, 220)
    shadow: Color = (206, 214, 226, 200)
    image = Image(32, 14, TRANSPARENT)
    image.fill_rect(4, 6, 24, 6, body)
    image.fill_rect(8, 2, 12, 6, body)
    image.fill_rect(18, 3, 10, 6, body)
    image.fill_rect(4, 10, 24, 2, shadow)
    return image


def sign() -> Image:
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
    body: Color = (108, 176, 214, 235)
    bright: Color = (176, 224, 246, 235)
    dark: Color = (56, 104, 148, 235)
    image = Image(14, 22, TRANSPARENT)
    for y in range(22):
        half = 1 + min(y, 21 - y) * 6 // 11
        image.fill_rect(7 - half, y, half * 2, 1, body)
    image.fill_rect(5, 4, 2, 14, bright)
    image.fill_rect(8, 6, 2, 12, dark)
    return image


def stalactite() -> Image:
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
    blade: Color = (74, 148, 64, 255)
    blade_dark: Color = (46, 104, 44, 255)
    image = Image(18, 10, TRANSPARENT)
    for index, x in enumerate((1, 4, 7, 10, 13, 16)):
        height = 6 + (index % 3) * 2
        image.fill_rect(x, 10 - height, 2, height, blade if index % 2 == 0 else blade_dark)
    image.fill_rect(0, 9, 18, 1, blade_dark)
    return image


def lantern() -> Image:
    metal: Color = (86, 80, 70, 255)
    glass: Color = (250, 224, 150, 255)
    glow: Color = (250, 236, 190, 120)
    image = Image(14, 24, TRANSPARENT)
    image.fill_rect(6, 0, 2, 7, metal)
    image.fill_rect(3, 7, 8, 2, metal)
    image.fill_rect(4, 9, 6, 10, glass)
    image.fill_rect(3, 9, 1, 10, metal)
    image.fill_rect(10, 9, 1, 10, metal)
    image.fill_rect(3, 19, 8, 2, metal)
    image.blend_rect(2, 11, 10, 6, glow)
    return image


def pillar() -> Image:
    stone: Color = (128, 124, 132, 255)
    dark: Color = (86, 82, 90, 255)
    light: Color = (162, 158, 168, 255)
    image = Image(20, 40, TRANSPARENT)
    image.fill_rect(3, 4, 14, 32, stone)
    image.fill_rect(0, 0, 20, 5, stone)
    image.fill_rect(0, 35, 20, 5, stone)
    image.fill_rect(0, 3, 20, 2, dark)
    image.fill_rect(0, 35, 20, 2, dark)
    image.fill_rect(5, 6, 2, 28, light)
    image.fill_rect(13, 8, 2, 26, dark)
    return image


def gear() -> Image:
    metal: Color = (146, 124, 92, 255)
    dark: Color = (96, 78, 56, 255)
    image = Image(24, 24, TRANSPARENT)
    for y in range(24):
        for x in range(24):
            distance = ((x - 11.5) ** 2 + (y - 11.5) ** 2) ** 0.5
            if distance <= 9.5:
                image.set(x, y, metal if distance > 4.0 else TRANSPARENT)
            if 4.0 < distance <= 5.5:
                image.set(x, y, dark)
    for x, y in ((10, 0), (10, 21), (0, 10), (21, 10)):
        image.fill_rect(x, y, 4, 3, metal)
    return image


def star() -> Image:
    """Un eclat lointain, quatre branches courtes -- motif du plan LOINTAIN nocturne (LOT-70)."""
    glow: Color = (255, 250, 220, 235)
    dim: Color = (255, 250, 220, 130)
    image = Image(5, 5, TRANSPARENT)
    image.set(2, 2, glow)
    image.blend_rect(1, 2, 3, 1, dim)
    image.blend_rect(2, 1, 1, 3, dim)
    return image


PROCEDURAL_MOTIFS = {
    "bush": bush,
    "branch": branch,
    "sign": sign,
    "rock": rock,
    "cloud": cloud,
    "mushroom": mushroom,
    "crystal": crystal,
    "stalactite": stalactite,
    "vine": vine,
    "grass_tuft": grass_tuft,
    "lantern": lantern,
    "pillar": pillar,
    "gear": gear,
    "star": star,
}

# Motifs issus des packs Kenney (CC0 1.0), conserves comme ENTREES du generateur : ils ne sont
# plus des assets charges par le jeu depuis le retrait des decors, mais le script doit pouvoir se
# rejouer -- un generateur dont les sources ont disparu n'est plus reproductible.
KENNEY_MOTIFS = ("kenney_torch", "kenney_chain", "kenney_ladder")

_motif_cache: dict[str, Image] = {}


def motif(name: str) -> Image:
    """Motif nomme, procedural ou lu dans `scripts/motifs/` (memoise)."""
    if name not in _motif_cache:
        if name in PROCEDURAL_MOTIFS:
            _motif_cache[name] = PROCEDURAL_MOTIFS[name]()
        else:
            _motif_cache[name] = Image.from_png(MOTIFS_DIR / f"{name}.png")
    return _motif_cache[name]


# --------------------------------------------------------------------------------------------
# Composition des plans.
# --------------------------------------------------------------------------------------------


def place(
    plane: Image, name: str, world_x: float, world_y: float, pixels_per_unit: int
) -> None:
    """Pose un motif a une position en UNITES MONDE, comme le faisait un decor-sprite.

    Le coin haut-gauche du motif tombe sur la position, et sa taille monde reste
    `pixels / 16` unites : c'est exactement la geometrie de l'ancien rendu de decor
    (`hmi::resolveSpriteAppearance`), ce qui rend le report fidele plutot qu'approximatif.
    """
    divisor = NATIVE_PIXELS_PER_UNIT // pixels_per_unit
    blit(
        plane,
        motif(name),
        int(round(world_x * pixels_per_unit)),
        int(round(world_y * pixels_per_unit)),
        divisor,
    )


def wash(plane: Image, top: Color, bottom: Color, from_row: int = 0) -> None:
    """Degrade vertical plein, du haut vers le bas -- l'assise du fond peint."""
    span = max(1, plane.height - from_row - 1)
    for row in range(from_row, plane.height):
        ratio = (row - from_row) / span
        color = tuple(int(round(top[i] + (bottom[i] - top[i]) * ratio)) for i in range(4))
        plane.fill_rect(0, row, plane.width, 1, (color[0], color[1], color[2], color[3]))


def stand_on(
    plane: Image,
    rng: random.Random,
    names: tuple[str, ...],
    count: int,
    baseline_row: int,
    pixels_per_unit: int,
) -> None:
    """Pose des motifs POSES sur une ligne d'horizon : leur bas touche `baseline_row`.

    Semer librement en hauteur donnait un fond bruite -- des silhouettes flottant en plein ciel,
    qu'aucun oeil ne lit comme un decor. Un motif de fond doit reposer sur quelque chose : c'est
    la seule regle qui separe un arriere-plan d'un semis de sprites.
    """
    if count <= 0 or not names:
        return
    step = plane.width / count
    for index in range(count):
        image = motif(rng.choice(names))
        divisor = NATIVE_PIXELS_PER_UNIT // pixels_per_unit
        x = int(index * step + rng.uniform(0.0, step * 0.6)) - image.width // (2 * divisor)
        # Enfonce de quelques pixels dans l'horizon : un motif pose pile dessus semble decolle.
        y = baseline_row - image.height // divisor + rng.randint(1, 3)
        blit(plane, image, x, y, divisor)


def hang_from_top(
    plane: Image,
    rng: random.Random,
    names: tuple[str, ...],
    count: int,
    pixels_per_unit: int,
) -> None:
    """Suspend des motifs au bord SUPERIEUR du plan : ils y sont manifestement accroches."""
    if count <= 0 or not names:
        return
    step = plane.width / count
    for index in range(count):
        image = motif(rng.choice(names))
        divisor = NATIVE_PIXELS_PER_UNIT // pixels_per_unit
        x = int(index * step + rng.uniform(0.0, step * 0.6))
        blit(plane, image, x, -rng.randint(0, 2), divisor)


def drift(
    plane: Image,
    rng: random.Random,
    names: tuple[str, ...],
    count: int,
    top_row: int,
    bottom_row: int,
    pixels_per_unit: int,
) -> None:
    """Motifs qui flottent legitimement (nuages) : repartis dans la seule bande du ciel."""
    if count <= 0 or not names or bottom_row <= top_row:
        return
    step = plane.width / count
    for index in range(count):
        image = motif(rng.choice(names))
        divisor = NATIVE_PIXELS_PER_UNIT // pixels_per_unit
        x = int(index * step + rng.uniform(0.0, step * 0.5))
        y = rng.randint(top_row, max(top_row, bottom_row - image.height // divisor))
        blit(plane, image, x, y, divisor)


# Habillage par theme, choisi sur l'image de fond declaree par le niveau : le plan prolonge le
# fond, il ne le contredit pas.
THEMES: dict[str, dict] = {
    "test_forest.png": {
        "sky": ((150, 196, 152, 255), (86, 142, 92, 255)),
        "canopy": ((44, 96, 52, 255), (30, 70, 38, 255)),
        "motifs": ("bush", "grass_tuft", "mushroom"),
        "hanging": ("vine", "branch"),
        "drifting": ("cloud",),
    },
    "test_cave.png": {
        "sky": ((58, 54, 70, 255), (32, 30, 42, 255)),
        "canopy": ((44, 42, 56, 255), (28, 26, 38, 255)),
        "motifs": ("rock", "crystal", "mushroom"),
        "hanging": ("stalactite",),
        "drifting": (),
    },
    "test_sky.png": {
        "sky": ((140, 194, 240, 255), (198, 226, 246, 255)),
        "canopy": None,
        "motifs": (),
        "hanging": (),
        "drifting": ("cloud",),
    },
    "test_sunset.png": {
        "sky": ((244, 158, 106, 255), (250, 214, 160, 255)),
        "canopy": ((122, 74, 88, 255), (86, 50, 66, 255)),
        "motifs": ("bush",),
        "hanging": (),
        "drifting": ("cloud",),
    },
    "test_industrial.png": {
        "sky": ((70, 72, 84, 255), (46, 48, 58, 255)),
        "canopy": ((58, 56, 62, 255), (38, 36, 42, 255)),
        "motifs": ("pillar", "gear", "rock"),
        "hanging": ("lantern",),
        "drifting": (),
        # Plan LOINTAIN (LOT-70) : silhouettes de piliers au loin, posees sur la crete lointaine.
        "far": {"drifting": (), "standing": ("pillar",)},
    },
    "test_night.png": {
        "sky": ((26, 30, 58, 255), (14, 16, 34, 255)),
        "canopy": ((22, 24, 44, 255), (12, 14, 28, 255)),
        "motifs": ("crystal", "rock"),
        "hanging": (),
        "drifting": (),
        # Plan LOINTAIN (LOT-70) : un semis d'etoiles au-dessus de la crete lointaine.
        "far": {"drifting": ("star",), "standing": ()},
    },
    "kenney_grass.png": {
        "sky": ((150, 200, 236, 255), (206, 232, 246, 255)),
        "canopy": ((92, 152, 84, 255), (62, 116, 60, 255)),
        "motifs": ("bush", "grass_tuft"),
        "hanging": (),
        "drifting": ("cloud",),
    },
}

DEFAULT_THEME = THEMES["test_sky.png"]


def jagged_ridge(
    plane: Image,
    rng: random.Random,
    top_row: int,
    fill: Color,
    crest: Color | None,
    roughness: int,
    pixels_per_unit: int,
    bottom_row: int | None = None,
) -> None:
    """Peint une silhouette dechiquetee : bandes verticales de largeur aleatoire, sommet jitteré.

    Remplit jusqu'a `bottom_row` (par defaut le bas du plan). Un plan dont la crete superpose une
    zone deja opaque sur un AUTRE plan -- le lointain sous l'horizon du fond, LOT-70 -- n'a pas
    besoin d'aller plus bas que cette zone : peindre au-dela serait invisible a l'ecran.
    """
    limit = plane.height if bottom_row is None else bottom_row
    column = 0
    while column < plane.width:
        span = max(2, rng.randint(3, 9) * pixels_per_unit // 4)
        top = max(0, top_row + rng.randint(-roughness, roughness) * pixels_per_unit // 4)
        plane.fill_rect(column, top, span, max(0, limit - top), fill)
        if crest is not None:
            plane.fill_rect(column, top, span, 2, crest)
        column += span


def paint_backdrop(
    width_units: int, height_units: int, pixels_per_unit: int, theme: dict, seed: str
) -> Image:
    """Peint le fond d'un niveau : voile de ciel, silhouette lointaine, motifs, retombees.

    **Le ciel reste largement transparent.** Un plan opaque du haut en bas recouvrirait l'image de
    fond du niveau (`background`, EX-REN-044), qui n'aurait alors plus jamais aucun effet -- une
    fonctionnalite livree, annulee par un choix de contenu. Le fond peint pose donc un simple
    **voile** teinte par-dessus, et ne devient franc qu'a partir de l'horizon.
    """
    plane = Image(width_units * pixels_per_unit, height_units * pixels_per_unit)
    rng = random.Random(seed)  # deterministe : meme niveau, meme image, a chaque execution.

    sky_top, sky_bottom = theme["sky"]
    wash(
        plane,
        (sky_top[0], sky_top[1], sky_top[2], SKY_VEIL_ALPHA),
        (sky_bottom[0], sky_bottom[1], sky_bottom[2], SKY_VEIL_ALPHA),
    )

    # Trois couches, dans cet ordre et pas un autre : le ciel, ce qui flotte dedans, la silhouette
    # lointaine, ce qui repose dessus, puis ce qui pend du haut. Chaque motif a ainsi une raison
    # d'etre la ou il est -- c'est ce qui fait lire un arriere-plan plutot qu'un semis.
    horizon = int(plane.height * 0.62)
    drift(plane, rng, theme.get("drifting", ()), max(1, width_units // 9), 0, horizon - 4,
          pixels_per_unit)

    if theme["canopy"] is not None:
        near, far = theme["canopy"]
        # DEUX crenelages, pas un : un lointain, plus clair et plus haut, puis un proche, plus
        # sombre, sur la ligne d'horizon. C'est ce decalage entre les deux qui donne la profondeur
        # -- une seule masse plate se lit comme un aplat, quelle que soit sa silhouette.
        distant = tuple(int(round((far[i] + theme["sky"][1][i]) / 2)) for i in range(4))
        for offset, fill, crest, roughness in (
            (-4, (distant[0], distant[1], distant[2], distant[3]), None, 3),
            (0, far, near, 2),
        ):
            jagged_ridge(plane, rng, horizon + offset, fill, crest, roughness, pixels_per_unit)

    stand_on(plane, rng, theme["motifs"], max(1, width_units // 7), horizon, pixels_per_unit)
    hang_from_top(plane, rng, theme["hanging"], max(1, width_units // 10), pixels_per_unit)
    return plane


def paint_far_backdrop(
    width_units: int, height_units: int, pixels_per_unit: int, theme: dict, seed: str
) -> Image:
    """Peint le plan LOINTAIN d'un niveau (LOT-70) : ciel plus estompe qu'au fond, UNE seule
    crete dechiquetee et un semis clairseme au-dessus.

    Une seule crete, contrairement a `paint_backdrop` : ici la profondeur vient du facteur de
    parallaxe propre a ce plan (`EX-DEC-043`, plus lent que celui du fond), pas d'un jeu de teintes
    empilees sur un plan unique. La crete s'arrete a l'horizon du plan FOND correspondant (meme
    formule, `0.62 * hauteur`) : plus bas, elle serait entierement recouverte par le fond opaque.
    """
    plane = Image(width_units * pixels_per_unit, height_units * pixels_per_unit)
    rng = random.Random(seed + "-lointain")  # deterministe, distinct de la graine du fond.

    sky_top, sky_bottom = theme["sky"]
    wash(
        plane,
        (sky_top[0], sky_top[1], sky_top[2], FAR_SKY_VEIL_ALPHA),
        (sky_bottom[0], sky_bottom[1], sky_bottom[2], FAR_SKY_VEIL_ALPHA),
    )

    fond_horizon = int(plane.height * 0.62)
    far_horizon = int(fond_horizon * FAR_HORIZON_RATIO)

    if theme["canopy"] is not None:
        near, _far = theme["canopy"]
        ridge_tone = tuple(int(round((near[i] + sky_bottom[i]) / 2)) for i in range(4))
        jagged_ridge(plane, rng, far_horizon, ridge_tone, None, 3, pixels_per_unit,
                     bottom_row=fond_horizon)

    far = theme.get("far", {})
    drift(plane, rng, far.get("drifting", ()), max(1, width_units // 8), 0, far_horizon - 2,
          pixels_per_unit)
    stand_on(plane, rng, far.get("standing", ()), max(1, width_units // 10), far_horizon,
             pixels_per_unit)
    return plane


# Les deux seuls niveaux ou hmi::planeParallaxActive est vrai (cadrage Follow/PerRoom) : les seuls
# ou un plan lointain, plus lent que le fond, serait effectivement visible a l'usage. Livrer ce
# plan partout couterait une texture et une passe de rendu (LOT-69 TACHE-09) pour un decalage que
# le cadrage WholeLevel neutralise ailleurs (LOT-70).
FAR_BACKDROP_LEVELS = frozenset({"demo-mouvement", "demo-final"})


# Les decors-sprites que les niveaux livres portaient AVANT le LOT-69, a leurs positions
# d'origine, recopies depuis les fichiers de niveau avant migration : c'est la seule trace qui en
# subsiste, et c'est ce qui rend le report verifiable plutot que declaratif. Les positions sont en
# unites monde ; la couche est celle de l'ancien format (`background` derriere les tuiles,
# `foreground` devant le personnage).
LEGACY_DECORS: dict[str, tuple[tuple[str, float, float, str], ...]] = {
    'demo-bloc-quart': (
        ('kenney_torch', 3.0, 2.2, 'background'),
    ),
    'demo-bloc-reduit': (
        ('bush', 15.0, 7.2, 'background'),
    ),
    'demo-bloc': (
        ('rock', 14.0, 7.3, 'background'),
    ),
    'demo-budget': (
        ('cloud', 5.0, 1.5, 'background'),
        ('grass_tuft', 12.0, 8.4, 'background'),
    ),
    'demo-cle': (
        ('kenney_torch', 2.0, 2.2, 'background'),
        ('grass_tuft', 16.0, 3.4, 'background'),
    ),
    'demo-concave': (
        ('kenney_torch', 4.0, 8.2, 'background'),
        ('crystal', 14.0, 8.5, 'background'),
        ('mushroom', 30.0, 8.2, 'background'),
    ),
    'demo-dangers-avances': (
        ('kenney_torch', 3.0, 8.2, 'background'),
        ('gear', 20.0, 1.0, 'background'),
        ('lantern', 33.0, 0.2, 'foreground'),
    ),
    'demo-dangers-directionnels': (
        ('kenney_torch', 3.0, 8.2, 'background'),
        ('stalactite', 12.0, 0.0, 'foreground'),
        ('crystal', 28.0, 7.5, 'background'),
    ),
    'demo-dash': (
        ('kenney_torch', 8.0, 3.0, 'background'),
        ('rock', 19.0, 3.3, 'background'),
        ('gear', 9.0, 1.0, 'background'),
        ('lantern', 18.0, 0.2, 'foreground'),
    ),
    'demo-deplacement': (
        ('cloud', 3.0, 1.0, 'background'),
        ('bush', 13.0, 7.2, 'background'),
        ('grass_tuft', 4.0, 8.4, 'background'),
        ('mushroom', 11.0, 8.2, 'background'),
    ),
    'demo-double-saut': (
        ('cloud', 6.0, 1.5, 'background'),
        ('branch', 9.0, 6.3, 'foreground'),
        ('grass_tuft', 3.0, 10.4, 'background'),
    ),
    'demo-final': (
        ('kenney_torch', 3.0, 9.2, 'background'),
        ('kenney_chain', 30.0, 2.0, 'foreground'),
        ('kenney_ladder', 47.0, 14.0, 'background'),
        ('pillar', 22.0, 4.5, 'background'),
        ('gear', 41.0, 3.0, 'background'),
        ('lantern', 8.0, 12.5, 'foreground'),
        ('crystal', 34.0, 21.5, 'background'),
    ),
    'demo-interrupteur': (
        ('kenney_fence', 2.0, 2.2, 'background'),
        ('gear', 15.0, 0.4, 'background'),
    ),
    'demo-mouvement': (
        ('cloud', 4.0, 2.0, 'background'),
        ('kenney_chain', 23.0, 2.0, 'foreground'),
    ),
    'demo-pente-gauche': (
        ('rock', 20.0, 9.3, 'background'),
        ('branch', 6.0, 6.3, 'foreground'),
        ('grass_tuft', 8.0, 10.4, 'background'),
    ),
    'demo-pente': (
        ('bush', 2.0, 9.2, 'background'),
        ('cloud', 18.0, 2.0, 'background'),
        ('mushroom', 6.0, 10.2, 'background'),
        ('grass_tuft', 22.0, 8.4, 'background'),
    ),
    'demo-plafond': (
        ('kenney_torch', 12.0, 4.2, 'background'),
        ('stalactite', 6.0, 2.0, 'foreground'),
        ('crystal', 20.0, 5.5, 'background'),
    ),
    'demo-plaque-pression': (
        ('kenney_fence', 3.0, 2.2, 'background'),
        ('gear', 17.0, 0.4, 'background'),
        ('lantern', 8.0, 0.2, 'foreground'),
    ),
    'demo-plateforme': (
        ('kenney_chain', 8.0, 1.0, 'foreground'),
        ('gear', 13.0, 1.0, 'background'),
        ('pillar', 15.5, 7.0, 'background'),
    ),
    'demo-saut': (
        ('sign', 2.5, 5.2, 'background'),
        ('rock', 16.0, 5.3, 'background'),
        ('grass_tuft', 3.0, 5.4, 'background'),
        ('vine', 15.0, 0.5, 'foreground'),
    ),
    'demo-synthese': (
        ('kenney_torch', 2.0, 6.2, 'background'),
        ('branch', 24.0, 5.3, 'foreground'),
        ('pillar', 30.0, 2.5, 'background'),
        ('lantern', 12.0, 0.2, 'foreground'),
    ),
    'demo-wall-jump': (
        ('kenney_torch', 1.2, 9.0, 'background'),
        ('kenney_chain', 2.0, 3.0, 'foreground'),
        ('crystal', 1.2, 11.0, 'background'),
        ('stalactite', 2.0, 0.0, 'foreground'),
    ),
}


def build_level(stem: str, width_units: int, height_units: int, theme: dict) -> dict[str, Image]:
    """Les plans d'un niveau : un fond peint qui recoit ses anciens decors d'arriere-plan, et --
    seulement s'il en avait -- un plan de devant qui ne porte que ceux de premier plan. Les deux
    niveaux de `FAR_BACKDROP_LEVELS` recoivent en plus un plan lointain (LOT-70).

    Le plan de devant n'est cree QUE si le niveau en a besoin : livrer partout un plan presque
    entierement transparent couterait une texture pleine taille et une passe de rendu par image
    (TACHE-09) pour ne rien montrer. Meme regle pour le plan lointain, restreint aux deux niveaux
    ou la parallaxe est effectivement active (`hmi::planeParallaxActive`).
    """
    decors = LEGACY_DECORS.get(stem, ())
    behind = paint_backdrop(width_units, height_units, BACKDROP_PIXELS_PER_UNIT, theme, stem)
    for name, x, y, layer in decors:
        if layer != "foreground":
            place(behind, name, x, y, BACKDROP_PIXELS_PER_UNIT)

    planes = {f"{stem}-fond.png": behind}

    if stem in FAR_BACKDROP_LEVELS:
        far = paint_far_backdrop(width_units, height_units, FAR_BACKDROP_PIXELS_PER_UNIT, theme,
                                  stem)
        planes[f"{stem}-lointain.png"] = far

    front_decors = [decor for decor in decors if decor[3] == "foreground"]
    if front_decors:
        front = Image(width_units * NATIVE_PIXELS_PER_UNIT, height_units * NATIVE_PIXELS_PER_UNIT)
        for name, x, y, _ in front_decors:
            place(front, name, x, y, NATIVE_PIXELS_PER_UNIT)
        planes[f"{stem}-devant.png"] = front
    return planes


def build_all() -> dict[str, Image]:
    """Toutes les images de plan attendues, indexees par nom de fichier."""
    planes: dict[str, Image] = {}
    for path in sorted(LEVELS_DIR.glob("demo-*.json")):
        level = json.loads(path.read_text(encoding="utf-8"))
        theme = THEMES.get(level.get("background", ""), DEFAULT_THEME)
        planes.update(build_level(path.stem, int(level["width"]), int(level["height"]), theme))
    return planes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=LEVELS_DIR / "Plans",
        help="dossier de destination",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="ne rien reecrire ; echouer si un fichier manque ou differe",
    )
    arguments = parser.parse_args()

    planes = build_all()
    if arguments.check:
        stale = []
        for name, image in planes.items():
            target = arguments.output / name
            if not target.exists() or target.read_bytes() != image.to_png():
                stale.append(name)
        if stale:
            print("Plans absents ou perimes : " + ", ".join(sorted(stale)))
            print("Rejouer : python scripts/generate_demo_plans.py")
            return 1
        print(f"{len(planes)} plans a jour dans {arguments.output}")
        return 0

    arguments.output.mkdir(parents=True, exist_ok=True)
    for name, image in sorted(planes.items()):
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")
    print(f"\n{len(planes)} plans ecrits dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
