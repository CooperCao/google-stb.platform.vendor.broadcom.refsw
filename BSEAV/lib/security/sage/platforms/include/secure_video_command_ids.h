/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef __SECURE_VIDEO_COMMAND_IDS_H__
#define __SECURE_VIDEO_COMMAND_IDS_H__


#ifdef __cplusplus
extern "C" {
#endif

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
#define SECURE_VIDEO_VER_ID 0x00010000
#else
#define SECURE_VIDEO_VER_ID 0x00020008
#endif
#define SECURE_VIDEO_V3D_ALIGNMENT 4096
#define SECURE_VIDEO_V3D_SIZE 0x4000

/* Common for all commands */
#define SECURE_VIDEO_IN_VER 0
#define SECURE_VIDEO_OUT_RETCODE 0
#define SECURE_VIDEO_OUT_VER 1

/* For bvn_monitor_CommandId_eSetCores command */
#define SECURE_VIDEO_SETCORES_IN_ADD 1
#define SECURE_VIDEO_SETCORES_IN_V3D_OFFSET 2
#define SECURE_VIDEO_SETCORES_IN_V3D_SIZE 3
#define SECURE_VIDEO_SETCORES_BLOCK_CORELIST 0

/* For bvn_monitor_CommandId_eUpdateHeaps command */
#define SECURE_VIDEO_UPDATEHEAPS_IN_COUNT 1
#define SECURE_VIDEO_UPDATEHEAPS_BLOCK_START 0
#define SECURE_VIDEO_UPDATEHEAPS_BLOCK_SIZE 1

/* For bvn_monitor_CommandId_eToggle command */
#define SECURE_VIDEO_TOGGLE_IN_URR 1
#define SECURE_VIDEO_TOGGLE_IN_XRR 2
#define SECURE_VIDEO_TOGGLE_IN_KEYSLOT 3
#define SECURE_VIDEO_TOGGLE_BLOCK_DMAMEM 0

/* For bvm_monitor_CommandId_eSecureRemap */
#define SECURE_VIDEO_REMAP_IN_MEMC 1
#define SECURE_VIDEO_REMAP_BLOCK_ARRAY 0

/* LEGACY support, no longer used */
typedef enum secureVideo_Toggle_e {
    bvn_monitor_Command_eIgnore,
    bvn_monitor_Command_eEnable,
    bvn_monitor_Command_eDisable
}secureVideo_Toggle_e;

/*
 * List of supported SAGE commands
 * */

typedef enum secureVideo_CommandId_e {
    bvn_monitor_CommandId_eSetCores = 0x1,
    bvn_monitor_CommandId_eToggle, /* Legacy support, no longer used */
    bvn_monitor_CommandId_eUpdateHeaps,
    bvn_monitor_CommandId_eSecureRemap
}secureVideo_CommandId_e;

typedef enum secureVideo_IndicationType_e
{
    bvn_monitor_IndicationType_eLockdown = 1
} secureVideo_IndicationType_e;

typedef enum secureVideo_IndicationType_Lockdown_e
{
    bvn_monitor_Indication_Lockdown_eNone,
    bvn_monitor_Indication_Lockdown_eGeneral,
    bvn_monitor_Indication_Lockdown_eBvn,
    bvn_monitor_Indication_Lockdown_eHdmiRx,
    bvn_monitor_Indication_Lockdown_eHDR,
    bvn_monitor_Indication_Lockdown_eMacrovision
} secureVideo_IndicationType_Lockdown_e;

#ifdef __cplusplus
}
#endif

#endif /*__SECURE_VIDEO_COMMAND_IDS_H__*/
