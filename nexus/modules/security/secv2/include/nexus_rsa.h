/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

/*
    This API supports HW assisted exponentiation operation for RSA. The API does not support data padding.
    In order to fulfil and encryption or decryption operation, it is the users responsibility to appropriately
    pad the data before using this API.
*/

#ifndef NEXUS_RSA__H_
#define NEXUS_RSA__H_

#include "nexus_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NEXUS_Rsa* NEXUS_RsaHandle;

#define NEXUS_RSA_INPUT_SIZE_MAX    (256)
#define NEXUS_RSA_OUTPUT_SIZE_MAX   (NEXUS_RSA_INPUT_SIZE_MAX)

typedef enum NEXUS_RsaRandomStall
{
    NEXUS_RsaRandomStall_eDisable    = 0,
    NEXUS_RsaRandomStall_e50         = 1,  /* add 50% stall overhead */
    NEXUS_RsaRandomStall_e25         = 2,  /* add 25% stall overhead */
    NEXUS_RsaRandomStall_e12         = 3,  /* add 12% stall overhead */
    NEXUS_RsaRandomStall_eMax
} NEXUS_RsaRandomStall;

typedef enum NEXUS_RsaKeySize
{
    NEXUS_RsaKeySize_e1024, /* 128 bytes */
    NEXUS_RsaKeySize_e2048, /* 256 bytes*/
    NEXUS_RsaKeySize_eMax
} NEXUS_RsaKeySize;


typedef struct NEXUS_RsaExponentiateSettings
{
    NEXUS_RsaKeySize keySize;

    /* size of each of these parameters is equal to the keySize parameter. */
    uint8_t modulus[NEXUS_RSA_INPUT_SIZE_MAX];
    uint8_t exponent[NEXUS_RSA_INPUT_SIZE_MAX];
    uint8_t inputBase[NEXUS_RSA_INPUT_SIZE_MAX];    /* data to be encrypted/decrypted */

    struct{
        bool protectModulus;               /* protect modulus. True to enable.  */
        bool protectPrivateKey;           /* protect private RSA key. True to enable. */
        NEXUS_RsaRandomStall randomStall; /* eanble random stall to thwart differential power analysis.*/
    }countermeasure;                      /* Anti power analysis configuration. */

    NEXUS_CallbackDesc rsaComplete;       /* callback to indicate  when BSP has completed the RSA request. The client must
                                             call NEXUS_Rsa_GetResult to retrieve result. The client can specify a NULL callback
                                             and use NEXUS_Rsa_GetResult to poll for completion. */
}NEXUS_RsaExponentiateSettings;


typedef struct NEXUS_RsaExponentiateResult
{
    uint8_t data[NEXUS_RSA_OUTPUT_SIZE_MAX];
    unsigned dataLength;

}NEXUS_RsaExponentiateResult;

/*
    Open a RSA context that will enable hardware accelerated RSA requests. There is no limit on the number of RSA
    contexts that are open at any one time.
*/
NEXUS_RsaHandle NEXUS_Rsa_Open(                /* attr{destructor=NEXUS_Rsa_Close} */
    unsigned index                             /* supports NEXUS_ANY_ID  */
    );

/*
   Release the RSA context.
*/
void NEXUS_Rsa_Close(
    NEXUS_RsaHandle handle
    );

/*
    Returns the current settings or default on first call after NEXUS_Rsa_Open
*/
void  NEXUS_Rsa_GetDefaultExponentiateSettings(
    NEXUS_RsaHandle handle,
    NEXUS_RsaExponentiateSettings *pSettings   /* [out] */
    );

/*
    Submit an RSA request for processing.
    Return values:
        - NEXUS_RSA_PENDING - Request was sucessfully submitted. The request may have been
                              submitted to HW, or it may be queued internally.
        - NEXUS_UNKNOWN
*/
NEXUS_Error NEXUS_Rsa_Exponentiate(
    NEXUS_RsaHandle handle,
    const NEXUS_RsaExponentiateSettings *pSettings
    );




void NEXUS_Rsa_GetDefaultResult(
    NEXUS_RsaExponentiateResult *pResult /* [out] */
    );

/*
    Poll NEXUS to check if a request has completed.
    Return values:
        - NEXUS_SECURITY_PENDING      Request is still pending.
        - NEXUS_SUCCESS               Data is available and returned.
        - NEXUS_UNKNOWN
*/
NEXUS_Error NEXUS_Rsa_GetResult(
    NEXUS_RsaHandle handle,
    NEXUS_RsaExponentiateResult *pResult /* [out] */
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
