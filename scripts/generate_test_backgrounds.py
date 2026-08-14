#!/usr/bin/env python3
"""Genere des fonds de niveau de TEST pour ProjectGaming (LOT-44, etoffes en LOT-65).

Cette image est volontairement schematique : elle sert a verifier que le rendu du fond
fonctionne (calque Background, ratio preserve, recadrage par le centre), pas a habiller
le jeu. Un artiste la remplacera par le vrai fond.

Le cadre de bordure et les graduations centrees rendent le recadrage bien visible : si le
ratio du niveau differe de celui de l'image, on doit voir le cadre deborde symetriquement
sur un seul axe (jamais deforme), jamais coupe de travers.

Le script n'a AUCUNE dependance externe (PNG ecrit avec zlib + struct, meme patron que
generate_test_skins.py) : il tourne sur n'importe quel poste et en CI, sans rien installer.

Usage :
    python scripts/generate_test_backgrounds.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Backgrounds/.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import zlib

WIDTH, HEIGHT = 320, 180
"""Dimensions de l'image de test, en pixels (ratio 16:9 -- dimensions libres pour ce
type d'asset, hmi::AssetFamily::Background, mais un ratio distinct d'un niveau carre rend
le recadrage par le centre facile a verifier a l'oeil)."""

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)


class Image:
    """Bitmap RGBA simple, origine en haut a gauche (identique a generate_test_skins.py)."""

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


def lerp(a: int, b: int, t: float) -> int:
    return int(a + (b - a) * t)


def lerp_color(a: Color, b: Color, t: float) -> Color:
    return tuple(lerp(a[i], b[i], t) for i in range(4))  # type: ignore[return-value]


def frame_and_ticks(image: Image, color: Color, tick: Color) -> None:
    """Cadre de bordure (2 px) + graduations centrees, communs a tous les fonds ci-dessous.

    Reste entier et centre quel que soit le recadrage applique par hmi::computeBackgroundFit --
    un cadre coupe ou asymetrique signalerait une regression. Factorise pour que les trois fonds
    partagent le meme repere de verification, plutot que de le dupliquer.
    """
    image.fill_rect(0, 0, image.width, 2, color)
    image.fill_rect(0, image.height - 2, image.width, 2, color)
    image.fill_rect(0, 0, 2, image.height, color)
    image.fill_rect(image.width - 2, 0, 2, image.height, color)

    mid_x, mid_y = image.width // 2, image.height // 2
    image.fill_rect(mid_x - 10, mid_y, 20, 2, tick)
    image.fill_rect(mid_x, mid_y - 10, 2, 20, tick)


def sky_gradient() -> Image:
    """Fond degrade ciel/horizon, avec une bande de sol, un soleil et un cadre de bordure."""
    sky_top: Color = (76, 118, 196, 255)
    sky_horizon: Color = (196, 158, 132, 255)
    ground: Color = (86, 132, 84, 255)
    ground_light: Color = (108, 158, 104, 255)
    sun: Color = (250, 224, 140, 255)

    ground_height = HEIGHT // 4
    horizon_y = HEIGHT - ground_height

    image = Image(WIDTH, HEIGHT)

    # Degrade vertical du ciel, plus fonce vers le haut, clair pres de l'horizon.
    for y in range(horizon_y):
        color = lerp_color(sky_top, sky_horizon, y / max(horizon_y - 1, 1))
        image.fill_rect(0, y, WIDTH, 1, color)

    # Soleil : disque simple, decale du centre pour que l'orientation soit lisible.
    sun_center_x, sun_center_y, sun_radius = WIDTH * 2 // 3, horizon_y // 3, HEIGHT // 10
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if (x - sun_center_x) ** 2 + (y - sun_center_y) ** 2 <= sun_radius**2:
                image.set(x, y, sun)

    # Bande de sol, avec une ligne plus claire juste sous l'horizon (lecture de profondeur).
    image.fill_rect(0, horizon_y, WIDTH, ground_height, ground)
    image.fill_rect(0, horizon_y, WIDTH, 3, ground_light)

    frame_and_ticks(image, (255, 255, 255, 255), (255, 255, 255, 180))
    return image


def night_sky() -> Image:
    """Fond nocturne : degrade sombre, lune, etoiles semees et silhouette de collines."""
    sky_top: Color = (10, 14, 36, 255)
    sky_horizon: Color = (36, 40, 74, 255)
    hills: Color = (18, 22, 42, 255)
    moon: Color = (222, 226, 200, 255)
    star: Color = (235, 235, 245, 220)

    ground_height = HEIGHT // 4
    horizon_y = HEIGHT - ground_height

    image = Image(WIDTH, HEIGHT)
    for y in range(horizon_y):
        color = lerp_color(sky_top, sky_horizon, y / max(horizon_y - 1, 1))
        image.fill_rect(0, y, WIDTH, 1, color)

    # Lune : disque simple, opposee au soleil de sky_gradient (cote gauche).
    moon_center_x, moon_center_y, moon_radius = WIDTH // 4, horizon_y // 3, HEIGHT // 11
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if (x - moon_center_x) ** 2 + (y - moon_center_y) ** 2 <= moon_radius**2:
                image.set(x, y, moon)

    # Etoiles : semis deterministe (pas de hasard, l'image doit rester reproductible a l'identique).
    for i in range(40):
        x = (i * 47 + 13) % WIDTH
        y = (i * 29 + 7) % horizon_y
        image.set(x, y, star)

    # Silhouette de collines ondulantes en bas de ciel, puis bande de sol nocturne.
    for x in range(WIDTH):
        wave = int(6 * ((x // 12) % 2))
        image.fill_rect(x, horizon_y - wave, 1, wave, hills)
    image.fill_rect(0, horizon_y, WIDTH, ground_height, hills)

    frame_and_ticks(image, (220, 220, 235, 255), (220, 220, 235, 160))
    return image


def cave_depths() -> Image:
    """Fond souterrain : degrade tres sombre, stalactites en silhouette, sans horizon net."""
    deep: Color = (14, 10, 18, 255)
    shallow: Color = (44, 34, 46, 255)
    rock: Color = (26, 20, 28, 255)
    glow: Color = (120, 200, 190, 140)

    image = Image(WIDTH, HEIGHT)
    for y in range(HEIGHT):
        color = lerp_color(shallow, deep, y / max(HEIGHT - 1, 1))
        image.fill_rect(0, y, WIDTH, 1, color)

    # Stalactites : triangles descendant du haut de l'image, espacement regulier.
    for i, x in enumerate(range(10, WIDTH - 10, 34)):
        length = 14 + (i % 3) * 8
        for row in range(length):
            width = max(1, (length - row) // 3)
            image.fill_rect(x - width // 2, row, width, 1, rock)

    # Points de lueur (cristaux) : semis deterministe, meme principe que night_sky.
    for i in range(14):
        x = (i * 53 + 21) % WIDTH
        y = HEIGHT - 1 - (i * 17 + 5) % (HEIGHT // 2)
        image.set(x, y, glow)

    frame_and_ticks(image, (150, 150, 160, 255), (150, 150, 160, 150))
    return image


BACKGROUNDS = {
    "test_sky.png": sky_gradient,
    "test_night.png": night_sky,
    "test_cave.png": cave_depths,
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
        / "Backgrounds",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, build in BACKGROUNDS.items():
        image = build()
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")

    print(f"\n{len(BACKGROUNDS)} fond(s) de test ecrit(s) dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
