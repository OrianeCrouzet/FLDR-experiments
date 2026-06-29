/*
  Name:     flip.h
  Purpose:  Generating a sequence of pseudo-random bits.
  Author:   F. A. Saad
  Copyright (C) 2020 Feras A. Saad, All Rights Reserved.

  Released under Apache 2.0; refer to LICENSE.txt
*/

#ifndef FLIP_H
#define FLIP_H

//#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


extern unsigned long NUM_RNG_CALLS;


// *********************************************************************************
//              UNIFORM AND BERNOULLI - LUMBROSO
// *********************************************************************************

uint32_t uniform(uint32_t n);
uint32_t bernoulli(uint32_t numer, uint32_t denom);


// *********************************************************************************
//              RANDOM BIT GENERATOR
// *********************************************************************************

#define BUFFER_SIZE 64  // doit être une puissance de 2 à cause des orépration bis-à-bit.
#define BUFFER_MASK (BUFFER_SIZE - 1)
// pour INT_MASK
// 30 si rand() -> 31 bits alea + 1 signe;
// 31 si arc4random -> 32 bits alea
#define INT_MASK 31

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    uint32_t data[BUFFER_SIZE];
    size_t head; 
    size_t tail; 
    size_t bit_offset; // Nombre de bits déjà lus dans le uint32 pointé par head (0..31)
    volatile bool running;
} SPSC_RandomQueue;

// On déclare la variable q comme externe pour qu'elle soit visible par main.c
extern SPSC_RandomQueue q;

// On déclare la fonction d'initialisation
void init_spsc_queue(void);

void* spsc_producer(void* arg);
uint32_t get_random_bits_spsc(size_t nbbits);


#define min(a, b) ((a) < (b) ? (a) : (b))


#endif