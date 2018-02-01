/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
/*
    This API supports HW assisted exponentiation operation for RSA. The API does not support data padding.
    In order to fulfil and encryption or decryption operation, it is the users responsibility to appropriately
    pad the data before using this API.
*/

#ifndef BHSM_RSA__H_
#define BHSM_RSA__H_

#include "bstd.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct BHSM_Rsa* BHSM_RsaHandle;

#define BHSM_RSA_OUTPUT_SIZE_MAX   (256) /*2048 bits*/

typedef enum BHSM_RsaKeySize
{
    BHSM_RsaKeySize_e1024, /* 128 bytes */
    BHSM_RsaKeySize_e2048, /* 256 bytes*/
    BHSM_RsaKeySize_eMax
} BHSM_RsaKeySize;


typedef struct BHSM_RsaExponentiateSettings
{
    BHSM_RsaKeySize keySize;       /* the RSA key size */
    bool counterMeasure;           /* true to enable counter measures */
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0)
    BSTD_DeviceOffset rsaData;     /* contains the modulus, exponent, and base in contigious memory.*/
#else
    uint8_t *rsaData; /* contains the modulus, exponent, and base in contigious memory.*/
#endif
                                     /*TODO, explain layout. */
}BHSM_RsaExponentiateSettings;


typedef struct BHSM_RsaExponentiateResult
{
    uint8_t data[BHSM_RSA_OUTPUT_SIZE_MAX];
    unsigned dataLength;

}BHSM_RsaExponentiateResult;

/*  Open/Close a RSA context that will enable hardware accelerated RSA requests. Only one RSA contexts that are open
    at any one time. */
BHSM_RsaHandle BHSM_Rsa_Open( BHSM_Handle hHsm );
void BHSM_Rsa_Close( BHSM_RsaHandle handle );

/*  Submit an RSA request for processing.
    Return values:
        - BHSM_RSA_PENDING - Request was sucessfully submitted. The request may have been
                              submitted to HW, or it may be queued internally.
        - BHSM_UNKNOWN */
BERR_Code BHSM_Rsa_Exponentiate( BHSM_RsaHandle handle, const BHSM_RsaExponentiateSettings *pSettings);

/*  Poll HSM to check if a request has completed.
    Return values:
        - BHSM_SECURITY_PENDING      Request is still pending.
        - BHSM_SUCCESS               Data is available and returned.
        - BHSM_UNKNOWN   */
BERR_Code BHSM_Rsa_GetResult( BHSM_RsaHandle handle, BHSM_RsaExponentiateResult *pResult );



/*************************************************************************/
/******************************** PRIVATE ********************************/
/*************************************************************************/

typedef struct{
    unsigned dummy;
}BHSM_RsaModuleSettings;
/* Called internally on module platform initialisation. */
BERR_Code BHSM_Rsa_Init( BHSM_Handle hHsm, BHSM_RsaModuleSettings *pSettings );
void BHSM_Rsa_Uninit( BHSM_Handle hHsm );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
