/*
  Name:     construct.c
  Purpose:  Utilities for constructing data structures.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdint.h>
#include "construct.h"
#include "vector_int.h"
#include "sstructs.h"
#include "fraction.h"


// *********************************************************************************
//              ALIAS INTEGERS
// *********************************************************************************

void cons_alias5(unsigned int n, VectorInt* D, uint32_t cs,VectorInt* T, VectorInt* Threshold){
    // Version originale, avec des tableaux fixes pour small et large
    int* small = malloc(n * sizeof(int));
    int* large = malloc(n * sizeof(int));

    int t = 0;
    int n_small = 0, n_large = 0;

    for (unsigned int i = 0; i < n; i++) {
        if (D->data[i] > cs) {
            large[n_large++] = i;
        } else {
            small[n_small++] = i;
        }
    }

    int w = 0;
    int w2 = 0;
    int temp = 0;

    while (n_large > 0) {
        unsigned int x = large[--n_large];
        w = D->data[x];

        if (n_small > 0) {
            unsigned int x2 = small[--n_small];
            w2 = D->data[x2];

            if (w2 != cs) {
                vector_push(T, x2);
                vector_push(T, x);
                vector_push(Threshold, w2);
                temp = cs - w2;
            } else {
                vector_push(T, x2);
                vector_push(T, -1);
                vector_push(Threshold, cs);
                temp = 0;
            }
            w -= temp;
        } else {
            vector_push(T, x);
            vector_push(T, -1);
            vector_push(Threshold, cs);
            w -= cs;
        }

        t += 2;


        D->data[x] = w;
        if (w > cs) {
            large[n_large++] = x;
        } else {
            small[n_small++] = x;
        }
    }
    while (n_small > 0) {
        
        unsigned int x2 = small[--n_small];
        vector_push(T, x2);
        vector_push(T, -1);
        vector_push(Threshold, cs);

        t += 2;
    }

    free(small);
    free(large);
}

struct sample_alias_integers_s preprocess_alias_integers_old(int* a, int n) {
    struct sample_alias_integers_s sampler;

    // Init des vecteurs
    vector_init(&sampler.T);
    vector_init(&sampler.Threshold);
    vector_reserve(&sampler.T, 2*n);
    vector_reserve(&sampler.Threshold, 2*n);

    sampler.cs = 0;

    VectorInt D;
    vector_init(&D);

    int w = 0;
    // Copie a[] dans un VectorInt D
    for (int i = 0; i < n; ++i) {
        uint32_t val = a[i];
        vector_push(&D, val);
        w += val;
    }

    int r = 0;

    sampler.cs = w / n;
    r = w % sampler.cs;

    int virtual_obj = -1;
    if (r > 0) {
        virtual_obj = n;
        vector_push(&D, sampler.cs - r);            // ajout de l’objet virtuel
        n++;
    }
    /* store virtual object index in sampler for use at sampling time */
    sampler.virtual_obj = virtual_obj;
    printf("virtual_obj_old: %d\n", virtual_obj);

    cons_alias5(n, &D, sampler.cs, &sampler.T, &sampler.Threshold);

    vector_free(&D);

    // // calcul la taille de la structure
    // size_t sampler_size = 0;
    // sampler_size += sizeof(sampler.T);
    // if (sampler.T.data != NULL) {
    //     sampler_size += sampler.T.size * sizeof(int);
    // }
    // sampler_size += sizeof(sampler.Threshold);
    // if (sampler.Threshold.data != NULL) {
    //     sampler_size += sampler.Threshold.size * sizeof(int);
    // }
    // sampler_size += sizeof(sampler.cs);
    // sampler_size += sizeof(sampler.virtual_obj);
    // printf("sampler total size: %zu bytes\n", sampler_size);
    // // fin du calcul et de son affichage

    return sampler;
}


// *********************************************************************************
//              ALDR
// *********************************************************************************

struct sample_aldr_s preprocess_aldr_flat_k(int* a, int n, int kmul) {
    // assume k <= 31
    int m = 0;
    for (int i = 0; i < n; ++i) {
        m += a[i];
    }
    int k = 32 - __builtin_clz(m) - (1 == __builtin_popcount(m));
    int K = k * kmul;                       // depth
    long long c = (1ll << K) / m;           // amplification factor
    long long r = (1ll << K) % m;           // reject weight

    int num_leaves = __builtin_popcountll(r);
    for (int i = 0; i < n; ++i) {
        num_leaves += __builtin_popcountll(c * a[i]);
    }

    int *breadths = calloc(K + 1, sizeof(int));
    int *leaves_flat = calloc(num_leaves, sizeof(int));

