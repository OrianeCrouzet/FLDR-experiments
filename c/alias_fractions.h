/*
    This file contains one version of the Alias algorithm with rationnal numbers.
    *************************************
    Classic Alias using rational numbers.
    *************************************
*/

#ifndef ALIAS_FRACTIONS_H
#define ALIAS_FRACTIONS_H

#include <gmp.h>
#include "fraction.h"
#include "sstructs.h"

// Déclarations des fonctions utilitaires
struct AliasEntry* algo_alias_fractions(struct Fraction* distrib, int N);
void free_sample_alias_fractions(struct sample_alias_fractions_s x);
struct sample_alias_fractions_s preprocess_alias_fractions(int* a, int n);

#endif