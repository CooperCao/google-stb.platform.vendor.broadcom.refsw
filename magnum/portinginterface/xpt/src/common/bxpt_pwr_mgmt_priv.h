/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BCHP_PWR_MGMT_PRIV_H__
#define BCHP_PWR_MGMT_PRIV_H__

#include "bstd.h"
#include "breg_mem.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BXPT_P_Submodule
{
    BXPT_P_Submodule_eMsg = 0,
    BXPT_P_Submodule_eRave,
    BXPT_P_Submodule_eMcpb,
    BXPT_P_Submodule_ePsub,
    BXPT_P_Submodule_ePcrOffset,
    BXPT_P_Submodule_eRemux,
    BXPT_P_Submodule_eMpod,
    BXPT_P_Submodule_eXmemif,
    BXPT_P_Submodule_eTsio,
    BXPT_P_Submodule_eMax
} BXPT_P_Submodule;

struct BXPT_P_TransportData;
void BXPT_P_AcquireSubmodule(struct BXPT_P_TransportData *hXpt, BXPT_P_Submodule sub);
void BXPT_P_ReleaseSubmodule(struct BXPT_P_TransportData *hXpt, BXPT_P_Submodule sub);
#ifdef BCHP_PWR_RESOURCE_XPT
BERR_Code BXPT_P_Mcpb_Standby(BXPT_Handle hXpt);
void BXPT_P_Mcpb_Resume(BXPT_Handle hXpt);
BERR_Code BXPT_P_Dma_Standby(BXPT_Handle hXpt);
void BXPT_P_Dma_Resume(BXPT_Handle hXpt);
#endif

typedef struct BXPT_P_RegisterRange {
    const char* name;
    uint32_t startAddress;
    uint32_t endAddress;
} BXPT_P_RegisterRange;

struct BXPT_Backup
{
    uint32_t **reg; /* array of pages of register values */
    unsigned pagesize; /* in bytes */
    unsigned numpages; /* size of reg[], = (total+pagesize-1)/pagesize */
    unsigned total; /* total registers */
};

BERR_Code BXPT_P_RegisterToMemory(BREG_Handle reg, struct BXPT_Backup *backup, const BXPT_P_RegisterRange *pRanges);
void BXPT_P_MemoryToRegister(BREG_Handle reg, struct BXPT_Backup *backup, const BXPT_P_RegisterRange *pRanges);
void BXPT_P_FreeBackup(struct BXPT_Backup *backup);

#ifdef BCHP_PWR_RESOURCE_XPT
extern const BXPT_P_RegisterRange XPT_SRAM_LIST[];
extern const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[];
#endif

#ifdef __cplusplus
}
#endif

#endif /* BCHP_PWR_MGMT_PRIV_H__ */

