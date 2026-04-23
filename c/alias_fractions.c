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
#include <gmp.h>

#include "sstructs.h"
#include "alias_fractions.h"
#include "flip.h"
#include "fraction.h"

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

                // distrib[plus] -= diff
                struct Fraction new_plus = frac_sub(distrib[plus], diff);
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
    }

    free(res.S0);
    free(res.S1);

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

    if (total == 0) {
        fprintf(stderr, "ERROR: total = 0 in preprocess_alias_fractions\n");
        exit(1);
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