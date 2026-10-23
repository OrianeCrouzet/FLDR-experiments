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

struct sample_fldr_opt_s {
    uint32_t n;          // labels >= n are rejected (same header as rej.enc_gmp)
    uint32_t length;     // length of the encoding
    int32_t *encoding;   // pack_tree: leaf = [-label], internal node = [loc(left), loc(right)]
};

struct sample_fldr_opt_s read_sample_fldr_opt(char *fname);
void free_sample_fldr_opt(struct sample_fldr_opt_s x);
size_t get_size_sample_fldr_opt_s(struct sample_fldr_opt_s *s);
int sample_fldr_opt(struct sample_fldr_opt_s *x);

#endif