    int location = 0;
    for(int j = 0; j <= K; j++) {
        long long bit = (1ll << (K - j));
        if (r & bit) {
            leaves_flat[location] = 0;
            ++breadths[j];
            ++location;
        }
        for (int i = 0; i < n; ++i) {
            long long Qi = c*a[i];
            if (Qi & bit) {
                leaves_flat[location] = i + 1;
                ++breadths[j];
                ++location;
            }
        }
    }

    struct sample_aldr_s sampler = (struct sample_aldr_s){
            .length_breadths = K + 1,
            .length_leaves_flat = num_leaves,
            .breadths = breadths,
            .leaves_flat = leaves_flat
        };

    // // Taille totale = structure + tableaux alloues dynamiquement.
    // size_t sampler_size = sizeof(sampler);
    // if (sampler.breadths != NULL) {
    //     sampler_size += (size_t)sampler.length_breadths * sizeof(int);
    // }
    // if (sampler.leaves_flat != NULL) {
    //     sampler_size += (size_t)sampler.length_leaves_flat * sizeof(int);
    // }
    // printf("sampler ALDR total size: %zu bytes\n", sampler_size);

    return sampler;
}


struct sample_aldr_s preprocess_aldr_flat(int* a, int n) {
    return preprocess_aldr_flat_k(a, n, 2);
}


// *********************************************************************************
//              ALIAS FRACTIONS
// *********************************************************************************

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


// *********************************************************************************
//              ALIAS FROM RUST
// *********************************************************************************

static unsigned int pairwise_sum(const unsigned int *weights, unsigned int n)
{
    if (n <= 32) {
        unsigned int sum = 0;
        for (unsigned int i = 0; i < n; i++)
            sum += weights[i];
        return sum;
    } else {
        unsigned int mid = n / 2;
        return pairwise_sum(weights, mid) + pairwise_sum(weights + mid, n - mid);
    }
}

WeightedError weighted_alias_new(
    alias_rust_s *out,
    const unsigned int *weights,
    unsigned int n
)
{

    if(n==0)
        return WEIGHTED_NO_ITEM;

    vector_init(&out->aliases);
    vector_resize_zero(&out->aliases,n);

    vector_init(&out->small);
    vector_init(&out->large);

    out->prob =
        malloc(sizeof(unsigned int)*n);

    unsigned int sum =
        pairwise_sum(weights,n);

    if(sum==0)
        return WEIGHTED_ALL_ZERO;

    out->weight_sum = sum;

    unsigned int n_unsigned = (unsigned int)n;

    for(unsigned int i=0;i<n;i++)
    {

        unsigned int p =
            weights[i] * n_unsigned;

        out->prob[i]=p;

        if(p < sum)
            vector_push(&out->small,i);
        else
            vector_push(&out->large,i);

    }

    while(
        out->small.size>0 &&
        out->large.size>0
    )
    {

        unsigned int s =
            vector_get(&out->small,
                out->small.size-1);

        out->small.size--;

        unsigned int l =
            vector_get(&out->large,
                out->large.size-1);

        out->large.size--;

        vector_set(
            &out->aliases,
            s,
            l
        );

        out->prob[l] =
            out->prob[l]
            + out->prob[s]
            - sum;

        if(out->prob[l] < sum)
            vector_push(&out->small,l);
        else
            vector_push(&out->large,l);

    }

    while(out->large.size>0)
    {

        unsigned int l =
            vector_get(
                &out->large,
                out->large.size-1);

        out->large.size--;

        out->prob[l]=sum;

    }

    while(out->small.size>0)
    {

        unsigned int s =
            vector_get(
                &out->small,
                out->small.size-1);

        out->small.size--;

        out->prob[s]=sum;

    }

    out->n=n;

    // // Taille logique (size) et taille allouee (capacity) de la structure resultat.
    // size_t out_size_by_size = sizeof(*out);

    // if (out->prob != NULL) {
    //     out_size_by_size += (size_t)out->n * sizeof(unsigned int);
    // }

    // if (out->aliases.data != NULL) {
    //     out_size_by_size += (size_t)out->aliases.size * sizeof(int);
    // }

    // // if (out->small.data != NULL) {
    // //     out_size_by_size += (size_t)out->small.size * sizeof(int);
    // // }

    // // if (out->large.data != NULL) {
    // //     out_size_by_size += (size_t)out->large.size * sizeof(int);
    // // }

    // printf("alias_rust: out size (using size) = %zu bytes\n", out_size_by_size);


    return WEIGHTED_OK;
}