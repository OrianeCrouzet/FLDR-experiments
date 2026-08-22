/*
  Name:     sizes.c
  Purpose:  Functions to measure sizes of data structures for sampling algorithms.
  Author:   Oriane Crouzet
  Copyright (C) 2026 Oriane Crouzet, All Rights Reserved.
*/

#include "sizes.h"


// *********************************************************************************
//              ALIAS INTEGERS
// *********************************************************************************

size_t get_size_sample_alias_integers_s(struct sample_alias_integers_s *s) {
    size_t sampler_size = sizeof(*s);

    sampler_size += sizeof(s->T);
    if (s->T.data != NULL) {
        sampler_size += (size_t)s->T.size * sizeof(s->T.data[0]);
    }
    sampler_size += sizeof(s->Threshold);
    if (s->Threshold.data != NULL) {
        sampler_size += (size_t)s->Threshold.size * sizeof(s->Threshold.data[0]);
    }
    // sampler_size += sizeof(s->cs);
    // sampler_size += sizeof(s->virtual_obj);

    return sampler_size;
}


// *********************************************************************************
//              ALIAS INTEGERS - GMP (entiers taille arbitraire)
// *********************************************************************************

size_t get_size_sample_gmp_alias_integers_s(struct sample_gmp_alias_integers_s *s) {
    // Structure principale (inclut T, Threshold, cs et virtual_obj)
    size_t total = sizeof(*s);

    // Mémoire des entiers de T
    total += s->T.size * sizeof(int);

    // Mémoire pour le vecteur Threshold
    total += s->Threshold.size * sizeof(mpz_t);

    // Données utiles de chaque mpz_t dans Threshold
    for (unsigned int i = 0; i < s->Threshold.size; i++) {
        size_t bits = mpz_sizeinbase(s->Threshold.data[i], 2);
        size_t bytes = (bits + 7) / 8; // Conversion exacte bits -> octets
        total += bytes;
    }

    // Données utiles de cs (l'en-tête est déjà dans sizeof(*s))
    size_t cs_bits = mpz_sizeinbase(s->cs, 2);
    total += (cs_bits + 7) / 8;

    return total;
}


// *********************************************************************************
//              ALIAS FROM RUST
// *********************************************************************************

size_t get_size_sample_alias_rust_s(struct sample_alias_rust_s *s) {
    size_t size = sizeof(*s);
    if (s->aliases.data != NULL) {
        size += (size_t)s->aliases.size * sizeof(s->aliases.data[0]);
    }
    if (s->small.data != NULL) {
        size += (size_t)s->small.size * sizeof(s->small.data[0]);
    }
    if (s->large.data != NULL) {
        size += (size_t)s->large.size * sizeof(s->large.data[0]);
    }
    if (s->prob != NULL) {
        size += (size_t)s->n * sizeof(s->prob[0]);
    }

    return size;
}


// *********************************************************************************
//              ALIAS FROM RUST - GMP (entiers taille arbitraire)
// *********************************************************************************

size_t get_size_sample_alias_rust_gmp_s(struct sample_alias_rust_gmp_s *s) {
    // Structure principale (en-têtes des vecteurs, n, et en-tête weight_sum)
    size_t size = sizeof(*s);

    // Vecteur aliases (VectorInt)
    if (s->aliases.data != NULL) {
        size += (size_t)s->aliases.size * sizeof(int);
    }

    // Vecteurs d'indices d'attente (VectorInt)
    if (s->small.data != NULL) {
        size += (size_t)s->small.size * sizeof(int);
    }
    if (s->large.data != NULL) {
        size += (size_t)s->large.size * sizeof(int);
    }

    // Vecteur des probabilités (VectorMpz)
    if (s->prob.data != NULL) {
        // En-têtes mpz_t stockés dans le tableau
        size += (size_t)s->prob.size * sizeof(mpz_t);

        // Données dynamiques pour chaque mpz_t
        for (unsigned int i = 0; i < s->prob.size; i++) {
            size_t bits = mpz_sizeinbase(s->prob.data[i], 2);
            size += (bits + 7) / 8;
        }
    }

    // Données dynamiques de weight_sum
    size_t ws_bits = mpz_sizeinbase(s->weight_sum, 2);
    size += (ws_bits + 7) / 8;

    return size;
}


// *********************************************************************************
//              ALIAS FRACTIONS
// *********************************************************************************

size_t get_size_sample_alias_fractions_s(struct sample_alias_fractions_s *s) {
    size_t size = sizeof(*s);
    if (s->table != NULL) {
        size += (size_t)s->taille * sizeof(struct AliasEntry);
    }

    return size;
}


