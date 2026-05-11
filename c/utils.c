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

int binary_search_interval(int *arr, int length, int x) {
    int l = 0;
    int r = length - 1;

    while (l <= r) {
        int mid = l + (r - l) / 2;

        if (mid == 0) {
            if (x < arr[0]) {
                return -1;
            }
            if (length > 1 && x < arr[1]) {
                return 0;
            }
            l = 1;
            continue;
        }

        if ((arr[mid-1] <= x) && (x < arr[mid])) {
            return mid - 1;
        } else if (arr[mid] <= x) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }

    return -1;
}

int binary_search_interval_nested(int *arr, int arr_denominator, int length,
        int a, int b, int denominator) {

    int l = 0;
    int r = length - 1;

    int common_a = a * arr_denominator;
    int common_b = b * arr_denominator;

    while (l <= r) {
        int mid = l + (r - l) / 2;

        if (mid == 0) {
            if (common_b <= arr[0] * denominator) {
                return 0;
            }
            l = 1;
            continue;
        }

        if ((arr[mid-1] * denominator <= common_a)
                && (common_b <= arr[mid] * denominator)) {
            return mid - 1;
        } else if (arr[mid] * denominator <= common_a) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }
    return -1;
}

// ***** Mes ajouts *****

void cons_alias(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold){
    // ORIGINALE
    VectorInt H, L;
    vector_init(&H);
    vector_init(&L);
    vector_reserve(&H, n);
    vector_reserve(&L, n);

    int virtual_cell = -1;
    int t = 0;


    for (unsigned int i = 0; i < n; i++) {
        if (D->data[i] >= cs){
            vector_push(&H, i);
        }else{
            vector_push(&L, i);
        }
    }

    int w = 0;
    int w2 = 0;
    int temp = 0;

    while (H.size > 0) {
        unsigned int x = vector_get(&H, H.size - 1);
        H.size--; // pop
        w = D->data[x];

        if (L.size > 0) {
            unsigned int x2 = vector_get(&L, L.size - 1);
            L.size--; // pop
            w2 = D->data[x2];

            if ((int)x2 != virtual_obj) {
                vector_push(T, x2);
                vector_push(T, x);
                vector_push(Threshold, w2);
            } else {
                virtual_cell = t;
                vector_push(T, x);
                vector_push(T, x2);
                temp = cs - w2;
                vector_push(Threshold, temp);
            }

            temp = cs - w2;
            w -= temp;
        } else {
            vector_push(T, x);
            vector_push(T, -1);
            vector_push(Threshold, cs);
            w -= cs;
        }

        t += 2;

        if (w > 0) {
            D->data[x] = w;
            if (w >= cs) {
                vector_push(&H, x);
            } else {
                vector_push(&L, x);
            }
        }
    }

    if (virtual_cell > -1) {
        int last = t - 2;

        int tmp1 = vector_get(T, last);
        int tmp2 = vector_get(T, last + 1);

        if ((last/2) < Threshold->size && (virtual_cell/2) < Threshold->size)
            vector_swap(Threshold, last/2, virtual_cell/2);

        T->data[last] = T->data[virtual_cell];
        T->data[last + 1] = T->data[virtual_cell + 1];

        T->data[virtual_cell] = tmp1;
        T->data[virtual_cell + 1] = tmp2;
    }

    vector_free(&H);
    vector_free(&L);
}

// Comparison function for qsort
int compare_pairs(const void *a, const void *b) {
    struct sample_inversion_bernoulli_s *p1 = (struct sample_inversion_bernoulli_s *)a;
    struct sample_inversion_bernoulli_s *p2 = (struct sample_inversion_bernoulli_s *)b;

    if (p1->a < p2->a) return -1;
    if (p1->a > p2->a) return 1;
    return 0;
}

// // Sort the array
//    qsort(table, size, sizeof(Pair), compare_pairs);

void cons_alias_2(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold){
    struct sample_inversion_bernoulli_s table[n];
    for (unsigned int i = 0; i < n; i++) {
        table[i].a = D->data[i];
        table[i].M = i;
    }
    qsort(table, n, sizeof(struct sample_inversion_bernoulli_s), compare_pairs);
    while(table[n-1].a > 0){
        int i = 0;
        while(table[i].a == 0)
            i++;
        if(table[i].a >= cs){
            vector_push(T, table[i].M);
            vector_push(T, -1);
            vector_push(Threshold, cs);
            table[i].a -= cs;
        }
        else{
            int j = 0;
            while(table[j].a + table[i].a < cs)
                j++;
            vector_push(T, table[i].M);
            vector_push(T, table[j].M);
            vector_push(Threshold, table[i].a);
            table[j].a += table[i].a - cs;
            table[i].a = 0;
            qsort(table, n, sizeof(struct sample_inversion_bernoulli_s), compare_pairs);
        }

    }
}

