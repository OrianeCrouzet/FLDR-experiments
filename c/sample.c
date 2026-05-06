/*
  Name:     sample.c
  Purpose:  Exact samplers for discrete probability distributions.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdbool.h>
#include <stdlib.h>

#include "flip.h"
#include "sample.h"
#include "sstructs.h"
#include "utils.h"
#include "alias_rust.h"
#include "alias_fractions.h"
#include "fraction.h"

int sample_ky_encoding(struct sample_ky_encoding_s *x) {

    if (x->encoding.length == 1) {
        return 1;
    }

    int *enc = x->encoding.a;
    int c = 0;
    while (true) {
        int b = flip();
        c = enc[c+b];
        if (enc[c] < 0) {
            return -enc[c];
        }
    }
}

int sample_ky_matrix(struct sample_ky_matrix_s *x) {
    if (x->P.nrows == 1) {
        return 1;
    }

    int **P = x->P.P;
    int c = 0;
    int d = 0;

    while (true) {
        int b = flip();
        d = 2 * d + (1-b);
        for (int r = 0; r < x->P.nrows; r++) {
            d = d - P[r][c];
            if (d == - 1) {
                return r + 1;
            }
        }
        if (c == x->k - 1) {
            c = x->l;
        } else {
            c = c + 1;
        }
    }
}

int sample_ky_matrix_cached(struct sample_ky_matrix_cached_s *x) {
    if (x->T.nrows == 1) {
        return 1;
    }

    int **T = x->T.P;
    int *h = x->h.a;

    int c = 0;
    int d = 0;

    while (true) {
        int b = flip();
        d = 2 * d + (1-b);
        if (d < h[c]) {
            return T[d][c] + 1;
        }
        d = d - h[c];
        if (c == x->k - 1) {
            c = x->l;
        } else {
            c = c + 1;
        }
    }
}

int sample_fdr(struct sample_fdr_s *x) {
    int v = 1;
    int c = 0;

    while (true) {
        int b = flip();
        v = v << 1;
        c = (c << 1) + b;
        if (x->n <= v) {
            if (c < x->n) {
                return c + 1;
            } else {
                v = v - x->n;
                c = c - x->n;
            }
        }
    }
} 

int sample_inversion_bernoulli(struct sample_inversion_bernoulli_s *x) {
    int v = x->a;
    int M = x->M;
    int y;

    while (true) {
        v = 2*v;
        if (M <= v) {
            v = v - M;
            y = 1;
        } else {
            y = 0;
        }
        int b = flip();
        if (b == 1) {
            return y;
        }
    }
}

int sample_rejection_uniform(struct sample_rejection_uniform_s *x) {
    struct sample_fdr_s n = {.n = x->n};
    while (true) {
        int j = sample_fdr(&n);
        int b = sample_inversion_bernoulli(&(x->ratios[j-1]));
        if (b == 1) {
            return j;
        }
    }
}

int sample_rejection_hash_table(struct sample_rejection_hash_table_s *x) {
    int *T = x->T.a;
    while (true) {
        int W = randint(x->k);
        if (W < x->Z) {
            return T[W];
        }
    }
}

int sample_rejection_binary_search(struct sample_rejection_binary_search_s *x) {
    while (true) {
        int W = randint(x->k);
        if (W < x->Z) {
            int j = binary_search_interval(x->cdf.a, x->cdf.length, W);
            return j + 1;
        }
    }

}

int sample_rejection_encoding(struct sample_ky_encoding_s *x) {
    int *enc = x->encoding.a;
    int n = x->n;
    int c = 0; int s;
    while (true) {
        int b = flip();
        c = enc[c+b];
        s = -enc[c];
        if (s > 0) {
            if ( s < n ) {
                return s;
            } else {
                c = 0;
            }
        }
    }
}

int sample_rejection_matrix(struct sample_ky_matrix_s *x) {
    int n = x->P.nrows;
    while (true) {
        int s = sample_ky_matrix(x);
        if (s < n) {
            return s;
        }
    }
}

int sample_rejection_matrix_cached(struct sample_ky_matrix_cached_s *x) {

    int **T = x->T.P;
    int *h = x->h.a;

    int n = x->T.nrows;
    int c = 0;
    int d = 0;

    while (true) {
        int b = flip();
        d = 2 * d + (1-b);
        if (d < h[c]) {
            int s = T[d][c];
            if (s < n-1) {
                return s + 1;
            } else {
                d = 0;
                c = 0;
            }
        } else {
            d = d - h[c];
            c = c + 1;
        }
    }
}

int sample_interval(struct sample_interval_s *x) {
    int alpha = 0;
    int beta = 1;
    int alpha_prev = 0;
    int denominator = 1;
    int location = -1;
    int b = 0;

    while (location == -1) {
        b = flip();

        alpha_prev = alpha;
        alpha = 2 * alpha + (beta - alpha) * b;
        beta = 2 * alpha_prev + (beta - alpha_prev) * (b+1);

        if ((alpha % 2 == 0) && (beta % 2 == 0)) {
            alpha /= 2;
            beta /= 2;
        } else {
            denominator *= 2;
        }

        location = binary_search_interval_nested(
            x->cdf.a, x->Z, x->cdf.length, alpha, beta, denominator);
    }

    return location;
}

int sample_alias_gsl(struct sample_alias_gsl_s *x) {
    int draw = gsl_ran_discrete(x->prng, x->distribution);
    return draw;
}

int sample_alias_exact(struct sample_alias_exact_s *x) {
    struct sample_fdr_s xn = {.n = x->n};
    int n = sample_fdr(&xn);
    int b = sample_inversion_bernoulli(&(x->ratios[n-1]));
    if (b == 1) {
        return n;
    } else {
        return x->j.a[n-1] + 1;
    }
}

// Mettre 2 à la fin de la fonction que vous ne voulez PAS utiliser (ça évite de modifier les appels partout dans le code...)
// sample_alias_integers
uint32_t sample_alias_integers(struct sample_alias_integers_s *x) {
    // ORIGINALE
    // printf("tirage integers");
    // fflush(stdout);
    int q;

    // Tirage d'une case dans la table
    uint32_t n = x->Threshold.size;
    q = uniform(n);
    
    // Tirage de Bernoulli pour savoir si on prend T[2q] ou T[2q + 1]
    uint32_t b = bernoulli(x->Threshold.data[(unsigned int)q], x->cs);

    // Calcul de l'index final dans T sans branchement conditionnel
    uint32_t final_index = 2 * (unsigned int)q + 1 - b;    

    return x->T.data[final_index];
}

uint32_t sample_alias_integers2(struct sample_alias_integers_s *x) {
    // VERSION 2 d'Antoine
    int q;

    // Tirage d'une case dans la table
    int n = x->Threshold.size;
    q = uniform(n);
    
    // Tirage de Bernoulli pour savoir si on prend T[2q] ou T[2q + 1]
    int numer = x->Threshold.data[(unsigned int)q];
    int denom = x->cs;
    uint32_t b = numer==denom || bernoulli(numer, denom);

    // Calcul de l'index final dans T sans branchement conditionnel
    uint32_t final_index = 2 * (unsigned int)q + 1 - b;    

    return x->T.data[final_index];
}

// sample_aldr
int sample_aldr(struct sample_aldr_s *x){
    while (1) {
        int depth = 0;
        int location = 0;
        int val = 0;
        for (;;) {
            if (val < x->breadths[depth]) {
                int ans = x->leaves_flat[location + val];
                if (ans) return ans - 1;
                else break;
            }
            location += x->breadths[depth];
            val = ((val - x->breadths[depth]) << 1) | flip();
            ++depth;
        }
    }
}

// sample_alias_rust
static unsigned int alias_rust_rng_index(unsigned int n) {
    return (n <= 1) ? 0u : uniform(n);
}

static unsigned int alias_rust_rng_weight(unsigned int max) {
    if (max <= 1) {
        return 0;
    }
    return (unsigned int) uniform((uint32_t) max);
}

uint32_t sample_alias_rust(struct sample_alias_rust_s *x){
    return weighted_alias_sample(
        (alias_rust_s *)x,
        alias_rust_rng_index,
        alias_rust_rng_weight
    );
}

// alias_fractions ORIGINALE
uint32_t sample_alias_fractions(struct sample_alias_fractions_s *x) {
    uint32_t n = (uint32_t)x->taille;

    uint32_t index = uniform(n);

    struct Fraction f = x->table[index].prob;

    uint32_t numer = f.num;
    uint32_t denom = f.denom;
    uint32_t b = bernoulli(numer, denom);

    return b ? x->table[index].i : x->table[index].j;
}

// A SUPPRIMER (vérifier dépendances)
uint32_t sample_alias_fractions2(struct sample_alias_fractions_s *x) {
    uint32_t n = x->taille;
    uint32_t index = uniform(n);

    if (x->table[index].j == -1)
        return x->table[index].i;

    struct Fraction f = x->table[index].prob;

    return bernoulli(f.num, f.denom)
        ? x->table[index].i
        : x->table[index].j;
}
