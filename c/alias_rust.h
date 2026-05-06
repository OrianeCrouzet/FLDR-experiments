#ifndef ALIAS_RUST_H
#define ALIAS_RUST_H

#include <stdint.h>
#include "vector_int.h"

typedef struct {
    VectorInt aliases;
    //double *prob;
    unsigned int *prob;
    VectorInt small;
    VectorInt large;
    unsigned int n;
    //double weight_sum;
    unsigned int weight_sum;
} alias_rust_s;

typedef enum {
    WEIGHTED_OK = 0,
    WEIGHTED_NO_ITEM,
    WEIGHTED_TOO_MANY,
    WEIGHTED_INVALID_WEIGHT,
    WEIGHTED_ALL_ZERO
} WeightedError;

WeightedError weighted_alias_new(
    alias_rust_s *out,
    //const double *weights,
    const unsigned int *weights,
    uint32_t n
);

unsigned int weighted_alias_sample(
    alias_rust_s *dist,
    unsigned int (*rng_index)(unsigned int),
    //double (*rng_weight)(double max)
    unsigned int (*rng_weight)(unsigned int max)
);

void weighted_alias_free(alias_rust_s *dist);

#endif