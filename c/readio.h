/*
  Name:     readio.h
  Purpose:  Loading sampling data structures from disk..
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef READIO_H
#define READIO_H

#include <stdio.h>
#include "sstructs.h"

// *************************** UTILS - READ AND FREE ***************************

struct matrix_s load_matrix(FILE *fp);
struct array_s load_array(FILE *fp);
struct sample_fdr_s read_sample_fdr(char *fname);
struct sample_inversion_bernoulli_s read_sample_inversion_bernoulli(char *fname);;

void free_matrix_s(struct matrix_s x);
void free_array_s(struct array_s x);
void free_sample_fdr_s(struct sample_fdr_s x);
void free_sample_inversion_bernoulli_s(struct sample_inversion_bernoulli_s x);


// ***************************** SAMPLERS - READ *****************************

struct sample_ky_encoding_s read_sample_ky_encoding(char *fname);
struct sample_alias_gsl_s read_sample_alias_gsl(char *fname);
struct sample_alias_exact_s read_sample_alias_exact(char *fname);
struct sample_alias_integers_s read_sample_alias_integers_old(char *fname);
struct sample_aldr_s read_sample_aldr(char *fname);
struct sample_alias_rust_s read_sample_alias_rust(char *fname);
struct sample_alias_rust_gmp_s read_sample_alias_rust_gmp(char *fname);
struct sample_alias_fractions_s read_sample_alias_fractions(char *fname);

struct sample_gmp_alias_integers_s read_sample_alias_integers_gmp(char *fname);
struct sample_aldr_gmp_s read_sample_aldr_gmp(char *fname);
struct sample_ky_encoding_gmp_s read_sample_ky_encoding_gmp(char *fname);
struct sample_alias_fractions_gmp_s read_sample_alias_fractions_gmp(char *fname);

// ***************************** SAMPLERS - FREE *****************************

void free_sample_ky_encoding_s(struct sample_ky_encoding_s x);
void free_sample_alias_gsl_s(struct sample_alias_gsl_s x);
void free_sample_alias_exact_s(struct sample_alias_exact_s x);
void free_sample_alias_integers_s_old(struct sample_alias_integers_s x);
void free_sample_aldr_s(struct sample_aldr_s x);
void free_sample_alias_rust_s(struct sample_alias_rust_s x);
void free_sample_alias_rust_gmp_s(struct sample_alias_rust_gmp_s x);
void free_sample_alias_fractions_s(struct sample_alias_fractions_s x);

void free_sample_ky_encoding_gmp_s(struct sample_ky_encoding_gmp_s x);
void free_sample_gmp_alias_integers(struct sample_gmp_alias_integers_s x);
void free_sample_aldr_gmp_s (struct sample_aldr_gmp_s x);
void free_sample_alias_fractions_gmp_s(struct sample_alias_fractions_gmp_s x);

#endif
