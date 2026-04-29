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

// DEBUG MODE
#define DBG 1
#define DPRINT(...) if (DBG) printf(__VA_ARGS__)

struct Fraction* ints_to_fractions(int* vals, int N) {
    int sum = 0;
    for (int i = 0; i < N; i++) sum += vals[i];
    struct Fraction* probs = malloc(N * sizeof(struct Fraction));
    for (int i = 0; i < N; i++) {
        probs[i].num = vals[i];
        probs[i].denom = sum;
        probs[i] = frac_reduce(probs[i]);
    }
    return probs;
}

PileResult piles(struct Fraction* distrib, int N) {
    PileResult res;

    // pdsCase = 1 / N
    res.pdsCase.num = 1;
    res.pdsCase.denom = (uint32_t) N;

    res.S0 = malloc(N * sizeof(int));
    res.S1 = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        res.S0[i] = -1;
        res.S1[i] = -1;
    }

    res.lenS0 = 0;
    res.lenS1 = 0;

    for (int n = 0; n < N; n++) {

        // distrib[n] > 1/N
        // then distrib[n].num / distrib[n].denom > 1 / N
        // then distrib[n].num * N > distrib[n].denom

        uint64_t left  = (uint64_t)distrib[n].num * (uint64_t)N;
        uint64_t right = distrib[n].denom;

        if (left > right) {
            res.S0[res.lenS0++] = n;
        } else {
            res.S1[res.lenS1++] = n;
        }
    }

    return res;
}

struct AliasEntry* algo_alias_fractions(struct Fraction* distrib, int N) {
    // ORIGINALE
    
    PileResult res = piles(distrib, N);
    struct Fraction pdsCase = res.pdsCase;

    int* S0 = res.S0;
    int* S1 = res.S1;
    int lenS0 = res.lenS0;
    int lenS1 = res.lenS1;

    struct AliasEntry* T = malloc(N * sizeof(struct AliasEntry));

    int idx0 = 0;
    int idx1 = 0;

    for (int t = 0; t < N; t++) {

        if ((lenS0 - idx0 == 0) && (lenS1 - idx1 == 0)) {
            fprintf(stderr, "ERREUR: plus de lourds et légers à t=%d\n", t);
            exit(EXIT_FAILURE);
        }

        if (lenS1 - idx1 > 0) {
            int moins = S1[idx1];

            if (frac_eq(distrib[moins], pdsCase)) {

                T[t].i = moins;
                T[t].j = -1;
                T[t].prob = frac_copy(pdsCase);
                idx1++;

            } else {
                int plus = S0[idx0];
                T[t].i = moins;
                T[t].j = plus;
                T[t].prob = frac_copy(distrib[moins]);

                idx1++;

                // diff = pdsCase - distrib[moins]
                struct Fraction diff = frac_sub(pdsCase, distrib[moins]);
                diff = frac_reduce(diff);

                // distrib[plus] -= diff
                struct Fraction new_plus = frac_sub(distrib[plus], diff);
                new_plus = frac_reduce(new_plus);
                distrib[plus] = new_plus;

                // distrib[moins] = 0
                distrib[moins] = frac_zero();

                if (frac_le(distrib[plus], pdsCase)) {
                    S1[--idx1] = plus;
                    idx0++;
                }
            }

        } else {

            int only_heavy = S0[idx0];

            T[t].i = only_heavy;
            T[t].j = -1;
            T[t].prob = frac_copy(pdsCase);

            distrib[only_heavy] =
                frac_sub(distrib[only_heavy], pdsCase);

            distrib[only_heavy] = frac_reduce(distrib[only_heavy]);

            if (frac_le(distrib[only_heavy], pdsCase)) {
                S1[--idx1] = only_heavy;
                idx0++;
            }
        }
    }

    // normalization step (ancien mpq_div)
    for (int t = 0; t < N; t++) {

        // T[t].prob = T[t].prob / pdsCase
        uint64_t num = (uint64_t)T[t].prob.num * pdsCase.denom;
        uint64_t den = (uint64_t)T[t].prob.denom * pdsCase.num;

        T[t].prob.num = (uint32_t)num;
        T[t].prob.denom = (uint32_t)den;

        T[t].prob = frac_reduce(T[t].prob);
    }

    for (int i = 0; i < N; i++) {
        printf("%d: i=%d j=%d prob=%d/%d\n",
            i,
            T[i].i,
            T[i].j,
            T[i].prob.num,
            T[i].prob.denom);
    }

    free(res.S0);
    free(res.S1);

    return T;
}

struct AliasEntry* algo_alias_fractions2(struct Fraction* distrib, int N) {
    // DEBUG de la fonction juste au-dessus

