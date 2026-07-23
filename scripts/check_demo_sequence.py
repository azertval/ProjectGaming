#!/usr/bin/env python3
"""Vérifie que la séquence de niveaux démo (LOT-25) reste identique entre le
jeu et le test système qui la rejoue.

Deux endroits énumèrent, dans le même ordre, les fichiers ``demo-*.json`` de
``Source/Elements/Levels`` :
- ``Source/HMI/main.cpp`` (``ScreenId::Game``) : la séquence réellement jouée.
- ``Source/Test/Systeme/test_parcours_complet.cpp`` : le test système qui la
  rejoue de bout en bout.

Sans ce script, rien n'empêche d'ajouter/retirer/réordonner un niveau d'un
côté sans répercuter le changement de l'autre — un niveau chargé en jeu mais
jamais couvert par le test système (ou l'inverse) passerait inaperçu.

Usage :
  python scripts/check_demo_sequence.py
"""
import os
import re
import sys

MAIN_CPP = os.path.join('Source', 'HMI', 'main.cpp')
SYSTEM_TEST = os.path.join('Source', 'Test', 'Systeme', 'test_parcours_complet.cpp')

DEMO_FILE_RE = re.compile(r'"(demo-[A-Za-z0-9_-]+\.json)"')


def extract_sequence(path):
    """Retourne la liste ordonnée des fichiers ``demo-*.json`` mentionnés dans @p path."""
    with open(path, encoding='utf-8') as handle:
        content = handle.read()
    return DEMO_FILE_RE.findall(content)


def check(root):
    main_path = os.path.join(root, MAIN_CPP)
    test_path = os.path.join(root, SYSTEM_TEST)

    main_sequence = extract_sequence(main_path)
    test_sequence = extract_sequence(test_path)

    errors = []
    if not main_sequence:
        errors.append('Aucun niveau demo-*.json trouve dans %s' % MAIN_CPP)
    if not test_sequence:
        errors.append('Aucun niveau demo-*.json trouve dans %s' % SYSTEM_TEST)

    if main_sequence != test_sequence:
        errors.append(
            'Sequences divergentes entre %s et %s :\n    %s : %s\n    %s : %s'
            % (MAIN_CPP, SYSTEM_TEST, MAIN_CPP, main_sequence, SYSTEM_TEST, test_sequence))

    if errors:
        print('Sequence demo (LOT-25) : %d probleme(s)' % len(errors))
        for message in errors:
            print('  - ' + message)
        return 1

    print('Sequence demo (LOT-25) : OK (%d niveaux, identiques et dans le meme ordre).'
          % len(main_sequence))
    return 0


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return check(root)


if __name__ == '__main__':
    sys.exit(main())
