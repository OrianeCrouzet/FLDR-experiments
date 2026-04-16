# Questions lors des réunions

## Comment compter le nombre de bits aléatoires consommés par algorithme ?

Dans le fichier `/c/flip.c`, on retrouve la variable globale `NUM_RNG_CALLS`, qui créé un nouveau mot de 32 bits dès que cela est nécessaire. On a un pointeur qui se décale à chaque fois qu'on fait appel à `flip()`. Ainsi, si on fait `NUM_RNG_CALLS * 32`, on obtiendra le nombre de bits utilisés par tirage, pour chaque algorithme, avec une marge d'erreur de 31 bits possible. 

Dans `/c/main.c`, à la toute fin de la fonction `main()`, on renvoie 3 arguments : le nom du sampler, e (le temps d'exécution) et `NUM_RNG_CALLS`. 

```c
printf("%s %1.5f %ld\n", sampler, e, NUM_RNG_CALLS);
```

Ainsi, on peut récupérer `NUM_RNG_CALLS` dans `/experiments/pipeline.sh`, au niveau de la commande `aggregate-runtimes`. C'est cette commande qui créé les fichiers `.runtime`, `.calls`, et à présent `.bits`, dans les dossiers de mesures des distributions. 

Le fichier `{my_algo}.calls` contient le nombre de mots aléatoires générés (`NUM_RNG_CALLS`) pour faire tous les tirages pour une distribution, pour l'algorithme `{my_algo}`. Ainsi, il a suffit de créer, dans la commande `aggregate-runtimes`, un nouveau fichier `.bits` qui contient cette fois le nombre de bits aléatoires consommés en moyenne par distribution, pour `{my_algo}`. 

On a donc : 

    cat ${f} | tail -n1 | awk -v steps=${steps} '{print ($3 * 32) / steps}' >> ${fn_bits}

--> avec `steps` le nombre de tirages effectués par distribution. `$3` récupère ici l'argument numéro 3 de la fonction `main()` de `/c/main.c`, à savoir `NUM_RNG_CALLS`. 

Enfin, on créé un fichier `avg_bits` qui répertorie le nombre moyen de bits consommés sur l'ensemble des tirages pour l'ensemble des distributions, par algorithme. 