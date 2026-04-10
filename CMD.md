# Commandes pour reproduire la figure 4

## Installation

    $ pip install -r requirements.txt
    $ pip install .

## Nouvel algorithme

Si vous avez ajouté un nouvel algorithme pour refaire les expérimentations, il faut relancer les commandes suivantes : 

    $ cd c && make all
    $ cd .. && ./check.sh

## Mesures

Pour relancer les mesures, plusieurs choix s'offrent à vous : 

### Option 1 : Lancer les distributions de Saad

Il faut lancer la commande : 

    $ cd experiments
    $ ./pipeline.sh '' run-all-memory-runtime <Z> <n>

Avec `Z` la somme des poids de la distribution et `n` la taille de la dsitribution. **Il faut impérativement que n <= Z.** 

Les mesures seront alors stockées dans un dossier appelé `dists.{n}.{Z}.2`.

### Option 2 : Lancer un dossier pré-existant de distributions

J'ai ajouté des dossiers contenant des distributions variées. Pour lancer les mesures sur un dossier de distributions, il faut lancer la commande : 

    $ cd experiments
    $ ./pipeline.sh <distrib_path> measure-runtimes-existing <steps>

Avec `distrib_path` le chemin vers le dossier souhaité et `steps` le nombre de tirage par algorithme. 

_Exemple de `distrib_path`_ : 'dists.custom.very-biased.2.4.2'

Les mesures seront alors stockées dans un dossier appelé `{distrib_path}-work`.

## Figure 4

Lancer la commande : 

    $ python3 -m jupyterlab

Encore une fois, deux choix s'offrent à vous : 

### Notebook de Saad

Si vous avez choisi l'option 1 à l'étape précédente, il faut lancer le notebook `figure-4-sampler-comparison.ipynb`. 

### Notre notebook

Si vous avez choisi l'option 2 à l'étape précédente, il faut lancer le notebook `our-figure-4.ipynb`. 

## 

Dans les deux cas, il faut penser à bien modifier le notebook en fonction des samplers que vous voulez voir apparaître sur la figure, et à modifier les paramètres `n`, `Z` et `dist_type` (si présent) pour charger le bon jeu de données. 

Lancer un nouveau kernel pour voir la figure renouvelée. 