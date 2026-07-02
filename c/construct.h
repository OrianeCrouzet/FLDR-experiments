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
#include "vector_int.h"
#include "sstructs.h"
#include "fraction.h"


void cons_alias5(unsigned int n, VectorInt* D, uint32_t cs, VectorInt* T, VectorInt* Threshold);
struct sample_alias_integers_s preprocess_alias_integers_old(int* a, int n);

struct sample_aldr_s preprocess_aldr_flat_k(int* a, int n, int kmul);
struct sample_aldr_s preprocess_aldr_flat(int* a, int n);

struct AliasEntry* algo_alias_fractions(struct Fraction* distrib, int N);
struct sample_alias_fractions_s preprocess_alias_fractions(int* a, int n);

WeightedError weighted_alias_new(alias_rust_s *out, const unsigned int *weights, uint32_t n);

#endif
