/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <algorithm>

#include "DrmPlayReady.h"
#include "string.h"

static const uint8_t kPlayReadySystemId[] = {
    0x9a, 0x04, 0xf0, 0x79, 0x98, 0x40, 0x42, 0x86,
    0xab, 0x92, 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95};

static const uint16_t kWRMHEADERRecord = 0x1;

static void swapBytes(void *data, uint8_t size)
{
    uint8_t *bytes = static_cast<uint8_t *>(data);

    uint8_t tmp;
    for (uint8_t i = 0, j = size-1; i < j; ++i, --j) {
        tmp = bytes[i];
        bytes[i] = bytes[j];
        bytes[j] = tmp;
    }
}

// copy from trellis/media/player/source/mediasource/chromium/third_party/modp_b64/modp_b64.h
/* special decode tables for little endian (intel) cpus */

static const uint32_t d0[256] = {
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x000000f8, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x000000fc,
0x000000d0, 0x000000d4, 0x000000d8, 0x000000dc, 0x000000e0, 0x000000e4,
0x000000e8, 0x000000ec, 0x000000f0, 0x000000f4, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
0x00000004, 0x00000008, 0x0000000c, 0x00000010, 0x00000014, 0x00000018,
0x0000001c, 0x00000020, 0x00000024, 0x00000028, 0x0000002c, 0x00000030,
0x00000034, 0x00000038, 0x0000003c, 0x00000040, 0x00000044, 0x00000048,
0x0000004c, 0x00000050, 0x00000054, 0x00000058, 0x0000005c, 0x00000060,
0x00000064, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x00000068, 0x0000006c, 0x00000070, 0x00000074, 0x00000078,
0x0000007c, 0x00000080, 0x00000084, 0x00000088, 0x0000008c, 0x00000090,
0x00000094, 0x00000098, 0x0000009c, 0x000000a0, 0x000000a4, 0x000000a8,
0x000000ac, 0x000000b0, 0x000000b4, 0x000000b8, 0x000000bc, 0x000000c0,
0x000000c4, 0x000000c8, 0x000000cc, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const uint32_t d1[256] = {
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x0000e003, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x0000f003,
0x00004003, 0x00005003, 0x00006003, 0x00007003, 0x00008003, 0x00009003,
0x0000a003, 0x0000b003, 0x0000c003, 0x0000d003, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
0x00001000, 0x00002000, 0x00003000, 0x00004000, 0x00005000, 0x00006000,
0x00007000, 0x00008000, 0x00009000, 0x0000a000, 0x0000b000, 0x0000c000,
0x0000d000, 0x0000e000, 0x0000f000, 0x00000001, 0x00001001, 0x00002001,
0x00003001, 0x00004001, 0x00005001, 0x00006001, 0x00007001, 0x00008001,
0x00009001, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x0000a001, 0x0000b001, 0x0000c001, 0x0000d001, 0x0000e001,
0x0000f001, 0x00000002, 0x00001002, 0x00002002, 0x00003002, 0x00004002,
0x00005002, 0x00006002, 0x00007002, 0x00008002, 0x00009002, 0x0000a002,
0x0000b002, 0x0000c002, 0x0000d002, 0x0000e002, 0x0000f002, 0x00000003,
0x00001003, 0x00002003, 0x00003003, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const uint32_t d2[256] = {
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x00800f00, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00c00f00,
0x00000d00, 0x00400d00, 0x00800d00, 0x00c00d00, 0x00000e00, 0x00400e00,
0x00800e00, 0x00c00e00, 0x00000f00, 0x00400f00, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
0x00400000, 0x00800000, 0x00c00000, 0x00000100, 0x00400100, 0x00800100,
0x00c00100, 0x00000200, 0x00400200, 0x00800200, 0x00c00200, 0x00000300,
0x00400300, 0x00800300, 0x00c00300, 0x00000400, 0x00400400, 0x00800400,
0x00c00400, 0x00000500, 0x00400500, 0x00800500, 0x00c00500, 0x00000600,
0x00400600, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x00800600, 0x00c00600, 0x00000700, 0x00400700, 0x00800700,
0x00c00700, 0x00000800, 0x00400800, 0x00800800, 0x00c00800, 0x00000900,
0x00400900, 0x00800900, 0x00c00900, 0x00000a00, 0x00400a00, 0x00800a00,
0x00c00a00, 0x00000b00, 0x00400b00, 0x00800b00, 0x00c00b00, 0x00000c00,
0x00400c00, 0x00800c00, 0x00c00c00, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const uint32_t d3[256] = {
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x003e0000, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x003f0000,
0x00340000, 0x00350000, 0x00360000, 0x00370000, 0x00380000, 0x00390000,
0x003a0000, 0x003b0000, 0x003c0000, 0x003d0000, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
0x00010000, 0x00020000, 0x00030000, 0x00040000, 0x00050000, 0x00060000,
0x00070000, 0x00080000, 0x00090000, 0x000a0000, 0x000b0000, 0x000c0000,
0x000d0000, 0x000e0000, 0x000f0000, 0x00100000, 0x00110000, 0x00120000,
0x00130000, 0x00140000, 0x00150000, 0x00160000, 0x00170000, 0x00180000,
0x00190000, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x001a0000, 0x001b0000, 0x001c0000, 0x001d0000, 0x001e0000,
0x001f0000, 0x00200000, 0x00210000, 0x00220000, 0x00230000, 0x00240000,
0x00250000, 0x00260000, 0x00270000, 0x00280000, 0x00290000, 0x002a0000,
0x002b0000, 0x002c0000, 0x002d0000, 0x002e0000, 0x002f0000, 0x00300000,
0x00310000, 0x00320000, 0x00330000, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

/**
 * Given a base64 string of length len,
 *   this returns the amount of memory required for output string
 *  It maybe be more than the actual number of bytes written.
 * NOTE: remember this is integer math
 * this allocates a bit more memory than traditional versions of b64
 * decode  4 chars turn into 3 bytes
 * floor[len * 3/4] + 2
 */
#define modp_b64_decode_len(A) (A / 4 * 3 + 2)

// copy from trellis/media/player/source/mediasource/chromium/third_party/modp_b64/modp_b64_data.h
#define CHARPAD '='

// copy from trellis/media/player/source/mediasource/chromium/third_party/modp_b64/modp_b64.cc
#define BADCHAR 0x01FFFFFF

/**
 * you can control if we use padding by commenting out this
 * next line.  However, I highly recommend you use padding and not
 * using it should only be for compatability with a 3rd party.
 * Also, 'no padding' is not tested!
 */
#define DOPAD 1

/*
 * if we aren't doing padding
 * set the pad character to NULL
 */
#ifndef DOPAD
#undef CHARPAD
#define CHARPAD '\0'
#endif

/* LITTLE  ENDIAN -- INTEL AND FRIENDS */
static int modp_b64_decode(char* dest, const char* src, int len)
{
    if (len == 0) return 0;

#ifdef DOPAD
    /*
     * if padding is used, then the message must be at least
     * 4 chars and be a multiple of 4
     */
    if (len < 4 || (len % 4 != 0)) return -1; /* error */
    /* there can be at most 2 pad chars at the end */
    if (src[len-1] == CHARPAD) {
        len--;
        if (src[len -1] == CHARPAD) {
            len--;
        }
    }
#endif

    int i;
    int leftover = len % 4;
    int chunks = (leftover == 0) ? len / 4 - 1 : len /4;

    uint8_t* p = (uint8_t*)dest;
    uint32_t x = 0;
    uint32_t* destInt = (uint32_t*) p;
    uint32_t* srcInt = (uint32_t*) src;
    uint32_t y = *srcInt++;
    for (i = 0; i < chunks; ++i) {
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *destInt = x;
        p += 3;
        destInt = (uint32_t*)p;
        y = *srcInt++;}


    switch (leftover) {
    case 0:
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *p++ =  ((uint8_t*)(&x))[0];
        *p++ =  ((uint8_t*)(&x))[1];
        *p =    ((uint8_t*)(&x))[2];
        return (chunks+1)*3;
        break;
    case 1:  /* with padding this is an impossible case */
        x = d0[y & 0xff];
        *p = *((uint8_t*)(&x)); // i.e. first char/byte in int
        break;
    case 2: // * case 2, 1  output byte */
        x = d0[y & 0xff] | d1[y >> 8 & 0xff];
        *p = *((uint8_t*)(&x)); // i.e. first char
        break;
    default: /* case 3, 2 output bytes */
        x = d0[y & 0xff] |
            d1[y >> 8 & 0xff ] |
            d2[y >> 16 & 0xff];  /* 0x3c */
        *p++ =  ((uint8_t*)(&x))[0];
        *p =  ((uint8_t*)(&x))[1];
        break;
    }

    if (x >= BADCHAR) return -1;

    return 3*chunks + (6*leftover)/8;
}

// copy from trellis/media/player/source/mediasource/chromium/base/base64.cc
static bool Base64Decode(const std::string& input, std::string* output)
{
  std::string temp;
  temp.resize(modp_b64_decode_len(input.size()));

  // does not null terminate result since result is binary data!
  int input_size = static_cast<int>(input.size());
  int output_size = modp_b64_decode(&(temp[0]), input.data(), input_size);
  if (output_size < 0)
    return false;

  temp.resize(output_size);
  output->swap(temp);
  return true;
}

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(DrmPlayReady);

DrmPlayReady::DrmPlayReady(MediaDrmAdaptor *mediaDrmAdaptor)
    : Drm(mediaDrmAdaptor)
{
}

DrmPlayReady::~DrmPlayReady()
{
}

bool DrmPlayReady::parseInitData(
        const std::string& initData,
        std::string& wrmheader,
        std::string& pssh)
{
    wrmheader.clear();
    pssh.clear();

    uint32_t initDataSize = initData.size();
    const char *data = initData.data();
    uint32_t pos = 0;

    // one PlayReady PSSH consists of:
    // 4 byte size of the PSSH atom, inclusive
    // "pssh"
    // 4 byte flags, value 0
    // 16 byte system id
    // 4 byte size of PlayReady data, exclusive
    // !!!!the following fields are all little-endian
    // 4 byte size of PlayReady data, inclusive
    // 2 byte PlayReady record count, followed by a sequence of records:
    //   2 byte type of data (1: WRMHEADER, 3: embedded license store),
    //   2 byte data length, exclusive
    //   finally, the blob of data
    while (pos < initDataSize) {
        // size of PSSH atom, used for skipping
        uint32_t size;

        // size (4 bytes)
        memcpy(&size, data+pos, 4);
        pos += 4;
        swapBytes(static_cast<void *>(&size), 4);

        // "pssh" (4 bytes)
        uint8_t psshStr[4];
        memcpy(psshStr, data+pos, 4);
        pos += 4;

        if (memcmp(&psshStr[0], "pssh", 4)) {
            BME_DEBUG_ERROR(("not pssh."));
            return false;
        }

        // flags (4 bytes)
        uint32_t flags;
        memcpy(&flags, data+pos, 4);
        pos += 4;
        if (flags != 0) {
            BME_DEBUG_ERROR(("invalid flag"));
            return false;
        }

        // system id (16 bytes)
        uint8_t systemId[16];
        memcpy(&systemId, data+pos, 16);
        pos += 16;
        if (memcmp(&systemId[0], kPlayReadySystemId, 16)) {
            // skip the remaining contents of the atom,
            // after size field, atom name, flags and system id
            uint32_t bytesToSkip = size - 4 - 4 - 4 - 16;
            pos += bytesToSkip;
            BME_DEBUG_TRACE(("playready system id does not match."));
            continue;
        }

        // size of PlayReady blob, ignored (4 bytes)
        pos += 4;

        uint32_t psshSize;
        memcpy(&psshSize, data+pos, 4);
        pos += 4;
        pssh.assign(data + pos - 4, psshSize);

        uint16_t recordCount;
        memcpy(&recordCount, data+pos, 2);
        pos += 2;

        uint16_t recordNum;
        for (recordNum = 0; recordNum < recordCount; ++recordNum) {
            // record type
            uint16_t recordType;
            memcpy(&recordType, data+pos, 2);
            pos += 2;

            // record length
            uint16_t recordLength;
            memcpy(&recordLength, data+pos, 2);
            pos += 2;

            // the WRMHEADER itself
            wrmheader.assign(data+pos, recordLength);
            pos += recordLength;

            // bail when we found the first record of the correct type
            if (recordType == kWRMHEADERRecord) {
                return true;
            }
        }
    }

    // we did not find a matching record
    BME_DEBUG_ERROR(("cannot find a matching record"));
    return false;
}

bool DrmPlayReady::playreadyExtractKeyId(
        const std::string& wrmheader,
        std::string& keyId)
{
    // WRMHEADER is in little UTF-16. In practice, it is a sequence of pairs of
    // one ASCII character and one NUL byte, like "<\0K\0I\0D\0>\0". Convert it to
    // just ASCII characters before doing anything
    std::string init_data_ascii;
    init_data_ascii.reserve(wrmheader.size());

    for (size_t i = 0; i < wrmheader.size(); i++) {
        if (i % 2) {
            if (wrmheader[i] != 0)
                return false;
        } else {
            init_data_ascii.push_back(wrmheader[i]);
        }
    }

    // TODO(ppergame): parse the XML instead of doing this
    static const std::string kKIDOpen = "<KID>";
    static const std::string kKIDClose = "</KID>";
    size_t start = init_data_ascii.find(kKIDOpen);

    if (start == std::string::npos) {
        return false;
    }

    start += kKIDOpen.size();
    size_t end = init_data_ascii.find(kKIDClose, start);

    if (end == std::string::npos) {
        return false;
    }

    std::string base64_key_id(init_data_ascii, start, end - start);
    std::string out;

    if (!Base64Decode(base64_key_id, &out)) {
        return false;
    }

    // playready key size is 16
    static const size_t kKeyIdSize = 16;
    if (out.size() != kKeyIdSize) {
        return false;
    }

    // Swap first four bytes, swap next two bytes, swap next two bytes, leave the
    // remaining 8 bytes alone
    std::reverse(out.begin(), out.begin() + 4);
    std::swap(out[4], out[5]);
    std::swap(out[6], out[7]);
    keyId.swap(out);

    return true;
}

}  // namespace Media
}  // namespace Broadcom
