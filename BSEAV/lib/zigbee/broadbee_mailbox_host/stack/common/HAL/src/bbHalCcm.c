/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/src/bbHalCcm.c $
*
* DESCRIPTION:
*   CCM* routines HAL handler implementation.
*
* $Revision: 3878 $
* $Date: 2014-10-03 15:15:36Z $
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "bbHalCcm.h"            /* The subsequent header. */
#include "bbSysStackData.h"
#include "bbSecurity.h"

#if defined(__SoC__) && defined(SECURITY_EMU)
#warn Do not use CCM emulation on SoC
#endif

static void LeftshiftOneBit(uint8_t *input, uint8_t *output);
static void AESEncrypt(uint8_t *data, uint8_t *key);

/************************************************************************************//**
 \brief Calculates AES-CMAC value. Generates sub keys.

 \param[in] key - the security key to be used.
 \param[out] K1 - first key to be generated.
 \param[out] K2 - second key to be generated.
 \return Nothing.
 ****************************************************************************************/
static const uint8_t const_Rb[16] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};
void GenerateSubkey(uint8_t *key, uint8_t *K1, uint8_t *K2)
{
    uint8_t L[16], tmp[16];

    memset(L, 0, sizeof(L));
    memcpy(tmp, key, sizeof(tmp));
    AESEncrypt(L, tmp);
    if (0 == (L[0] & 0x80))
        /* If MSB(L) = 0, then K1 = L << 1 */
        LeftshiftOneBit(L, K1);
    else
    {
        /* Else K1 = ( L << 1 ) (+) Rb */
        LeftshiftOneBit(L, tmp);
        XOR128(tmp, (uint8_t*)const_Rb, K1);
    } /* if (0 == (L[0] & 0x80)) */

    if (0 == (K1[0] & 0x80))
        LeftshiftOneBit(K1, K2);
    else
    {
        LeftshiftOneBit(K1, tmp);
        XOR128(tmp, (uint8_t*)const_Rb, K2);
    }
} /* GenerateSubkey() */

/************************************************************************************//**
 \brief Calculates AES-CMAC value. Performs padding functionality.

 \param[in] lastb - pointer to the last data.
 \param[out] pad - pointer to the output padded data.
 \param[in] length - length of the padded data.
 \return Nothing.
 ****************************************************************************************/
void Padding(uint8_t *lastb, uint8_t *pad, uint8_t length)
{
    uint8_t j;

    /* original last block */
    for (j = 0; j < 16; ++j)
        if (j < length)
            pad[j] = lastb[j];
        else
            if (j == length)
                pad[j] = 0x80;
              else
                pad[j] = 0x00;
} /* Padding() */

/************************************************************************************//**
 \brief Calculates AES-CMAC value. Performs 128 bit XORing.

 \param[in] a - pointer to the first 128 bit data.
 \param[in] b - pointer to the second 128 bit data.
 \param[out] out - pointer to the resulting data.
 \return Nothing.
 ****************************************************************************************/
void XOR128(uint8_t *a, uint8_t *b, uint8_t *out)
{
    uint64_t *p0 = (uint64_t*)a, *p1 = (uint64_t*)b, *pr = (uint64_t*)out, *pe = p0 + 2;
    for (; p0 < pe; ++p0, ++p1, ++pr)
        *pr = *p0 ^ *p1;
} /* XOR128() */

/************************************************************************************//**
 \brief Calculates AES-CMAC value. Performs 128 bit shifting to the left.

 \param[in] input - pointer to the input 128 bit data.
 \param[out] output - pointer to the output 128 bit data.
 \return Nothing.
 ****************************************************************************************/
static void LeftshiftOneBit(uint8_t *input, uint8_t *output)
{
    uint8_t overflow = 0;

    /* Consider conversion as big-endian */
    for (int i=15; i>=0; i-- )
    {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80) ? 1 : 0;
    }
} /* LeftshiftOneBit() */

#if !defined(SECURITY_EMU)
// Use Hardware Security Engine

#include "bbSocCcm.h"

void HAL_CcmClearOutput(void)
{
    /* Just zero the output buffer */
    SOC_CcmClearOutput();
} /* HAL_CcmClearOutput() */

/************************************************************************************//**
 \brief Sets the CCM encryption key for a subsequent operations.

 \param[in] key - the key to be set.
 \return Nothing.
 ****************************************************************************************/
void HAL_CcmSetKey(const uint8_t *key)
{
    SOC_CcmSetKey(key);
} /* HAL_CcmSetKey() */

/************************************************************************************//**
 \brief AES-128 encryption.

 \param[in] data - pointer to the data to be encrypted.
 \param[in] key - pointer to the encryption key.
 \return Nothing.
 ****************************************************************************************/
