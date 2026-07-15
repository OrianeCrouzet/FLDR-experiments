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