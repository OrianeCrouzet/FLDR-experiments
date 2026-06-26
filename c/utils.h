/*
  Name:     utils.h
  Purpose:  Utilities for searching.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "vector_int.h"
#include "sstructs.h"

// ***** Mes ajouts *****

void cons_alias5(unsigned int n, VectorInt* D, uint32_t cs, VectorInt* T, VectorInt* Threshold);

struct sample_alias_integers_s preprocess_alias_integers_old(int* a, int n);


//uint32_t poids_total_v2(VectorInt D, unsigned int size);

struct sample_aldr_s preprocess_aldr_flat_k(int* a, int n, int kmul);
struct sample_aldr_s preprocess_aldr_flat(int* a, int n);

#endif
