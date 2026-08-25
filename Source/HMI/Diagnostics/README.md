# HMI/Diagnostics/

Ce que l'application expose du journal de la session **à l'utilisateur**, par opposition à
`Core/Diagnostics/`, qui définit la journalisation elle-même (niveaux, puits, format).

- `SessionLog` — sérialisation et enregistrement sur disque des messages de log de la session,
  derrière le bouton **« Enregistrer les journaux »** de l'écran Options. C'est ce qui permet à un
  joueur non-développeur de joindre un fichier exploitable à un rapport de défaut, sans lancer
  l'application depuis un terminal.

Réf. specs : `EX-NFR-040` ; guide
[`guide-journalisation`](../../../Documentation/Guide/guide-journalisation.md).
