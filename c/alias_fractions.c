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

// FDR with GMP, translated from flip.c (Saad)
void uniform_with_gmp(mpz_t result, const mpz_t n) {
    size_t num_bits_presample = mpz_sizeinbase(n, 2); // num_bits_presample = 32 - __builtin_clz(n - 1);
    
    mpz_t bound, x;
    mpz_inits(bound, x, NULL);

    mpz_ui_pow_ui(bound, 2, num_bits_presample);   // bound = 1 << num_bits_presample
    mpz_set_ui(x, flip_n(num_bits_presample));     // x = flip_n(num_bits_presample)

    for (;;) {
        if (mpz_cmp(bound, n) >= 0) {
            if (mpz_cmp(x, n) < 0) { 
                mpz_set(result, x);
                break;
            }
            mpz_sub(bound, bound, n);
            mpz_sub(x, x, n);
        }
        mpz_mul_2exp(bound, bound, 1);  // bound <<= 1
        mpz_mul_2exp(x, x, 1);          
        mpz_add_ui(x, x, flip());       // x = (x << 1) | flip();
    }
    mpz_clears(x, bound, NULL);
}

//Bernoulli with GMP, translated from flip.c (Saad)
int bernoulli_with_gmp(mpz_t numer, mpz_t denom) {
    if (mpz_cmp_ui(numer, 0) == 0) return 0;
    if (mpz_cmp(numer, denom) == 0) return 1; 

    int y;

    for (;;) {
        mpz_mul_2exp(numer, numer, 1);  //numer <<= 1

        if (mpz_cmp(numer, denom) == 0) {
            return flip();
        }
        if (y = (mpz_cmp(numer, denom) > 0)) {
            mpz_sub(numer, numer, denom);
        }
        if (flip()) {
            return y;
        }
    }
}

PileResult piles(mpq_t* distrib, int N) {
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

    return res;
}

struct AliasEntry* algo_alias_fractions(mpq_t* distrib, int N) {
    // piles(distrib) -> pdsCase, S0, S1
    PileResult res = piles(distrib, N);
    mpq_t pdsCase;
    mpq_init(pdsCase);
    mpq_set(pdsCase, res.pdsCase);  // pdsCase = 1/N

    int* S0 = res.S0;
    int lenS0 = res.lenS0;
    int* S1 = res.S1;
    int lenS1 = res.lenS1;

    struct AliasEntry* T = malloc(N * sizeof(struct AliasEntry));

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

    return T;
}

void free_sample_alias_fractions(struct sample_alias_fractions_s x) {
    for (int i = 0; i < x.taille; i++) {
        mpq_clear(x.table[i].prob);  // Libère le mpq_t
    }
    free(x.table);  // Libère le tableau
}

struct sample_alias_fractions_s preprocess_alias_fractions(int* a, int n) {
    // Allouer et remplir une distribution rationnelle à partir de `a`
    mpq_t* distrib = malloc(n * sizeof(mpq_t));
    mpq_t total;
    mpq_init(total);
    mpq_set_ui(total, 0, 1);

    for (int i = 0; i < n; i++) {
        mpq_init(distrib[i]);
        mpq_set_ui(distrib[i], a[i], 1);  // distrib[i] = a[i] / 1
        mpq_add(total, total, distrib[i]);
    }

    // Normaliser : distrib[i] = distrib[i] / total
    for (int i = 0; i < n; i++) {
        mpq_div(distrib[i], distrib[i], total);
    }

    mpq_clear(total);

    // Appel de l'algo principal
    struct AliasEntry* T = algo_alias_fractions(distrib, n);

    // Libérer la distribution
    for (int i = 0; i < n; i++) {
        mpq_clear(distrib[i]);
    }
    free(distrib);

    // Construire et retourner la structure
    struct sample_alias_fractions_s result;
    result.taille = n;
    result.table = T;

    return result;
}