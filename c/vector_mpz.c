#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#include <stdio.h>
#include "vector_mpz.h"

void vector_mpz_init(VectorMpz* v) {
    v->capacity = 16;
    v->size = 0;
    v->data = malloc(v->capacity * sizeof(mpz_t));
    if (!v->data) {
        printf("Erreur realloc dans vector_mpz_push\n");
        exit(EXIT_FAILURE);
    }
}

void vector_mpz_push(VectorMpz* vec, mpz_t val) {
    if (vec->size >= vec->capacity) {
        vec->capacity = vec->capacity ? 2 * vec->capacity : 4;
        vec->data = realloc(vec->data, vec->capacity * sizeof(mpz_t));
        if (!vec->data) {
            printf("Erreur realloc dans vector_mpz_push\n");
            exit(EXIT_FAILURE);
        }
    }

    mpz_init(vec->data[vec->size]);             // init nouvelle case
    mpz_set(vec->data[vec->size], val);         // copie de val
    vec->size++;
}


void vector_mpz_get(mpz_t dest, VectorMpz* v, unsigned int index) {
    mpz_set(dest, v->data[index]);
}

void vector_mpz_set(VectorMpz* v, unsigned int index, mpz_t val) {
    mpz_set(v->data[index], val);
}

void vector_mpz_swap(VectorMpz* v, unsigned int i, unsigned int j) {
    mpz_swap(v->data[i], v->data[j]);
}

void vector_mpz_free(VectorMpz* v) {
    for (unsigned int i = 0; i < v->size; i++) {
        mpz_clear(v->data[i]);
    }
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vector_mpz_reserve(VectorMpz *vec, const mpz_t min_capacity)
{
    // Vérifier que la capacité est représentable par size_t
    if (!mpz_fits_ulong_p(min_capacity)) {
        return;
    }

    size_t min = (size_t) mpz_get_ui(min_capacity);

    if (vec->capacity >= min)
        return;

    size_t new_capacity = (vec->capacity == 0) ? 1 : vec->capacity;

    while (new_capacity < min) {
        new_capacity *= 2;
    }

    mpz_t *temp = realloc(vec->data, new_capacity * sizeof(mpz_t));

    if (temp == NULL)
        return;

    vec->data = temp;

    // Initialiser uniquement les nouvelles cases
    for (size_t i = vec->capacity; i < new_capacity; i++) {
        mpz_init(vec->data[i]);
    }

    vec->capacity = new_capacity;
}
