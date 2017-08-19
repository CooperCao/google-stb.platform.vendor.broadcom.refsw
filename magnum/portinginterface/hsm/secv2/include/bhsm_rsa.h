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

#ifndef BHSM_RSA__H_
#define BHSM_RSA__H_


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct BHSM_Rsa* BHSM_RsaHandle;

#define BHSM_RSA_INPUT_SIZE_MAX    (256)
#define BHSM_RSA_OUTPUT_SIZE_MAX   (BHSM_RSA_INPUT_SIZE_MAX)

typedef enum BHSM_RsaRandomStall
{
    BHSM_RsaRandomStall_eDisable    = 0,
    BHSM_RsaRandomStall_e50         = 1,  /* add 50% stall overhead */
    BHSM_RsaRandomStall_e25         = 2,  /* add 25% stall overhead */
    BHSM_RsaRandomStall_e12         = 3,  /* add 12% stall overhead */
    BHSM_RsaRandomStall_eMax
} BHSM_RsaRandomStall;

typedef enum BHSM_RsaKeySize
{
    BHSM_RsaKeySize_e1024, /* 128 bytes */
    BHSM_RsaKeySize_e2048, /* 256 bytes*/
    BHSM_RsaKeySize_eMax
} BHSM_RsaKeySize;


typedef struct BHSM_RsaExponentiateSettings
{
    BHSM_RsaKeySize keySize;

    /* size of each of these parameters is equal to the keySize parameter. */
    uint8_t modulus[BHSM_RSA_INPUT_SIZE_MAX];
    uint8_t exponent[BHSM_RSA_INPUT_SIZE_MAX];
    uint8_t inputBase[BHSM_RSA_INPUT_SIZE_MAX];    /* data to be encrypted/decrypted */

    struct{
        bool protectModulus;               /* protect modulus. True to enable.  */
        bool protectPrivateKey;           /* protect private RSA key. True to enable. */
        BHSM_RsaRandomStall randomStall; /* eanble random stall to thwart differential power analysis.*/
    }countermeasure;                      /* Anti power analysis configuration. */

    BHSM_CallbackDesc rsaComplete;       /* callback to indicate  when BSP has completed the RSA request. The client must
                                             call BHSM_Rsa_GetResult to retrieve result. The client can specify a NULL callback
                                             and use BHSM_Rsa_GetResult to poll for completion. */
}BHSM_RsaExponentiateSettings;


typedef struct BHSM_RsaExponentiateResult
{
    uint8_t data[BHSM_RSA_OUTPUT_SIZE_MAX];
    unsigned dataLength;

}BHSM_RsaExponentiateResult;

/*
    Open a RSA context that will enable hardware accelerated RSA requests. There is no limit on the number of RSA
    contexts that are open at any one time.
*/
BHSM_RsaHandle BHSM_Rsa_Open( /* attr{destructor=BHSM_Rsa_Close}  */
    BHSM_Handle hHsm,
    unsigned index /* supports BHSM_ANY_ID  */  /* TODO consider wheather struct should be used. */
    );

/*
   Release the RSA context.
*/
void BHSM_Rsa_Close(
    BHSM_RsaHandle handle
    );

/*
    Returns the current settings or default on first call after BHSM_Rsa_Open
*/
void  BHSM_Rsa_GetDefaultExponentiateSettings(
    BHSM_RsaHandle handle,
    BHSM_RsaExponentiateSettings *pSettings   /* [out] */
    );

/*
    Submit an RSA request for processing.
    Return values:
        - BHSM_RSA_PENDING - Request was sucessfully submitted. The request may have been
                              submitted to HW, or it may be queued internally.
        - BHSM_UNKNOWN
*/
BERR_Code BHSM_Rsa_Exponentiate(
    BHSM_RsaHandle handle,
    const BHSM_RsaExponentiateSettings *pSettings
    );




void BHSM_Rsa_GetDefaultResult(
    BHSM_RsaExponentiateResult *pResult /* [out] */
    );

/*
    Poll NEXUS to check if a request has completed.
    Return values:
        - BHSM_SECURITY_PENDING      Request is still pending.
        - BHSM_SUCCESS               Data is available and returned.
        - BHSM_UNKNOWN
*/
BERR_Code BHSM_Rsa_GetResult(
    BHSM_RsaHandle handle,
    BHSM_RsaExponentiateResult *pResult /* [out] */
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
