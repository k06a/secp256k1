/***********************************************************************
 * Copyright (c) 2014 Pieter Wuille                                    *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#ifndef SECP256K1_SCALAR_REPR_IMPL_H
#define SECP256K1_SCALAR_REPR_IMPL_H

#include "modinv32_impl.h"

/* Limbs of the secp256k1 order. */
#define SECP256K1_N_0 ((uint32_t)0xD0364141UL)
#define SECP256K1_N_1 ((uint32_t)0xBFD25E8CUL)
#define SECP256K1_N_2 ((uint32_t)0xAF48A03BUL)
#define SECP256K1_N_3 ((uint32_t)0xBAAEDCE6UL)
#define SECP256K1_N_4 ((uint32_t)0xFFFFFFFEUL)
#define SECP256K1_N_5 ((uint32_t)0xFFFFFFFFUL)
#define SECP256K1_N_6 ((uint32_t)0xFFFFFFFFUL)
#define SECP256K1_N_7 ((uint32_t)0xFFFFFFFFUL)

/* Limbs of 2^256 minus the secp256k1 order. */
#define SECP256K1_N_C_0 (~SECP256K1_N_0 + 1)
#define SECP256K1_N_C_1 (~SECP256K1_N_1)
#define SECP256K1_N_C_2 (~SECP256K1_N_2)
#define SECP256K1_N_C_3 (~SECP256K1_N_3)
#define SECP256K1_N_C_4 (1)

/* Limbs of half the secp256k1 order. */
#define SECP256K1_N_H_0 ((uint32_t)0x681B20A0UL)
#define SECP256K1_N_H_1 ((uint32_t)0xDFE92F46UL)
#define SECP256K1_N_H_2 ((uint32_t)0x57A4501DUL)
#define SECP256K1_N_H_3 ((uint32_t)0x5D576E73UL)
#define SECP256K1_N_H_4 ((uint32_t)0xFFFFFFFFUL)
#define SECP256K1_N_H_5 ((uint32_t)0xFFFFFFFFUL)
#define SECP256K1_N_H_6 ((uint32_t)0xFFFFFFFFUL)
#define SECP256K1_N_H_7 ((uint32_t)0x7FFFFFFFUL)

SECP256K1_INLINE static void secp256k1_scalar_clear(secp256k1_scalar *r) {
    r->d[0] = 0;
    r->d[1] = 0;
    r->d[2] = 0;
    r->d[3] = 0;
    r->d[4] = 0;
    r->d[5] = 0;
    r->d[6] = 0;
    r->d[7] = 0;
}

SECP256K1_INLINE static void secp256k1_scalar_set_int(secp256k1_scalar *r, unsigned int v) {
    r->d[0] = v;
    r->d[1] = 0;
    r->d[2] = 0;
    r->d[3] = 0;
    r->d[4] = 0;
    r->d[5] = 0;
    r->d[6] = 0;
    r->d[7] = 0;
}

SECP256K1_INLINE static unsigned int secp256k1_scalar_get_bits(const secp256k1_scalar *a, unsigned int offset, unsigned int count) {
    VERIFY_CHECK((offset + count - 1) >> 5 == offset >> 5);
    return (a->d[offset >> 5] >> (offset & 0x1F)) & ((1 << count) - 1);
}

SECP256K1_INLINE static unsigned int secp256k1_scalar_get_bits_var(const secp256k1_scalar *a, unsigned int offset, unsigned int count) {
    VERIFY_CHECK(count < 32);
    VERIFY_CHECK(offset + count <= 256);
    if ((offset + count - 1) >> 5 == offset >> 5) {
        return secp256k1_scalar_get_bits(a, offset, count);
    } else {
        VERIFY_CHECK((offset >> 5) + 1 < 8);
        return ((a->d[offset >> 5] >> (offset & 0x1F)) | (a->d[(offset >> 5) + 1] << (32 - (offset & 0x1F)))) & ((((uint32_t)1) << count) - 1);
    }
}

static unsigned int secp256k1_scalar_get_bit(const secp256k1_scalar *a, unsigned int offset) {
    return (a->d[offset >> 5] >> (offset & 31)) & 1;
}

SECP256K1_INLINE static int secp256k1_scalar_check_overflow(const secp256k1_scalar *a) {
    int yes = 0;
    int no = 0;
    no |= (a->d[7] < SECP256K1_N_7); /* No need for a > check. */
    no |= (a->d[6] < SECP256K1_N_6); /* No need for a > check. */
    no |= (a->d[5] < SECP256K1_N_5); /* No need for a > check. */
    no |= (a->d[4] < SECP256K1_N_4);
    yes |= (a->d[4] > SECP256K1_N_4) & ~no;
    no |= (a->d[3] < SECP256K1_N_3) & ~yes;
    yes |= (a->d[3] > SECP256K1_N_3) & ~no;
    no |= (a->d[2] < SECP256K1_N_2) & ~yes;
    yes |= (a->d[2] > SECP256K1_N_2) & ~no;
    no |= (a->d[1] < SECP256K1_N_1) & ~yes;
    yes |= (a->d[1] > SECP256K1_N_1) & ~no;
    yes |= (a->d[0] >= SECP256K1_N_0) & ~no;
    return yes;
}

