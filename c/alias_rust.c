#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "vector_int.h"
#include "alias_rust.h"

static unsigned int pairwise_sum(const unsigned int *weights, unsigned int n)
{
    if (n <= 32) {
        unsigned int sum = 0;
        for (unsigned int i = 0; i < n; i++)
            sum += weights[i];
        return sum;
    } else {
        unsigned int mid = n / 2;
        return pairwise_sum(weights, mid) + pairwise_sum(weights + mid, n - mid);
    }
}

// ***** Construction *****

WeightedError weighted_alias_new(
    alias_rust_s *out,
    const unsigned int *weights,
    unsigned int n
)
{

    if(n==0)
        return WEIGHTED_NO_ITEM;

    vector_init(&out->aliases);
    vector_resize_zero(&out->aliases,n);

    vector_init(&out->small);
    vector_init(&out->large);

    out->prob =
        malloc(sizeof(unsigned int)*n);

    unsigned int sum =
        pairwise_sum(weights,n);

    if(sum==0)
        return WEIGHTED_ALL_ZERO;

    out->weight_sum = sum;

    unsigned int n_unsigned = (unsigned int)n;

    for(unsigned int i=0;i<n;i++)
    {

        unsigned int p =
            weights[i] * n_unsigned;

        out->prob[i]=p;

        if(p < sum)
            vector_push(&out->small,i);
        else
            vector_push(&out->large,i);

    }

    while(
        out->small.size>0 &&
        out->large.size>0
    )
    {

        unsigned int s =
            vector_get(&out->small,
                out->small.size-1);

        out->small.size--;

        unsigned int l =
            vector_get(&out->large,
                out->large.size-1);

        out->large.size--;

        vector_set(
            &out->aliases,
            s,
            l
        );

        out->prob[l] =
            out->prob[l]
            + out->prob[s]
            - sum;

        if(out->prob[l] < sum)
            vector_push(&out->small,l);
        else
            vector_push(&out->large,l);

    }

    while(out->large.size>0)
    {

        unsigned int l =
            vector_get(
                &out->large,
                out->large.size-1);

        out->large.size--;

        out->prob[l]=sum;

    }

    while(out->small.size>0)
    {

        unsigned int s =
            vector_get(
                &out->small,
                out->small.size-1);

        out->small.size--;

        out->prob[s]=sum;

    }

    out->n=n;

    // // Taille logique (size) et taille allouee (capacity) de la structure resultat.
    // size_t out_size_by_size = sizeof(*out);

    // if (out->prob != NULL) {
    //     out_size_by_size += (size_t)out->n * sizeof(unsigned int);
    // }

    // if (out->aliases.data != NULL) {
    //     out_size_by_size += (size_t)out->aliases.size * sizeof(int);
    // }

    // // if (out->small.data != NULL) {
    // //     out_size_by_size += (size_t)out->small.size * sizeof(int);
    // // }

    // // if (out->large.data != NULL) {
    // //     out_size_by_size += (size_t)out->large.size * sizeof(int);
    // // }

    // printf("alias_rust: out size (using size) = %zu bytes\n", out_size_by_size);


    return WEIGHTED_OK;
}

// ***** Sample *****

unsigned int weighted_alias_sample(
    alias_rust_s *dist,
    unsigned int (*rng_index)(unsigned int),
    unsigned int (*rng_weight)(unsigned int)
)
{
    unsigned int i =
        rng_index(dist->n);

    unsigned int r =
        rng_weight(dist->weight_sum);

    if(r < dist->prob[i])
        return i;

    return vector_get(
        &dist->aliases,
        i
    );
}

// ***** Free *****

void weighted_alias_free(
    alias_rust_s *dist
)
{
    vector_free(&dist->aliases);
    vector_free(&dist->small);
    vector_free(&dist->large);
    free(dist->prob);
}