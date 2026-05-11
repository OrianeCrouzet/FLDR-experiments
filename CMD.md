# Commandes pour reproduire la figure 4

## 1) Installation

    $ pip install -r requirements.txt
    $ pip install .

## 2) Nouvel algorithme

_Si_ vous avez ajouté un nouvel algorithme pour refaire de nouvelles expérimentations, il faut relancer les commandes suivantes : 

    $ cd c && make all
    $ cd .. && ./check.sh

## 3) Option 1 : Mesures sur les distributions de Saad

### Lancer la création des distributions et les mesures

Il faut lancer la commande : 

    $ cd experiments
    $ ./pipeline.sh '' run-all-memory-runtime <Z> <n>

--> avec `Z` la somme des poids de la distribution et `n` la taille de la distribution. **Il faut impérativement que n <= Z.** 

Les distributions _et_ les mesures seront alors stockées dans un dossier appelé `/experiments/dists.{n}.{Z}.2`.

## 

### Figure 4 : Notebook de Saad

Lancer la commande : 

    $ python3 -m jupyterlab

Ensuite, il faut aller dans le dossier `experiments/notebooks`.

Il faut lancer le notebook `figure-4-interpolation.ipynb` (qui est une version améliorée du notebook original de Saad). Ce notebook propose une régression linéaire pour la figure 4b, pour plus de lisibilité. 

## 3) Option 2 : Mesures sur nos distributions

### Lancer la création des distributions et les mesures

**Attention** : pour cette option, les distributions et les mesures sur les distributions sont stockées dans des dossiers différents, il y a deux étapes distinctes à effectuer.

#### A) Distributions

_Si_ les dossiers de distributions ne sont pas présents dans le dépôt (dans le dossier `experiments/distributions`), il faut simplement relancer la commande : 

    $ cd experiments
    $ python3 create_distributions.py <n> <Z> <m>

--> avec `n` la taille de la distribution, `Z` la somme des poids de la distribution et `m` le nombre de distributions à créer par famille de distributions.

_Exemple_ : `python3 create_distributions.py 1000 40001 200` va créer 200 distributions de taille 1000 avec Z = 40001. 

Cette commande lance la création de toutes les familles de distributions du fichier `/experiments/create_distributions.py`, c'est-à-dire : 

- uniform
- dirichlet
- sparse
- gaussian
- exponential
- powerlaw
- dominant

Les distributions seront alors stockées dans un dossier appelé `experiments/distributions/{distrib_path}`.

_Exemple de `distrib_path`_ : 'dists.custom.gaussian.1000.40001.42'

#### B) Mesures
Pour lancer les mesures sur un dossier de distributions en particulier, il faut lancer la commande : 

    $ ./pipeline.sh <distrib_path> measure-runtimes-existing <steps>

--> avec `distrib_path` le chemin vers le dossier souhaité et `steps` le nombre de tirage par algorithme. 

Les mesures seront alors stockées dans un dossier appelé `experiments/measures/{distrib_path}-work`.

## 

### Figure 4 : Notre Notebook

Lancer la commande : 

    $ python3 -m jupyterlab

Ensuite, il faut aller dans le dossier `experiments/notebooks`.

Vous pouvez lancer le notebook `our-figure-4.ipynb`, qui permet de relancer la figure 4 sur une famille de distribution en particulier. 

!! Il faut au préalable bien avoir lancé les **mesures** sur le dossier de distribution souhaité (_revoir partie 3, option 2, point B_) !!

Ce notebook propose aussi une régression linéaire pour la figure 4b, pour la lisibilité.

# 

Dans les deux options, il faut penser à bien modifier le notebook en fonction des samplers que vous voulez voir apparaître sur la figure, et à modifier les paramètres `n`, `Z` et `dist_type` (si présent) pour charger le bon jeu de données. 

Lancer un nouveau kernel pour voir la figure renouvelée. 

# Commandes pour reproduire la figure 5

La figure 5 permet de comparer le temps de préprocessing des algorithmes souhaités. Le code se trouve dans `c/preprocess.c`. Dans ce fichier, l'ancien code qui permettait de comparer FLDR et Alias GSL a été conservé en commentaires. La version actuelle permet de comparer le temps de préprocessing d'Alias Integers et Alias Fractions. 

Pour relancer tous les calculs, il faut lancer les commandes suivantes : 

    $ cd experiments
    $ ./pipeline.sh pp.mn.418 preprocess-initialize
    $ ./pipeline.sh pp.mn.418 preprocess-measure 
    $ ./pipeline.sh pp.mn.418 preprocess-aggregate

Ensuite, comme d'habitude, il faut lancer :

    $ python3 -m jupyterlab

Puis, il faut charger le notebook `figure-5-preprocessing-time.ipynb` et relancer un nouveau kernel. 