/*
  Name:     main.c
  Purpose:  Command line interface for sampling algorithms.
  Author:   F. A. Saad and C. E. Freer
  Copyright (C) 2020 Feras A. Saad and C. E. Freer, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>


#include "flip.h"
#include "readio.h"
#include "sample.h"
#include "sstructs.h"
#include "construct.h"
#include "alias_integers_opt.h"
#include "alias_integers_v2.h"
#include "fldr_opt.h"
#include "aldr_opt.h"
#include "alias_rust_opt.h"
#include "alias_fractions_opt.h"
#include "flip_opt.h"
#include "macros.c"


#define CACHE_SIZE_BYTES (32 * 1024 * 1024 * 4) // 128 Mo (à adapter selon votre CPU)

void flush_cache() {
    volatile char *dummy = malloc(CACHE_SIZE_BYTES);
    if (!dummy) return;

    // On lit/écrit dans tout le tableau pour forcer le chargement en cache
    for (size_t i = 0; i < CACHE_SIZE_BYTES; i++) {
        dummy[i] = i % 256;
    }

    free((void*)dummy);
}


int main(int argc, char **argv) {
    // Read command line arguments.
    if (argc != 5) {
        printf("usage: %s seed steps sampler path\n", argv[0]);
        exit(0);
    }
    int seed = atoi(argv[1]);
    int steps = atoi(argv[2]);
    char *sampler = argv[3];
    char *path = argv[4];

    printf("%d %d %s %s\n", seed, steps, sampler, path);
    srand(seed);


    flush_cache();

    pthread_t prod_thread;
    bool is_spsc_sampler = false;

    // Les échantillonneurs *_opt et alias.integers_v2 utilisent le flux de bits optimisé (flip_opt.c).
    size_t len_sampler = strlen(sampler);
    bool is_opt_sampler = (len_sampler > 4 && strcmp(sampler + len_sampler - 4, "_opt") == 0)
        || strcmp(sampler, "alias.integers_v2") == 0;
    if (is_opt_sampler) {
        fo_start();
    } else {
    // Vérifie si l'échantillonneur actuel est 'alias.integers_old'
   // if (strcmp(sampler, "alias.integers_old") == 0 || strcmp(sampler, "alias.rust") == 0) {
        is_spsc_sampler = true;
        init_spsc_queue(); // Initialise la file SPSC
        //printf("[Main] Initialisation de la file SPSC et démarrage du thread producteur.\n");
        if (pthread_create(&prod_thread, NULL, spsc_producer, NULL) != 0) {
            perror("Erreur lors de la création du thread producteur SPSC");
            return EXIT_FAILURE;
        }
    //}
    }

    int x = 0;
    clock_t t;
    READ_SAMPLE_TIME("ky.enc",
        sampler,
        sample_ky_encoding_s,
        read_sample_ky_encoding,
        sample_ky_encoding,
        free_sample_ky_encoding_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("fdr",
        sampler,
        sample_fdr_s,
        read_sample_fdr,
        sample_fdr,
        free_sample_fdr_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("inv.bern",
        sampler,
        sample_inversion_bernoulli_s,
        read_sample_inversion_bernoulli,
        sample_inversion_bernoulli,
        free_sample_inversion_bernoulli_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("rej.enc",    // FLDR
        sampler,
        sample_ky_encoding_s,
        read_sample_ky_encoding,
        sample_rejection_encoding,
        free_sample_ky_encoding_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("rej.enc_gmp",    // FLDR with GMP
        sampler,
        sample_ky_encoding_gmp_s,
        read_sample_ky_encoding_gmp,
        sample_rejection_encoding_gmp,
        free_sample_ky_encoding_gmp_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.exact",
        sampler,
        sample_alias_exact_s,
        read_sample_alias_exact,
        sample_alias_exact,
        free_sample_alias_exact_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.gsl",
        sampler,
        sample_alias_gsl_s,
        read_sample_alias_gsl,
        sample_alias_gsl,
        free_sample_alias_gsl_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.integers_old",
        sampler,
        sample_alias_integers_s,
        read_sample_alias_integers_old,
        sample_alias_integers_old,
        free_sample_alias_integers_s_old,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.integers_gmp",
        sampler,
        sample_gmp_alias_integers_s,
        read_sample_alias_integers_gmp,
        sample_gmp_alias_integers,
        free_sample_gmp_alias_integers,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.integers_opt",
        sampler,
        sample_alias_integers_opt_s,
        read_sample_alias_integers_opt,
        sample_alias_integers_opt,
        free_sample_alias_integers_opt,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.integers_v2",
        sampler,
        sample_alias_integers_v2_s,
        read_sample_alias_integers_v2,
        sample_alias_integers_v2,
        free_sample_alias_integers_v2,
        path, steps, t, x)
    else READ_SAMPLE_TIME("rej.enc_opt",
        sampler,
        sample_fldr_opt_s,
        read_sample_fldr_opt,
        sample_fldr_opt,
        free_sample_fldr_opt,
        path, steps, t, x)
    else READ_SAMPLE_TIME("aldr_opt",
        sampler,
        sample_aldr_opt_s,
        read_sample_aldr_opt,
        sample_aldr_opt,
        free_sample_aldr_opt,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.rust_opt",
        sampler,
        sample_alias_rust_opt_s,
        read_sample_alias_rust_opt,
        sample_alias_rust_opt,
        free_sample_alias_rust_opt,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.fractions_opt",
        sampler,
        sample_alias_fractions_opt_s,
        read_sample_alias_fractions_opt,
        sample_alias_fractions_opt,
        free_sample_alias_fractions_opt,
        path, steps, t, x)
    else READ_SAMPLE_TIME("aldr",
        sampler,
        sample_aldr_s,
        read_sample_aldr,
        sample_aldr,
        free_sample_aldr_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("aldr_gmp",
        sampler,
        sample_aldr_gmp_s,
        read_sample_aldr_gmp,
        sample_aldr_flat_gmp,
        free_sample_aldr_gmp_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.rust",
        sampler,
        sample_alias_rust_s,
        read_sample_alias_rust,
        sample_alias_rust,
        free_sample_alias_rust_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.rust_gmp",
        sampler,
        sample_alias_rust_gmp_s,
        read_sample_alias_rust_gmp,
        sample_alias_rust_gmp,
        free_sample_alias_rust_gmp_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.fractions",
        sampler,
        sample_alias_fractions_s,
        read_sample_alias_fractions,
        sample_alias_fractions,
        free_sample_alias_fractions_s,
        path, steps, t, x)
    else READ_SAMPLE_TIME("alias.fractions_gmp",
        sampler,
        sample_alias_fractions_gmp_s,
        read_sample_alias_fractions_gmp,
        sample_alias_fractions_gmp_s,
        free_sample_alias_fractions_gmp_s,
        path, steps, t, x)
    else {
        printf("Unknown sampler: %s\n", sampler);
        exit(1);
    }

    double e = ((double)t) / CLOCKS_PER_SEC;
    printf("%s %1.5f %ld %ld\n", sampler, e, NUM_RNG_CALLS, REJET);
    
    if (is_opt_sampler) {
        fo_stop();
    }
     if (is_spsc_sampler) {
        //printf("[Main] Signalement au producteur SPSC de s'arrêter et attente de la fin du thread.\n");
        q.running = false; // Signale au thread producteur de s'arrêter
        if (pthread_join(prod_thread, NULL) != 0) {
            perror("Erreur lors de l'attente de la fin du thread producteur SPSC");
            return EXIT_FAILURE;
        }
        //printf("[Main] Thread producteur SPSC arrêté.\n");
    }

    flush_cache();

    return 0;
}