void cons_alias3(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold) {
    // Version optimisée
    VectorInt H, L;
    vector_init(&H);
    vector_init(&L);
    vector_reserve(&H, n);
    vector_reserve(&L, n);

    int virtual_cell = -1;
    int t = 0;

    // Optimisation 1 : Éviter les appels répétés à vector_get et D->data
    int* D_data = D->data;
    
    for (unsigned int i = 0; i < n; i++) {
        if (D_data[i] >= cs) {
            vector_push(&H, i);
        } else {
            vector_push(&L, i);
        }
    }

    int w = 0;
    int w2 = 0;
    int temp = 0;
    
    int* H_data = H.data;
    int* L_data = L.data;
    int* T_data = T->data;
    int* Threshold_data = Threshold->data;
    int H_pos = H.size;
    int L_pos = L.size;

    while (H_pos > 0) {
        unsigned int x = H_data[--H_pos]; // pop from H
        w = D_data[x];

        if (L_pos > 0) {
            unsigned int x2 = L_data[--L_pos]; // pop from L
            w2 = D_data[x2];

            if ((int)x2 != virtual_obj) {
                // Optimisation 3 : Écriture directe quand possible
                T_data[T->size] = x2;
                T_data[T->size + 1] = x;
                Threshold_data[Threshold->size] = w2;
                T->size += 2;
                Threshold->size++;
            } else {
                virtual_cell = t;
                T_data[T->size] = x;
                T_data[T->size + 1] = x2;
                temp = cs - w2;
                Threshold_data[Threshold->size] = temp;
                T->size += 2;
                Threshold->size++;
            }

            temp = cs - w2;
            w -= temp;
        } else {
            T_data[T->size] = x;
            T_data[T->size + 1] = -1;
            Threshold_data[Threshold->size] = cs;
            T->size += 2;
            Threshold->size++;
            w -= cs;
        }

        t += 2;

        if (w > 0) {
            D_data[x] = w;
            if (w >= cs) {
                H_data[H_pos++] = x;
            } else {
                L_data[L_pos++] = x;
            }
        }
    }

    // Mise à jour des tailles finales
    H.size = H_pos;
    L.size = L_pos;

    if (virtual_cell > -1) {
        int last = t - 2;
        int last_idx = last / 2;
        int virtual_idx = virtual_cell / 2;

        if (last_idx < Threshold->size && virtual_idx < Threshold->size) {
            // Optimisation 4 : Échange direct sans fonction vector_swap
            int tmp_threshold = Threshold_data[last_idx];
            Threshold_data[last_idx] = Threshold_data[virtual_idx];
            Threshold_data[virtual_idx] = tmp_threshold;
        }

        // Échange des éléments dans T
        int tmp1 = T_data[last];
        int tmp2 = T_data[last + 1];

        T_data[last] = T_data[virtual_cell];
        T_data[last + 1] = T_data[virtual_cell + 1];

        T_data[virtual_cell] = tmp1;
        T_data[virtual_cell + 1] = tmp2;
    }

    vector_free(&H);
    vector_free(&L);
}

struct sample_alias_integers_s preprocess_alias_integers(int* a, int n) {
    struct sample_alias_integers_s sampler;

    // Init des vecteurs
    vector_init(&sampler.T);
    vector_init(&sampler.Threshold);
    //vector_reserve(&sampler.T, 2*n);
    //vector_reserve(&sampler.Threshold, 2*n);

    sampler.cs = 0;

    VectorInt D;
    vector_init(&D);

    // Copie a[] dans un VectorInt D
    for (int i = 0; i < n; ++i) {
        uint32_t val = a[i];
        vector_push(&D, val);
    }

    int w = 0;
    int w2 = 0;
    int q = 0;
    int r = 0;

    w = poids_total_v2(D, D.size);
    w2 = w;
    sampler.cs = w / n;
    q = w / sampler.cs;
    r = w % sampler.cs;

    int virtual_obj = -1;
    if (r > 0) {
        virtual_obj = n;

        int delta = 0;
        delta = sampler.cs - r;
        w2 = w + delta;
        q++;

        vector_push(&D, delta);            // ajout de l’objet virtuel

        n = D.size;
    }

    cons_alias(D.size, &D, sampler.cs, virtual_obj, &sampler.T, &sampler.Threshold);

    vector_free(&D);

    return sampler;
}

uint32_t poids_total_v2(VectorInt D, unsigned int size){
    uint32_t result = 0;
    for (unsigned int i = 0; i < size; i++) {
        result += D.data[i];
    }
    return result;
}

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