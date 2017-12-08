/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
MSP (Multi Stage Programming) OTP (One Time Program) is a set of over 600 four byte programmable onchip entries. Each individual
entry (referenced by an index) has a dedicated purpose.
Note:
    - Some entries have read restrictions; the can't be read.
    - Some entries have write restrictions.
    - Some entries will be programmed on delivered chips.
*/

#include "bhsm.h"

#ifndef BHSM_OTP_MSP__H_
#define BHSM_OTP_MSP__H_

#ifdef __cplusplus
extern "C"
{
#endif

#define BHSM_OTPMSP_MAX_READ_RANGE  (80)


/* parameters to BHSM_OtpMsp_ReadRange */
typedef struct
{
    unsigned startIndex;        /* the first MSP to read */
    unsigned numMsp;            /* the number of MSP entries to read */
    bool     readLock;          /* "false" to read the MSP data. "true" to read the MSP lock masks. */

    unsigned memSize;           /* the amount of memory availabe at pMem (in units of uint32_t). memSize must be greater or equal to numMsp */
    uint32_t *pMem;             /* client managed memory into which the data will be read. */

}BHSM_OtpMspReadRange;

/* parameters to BHSM_OtpMsp_Read */
typedef struct
{
    /* input */
    unsigned index;    /* MSP OTP entry locator. */

    /* output */
    uint32_t data;    /* [out] data read from specified MSP OTP index location */
    uint32_t valid;   /* [out] mask indicating what bits of "dataD are valid.
                                                        1 indicates the bit is prorammed and retured in "data" .
                                                        0 indicates the bit is NOT prorammed and assumed to have a value of 0. */
} BHSM_OtpMspRead;

/* parameters to BHSM_OtpMsp_Write */
typedef struct
{
    /* input */
    unsigned index;    /* MSP OTP entry locator. */
    uint32_t data;     /* data to be written */
    uint32_t mask;     /* mask indicating what bits of "data" are to be programmed */

    /* output */
    /*none*/

} BHSM_OtpMspWrite;

/*
    Read an MSP OTP entry.
*/
BERR_Code BHSM_OtpMsp_Read(
    BHSM_Handle hHsm,
    BHSM_OtpMspRead *pParam
    );

/*
    Write an MSP OTP entry. MSP bits can only be written once.
*/
BERR_Code BHSM_OtpMsp_Write(
    BHSM_Handle hHsm,
    BHSM_OtpMspWrite *pParam
    );

/*
    Read a range of  MSP entries.
    The client is responsible for the memory into which the MSP entries are read.
    An MSP entries' data *or* lock mask can be read with this function.
*/
BERR_Code BHSM_OtpMsp_ReadRange(
    BHSM_Handle hHsm,
    BHSM_OtpMspReadRange *pParam         /* Identify the range of MSP entries to read. */
    );


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
