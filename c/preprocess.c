/*
  Name:     preprocess.c
  Purpose:  Preprocessing algorithm for FLDR.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <gsl/gsl_randist.h>

#include "readio.h"
#include "sstructs.h"
#include "alias_fractions.h"
#include "utils.h"
#include "alias_rust.h"

int ceil_log2(unsigned long long x) {
  static const unsigned long long t[6] = {
    0xFFFFFFFF00000000ull,
    0x00000000FFFF0000ull,
    0x000000000000FF00ull,
    0x00000000000000F0ull,
    0x000000000000000Cull,
    0x0000000000000002ull
  };

  int y = (((x & (x - 1)) == 0) ? 0 : 1);
  int j = 32;
  int i;

  for (i = 0; i < 6; i++) {
    int k = (((x & t[i]) == 0) ? 0 : j);
    y += k;
    x >>= k;
    j >>= 1;
  }

  return y;
}

int preprocess_fldr(struct array_s x, int m) {
    int k = ceil_log2(m);
    int r = (1 << k) - m;
    int n = x.length + 1;

    int *h = calloc(k, sizeof(int));
    int *H = calloc(n*k, sizeof(int));

    int d;
    for(int j = 0; j < k; j++) {
        d = 0;
        for (int i = 0 ; i < x.length; i++) {
            bool w = (x.a[i] >> ((k-1) -j)) & 1;
            h[j] += w;
            if (w) {
                H[d*k + j] = i;
                d += 1;
            }
        }
        // Reject outcome.
        bool w = (w >> ((k-1) -j)) & 1;
        h[j] += w;
        if (w) {
            H[d*k + j] = n;
            d += 1;
        }
    }
    return d;
}

void preprocess_alias_gsl(struct array_s x, int m) {
    gsl_ran_discrete_preproc(x.length, (double*)x.a);       // code alias exact ?
}

int main(int argc, char **argv) { 
    // Read command line arguments.
    if (argc != 2) {
        printf("usage: ./mainc path\n");
        exit(0);
    }
    char *path = argv[1];


    //********************************
    // Mesures pour FLDR et Alias GSL
    //********************************

    /*FILE *fp = fopen(path, "r");
    int Z;
    fscanf(fp, "%d", &Z);
    struct array_s x = load_array(fp);
    fclose(fp);

    // Measure time of FLDR.
    clock_t t;
    t = clock();
    int d = preprocess_fldr(x, Z);
    t = clock() - t;
    double t_fldr = ((double) t) / CLOCKS_PER_SEC;

    // Measure time of Alias GSL.
    t = clock();
    preprocess_alias_gsl(x, Z);
    t = clock() - t;
    double t_alias = ((double) t) / CLOCKS_PER_SEC;

    printf("%dc %1.6f %1.6f\n", d, t_fldr, t_alias); */

    

    //***********************************************
    // Mesures pour Alias Integers et Alias Fractions
    //***********************************************

    FILE *fp = fopen(path, "r");
    int kmul;
    fscanf(fp, "%d", &kmul);
    int n;
    fscanf(fp, "%d", &n);
    int* array = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) {
        fscanf(fp, "%d", &array[i]);
    }
    fclose(fp);

    // Measure time of Alias Integers.
    // Echauffement
    struct sample_alias_integers_s temp10 = preprocess_alias_integers(array, n);
    clock_t t;
    t = clock();
    struct sample_alias_integers_s temp1 = preprocess_alias_integers(array, n);
    t = clock() - t;
    double t_alias_integers = ((double) t) / CLOCKS_PER_SEC;

    // Measure time of Alias Fractions.
    // Echauffement
    struct sample_alias_fractions_s temp20 = preprocess_alias_fractions(array, n);
    t = clock();
    struct sample_alias_fractions_s temp2 = preprocess_alias_fractions(array, n);
    t = clock() - t;
    double t_alias_fractions = ((double) t) / CLOCKS_PER_SEC;

    // Measure time of Alias Rust
    // Echauffement
    alias_rust_s temp30;
    weighted_alias_new(&temp30, array, n);
    t = clock();
    alias_rust_s temp3;
    weighted_alias_new(&temp3, array, n);
    t = clock() - t;
    double t_alias_rust = ((double) t) / CLOCKS_PER_SEC;

    // // Afficher que les tables pratiquement égales en terme de taille
    // if ((temp1.T.size)/2 < temp2.taille + 2) {
    //     printf("%dc %1.6f %1.6f\n", n, t_alias_integers, t_alias_fractions);
    //     fflush(stdout);
    // }

    printf("%dc %1.6f %1.6f\n", n, t_alias_integers, t_alias_fractions);
    fflush(stdout);
}