static void AESEncrypt(uint8_t *data, uint8_t *key)
{
    SOC_AESEncrypt(data, key);
}

/************************************************************************************//**
 \brief Starts the authentication transformation with given nonce value.

 \param[in] nonce - the given nonce value.
 \param[in] mLength - the given message length value.
 \param[in] m - the M value.
 \param[in] hasA - true if A was set.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmStartAuthTransformation(uint8_t *nonce, uint8_t mLength, uint8_t m, bool hasA)
{
    return SOC_CcmStartAuthTransformation(nonce, mLength, m, hasA);
} /* HAL_CcmStartAuthTransformation() */

/************************************************************************************//**
 \brief Make a step of the authentication transformation with a given piece of data.

 \param[in] authData - a pointer to the piece of authentication data.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmAuthTransformation(uint8_t *authData)
{
    return SOC_CcmAuthTransformation(authData);
} /* HAL_CcmAuthTransformation() */

/************************************************************************************//**
 \brief Performs a step of the encryption transformation.

 \param[in] nonce - the given nonce value.
 \param[in] message - a pointer to the piece of data to be encrypted/decrypted.
 \param[in] iteration - current index to form Ai
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmEncryptionTransformation(uint8_t *nonce, uint8_t *message, uint8_t iteration)
{
    return SOC_CcmEncryptionTransformation(nonce, message, iteration);
} /* HAL_CcmEncryptionTransformation() */

/************************************************************************************//**
 \brief Calculates AES-CMAC value.

 \param[in] key - the security key to be used.
 \param[in] input - the data to be calculated.
 \param[in] length - length of the data to be calculated.
 \param[out] mac - the result output.
 \return Nothing.
 ****************************************************************************************/
void HAL_AES_CMAC(uint8_t *key, uint8_t *input, uint8_t length, uint8_t *mac)
{
    SOC_AES_CMAC(key, input, length, mac);
} /* HAL_AES_CMAC() */

/************************************************************************************//**
 \brief Performs a step of the Hash function calculation.

 \param[in] data - pointer to the given data value.
 \return pointer to the resulting hash.
 ****************************************************************************************/
uint8_t* HAL_CbbHash(uint8_t *data)
{
    return SOC_CbbHash(data);
} /* HAL_CbbHash() */

#else //#if !defined(SECURITY_EMU)
// Use emulation mode

/************************* STATIC FUNCTIONS ********************************************/
static void AESEncrypt(uint8_t *data, uint8_t *key);
static uint8_t galoIsMul2(uint8_t value);

/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
 \brief Sets the CCM encryption key for a subsequent operations.

 \param[in] key - the key to be set.
 \return Nothing.
 ****************************************************************************************/
void HAL_CcmSetKey(const uint8_t *key)
{
    /* Here we just copy the received pointer to the static data */
    GET_HAL_CCM_DATA_FIELD()->key = key;
} /* HAL_CcmSetKey() */

/************************************************************************************//**
 \brief Starts the authentication transformation with given nonce value.

 \param[in] nonce - the given nonce value.
 \param[in] mLength - the given message length value.
 \param[in] m - the M value.
 \param[in] hasA - true if A was set.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmStartAuthTransformation(uint8_t *nonce, uint8_t mLength, uint8_t m, bool hasA)
{
    /* Local variables */
    uint8_t key[16];

    /* Setup first value as (FLAGS || nonce || mLength). See IEEE Std 802.15.4-2006 Annex C.2.2. */
    /* Build flags field */
    GET_HAL_CCM_DATA_FIELD()->data[0] = SET_ADATA_FLAG(SET_M_FLAG(SET_L_FLAG(), m), hasA);
    /* Copy nonce */
    memcpy(&GET_HAL_CCM_DATA_FIELD()->data[1], nonce, SECURITY_CCM_NONCE_SIZE);
    /* Build last 2 bytes as zero and mLength respectively */
    GET_HAL_CCM_DATA_FIELD()->data[14] = 0;
    GET_HAL_CCM_DATA_FIELD()->data[15] = mLength;
    /* Copy key to temporary variable */
    memcpy(key, GET_HAL_CCM_DATA_FIELD()->key, sizeof(key));
    /* Encrypt data */
    AESEncrypt(GET_HAL_CCM_DATA_FIELD()->data, key);
    /* Return encrypted data buffer */
    return GET_HAL_CCM_DATA_FIELD()->data;
} /* HAL_CcmStartAuthTransformation() */