// *********************************************************************************
//              ALIAS FRACTIONS - GMP (entiers taille arbitraire)
// *********************************************************************************

size_t get_size_sample_alias_fractions_gmp_s(struct sample_alias_fractions_gmp_s *s) {
    // Structure principale (contient l'entier 'taille' et le pointeur 'table')
    size_t total = sizeof(*s);

    // Mémoire du tableau d'alias
    // (Inclut les int i, j et la structure d'en-tête mpq_t de chaque entrée)
    total += s->taille * sizeof(struct AliasEntryGMP);

    // Données utiles des fractions GMP
    for (int i = 0; i < s->taille; i++) {
        mpz_srcptr num = mpq_numref(s->table[i].prob);
        mpz_srcptr den = mpq_denref(s->table[i].prob);

        size_t num_bits = mpz_sizeinbase(num, 2);
        size_t den_bits = mpz_sizeinbase(den, 2);

        // Conversion bits -> octets
        total += (num_bits + 7) / 8;
        total += (den_bits + 7) / 8;
    }

    return total;
}

// *********************************************************************************
//              FLDR
// *********************************************************************************

size_t get_size_sample_ky_encoding_s(struct sample_ky_encoding_s *s) {
    size_t size = sizeof(*s);
    if (s->encoding.a != NULL) {
        size += (size_t)s->encoding.length * sizeof(s->encoding.a[0]);
    }
    return size;
}


// *********************************************************************************
//              FLDR - GMP (entiers taille arbitraire)
// *********************************************************************************

size_t get_size_sample_ky_encoding_gmp_s(struct sample_ky_encoding_gmp_s *s) {
    size_t total = sizeof(*s);
    total += sizeof(mpz_t) * 2;

    if (s->encoding.data != NULL) {
        total += (size_t)s->encoding.size * sizeof(mpz_t);
        for (unsigned int i = 0; i < s->encoding.size; i++) {
            size_t nbits = mpz_sizeinbase(s->encoding.data[i], 2);
            total += (nbits + 7) / 8;
        }
    }

    return total;
}


// *********************************************************************************
//              ALIAS WALKER/VOSE
// *********************************************************************************

size_t get_size_sample_alias_exact_s(struct sample_alias_exact_s *s) {
    size_t size = sizeof(*s);
    if (s->ratios != NULL) {
        size += (size_t)s->n * sizeof(s->ratios[0]);
    }
    if (s->j.a != NULL) {
        size += (size_t)s->j.length * sizeof(s->j.a[0]);
    }
    return size;
}


// *********************************************************************************
//              ALDR
// *********************************************************************************

size_t get_size_sample_aldr_s(struct sample_aldr_s *s) {
    size_t size = sizeof(*s);
    if (s->breadths != NULL) {
        size += (size_t)s->length_breadths * sizeof(s->breadths[0]);
    }
    if (s->leaves_flat != NULL) {
        size += (size_t)s->length_leaves_flat * sizeof(s->leaves_flat[0]);
    }

    return size;
}


// *********************************************************************************
//              ALDR - GMP (entiers taille arbitraire)
// *********************************************************************************

size_t get_size_sample_aldr_gmp_s(struct sample_aldr_gmp_s* x) {
    // Structure principale (inclut breadths, leaves_flat et les en-têtes mpz_t de length_*)
    size_t total = sizeof(*x);

    // Données utiles de length_breadths
    size_t bits_lb = mpz_sizeinbase(x->length_breadths, 2);
    total += (bits_lb + 7) / 8;

    // Données utiles de length_leaves_flat
    size_t bits_llf = mpz_sizeinbase(x->length_leaves_flat, 2);
    total += (bits_llf + 7) / 8;

    // Vecteur breadths (VectorMpz)
    if (x->breadths.data != NULL) {
        total += (size_t)x->breadths.size * sizeof(mpz_t);
        for (unsigned int i = 0; i < x->breadths.size; i++) {
            size_t nbits = mpz_sizeinbase(x->breadths.data[i], 2);
            total += (nbits + 7) / 8;
        }
    }

    // Vecteur leaves_flat (VectorMpz)
    if (x->leaves_flat.data != NULL) {
        total += (size_t)x->leaves_flat.size * sizeof(mpz_t);
        for (unsigned int i = 0; i < x->leaves_flat.size; i++) {
            size_t nbits = mpz_sizeinbase(x->leaves_flat.data[i], 2);
            total += (nbits + 7) / 8;
        }
    }

    return total;
}