SECP256K1_INLINE static int secp256k1_scalar_reduce(secp256k1_scalar *r, uint32_t overflow) {
    uint64_t t;
    VERIFY_CHECK(overflow <= 1);
    t = (uint64_t)r->d[0] + overflow * SECP256K1_N_C_0;
    r->d[0] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[1] + overflow * SECP256K1_N_C_1;
    r->d[1] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[2] + overflow * SECP256K1_N_C_2;
    r->d[2] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[3] + overflow * SECP256K1_N_C_3;
    r->d[3] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[4] + overflow * SECP256K1_N_C_4;
    r->d[4] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[5];
    r->d[5] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[6];
    r->d[6] = t & 0xFFFFFFFFUL; t >>= 32;
    t += (uint64_t)r->d[7];
    r->d[7] = t & 0xFFFFFFFFUL;
    return overflow;
}

static int secp256k1_scalar_add(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b) {
    int overflow;
    uint64_t t = (uint64_t)a->d[0] + b->d[0];
    r->d[0] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[1] + b->d[1];
    r->d[1] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[2] + b->d[2];
    r->d[2] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[3] + b->d[3];
    r->d[3] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[4] + b->d[4];
    r->d[4] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[5] + b->d[5];
    r->d[5] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[6] + b->d[6];
    r->d[6] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)a->d[7] + b->d[7];
    r->d[7] = t & 0xFFFFFFFFULL; t >>= 32;
    overflow = t + secp256k1_scalar_check_overflow(r);
    VERIFY_CHECK(overflow == 0 || overflow == 1);
    secp256k1_scalar_reduce(r, overflow);
    return overflow;
}

static int secp256k1_scalar_plus(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b) {
    uint64_t t = (uint64_t)a->d[0] + b->d[0];
    r->d[0] = t; t >>= 32;
    t += (uint64_t)a->d[1] + b->d[1];
    r->d[1] = t; t >>= 32;
    t += (uint64_t)a->d[2] + b->d[2];
    r->d[2] = t; t >>= 32;
    t += (uint64_t)a->d[3] + b->d[3];
    r->d[3] = t; t >>= 32;
    t += (uint64_t)a->d[4] + b->d[4];
    r->d[4] = t; t >>= 32;
    t += (uint64_t)a->d[5] + b->d[5];
    r->d[5] = t; t >>= 32;
    t += (uint64_t)a->d[6] + b->d[6];
    r->d[6] = t; t >>= 32;
    t += (uint64_t)a->d[7] + b->d[7];
    r->d[7] = t; t >>= 32;
    return t;
}

static int secp256k1_scalar_minus(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b) {
    uint64_t t = (int64_t)a->d[0] - b->d[0];
    r->d[0] = t; t >>= 32;
    t += (int64_t)a->d[1] - b->d[1];
    r->d[1] = t; t >>= 32;
    t += (int64_t)a->d[2] - b->d[2];
    r->d[2] = t; t >>= 32;
    t += (int64_t)a->d[3] - b->d[3];
    r->d[3] = t; t >>= 32;
    t += (int64_t)a->d[4] - b->d[4];
    r->d[4] = t; t >>= 32;
    t += (int64_t)a->d[5] - b->d[5];
    r->d[5] = t; t >>= 32;
    t += (int64_t)a->d[6] - b->d[6];
    r->d[6] = t; t >>= 32;
    t += (int64_t)a->d[7] - b->d[7];
    r->d[7] = t; t >>= 32;
    return t;
}

static void secp256k1_scalar_cadd_bit(secp256k1_scalar *r, unsigned int bit, int flag) {
    uint64_t t;
    VERIFY_CHECK(bit < 256);
    bit += ((uint32_t) flag - 1) & 0x100;  /* forcing (bit >> 5) > 7 makes this a noop */
    t = (uint64_t)r->d[0] + (((uint32_t)((bit >> 5) == 0)) << (bit & 0x1F));
    r->d[0] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[1] + (((uint32_t)((bit >> 5) == 1)) << (bit & 0x1F));
    r->d[1] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[2] + (((uint32_t)((bit >> 5) == 2)) << (bit & 0x1F));
    r->d[2] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[3] + (((uint32_t)((bit >> 5) == 3)) << (bit & 0x1F));
    r->d[3] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[4] + (((uint32_t)((bit >> 5) == 4)) << (bit & 0x1F));
    r->d[4] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[5] + (((uint32_t)((bit >> 5) == 5)) << (bit & 0x1F));
    r->d[5] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[6] + (((uint32_t)((bit >> 5) == 6)) << (bit & 0x1F));
    r->d[6] = t & 0xFFFFFFFFULL; t >>= 32;
    t += (uint64_t)r->d[7] + (((uint32_t)((bit >> 5) == 7)) << (bit & 0x1F));
    r->d[7] = t & 0xFFFFFFFFULL;
#ifdef VERIFY
    VERIFY_CHECK((t >> 32) == 0);
    VERIFY_CHECK(secp256k1_scalar_check_overflow(r) == 0);
#endif
}

