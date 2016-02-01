/***************************************************************************
 *     (c)2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#ifndef NEXUS_SAGE_BOOT_TYPES_H_
#define NEXUS_SAGE_BOOT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp_scpu_globalram.h"

/* SAGE boot status */
typedef enum
{
    NEXUS_SageBoot_eStarted = 0x0000,
    NEXUS_SageBoot_eBlStarted = 0x0001,
    NEXUS_SageBoot_eNotStarted = 0x00FF,
    NEXUS_SageBoot_eError = 0xFFFF
} NEXUS_SageBoot;


/* Global SRAM register indexes */
typedef enum
{
    NEXUS_SageGlobalSram_eRequestBuffer = 0x20,
    NEXUS_SageGlobalSram_eRequestBufferSize,
    NEXUS_SageGlobalSram_eAckBuffer,
    NEXUS_SageGlobalSram_eAckBufferSize,
    NEXUS_SageGlobalSram_eResponseBuffer,
    NEXUS_SageGlobalSram_eResponseBufferSize,
    NEXUS_SageGlobalSram_eRestrictedRegion1,
    NEXUS_SageGlobalSram_eRestrictedRegion1Size,
    NEXUS_SageGlobalSram_eRestrictedRegion2,
    NEXUS_SageGlobalSram_eRestrictedRegion2Size,
    NEXUS_SageGlobalSram_eRestrictedRegion3,
    NEXUS_SageGlobalSram_eRestrictedRegion3Size,
    NEXUS_SageGlobalSram_eRestrictedRegion4,
    NEXUS_SageGlobalSram_eRestrictedRegion4Size,
    NEXUS_SageGlobalSram_eRestrictedRegion5,
    NEXUS_SageGlobalSram_eRestrictedRegion5Size,
    NEXUS_SageGlobalSram_eRestrictedRegion6,
    NEXUS_SageGlobalSram_eRestrictedRegion6Size,
    NEXUS_SageGlobalSram_eRestrictedRegion7,
    NEXUS_SageGlobalSram_eRestrictedRegion7Size,
    NEXUS_SageGlobalSram_eKernel,
    NEXUS_SageGlobalSram_eKernelSize,
    NEXUS_SageGlobalSram_eKernelSignature,
    NEXUS_SageGlobalSram_eDataSectionOffset,
    NEXUS_SageGlobalSram_eInstructionSectionSize,
    NEXUS_SageGlobalSram_eDataSectionSize,
    NEXUS_SageGlobalSram_eDecryptInfo,
    NEXUS_SageGlobalSram_eProcIn1,
    NEXUS_SageGlobalSram_eProcIn2 = NEXUS_SageGlobalSram_eProcIn1 + 4,
    NEXUS_SageGlobalSram_eProcIn3 = NEXUS_SageGlobalSram_eProcIn2 + 4,
    NEXUS_SageGlobalSram_eLastError = NEXUS_SageGlobalSram_eProcIn3 + 4,
    NEXUS_SageGlobalSram_eBootStatus,
    NEXUS_SageGlobalSram_eSageReservedVkl,
    NEXUS_SageGlobalSram_eBlVersion,
    NEXUS_SageGlobalSram_eSSFWVersion,
    NEXUS_SageGlobalSram_eVendorId,
    NEXUS_SageGlobalSram_eSwizzle0aVersion,
    NEXUS_SageGlobalSram_eSwizzle0aVariant,
    NEXUS_SageGlobalSram_eModuleId_StbOwnerId,
    NEXUS_SageGlobalSram_eHsmReserved,
    NEXUS_SageGlobalSram_eSecondTierKey = NEXUS_SageGlobalSram_eHsmReserved + 16,
    NEXUS_SageGlobalSram_eMarketId,
    NEXUS_SageGlobalSram_eMarketIdMask,
    NEXUS_SageGlobalSram_eEpoch,
    NEXUS_SageGlobalSram_eEpochMask,
    NEXUS_SageGlobalSram_eGeneralHeap,
    NEXUS_SageGlobalSram_eGeneralHeapSize,
    NEXUS_SageGlobalSram_eClientHeap,
    NEXUS_SageGlobalSram_eClientHeapSize,
    NEXUS_SageGlobalSram_eReset, /* DO NOT MOVE : this is used by BCHP and SAGE for restart/reset feature */
    NEXUS_SageGlobalSram_eKey0Select,
    NEXUS_SageGlobalSram_eSuspend,
    NEXUS_SageGlobalSram_eEpochSelect
} NEXUS_SageGlobalSram;


#define NEXUS_SageGlobalSram_GetOffset(FIELD) ((FIELD)*4)
#define NEXUS_SageGlobalSram_GetRegister(FIELD) (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + NEXUS_SageGlobalSram_GetOffset(FIELD))

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_SAGE_BOOT_TYPES_H_ */
