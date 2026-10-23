/*
  Name:     fldr_opt.h
  Purpose:  Optimized FLDR sampler (rej.enc_opt).
            Same DDG-tree encoding as rej.enc_gmp (built from arbitrary-precision
            weights by pack_tree), walked as in Saad's sample_rejection_encoding.
            The encoding holds memory locations and labels, stored as 32-bit
            integers; the tree is walked inside the current word of the
            random-bit stream (flip_opt.h).
*/

#ifndef FLDR_OPT_H
#define FLDR_OPT_H

#include <stddef.h>
#include <stdint.h>
#include <gmp.h>

struct sample_fldr_opt_s {
    uint32_t n;          // labels >= n are rejected (same header as rej.enc_gmp)
    uint32_t length;     // length of the encoding
    int32_t *encoding;   // pack_tree: leaf = [-label], internal node = [loc(left), loc(right)]
};

struct sample_fldr_opt_s read_sample_fldr_opt(char *fname);
// Construction from the probabilities w_i / Z (w is not modified): same encoding as
// construct_sample_rejection_encoding_gmp + pack_tree (src/), written in C. Z is the
// first line of the input files; it equals sum(w) except for the uniform distribution
// of distributions_entropy, where sum(w) exceeds it slightly.
struct sample_fldr_opt_s preprocess_fldr_opt(mpz_t *w, uint32_t n, const mpz_t Z);
void free_sample_fldr_opt(struct sample_fldr_opt_s x);
size_t get_size_sample_fldr_opt_s(struct sample_fldr_opt_s *s);
int sample_fldr_opt(struct sample_fldr_opt_s *x);

#endif
