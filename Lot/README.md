# lot/

Lots de travail organisés **un sous-dossier par lot**.

## Structure d'un lot
```
lot/
└── LOT-XX-nom-du-lot/
    ├── epic.md        # Définition de l'epic : objectif, périmètre, critères d'acceptation
    ├── tache-01.md    # Une tâche = une unité de travail
    ├── tache-02.md
    └── ...
```

## Fichier `epic.md`
Décrit l'epic du lot :
- **Objectif** : ce que le lot doit livrer.
- **Périmètre** : ce qui est inclus / exclu.
- **Specs liées** : références vers `../specification/`.
- **Critères d'acceptation** : conditions de « terminé ».

## Fichiers de tâches
Chaque tâche précise : contexte, travail à réaliser, fichiers `Source/` impactés, et définition de fait.
