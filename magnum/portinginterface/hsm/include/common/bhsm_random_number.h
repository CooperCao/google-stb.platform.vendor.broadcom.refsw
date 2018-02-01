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


#ifndef BHSM_RANDOM_NUMBER_H__
#define BHSM_RANDOM_NUMBER_H__

#include "bhsm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_MAX_RANDOM_NUM_LEN     352

typedef enum
{
    BHSM_RandomNumber_Type_eRNGRAW    = 0,
    BHSM_RandomNumber_Type_eRNGSHA    = 1

}BHSM_RandomNumber_Type_e;


typedef struct BHSM_UserRandomNumberIO
{

    /* In: Selects the random number generation type:
            0x00 = raw random number from RNG, i.e. BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGRAW
            0x01 = raw random number from SHA, i.e. BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGSHA
    */
    BHSM_RandomNumber_Type_e        RandomNumType;

    /* In:  specify the destination of the output
        RANDOM_NUMBER_OUTPUT_TO_HOST=0x00, Output as many random numbers as specified in the Length field below to Host;
    */
    unsigned char                    keyDest;


     /* In: Length of the Random Number in bytes. It should be 32-bit word aligned.
             If keyDest is RANDOM_NUMBER_OUTPUT_TO_HOST, the maximum size shall be 352 bytes.*/
    unsigned int                    unDataLen;

    uint32_t                        unStatus; /* DEPRECATED */

    /* Out: the returned/generated random number according to the inputs*/
    unsigned char                    aucData[BHSM_MAX_RANDOM_NUM_LEN];

} BHSM_UserRandomNumberIO_t;

/*****************************************************************************
Summary:

This function provides access to the BSP random number generator (RNG).

Calling Context:
This function can be called any time after the system and BSP is initialized

Performance and Timing:
This is a synchronous/blocking function that won't return until it is done or failed.

*****************************************************************************/
BERR_Code BHSM_UserRandomNumber (
        BHSM_Handle                hHsm,
        BHSM_UserRandomNumberIO_t *pRandomNumber
);


#ifdef __cplusplus
}
#endif

#endif /* BHSM_RANDOM_NUMBER_H__ */