static void secp256k1_scalar_set_b32(secp256k1_scalar *r, const unsigned char *b32, int *overflow) {
    int over;
    r->d[0] = (uint32_t)b32[31] | (uint32_t)b32[30] << 8 | (uint32_t)b32[29] << 16 | (uint32_t)b32[28] << 24;
    r->d[1] = (uint32_t)b32[27] | (uint32_t)b32[26] << 8 | (uint32_t)b32[25] << 16 | (uint32_t)b32[24] << 24;
    r->d[2] = (uint32_t)b32[23] | (uint32_t)b32[22] << 8 | (uint32_t)b32[21] << 16 | (uint32_t)b32[20] << 24;
    r->d[3] = (uint32_t)b32[19] | (uint32_t)b32[18] << 8 | (uint32_t)b32[17] << 16 | (uint32_t)b32[16] << 24;
    r->d[4] = (uint32_t)b32[15] | (uint32_t)b32[14] << 8 | (uint32_t)b32[13] << 16 | (uint32_t)b32[12] << 24;
    r->d[5] = (uint32_t)b32[11] | (uint32_t)b32[10] << 8 | (uint32_t)b32[9] << 16 | (uint32_t)b32[8] << 24;
    r->d[6] = (uint32_t)b32[7] | (uint32_t)b32[6] << 8 | (uint32_t)b32[5] << 16 | (uint32_t)b32[4] << 24;
    r->d[7] = (uint32_t)b32[3] | (uint32_t)b32[2] << 8 | (uint32_t)b32[1] << 16 | (uint32_t)b32[0] << 24;
    over = secp256k1_scalar_reduce(r, secp256k1_scalar_check_overflow(r));
    if (overflow) {
        *overflow = over;
    }
}

static void secp256k1_scalar_get_b32(unsigned char *bin, const secp256k1_scalar* a) {
    bin[0] = a->d[7] >> 24; bin[1] = a->d[7] >> 16; bin[2] = a->d[7] >> 8; bin[3] = a->d[7];
    bin[4] = a->d[6] >> 24; bin[5] = a->d[6] >> 16; bin[6] = a->d[6] >> 8; bin[7] = a->d[6];
    bin[8] = a->d[5] >> 24; bin[9] = a->d[5] >> 16; bin[10] = a->d[5] >> 8; bin[11] = a->d[5];
    bin[12] = a->d[4] >> 24; bin[13] = a->d[4] >> 16; bin[14] = a->d[4] >> 8; bin[15] = a->d[4];
    bin[16] = a->d[3] >> 24; bin[17] = a->d[3] >> 16; bin[18] = a->d[3] >> 8; bin[19] = a->d[3];
    bin[20] = a->d[2] >> 24; bin[21] = a->d[2] >> 16; bin[22] = a->d[2] >> 8; bin[23] = a->d[2];
    bin[24] = a->d[1] >> 24; bin[25] = a->d[1] >> 16; bin[26] = a->d[1] >> 8; bin[27] = a->d[1];
    bin[28] = a->d[0] >> 24; bin[29] = a->d[0] >> 16; bin[30] = a->d[0] >> 8; bin[31] = a->d[0];
}

SECP256K1_INLINE static int secp256k1_scalar_is_zero(const secp256k1_scalar *a) {
    return (a->d[0] | a->d[1] | a->d[2] | a->d[3] | a->d[4] | a->d[5] | a->d[6] | a->d[7]) == 0;
}

static void secp256k1_scalar_negate(secp256k1_scalar *r, const secp256k1_scalar *a) {
    uint32_t nonzero = 0xFFFFFFFFUL * (secp256k1_scalar_is_zero(a) == 0);
    uint64_t t = (uint64_t)(~a->d[0]) + SECP256K1_N_0 + 1;
    r->d[0] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[1]) + SECP256K1_N_1;
    r->d[1] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[2]) + SECP256K1_N_2;
    r->d[2] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[3]) + SECP256K1_N_3;
    r->d[3] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[4]) + SECP256K1_N_4;
    r->d[4] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[5]) + SECP256K1_N_5;
    r->d[5] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[6]) + SECP256K1_N_6;
    r->d[6] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[7]) + SECP256K1_N_7;
    r->d[7] = t & nonzero;
}

static void secp256k1_scalar_neg(secp256k1_scalar *r, const secp256k1_scalar *a) {
    uint32_t nonzero = 0xFFFFFFFFUL * (secp256k1_scalar_is_zero(a) == 0);
    uint64_t t = (uint64_t)(~a->d[0]) + 1;
    r->d[0] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[1]);
    r->d[1] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[2]);
    r->d[2] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[3]);
    r->d[3] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[4]);
    r->d[4] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[5]);
    r->d[5] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[6]);
    r->d[6] = t & nonzero; t >>= 32;
    t += (uint64_t)(~a->d[7]);
    r->d[7] = t & nonzero;
}

SECP256K1_INLINE static int secp256k1_scalar_is_one(const secp256k1_scalar *a) {
    return ((a->d[0] ^ 1) | a->d[1] | a->d[2] | a->d[3] | a->d[4] | a->d[5] | a->d[6] | a->d[7]) == 0;
}

static int secp256k1_scalar_is_high(const secp256k1_scalar *a) {
    int yes = 0;
    int no = 0;
    no |= (a->d[7] < SECP256K1_N_H_7);
    yes |= (a->d[7] > SECP256K1_N_H_7) & ~no;
    no |= (a->d[6] < SECP256K1_N_H_6) & ~yes; /* No need for a > check. */
    no |= (a->d[5] < SECP256K1_N_H_5) & ~yes; /* No need for a > check. */
    no |= (a->d[4] < SECP256K1_N_H_4) & ~yes; /* No need for a > check. */
    no |= (a->d[3] < SECP256K1_N_H_3) & ~yes;
    yes |= (a->d[3] > SECP256K1_N_H_3) & ~no;
    no |= (a->d[2] < SECP256K1_N_H_2) & ~yes;
    yes |= (a->d[2] > SECP256K1_N_H_2) & ~no;
    no |= (a->d[1] < SECP256K1_N_H_1) & ~yes;
    yes |= (a->d[1] > SECP256K1_N_H_1) & ~no;
    yes |= (a->d[0] > SECP256K1_N_H_0) & ~no;
    return yes;
}

