/*
  Name:     sstructs.h
  Purpose:  Structures for exact sampling algorithms.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saads, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef SSTRUCTS_H
#define SSTRUCTS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gmp.h>
#include "vector_int.h"
#include "fraction.h"

// matrix
struct matrix_s {
    int nrows;
    int ncols;
    int **P;
};

// array
struct array_s {
    int length;
    int *a;
};


// sample_ky_encoding
struct sample_ky_encoding_s {
    int n;
    int k;
    struct array_s encoding;
};

// sample_ky_matrix
struct sample_ky_matrix_s {
    int k;
    int l;
    struct matrix_s P;
};

// sample_ky_matrix_cached
struct sample_ky_matrix_cached_s {
    int k;
    int l;
    struct array_s h;
    struct matrix_s T;
};

// sample_fdr
struct sample_fdr_s {
    int n;
};

// sample_inversion_bernoulli
struct sample_inversion_bernoulli_s {
    int a;
    int M;
};

// sample_rejection_uniform
struct sample_rejection_uniform_s {
    int n;
    int M;
    struct array_s Ms;
    struct sample_inversion_bernoulli_s *ratios;
};

// sample_rejection_hash_table
struct sample_rejection_hash_table_s {
    int k;
    int Z;
    struct array_s T;
};

// sample_rejection_binary_search
struct sample_rejection_binary_search_s {
    int k;
    int Z;
    struct array_s cdf;
};

// sample_interval
struct sample_interval_s {
    int k;
    int Z;
    struct array_s cdf;
};

// sample_alias_gsl
struct sample_alias_exact_s {
    int n;
    struct sample_inversion_bernoulli_s *ratios;
    struct array_s j;
};

// sample_alias_gsl
struct sample_alias_gsl_s {
    gsl_rng *prng;
    gsl_ran_discrete_t *distribution;
};

// ***** Mes ajouts *****

// sample_alias_integers
struct sample_alias_integers_s {
    VectorInt T;
    VectorInt Threshold;
    uint32_t cs;
    int virtual_obj; // index de l'objet virtuel, -1 si absent
};

// sample_aldr
struct sample_aldr_s {
    // flattened ALDR tree
    int length_breadths;
    int length_leaves_flat;
    int *breadths;
    int *leaves_flat;
};

// sample_alias_rust
struct sample_alias_rust_s {
    VectorInt aliases;
    double *prob;
    VectorInt small;
    VectorInt large;
    unsigned int n;
    double weight_sum;
};

// sample_alias_fractions
struct AliasEntry{
    int i;          
    int j;          // -1 if NULL
    struct Fraction prob;     // = k / pdsCase
};

// Structure de l'Alias Fractions
struct sample_alias_fractions_s{
    int taille;           // Nombre d'éléments
    struct AliasEntry* table;    // Tableau de AliasEntry
};

#endif
