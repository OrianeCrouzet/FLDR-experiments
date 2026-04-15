# /experiments

## /distributions

Ce sous-dossier contient toutes les distributions que nous créons nous-même grâce au fichier `create_distributions.py`. Plusieurs familles de distributions seront générées chaque fois que la commande est lancée.

Les familles de distributions sont stockées au format : `dists.custom.{family}.{n}.{Z}.{seed}`. Nous appellerons ce format `distrib_path` par la suite.

--> avec `family` la famille de distributions, `n` la taille de la distribution, `Z` la somme des poids de la distribution et `seed` la graine de génération aléatoire (42 par défaut).

_Exemple_ : 'dists.custom.gaussian.1000.40001.42'

## /donnees_antoine

Ce sous-dossier contient les distributions fortement dégénérées fournies par Antoine au format `.txt`. 

En lançant le fichier `convert_donnees_antoine.py`, on obtient les distributions au bon format dans le dossier `/distributions`. Ces distributions ont pour `family` : '_very-biased_'.

## /measures

Ce sous-dossier contient toutes les mesures effectuées sur nos distributions (du dossier `/distributions`).

Chaque fois qu'on exécute la commande pour lancer les mesures expérimentales sur un dossier de distribution, les résultats apparaissent ici, au format : `{distrib_path}-work`. 

_Les distributions sont séparées des résultats des mesures expérimentales pour faciliter la reproduction des mesures._ 

## /notebooks

Le sous-dossier `/notebooks` contient tous les notebooks Python pour recréer les figures du papier de Saad. Seulement ces notebooks nous intéressent pour refaire la figure 4 : 

- `figure-4-sampler-comparison.ipynb` : notebook original de Saad, pour recréer la figure 4 avec ses distributions à lui. Le fichier a été légèrement modifié, mais le fonctionnement est exactement le même que l'original.
- `figure-4-interpolation.ipynb` : notebook qui reprend le même principe que le notebook précédent, cependant celui-ci propose une régression linéaire pour apporter plus de lisibilité à la figure 4b. Ce notebook est aussi à utiliser dans le cas où on travaille avec les distributions de Saad.
- `our-figure-4` : un nouveau notebook qui permet de lancer la figure 4 sur nos distributions à nous (le principe est encore le même, c'est simplement que le format des noms de nos distributions diffèrent légèrement de celles de Saad, mais ce notebook fait aussi la même chose). Ce notebook propose aussi une régression linéaire pour la figure 4b.

_**NB** : J'essaierai sûrement de fusioner les notebooks en un seul plus tard pour faciliter la reproduction de la figure 4._

Dans ces trois notebooks, il existe des variables à modifier pour charger le bon jeu de données dont on veut faire la figure. 

On les retrouve dans la cellule 4 : 

```python
# Penser à changer le n, le Z, la seed (et le type de distribution) en fonction du dossier que l'on veut charger

dist_type = 'gaussian'  #si présent dans le notebook en question
n = 1000
Z = 40001
seed = 42
```

Dans cet exemple, le dossier chargé sera : `dists.custom.gaussian.1000.40001.42`. Il faut donc penser à changer ces variables selon le dossier que vous voulez utiliser avant de relancer un kernel. 

## 

**Attention** : pour le notebook `our-figure-4`, il faut bien avoir fait les **mesures** sur votre dossier de distributions pour que cela fonctionne. En effet, le notebook charge les mesures du dossier `/measures`, avec pour format `{distrib_path}-work`. 

Si on reprend l'exemple précédent, on doit bien avoir un dossier `dists.custom.gaussian.1000.40001.42-work` dans le dossier `/measures` pour qu'il puisse être chargé par le notebook. 

Pour plus d'informations sur comment lancer les mesures, consulter [CMD.md](../CMD.md).

## pipeline.sh

## dists.py

Ce fichier permet de générer les distributions de Saad. 

### _write_samplers()_

Dans cette fonction, _si_ vous avez ajouté un nouvel algorithme dans les dossiers `/c` et `/src`, alors il faut ajouter ceci dans `structures`, à la suite des autres : 

```python
('my_algo', # Le nom doit être exactement le même que celui ajouté dans /c/main.c
    construct_sample_my_algo,   # Fonction Python dans le fichier /src/construct.py
    write_sample_my_algo),      # Fonction Python dans le fichier /src/writeio.py
```

Penser également à ajouter dans les imports tout en haut du fichier : 

    from discrete_sampling.writeio import write_sample_my_algo
    # ...
    from discrete_sampling.construct import write_sample_my_algo


### _generate_distributions()_

1. Définit un dossier `dists.{n}.{Z}.{seed}`. Le créé s'il est inexistant, ou le supprime et le recréé s'il était déjà existant.
2. Génère une liste de paramètres `alphas` via `/src/entropy.py/get_alpha_entropies()`, afin de faire varier l'entropie des distributions. 
3. Pour chaque _alpha_ de `alphas`, génère une distribution via `/src/utils.py/sample_dirichlet_multinomial_positive()`. 
4. Calcule l'entropie de chaque distribution.
5. Trie les distributions par entropie croissante.
6. Pour chaque distribution triée, écrit sur disque avec `write_samplers`. 

## create_distributions.py

Ce fichier permet de générer nos propres distributions, avec des familles de distributions variées.

On peut actuellement générer les familles de distributions suivantes : 

- uniform
- dirichlet
- sparse
- gaussian
- exponential
- powerlaw
- dominant

Pour les distributions gaussiennes, on stocke les entropies et les sigmas utilisés pour chaque distribution, et on créé une courbe qui représente les entropies des distributions en fonction du sigma utilisé, afin de voir le rôle de sigma dans l'entropie d'une distribution gaussienne. 

## convert_donnees_antoine.py

Ce fichier permet de convertir les fichiers `.txt` fournis par Antoine au format que l'on utilise dans les mesures expérimentales.