/*
  Name:     flip_opt.c
  Purpose:  Producer thread of the optimized random-bit stream (see flip_opt.h).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "flip_opt.h"

fo_queue_t fo_q;
static pthread_t fo_thread;

static void *fo_producer(void *arg) {
    (void)arg;
    while (fo_q.running) {
        uint64_t t = fo_q.tail;
        uint64_t h = __atomic_load_n(&fo_q.head, __ATOMIC_ACQUIRE);
        if (FO_WORDS - (t - h) >= FO_CHUNK) {
            // FO_WORDS is a multiple of FO_CHUNK: a chunk never wraps around.
            arc4random_buf(&fo_q.data[t & FO_MASK], FO_CHUNK * sizeof(uint64_t));
            __atomic_store_n(&fo_q.tail, t + FO_CHUNK, __ATOMIC_RELEASE);
        } else {
            // Full buffer (~2 million bits): sleep instead of spinning.
            usleep(50);
        }
    }
    return NULL;
}

void fo_start(void) {
    memset(&fo_q, 0, sizeof(fo_q));
    fo_q.running = 1;
    NUM_RNG_CALLS = 0;
    if (pthread_create(&fo_thread, NULL, fo_producer, NULL) != 0) {
        perror("fo_start: pthread_create");
        exit(EXIT_FAILURE);
    }
}

void fo_stop(void) {
    fo_q.running = 0;
    pthread_join(fo_thread, NULL);
}

// Called when every known word has been consumed.
void fo_wait_slow(void) {
    __atomic_store_n(&fo_q.head, fo_q.c_head, __ATOMIC_RELEASE);
    fo_q.c_publish = 0;
    for (;;) {
        fo_q.c_tail = __atomic_load_n(&fo_q.tail, __ATOMIC_ACQUIRE);
        if (fo_q.c_head != fo_q.c_tail) {
            return;
        }
        sched_yield();
    }
}


// *********************************************************************************
//   Bernoulli with precomputed digits
// *********************************************************************************

uint32_t fo_digits(const mpz_t t, const mpz_t d, mpz_t tmp, mpz_t rem) {
    // digits = floor(t * 2^FO_DIGITS / d) < 2^FO_DIGITS, rem = t * 2^FO_DIGITS mod d
    mpz_mul_2exp(tmp, t, FO_DIGITS);
    mpz_fdiv_qr(tmp, rem, tmp, d);
    return ((uint32_t)mpz_get_ui(tmp) << (32 - FO_DIGITS)) | (mpz_sgn(rem) == 0);
}

uint32_t fo_bernoulli_tail(const mpz_t t, const mpz_t d) {
    mpz_t tmp;
    mpz_init(tmp);
    mpz_mul_2exp(tmp, t, FO_DIGITS);
    mpz_fdiv_r(tmp, tmp, d);
    uint32_t y;
    for (;;) {
        mpz_mul_2exp(tmp, tmp, 1);
        if (mpz_cmp(tmp, d) == 0) {
            y = fo_bit();
            break;
        }
        y = (mpz_cmp(tmp, d) > 0);
        if (y) {
            mpz_sub(tmp, tmp, d);
        }
        if (fo_bit()) {
            break;
        }
    }
    mpz_clear(tmp);
    return y;
}