static int secp256k1_scalar_cond_negate(secp256k1_scalar *r, int flag) {
    /* If we are flag = 0, mask = 00...00 and this is a no-op;
     * if we are flag = 1, mask = 11...11 and this is identical to secp256k1_scalar_negate */
    uint32_t mask = !flag - 1;
    uint32_t nonzero = 0xFFFFFFFFUL * (secp256k1_scalar_is_zero(r) == 0);
    uint64_t t = (uint64_t)(r->d[0] ^ mask) + ((SECP256K1_N_0 + 1) & mask);
    r->d[0] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[1] ^ mask) + (SECP256K1_N_1 & mask);
    r->d[1] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[2] ^ mask) + (SECP256K1_N_2 & mask);
    r->d[2] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[3] ^ mask) + (SECP256K1_N_3 & mask);
    r->d[3] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[4] ^ mask) + (SECP256K1_N_4 & mask);
    r->d[4] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[5] ^ mask) + (SECP256K1_N_5 & mask);
    r->d[5] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[6] ^ mask) + (SECP256K1_N_6 & mask);
    r->d[6] = t & nonzero; t >>= 32;
    t += (uint64_t)(r->d[7] ^ mask) + (SECP256K1_N_7 & mask);
    r->d[7] = t & nonzero;
    return 2 * (mask == 0) - 1;
}


/* Inspired by the macros in OpenSSL's crypto/bn/asm/x86_64-gcc.c. */

/** Add a*b to the number defined by (c0,c1,c2). c2 must never overflow. */
#define muladd(a,b) { \
    uint32_t tl, th; \
    { \
        uint64_t t = (uint64_t)a * b; \
        th = t >> 32;         /* at most 0xFFFFFFFE */ \
        tl = t; \
    } \
    c0 += tl;                 /* overflow is handled on the next line */ \
    th += (c0 < tl);          /* at most 0xFFFFFFFF */ \
    c1 += th;                 /* overflow is handled on the next line */ \
    c2 += (c1 < th);          /* never overflows by contract (verified in the next line) */ \
    VERIFY_CHECK((c1 >= th) || (c2 != 0)); \
}

/** Add a*b to the number defined by (c0,c1). c1 must never overflow. */
#define muladd_fast(a,b) { \
    uint32_t tl, th; \
    { \
        uint64_t t = (uint64_t)a * b; \
        th = t >> 32;         /* at most 0xFFFFFFFE */ \
        tl = t; \
    } \
    c0 += tl;                 /* overflow is handled on the next line */ \
    th += (c0 < tl);          /* at most 0xFFFFFFFF */ \
    c1 += th;                 /* never overflows by contract (verified in the next line) */ \
    VERIFY_CHECK(c1 >= th); \
}

/** Add a to the number defined by (c0,c1,c2). c2 must never overflow. */
#define sumadd(a) { \
    unsigned int over; \
    c0 += (a);                  /* overflow is handled on the next line */ \
    over = (c0 < (a)); \
    c1 += over;                 /* overflow is handled on the next line */ \
    c2 += (c1 < over);          /* never overflows by contract */ \
}

/** Add a to the number defined by (c0,c1). c1 must never overflow, c2 must be zero. */
#define sumadd_fast(a) { \
    c0 += (a);                 /* overflow is handled on the next line */ \
    c1 += (c0 < (a));          /* never overflows by contract (verified the next line) */ \
    VERIFY_CHECK((c1 != 0) | (c0 >= (a))); \
    VERIFY_CHECK(c2 == 0); \
}

/** Extract the lowest 32 bits of (c0,c1,c2) into n, and left shift the number 32 bits. */
#define extract(n) { \
    (n) = c0; \
    c0 = c1; \
    c1 = c2; \
    c2 = 0; \
}

/** Extract the lowest 32 bits of (c0,c1,c2) into n, and left shift the number 32 bits. c2 is required to be zero. */
#define extract_fast(n) { \
    (n) = c0; \
    c0 = c1; \
    c1 = 0; \
    VERIFY_CHECK(c2 == 0); \
}

