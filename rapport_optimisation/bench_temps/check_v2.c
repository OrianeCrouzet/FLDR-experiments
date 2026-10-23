/*
  Name:     check_v2.c
  Purpose:  Histogram (chi2.py format) and raw sequence of alias.integers_v2,
            to test its exactness.
  Usage:    check_v2.out FILE N HIST_OUT SEQ_OUT
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alias_integers_v2.h"
#include "flip_opt.h"

unsigned long NUM_RNG_CALLS = 0;
unsigned long REJET = 0;

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s FILE N HIST_OUT SEQ_OUT\n", argv[0]);
        return EXIT_FAILURE;
    }
    long N = atol(argv[2]);
    struct sample_alias_integers_v2_s s = read_sample_alias_integers_v2(argv[1]);
    fo_start();
    uint64_t *h = calloc(s.q + 2, sizeof(uint64_t));
    FILE *fs = fopen(argv[4], "wb");
    for (long k = 0; k < N; k++) {
        uint32_t x = sample_alias_integers_v2(&s);
        h[x]++;
        fwrite(&x, sizeof(x), 1, fs);
    }
    fclose(fs);
    fo_stop();
    FILE *fh = fopen(argv[3], "w");
    fprintf(fh, "alias.integers_v2\n");
    for (uint32_t k = 0; k < s.q + 2; k++) {
        fprintf(fh, "%llu\n", (unsigned long long)h[k]);
    }
    fclose(fh);
    printf("bits/tirage %.3f\n", 32.0 * NUM_RNG_CALLS / N);
    free_sample_alias_integers_v2(s);
    return 0;
}
