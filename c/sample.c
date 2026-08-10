/*
  Name:     sample.c
  Purpose:  Exact samplers for discrete probability distributions.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdbool.h>
#include <stdlib.h>
#include <gmp.h>

#include "flip.h"
#include "sample.h"
#include "sstructs.h"
#include "construct.h"
#include "fraction.h"

unsigned long REJET = 0;

// *********************************************************************************
//              UTILS
// *********************************************************************************

int sample_ky_encoding(struct sample_ky_encoding_s *x) {

    if (x->encoding.length == 1) {
        return 1;
    }

    int *enc = x->encoding.a;
    int c = 0;
    while (true) {
        int b = get_random_bits_spsc(1);
        c = enc[c+b];
        if (enc[c] < 0) {
            return -enc[c];
        }
    }
}

int sample_fdr(struct sample_fdr_s *x) {
    int v = 1;
    int c = 0;

    while (true) {
        int b = get_random_bits_spsc(1);
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
        int b = get_random_bits_spsc(1);
        if (b == 1) {
            return y;
        }
    }
}


// *********************************************************************************
//              FLDR
// *********************************************************************************

int sample_rejection_encoding(struct sample_ky_encoding_s *x) {
    int *enc = x->encoding.a;
    int n = x->n;
    int c = 0; int s;
    while (true) {
        int b = get_random_bits_spsc(1);
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


// *********************************************************************************
//              ALIAS WALKER/VOSE
// *********************************************************************************

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


// *********************************************************************************
//              ALIAS INTEGERS
// *********************************************************************************

// REJET OBJ VIRTUEL
// TODO : est-c equ'on garde cette version ??
uint32_t sample_alias_integers(struct sample_alias_integers_s *x) {
    int q = 0;
    int n = x->Threshold.size;
    uint32_t result;

    do {
        unsigned int q_index = (unsigned int)uniform(n);

        uint32_t b = bernoulli(x->Threshold.data[q_index], x->cs);

        uint32_t final_index = 2 * q_index + 1 - b;

        result = x->T.data[final_index];

    } while (x->virtual_obj != -1 &&
             result == (uint32_t)x->virtual_obj);

    return result;
}


// REJET OBJ VIRTUEL
uint32_t sample_alias_integers_old(struct sample_alias_integers_s *x) {
    int q = 0;
    int n = x->Threshold.size;
    uint32_t result;

    REJET -= 1;
    
    do {
        unsigned int q_index = (unsigned int)uniform(n);

        uint32_t b = bernoulli(x->Threshold.data[q_index], x->cs);

        uint32_t final_index = 2 * q_index + 1 - b;

        result = x->T.data[final_index];

        REJET += 1; 

    } while (result == (uint32_t)x->virtual_obj);

    return result;
}

// x->virtual_obj != -1 &&


// *********************************************************************************
//              ALIAS INTEGERS - GMP (entiers taille arbitraire)
// *********************************************************************************

uint32_t sample_gmp_alias_integers(struct sample_gmp_alias_integers_s* x) {
    mpz_t q;
    mpz_init(q);
    mpz_t n;
    mpz_init_set_ui(n, x->Threshold.size);
    uint32_t result;

    REJET -= 1;

    do{
        uniform_with_gmp(q, n);
        unsigned int q_index = mpz_get_ui(q);
        int b = bernoulli_with_gmp(x->Threshold.data[q_index], x->cs);
        unsigned int final_index = 2 * q_index + 1 - b;  
        result = x->T.data[final_index];

        REJET += 1; 
    } while (result == (uint32_t)x->virtual_obj);
    
    mpz_clears(q, n, NULL);
    return result;
}


// *********************************************************************************
//              ALDR
// *********************************************************************************

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
            val = ((val - x->breadths[depth]) << 1) | get_random_bits_spsc(1);
            ++depth;
        }
    }
}


// *********************************************************************************
//              ALIAS FROM RUST
// *********************************************************************************

unsigned int weighted_alias_sample(
    alias_rust_s *dist,
    unsigned int (*rng_index)(unsigned int),
    unsigned int (*rng_weight)(unsigned int)
)
{
    unsigned int i =
        rng_index(dist->n);

    unsigned int r =
        rng_weight(dist->weight_sum);

    if(r < dist->prob[i])
        return i;

    return vector_get(
        &dist->aliases,
        i
    );
}

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


// *********************************************************************************
//              ALIAS FRACTIONS
// *********************************************************************************

uint32_t sample_alias_fractions(struct sample_alias_fractions_s *x) {
    uint32_t n = (uint32_t)x->taille;

    uint32_t index = uniform(n);

    struct Fraction f = x->table[index].prob;

    uint32_t numer = f.num;
    uint32_t denom = f.denom;
    uint32_t b = bernoulli(numer, denom);

    return b ? x->table[index].i : x->table[index].j;
}
