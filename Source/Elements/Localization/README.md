# Elements/Localization/

Catalogues de traduction de l'interface, un **fichier par langue** (`<langue>.lang`).

- Format texte simple : une paire `clé = valeur` par ligne, encodage **UTF-8**.
- Lignes vides et lignes commençant par `#` (commentaires) ignorées ; seul le premier `=`
  sépare la clé de la valeur.
- Les **clés** sont stables et référencées par le code (aucun libellé d'interface en dur) ;
  ajouter une langue = ajouter un fichier `<langue>.lang`, sans modifier le code.
- Chargés à l'exécution par `hmi::Localization` (copiés à côté de l'exécutable par CMake).

Réf. specs : `EX-REN-033` (catalogue de traduction), `EX-REN-032` (texte).
