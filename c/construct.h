/*
  Name:     construct.h
  Purpose:  Utilities for constructing data structures.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include <stdint.h>
#include <gmp.h>
#include "vector_int.h"
#include"vector_mpz.h"
#include "sstructs.h"
#include "fraction.h"


void cons_alias5(unsigned int n, VectorInt* D, uint32_t cs, VectorInt* T, VectorInt* Threshold);
struct sample_alias_integers_s preprocess_alias_integers_old(int* a, int n);

void poids_total(VectorMpz D, unsigned int size, mpz_t result);
void cons_alias(unsigned int n, VectorMpz D, mpz_t cs, VectorInt* T, VectorMpz* Threshold);
struct sample_gmp_alias_integers_s preprocess_gmp_alias_integers(int* a, int n);

struct sample_aldr_s preprocess_aldr_flat_k(int* a, int n, int kmul);
struct sample_aldr_s preprocess_aldr_flat(int* a, int n);

struct sample_aldr_gmp_s preprocess_aldr_flat_k_gmp(int* a, int n, int kmul);
struct sample_aldr_gmp_s preprocess_aldr_flat_gmp(int* a, int n);

struct AliasEntry* algo_alias_fractions(struct Fraction* distrib, int N);
struct sample_alias_fractions_s preprocess_alias_fractions(int* a, int n);

WeightedError weighted_alias_new(alias_rust_s *out, const unsigned int *weights, uint32_t n);
WeightedError weighted_alias_new_gmp(struct sample_alias_rust_gmp_s *out, const mpz_t *weights, uint32_t n);

#endif
