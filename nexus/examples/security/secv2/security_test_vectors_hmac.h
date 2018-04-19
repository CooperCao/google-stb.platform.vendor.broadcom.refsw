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

#ifndef _SECURITY_TEST_HMAC__
#define _SECURITY_TEST_HMAC__

/*
    The sha and hmac test data generated from openssl. These following test data is used by
    the test framework and the nexus security example applications.
*/

/*
    2.1.1 Usual sha160 HMAC test data. (Alternatively it can be something generated with the followings.)
    echo -n "sha1 hmac test value" | xxd -p -u -i -c 16
    echo -n "sha1 hmac test value" | openssl sha1 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/
char          sha_160_hmac_usual_text[] = { 0x77, 0x68, 0x61, 0x74 };

char          sha_160_hmac_usual_key[160 / 8] = { 0x4a, 0x65, 0x66, 0x65 };

uint8_t       sha_160_hmac_usual[160 / 8] = {
    0x25, 0xe3, 0x03, 0xbf, 0x21, 0x2d, 0x80, 0xad, 0xd7, 0x79,
    0xb9, 0x45, 0x5b, 0x51, 0xf4, 0xef, 0x0d, 0x10, 0x89, 0xb5
    };


/*
    2.1.2 Usual sha224 HMAC test data
    echo -n "sha224 hmac test value" | xxd -p -u -i -c 16
    echo -n "sha224 hmac test value" | openssl sha224 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/

char          sha_224_hmac_usual_text[] = {
    0X73, 0X68, 0X61, 0X32, 0X32, 0X34, 0X20, 0X68, 0X6D, 0X61, 0X63, 0X20, 0X74, 0X65, 0X73, 0X74,
    0X20, 0X76, 0X61, 0X6C, 0X75, 0X65
};

char          sha_224_hmac_usual_key[224 / 8] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,0x00
};

uint8_t       sha_224_hmac_usual[224 / 8] = {
    0XE7, 0X84, 0XF3, 0X30, 0X54, 0XAA, 0X75, 0XB5, 0X7F, 0X39, 0X9E, 0X1C, 0XC4, 0X1C, 0X39, 0X3E,
    0X22, 0XB4, 0X20, 0X85, 0XCA, 0XB3, 0XC5, 0X1C, 0X90, 0X09, 0XA5, 0XE8
};

/*
    2.1.3 Usual sha256 HMAC test data
    echo -n "sha256 hmac test value" | xxd -p -u -i -c 16
    echo -n "sha256 hmac test value" | openssl sha256 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/

char          sha_256_hmac_usual_text[] = {
    0X73, 0X68, 0X61, 0X32, 0X35, 0X36, 0X20, 0X68, 0X6D, 0X61, 0X63, 0X20, 0X74, 0X65, 0X73, 0X74,
    0X20, 0X76, 0X61, 0X6C, 0X75, 0X65
};

char          sha_256_hmac_usual_key[256 / 8] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,0x00
};

uint8_t       sha_256_hmac_usual[256 / 8] = {
    0X11, 0XAA, 0X1B, 0X49, 0X00, 0X0D, 0XC9, 0XDF, 0X26, 0XB8, 0X88, 0X00, 0X59, 0X2B, 0X33, 0XA0,
    0X4F, 0X3E, 0X32, 0X2D, 0X89, 0XDA, 0X0B, 0XC3, 0XA0, 0X6B, 0X63, 0X77, 0X74, 0XCB, 0X48, 0X57
};

/*
    2.2.1 One byte sha160 HMAC test data
    echo -n "1" | openssl sha1 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/
char          sha_160_hmac_one_text[] = {
    0X31,
};

char          sha_160_hmac_one_key[160 / 8] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,0x00
};

uint8_t       sha_160_hmac_one[160 / 8] = {
    0X68, 0X85, 0XED, 0X9E, 0XA8, 0XC3, 0X32, 0X11, 0X06, 0XA3, 0XFD, 0XD2, 0X30, 0XD1, 0XE5, 0X70,
    0XF7, 0X28, 0X7F, 0XC7
};

/*
    2.2.2 One byte sha224 HMAC test data
    echo -n "1" | openssl sha224 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/
char          sha_224_hmac_one_text[] = {
    0X31,
};

char          sha_224_hmac_one_key[224 / 8] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,0x00
};

uint8_t       sha_224_hmac_one[224 / 8] = {
    0X41, 0X21, 0XEB, 0X27, 0X92, 0X78, 0X2C, 0X14, 0XF6, 0X3A, 0XA4, 0X05, 0XDA, 0XF8, 0X68, 0X68,
    0X92, 0X9F, 0X6D, 0XAA, 0X0C, 0XE0, 0X72, 0X71, 0XD2, 0X97, 0X4A, 0XAF
};

/*
    2.2.3 One byte sha256 HMAC test data
    echo -n "1" | openssl sha256 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 -binary | xxd -p -u -i -c 16
*/
char          sha_256_hmac_one_text[] = {
    0X31,
};

char          sha_256_hmac_one_key[256 / 8] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,0x00
};

uint8_t       sha_256_hmac_one[256 / 8] = {
    0XE4, 0X5C, 0XA6, 0XE9, 0X35, 0X4A, 0X50, 0XCC, 0XC7, 0X9D, 0XAA, 0XDF, 0X09, 0X3F, 0XD4, 0X7E,
    0XE8, 0X51, 0X51, 0XA7, 0X98, 0XB0, 0XD2, 0XE6, 0X0B, 0X5C, 0X20, 0XBC, 0X82, 0X92, 0X0B, 0XF5
};

unsigned sha_160_hmac_usual_text_lenght = sizeof(sha_160_hmac_usual_text);
unsigned sha_160_hmac_usual_lenght = sizeof(sha_160_hmac_usual);
unsigned sha_224_hmac_usual_text_lenght = sizeof(sha_224_hmac_usual_text);
unsigned sha_224_hmac_usual_lenght = sizeof(sha_224_hmac_usual);
unsigned sha_256_hmac_usual_text_lenght = sizeof(sha_256_hmac_usual_text);
unsigned sha_256_hmac_usual_lenght = sizeof(sha_256_hmac_usual);

unsigned sha_160_hmac_zero_text_lenght = 0;
unsigned sha_224_hmac_zero_text_lenght = 0;
unsigned sha_256_hmac_zero_text_lenght = 0;
unsigned sha_160_hmac_one_text_lenght = 1;
unsigned sha_224_hmac_one_text_lenght = 1;
unsigned sha_256_hmac_one_text_lenght = 1;

unsigned sha_160_hmac_one_lenght = sizeof(sha_160_hmac_usual);
unsigned sha_160_hmac_zero_lenght = sizeof(sha_160_hmac_usual);
unsigned sha_224_hmac_one_lenght = sizeof(sha_224_hmac_usual);
unsigned sha_224_hmac_zero_lenght = sizeof(sha_224_hmac_usual);
unsigned sha_256_hmac_one_lenght = sizeof(sha_256_hmac_usual);
unsigned sha_256_hmac_zero_lenght = sizeof(sha_256_hmac_usual);

#endif /* #ifndef _SECURITY_TEST_HMAC__ */
