/*
  Name:     sizes.c
  Purpose:  Functions to measure sizes of data structures for sampling algorithms.
  Author:   Oriane Crouzet
  Copyright (C) 2026 Oriane Crouzet, All Rights Reserved.
*/

#include "sizes.h"

size_t get_size_sample_alias_integers_s(struct sample_alias_integers_s *s) {
    size_t sampler_size = sizeof(*s);

    sampler_size += sizeof(s->T);
    if (s->T.data != NULL) {
        sampler_size += (size_t)s->T.size * sizeof(s->T.data[0]);
    }
    sampler_size += sizeof(s->Threshold);
    if (s->Threshold.data != NULL) {
        sampler_size += (size_t)s->Threshold.size * sizeof(s->Threshold.data[0]);
    }
    // sampler_size += sizeof(s->cs);
    // sampler_size += sizeof(s->virtual_obj);

    return sampler_size;
}

size_t get_size_sample_alias_rust_s(struct sample_alias_rust_s *s) {
    size_t size = sizeof(*s);
    if (s->aliases.data != NULL) {
        size += (size_t)s->aliases.size * sizeof(s->aliases.data[0]);
    }
    if (s->small.data != NULL) {
        size += (size_t)s->small.size * sizeof(s->small.data[0]);
    }
    if (s->large.data != NULL) {
        size += (size_t)s->large.size * sizeof(s->large.data[0]);
    }
    if (s->prob != NULL) {
        size += (size_t)s->n * sizeof(s->prob[0]);
    }

    return size;
}

size_t get_size_sample_alias_fractions_s(struct sample_alias_fractions_s *s) {
    size_t size = sizeof(*s);
    if (s->table != NULL) {
        size += (size_t)s->taille * sizeof(struct AliasEntry);
    }

    return size;
}

size_t get_size_sample_ky_encoding_s(struct sample_ky_encoding_s *s) {
    size_t size = sizeof(*s);
    if (s->encoding.a != NULL) {
        size += (size_t)s->encoding.length * sizeof(s->encoding.a[0]);
    }
    return size;
}

size_t get_size_sample_alias_exact_s(struct sample_alias_exact_s *s) {
    size_t size = sizeof(*s);
    if (s->ratios != NULL) {
        size += (size_t)s->n * sizeof(s->ratios[0]);
    }
    if (s->j.a != NULL) {
        size += (size_t)s->j.length * sizeof(s->j.a[0]);
    }
    return size;
}

size_t get_size_sample_aldr_s(struct sample_aldr_s *s) {
    size_t size = sizeof(*s);
    if (s->breadths != NULL) {
        size += (size_t)s->length_breadths * sizeof(s->breadths[0]);
    }
    if (s->leaves_flat != NULL) {
        size += (size_t)s->length_leaves_flat * sizeof(s->leaves_flat[0]);
    }

    return size;
}

