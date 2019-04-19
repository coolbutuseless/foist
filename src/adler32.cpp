
#include "Rcpp.h"

using namespace Rcpp;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// adler32.c -- compute the Adler-32 checksum of a data stream
// Copyright (C) 1995-2011, 2016 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h
//
// Source from: https://raw.githubusercontent.com/madler/zlib/master/adler32.c
//
// modified by mikefc@coolbutuseless.com for inclusion in 'foist'
//  - removed small writes as data will always be in big image slices
//  - removed check for NULL buffer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define BASE 65521U  /* largest prime smaller than 65536 */
#define NMAX 5552    /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);


#define MOD(a)   a %= BASE
#define MOD28(a) a %= BASE
#define MOD63(a) a %= BASE


uint32_t update_adler32(uint32_t adler, const unsigned char *buf, size_t len) {
    unsigned long sum2;
    unsigned int n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* do length NMAX blocks -- requires just one modulo operation */
    while (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        do {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } while (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    if (len) {                  /* avoid modulos if none remaining */
        while (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    return adler | (sum2 << 16);
}
