#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flip.h"
#include "sample.h"
#include "readio.h"
#include "sstructs.h"

#define RUN(NAME, TYPE, READ, SAMPLE, FREE)                     \
if (strcmp(name, NAME) == 0) {                                  \
    TYPE s = READ(path);                                        \
    FILE *f;                                                    \
    char outname[256];                                          \
    snprintf(outname, sizeof(outname), "histogram/%s.histo", name); \
    f = fopen(outname, "w");                                    \
    if (!f) { perror("fopen"); exit(1); }                      \
                                                                \
    for (int i = 0; i < steps; i++) {                          \
        fprintf(f, "%d ", SAMPLE(&s));                         \
    }                                                           \
    fprintf(f, "\n");                                           \
                                                                \
    fclose(f);                                                  \
    FREE(s);                                                    \
    return;                                                     \
}

void run_sampler(char *name, char *path, int steps)
{
    RUN("alias.exact",
        struct sample_alias_exact_s,
        read_sample_alias_exact,
        sample_alias_exact,
        free_sample_alias_exact_s)

    RUN("alias.integers",
        struct sample_alias_integers_s,
        read_sample_alias_integers,
        sample_alias_integers,
        free_sample_alias_integers_s)

    RUN("aldr",
        struct sample_aldr_s,
        read_sample_aldr,
        sample_aldr,
        free_sample_aldr_s)

    RUN("rej.enc",
        struct sample_ky_encoding_s,
        read_sample_ky_encoding,
        sample_rejection_encoding,
        free_sample_ky_encoding_s)

    RUN("alias.rust",
        struct sample_alias_rust_s,
        read_sample_alias_rust,
        sample_alias_rust,
        free_sample_alias_rust_s)

    RUN("alias.fractions",
        struct sample_alias_fractions_s,
        read_sample_alias_fractions,
        sample_alias_fractions,
        free_sample_alias_fractions_s)

    RUN("alias.integers_old",
        struct sample_alias_integers_s,
        read_sample_alias_integers_old,
        sample_alias_integers_old,
        free_sample_alias_integers_s)

    printf("Unknown sampler %s\n", name);
}

int main(int argc, char **argv)
//      Lancer : ./histogram.out histogram/*.dist 1000000
{
    if (argc != 3) {
        printf("usage: %s file.dist steps\n", argv[0]);
        return 1;
    }

    pthread_t prod_thread;
    bool is_spsc_sampler = false;

    char *dist = argv[1];
    int steps = atoi(argv[2]);

    char samplers[][32] = {
        "alias.integers",
        "aldr",
        "alias.rust",
        "alias.fractions",
        "alias.integers_old",
    };

    for (int i = 0; i < 5; i++) {
        // Vérifie si l'échantillonneur actuel utilise la file SPSC
        if (strcmp(samplers[i], "alias.integers") == 0 || strcmp(samplers[i], "alias.integers_old") == 0 || strcmp(samplers[i], "alias.rust") == 0) {
            is_spsc_sampler = true;
            init_spsc_queue(); // Initialise la file SPSC
            //printf("[Main] Initialisation de la file SPSC et démarrage du thread producteur.\n");
            if (pthread_create(&prod_thread, NULL, spsc_producer, NULL) != 0) {
                perror("Erreur lors de la création du thread producteur SPSC");
                return EXIT_FAILURE;
            }
        }

        printf("=== %s ===\n", samplers[i]);
        run_sampler(samplers[i], dist, steps);
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

    return 0;
}