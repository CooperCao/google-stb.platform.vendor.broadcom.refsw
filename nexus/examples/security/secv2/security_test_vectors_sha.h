/******************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef _SECURITY_TEST_SHA__
#define _SECURITY_TEST_SHA__

/*
    The sha and hmac test data generated from openssl. These following test data is used by
    the test framework and the nexus security example applications.
*/

/*
    1.1.1 Usual sha160 test data
    echo -n "sha1 test value" | xxd -p -u -i -c 16
    echo -n "sha1 test value" | openssl dgst -sha1  -binary | xxd -p -u -i -c 16
*/
char          sha_160_usual_text[] = {
    0X73, 0X68, 0X61, 0X31, 0X20, 0X74, 0X65, 0X73, 0X74, 0X20, 0X76, 0X61, 0X6C, 0X75, 0X65
};
uint8_t       sha_160_usual[160 / 8] = {
    0X01, 0XB4, 0X9A, 0X39, 0XF0, 0XD6, 0X5D, 0X9E, 0XEB, 0XBD, 0XFC, 0X6C, 0X48, 0XC6, 0XE5, 0XA0,
    0X75, 0X53, 0X4B, 0X2E
};
unsigned sha_160_usual_text_lenght = sizeof(sha_160_usual_text);
unsigned sha_160_usual_lenght = sizeof(sha_160_usual);

/*
    1.1.2 Usual sha224 test data
    echo -n "sha224 test value" | xxd -p -u -i -c 16
    echo -n "sha224 test value" | openssl dgst -sha224  -binary | xxd -p -u -i -c 16
*/

char          sha_224_usual_text[] = {
    0X73, 0X68, 0X61, 0X32, 0X32, 0X34, 0X20, 0X74, 0X65, 0X73, 0X74, 0X20, 0X76, 0X61, 0X6C, 0X75,
    0X65
};

uint8_t       sha_224_usual[224 / 8] = {
    0X49, 0X01, 0X30, 0XF6, 0XE6, 0X74, 0X3F, 0XDF, 0X18, 0XE4, 0XD6, 0XF3, 0X19, 0X94, 0XB0, 0X15,
    0X4F, 0X18, 0X82, 0X1A, 0X52, 0X1B, 0X5F, 0XC4, 0X1D, 0X84, 0X05, 0X5D
};
unsigned sha_224_usual_text_lenght = sizeof(sha_224_usual_text);
unsigned sha_224_usual_lenght = sizeof(sha_224_usual);
/*
    1.1.3 Usual sha256 test data
    echo -n "sha256 test value" | xxd -p -u -i -c 16
    echo -n "sha256 test value" | openssl dgst -sha256  -binary | xxd -p -u -i -c 16
*/

char          sha_256_usual_text[] = {
    0X73, 0X68, 0X61, 0X32, 0X35, 0X36, 0X20, 0X74, 0X65, 0X73, 0X74, 0X20, 0X76, 0X61, 0X6C, 0X75,
    0X65
};

uint8_t       sha_256_usual[256 / 8] = {
    0X84, 0X36, 0XAC, 0XC9, 0X40, 0X96, 0X1C, 0X57, 0X6C, 0X25, 0XC2, 0X6F, 0X19, 0X98, 0X2D, 0X8A,
    0X2A, 0X9F, 0XCC, 0X06, 0X11, 0X61, 0X68, 0XE8, 0XA5, 0X03, 0X56, 0X31, 0X92, 0X50, 0X93, 0X85
};
unsigned sha_256_usual_text_lenght = sizeof(sha_256_usual_text);
unsigned sha_256_usual_lenght = sizeof(sha_256_usual);
/*
    1.2.1 Zero byte sha160 test data
    echo -n "ZERO" |xxd -p -u -i -c 16
    echo -n "" | openssl dgst -sha1  -binary | xxd -p -u -i -c 16
*/
char          sha_160_zero_text[] = {
    'Z', 'E', 'R', 'O'
};

uint8_t       sha_160_zero[160 / 8] = {
    0XDA, 0X39, 0XA3, 0XEE, 0X5E, 0X6B, 0X4B, 0X0D, 0X32, 0X55, 0XBF, 0XEF, 0X95, 0X60, 0X18, 0X90,
    0XAF, 0XD8, 0X07, 0X09
};

