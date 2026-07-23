#!/usr/bin/env python3
"""Génère Documentation/CahierTest.md à partir des blocs ``\\castest{...}`` du code de test.

Le Cahier de test agrégeait auparavant tous les cas de test sur une seule page plate (mécanisme
Doxygen ``\\xrefitem``, sans aucune section) — illisible dès que le nombre de tests dépasse
quelques dizaines (321 aujourd'hui). Ce script reconstruit une page structurée, avec des titres
Markdown reflétant l'arborescence réelle de ``Source/Test/`` (Unitaire/Intégration/Système, puis
module), que Doxygen transforme en arbre de navigation repliable (comme toute autre page du site).

Les blocs ``\\castest{...}`` restent la source de vérité unique (un seul endroit à maintenir, au
plus près du test qu'ils décrivent) ; ce script ne fait que les collecter et les réorganiser.

Usage :
  python scripts/generate_cahier_test.py            # régénère Documentation/CahierTest.md
  python scripts/generate_cahier_test.py --check     # vérifie que le fichier est à jour (CI)
"""
import os
import re
import sys

TEST_ROOT = 'Source/Test'
OUTPUT_PATH = 'Documentation/CahierTest.md'

# Un bloc castest s'étend de `\castest{` jusqu'à la fermeture `}` puis `*/`, immédiatement suivi
# de la déclaration du test (TEST(Suite, Nom)) : les deux sont capturés ensemble pour associer
# chaque cas de test à son emplacement dans le code.
CASTEST_RE = re.compile(
    r'\\castest\{(?P<content>.*?)\}\s*\n\s*\*/\s*\nTEST\(\s*(?P<suite>[^,]+),\s*(?P<name>[^)]+)\)',
    re.DOTALL)

FIELD_RE = re.compile(r'\\t(cat|crit|etapes|attendu)\s+')
TAG_RE = re.compile(r'</?b>')
BR_RE = re.compile(r'<br\s*/?>')


def unwrap_comment_lines(text):
    """Recolle les lignes d'un commentaire Doxygen (` * suite...`) en un texte continu."""
    return re.sub(r'\n\s*\*\s?', ' ', text)


def clean_fragment(text):
    text = TAG_RE.sub('', text)
    text = BR_RE.sub(' ', text)
    return re.sub(r'\s+', ' ', text).strip()


def parse_castest_content(raw_content):
    """Découpe un bloc castest nettoyé en (titre, categorie, criticite, etapes, attendu)."""
    flat = unwrap_comment_lines(raw_content)
    pieces = FIELD_RE.split(flat)
    # pieces alterne : [avant_premier_marqueur, marqueur1, texte1, marqueur2, texte2, ...]
    fields = {'title': clean_fragment(pieces[0])}
    for index in range(1, len(pieces), 2):
        fields[pieces[index]] = clean_fragment(pieces[index + 1])
    return fields


def collect_cases(root):
    """Retourne la liste des cas de test, chacun un dict (chemin, ligne, suite, nom, champs...)."""
    cases = []
    for dirpath, _dirnames, filenames in sorted(os.walk(root)):
        for filename in sorted(filenames):
            if not filename.startswith('test_') or not filename.endswith('.cpp'):
                continue
            path = os.path.join(dirpath, filename)
            with open(path, encoding='utf-8') as handle:
                content = handle.read()
            for match in CASTEST_RE.finditer(content):
                line = content.count('\n', 0, match.start()) + 1
                fields = parse_castest_content(match.group('content'))
                cases.append({
                    'path': path.replace('\\', '/'),
                    'line': line,
                    'suite': match.group('suite').strip(),
                    'name': match.group('name').strip(),
                    **fields,
                })
    return cases


def group_key(relative_dir):
    """Éclate un chemin de dossier relatif à TEST_ROOT en (categorie, sous-dossiers...)."""
    parts = [part for part in relative_dir.replace('\\', '/').split('/') if part not in ('', '.')]
    return tuple(parts)


CATEGORY_TITLES = {
    'Unit': 'Tests unitaires',
    'Integration': "Tests d'intégration",
    'Systeme': 'Tests système',
}


