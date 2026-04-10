# ALIAS Integers Experiments - Figure 4

Ce dépôt GitHub reprend celui de F. Saad, [fast-loaded-dice-roller-experiments](https://github.com/probsys/fast-loaded-dice-roller-experiments), afin de reproduire sa figure 4 dans le cadre de notre article. 

## Objectif

Nous avons pour objectif de comparer notre ALIAS en arithmétique entière à d'autres algorithmes de tirage. On retrouvera l'ALIAS classique de Walker, l'ALIAS de Rust, mais également les algorithmes de Saad : FLDR et ALDR. Nous reprenons ainsi son code pour refaire la figure 4 de son article [The Fast Loaded Dice Roller: A Near-Optimal Exact Sampler for Discrete Probability Distributions](https://proceedings.mlr.press/v108/saad20a/saad20a.pdf).

**Lien vers notre brouillon d'article** : https://plmlatex.math.cnrs.fr/project/671651d4449a94d80d7be5c7

## 1. Parcourir le dépôt

Plusieurs fichiers Markdown ont été créé pour faciliter la compréhension et/ou la prise en main du code afin de reproduire la figure 4 autant de fois que nécessaire. 

- Pour en apprendre plus sur l'architecture globale du dépôt, consultez [ARCHI.md](ARCHI.md).
- Pour lire le README.md du dépôt original, consultez [SAAD_README.md](SAAD_README.md).

## 2. Détails du code

Afin de faciliter la compréhension du code dans ce dépôt, il existe un fichier Markdown pour chaque sous-dossier, afin de décrire avec exactitude tout ce qu'il faut faire pour apporter des changements, ainsi que les informations importantes qu'on a relevées au fur et à mesure de notre travail.

Vous pouvez suivre le détail du code dans l'ordre indiqué ci-après, ou directement aller regarder le Markdown du sous-dossier qui vous intéresse. L'ordre proposé est celui qu'on utilise pour ajouter un nouvel algorithme dans le code.

**Détails du code** :

1. [C.md](/c/C.md)
2. [SRC.md](/src/SRC.md)
3. [TESTS.md](/tests/TESTS.md)
4. [EXPERIMENTS.md](/experiments/EXPERIMENTS.md)
5. [NOTEBOOKS.md](/experiments//notebooks/NOTEBOOKS.md)

## 3. Commandes

Enfin, pour connaître toutes les commandes à appliquer pour reproduire la figure 4, consultez [CMD.md](CMD.md).