/*
    1.2.2 Zero byte sha224 test data
    echo -n "" | openssl dgst -sha224  -binary | xxd -p -u -i -c 16
*/
char          sha_224_zero_text[] = {
    'Z', 'E', 'R', 'O'
};

uint8_t       sha_224_zero[224 / 8] = {
    0XD1, 0X4A, 0X02, 0X8C, 0X2A, 0X3A, 0X2B, 0XC9, 0X47, 0X61, 0X02, 0XBB, 0X28, 0X82, 0X34, 0XC4,
    0X15, 0XA2, 0XB0, 0X1F, 0X82, 0X8E, 0XA6, 0X2A, 0XC5, 0XB3, 0XE4, 0X2F
};

/*
    1.2.3 Zero byte sha256 test data
    echo -n "" | openssl dgst -sha256  -binary | xxd -p -u -i -c 16
*/
char          sha_256_zero_text[] = {
    'Z', 'E', 'R', 'O'
};

uint8_t       sha_256_zero[256 / 8] = {
    0XE3, 0XB0, 0XC4, 0X42, 0X98, 0XFC, 0X1C, 0X14, 0X9A, 0XFB, 0XF4, 0XC8, 0X99, 0X6F, 0XB9, 0X24,
    0X27, 0XAE, 0X41, 0XE4, 0X64, 0X9B, 0X93, 0X4C, 0XA4, 0X95, 0X99, 0X1B, 0X78, 0X52, 0XB8, 0X55
};

/*
    1.3.1 One byte sha160 test data
    echo -n "1" | xxd -p -u -i -c 16
    echo -n "1" | openssl dgst -sha1  -binary | xxd -p -u -i -c 16
*/
char          sha_160_one_text[] = {
    0X31,
};

uint8_t       sha_160_one[160 / 8] = {
    0X35, 0X6A, 0X19, 0X2B, 0X79, 0X13, 0XB0, 0X4C, 0X54, 0X57, 0X4D, 0X18, 0XC2, 0X8D, 0X46, 0XE6,
    0X39, 0X54, 0X28, 0XAB
};

/*
    1.3.2 One byte sha224 test data
    echo -n "1" | openssl dgst -sha224  -binary | xxd -p -u -i -c 16
*/
char          sha_224_one_text[] = {
    0X31,
};

uint8_t       sha_224_one[224 / 8] = {
    0XE2, 0X53, 0X88, 0XFD, 0XE8, 0X29, 0X0D, 0XC2, 0X86, 0XA6, 0X16, 0X4F, 0XA2, 0XD9, 0X7E, 0X55,
    0X1B, 0X53, 0X49, 0X8D, 0XCB, 0XF7, 0XBC, 0X37, 0X8E, 0XB1, 0XF1, 0X78
};

/*
    1.3.3 One byte sha256 test data
    echo -n "1" | openssl dgst -sha256  -binary | xxd -p -u -i -c 16
*/
char          sha_256_one_text[] = {
    0X31,
};

uint8_t       sha_256_one[256 / 8] = {
    0X6B, 0X86, 0XB2, 0X73, 0XFF, 0X34, 0XFC, 0XE1, 0X9D, 0X6B, 0X80, 0X4E, 0XFF, 0X5A, 0X3F, 0X57,
    0X47, 0XAD, 0XA4, 0XEA, 0XA2, 0X2F, 0X1D, 0X49, 0XC0, 0X1E, 0X52, 0XDD, 0XB7, 0X87, 0X5B, 0X4B
};


unsigned sha_160_zero_text_lenght = 0;
unsigned sha_224_zero_text_lenght = 0;
unsigned sha_256_zero_text_lenght = 0;
unsigned sha_160_one_text_lenght = 1;
unsigned sha_224_one_text_lenght = 1;
unsigned sha_256_one_text_lenght = 1;

unsigned sha_160_one_lenght = sizeof(sha_160_usual);
unsigned sha_160_zero_lenght = sizeof(sha_160_usual);
unsigned sha_224_one_lenght = sizeof(sha_224_usual);
unsigned sha_224_zero_lenght = sizeof(sha_224_usual);
unsigned sha_256_one_lenght = sizeof(sha_256_usual);
unsigned sha_256_zero_lenght = sizeof(sha_256_usual);


#endif /* #ifndef _SECURITY_TEST_SHA__ */
