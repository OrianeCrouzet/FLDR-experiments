/*
    ******************************************************
    Our version of the ALIAS using integers (without GMP).
    ******************************************************
*/

#ifndef ALIAS_INTEGERS_V2_H
#define ALIAS_INTEGERS_V2_H

#include "vector_int.h"

// Structure ALias
typedef struct sample_weighted_alias_integers_s_v2{
    VectorInt T;
    VectorInt Threshold;
    uint32_t cs;
} sample_weighted_alias_integers_s_v2;


// Déclaration des fonctions utilitaires
uint32_t poids_total_v2(VectorInt D, unsigned int size);
void cons_alias_v2(unsigned int n, VectorInt* D, uint32_t cs, int virtual_obj, VectorInt* T, VectorInt* Threshold);
unsigned int bit_length_v2(unsigned int x);

// Benchmark
void free_sample_alias_integers_s(sample_weighted_alias_integers_s_v2 x);
sample_weighted_alias_integers_s_v2 preprocess_weighted_alias_integers_v2(int* a, int n);
uint32_t sample_weighted_alias_integers_v2(sample_weighted_alias_integers_s_v2 *x);
int bytes_sample_weighted_alias_integers_v2(sample_weighted_alias_integers_s_v2 *x);

#endif