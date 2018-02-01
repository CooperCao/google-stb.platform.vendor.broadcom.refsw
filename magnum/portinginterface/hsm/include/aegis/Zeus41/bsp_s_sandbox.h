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

#ifndef ZEUS_VERSION
#warning "ZEUS_VERSION is not defined under /cygdrive/f/zeus_src_65nm/zeus_src/aegis/DownloadCode/Common/share/bsp_s_sandbox.h"
#endif

#ifndef BSP_S_SANDBOX_H__
#define BSP_S_SANDBOX_H__

#if (ZEUS_VERSION < ZEUS_4_0)

#warning "bsp_s_sandbox.h is for Zeus 4+ ONLY. Use bsp_s_webcpu.h instead."
#include "bsp_s_webcpu.h"

#else


typedef enum BCMD_Sandbox_DeviceType_e
{
    BCMD_Sandbox_DeviceType_Webcpu = 0x01,
    BCMD_Sandbox_DeviceType_Docsis = 0x02,
    BCMD_Sandbox_DeviceType_eMax
} BCMD_Sandbox_DeviceType_e;

typedef enum BCMD_Sandbox_Subcommand_e
{
    BCMD_Sandbox_Subcommand_Gisb = 0,
    BCMD_Sandbox_Subcommand_Biu_Arch = 1,
    BCMD_Sandbox_Subcommand_Memc_Arch = 2,
    BCMD_Sandbox_Subcommand_eMax
} BCMD_Sandbox_Subcommand_e;

typedef enum BCMD_SandboxConfig_InCmdField_e
{
    BCMD_SandboxConfig_InCmdField_eDeviceType           = ( 5 << 2) + 3,
    BCMD_SandboxConfig_InCmdField_eSubcommand           = ( 5 << 2) + 2,

    BCMD_SandboxConfig_InCmdField_eGisbCfgIdx           = ( 6 << 2) + 3,
    BCMD_SandboxConfig_InCmdField_eGisbHiStrtAddr       = ( 7 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbStrtAddr         = ( 8 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbHiEndAddr        = ( 9 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbEndAddr          = (10 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbClientsRd        = (11 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbClientsWr        = (12 << 2),
    BCMD_SandboxConfig_InCmdField_eGisbControl          = (13 << 2),

    BCMD_SandboxConfig_InCmdField_eBiuArchCfgIdx        = ( 6 << 2) + 3,
    BCMD_SandboxConfig_InCmdField_eBiuArchEndAddr       = ( 7 << 2),
    BCMD_SandboxConfig_InCmdField_eBiuArchStrtAddr      = ( 8 << 2),
    BCMD_SandboxConfig_InCmdField_eBiuArchClients       = ( 9 << 2),
    BCMD_SandboxConfig_InCmdField_eBiuArchControl       = (10 << 2),

    BCMD_SandboxConfig_InCmdField_eMemcArchCfgIdx       = ( 6 << 2) + 3,
    BCMD_SandboxConfig_InCmdField_eMemcArchControl      = ( 7 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchStrtAddrMsb  = ( 8 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchStrtAddr     = ( 9 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchEndAddrMsb   = (10 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchEndAddr      = (11 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb0       = (12 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb1       = (13 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb2       = (14 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb3       = (15 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb4       = (16 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb5       = (17 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb6       = (18 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchRdScb7       = (19 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb0       = (20 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb1       = (21 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb2       = (22 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb3       = (23 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb4       = (24 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb5       = (25 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb6       = (26 << 2),
    BCMD_SandboxConfig_InCmdField_eMemcArchWrScb7       = (27 << 2),
    BCMD_SandboxConfig_InCmdField_eMax

} BCMD_SandboxConfig_InCmdField_e;

typedef enum BCMD_SandboxConfig_OutCmdField_e
{
    BCMD_SandboxConfig_OutCmdField_eStatus        = (5<<2) + 3,
    BCMD_SandboxConfig_OutCmdField_eMax
} BCMD_SandboxConfig_OutCmdField_e;

#endif

#endif
