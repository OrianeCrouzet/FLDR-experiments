#ifndef FRACTION_H
#define FRACTION_H

#include <stdint.h>

struct Fraction {
    uint32_t num;
    uint32_t denom;
};

static inline struct Fraction frac_zero() {
    return (struct Fraction){0, 1};
}

static inline int frac_eq(struct Fraction a, struct Fraction b) {
    return (uint64_t)a.num * b.denom == (uint64_t)b.num * a.denom;
}

static inline int frac_le(struct Fraction a, struct Fraction b) {
    return (uint64_t)a.num * b.denom <= (uint64_t)b.num * a.denom;
}

static inline struct Fraction frac_sub(struct Fraction a, struct Fraction b) {
    // a - b
    uint64_t num = (uint64_t)a.num * b.denom - (uint64_t)b.num * a.denom;
    uint64_t den = (uint64_t)a.denom * b.denom;
    return (struct Fraction){(uint32_t)num, (uint32_t)den};
}

static inline struct Fraction frac_copy(struct Fraction a) {
    return a;
}

#endif // FRACTION_H