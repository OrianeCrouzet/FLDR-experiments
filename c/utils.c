/*
  Name:     utils.c
  Purpose:  Utilities for searching.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdint.h>
#include "utils.h"
#include "vector_int.h"
#include "sstructs.h"


// ***** Mes ajouts *****

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

    return sampler;
}

// uint32_t poids_total_v2(VectorInt D, unsigned int size){
//     uint32_t result = 0;
//     for (unsigned int i = 0; i < size; i++) {
//         result += D.data[i];
//     }
//     return result;
// }

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

    return (struct sample_aldr_s){
            .length_breadths = K+1,
            .length_leaves_flat = num_leaves,
            .breadths = breadths,
            .leaves_flat = leaves_flat
        };
}


struct sample_aldr_s preprocess_aldr_flat(int* a, int n) {
    return preprocess_aldr_flat_k(a, n, 2);
}