#!/usr/bin/env python3
"""Genere un fond de niveau de TEST pour ProjectGaming (LOT-44).

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


def sky_gradient() -> Image:
    """Fond degrade ciel/horizon, avec une bande de sol, un soleil et un cadre de bordure."""
    sky_top: Color = (76, 118, 196, 255)
    sky_horizon: Color = (196, 158, 132, 255)
    ground: Color = (86, 132, 84, 255)
    ground_light: Color = (108, 158, 104, 255)
    sun: Color = (250, 224, 140, 255)
    frame: Color = (255, 255, 255, 255)
    tick: Color = (255, 255, 255, 180)

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

    # Cadre de bordure (2 px) : reste entier et centre quel que soit le recadrage applique par
    # hmi::computeBackgroundFit -- un cadre coupe ou asymetrique signalerait une regression.
    image.fill_rect(0, 0, WIDTH, 2, frame)
    image.fill_rect(0, HEIGHT - 2, WIDTH, 2, frame)
    image.fill_rect(0, 0, 2, HEIGHT, frame)
    image.fill_rect(WIDTH - 2, 0, 2, HEIGHT, frame)

    # Graduations centrees (croix) sur les deux axes medians : reperes visuels supplementaires
    # pour verifier que le recadrage est bien CENTRE, pas cale sur un coin.
    mid_x, mid_y = WIDTH // 2, HEIGHT // 2
    image.fill_rect(mid_x - 10, mid_y, 20, 2, tick)
    image.fill_rect(mid_x, mid_y - 10, 2, 20, tick)

    return image


BACKGROUNDS = {
    "test_sky.png": sky_gradient,
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
