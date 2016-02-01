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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BMXT_PRIV_H__
#define BMXT_PRIV_H__

#include "breg_mem.h"
#include "bchp.h"
#include "bmxt.h"

#define BMXT_NOREG 0xffffffff

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BMXT_P_PlatformType
{
    BMXT_P_PlatformType_eHab,
    BMXT_P_PlatformType_eRpc,
    BMXT_P_PlatformType_eReg
} BMXT_P_PlatformType;

typedef struct BMXT_P_TransportData
{
    BCHP_Handle hChp;
    BREG_Handle hReg;
    BMXT_Settings settings;
    void* hHab;
    void* hRpc;
    BMXT_PowerSaverSettings powerSettings;

    struct {
        unsigned *num;
        uint32_t regbase, regbaseWakeup;
        uint32_t *regoffsets, *regoffsetsWakeup;
        unsigned *stepsize;
        BMXT_P_PlatformType type;
    } platform;
} BMXT_P_TransportData;

void BMXT_P_SetPlatform(BMXT_Handle mxt, BMXT_Chip chip, BMXT_ChipRev rev, unsigned **ptr);

uint32_t BMXT_RegRead32_common(BMXT_Handle handle, uint32_t addr);
void BMXT_RegWrite32_common(BMXT_Handle handle, uint32_t addr, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BMXT_PRIV_H__ */