def render_case(case):
    lines = [f"- **{case['title']}** *(criticité : {case.get('crit', '?')})* — "
             f"catégorie : {case.get('cat', '?')}"]
    if case.get('etapes'):
        lines.append(f"  - Étapes : {case['etapes']}")
    # Le résultat attendu répète parfois mot pour mot le titre (convention de certaines suites) :
    # l'afficher deux fois n'apporte rien, seul le cas où il diffère est utile au lecteur.
    if case.get('attendu') and case['attendu'] != case['title']:
        lines.append(f"  - Résultat attendu : {case['attendu']}")
    lines.append(f"  - `{case['path']}:{case['line']}` — `{case['suite']}.{case['name']}`")
    return '\n'.join(lines)


def render_markdown(cases):
    by_category = {}
    for case in cases:
        relative_dir = os.path.relpath(os.path.dirname(case['path']), TEST_ROOT)
        key = group_key(relative_dir)
        by_category.setdefault(key[0], {}).setdefault(key[1:], []).append(case)

    total = len(cases)
    lines = [
        '# Cahier de test {#cahiertest}',
        '',
        f'**{total} cas de test**, générés depuis les blocs `\\castest{{...}}` du code par '
        '`scripts/generate_cahier_test.py` (ne pas éditer directement — modifier le commentaire '
        'du test concerné puis relancer le script). Organisés ici selon l\'arborescence de '
        '`Source/Test/` pour rester lisibles page par page.',
        '',
    ]

    for category in ('Unit', 'Integration', 'Systeme'):
        if category not in by_category:
            continue
        subgroups = by_category[category]
        category_total = sum(len(items) for items in subgroups.values())
        lines.append(f'## {CATEGORY_TITLES[category]} ({category_total})')
        lines.append('')

        if category == 'Unit':
            # Deux niveaux : namespace racine (Core/HMI) puis module (Ecs, Editor, ...).
            top_levels = {}
            for subpath, items in subgroups.items():
                top = subpath[0] if subpath else '(racine)'
                top_levels.setdefault(top, {}).setdefault(subpath[1:], []).extend(items)
            for top in sorted(top_levels):
                lines.append(f'### {top}')
                lines.append('')
                modules = top_levels[top]
                for module_path in sorted(modules, key=lambda p: p or ('',)):
                    module_name = module_path[0] if module_path else None
                    items = modules[module_path]
                    if module_name:
                        lines.append(f'#### {module_name} ({len(items)})')
                        lines.append('')
                    by_file = {}
                    for case in items:
                        by_file.setdefault(os.path.basename(case['path']), []).append(case)
                    for filename in sorted(by_file):
                        lines.append(f'**`{filename}`**')
                        lines.append('')
                        for case in by_file[filename]:
                            lines.append(render_case(case))
                        lines.append('')
        else:
            # Integration/Systeme : pas de sous-dossiers, un groupe par fichier de test.
            by_file = {}
            for items in subgroups.values():
                for case in items:
                    by_file.setdefault(os.path.basename(case['path']), []).append(case)
            for filename in sorted(by_file):
                items = by_file[filename]
                subtitle = items[0].get('cat', filename).split('·')[-1].strip()
                lines.append(f'### {subtitle} — `{filename}` ({len(items)})')
                lines.append('')
                for case in items:
                    lines.append(render_case(case))
                lines.append('')

    return '\n'.join(lines).rstrip() + '\n'


def main():
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(repo_root)

    cases = collect_cases(TEST_ROOT)
    if not cases:
        print('Aucun bloc \\castest trouve : verifier le chemin TEST_ROOT.', file=sys.stderr)
        return 1

    rendered = render_markdown(cases)

    if '--check' in sys.argv:
        with open(OUTPUT_PATH, encoding='utf-8') as handle:
            current = handle.read()
        if current != rendered:
            print(f'{OUTPUT_PATH} n\'est pas a jour : relancer '
                  '"python scripts/generate_cahier_test.py".', file=sys.stderr)
            return 1
        print(f'Cahier de test a jour ({len(cases)} cas de test).')
        return 0

    with open(OUTPUT_PATH, 'w', encoding='utf-8', newline='\n') as handle:
        handle.write(rendered)
    print(f'{OUTPUT_PATH} regenere ({len(cases)} cas de test).')
    return 0


if __name__ == '__main__':
    sys.exit(main())