static void secp256k1_scalar_reduce_512(secp256k1_scalar *r, const uint32_t *l) {
    uint64_t c;
    uint32_t n0 = l[8], n1 = l[9], n2 = l[10], n3 = l[11], n4 = l[12], n5 = l[13], n6 = l[14], n7 = l[15];
    uint32_t m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12;
    uint32_t p0, p1, p2, p3, p4, p5, p6, p7, p8;

    /* 96 bit accumulator. */
    uint32_t c0, c1, c2;

    /* Reduce 512 bits into 385. */
    /* m[0..12] = l[0..7] + n[0..7] * SECP256K1_N_C. */
    c0 = l[0]; c1 = 0; c2 = 0;
    muladd_fast(n0, SECP256K1_N_C_0);
    extract_fast(m0);
    sumadd_fast(l[1]);
    muladd(n1, SECP256K1_N_C_0);
    muladd(n0, SECP256K1_N_C_1);
    extract(m1);
    sumadd(l[2]);
    muladd(n2, SECP256K1_N_C_0);
    muladd(n1, SECP256K1_N_C_1);
    muladd(n0, SECP256K1_N_C_2);
    extract(m2);
    sumadd(l[3]);
    muladd(n3, SECP256K1_N_C_0);
    muladd(n2, SECP256K1_N_C_1);
    muladd(n1, SECP256K1_N_C_2);
    muladd(n0, SECP256K1_N_C_3);
    extract(m3);
    sumadd(l[4]);
    muladd(n4, SECP256K1_N_C_0);
    muladd(n3, SECP256K1_N_C_1);
    muladd(n2, SECP256K1_N_C_2);
    muladd(n1, SECP256K1_N_C_3);
    sumadd(n0);
    extract(m4);
    sumadd(l[5]);
    muladd(n5, SECP256K1_N_C_0);
    muladd(n4, SECP256K1_N_C_1);
    muladd(n3, SECP256K1_N_C_2);
    muladd(n2, SECP256K1_N_C_3);
    sumadd(n1);
    extract(m5);
    sumadd(l[6]);
    muladd(n6, SECP256K1_N_C_0);
    muladd(n5, SECP256K1_N_C_1);
    muladd(n4, SECP256K1_N_C_2);
    muladd(n3, SECP256K1_N_C_3);
    sumadd(n2);
    extract(m6);
    sumadd(l[7]);
    muladd(n7, SECP256K1_N_C_0);
    muladd(n6, SECP256K1_N_C_1);
    muladd(n5, SECP256K1_N_C_2);
    muladd(n4, SECP256K1_N_C_3);
    sumadd(n3);
    extract(m7);
    muladd(n7, SECP256K1_N_C_1);
    muladd(n6, SECP256K1_N_C_2);
    muladd(n5, SECP256K1_N_C_3);
    sumadd(n4);
    extract(m8);
    muladd(n7, SECP256K1_N_C_2);
    muladd(n6, SECP256K1_N_C_3);
    sumadd(n5);
    extract(m9);
    muladd(n7, SECP256K1_N_C_3);
    sumadd(n6);
    extract(m10);
    sumadd_fast(n7);
    extract_fast(m11);
    VERIFY_CHECK(c0 <= 1);
    m12 = c0;

    /* Reduce 385 bits into 258. */
    /* p[0..8] = m[0..7] + m[8..12] * SECP256K1_N_C. */
    c0 = m0; c1 = 0; c2 = 0;
    muladd_fast(m8, SECP256K1_N_C_0);
    extract_fast(p0);
    sumadd_fast(m1);
    muladd(m9, SECP256K1_N_C_0);
    muladd(m8, SECP256K1_N_C_1);
    extract(p1);
    sumadd(m2);
    muladd(m10, SECP256K1_N_C_0);
    muladd(m9, SECP256K1_N_C_1);
    muladd(m8, SECP256K1_N_C_2);
    extract(p2);
    sumadd(m3);
    muladd(m11, SECP256K1_N_C_0);
    muladd(m10, SECP256K1_N_C_1);
    muladd(m9, SECP256K1_N_C_2);
    muladd(m8, SECP256K1_N_C_3);
    extract(p3);
    sumadd(m4);
    muladd(m12, SECP256K1_N_C_0);
    muladd(m11, SECP256K1_N_C_1);
    muladd(m10, SECP256K1_N_C_2);
    muladd(m9, SECP256K1_N_C_3);
    sumadd(m8);
    extract(p4);
    sumadd(m5);
    muladd(m12, SECP256K1_N_C_1);
    muladd(m11, SECP256K1_N_C_2);
    muladd(m10, SECP256K1_N_C_3);
    sumadd(m9);
    extract(p5);
    sumadd(m6);
    muladd(m12, SECP256K1_N_C_2);
    muladd(m11, SECP256K1_N_C_3);
    sumadd(m10);
    extract(p6);
    sumadd_fast(m7);
    muladd_fast(m12, SECP256K1_N_C_3);
    sumadd_fast(m11);
    extract_fast(p7);
    p8 = c0 + m12;
    VERIFY_CHECK(p8 <= 2);

    /* Reduce 258 bits into 256. */
    /* r[0..7] = p[0..7] + p[8] * SECP256K1_N_C. */
    c = p0 + (uint64_t)SECP256K1_N_C_0 * p8;
    r->d[0] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p1 + (uint64_t)SECP256K1_N_C_1 * p8;
    r->d[1] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p2 + (uint64_t)SECP256K1_N_C_2 * p8;
    r->d[2] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p3 + (uint64_t)SECP256K1_N_C_3 * p8;
    r->d[3] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p4 + (uint64_t)p8;
    r->d[4] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p5;
    r->d[5] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p6;
    r->d[6] = c & 0xFFFFFFFFUL; c >>= 32;
    c += p7;
    r->d[7] = c & 0xFFFFFFFFUL; c >>= 32;

    /* Final reduction of r. */
    secp256k1_scalar_reduce(r, c + secp256k1_scalar_check_overflow(r));
}

