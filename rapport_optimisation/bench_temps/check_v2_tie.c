/*
  Name:     check_v2_tie.c
  Purpose:  Tests the slow path of alias.integers_v2 (lo = D_i, probability 2^-32
            in normal use): its frequency of 1 must be frac(T_i 2^32 / cs).
  Usage:    check_v2_tie.out FILE CALLS
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../c/alias_integers_v2_sample.c"

unsigned long NUM_RNG_CALLS = 0;
unsigned long REJET = 0;

int main(int argc, char **argv) {
    struct sample_alias_integers_v2_s s = read_sample_alias_integers_v2(argv[1]);
    long N = atol(argv[2]);
    fo_start();
    mpz_t t, r;
    mpz_inits(t, r, NULL);
    int shown = 0;
    // Cells with an alias, spread over the table.
    for (uint32_t i = 0; i < s.base.m && shown < 8; i += s.base.m / 8 + 1) {
        const struct alias_v2_cell *c = &s.cells[i];
        uint32_t end = (i + 1 < s.base.m) ? s.cells[i + 1].thr : s.base.thr_size;
        mpz_import(t, end - c->thr, -1, 1, 0, 0, s.base.Threshold + c->thr);
        mpz_mul_2exp(t, t, 32);
        mpz_fdiv_r(r, t, s.base.cs);
        double p = mpz_get_d(r) / mpz_get_d(s.base.cs);
        long ones = 0;
        for (long k = 0; k < N; k++) {
            ones += bernoulli_tie(&s, i);
        }
        double f = (double)ones / N;
        double z = (p > 0 && p < 1) ? (f - p) / sqrt(p * (1 - p) / N) : (f - p) * 1e9;
        printf("cellule %6u  attendu %.6f  observé %.6f  z = %+.2f\n", i, p, f, z);
        shown++;
    }
    mpz_clears(t, r, NULL);
    fo_stop();
    return 0;
}
