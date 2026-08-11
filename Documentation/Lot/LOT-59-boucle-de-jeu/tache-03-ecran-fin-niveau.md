# TACHE-03 — Écran de fin de niveau et de fin de séquence {#lot-59-tache-03-ecran-fin-niveau}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** non commencé

## Contexte
Atteindre la sortie enchaîne **instantanément** sur le tableau suivant. Rien ne marque la réussite :
le joueur constate que le décor a changé, sans savoir s'il a gagné ou s'il vient d'être téléporté.
Après le dernier tableau, le retour au menu est tout aussi muet — c'est la seconde moitié de
`EX-REN-031`, et la raison pour laquelle `EX-LVL-011` parle d'un « écran de fin » entre parenthèses
depuis le début.

C'est aussi le seul endroit où un bruitage de victoire (`LOT-60`) et un effet de réussite
(`LOT-53`) auront une place.

## Travail à réaliser
- **Écran de fin de niveau** affiché à la réussite (`EX-GP-030`) : nom du tableau terminé, et deux
  entrées — *Continuer* (tableau suivant) et *Rejouer*. Retour au menu accessible.
- **Écran de fin de séquence** après le dernier tableau : message de fin, retour au menu.
- **Avance explicite.** L'enchaînement automatique de `EX-LVL-011` est conservé comme
  **comportement**, mais passe désormais par cet écran : c'est le joueur qui valide, ou une
  temporisation courte si l'on veut préserver le rythme (à trancher à l'implémentation, une
  constante nommée dans les deux cas — jamais une valeur en dur au milieu du code).
- **Marquer le tableau comme terminé** : l'écran est le point où la progression de la `TACHE-05`
  est mise à jour, une seule fois, avant tout chargement du niveau suivant.
- Confondre ou non les deux écrans est une décision d'implémentation : s'ils partagent tout sauf
  un libellé, un seul écran paramétré suffit — et alors `ScreenId` n'a pas besoin d'un troisième
  état.

## Fichiers impactés
- `Source/HMI/Interface/LevelCompleteScreen.{h,cpp}` (nouveau), mise en page dans
  `Source/Elements/UI/LevelCompleteScreen.ui`.
- `Source/HMI/Game/GameSession.{h,cpp}` — signalement de la réussite au lieu du chargement direct
  du suivant.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — câblage.
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/Test/Unit/HMI/Interface/test_screen_flow.cpp` (étendu).

## Tests (obligatoires)
- Une réussite mène à `NiveauTermine`, jamais directement au tableau suivant.
- *Continuer* sur le **dernier** tableau mène à l'écran de fin de séquence, pas à un chargement hors
  bornes — le cas limite le plus probable.
- *Rejouer* recharge le tableau **terminé**, pas le suivant.
- Le tableau est marqué terminé **une seule fois**, même si l'écran est traversé deux fois
  (*Rejouer* puis réussite à nouveau).
- Le test système existant (`test_parcours_complet.cpp`) franchit toujours la séquence complète.
- Libellés présents dans les deux catalogues de traduction.

## Points d'attention
- **Ne pas casser le test système.** Il rejoue les quinze niveaux via `Core` ; l'écran vit dans
  `HMI` et ne doit pas devenir un passage obligé de la logique d'enchaînement testée sans GPU.
- Le dernier tableau est le cas limite : vérifier explicitement l'indice de fin plutôt que de s'en
  remettre à un `+1` non borné.
- Un échec (`EX-GP-031`) ne passe **pas** par cet écran : le redémarrage immédiat est le
  comportement voulu, et le manuel le décrit ainsi (« aucune pénalité au-delà de recommencer »).

## Définition de fait (DoD)
- La réussite d'un tableau et la fin de la séquence sont marquées par un écran, la progression y est
  enregistrée une fois, le dernier tableau ne provoque aucun accès hors bornes, le test système
  passe inchangé ; traduit ; `/W4 /WX` propre.

## Exigences
`EX-IHM-004` (écran de fin de niveau) ; lève `EX-REN-031` pour sa partie fin de niveau ; réutilise
`EX-GP-030` (succès), `EX-GP-031` (échec, non concerné), `EX-LVL-011` (enchaînement),
`EX-LVL-014` (progression), `EX-REN-033` (traduction), `EX-NFR-021` (test système).
