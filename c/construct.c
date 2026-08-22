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
#include"vector_mpz.h"
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
    int q = w / sampler.cs;
    r = w % sampler.cs;

    vector_reserve(&sampler.T, q);
    vector_reserve(&sampler.Threshold, q);

    int virtual_obj = -1;
    if (r > 0) {
        virtual_obj = n;
        vector_push(&D, sampler.cs - r);            // ajout de l’objet virtuel
        n++;
    }
    /* store virtual object index in sampler for use at sampling time */
    sampler.virtual_obj = virtual_obj;

    cons_alias5(n, &D, sampler.cs, &sampler.T, &sampler.Threshold);

    vector_free(&D);

    return sampler;
}


// *********************************************************************************
//              ALIAS INTEGERS - GMP (entiers taille arbitraire)
// *********************************************************************************

void poids_total(VectorMpz D, unsigned int size, mpz_t result) {
    mpz_set_ui(result, 0);
    for (unsigned int i = 0; i < size; i++) {
        mpz_add(result, result, D.data[i]);
    }
}

void cons_alias_gmp(unsigned int n, VectorMpz D, mpz_t cs, VectorInt* T, VectorMpz* Threshold) {
    int* small = malloc(n * sizeof(int));
    int* large = malloc(n * sizeof(int));
    // VectorInt H, L;
    // vector_init(&H);
    // vector_init(&L);
    //int virtual_cell = -1;
    int t = 0;
    int n_small = 0, n_large = 0;


    for (unsigned int i = 0; i < n; i++) {
        if (mpz_cmp(D.data[i], cs) > 0){
            large[n_large++] = i;
        }else{
            small[n_small++] = i;
        }
    }

    mpz_t w, w2, temp;
    mpz_inits(w, w2, temp, NULL);

    while (n_large > 0) {
        // unsigned int x = vector_get(&H, H.size - 1);
        // H.size--; // pop
        unsigned int x = large[--n_large];
        mpz_set(w, D.data[x]);

        if (n_small > 0) {
            // unsigned int x2 = vector_get(&L, L.size - 1);
            // L.size--; // pop
            unsigned int x2 = small[--n_small];
            mpz_set(w2, D.data[x2]);

            if (mpz_cmp(w2, cs) != 0) {
                vector_push(T, x2);
                vector_push(T, x);
                vector_mpz_push(Threshold, w2);
                mpz_sub(temp, cs, w2);
            } else {
                //virtual_cell = t;
                vector_push(T, x2);
                vector_push(T, -1);
                vector_mpz_push(Threshold, cs);
                mpz_set_ui(temp, 0);
            }
            //mpz_sub(temp, cs, w2);
            mpz_sub(w, w, temp);
        } else {
            vector_push(T, x);
            vector_push(T, -1);
            vector_mpz_push(Threshold, cs);
            mpz_sub(w, w, cs);
        }

        t += 2;

        //if (mpz_cmp_ui(w, 0) > 0) {
        mpz_set(D.data[x], w);
        if (mpz_cmp(w, cs) > 0) {
            large[n_large++] = x;
        } else {
            small[n_small++] = x;
        }
        //}
    }

    while (n_small > 0) {
        
        unsigned int x2 = small[--n_small];
        vector_push(T, x2);
        vector_push(T, -1);
        vector_mpz_push(Threshold, cs);

        t += 2;
    }

    mpz_clears(w, w2, temp, NULL);
    free(small);
    free(large);
}

struct sample_gmp_alias_integers_s preprocess_gmp_alias_integers(mpz_t* a, int n) {
    struct sample_gmp_alias_integers_s sampler;

    // Init des vecteurs et de cs
    vector_init(&sampler.T);
    vector_mpz_init(&sampler.Threshold);
    mpz_init(sampler.cs);

    VectorMpz D;
    vector_mpz_init(&D);

    mpz_t w;
    mpz_init_set_ui(w, 0);

