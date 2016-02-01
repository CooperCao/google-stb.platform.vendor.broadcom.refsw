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
#ifndef NEXUS_SCM_BOOT_TYPES_H_
#define NEXUS_SCM_BOOT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp_scpu_globalram.h"

/* SCM boot status */
typedef enum
{
    NEXUS_ScmBoot_eStarted = 0x0000,
    NEXUS_ScmBoot_eBlStarted = 0x0001,
    NEXUS_ScmBoot_eNotStarted = 0x00FF,
    NEXUS_ScmBoot_eError = 0xFFFF
} NEXUS_ScmBoot;


/* Global SRAM register indexes */
typedef enum
{
    NEXUS_ScmGlobalSram_eRequestBuffer = 0x20,
    NEXUS_ScmGlobalSram_eRequestBufferSize,
    NEXUS_ScmGlobalSram_eAckBuffer,
    NEXUS_ScmGlobalSram_eAckBufferSize,
    NEXUS_ScmGlobalSram_eResponseBuffer,
    NEXUS_ScmGlobalSram_eResponseBufferSize,
    NEXUS_ScmGlobalSram_eBootStatus,
    NEXUS_ScmGlobalSram_eLastError,
    NEXUS_ScmGlobalSram_eBlVersion,
    NEXUS_ScmGlobalSram_eReset /* DO NOT MOVE : this is used by BCHP and SCM for restart/reset feature */
} NEXUS_ScmGlobalSram;


#define NEXUS_ScmGlobalSram_GetOffset(FIELD) ((FIELD)*4)
#define NEXUS_ScmGlobalSram_GetRegister(FIELD) (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + NEXUS_ScmGlobalSram_GetOffset(FIELD))

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_SCM_BOOT_TYPES_H_ */
