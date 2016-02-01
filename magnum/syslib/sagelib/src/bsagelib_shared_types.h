/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#ifndef BSAGELIB_SHARED_TYPES_H_
#define BSAGELIB_SHARED_TYPES_H_

#include "bchp_scpu_globalram.h"


/* SAGE global SRAM register indexes */
typedef enum
{
    BSAGElib_GlobalSram_eRequestBuffer = 0x20,
    BSAGElib_GlobalSram_eRequestBufferSize,
    BSAGElib_GlobalSram_eAckBuffer,
    BSAGElib_GlobalSram_eAckBufferSize,
    BSAGElib_GlobalSram_eResponseBuffer,
    BSAGElib_GlobalSram_eResponseBufferSize,
    BSAGElib_GlobalSram_eRestrictedRegion1,
    BSAGElib_GlobalSram_eRestrictedRegion1Size,
    BSAGElib_GlobalSram_eRestrictedRegion2,
    BSAGElib_GlobalSram_eRestrictedRegion2Size,
    BSAGElib_GlobalSram_eRestrictedRegion3,
    BSAGElib_GlobalSram_eRestrictedRegion3Size,
    BSAGElib_GlobalSram_eRestrictedRegion4,
    BSAGElib_GlobalSram_eRestrictedRegion4Size,
    BSAGElib_GlobalSram_eRestrictedRegion5,
    BSAGElib_GlobalSram_eRestrictedRegion5Size,
    BSAGElib_GlobalSram_eSageLogBuffer,         /*BSAGElib_GlobalSram_eRestrictedRegion6*/
    BSAGElib_GlobalSram_eSageLogBufferSize,     /*BSAGElib_GlobalSram_eRestrictedRegion6Size*/
    BSAGElib_GlobalSram_eSageLogWriteCountLSB,  /*BSAGElib_GlobalSram_eRestrictedRegion7*/
    BSAGElib_GlobalSram_eSageLogWriteCountMSB,  /*BSAGElib_GlobalSram_eRestrictedRegion7Size*/
    BSAGElib_GlobalSram_eKernel,
    BSAGElib_GlobalSram_eKernelSize,
    BSAGElib_GlobalSram_eKernelSignature,
    BSAGElib_GlobalSram_eDataSectionOffset,
    BSAGElib_GlobalSram_eInstructionSectionSize,
    BSAGElib_GlobalSram_eDataSectionSize,
    BSAGElib_GlobalSram_eDecryptInfo,
    BSAGElib_GlobalSram_eProcIn1,
    BSAGElib_GlobalSram_eProcIn2 = BSAGElib_GlobalSram_eProcIn1 + 4,
    BSAGElib_GlobalSram_eProcIn3 = BSAGElib_GlobalSram_eProcIn2 + 4,
    BSAGElib_GlobalSram_eLastError = BSAGElib_GlobalSram_eProcIn3 + 4,
    BSAGElib_GlobalSram_eBootStatus,
    BSAGElib_GlobalSram_eSageReservedVkl,
    BSAGElib_GlobalSram_eBlVersion,
    BSAGElib_GlobalSram_eSSFWVersion,
    BSAGElib_GlobalSram_eVendorId,
    BSAGElib_GlobalSram_eSwizzle0aVersion,
    BSAGElib_GlobalSram_eSwizzle0aVariant,
    BSAGElib_GlobalSram_eModuleId_StbOwnerId,
    BSAGElib_GlobalSram_eHsmReserved,
    BSAGElib_GlobalSram_eSecondTierKey = BSAGElib_GlobalSram_eHsmReserved + 16,
    BSAGElib_GlobalSram_eMarketId,
    BSAGElib_GlobalSram_eMarketIdMask,
    BSAGElib_GlobalSram_eEpoch,
    BSAGElib_GlobalSram_eEpochMask,
    BSAGElib_GlobalSram_eGeneralHeap,
    BSAGElib_GlobalSram_eGeneralHeapSize,
    BSAGElib_GlobalSram_eClientHeap,
    BSAGElib_GlobalSram_eClientHeapSize,
    BSAGElib_GlobalSram_eReset, /* DO NOT MOVE : this is used by BCHP and SAGE for restart/reset feature */
    BSAGElib_GlobalSram_eKey0Select,
    BSAGElib_GlobalSram_eSuspend,
    BSAGElib_GlobalSram_eEpochSelect,
    BSAGElib_GlobalSram_eSageBlAr,
    BSAGElib_GlobalSram_eSageStatusFlags
} BSAGElib_GlobalSram;


#define BSAGElib_GlobalSram_GetOffset(FIELD) ((FIELD)*4)
#define BSAGElib_GlobalSram_GetRegister(FIELD) (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BSAGElib_GlobalSram_GetOffset(FIELD))

/* SAGE System Command IDs */
typedef enum {
    BSAGElib_SystemCommandId_eUnknown = 0,
    BSAGElib_SystemCommandId_ePlatformOpen,
    BSAGElib_SystemCommandId_ePlatformInit,
    BSAGElib_SystemCommandId_ePlatformClose,
    BSAGElib_SystemCommandId_eModuleInit,
    BSAGElib_SystemCommandId_eModuleUninit,
    BSAGElib_SystemCommandId_eModuleProcessCommand,
    BSAGElib_SystemCommandId_ePowerManagement
} BSAGElib_SystemCommandId;

#endif /* BSAGELIB_SHARED_TYPES_H_ */