    // Copie des poids GMP et calcul du poids total w
    for (int i = 0; i < n; ++i) {
        vector_mpz_push(&D, a[i]); // pousse le mpz_t directement
        mpz_add(w, w, a[i]);
    }

    mpz_t q, r;
    mpz_inits(q, r, NULL);   
    
    mpz_fdiv_q_ui(sampler.cs, w, D.size);      // cs = w / n

    if (mpz_cmp_ui(sampler.cs, 0) == 0) {
        fprintf(stderr, "[Erreur Fatal] cs vaut 0 (Poids total w < n). Division par zéro évitée.\n");
        exit(EXIT_FAILURE);
    }

    // q = w // cs ; r = w % cs
    mpz_fdiv_qr(q, r, w, sampler.cs);          // Sécurisé   

    vector_reserve(&sampler.T, mpz_get_ui(q));
    vector_mpz_reserve(&sampler.Threshold, q);

    int virtual_obj = -1;
    if (mpz_cmp_ui(r, 0) > 0) {
        virtual_obj = n;

        mpz_t delta;
        mpz_init(delta);
        mpz_sub(delta, sampler.cs, r);         // delta = cs - r

        vector_mpz_push(&D, delta);            // ajout de l'objet virtuel
        mpz_clear(delta);

        n++;
    }

    sampler.virtual_obj = virtual_obj;

    // Construction de la table Alias
    cons_alias_gmp(n, D, sampler.cs, &sampler.T, &sampler.Threshold);

    mpz_clears(w, q, r, NULL);
    vector_mpz_free(&D);

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

    return sampler;
}


struct sample_aldr_s preprocess_aldr_flat(int* a, int n) {
    return preprocess_aldr_flat_k(a, n, 2);
}


// *********************************************************************************
//              ALDR - GMP (entiers taille arbitraire)
// *********************************************************************************

struct sample_aldr_gmp_s preprocess_aldr_flat_k_gmp(mpz_t* a, int n, int kmul) {
    mpz_t m, k, K, c, r, tmp, pow2K, Qi, bit, num_leaves, location, val_ui;
    mpz_inits(m, k, K, c, r, tmp, pow2K, Qi, bit, num_leaves, location, val_ui, NULL);

    // 1. Calcul de m (somme globale des poids GMP)
    mpz_set_ui(m, 0);
    for (int i = 0; i < n; ++i) {
        mpz_add(m, m, a[i]);
    }

    // 2. Calcul du nombre de bits k = log2_ceil(m)
    // mpz_sizeinbase(m, 2) donne la taille exacte en bits
    size_t bits_m = mpz_sizeinbase(m, 2);
    
    // Si m est une puissance exacte de 2, la taille est bits_m - 1
    mpz_t m_minus_1;
    mpz_init_set(m_minus_1, m);
    mpz_sub_ui(m_minus_1, m_minus_1, 1);
    if (mpz_sizeinbase(m_minus_1, 2) < bits_m) {
        bits_m--;
    }
    mpz_clear(m_minus_1);

    mpz_set_ui(k, bits_m);
    mpz_mul_ui(K, k, kmul); // K = k * kmul

    // 3. pow2K = 2^K
    unsigned long K_ui = mpz_get_ui(K); // K (l'exposant) tient dans un unsigned long
    mpz_ui_pow_ui(pow2K, 2, K_ui);

    // 4. c = pow2K / m  et  r = pow2K % m
    mpz_fdiv_qr(c, r, pow2K, m);

    // 5. Calcul de num_leaves avec mpz_popcount (support de taille arbitraire)
    mpz_set_ui(num_leaves, mpz_popcount(r));

    for (int i = 0; i < n; ++i) {
        mpz_mul(Qi, c, a[i]);
        mpz_add_ui(num_leaves, num_leaves, mpz_popcount(Qi));
    }

    // 6. Initialisation de la structure résultat
    struct sample_aldr_gmp_s result;
    mpz_init(result.length_breadths);
    mpz_init(result.length_leaves_flat);
    vector_mpz_init(&result.breadths);
    vector_mpz_init(&result.leaves_flat);
    