/************************************************************************************//**
 \brief Make a step of the authentication transformation with a given piece of data.

 \param[in] authData - a pointer to the piece of authentication data.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmAuthTransformation(uint8_t *authData)
{
    /* Local variables */
    uint8_t i;
    uint8_t key[16];

    /* XOR source authData with the static data buffer */
    for (i = 0; i < 16; ++i)
        GET_HAL_CCM_DATA_FIELD()->data[i] ^= authData[i];
    /* Copy the authentication key */
    memcpy(key, GET_HAL_CCM_DATA_FIELD()->key, sizeof(key));
    /* Encrypt the data */
    AESEncrypt(GET_HAL_CCM_DATA_FIELD()->data, key);
    /* Return encrypted data buffer */
    return GET_HAL_CCM_DATA_FIELD()->data;
} /* HAL_CcmAuthTransformation() */

/************************************************************************************//**
 \brief Performs a step of the encryption transformation.

 \param[in] nonce - the given nonce value.
 \param[in] message - a pointer to the piece of data to be encrypted/decrypted.
 \param[in] iteration - current index to form Ai
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmEncryptionTransformation(uint8_t *nonce, uint8_t *message, uint8_t iteration)
{
    /* Local variables */
    uint8_t i;
    uint8_t key[16];

    /* Setup first value as (FLAGS || nonce || mLength). See IEEE Std 802.15.4-2006 Annex C.2.2.
       Where FLAGS is Reserved || Reserved || 0 || L, i.e. simply L. */
    /* Setting up flags */
    GET_HAL_CCM_DATA_FIELD()->data[0] = SET_L_FLAG();
    /* Copying nonce */
    memcpy(&GET_HAL_CCM_DATA_FIELD()->data[1], nonce, SECURITY_CCM_NONCE_SIZE);
    /* Setup last 2 bytes as zero and iteration respectively */
    GET_HAL_CCM_DATA_FIELD()->data[14] = 0;
    GET_HAL_CCM_DATA_FIELD()->data[15] = iteration;
    /* Copy the key */
    memcpy(key, GET_HAL_CCM_DATA_FIELD()->key, sizeof(key));
    /* Encrypt data */
    AESEncrypt(GET_HAL_CCM_DATA_FIELD()->data, key);
    /* XOR encrypted data with the supplied message */
    for (i = 0; i < 16; ++i)
        message[i] = (GET_HAL_CCM_DATA_FIELD()->data[i] ^= message[i]);
    /* Return encrypted data buffer */
    return message;
} /* HAL_CcmEncryptionTransformation() */

/************************************************************************************//**
 \brief Calculates AES-CMAC value.

 \param[in] key - the security key to be used.
 \param[in] input - the data to be calculated.
 \param[in] length - length of the data to be calculated.
 \param[out] mac - the result output.
 \return Nothing.
 ****************************************************************************************/
void HAL_AES_CMAC(uint8_t *key, uint8_t *input, uint8_t length, uint8_t *mac)
{
    uint8_t X[16], Y[16], M_last[16], padded[16], K1[16], K2[16], keyInt[16], dataInt[16];
    uint8_t n, i, flag;

    GenerateSubkey(key, K1, K2);
    n = (length + 15) >> 4;
    if (0 == n)
    {
        n = 1;
        flag = 0;
    }
    else /* if (0 == n)  */
        flag = (0 == (length & 0xF)) ? 1 : 0;
    if (flag)
        XOR128(&input[(n - 1) << 4], K1, M_last);
    else
    {
        Padding(&input[(n - 1) << 4], padded, length & 0xF);
        XOR128(padded, K2, M_last);
    } /* if (flag) */

    memset(X, 0, sizeof(X));
    for (i = 0; i < n - 1; ++i)
    {
        XOR128(X, &input[i << 4], Y);        /* Y := Mi (+) X  */
        memcpy(keyInt, key, sizeof(keyInt));
        memcpy(dataInt, Y, sizeof(dataInt));
        AESEncrypt(dataInt, keyInt);
        memcpy(X, dataInt, sizeof(dataInt));  /* X := AES-128(KEY, Y); */
    } /* for (i = 0; i < n - 1; ++i) */

    XOR128(X, M_last, Y);
    memcpy(keyInt, key, sizeof(keyInt));
    memcpy(dataInt, Y, sizeof(dataInt));
    AESEncrypt(dataInt, keyInt);

    memcpy(mac, dataInt, sizeof(dataInt));
} /* HAL_AES_CMAC() */

/************************************************************************************//**
 \brief Performs a step of the Hash function calculation.

 \param[in] data - pointer to the given data value.
 \return pointer to the resulting hash.
 ****************************************************************************************/
uint8_t* HAL_CbbHash(uint8_t *data)
{
    /* HASH(i) = AES(HASH(i - 1), data) ^ data */
    uint8_t the_data[16], i;

    memcpy(the_data, data, sizeof(the_data));
    AESEncrypt(the_data, GET_HAL_CCM_DATA_FIELD()->data);
    for (i = 0; i < sizeof(the_data); ++i)
        GET_HAL_CCM_DATA_FIELD()->data[i] = data[i] ^ the_data[i];
    return GET_HAL_CCM_DATA_FIELD()->data;
} /* HAL_CbbHash() */

