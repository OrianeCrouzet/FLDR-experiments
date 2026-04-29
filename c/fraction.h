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

static inline struct Fraction frac_create(uint32_t num, uint32_t denom) {
    return (struct Fraction){num, denom};
}

static inline int frac_eq(struct Fraction a, struct Fraction b) {
    return (uint64_t)a.num * b.denom == (uint64_t)b.num * a.denom;
}

static inline int frac_lt(struct Fraction a, struct Fraction b) {
    return (uint64_t)a.num * b.denom < (uint64_t)b.num * a.denom;
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

static inline struct Fraction frac_add(struct Fraction a, struct Fraction b) {
    uint64_t num = (uint64_t)a.num * b.denom + (uint64_t)b.num * a.denom;
    uint64_t den = (uint64_t)a.denom * b.denom;
    return (struct Fraction){(uint32_t)num, (uint32_t)den};
}

static inline struct Fraction frac_div(struct Fraction a, struct Fraction b) {
    // a / b = (a.num / a.denom) / (b.num / b.denom)
    uint64_t num = (uint64_t)a.num * b.denom;
    uint64_t den = (uint64_t)a.denom * b.num;

    return (struct Fraction){(uint32_t)num, (uint32_t)den};
}

static inline struct Fraction frac_mul(struct Fraction a, struct Fraction b) {
    uint64_t num = (uint64_t)a.num * b.num;
    uint64_t den = (uint64_t)a.denom * b.denom;
    return (struct Fraction){(uint32_t)num, (uint32_t)den};
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