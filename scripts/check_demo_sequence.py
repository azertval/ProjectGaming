#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Vérifie que la séquence de niveaux démo (LOT-25, donnée de contenu depuis
LOT-59 TACHE-04) reste identique entre le jeu et le test système qui la
rejoue.

Deux endroits énumèrent, dans le même ordre, les fichiers ``demo-*.json`` de
``Source/Elements/Levels`` :
- ``Source/Elements/Levels/sequence-demo.json`` (champ ``levels``) : la
  séquence réellement jouée par l'éditeur Qt (``MainWindow::showGame``,
  ``EX-LVL-013`` — donnée de contenu, plus un littéral C++).
- ``Source/Test/Systeme/ScriptedLevelSequence.h`` : la séquence scriptée
  (extraite de ``test_parcours_complet.cpp`` par LOT-ANNEXE-08 pour être
  partagée avec le test d'intégration IA) que le test système et le test
  d'intégration rejouent tous deux de bout en bout.

Sans ce script, rien n'empêche d'ajouter/retirer/réordonner un niveau d'un
côté sans répercuter le changement de l'autre — un niveau chargé en jeu mais
jamais couvert par le test système (ou l'inverse) passerait inaperçu.

Usage :
  python scripts/check_demo_sequence.py
"""
import json
import os
import re
import sys

SEQUENCE_JSON = os.path.join('Source', 'Elements', 'Levels', 'sequence-demo.json')
SYSTEM_TEST = os.path.join('Source', 'Test', 'Systeme', 'ScriptedLevelSequence.h')

DEMO_FILE_RE = re.compile(r'"(demo-[A-Za-z0-9_-]+\.json)"')


def extract_sequence_from_json(path):
    """Retourne la liste ordonnée du champ ``levels`` du fichier de séquence @p path."""
    with open(path, encoding='utf-8') as handle:
        data = json.load(handle)
    return list(data.get('levels', []))


def extract_sequence_from_cpp(path):
    """Retourne la liste ordonnée des fichiers ``demo-*.json`` mentionnés dans @p path."""
    with open(path, encoding='utf-8') as handle:
        content = handle.read()
    return DEMO_FILE_RE.findall(content)


def check(root):
    sequence_path = os.path.join(root, SEQUENCE_JSON)
    test_path = os.path.join(root, SYSTEM_TEST)

    sequence = extract_sequence_from_json(sequence_path)
    test_sequence = extract_sequence_from_cpp(test_path)

    errors = []
    if not sequence:
        errors.append('Aucun niveau dans le champ "levels" de %s' % SEQUENCE_JSON)
    if not test_sequence:
        errors.append('Aucun niveau demo-*.json trouve dans %s' % SYSTEM_TEST)

    if sequence != test_sequence:
        errors.append(
            'Sequences divergentes entre %s et %s :\n    %s : %s\n    %s : %s'
            % (SEQUENCE_JSON, SYSTEM_TEST, SEQUENCE_JSON, sequence, SYSTEM_TEST, test_sequence))

    if errors:
        print('Sequence demo (LOT-25) : %d probleme(s)' % len(errors))
        for message in errors:
            print('  - ' + message)
        return 1

    print('Sequence demo (LOT-25) : OK (%d niveaux, identiques et dans le meme ordre).'
          % len(sequence))
    return 0


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return check(root)


if __name__ == '__main__':
    sys.exit(main())
