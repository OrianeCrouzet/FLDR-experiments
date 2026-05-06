#include <stdlib.h>
#include <stdio.h>
#include "vector_int.h"

void vector_init(VectorInt* v) {
    v->capacity = 16;
    v->size = 0;
    v->data = malloc(v->capacity * sizeof(int));
    // if (!v->data) {
    //     fprintf(stderr, "Erreur d’allocation mémoire\n");
    //     exit(EXIT_FAILURE);
    // }
}

void vector_push(VectorInt* v, int value) {
    if (v->size >= v->capacity) {
        v->capacity *= 2;
        v->data = realloc(v->data, v->capacity * sizeof(int));
        // if (!v->data) {
        //     fprintf(stderr, "Erreur d’allocation mémoire\n");
        //     exit(EXIT_FAILURE);
        // }
        
    }
    v->data[v->size++] = value;
}

int vector_get(VectorInt* v, unsigned int index) {
    // if (index >= v->size) {
    //     printf("Vector index out of bounds\n");
    //     exit(EXIT_FAILURE);
    // }
    return v->data[index];
}

void vector_set(VectorInt* v, unsigned int index, unsigned int val) {
    v->data[index] = val;
}

void vector_free(VectorInt* v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vector_resize_zero(VectorInt* vec, unsigned int new_size) {
    if (new_size > vec->capacity) {
        vec->capacity = new_size * 2; // un peu de marge pour futurs push
        vec->data = realloc(vec->data, vec->capacity * sizeof(int));
        // if (!vec->data) {
        //     printf("Erreur realloc dans vector_resize_zero\n");
        //     exit(EXIT_FAILURE);
        // }
    }

    for (unsigned int i = vec->size; i < new_size; i++) {
        vec->data[i] = 0;
    }

    vec->size = new_size;
}

void vector_swap(VectorInt* v, unsigned int i, unsigned int j) {
    // if (i >= v->size || j >= v->size) {
    //     printf("Swap error %u %u size %u\n", i, j, v->size);
    //     exit(1);
    // }
    int temp = v->data[i];
    v->data[i] = v->data[j];
    v->data[j] = temp;
}

// Pré-allouer une capacité minimum
void vector_reserve(VectorInt* vec, unsigned int min_capacity) {
    if (vec->capacity < min_capacity) {
        unsigned int new_capacity = vec->capacity;
        while (new_capacity < min_capacity) new_capacity *= 2;
        
        int* temp = realloc(vec->data, new_capacity * sizeof(int));
        // if (!temp) {
        //     fprintf(stderr, "Erreur allocation mémoire (reserve)\n");
        //     free(vec->data);
        //     exit(EXIT_FAILURE);
        // }
        
        vec->data = temp;
        vec->capacity = new_capacity;
    }
}

