# Architecture du dépôt

## /c

Ce dossier contient tous les algorithmes en C. Les fichiers importants sont les suivants : 
- `readio`, qui permet de lire la structure de données depuis le filepath pour chaque algorithme. 
- `sample`, qui contient les fonctions de tirage.
- `sstructs`, où sont stockées les structures de chaque algorithme.
- `main`, auquel il faut ajouter une nouvelle instance de la macro à chaque fois que l'on souhaite ajouter un nouvel algorithme. 
- `/d`, dossier qui contient une distribution d'exemple au bon format pour l'algorithme en question. **Obligatoire pour lancer les tests**.
- `Makefile`, à bien modifier si on ajoute un nouveau fichier C. 

Quand on ajoute un nouvel algorithme, on peut : 

1. soit ajouter un fichier .c et .h, puis faire le pont entre ces fonctions et les fonctions requises dans la macro dans les fichiers `readio`, `sample` et `sstructs` (exemple : alias_rust.*). Dans ce cas, bien penser à mettre à jour le `Makefile`.

2. soit ajouter directement les fonctions nécessaires dans les bons fichiers (exemple : alias.integers, aldr)

## /src 

Ce dossier contient une version Python des algorithmes. Il faut ajouter un nouveau fichier Python par algorithme que l'on veut tester. Ensuite, il faut créer les fonctions nécessaires dans `construct.py`, `sample.py` et `writeio.py`. 

**Cette étape est obligatoire**. 

## /experiments 

Contient les fichiers de distributions et les notebooks pour relancer les figures. 

- Le fichier `pipeline.sh` permet de lancer différentes commandes sur le terminal. Entre autres, on peut créer des distributions à la volée grâce à la fonction `generate_distributions` dans le fichier `dists.py`, mesurer les temps de calcul et l'espace mémoire utilisé, etc. Dans ce fichier, bien penser à ajouter chaque nouvel algorithme dans `SAMPLERS` (ou mettre seulement ceux que vous voulez).

- C'est dans ce dossier que seront (créés) les dossiers contenant nos jeux de distributions. 

- Le sous-dossier `donnees_antoine` contient les fichiers *.txt fournis. 

## /tests

Un dossier qui contient une suite de tests, il faut simplement importer la construction de chaque nouvel algorithme dans `test_rejection.py`. A part cela, rien à modifier.