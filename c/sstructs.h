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
#include"vector_mpz.h"
#include "fraction.h"


// *********************************************************************************
//              UTILS
// *********************************************************************************

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

// sample_fdr
struct sample_fdr_s {
    int n;
};

// sample_inversion_bernoulli
struct sample_inversion_bernoulli_s {
    int a;
    int M;
};


// *********************************************************************************
//              FLDR
// *********************************************************************************

// sample_ky_encoding
struct sample_ky_encoding_s {
    int n;
    int k;
    struct array_s encoding;
};


// *********************************************************************************
//              FLDR - GMP (entiers taille arbitraire)
// *********************************************************************************

struct sample_ky_encoding_gmp_s {
    mpz_t n;
    mpz_t k;
    VectorMpz encoding;
};


// *********************************************************************************
//              ALIAS WALKER/VOSE
// *********************************************************************************

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


// *********************************************************************************
//              ALIAS INTEGERS
// *********************************************************************************

struct sample_alias_integers_s {
    VectorInt T;
    VectorInt Threshold;
    uint32_t cs;
    int virtual_obj; // index of the virtual object, -1 if absent
};


// *********************************************************************************
//              ALIAS INTEGERS - GMP (entiers taille arbitraire)
// *********************************************************************************

struct sample_gmp_alias_integers_s{
    VectorInt T;
    VectorMpz Threshold;
    mpz_t cs;
    int virtual_obj; // index of the virtual object, -1 if absent
};


// *********************************************************************************
//              ALDR
// *********************************************************************************

struct sample_aldr_s {
    // flattened ALDR tree
    int length_breadths;
    int length_leaves_flat;
    int *breadths;
    int *leaves_flat;
};


// *********************************************************************************
//              ALDR - GMP (entiers taille arbitraire)
// *********************************************************************************

// flattened ALDR tree
struct sample_aldr_gmp_s
{
    mpz_t length_breadths;
    mpz_t length_leaves_flat;
    VectorMpz breadths;
    VectorMpz leaves_flat;
};


// *********************************************************************************
//              ALIAS FROM RUST
// *********************************************************************************

typedef struct {
    VectorInt aliases;
    //double *prob;
    unsigned int *prob;
    VectorInt small;
    VectorInt large;
    unsigned int n;
    //double weight_sum;
    unsigned int weight_sum;
} alias_rust_s;

typedef enum {
    WEIGHTED_OK = 0,
    WEIGHTED_NO_ITEM,
    WEIGHTED_TOO_MANY,
    WEIGHTED_INVALID_WEIGHT,
    WEIGHTED_ALL_ZERO
} WeightedError;

struct sample_alias_rust_s {
    VectorInt aliases;
    double *prob;
    VectorInt small;
    VectorInt large;
    unsigned int n;
    double weight_sum;
};


// *********************************************************************************
//              ALIAS FRACTIONS
// *********************************************************************************

struct AliasEntry{
    int i;          
    int j;          // -1 if NULL
    struct Fraction prob;     // = k / pdsCase
};

// Structure of Alias Fractions
struct sample_alias_fractions_s{
    int taille;           // Number of elements
    struct AliasEntry* table;    // AliasEntry table
};

#endif
