# Ajouter un nouvel algorithme en C

_BROUILLON_

En guise d'exemple, nous utiliserons l'algorithme `my_algo` pour expliquer les ajouts à faire.

## main.c

Dans `c/main.c`, ajoutez une branche `READ_SAMPLE_TIME` pour `my_algo`, comme pour les autres samplers.

Cette macro attend :

- un nom de sampler (`"my_algo"`),
- la structure interne (`sample_my_algo_s`),
- la fonction de lecture (`read_sample_my_algo`),
- la fonction de tirage (`sample_my_algo`),
- la fonction de libération (`free_sample_my_algo_s`).

## sstructs.h

Définissez la structure utilisée par `my_algo` :

```c
// sample_my_algo
struct sample_my_algo_s {
    //tout ce dont vous avez besoin pour votre structure de données
};
```

La structure doit contenir tous les champs nécessaires au tirage, y compris les tableaux ou matrices.

## readio.c (.h)

### `struct sample_my_algo_s read_sample_my_algo(char *fname)`

- `fname` : chemin vers le fichier qui contient la structure.
- Lire le format dans le même ordre que l’écriture.
- Remplir et retourner `sample_my_algo_s`.

### `void free_sample_my_algo_s(struct sample_my_algo_s x)`

- Libérer tous les champs alloués dynamiquement.
- `x` est passé par valeur, c’est suffisant pour libérer les pointeurs internes.
- Ne pas libérer la structure elle-même.

## sample.c (.h)

Implémentez la fonction de tirage `sample_my_algo` qui prend la structure chargée et retourne un indice ou une valeur.

- La structure doit déjà être construite par `read_sample_my_algo`.
- `sample_my_algo` ne doit pas ouvrir de fichiers ni allouer de mémoire.
- Elle doit uniquement utiliser les champs de `sample_my_algo_s`.

## Makefile

Si vous ajoutez un nouveau fichier source, ajoutez-le au `Makefile` pour qu’il soit compilé.

- `make all` doit reconstruire `main.out.opt` et `main.out`.
- Vérifiez que `my_algo` est accepté par le `main.c` et que le binaire compile.

## Notes importantes

- La lecture (`read_sample_my_algo`) et la libération (`free_sample_my_algo_s`) doivent correspondre exactement au format du fichier.
- Utilisez `load_array` ou `load_matrix` si votre structure contient des tableaux ou des matrices.
- Si l’algorithme n’a pas de champs dynamiques, `free_sample_my_algo_s` peut rester vide.
- Gardez les noms cohérents : `sample_my_algo_s`, `read_sample_my_algo`, `sample_my_algo`, `free_sample_my_algo_s`.

_Plus de détails ultérieurement._