    mpz_set(result.length_breadths, K);
    mpz_add_ui(result.length_breadths, result.length_breadths, 1);
    mpz_set(result.length_leaves_flat, num_leaves);

    mpz_set_ui(location, 0);

    // 7. Boucle de construction des feuilles
    for (unsigned long j = 0; j <= K_ui; ++j) {
        unsigned long bit_index = K_ui - j;

        // Test du bit_index dans r
        if (mpz_tstbit(r, bit_index)) {
            mpz_set_ui(val_ui, 0);
            vector_mpz_push(&result.leaves_flat, val_ui);

            while (result.breadths.size <= j) {
                mpz_set_ui(val_ui, 0);
                vector_mpz_push(&result.breadths, val_ui);
            }
            mpz_add_ui(result.breadths.data[j], result.breadths.data[j], 1);
            mpz_add_ui(location, location, 1);
        }

        for (int i = 0; i < n; ++i) {
            mpz_mul(Qi, c, a[i]);
            if (mpz_tstbit(Qi, bit_index)) {
                mpz_set_ui(val_ui, i + 1);
                vector_mpz_push(&result.leaves_flat, val_ui);

                while (result.breadths.size <= j) {
                    mpz_set_ui(val_ui, 0);
                    vector_mpz_push(&result.breadths, val_ui);
                }
                mpz_add_ui(result.breadths.data[j], result.breadths.data[j], 1);
                mpz_add_ui(location, location, 1);
            }
        }
    }

    // Nettoyage
    mpz_clears(m, k, K, c, r, tmp, pow2K, Qi, bit, num_leaves, location, val_ui, NULL);

    return result;
}

struct sample_aldr_gmp_s preprocess_aldr_flat_gmp(mpz_t* a, int n) {
    return preprocess_aldr_flat_k_gmp(a, n, 2);
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

    return WEIGHTED_OK;
}


// *********************************************************************************
//              ALIAS FROM RUST - GMP (entiers taille arbitraire)
// *********************************************************************************

WeightedError weighted_alias_new_gmp(
    alias_rust_gmp_s *out,
    const mpz_t *weights,
    unsigned int n
)
{
    if (n == 0) return WEIGHTED_NO_ITEM;

    vector_init(&out->aliases);
    vector_resize_zero(&out->aliases, n);
    vector_init(&out->small);
    vector_init(&out->large);
    
    vector_mpz_init(&out->prob);
    out->prob.data = malloc(n * sizeof(mpz_t));
    out->prob.size = n;
    out->prob.capacity = n;

    for (unsigned int i = 0; i < n; i++) {
        mpz_init(out->prob.data[i]);
    }

    mpz_init(out->weight_sum);

    mpz_t sum;
    mpz_init_set_ui(sum, 0);
    for (unsigned int i = 0; i < n; i++) {
        mpz_add(sum, sum, weights[i]);
    }

    if (mpz_cmp_ui(sum, 0) == 0) {
        for (unsigned int i = 0; i < n; i++) {
            mpz_clear(out->prob.data[i]);
        }
        free(out->prob.data);
        vector_free(&out->aliases);
        vector_free(&out->small);
        vector_free(&out->large);
        mpz_clear(out->weight_sum);
        mpz_clear(sum);
        return WEIGHTED_ALL_ZERO;
    }

    mpz_set(out->weight_sum, sum);

    mpz_t p, tmp;
    mpz_inits(p, tmp, NULL);

    for (unsigned int i = 0; i < n; i++) {
        mpz_mul_ui(p, weights[i], n);
        mpz_set(out->prob.data[i], p);

        if (mpz_cmp(p, sum) < 0)
            vector_push(&out->small, i);
        else
            vector_push(&out->large, i);
    }

    while (out->small.size > 0 && out->large.size > 0) {
        unsigned int s = vector_get(&out->small, out->small.size - 1);
        out->small.size--;

        unsigned int l = vector_get(&out->large, out->large.size - 1);
        out->large.size--;

        vector_set(&out->aliases, s, l);

        mpz_set(tmp, out->prob.data[l]);
        mpz_add(tmp, tmp, out->prob.data[s]);
        mpz_sub(tmp, tmp, sum);
        mpz_set(out->prob.data[l], tmp);

        if (mpz_cmp(tmp, sum) < 0)
            vector_push(&out->small, l);
        else
            vector_push(&out->large, l);
    }

    while (out->large.size > 0) {
        unsigned int l = vector_get(&out->large, out->large.size - 1);
        out->large.size--;
        mpz_set(out->prob.data[l], sum);
    }

    while (out->small.size > 0) {
        unsigned int s = vector_get(&out->small, out->small.size - 1);
        out->small.size--;
        mpz_set(out->prob.data[s], sum);
    }

    out->n = n;

    mpz_clears(p, tmp, sum, NULL);
    return WEIGHTED_OK;
}


