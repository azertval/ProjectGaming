#!/usr/bin/env python3
"""Génère la documentation Doxygen, depuis n'importe quel répertoire courant.

Doxygen résout les chemins de son fichier de configuration (``INPUT``,
``OUTPUT_DIRECTORY``) relativement au **répertoire courant**, pas à
l'emplacement du ``Doxyfile``. Or ``Documentation/Doxyfile`` utilise des chemins
relatifs (``index.md``, ``Guide``, ``../Source``, ``generated``) : lancer
``doxygen Documentation/Doxyfile`` depuis la racine du dépôt échoue avec une
série de ``source '...' is not a readable file or directory``, alors que le même
fichier fonctionne parfaitement depuis ``Documentation/``.

Le piège est d'autant plus facile que les trois autres scripts de vérification
(``lint_exigences.py``, ``generate_cahier_test.py``, ``check_demo_sequence.py``)
se lancent, eux, **depuis la racine**. Ce script rétablit la symétrie : il se
place dans ``Documentation/`` avant d'appeler Doxygen, quel que soit le
répertoire d'où on l'invoque.

Le ``Doxyfile`` est configuré en ``QUIET = YES`` et
``WARN_AS_ERROR = FAIL_ON_WARNINGS`` : une génération réussie n'affiche donc
rien et renvoie 0, un avertissement fait échouer la commande — même garde-fou
qu'en intégration continue.

Usage :
  python scripts/build_docs.py
"""
import os
import subprocess
import sys

DOC_DIRECTORY = 'Documentation'
DOXYFILE = 'Doxyfile'


def build(root):
    """Lance Doxygen depuis @p root/Documentation et retourne son code de sortie."""
    documentation = os.path.join(root, DOC_DIRECTORY)
    if not os.path.isfile(os.path.join(documentation, DOXYFILE)):
        print('ERREUR : %s introuvable.' % os.path.join(documentation, DOXYFILE))
        return 1

    try:
        completed = subprocess.run(['doxygen', DOXYFILE], cwd=documentation)
    except FileNotFoundError:
        print("ERREUR : doxygen introuvable dans le PATH.")
        print('Installez Doxygen (https://www.doxygen.nl) ou ajoutez-le au PATH.')
        return 1

    if completed.returncode != 0:
        print('Documentation : ECHEC (avertissements traites comme des erreurs, '
              'cf. WARN_AS_ERROR).')
        return completed.returncode

    print('Documentation : OK (generee dans %s).'
          % os.path.join(DOC_DIRECTORY, 'generated', 'html'))
    return 0


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return build(root)


if __name__ == '__main__':
    sys.exit(main())
