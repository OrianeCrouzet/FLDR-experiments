/*
  Name:     flip.c
  Purpose:  Generating a sequence of pseudo-random bits.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#include <stdlib.h>
#include <sys/random.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <gmp.h>


#include "flip.h"


unsigned long NUM_RNG_CALLS = 0;


// *********************************************************************************
//              RANDOM BIT GENERATOR
// *********************************************************************************

// Initialisation globale de notre queue
SPSC_RandomQueue q;

// total bits consumed by the consumer
//_Atomic size_t total_bits_used = 0;

void init_spsc_queue(void) {
    memset(&q, 0, sizeof(SPSC_RandomQueue));
    q.running = true;
    // Réinitialisation des compteurs de bits si nécessaire
    NUM_RNG_CALLS = 0;
    //atomic_store(&total_bits_used, 0);
}

// --- LE PRODUCTEUR (Arrière-plan) ---
void* spsc_producer(void* arg) {
    //(void)arg; // pour utiliser arg (évitant le warning)
    //printf("[Producteur] Initialisé et prêt à remplir le buffer.\n");

    // On charge la queue une seule fois au début, car seul ce thread la modifie.
    //size_t current_tail = atomic_load_explicit(&q.tail, memory_order_relaxed);

    while (q.running) {
        //size_t current_head = atomic_load_explicit(&q.head, memory_order_acquire);

        size_t free_space = BUFFER_SIZE - 1 - ((q.tail - q.head) & BUFFER_MASK);

        if (free_space > 0) {
            // Remplissage d'un slot INT_MASK bits
            uint32_t val;
            arc4random_buf(&val, sizeof(val));
            //val = rand();

            q.data[q.tail] = val;
            q.tail = (q.tail + 1) & BUFFER_MASK;
            //atomic_store_explicit(&q.tail, current_tail, memory_order_release);
        } else {
            // Le buffer n'a pas encore assez de place, on réessaie très vite.
            sched_yield();
        }
    }
   //printf("[Producteur] Arrêt du thread.\n");
    return NULL;
}

// --- LE CONSOMMATEUR (Fonction de lecture) ---
uint32_t get_random_bits_spsc(size_t nbbits) {
    //size_t h = atomic_load_explicit(&q.head, memory_order_relaxed);
    //size_t t = atomic_load_explicit(&q.tail, memory_order_acquire);

    if (q.head == q.tail && q.running) {
        sched_yield();
        return get_random_bits_spsc(nbbits);
    }

    //size_t bit_offset = atomic_load_explicit(&q.bit_offset, memory_order_relaxed);
    uint32_t num_bits_extract = MIN(nbbits, INT_MASK + 1 - q.bit_offset);
    
    // Création d'un masque correct pour num_bits_extract bits
  //  uint32_t mask = (num_bits_extract == INT_MASK + 1) ? 0xFFFFFFFF : (1U << num_bits_extract) - 1;
  //  uint32_t b = (q.data[q.head] >> q.bit_offset) & mask;
    uint32_t b = (q.data[q.head] >> q.bit_offset) & (UINT32_MAX >> (32 - num_bits_extract));
    
    q.bit_offset += num_bits_extract;
    if (q.bit_offset > INT_MASK) {
        q.bit_offset = 0;
        q.head = (q.head + 1) & BUFFER_MASK;
        NUM_RNG_CALLS++;
    }

    //atomic_fetch_add(&total_bits_used, num_bits_extract);
    //atomic_store_explicit(&q.bit_offset, bit_offset, memory_order_release);
    return (num_bits_extract == nbbits) ? b :
        (b << (nbbits - num_bits_extract)) | get_random_bits_spsc(nbbits - num_bits_extract);
}


// *********************************************************************************
//              UNIFORM AND BERNOULLI - LUMBROSO
// *********************************************************************************


uint32_t uniform(uint32_t n) {    
    uint32_t num_bits_presample = 32 - __builtin_clz(n - 1);
    uint32_t bound = 1 << num_bits_presample;
    uint32_t x = get_random_bits_spsc(num_bits_presample);
    while (bound < n || x >= n) {
        if (bound >= n) {
            bound -= n;
            x -= n;
        }
        bound <<= 1;
        x = (x << 1) | get_random_bits_spsc(1);
    }
    return x;
}

uint32_t bernoulli(uint32_t numer, uint32_t denom) {
    if(numer == denom){
        return 1;
    }

    numer <<= 1;
    uint32_t y;
    while (numer != denom) {
        y = numer > denom;
        if (y) {
            numer -= denom;
        }
        if (get_random_bits_spsc(1)) {
            return y;
        }
        numer <<= 1;
    }
    return get_random_bits_spsc(1);
}

// ***** GMP - entiers de taille arbitraire *****

// FDR with GMP, translated from flip.c (Saad)
void uniform_with_gmp(mpz_t result, const mpz_t n) {
    if (mpz_cmp_ui(n, 1) <= 0) {
        mpz_set_ui(result, 0);
        return;
    }

    // 1. Calcul exact de num_bits = bits(n - 1)
    mpz_t n_minus_1;
    mpz_init_set(n_minus_1, n);
    mpz_sub_ui(n_minus_1, n_minus_1, 1);
    size_t num_bits_presample = mpz_sizeinbase(n_minus_1, 2);
    mpz_clear(n_minus_1);

    mpz_t bound, x;
    mpz_inits(bound, x, NULL);

    // bound = 2^num_bits_presample
    mpz_ui_pow_ui(bound, 2, num_bits_presample);

    // 2. REMPLISSAGE SÉCURISÉ PAR BLOCS DE 32 BITS (Anti-Overflow)
    mpz_set_ui(x, 0);
    size_t bits_left = num_bits_presample;
    while (bits_left > 0) {
        size_t chunk = (bits_left > 32) ? 32 : bits_left;
        mpz_mul_2exp(x, x, chunk);
        mpz_add_ui(x, x, get_random_bits_spsc(chunk));
        bits_left -= chunk;
    }

    // 3. Boucle Fast Dice Roller (FDR)
    for (;;) {
        if (mpz_cmp(bound, n) >= 0) {
            if (mpz_cmp(x, n) < 0) { 
                mpz_set(result, x);
                break;
            }
            mpz_sub(bound, bound, n);
            mpz_sub(x, x, n);
        }
        mpz_mul_2exp(bound, bound, 1);
        mpz_mul_2exp(x, x, 1);          
        mpz_add_ui(x, x, get_random_bits_spsc(1));
    }
    
    mpz_clears(x, bound, NULL);
}

//Bernoulli with GMP, translated from flip.c (Saad)
int bernoulli_with_gmp(const mpz_t numer, const mpz_t denom) {
    if (mpz_cmp_ui(numer, 0) == 0) return 0;
    if (mpz_cmp(numer, denom) == 0) return 1;

    mpz_t tmp;
    mpz_init_set(tmp, numer);

    int y;
    for (;;) {
        mpz_mul_2exp(tmp, tmp, 1);
        if (mpz_cmp(tmp, denom) == 0) {
            y = get_random_bits_spsc(1);
            break;
        }
        y = (mpz_cmp(tmp, denom) > 0);
        if (y) {
            mpz_sub(tmp, tmp, denom);
        }
        if (get_random_bits_spsc(1)) {
            break;
        }
    }

    mpz_clear(tmp);
    return y;
}

// *********************************************************************************
//                                      UTILS
// *********************************************************************************

// Calcule le nombre de bits nécessaires pour représenter un entier non nul
unsigned int bit_length(unsigned int x) {
    unsigned int length = 0;
    while (x > 0) {
        length++;
        x >>= 1;
    }
    return length;
}