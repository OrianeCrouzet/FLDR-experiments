/*
  Name:     flip.c
  Purpose:  Generating a sequence of pseudo-random bits.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdlib.h>
#include <sys/random.h>

#include "flip.h"

unsigned long NUM_RNG_CALLS = 0;

// RAND_MAX is 2**31-1 signed, so bits are 0,...29 with 30th
static int k = 30;
static int flip_word = 0;
static int flip_pos = 0;

// assume RAND_MAX is a Mersenne number
uint32_t flip_k = 32 - __builtin_clz(RAND_MAX);

int flip(void){
    if (flip_pos == 0) {
        NUM_RNG_CALLS++;
        flip_word = rand();
        flip_pos = k;
    }
    --flip_pos;
    return (flip_word >> flip_pos) & 1;
}

int randint(int k) {
    int n = 0;

    for (int i = 0; i < k; i++) {
        int b = flip();
        n <<= 1;
        n += b;
    }

    return n;
}

void check_refill(void) {
    if (flip_pos == 0) {
        ++NUM_RNG_CALLS;
        // we set flip_k to 32 to use sysrandom
        if (flip_k == 32) {
            flip_word = arc4random();
            // getrandom(&flip_word, sizeof(flip_word), 0);  --> not available on Mac
        } else {
            flip_word = rand();
        }
        flip_pos = flip_k;
    }
}

uint32_t flip_n(uint32_t n) {
    check_refill();
    uint32_t num_bits_extract = min(n, flip_pos);
    flip_pos -= num_bits_extract;
    uint32_t b = (flip_word >> flip_pos) & (UINT32_MAX >> (32 - num_bits_extract));
    return (num_bits_extract == n) ? b :
        (b << (n - num_bits_extract)) | flip_n(n - num_bits_extract);
}

uint32_t uniform(uint32_t n) {
    uint32_t num_bits_presample = 32 - __builtin_clz(n - 1);
    uint32_t bound = 1 << num_bits_presample;
    uint32_t x = flip_n(num_bits_presample);
    for (;;) {
        if (bound >= n) {
            if (x < n) { return x; }
            bound -= n;
            x -= n;
        }
        bound <<= 1;
        x = (x << 1) | flip();
    }
}

uint32_t bernoulli(uint32_t numer, uint32_t denom) {
    if (numer == 0) {
        return 0;
    }
    if (numer == denom) {
        return 1;
    }
    uint32_t y;

    for (;;) {
        numer <<= 1;
        if (numer == denom) {
            return flip();
        }
        if (y = numer > denom) {
            numer -= denom;
        }
        if (flip()) {
            return y;
        }
    }
}
