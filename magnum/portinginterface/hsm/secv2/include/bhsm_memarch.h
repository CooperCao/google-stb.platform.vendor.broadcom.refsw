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

#ifndef BHSM_MEMARCH_
#define BHSM_MEMARCH_

#include "bstd.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BHSM_MEMCARCH_MAX_TAGS (24)


typedef struct BHSM_MemArch_Client_t
{
    uint32_t read;
    uint32_t write;
}BHSM_MemArch_Client_t;


typedef struct BHSM_MemArchEnableGeneric
{
    /* in */
    uint32_t modeControl;
    struct{
        BSTD_DeviceOffset  upper;
        BSTD_DeviceOffset  lower;
    }addressRange;

    BHSM_MemArch_Client_t clients[8];
    bool enableBackgroundCheck;

    /* out */
}BHSM_MemArchEnableGeneric;


typedef struct BHSM_MemArchQueryUserTag
{
    /* in */
    /* out */
    unsigned numTags;
    struct{
        unsigned memcIndex;
        unsigned archIndex;
        unsigned userTag;
    }arch[BHSM_MEMCARCH_MAX_TAGS];
}BHSM_MemArchQueryUserTag;


/* Enable a generic memory Address Range Checker. */
BERR_Code BHSM_MemArch_EnableGeneric( BHSM_Handle hHsm, BHSM_MemArchEnableGeneric *pParam );

/* Return user tag of all active MEMC ARCH, except CRR, SRR and PCIe. */
BERR_Code BHSM_MemArch_QueryUserTag( BHSM_Handle hHsm,  BHSM_MemArchQueryUserTag *pParam );

#ifdef __cplusplus
}
#endif

#endif /* BHSM_MEMARCH_ */