    PileResult res = piles(distrib, N);
    struct Fraction pdsCase = res.pdsCase;

    int* S0 = res.S0;
    int* S1 = res.S1;
    int lenS0 = res.lenS0;
    int lenS1 = res.lenS1;

    struct AliasEntry* T = malloc(N * sizeof(struct AliasEntry));

    int idx0 = 0;
    int idx1 = 0;

    DPRINT("\n================ ALIAS DEBUG ================\n");
    DPRINT("N=%d lenS0=%d lenS1=%d pdsCase=%d/%d\n",
           N, lenS0, lenS1, pdsCase.num, pdsCase.denom);

    int t = 0;

    // =========================================================
    // FIX 1: boucle sécurisée (évite écriture hors structures)
    // =========================================================
    while (t < N && (idx0 < lenS0 || idx1 < lenS1)) {

        DPRINT("\n--------------------------------------------\n");
        DPRINT("[t=%d] idx0=%d idx1=%d\n", t, idx0, idx1);

        // =====================================================
        // FIX 2: check réel de disponibilité S1/S0 avant accès
        // =====================================================
        if (idx1 < lenS1) {

            int moins = S1[idx1];

            DPRINT("S1[%d]=%d\n", idx1, moins);

            // -------------------------------------------------
            // CAS EXACT
            // -------------------------------------------------
            if (frac_eq(distrib[moins], pdsCase)) {

                T[t].i = moins;
                T[t].j = -1;   // FIX 3: plus de -1 → self-loop safe
                T[t].prob = frac_copy(pdsCase);

                idx1++;
                t++;

            } else {

                // -------------------------------------------------
                // CAS ALIAS
                // -------------------------------------------------

                // FIX 4: sécurité accès S0
                if (idx0 >= lenS0) {
                    printf("💥 S0 exhausted while S1 active (t=%d)\n", t);
                    exit(1);
                }

                int plus = S0[idx0];

                T[t].i = moins;
                T[t].j = plus;
                T[t].prob = frac_copy(distrib[moins]);

                idx1++;

                struct Fraction diff = frac_sub(pdsCase, distrib[moins]);
                diff = frac_reduce(diff);

                distrib[plus] = frac_reduce(frac_sub(distrib[plus], diff));
                distrib[moins] = frac_zero();

                if (frac_le(distrib[plus], pdsCase)) {

                    // FIX 5: réinsertion sécurisée S1
                    if (idx1 <= 0) {
                        printf("💥 idx1 underflow detected at t=%d\n", t);
                        exit(1);
                    }

                    S1[--idx1] = plus;
                    idx0++;
                }

                t++;
            }

        } else if (idx0 < lenS0) {

            // =====================================================
            // ONLY HEAVY CASE
            // =====================================================

            int only_heavy = S0[idx0];

            T[t].i = only_heavy;
            T[t].j = -1;  // FIX 6: suppression de -1
            T[t].prob = frac_copy(pdsCase);

            distrib[only_heavy] =
                frac_reduce(frac_sub(distrib[only_heavy], pdsCase));

            if (frac_le(distrib[only_heavy], pdsCase)) {

                if (idx1 <= 0) {
                    printf("💥 idx1 underflow in heavy case at t=%d\n", t);
                    exit(1);
                }

                S1[--idx1] = only_heavy;
                idx0++;
            }

            t++;

        } else {

            // =====================================================
            // FIX 7: arrêt propre si structures épuisées
            // =====================================================
            DPRINT("⚠️ structures épuisées avant N (t=%d)\n", t);
            break;
        }
    }

    // =========================================================
    // NORMALISATION (inchangée mais sécurisée)
    // =========================================================

    DPRINT("\n=========== NORMALISATION ===========\n");

    for (int i = 0; i < t; i++) {

        uint64_t num = (uint64_t)T[i].prob.num * pdsCase.denom;
        uint64_t den = (uint64_t)T[i].prob.denom * pdsCase.num;

        T[i].prob.num = (uint32_t)num;
        T[i].prob.denom = (uint32_t)den;

        T[i].prob = frac_reduce(T[i].prob);

        DPRINT("T[%d] = %d/%d (i=%d j=%d)\n",
               i,
               T[i].prob.num,
               T[i].prob.denom,
               T[i].i,
               T[i].j);
    }

    DPRINT("====================================\n\n");

    free(res.S0);
    free(res.S1);

    return T;
}

struct AliasEntry* algo_alias_fractions3(struct Fraction* distrib, int N) {
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

    struct AliasEntry* T = algo_alias_fractions3(distrib, n);

    free(distrib);

    struct sample_alias_fractions_s result;
    result.taille = n;
    result.table = T;

    return result;
}