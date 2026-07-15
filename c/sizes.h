/*
  Name:     sizes.h
  Purpose:  Functions to measure sizes of data structures for sampling algorithms.
  Author:   Oriane Crouzet
  Copyright (C) 2026 Oriane Crouzet
*/

#ifndef SIZES_H
#define SIZES_H

#include <stddef.h>
#include "sstructs.h"

size_t get_size_sample_alias_integers_s(struct sample_alias_integers_s *s);
size_t get_size_sample_alias_rust_s(struct sample_alias_rust_s *s);
size_t get_size_sample_alias_fractions_s(struct sample_alias_fractions_s *s);
size_t get_size_sample_aldr_s(struct sample_aldr_s *s);
size_t get_size_sample_ky_encoding_s(struct sample_ky_encoding_s *s);
size_t get_size_sample_alias_exact_s(struct sample_alias_exact_s *s);

#endif