// *********************************************************************************
//              ALIAS FRACTIONS - GMP (entiers taille arbitraire)
// *********************************************************************************

PileResult piles(mpq_t* distrib, int N, int affiche) {
    PileResult res;

    // pdsCase = 1 / N
    mpq_init(res.pdsCase);
    mpq_set_ui(res.pdsCase, 1, N);

    res.S0 = malloc(N * sizeof(int));
    res.S1 = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        res.S0[i] = -1;
        res.S1[i] = -1;
    }

    res.lenS0 = 0;
    res.lenS1 = 0;

    for (int n = 0; n < N; n++) {
        if (mpq_cmp(distrib[n], res.pdsCase) > 0) {
            res.S0[res.lenS0++] = n;
        } else {
            res.S1[res.lenS1++] = n;
        }
    }

    if (affiche) {
        printf("S0 (lourds) :\n");
        for (int i = 0; i < res.lenS0; i++) {
            int idx = res.S0[i];
            gmp_printf("(%d, %Qd)\n", idx, distrib[idx]);
        }
    }

    return res;
}

struct AliasEntryGMP* algo_alias_fractions_gmp(mpq_t* distrib, int N, int affiche) {
    // piles(distrib) -> pdsCase, S0, S1
    PileResult res = piles(distrib, N, 0);
    mpq_t pdsCase;
    mpq_init(pdsCase);
    mpq_set(pdsCase, res.pdsCase);  // pdsCase = 1/N

    int* S0 = res.S0;
    int lenS0 = res.lenS0;
    int* S1 = res.S1;
    int lenS1 = res.lenS1;

    if (affiche) {
        gmp_printf("pdsCase = %Qd\n", pdsCase);
        printf("lourds (S0): ");
        for (int i = 0; i < lenS0; i++) printf("%d ", S0[i]);
        printf("\nlégers (S1): ");
        for (int i = 0; i < lenS1; i++) printf("%d ", S1[i]);
        printf("\n");
    }

    struct AliasEntryGMP* T = malloc(N * sizeof(struct AliasEntryGMP));

    int idx0 = 0;  // for S0
    int idx1 = 0;  // for S1

