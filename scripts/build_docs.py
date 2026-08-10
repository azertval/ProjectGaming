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

Avant de générer, le script vérifie que le ``PROJECT_NUMBER`` du ``Doxyfile``
correspond à la ``VERSION`` du ``project()`` racine — la source unique de vérité
du numéro de version, dont ``core::Engine::version()`` est également dérivé.
Rien ne reliait ces valeurs jusqu'ici, et elles ont divergé pendant quatre
jalons (Doxyfile bumpé à chaque release, CMake resté à sa valeur d'amorçage).

Usage :
  python scripts/build_docs.py
"""
import os
import re
import subprocess
import sys

DOC_DIRECTORY = 'Documentation'
DOXYFILE = 'Doxyfile'
ROOT_CMAKELISTS = 'CMakeLists.txt'

PROJECT_VERSION_RE = re.compile(r'^\s*VERSION\s+(\S+)\s*$', re.MULTILINE)
PROJECT_NUMBER_RE = re.compile(r'^\s*PROJECT_NUMBER\s*=\s*(\S+)\s*$', re.MULTILINE)


def read_single(path, pattern, label):
    """Extrait l'unique capture de @p pattern dans @p path, ou None en signalant pourquoi."""
    try:
        with open(path, encoding='utf-8') as handle:
            matches = pattern.findall(handle.read())
    except OSError as error:
        print('ERREUR : %s illisible (%s).' % (path, error))
        return None
    if len(matches) != 1:
        print('ERREUR : %s attendu exactement une fois dans %s (trouve %d).'
              % (label, path, len(matches)))
        return None
    return matches[0]


def check_version_consistency(root):
    """Vrai si le PROJECT_NUMBER du Doxyfile égale la VERSION du project() racine."""
    cmake_version = read_single(os.path.join(root, ROOT_CMAKELISTS),
                                PROJECT_VERSION_RE, 'VERSION')
    doxygen_version = read_single(os.path.join(root, DOC_DIRECTORY, DOXYFILE),
                                  PROJECT_NUMBER_RE, 'PROJECT_NUMBER')
    if cmake_version is None or doxygen_version is None:
        return False
    if cmake_version != doxygen_version:
        print('ERREUR : versions incoherentes.')
        print('  %s      : VERSION %s' % (ROOT_CMAKELISTS, cmake_version))
        print('  %s/%s : PROJECT_NUMBER %s' % (DOC_DIRECTORY, DOXYFILE, doxygen_version))
        print("Le project() racine est la source unique de verite : aligner le Doxyfile sur lui.")
        return False
    return True


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
    if not check_version_consistency(root):
        return 1
    return build(root)


if __name__ == '__main__':
    sys.exit(main())