static void secp256k1_scalar_mul_512(uint32_t *l, const secp256k1_scalar *a, const secp256k1_scalar *b) {
    /* 96 bit accumulator. */
    uint32_t c0 = 0, c1 = 0, c2 = 0;

    /* l[0..15] = a[0..7] * b[0..7]. */
    muladd_fast(a->d[0], b->d[0]);
    extract_fast(l[0]);
    muladd(a->d[0], b->d[1]);
    muladd(a->d[1], b->d[0]);
    extract(l[1]);
    muladd(a->d[0], b->d[2]);
    muladd(a->d[1], b->d[1]);
    muladd(a->d[2], b->d[0]);
    extract(l[2]);
    muladd(a->d[0], b->d[3]);
    muladd(a->d[1], b->d[2]);
    muladd(a->d[2], b->d[1]);
    muladd(a->d[3], b->d[0]);
    extract(l[3]);
    muladd(a->d[0], b->d[4]);
    muladd(a->d[1], b->d[3]);
    muladd(a->d[2], b->d[2]);
    muladd(a->d[3], b->d[1]);
    muladd(a->d[4], b->d[0]);
    extract(l[4]);
    muladd(a->d[0], b->d[5]);
    muladd(a->d[1], b->d[4]);
    muladd(a->d[2], b->d[3]);
    muladd(a->d[3], b->d[2]);
    muladd(a->d[4], b->d[1]);
    muladd(a->d[5], b->d[0]);
    extract(l[5]);
    muladd(a->d[0], b->d[6]);
    muladd(a->d[1], b->d[5]);
    muladd(a->d[2], b->d[4]);
    muladd(a->d[3], b->d[3]);
    muladd(a->d[4], b->d[2]);
    muladd(a->d[5], b->d[1]);
    muladd(a->d[6], b->d[0]);
    extract(l[6]);
    muladd(a->d[0], b->d[7]);
    muladd(a->d[1], b->d[6]);
    muladd(a->d[2], b->d[5]);
    muladd(a->d[3], b->d[4]);
    muladd(a->d[4], b->d[3]);
    muladd(a->d[5], b->d[2]);
    muladd(a->d[6], b->d[1]);
    muladd(a->d[7], b->d[0]);
    extract(l[7]);
    muladd(a->d[1], b->d[7]);
    muladd(a->d[2], b->d[6]);
    muladd(a->d[3], b->d[5]);
    muladd(a->d[4], b->d[4]);
    muladd(a->d[5], b->d[3]);
    muladd(a->d[6], b->d[2]);
    muladd(a->d[7], b->d[1]);
    extract(l[8]);
    muladd(a->d[2], b->d[7]);
    muladd(a->d[3], b->d[6]);
    muladd(a->d[4], b->d[5]);
    muladd(a->d[5], b->d[4]);
    muladd(a->d[6], b->d[3]);
    muladd(a->d[7], b->d[2]);
    extract(l[9]);
    muladd(a->d[3], b->d[7]);
    muladd(a->d[4], b->d[6]);
    muladd(a->d[5], b->d[5]);
    muladd(a->d[6], b->d[4]);
    muladd(a->d[7], b->d[3]);
    extract(l[10]);
    muladd(a->d[4], b->d[7]);
    muladd(a->d[5], b->d[6]);
    muladd(a->d[6], b->d[5]);
    muladd(a->d[7], b->d[4]);
    extract(l[11]);
    muladd(a->d[5], b->d[7]);
    muladd(a->d[6], b->d[6]);
    muladd(a->d[7], b->d[5]);
    extract(l[12]);
    muladd(a->d[6], b->d[7]);
    muladd(a->d[7], b->d[6]);
    extract(l[13]);
    muladd_fast(a->d[7], b->d[7]);
    extract_fast(l[14]);
    VERIFY_CHECK(c1 == 0);
    l[15] = c0;
}

#undef sumadd
#undef sumadd_fast
#undef muladd
#undef muladd_fast
#undef extract
#undef extract_fast

static void secp256k1_scalar_mul(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b) {
    uint32_t l[16];
    secp256k1_scalar_mul_512(l, a, b);
    secp256k1_scalar_reduce_512(r, l);
}

static int secp256k1_scalar_shr_int(secp256k1_scalar *r, int n) {
    int ret;
    VERIFY_CHECK(n > 0);
    VERIFY_CHECK(n < 16);
    ret = r->d[0] & ((1 << n) - 1);
    r->d[0] = (r->d[0] >> n) | (r->d[1] << (32 - n));
    r->d[1] = (r->d[1] >> n) | (r->d[2] << (32 - n));
    r->d[2] = (r->d[2] >> n) | (r->d[3] << (32 - n));
    r->d[3] = (r->d[3] >> n) | (r->d[4] << (32 - n));
    r->d[4] = (r->d[4] >> n) | (r->d[5] << (32 - n));
    r->d[5] = (r->d[5] >> n) | (r->d[6] << (32 - n));
    r->d[6] = (r->d[6] >> n) | (r->d[7] << (32 - n));
    r->d[7] = (r->d[7] >> n);
    return ret;
}

static int secp256k1_scalar_shl_int(secp256k1_scalar *r, int n) {
    int ret;
    VERIFY_CHECK(n > 0);
    VERIFY_CHECK(n < 16);
    ret = r->d[7] >> (32 - n);
    r->d[7] = (r->d[7] << n) | (r->d[6] >> (32 - n));
    r->d[6] = (r->d[6] << n) | (r->d[5] >> (32 - n));
    r->d[5] = (r->d[5] << n) | (r->d[4] >> (32 - n));
    r->d[4] = (r->d[4] << n) | (r->d[3] >> (32 - n));
    r->d[3] = (r->d[3] << n) | (r->d[2] >> (32 - n));
    r->d[2] = (r->d[2] << n) | (r->d[1] >> (32 - n));
    r->d[1] = (r->d[1] << n) | (r->d[0] >> (32 - n));
    r->d[0] = (r->d[0] << n);
    return ret;
}

static void secp256k1_scalar_split_128(secp256k1_scalar *r1, secp256k1_scalar *r2, const secp256k1_scalar *k) {
    r1->d[0] = k->d[0];
    r1->d[1] = k->d[1];
    r1->d[2] = k->d[2];
    r1->d[3] = k->d[3];
    r1->d[4] = 0;
    r1->d[5] = 0;
    r1->d[6] = 0;
    r1->d[7] = 0;
    r2->d[0] = k->d[4];
    r2->d[1] = k->d[5];
    r2->d[2] = k->d[6];
    r2->d[3] = k->d[7];
    r2->d[4] = 0;
    r2->d[5] = 0;
    r2->d[6] = 0;
    r2->d[7] = 0;
}

