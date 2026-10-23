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

// *********************************************************************************
//   Construction (port of src/rejection.py, matrix.py, tree.py, packing.py)
// *********************************************************************************
//
// The probabilities are p_i = w_i / Z. With g = gcd(Z, w_i), their common denominator
// is Zc = Z / g, the rows are M_i = w_i / g (i < n) and the rejection row is
// M_n = 2^k - sum(M_i), with k = ceil(log2 Zc); sum(M) = 2^k. reduce_fractions
// then halves all the M while they are even. The DDG tree of a dyadic matrix has no
// back edge: at depth d (1..k), the children of the internal nodes of depth d-1 are,
// from left to right, the internal nodes of depth d, then the leaves of the rows whose
// bit k-d is 1, by decreasing row (make_leaf_table fills the level from the right).
// pack_tree writes it in preorder, left child first: a leaf is [-(row+1)], an internal
// node is [loc(left), loc(right)].

struct fldr_build {
    uint32_t k;
    uint32_t *n_internal;     // internal nodes per depth (depth 0 = root)
    uint32_t **rows;          // rows[d]: rows with a leaf at depth d, increasing
    uint32_t *n_rows;
    int32_t *enc;
    uint32_t pos;
};

// Writes the node at position j of depth d (preorder) and returns nothing; b->pos advances.
static void pack_node(struct fldr_build *b, uint32_t d, uint32_t j) {
    if (j >= b->n_internal[d]) {
        // Leaf: positions n_internal[d] .. width-1 hold the rows in decreasing order.
        uint32_t width = b->n_internal[d] + b->n_rows[d];
        uint32_t row = b->rows[d][width - 1 - j];
        b->enc[b->pos++] = -(int32_t)(row + 1);
        return;
    }
    uint32_t here = b->pos;
    b->pos += 2;
    b->enc[here] = (int32_t)b->pos;
    pack_node(b, d + 1, 2 * j);
    b->enc[here + 1] = (int32_t)b->pos;
    pack_node(b, d + 1, 2 * j + 1);
}

struct sample_fldr_opt_s preprocess_fldr_opt(mpz_t *w, uint32_t n, const mpz_t Zfile) {
    uint32_t N = n + 1;                       // rows, rejection included
    mpz_t *M = malloc(N * sizeof(mpz_t));
    mpz_t g, Z, t;
    mpz_inits(g, Z, t, NULL);
    mpz_set(g, Zfile);
    for (uint32_t i = 0; i < n; i++) {
        mpz_gcd(g, g, w[i]);
    }
    mpz_divexact(Z, Zfile, g);                // common denominator of the w_i / Z
    // get_rejection_precision: log2(Z) if Z is a power of 2, else bit_length(Z).
    uint32_t k = (uint32_t)mpz_sizeinbase(Z, 2);
    if (mpz_popcount(Z) == 1) {
        k--;
    }
    mpz_set_ui(t, 0);
    for (uint32_t i = 0; i < n; i++) {
        mpz_init(M[i]);
        mpz_divexact(M[i], w[i], g);
        mpz_add(t, t, M[i]);
    }
    mpz_init(M[n]);
    mpz_setbit(M[n], k);
    mpz_sub(M[n], M[n], t);                   // rejection: 2^k - sum(M_i)
    if (mpz_sgn(M[n]) < 0) {
        fprintf(stderr, "[Erreur Fatal] FLDR : somme des probabilités > 2^k.\n");
        exit(EXIT_FAILURE);
    }

    struct sample_fldr_opt_s x;
    x.n = N;
    // reduce_fractions, case l = k (dyadic).
    for (;;) {
        int one = 0, all_even = 1, all_equal = 1;
        for (uint32_t i = 0; i < N; i++) {
            if (mpz_sizeinbase(M[i], 2) == k + 1 && mpz_popcount(M[i]) == 1) {
                one = 1;                      // M_i = 2^k
            }
            if (mpz_odd_p(M[i])) {
                all_even = 0;
            }
            if (mpz_cmp(M[i], M[0]) != 0) {
                all_equal = 0;
            }
        }
        if (one) {
            // (k', l') = (1, 0) and P = [[1]]: a single leaf of label 1.
            x.n = 1;
            x.length = 1;
            x.encoding = malloc(sizeof(int32_t));
            x.encoding[0] = -1;
            for (uint32_t i = 0; i < N; i++) {
                mpz_clear(M[i]);
            }
            free(M);
            mpz_clears(g, Z, t, NULL);
            return x;
        }
        if (k == 0) {
            break;
        }
        if (all_even) {
            for (uint32_t i = 0; i < N; i++) {
                mpz_fdiv_q_2exp(M[i], M[i], 1);
            }
            k--;
            continue;
        }
        if (all_equal && mpz_popcount(M[0]) == 1) {
            // 2^k / M_0 = 2^k' : every row becomes 1 over 2^k'.
            uint32_t kp = k - (uint32_t)(mpz_sizeinbase(M[0], 2) - 1);
            for (uint32_t i = 0; i < N; i++) {
                mpz_set_ui(M[i], 1);
            }
            k = kp;
        }
        break;
    }

    struct fldr_build b;
    b.k = k;
    b.n_internal = calloc(k + 1, sizeof(uint32_t));
    b.n_rows = calloc(k + 1, sizeof(uint32_t));
    b.rows = calloc(k + 1, sizeof(uint32_t *));
    size_t total = 0;
    for (uint32_t d = 1; d <= k; d++) {
        uint32_t cnt = 0;
        for (uint32_t i = 0; i < N; i++) {
            cnt += mpz_tstbit(M[i], k - d);
        }
        b.rows[d] = malloc((cnt + 1) * sizeof(uint32_t));
        b.n_rows[d] = 0;
        for (uint32_t i = 0; i < N; i++) {
            if (mpz_tstbit(M[i], k - d)) {
                b.rows[d][b.n_rows[d]++] = i;
            }
        }
    }
    b.n_internal[0] = 1;
    for (uint32_t d = 1; d <= k; d++) {
        uint64_t width = 2 * (uint64_t)b.n_internal[d - 1];
        if (b.n_rows[d] > width) {
            fprintf(stderr, "[Erreur Fatal] arbre FLDR incohérent (profondeur %u).\n", d);
            exit(EXIT_FAILURE);
        }
        b.n_internal[d] = (uint32_t)(width - b.n_rows[d]);
        total += 2 * (size_t)b.n_internal[d - 1];
    }
    if (b.n_internal[k] != 0 || total + 2 >= INT32_MAX) {
        fprintf(stderr, "[Erreur Fatal] arbre FLDR incohérent ou trop grand.\n");
        exit(EXIT_FAILURE);
    }
    // Encoding: 2 cells per internal node, 1 per leaf.
    size_t length = 0;
    for (uint32_t d = 0; d < k; d++) {
        length += 2 * (size_t)b.n_internal[d];
    }
    for (uint32_t d = 1; d <= k; d++) {
        length += b.n_rows[d];
    }
    b.enc = malloc(length * sizeof(int32_t));
    b.pos = 0;
    pack_node(&b, 0, 0);

    x.length = b.pos;
    x.encoding = b.enc;
    for (uint32_t d = 1; d <= k; d++) {
        free(b.rows[d]);
    }
    free(b.rows);
    free(b.n_rows);
    free(b.n_internal);
    for (uint32_t i = 0; i < N; i++) {
        mpz_clear(M[i]);
    }
    free(M);
    mpz_clears(g, Z, t, NULL);
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
