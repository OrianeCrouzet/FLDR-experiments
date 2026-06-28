/*
    This file contains one version of the Alias algorithm with rationnal numbers.
    *************************************
    Classic Alias using rational numbers.
    *************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "sstructs.h"
#include "alias_fractions.h"
#include "flip.h"
#include "fraction.h"
#include <stdint.h>

struct AliasEntry* algo_alias_fractions(struct Fraction* distrib, int N) {
    // Version qui prend des fractions déjà normalisées
    // Mais on va travailler en entiers pour la précision
    
    // Le dénominateur doit être le même pour toutes les fractions
    uint32_t denom = distrib[0].denom;
    uint64_t total = denom;  // puisque les fractions sont normalisées avec somme=1
    
    uint64_t* scaled = malloc(N * sizeof(uint64_t));
    for (int i = 0; i < N; i++) {
        scaled[i] = (uint64_t)distrib[i].num * (uint64_t)N;
    }
    
    int* small = malloc(N * sizeof(int));
    int* large = malloc(N * sizeof(int));
    int n_small = 0, n_large = 0;
    
    for (int i = 0; i < N; i++) {
        if (scaled[i] < total) {
            small[n_small++] = i;
        } else {
            large[n_large++] = i;
        }
    }
    
    struct AliasEntry* T = malloc(N * sizeof(struct AliasEntry));
    
    while (n_small > 0 && n_large > 0) {
        int s = small[--n_small];
        int l = large[--n_large];
        
        T[s].prob.num = (uint32_t)scaled[s];
        T[s].prob.denom = (uint32_t)total;
        T[s].prob = frac_reduce(T[s].prob);
        T[s].i = s;
        T[s].j = l;
        
        scaled[l] = scaled[l] - (total - scaled[s]);
        
        if (scaled[l] < total) {
            small[n_small++] = l;
        } else {
            large[n_large++] = l;
        }
    }
    
    while (n_small > 0) {
        int s = small[--n_small];
        T[s].prob = frac_create(1, 1);
        T[s].i = s;
        T[s].j = -1;
    }
    while (n_large > 0) {
        int l = large[--n_large];
        T[l].prob = frac_create(1, 1);
        T[l].i = l;
        T[l].j = -1;
    }
    
    free(scaled);
    free(small);
    free(large);

    // // Taille de la table Alias T (sans compter les buffers temporaires).
    // size_t t_size = (size_t)N * sizeof(struct AliasEntry);
    // printf("alias_fractions: T size = %zu bytes (%d entries, %zu bytes/entry)\n",
    //        t_size, N, sizeof(struct AliasEntry));
    
    return T;
}

void free_sample_alias_fractions(struct sample_alias_fractions_s x) {
    free(x.table);  // Libère le tableau
}

struct sample_alias_fractions_s preprocess_alias_fractions(int* a, int n) {
    uint64_t total = 0;

    // calcul du total
    for (int i = 0; i < n; i++) {
        total += a[i];
    }

    struct Fraction* distrib = malloc(n * sizeof(struct Fraction));

    // normalisation exacte sous forme fraction
    for (int i = 0; i < n; i++) {
        distrib[i].num = a[i];
        distrib[i].denom = (uint32_t)total;
    }

    struct AliasEntry* T = algo_alias_fractions(distrib, n);

    free(distrib);

    struct sample_alias_fractions_s result;
    result.taille = n;
    result.table = T;

    return result;
}