SECP256K1_INLINE static int secp256k1_scalar_eq(const secp256k1_scalar *a, const secp256k1_scalar *b) {
    return ((a->d[0] ^ b->d[0]) | (a->d[1] ^ b->d[1]) | (a->d[2] ^ b->d[2]) | (a->d[3] ^ b->d[3]) | (a->d[4] ^ b->d[4]) | (a->d[5] ^ b->d[5]) | (a->d[6] ^ b->d[6]) | (a->d[7] ^ b->d[7])) == 0;
}

SECP256K1_INLINE static int secp256k1_scalar_cmp(const secp256k1_scalar *a, const secp256k1_scalar *b) {
    int c = (a->d[7] > b->d[7]) - (a->d[7] < b->d[7]);
    c = (c * 2) + (a->d[6] > b->d[6]) - (a->d[6] < b->d[6]);
    c = (c * 2) + (a->d[5] > b->d[5]) - (a->d[5] < b->d[5]);
    c = (c * 2) + (a->d[4] > b->d[4]) - (a->d[4] < b->d[4]);
    c = (c * 2) + (a->d[3] > b->d[3]) - (a->d[3] < b->d[3]);
    c = (c * 2) + (a->d[2] > b->d[2]) - (a->d[2] < b->d[2]);
    c = (c * 2) + (a->d[1] > b->d[1]) - (a->d[1] < b->d[1]);
    c = (c * 2) + (a->d[0] > b->d[0]) - (a->d[0] < b->d[0]);
    return c;
}

SECP256K1_INLINE static int secp256k1_scalar_cmp_var(const secp256k1_scalar *a, const secp256k1_scalar *b) {
    int i;
    for (i = 7; i >= 0; i--) {
        if (a->d[i] != b->d[i]) {
            return (a->d[i] > b->d[i]) - (a->d[i] < b->d[i]);
        }
    }
    return 0;
}

SECP256K1_INLINE static int _secp256k1_scalar_msb_32(unsigned long a) {
    int shift, r;
    if (v == 0) return 0;
    r = (v > 0xFFFF) << 4; v >>= r;
    shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
    return (r | (v >> 1)) + 1;
}

SECP256K1_INLINE static int secp256k1_scalar_msb(const secp256k1_scalar *a) {
    int i;
    for (i = 7; i >= 0; i--) {
        if (a->d[i] != 0) {
            return i*32 + _secp256k1_scalar_msb_32(a->d[i]);
        }
    }
    return 0;
}

SECP256K1_INLINE static int secp256k1_scalar_msb_neg(const secp256k1_scalar *a) {
    int r = 0, f = 0, i;
    for (i = 7; i >= 0; i--) {
        f &= (a->d[i] == 0);
        if (a->d[i] != 0xFFFFFFFFULL) {
            r = i*32 + _secp256k1_scalar_msb_32(-a->d[i]);
            f = ((~a->d[i]) & ((1 << (r & 31)) - 1)) == 0;
        }
    }
    return r + f;
}

