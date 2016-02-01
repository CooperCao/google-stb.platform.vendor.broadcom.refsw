/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
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
BERR_Code BXPT_P_Mcpb_Standby(BXPT_Handle hXpt);
void BXPT_P_Mcpb_Resume(BXPT_Handle hXpt);
BERR_Code BXPT_P_Dma_Standby(BXPT_Handle hXpt);
void BXPT_P_Dma_Resume(BXPT_Handle hXpt);

extern const char* const BXPT_P_SUBMODULE_STRING[];

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

