#ifndef VECTOR_MPZ_H
#define VECTOR_MPZ_H

#include <gmp.h>

typedef struct {
    mpz_t* data;
    unsigned int size;
    unsigned int capacity;
} VectorMpz;

void vector_mpz_init(VectorMpz* v);
void vector_mpz_push(VectorMpz* v, const mpz_t val);
void vector_mpz_get(mpz_t dest, const VectorMpz* v, unsigned int index);
void vector_mpz_set(VectorMpz* v, unsigned int index, const mpz_t val);
void vector_mpz_swap(VectorMpz* v, unsigned int i, unsigned int j);
void vector_mpz_free(VectorMpz* v);
void vector_mpz_reserve(VectorMpz *vec, const mpz_t min_capacity);


#endif