SECP256K1_INLINE static void secp256k1_scalar_mul_shift_var(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b, unsigned int shift) {
    uint32_t l[16];
    unsigned int shiftlimbs;
    unsigned int shiftlow;
    unsigned int shifthigh;
    VERIFY_CHECK(shift >= 256);
    secp256k1_scalar_mul_512(l, a, b);
    shiftlimbs = shift >> 5;
    shiftlow = shift & 0x1F;
    shifthigh = 32 - shiftlow;
    r->d[0] = shift < 512 ? (l[0 + shiftlimbs] >> shiftlow | (shift < 480 && shiftlow ? (l[1 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[1] = shift < 480 ? (l[1 + shiftlimbs] >> shiftlow | (shift < 448 && shiftlow ? (l[2 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[2] = shift < 448 ? (l[2 + shiftlimbs] >> shiftlow | (shift < 416 && shiftlow ? (l[3 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[3] = shift < 416 ? (l[3 + shiftlimbs] >> shiftlow | (shift < 384 && shiftlow ? (l[4 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[4] = shift < 384 ? (l[4 + shiftlimbs] >> shiftlow | (shift < 352 && shiftlow ? (l[5 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[5] = shift < 352 ? (l[5 + shiftlimbs] >> shiftlow | (shift < 320 && shiftlow ? (l[6 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[6] = shift < 320 ? (l[6 + shiftlimbs] >> shiftlow | (shift < 288 && shiftlow ? (l[7 + shiftlimbs] << shifthigh) : 0)) : 0;
    r->d[7] = shift < 288 ? (l[7 + shiftlimbs] >> shiftlow)  : 0;
    secp256k1_scalar_cadd_bit(r, 0, (l[(shift - 1) >> 5] >> ((shift - 1) & 0x1f)) & 1);
}

static SECP256K1_INLINE void secp256k1_scalar_cmov(secp256k1_scalar *r, const secp256k1_scalar *a, int flag) {
    uint32_t mask0, mask1;
    VG_CHECK_VERIFY(r->d, sizeof(r->d));
    mask0 = flag + ~((uint32_t)0);
    mask1 = ~mask0;
    r->d[0] = (r->d[0] & mask0) | (a->d[0] & mask1);
    r->d[1] = (r->d[1] & mask0) | (a->d[1] & mask1);
    r->d[2] = (r->d[2] & mask0) | (a->d[2] & mask1);
    r->d[3] = (r->d[3] & mask0) | (a->d[3] & mask1);
    r->d[4] = (r->d[4] & mask0) | (a->d[4] & mask1);
    r->d[5] = (r->d[5] & mask0) | (a->d[5] & mask1);
    r->d[6] = (r->d[6] & mask0) | (a->d[6] & mask1);
    r->d[7] = (r->d[7] & mask0) | (a->d[7] & mask1);
}

static void secp256k1_scalar_from_signed30(secp256k1_scalar *r, const secp256k1_modinv32_signed30 *a) {
    const uint32_t a0 = a->v[0], a1 = a->v[1], a2 = a->v[2], a3 = a->v[3], a4 = a->v[4],
                   a5 = a->v[5], a6 = a->v[6], a7 = a->v[7], a8 = a->v[8];

    /* The output from secp256k1_modinv32{_var} should be normalized to range [0,modulus), and
     * have limbs in [0,2^30). The modulus is < 2^256, so the top limb must be below 2^(256-30*8).
     */
    VERIFY_CHECK(a0 >> 30 == 0);
    VERIFY_CHECK(a1 >> 30 == 0);
    VERIFY_CHECK(a2 >> 30 == 0);
    VERIFY_CHECK(a3 >> 30 == 0);
    VERIFY_CHECK(a4 >> 30 == 0);
    VERIFY_CHECK(a5 >> 30 == 0);
    VERIFY_CHECK(a6 >> 30 == 0);
    VERIFY_CHECK(a7 >> 30 == 0);
    VERIFY_CHECK(a8 >> 16 == 0);

    r->d[0] = a0       | a1 << 30;
    r->d[1] = a1 >>  2 | a2 << 28;
    r->d[2] = a2 >>  4 | a3 << 26;
    r->d[3] = a3 >>  6 | a4 << 24;
    r->d[4] = a4 >>  8 | a5 << 22;
    r->d[5] = a5 >> 10 | a6 << 20;
    r->d[6] = a6 >> 12 | a7 << 18;
    r->d[7] = a7 >> 14 | a8 << 16;

#ifdef VERIFY
    VERIFY_CHECK(secp256k1_scalar_check_overflow(r) == 0);
#endif
}

static void secp256k1_scalar_to_signed30(secp256k1_modinv32_signed30 *r, const secp256k1_scalar *a) {
    const uint32_t M30 = UINT32_MAX >> 2;
    const uint32_t a0 = a->d[0], a1 = a->d[1], a2 = a->d[2], a3 = a->d[3],
                   a4 = a->d[4], a5 = a->d[5], a6 = a->d[6], a7 = a->d[7];

#ifdef VERIFY
    VERIFY_CHECK(secp256k1_scalar_check_overflow(a) == 0);
#endif

    r->v[0] =  a0                   & M30;
    r->v[1] = (a0 >> 30 | a1 <<  2) & M30;
    r->v[2] = (a1 >> 28 | a2 <<  4) & M30;
    r->v[3] = (a2 >> 26 | a3 <<  6) & M30;
    r->v[4] = (a3 >> 24 | a4 <<  8) & M30;
    r->v[5] = (a4 >> 22 | a5 << 10) & M30;
    r->v[6] = (a5 >> 20 | a6 << 12) & M30;
    r->v[7] = (a6 >> 18 | a7 << 14) & M30;
    r->v[8] =  a7 >> 16;
}

static const secp256k1_modinv32_modinfo secp256k1_const_modinfo_scalar = {
    {{0x10364141L, 0x3F497A33L, 0x348A03BBL, 0x2BB739ABL, -0x146L, 0, 0, 0, 65536}},
    0x2A774EC1L
};

static const secp256k1_scalar secp256k1_const_mod_scalar = SECP256K1_SCALAR_CONST(
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFC2F
);

static void secp256k1_scalar_inverse(secp256k1_scalar *r, const secp256k1_scalar *x) {
    secp256k1_modinv32_signed30 s;
#ifdef VERIFY
    int zero_in = secp256k1_scalar_is_zero(x);
#endif
    secp256k1_scalar_to_signed30(&s, x);
    secp256k1_modinv32(&s, &secp256k1_const_modinfo_scalar);
    secp256k1_scalar_from_signed30(r, &s);

#ifdef VERIFY
    VERIFY_CHECK(secp256k1_scalar_is_zero(r) == zero_in);
#endif
}

static void secp256k1_scalar_inverse_var(secp256k1_scalar *r, const secp256k1_scalar *x) {
#ifdef VERIFY
    int zero_in = secp256k1_scalar_is_zero(x);
#endif
    secp256k1_modinv64_scalar(r, x, &secp256k1_const_mod_scalar);

#ifdef VERIFY
    VERIFY_CHECK(secp256k1_scalar_is_zero(r) == zero_in);
#endif
}

SECP256K1_INLINE static int secp256k1_scalar_is_even(const secp256k1_scalar *a) {
    return !(a->d[0] & 1);
}

static void secp256k1_scalar_print(const secp256k1_scalar *a) {
    printf("%08llx%08llx%08llx%08llx%08llx%08llx%08llx%08llx", a->d[7], a->d[6], a->d[5], a->d[4], a->d[3], a->d[2], a->d[1], a->d[0]);
}

#endif /* SECP256K1_SCALAR_REPR_IMPL_H */
