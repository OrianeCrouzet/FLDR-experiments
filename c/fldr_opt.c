/*
  Name:     fldr_opt.c
  Purpose:  Optimized FLDR sampler (see fldr_opt.h).
*/

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "fldr_opt.h"
#include "flip_opt.h"

static int32_t read_int32(FILE *fp, mpz_t v, const char *fname) {
    if (mpz_inp_str(v, fp, 10) == 0 || !mpz_fits_sint_p(v)) {
        fprintf(stderr, "[Erreur] valeur invalide ou hors int32 dans %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return (int32_t)mpz_get_si(v);
}

// File: "n k" then the encoding length and the encoding (format of rej.enc_gmp).
struct sample_fldr_opt_s read_sample_fldr_opt(char *fname) {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        perror(fname);
        exit(EXIT_FAILURE);
    }
    struct sample_fldr_opt_s x;
    mpz_t v;
    mpz_init(v);
    int32_t n = read_int32(fp, v, fname);
    read_int32(fp, v, fname);                 // k, unused for sampling
    int32_t length = read_int32(fp, v, fname);
    if (n < 0 || length <= 0) {
        fprintf(stderr, "[Erreur] en-tête invalide dans %s\n", fname);
        exit(EXIT_FAILURE);
    }
    x.n = (uint32_t)n;
    x.length = (uint32_t)length;
    x.encoding = malloc((size_t)length * sizeof(int32_t));
    for (int32_t i = 0; i < length; i++) {
        x.encoding[i] = read_int32(fp, v, fname);
    }
    // A child location points either to a leaf (one cell) or to an internal node (two cells).
    for (int32_t i = 0; i < length; i++) {
        int32_t e = x.encoding[i];
        if (e >= 0 && (e >= length || (x.encoding[e] >= 0 && e + 1 >= length))) {
            fprintf(stderr, "[Erreur] index hors encodage dans %s\n", fname);
            exit(EXIT_FAILURE);
        }
    }
    mpz_clear(v);
    fclose(fp);
    return x;
}

void free_sample_fldr_opt(struct sample_fldr_opt_s x) {
    free(x.encoding);
}

size_t get_size_sample_fldr_opt_s(struct sample_fldr_opt_s *s) {
    return sizeof(*s) + (size_t)s->length * sizeof(int32_t);
}

// Walk of the encoding of pack_tree (Saad et al.), as in sample_rejection_encoding:
// c = 0; repeat { c = enc[c + bit]; if enc[c] < 0 the label is -enc[c] },
// restarting from the root when the label is >= n.
// Note: sample_ky_encoding_gmp tests enc[c + bit] < 0 instead, which reads one extra
// bit at each leaf and then the cell that follows the leaf: it is biased.
int sample_fldr_opt(struct sample_fldr_opt_s *x) {
    if (x->length == 1) {
        return 1;                     // as sample_ky_encoding
    }
    const int32_t *enc = x->encoding;
    int32_t c = 0;
    for (;;) {
        uint32_t avail;
        uint64_t w = fo_peek(&avail);
        uint32_t used = 0;
        while (used < avail) {
            c = enc[c + (int32_t)(w & 1)];
            w >>= 1;
            used++;
            int32_t e = enc[c];
            if (e < 0) {
                if ((uint32_t)(-e) < x->n) {
                    fo_skip(used);
                    return -e;
                }
                c = 0;
            }
        }
        fo_skip(used);
    }
}
