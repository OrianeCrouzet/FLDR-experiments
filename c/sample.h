/*
  Name:     sample.h
  Purpose:  Exact samplers for discrete probability distributions.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef SAMPLE_H
#define SAMPLE_H

#include "sstructs.h"

extern unsigned long REJET;

// *************************** UTILS ***************************

int sample_fdr(struct sample_fdr_s *x);
int sample_inversion_bernoulli(struct sample_inversion_bernoulli_s *x);
int sample_rejection_encoding(struct sample_ky_encoding_s *x);


// *************************** EXACT SAMPLERS ***************************

int sample_alias_gsl(struct sample_alias_gsl_s *x);
int sample_alias_exact(struct sample_alias_exact_s *x);
int sample_ky_encoding(struct sample_ky_encoding_s *x);
uint32_t sample_alias_integers_old(struct sample_alias_integers_s *x);
uint32_t sample_alias_integers(struct sample_alias_integers_s *x);

uint32_t sample_gmp_alias_integers(struct sample_gmp_alias_integers_s* x);

int sample_aldr(struct sample_aldr_s *x);
uint32_t sample_alias_rust(struct sample_alias_rust_s *x);
uint32_t sample_alias_fractions(struct sample_alias_fractions_s *x);

#endif
