#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Vérifie que chaque rejeu publié (`LOT-ANNEXE-18`, `Source/Elements/Replays/`) reste synchronisé
avec le niveau qu'il référence.

`LOT-ANNEXE-17` refuse un rejeu périmé au chargement (`aisolver::validateReplay`), mais seulement
au moment où le jeu ou la CLI le charge -- jamais en intégration continue. Un niveau modifié par
un commit ultérieur (rééquilibrage, correction) peut ainsi rendre un rejeu versionné silencieusement
obsolète pendant des semaines, sans qu'aucune Pull Request ne le signale. Ce script ferme cette
fenêtre : réimplémentation Python pure (aucun binaire C++, aucune dépendance tierce) de l'empreinte
FNV-1a 64 bits (`aisolver::computeLevelFingerprint`, `LOT-ANNEXE-17`), sur le modèle exact de
`scripts/check_demo_sequence.py`.

Convention d'octets stricte, identique à l'implémentation C++ (`LevelFingerprint.h`) : lecture du
fichier de niveau en mode binaire strict, aucune normalisation de fin de ligne ni d'encodage au-delà
d'UTF-8 brut -- piège classique de portabilité Windows/Unix, ici évité en ouvrant en mode ``'rb'``.

Usage :
  python scripts/check_ai_replays.py
"""
import glob
import json
import os
import sys

REPLAYS_DIR = os.path.join('Source', 'Elements', 'Replays')
LEVELS_DIR = os.path.join('Source', 'Elements', 'Levels')

FNV1A_64_OFFSET_BASIS = 0xcbf29ce484222325
FNV1A_64_PRIME = 0x100000001b3
FNV1A_64_MASK = 0xffffffffffffffff


def fnv1a_64(data: bytes) -> int:
    """Empreinte FNV-1a 64 bits de @p data, même algorithme et même convention d'octets que
    `aisolver::computeLevelFingerprint` (`LOT-ANNEXE-17`)."""
    hash_value = FNV1A_64_OFFSET_BASIS
    for byte in data:
        hash_value ^= byte
        hash_value = (hash_value * FNV1A_64_PRIME) & FNV1A_64_MASK
    return hash_value


def check(root):
    replays_dir = os.path.join(root, REPLAYS_DIR)
    levels_dir = os.path.join(root, LEVELS_DIR)

    replay_paths = sorted(glob.glob(os.path.join(replays_dir, '*.json')))

    errors = []
    for replay_path in replay_paths:
        replay_name = os.path.relpath(replay_path, root)
        with open(replay_path, encoding='utf-8') as handle:
            replay = json.load(handle)

        level_path_field = replay.get('levelPath', '')
        expected_fingerprint = replay.get('levelFingerprint', 0)
        level_path = os.path.join(levels_dir, level_path_field)

        if not os.path.isfile(level_path):
            errors.append('%s : niveau introuvable (%s)' % (replay_name, level_path_field))
            continue

        with open(level_path, 'rb') as handle:
            level_content = handle.read()
        actual_fingerprint = fnv1a_64(level_content)

        if actual_fingerprint != expected_fingerprint:
            errors.append(
                '%s : empreinte perimee pour %s (attendu %d, calcule %d)'
                % (replay_name, level_path_field, expected_fingerprint, actual_fingerprint))

    if errors:
        print('Rejeux IA (LOT-ANNEXE-20) : %d probleme(s)' % len(errors))
        for message in errors:
            print('  - ' + message)
        return 1

    print('Rejeux IA (LOT-ANNEXE-20) : OK (%d rejeu(x) verifie(s)).' % len(replay_paths))
    return 0


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return check(root)


if __name__ == '__main__':
    sys.exit(main())
