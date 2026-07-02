/*
  Name:     fraction.h
  Purpose:  Handling fractions as a pair of integers (numerator, denominator).
  Author:   Oriane Crouzet
  Copyright (C) 2026 Oriane Crouzet, All Rights Reserved.
*/

#ifndef FRACTION_H
#define FRACTION_H

#include <stdint.h>

struct Fraction {
    uint32_t num;
    uint32_t denom;
};

static inline struct Fraction frac_create(uint32_t num, uint32_t denom) {
    return (struct Fraction){num, denom};
}

static inline uint32_t gcd_u32(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static inline struct Fraction frac_reduce(struct Fraction f) {
    if (f.num == 0) {
        f.denom = 1;
        return f;
    }

    uint32_t g = gcd_u32(f.num, f.denom);
    f.num   /= g;
    f.denom /= g;
    return f;
}

#endif // FRACTION_H