    for (int t = 0; t < N; t++) {
         if ((lenS0 - idx0 == 0) && (lenS1 - idx1 == 0)) {
            fprintf(stderr, "ERREUR2 : plus de lourds et de légers à l'étape %d !\n", t);
            exit(EXIT_FAILURE);
        }

        if (lenS1 - idx1 > 0) {
            int moins = S1[idx1];

            if (mpq_cmp(distrib[moins], pdsCase) == 0) {
                // T[t] = (moins, None, pdsCase)
                T[t].i = moins;
                T[t].j = -1;
                mpq_init(T[t].prob);
                mpq_set(T[t].prob, pdsCase);
                idx1++;
            } else {
                if (idx0 >= lenS0) {
                    fprintf(stderr, "[ERREUR] idx0=%d dépasse lenS0=%d à l'étape %d !\n", idx0, lenS0, t);
                    exit(EXIT_FAILURE);
                }
                int plus = S0[idx0];

                T[t].i = moins;
                T[t].j = plus;
                mpq_init(T[t].prob);
                mpq_set(T[t].prob, distrib[moins]);

                idx1++;  // S1 = S1[1:]

                // distrib[plus] -= (pdsCase - distrib[moins])
                mpq_t diff;
                mpq_init(diff);
                mpq_sub(diff, pdsCase, distrib[moins]);
                mpq_sub(distrib[plus], distrib[plus], diff);
                mpq_clear(diff);

                // distrib[moins] = 0
                mpq_set_ui(distrib[moins], 0, 1);

                if (mpq_cmp(distrib[plus], pdsCase) <= 0) {
                    if (idx1 <= 0) {
                        fprintf(stderr, "[ERREUR] idx1=%d ne peut pas être décrémenté à l'étape %d !\n", idx1, t);
                        exit(EXIT_FAILURE);
                    }
                    S1[--idx1] = plus;  // S1 = [plus] + S1
                    idx0++; // S0 = S0[1:]
                }
            }
        }else {
            // Case : no more light ones, but still heavy ones
            if (idx0 >= lenS0) {
                fprintf(stderr, "[ERREUR] idx0=%d dépasse lenS0=%d : plus de 'lourds' disponibles !\n", idx0, lenS0);
                exit(1);
            }

            int only_heavy = S0[idx0];
            T[t].i = only_heavy;
            T[t].j = -1;
            mpq_init(T[t].prob);
            mpq_set(T[t].prob, pdsCase);

            mpq_sub(distrib[only_heavy], distrib[only_heavy], pdsCase);

            if (mpq_cmp(distrib[only_heavy], pdsCase) <= 0) {
                if (idx1 <= 0) {
                    fprintf(stderr, "[ERREUR] idx1=%d ne peut pas être décrémenté dans le cas only_heavy à l'étape %d !\n", idx1, t);
                    exit(EXIT_FAILURE);
                }
                S1[--idx1] = only_heavy;  // S1 = [only_heavy] + S1
                idx0++;
            }

        }
    }

    // prob = k / pdsCase
    for (int t = 0; t < N; t++) {
        mpq_div(T[t].prob, T[t].prob, pdsCase);
    }

    // Cleaning
    mpq_clear(pdsCase);
    mpq_clear(res.pdsCase);
    free(res.S0);
    free(res.S1);

    if (affiche) {
        printf("T:\n");
        for (int t = 0; t < N; t++) {
            gmp_printf("(%d, %d, %Qd)\n", T[t].i, T[t].j, T[t].prob);
        }
    }

    return T;
}

struct sample_alias_fractions_gmp_s preprocess_alias_fractions_gmp(mpz_t* a, int n) {
    // Allouer et remplir une distribution rationnelle à partir de `a`
    mpq_t* distrib = malloc(n * sizeof(mpq_t));
    mpq_t total;
    mpq_init(total);
    mpq_set_ui(total, 0, 1);

    for (int i = 0; i < n; i++) {
        mpq_init(distrib[i]);
        mpq_set_z(distrib[i], a[i]);  // distrib[i] = a[i] / 1
        mpq_add(total, total, distrib[i]);
    }

    // Normaliser : distrib[i] = distrib[i] / total
    for (int i = 0; i < n; i++) {
        mpq_div(distrib[i], distrib[i], total);
    }

    mpq_clear(total);

    // Appel de l'algo principal
    struct AliasEntryGMP* T = algo_alias_fractions_gmp(distrib, n, 0);    // affichage = False

    // Libérer la distribution
    for (int i = 0; i < n; i++) {
        mpq_clear(distrib[i]);
    }
    free(distrib);

    // Construire et retourner la structure
    struct sample_alias_fractions_gmp_s result;
    result.taille = n;
    result.table = T;

    return result;
}