/************************************************************************************//**
 \brief Clears the contents of CcmOut to all zeros.
 ****************************************************************************************/
void HAL_CcmClearOutput(void)
{
    /* Just zero the output buffer */
    memset(GET_HAL_CCM_DATA_FIELD()->data, 0, sizeof(GET_HAL_CCM_DATA_FIELD()->data));
} /* HAL_CcmClearOutput() */

/**//**
 * \brief The S-Box array.
 */
static const uint8_t sbox[] =   {
/* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,   /* 0 */
0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,   /* 1 */
0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,   /* 2 */
0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,   /* 3 */
0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,   /* 4 */
0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,   /* 5 */
0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,   /* 6 */
0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,   /* 7 */
0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,   /* 8 */
0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,   /* 9 */
0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,   /* A */
0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,   /* B */
0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,   /* C */
0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,   /* D */
0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,   /* E */
0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }; /* F */

/************************************************************************************//**
 \brief AES-128 encryption. Multiply by 2 in the GALOIS field

 \param[in] value - the value to be multiplied.
 \return Resulting value.
 ****************************************************************************************/
static uint8_t galoIsMul2(uint8_t value)
{
    if (value & 0x80)
        return ((value << 1) ^ 0x1b);
    else
        return (value << 1);
} /* galoIsMul2() */

/************************************************************************************//**
 \brief AES-128 encryption.

 \param[in] data - pointer to the data to be encrypted.
 \param[in] key - pointer to the encryption key.
 \return Nothing.
 ****************************************************************************************/
static void AESEncrypt(uint8_t *data, uint8_t *key)
{
#if !defined(_DUMMY_ENCRYPTION_)
    /* Local variables */
    uint8_t buf1, buf2, buf3, buf4, round, i;
    uint8_t rcon;

    /* Rcon initial value. All subsequent values are computed. */
    rcon = 0x01;

    /* Main AES data loop */
    for (round = 0; round < 10; ++round)
    {
        /* Add key + sbox */
        for (i = 0; i < 16; ++i)
            data[i] = sbox[data[i] ^ key[i]];
        /* Shift rows */
        buf1 = data[1];
        data[1] = data[5];
        data[5] = data[9];
        data[9] = data[13];
        data[13] = buf1;

        buf1 = data[2];
        buf2 = data[6];
        data[2] = data[10];
        data[6] = data[14];
        data[10] = buf1;
        data[14] = buf2;

        buf1 = data[15];
        data[15] = data[11];
        data[11] = data[7];
        data[7] = data[3];
        data[3] = buf1;

        /* Process mixcolumn for all rounds but the last one */
        if (round < 9)
            for (i = 0; i < 4; ++i)
            {
                /* Compute the current index */
                buf4 = (i << 2);
                buf1 = data[buf4] ^ data[buf4 + 1] ^ data[buf4 + 2] ^ data[buf4 + 3];
                buf2 = data[buf4];
                buf3 = data[buf4] ^ data[buf4 + 1];
                buf3 = galoIsMul2(buf3);
                data[buf4] ^= buf3 ^ buf1;
                buf3 = data[buf4 + 1] ^ data[buf4 + 2];
                buf3 = galoIsMul2(buf3);
                data[buf4 + 1] ^= buf3 ^ buf1;
                buf3 = data[buf4 + 2] ^ data[buf4 + 3];
                buf3 = galoIsMul2(buf3);
                data[buf4 + 2] ^= buf3 ^ buf1;
                buf3 = data[buf4 + 3] ^ buf2;
                buf3 = galoIsMul2(buf3);
                data[buf4 + 3] ^= buf3 ^ buf1;
            } /* for (i = 0; i < 4; i++) */

        /* Key schedule */
        /* Compute the 16 next round key bytes */
        key[0] = sbox[key[13]] ^ key[0] ^ rcon;
        key[1] = sbox[key[14]] ^ key[1];
        key[2] = sbox[key[15]] ^ key[2];
        key[3] = sbox[key[12]] ^ key[3];
        for (i = 4; i < 16; ++i)
            key[i] = key[i] ^ key[i - 4];
        /* Compute the next Rcon value */
        rcon = galoIsMul2(rcon);
    }

    /* Process last AddRoundKey */
    for (i = 0; i < 16; ++i)
        data[i] = data[i] ^ key[i];
#endif // defined(_DUMMY_ENCRYPTION_)
} /* AESEncrypt() */

#endif /* SECURITY_EMU */

/* eof bbHalCcm.c */
