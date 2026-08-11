/*
  Name:     measure_sizes.c
  Purpose:  Measure sizes of data structures for sampling algorithms.
  Author:   Oriane Crouzet
  Copyright (C) 2026 Oriane Crouzet
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readio.h"
#include "sizes.h"

static void usage(char *prog) {
    fprintf(stderr, "usage: %s sampler path out_file\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage(argv[0]);
    }

    char *sampler = argv[1];
    char *path = argv[2];
    char *out_path = argv[3];
    size_t size = 0;

    if (strcmp(sampler, "alias.integers_old") == 0) {
        struct sample_alias_integers_s s = read_sample_alias_integers_old(path);
        size = get_size_sample_alias_integers_s(&s);
        free_sample_alias_integers_s_old(s);

    } else if (strcmp(sampler, "alias.integers_gmp") == 0) {
        struct sample_gmp_alias_integers_s s = read_sample_alias_integers_gmp(path);
        size = get_size_sample_gmp_alias_integers_s(&s);
        free_sample_gmp_alias_integers(s);
        
    } else if (strcmp(sampler, "alias.rust") == 0) {
        struct sample_alias_rust_s s = read_sample_alias_rust(path);
        size = get_size_sample_alias_rust_s(&s);
        free_sample_alias_rust_s(s);

    } else if (strcmp(sampler, "aldr") == 0) {
        struct sample_aldr_s s = read_sample_aldr(path);
        size = get_size_sample_aldr_s(&s);
        free_sample_aldr_s(s);

    } else if (strcmp(sampler, "aldr_gmp") == 0) {
        struct sample_aldr_gmp_s s = read_sample_aldr_gmp(path);
        size = get_size_sample_aldr_gmp_s(&s);
        free_sample_aldr_gmp_s(s);

    } else if (strcmp(sampler, "alias.fractions") == 0) {
        struct sample_alias_fractions_s s = read_sample_alias_fractions(path);
        size = get_size_sample_alias_fractions_s(&s);
        free_sample_alias_fractions_s(s);

    } else if (strcmp(sampler, "rej.enc") == 0) {
        struct sample_ky_encoding_s s = read_sample_ky_encoding(path);
        size = get_size_sample_ky_encoding_s(&s);
        free_sample_ky_encoding_s(s);

    } else if (strcmp(sampler, "rej.enc_gmp") == 0) {
        struct sample_ky_encoding_gmp_s s = read_sample_ky_encoding_gmp(path);
        size = get_size_sample_ky_encoding_gmp_s(&s);
        free_sample_ky_encoding_gmp_s(s);

    } else if (strcmp(sampler, "alias.exact") == 0) {
        struct sample_alias_exact_s s = read_sample_alias_exact(path);
        size = get_size_sample_alias_exact_s(&s);
        free_sample_alias_exact_s(s);


    } else {
        fprintf(stderr, "Unknown sampler: %s\n", sampler);
        return EXIT_FAILURE;
    }

    FILE *out = fopen(out_path, "w");
    if (!out) {
        perror(out_path);
        return EXIT_FAILURE;
    }
    fprintf(out, "%zu\n", size);
    fclose(out);

    return 0;
}
