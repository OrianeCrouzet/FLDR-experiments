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

int binary_search_interval(int *arr, int length, int x);
int binary_search_interval_nested(int *arr, int arr_denominator,
    int length, int a, int b, int denominator);

// ***** Mes ajouts *****

void cons_alias(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold);
void cons_alias2(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold);
void cons_alias3(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold);
void cons_alias4(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold);
int compare_pairs(const void *a, const void *b);
struct sample_alias_integers_s preprocess_alias_integers(int* a, int n);
uint32_t poids_total_v2(VectorInt D, unsigned int size);

struct sample_aldr_s preprocess_aldr_flat_k(int* a, int n, int kmul);
struct sample_aldr_s preprocess_aldr_flat(int* a, int n);

#endif
