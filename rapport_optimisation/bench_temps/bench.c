/*
  Name:     bench.c
  Purpose:  Micro-benchmark of the uniform and Bernoulli building blocks of
            alias.integers_opt, against faster alternatives:
              - Lemire's multiply-shift uniform (64 bits per draw);
              - a "recycling" uniform and Bernoulli that keep a uniform state
                (U uniform on [0, M)) from one draw to the next;
              - an inexact 64-bit alias, used as a time floor.
            Every exact variant reads its bits from the stream of flip_opt.h.

  Usage:    bench.out FILE.alias.integers_opt N [hist PREFIX | seq VARIANT OUT]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alias_integers_opt.h"
#include "flip_opt.h"

unsigned long NUM_RNG_CALLS = 0;
unsigned long REJET = 0;

static struct sample_alias_integers_opt_s S;
static uint64_t *P64;        // floor(T_i / cs * 2^64), cells with an alias
static uint32_t *thr32;      // P64 >> 32, all cells (inexact baseline)
static uint32_t *alias2;     // alias, or primary for cells without alias

static inline uint64_t bits_used(void) {
    return 64 * fo_q.c_head + fo_q.c_off;
}

static inline uint64_t umulh(uint64_t a, uint64_t b) {
    return (uint64_t)(((unsigned __int128)a * b) >> 64);
}

// Cheap non-cryptographic index, only for load/Bernoulli-only baselines.
static uint64_t xs = 88172645463325252ull;
static inline uint32_t xs_index(uint32_t n) {
    xs ^= xs << 13; xs ^= xs >> 7; xs ^= xs << 17;
    return (uint32_t)umulh(xs, n);
}


// ********************************* Uniforms *********************************

// Lemire (2019), exact: one 64-bit word, rejection with probability < n / 2^64.
static inline uint32_t lemire_uniform(uint32_t n) {
    uint64_t x = fo_bits(64);
    unsigned __int128 m = (unsigned __int128)x * n;
    uint64_t l = (uint64_t)m;
    if (__builtin_expect(l < n, 0)) {
        uint64_t t = -(uint64_t)n % n;
        while (l < t) {
            x = fo_bits(64);
            m = (unsigned __int128)x * n;
            l = (uint64_t)m;
        }
    }
    return (uint32_t)(m >> 64);
}

// Lemire on a 32-bit word (n < 2^32): rejection with probability < n / 2^32.
static inline uint32_t lemire32_uniform(uint32_t n) {
    uint64_t m = (uint64_t)fo_bits(32) * n;
    uint32_t l = (uint32_t)m;
    if (__builtin_expect(l < n, 0)) {
        uint32_t t = -n % n;
        while (l < t) {
            m = (uint64_t)fo_bits(32) * n;
            l = (uint32_t)m;
        }
    }
    return (uint32_t)(m >> 32);
}

// Recycling state: U is uniform on [0, M), independent of every past output.
struct rstate { uint64_t U, M; };
static struct rstate RS = {0, 1}, RB = {0, 1};

static inline void rc_refill(struct rstate *s) {
    if (s->M < (1ull << 32)) {
        s->U = (s->U << 32) | fo_bits(32);
        s->M <<= 32;
    }
}

// Uniform on [0, n), n < 2^32. M = qn + r: if U < qn, (U mod n, U div n) is
// uniform on [0, n) x [0, q); otherwise U - qn is uniform on [0, r).
static inline uint32_t rc_uniform(struct rstate *s, uint32_t n) {
    for (;;) {
        rc_refill(s);
        uint64_t q = s->M / n;
        uint64_t lim = q * n;
        if (__builtin_expect(s->U < lim, 1)) {
            uint64_t d = s->U / n;
            uint32_t x = (uint32_t)(s->U - d * n);
            s->U = d;
            s->M = q;
            return x;
        }
        s->U -= lim;
        s->M -= lim;
    }
}


// Same, with divisions by n replaced by a multiplication by inv = floor(2^64 / n)
// (the estimate floor(x inv / 2^64) is q or q - 1, fixed by one comparison).
static uint64_t INV_Q;

static inline uint64_t div_n(uint64_t x, uint32_t n, uint64_t *rem) {
    uint64_t d = umulh(x, INV_Q);
    uint64_t r = x - d * n;
    if (r >= n) {
        d++;
        r -= n;
    }
    *rem = r;
    return d;
}

static inline uint32_t rc_uniform_mul(struct rstate *s, uint32_t n) {
    for (;;) {
        rc_refill(s);
        uint64_t r;
        uint64_t q = div_n(s->M, n, &r);
        uint64_t lim = s->M - r;
        if (__builtin_expect(s->U < lim, 1)) {
            uint64_t x;
            s->U = div_n(s->U, n, &x);
            s->M = q;
            return (uint32_t)x;
        }
        s->U -= lim;
        s->M = r;
    }
}


// ******************************** Bernoullis ********************************

// Lumbroso's Bernoulli(num/den), 0 < num < den, fresh bits.
static uint32_t gmp_bernoulli(mpz_t num, const mpz_t den) {
    for (;;) {
        mpz_mul_2exp(num, num, 1);
        int c = mpz_cmp(num, den);
        if (c == 0) {
            return fo_bit();
        }
        uint32_t y = (c > 0);
        if (y) {
            mpz_sub(num, num, den);
        }
        if (fo_bit()) {
            return y;
        }
    }
}

// Exact slow path of rc_bernoulli: s->U in {lo, lo+1}. With V uniform on
// [s->U/s->M, (s->U+1)/s->M), Pr(V < p) = clamp(p s->M - s->U, 0, 1) = clamp((t s->M - s->U cs) / cs).
static __attribute__((noinline)) uint32_t rc_bernoulli_slow(struct rstate *s, uint32_t i) {
    uint32_t start = S.cells[i].thr;
    uint32_t end = (i + 1 < S.m) ? S.cells[i + 1].thr : S.thr_size;
    mpz_t t, a, b;
    mpz_inits(t, a, b, NULL);
    mpz_import(t, end - start, -1, 1, 0, 0, S.Threshold + start);
    mpz_set_ui(a, s->M >> 32); mpz_mul_2exp(a, a, 32); mpz_add_ui(a, a, s->M & 0xffffffffu);
    mpz_mul(a, a, t);
    mpz_set_ui(b, s->U >> 32); mpz_mul_2exp(b, b, 32); mpz_add_ui(b, b, s->U & 0xffffffffu);
    mpz_mul(b, b, S.cs);
    mpz_sub(a, a, b);
    uint32_t y;
    if (mpz_sgn(a) <= 0) {
        y = 0;
    } else if (mpz_cmp(a, S.cs) >= 0) {
        y = 1;
    } else {
        y = gmp_bernoulli(a, S.cs);
    }
    mpz_clears(t, a, b, NULL);
    s->U = 0;      // the state is dropped (allowed: it only wastes bits)
    s->M = 1;
    return y;
}

// Bernoulli(T_i / cs) from the recycling state. lo = floor(P64 s->M / 2^64):
// s->U < lo implies V < p, s->U >= lo + 2 implies V >= p, else (proba <= 2^-31) slow path.
static inline uint32_t rc_bernoulli(struct rstate *s, uint32_t i) {
    rc_refill(s);
    uint64_t lo = umulh(P64[i], s->M);
    if (s->U < lo) {
        s->M = lo;
        return 1;
    }
    if (__builtin_expect(s->U >= lo + 2, 1)) {
        s->U -= lo + 2;
        s->M -= lo + 2;
        return 0;
    }
    return rc_bernoulli_slow(s, i);
}

// Exact Bernoulli by comparing 31 fresh bits with the 31 digits at once (31 bits per call).
static inline uint32_t cmp31_bernoulli(const struct alias_opt_cell *c, uint32_t i) {
    uint32_t B = (uint32_t)fo_bits(31);
    uint32_t D = c->bits >> 1;
    if (B != D) {
        return B < D;
    }
    // B = D: V lies in [D/2^31, (D+1)/2^31), continue lazily (rare).
    mpz_t t, num;
    mpz_inits(t, num, NULL);
    uint32_t start = c->thr;
    uint32_t end = (i + 1 < S.m) ? S.cells[i + 1].thr : S.thr_size;
    mpz_import(t, end - start, -1, 1, 0, 0, S.Threshold + start);
    mpz_mul_2exp(num, t, 31);
    mpz_fdiv_r(num, num, S.cs);
    uint32_t y = (mpz_sgn(num) == 0) ? 0 : gmp_bernoulli(num, S.cs);
    mpz_clears(t, num, NULL);
    return y;
}


// ********************************* Samplers *********************************

#define ALIAS_LOOP(UNIF, BERN)                                         \
    uint32_t result;                                                   \
    do {                                                               \
        uint32_t i = UNIF;                                             \
        const struct alias_opt_cell *c = &S.cells[i];                  \
        uint32_t b = 1;                                                \
        if (c->alias != ALIAS_OPT_NONE) {                              \
            b = BERN;                                                  \
        }                                                              \
        result = b ? c->primary : c->alias;                            \
    } while (result == S.virtual_obj);                                 \
    return result;

static __attribute__((noinline)) uint32_t ctz_tail(uint32_t i) {
    uint32_t end = (i + 1 < S.m) ? S.cells[i + 1].thr : S.thr_size;
    mpz_t t;
    mpz_init(t);
    mpz_import(t, end - S.cells[i].thr, -1, 1, 0, 0, S.Threshold + S.cells[i].thr);
    uint32_t y = fo_bernoulli_tail(t, S.cs);
    mpz_clear(t);
    return y;
}

// Same exact Bernoulli as alias.integers_opt (31 digits, ctz, GMP tail).
static inline uint32_t ctz_bern(const struct alias_opt_cell *c, uint32_t i) {
    uint32_t b = fo_bernoulli_digits(c->bits);
    if (__builtin_expect(b == 2, 0)) {
        b = ctz_tail(i);
    }
    return b;
}

static uint32_t s_lemire_ctz(void) { ALIAS_LOOP(lemire_uniform(S.q), ctz_bern(c, i)) }
static uint32_t s_lemire32_ctz(void) { ALIAS_LOOP(lemire32_uniform(S.q), ctz_bern(c, i)) }
static uint32_t s_lemire32_cmp31(void) { ALIAS_LOOP(lemire32_uniform(S.q), cmp31_bernoulli(c, i)) }
static uint32_t s_recyc_ctz(void)  { ALIAS_LOOP(rc_uniform(&RS, S.q), ctz_bern(c, i)) }
static uint32_t s_recyc_recyc(void){ ALIAS_LOOP(rc_uniform(&RS, S.q), rc_bernoulli(&RS, i)) }
static uint32_t s_recyc_recyc2(void){ ALIAS_LOOP(rc_uniform(&RS, S.q), rc_bernoulli(&RB, i)) }
static uint32_t s_recmul_recyc2(void){ ALIAS_LOOP(rc_uniform_mul(&RS, S.q), rc_bernoulli(&RB, i)) }
static uint32_t s_recmul_ctz(void){ ALIAS_LOOP(rc_uniform_mul(&RS, S.q), ctz_bern(c, i)) }
static uint32_t s_fdr_recyc(void)  { ALIAS_LOOP(fo_uniform(S.q, S.q_nbits), rc_bernoulli(&RB, i)) }
static uint32_t s_fdr_cmp31(void)  { ALIAS_LOOP(fo_uniform(S.q, S.q_nbits), cmp31_bernoulli(c, i)) }

// Inexact floor: one 64-bit word, 32 bits for the cell, 32 bits for the threshold.
static uint32_t s_floor(void) {
    uint32_t result;
    do {
        uint64_t x = fo_bits(64);
        uint32_t i = (uint32_t)(((x >> 32) * S.q) >> 32);
        result = ((uint32_t)x < thr32[i]) ? S.cells[i].primary : alias2[i];
    } while (result == S.virtual_obj);
    return result;
}

// Exact counterpart of s_floor: one 64-bit word, Lemire on the high 32 bits for the
// cell, the low 32 bits compared with the first 32 digits of T_i / cs (exact tail if equal).
static __attribute__((noinline)) uint32_t word_tail(uint32_t i) {
    uint32_t end = (i + 1 < S.m) ? S.cells[i + 1].thr : S.thr_size;
    mpz_t t, num;
    mpz_inits(t, num, NULL);
    mpz_import(t, end - S.cells[i].thr, -1, 1, 0, 0, S.Threshold + S.cells[i].thr);
    mpz_mul_2exp(num, t, 32);
    mpz_fdiv_r(num, num, S.cs);
    uint32_t y = (mpz_sgn(num) == 0) ? 0 : gmp_bernoulli(num, S.cs);
    mpz_clears(t, num, NULL);
    return y;
}

static uint32_t s_word(void) {
    uint32_t result;
    do {
        uint64_t x = fo_bits(64);
        uint64_t m = (x >> 32) * S.q;
        uint32_t i = (uint32_t)(m >> 32);
        if (__builtin_expect((uint32_t)m < S.q, 0)) {
            uint32_t t = -S.q % S.q;
            while ((uint32_t)m < t) {
                m = (uint64_t)fo_bits(32) * S.q;
            }
            i = (uint32_t)(m >> 32);
        }
        const struct alias_opt_cell *c = &S.cells[i];
        uint32_t b = 1;
        if (c->alias != ALIAS_OPT_NONE) {
            uint32_t B = (uint32_t)x;
            b = (B != thr32[i]) ? (B < thr32[i]) : word_tail(i);
        }
        result = b ? c->primary : c->alias;
    } while (result == S.virtual_obj);
    return result;
}

static uint32_t s_opt(void)       { return sample_alias_integers_opt(&S); }
static uint32_t u_fdr(void)       { return fo_uniform(S.q, S.q_nbits); }
static uint32_t u_recmul(void)     { return rc_uniform_mul(&RS, S.q); }
static uint32_t u_lemire32(void)  { return lemire32_uniform(S.q); }
static uint32_t u_lemire(void)    { return lemire_uniform(S.q); }
static uint32_t u_recyc(void)     { return rc_uniform(&RS, S.q); }
static uint32_t stream18(void)    { return (uint32_t)fo_bits(18); }
static uint32_t load_only(void)   { return S.cells[xs_index(S.q)].primary; }
static uint32_t b_ctz(void) {
    uint32_t i = xs_index(S.m);
    return fo_bernoulli_digits(S.cells[i].bits);
}
static uint32_t b_recyc(void) {
    uint32_t i = xs_index(S.m);
    return rc_bernoulli(&RB, i);
}
static uint32_t b_cmp31(void) {
    uint32_t i = xs_index(S.m);
    return cmp31_bernoulli(&S.cells[i], i);
}

struct variant { const char *name; uint32_t (*f)(void); int exact_sampler; };

static const struct variant VARIANTS[] = {
    {"opt (FDR + Bernoulli ctz)", s_opt, 1},
    {"Lemire + Bernoulli ctz", s_lemire_ctz, 1},
    {"Lemire32 + Bernoulli ctz", s_lemire32_ctz, 1},
    {"Lemire32 + Bernoulli cmp31", s_lemire32_cmp31, 1},
    {"recyclage + Bernoulli ctz", s_recyc_ctz, 1},
    {"FDR + Bernoulli recyclage", s_fdr_recyc, 1},
    {"recyclage + recyclage (1 etat)", s_recyc_recyc, 1},
    {"recyclage + recyclage (2 etats)", s_recyc_recyc2, 1},
    {"recyc. mult. + Bernoulli ctz", s_recmul_ctz, 1},
    {"recyc. mult. + recyc. (2 etats)", s_recmul_recyc2, 1},
    {"FDR + Bernoulli cmp31", s_fdr_cmp31, 1},
    {"mot 64 bits exact", s_word, 1},
    {"plancher inexact (64 bits)", s_floor, 0},
    {"uniforme FDR seule", u_fdr, 0},
    {"uniforme Lemire seule", u_lemire, 0},
    {"uniforme Lemire32 seule", u_lemire32, 0},
    {"uniforme recyclage seule", u_recyc, 0},
    {"uniforme recyc. mult. seule", u_recmul, 0},
    {"Bernoulli ctz seul", b_ctz, 0},
    {"Bernoulli recyclage seul", b_recyc, 0},
    {"Bernoulli cmp31 seul", b_cmp31, 0},
    {"lecture de 18 bits", stream18, 0},
    {"lecture d'une cellule", load_only, 0},
};

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static void prepare(void) {
    P64 = calloc(S.q, sizeof(uint64_t));
    thr32 = malloc(S.q * sizeof(uint32_t));
    alias2 = malloc(S.q * sizeof(uint32_t));
    mpz_t t, x;
    mpz_inits(t, x, NULL);
    for (uint32_t i = 0; i < S.q; i++) {
        const struct alias_opt_cell *c = &S.cells[i];
        if (c->alias == ALIAS_OPT_NONE) {
            thr32[i] = UINT32_MAX;
            alias2[i] = c->primary;
            continue;
        }
        uint32_t end = (i + 1 < S.m) ? S.cells[i + 1].thr : S.thr_size;
        mpz_import(t, end - c->thr, -1, 1, 0, 0, S.Threshold + c->thr);
        mpz_mul_2exp(x, t, 64);
        mpz_fdiv_q(x, x, S.cs);
        P64[i] = ((uint64_t)mpz_getlimbn(x, 0));
        thr32[i] = (uint32_t)(P64[i] >> 32);
        alias2[i] = c->alias;
    }
    mpz_clears(t, x, NULL);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s FILE.alias.integers_opt N [hist PREFIX | seq VARIANT OUT]\n", argv[0]);
        return EXIT_FAILURE;
    }
    long N = atol(argv[2]);
    S = read_sample_alias_integers_opt(argv[1]);
    prepare();
    INV_Q = UINT64_MAX / S.q;
    fo_start();

    if (argc >= 5 && strcmp(argv[3], "hist") == 0) {
        // Histograms of the exact samplers, for the chi-square test.
        uint32_t n_obj = S.q + 2;
        uint64_t *h = malloc(n_obj * sizeof(uint64_t));
        for (size_t v = 0; v < sizeof(VARIANTS) / sizeof(VARIANTS[0]); v++) {
            if (!VARIANTS[v].exact_sampler) {
                continue;
            }
            memset(h, 0, n_obj * sizeof(uint64_t));
            for (long k = 0; k < N; k++) {
                h[VARIANTS[v].f()]++;
            }
            char path[1024];
            snprintf(path, sizeof(path), "%s.%zu", argv[4], v);
            FILE *fp = fopen(path, "w");
            fprintf(fp, "%s\n", VARIANTS[v].name);
            for (uint32_t k = 0; k < n_obj; k++) {
                fprintf(fp, "%llu\n", (unsigned long long)h[k]);
            }
            fclose(fp);
        }
        fo_stop();
        return 0;
    }

    if (argc >= 6 && strcmp(argv[3], "seq") == 0) {
        // Raw sequence of variant number argv[4] (uint32), for independence tests.
        size_t v = (size_t)atoi(argv[4]);
        FILE *fp = fopen(argv[5], "wb");
        for (long k = 0; k < N; k++) {
            uint32_t x = VARIANTS[v].f();
            fwrite(&x, sizeof(x), 1, fp);
        }
        fclose(fp);
        fo_stop();
        return 0;
    }

    volatile uint32_t sink = 0;
    for (size_t v = 0; v < sizeof(VARIANTS) / sizeof(VARIANTS[0]); v++) {
        double best_wall = 1e300, best_cpu = 1e300, bits = 0;
        for (int rep = 0; rep < 3; rep++) {
            uint32_t acc = 0;
            uint64_t b0 = bits_used();
            clock_t c0 = clock();
            double t0 = now_ns();
            for (long k = 0; k < N; k++) {
                acc += VARIANTS[v].f();
            }
            double t1 = now_ns();
            clock_t c1 = clock();
            uint64_t b1 = bits_used();
            sink += acc;
            double wall = (t1 - t0) / N;
            double cpu = (double)(c1 - c0) / CLOCKS_PER_SEC * 1e9 / N;
            if (wall < best_wall) best_wall = wall;
            if (cpu < best_cpu) best_cpu = cpu;
            bits = (double)(b1 - b0) / N;
        }
        printf("%-30s %7.2f ns  %7.2f ns(cpu)  %7.2f bits\n",
               VARIANTS[v].name, best_wall, best_cpu, bits);
        fflush(stdout);
    }
    fo_stop();
    return (int)(sink & 0);
}
