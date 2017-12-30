/***************************************************************************
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
*
* API Description:
*
***************************************************************************/

#include "nexus_core_module.h"
#include "nexus_power_management.h"
#include "priv/nexus_core.h"
#include "bchp_pwr.h"

BDBG_MODULE(nexus_power_management);

void NEXUS_PowerManagement_Init(void)
{
    /* deprecated. bchp_pwr handles everything */
}

void NEXUS_PowerManagement_Uninit(void)
{
    /* deprecated. bchp_pwr handles everything */
}

/* some BCHP_PWR_Resource ID's may not exist on all chips. provide compatibility here so the nexus enum mapping is simple.
This code assumes that 0 is not a valid ID. */
#ifndef BCHP_PWR_RESOURCE_VBI_DEC
#define BCHP_PWR_RESOURCE_VBI_DEC 0
#endif
#ifndef BCHP_PWR_RESOURCE_VDEC
#define BCHP_PWR_RESOURCE_VDEC 0
#endif
#ifndef BCHP_PWR_RESOURCE_AVD
#define BCHP_PWR_RESOURCE_AVD 0
#endif
#ifndef BCHP_PWR_RESOURCE_CCIR656_INPUT
#define BCHP_PWR_RESOURCE_CCIR656_INPUT 0
#endif
#ifndef BCHP_PWR_RESOURCE_HD_DVI_INPUT
#define BCHP_PWR_RESOURCE_HD_DVI_INPUT 0
#endif
#ifndef BCHP_PWR_RESOURCE_HDMI_INPUT
#define BCHP_PWR_RESOURCE_HDMI_INPUT 0
#endif
#ifndef BCHP_PWR_RESOURCE_ANALOG_FRONTEND
#define BCHP_PWR_RESOURCE_ANALOG_FRONTEND 0
#endif
#ifndef BCHP_PWR_RESOURCE_DIGITAL_FRONTEND
#define BCHP_PWR_RESOURCE_DIGITAL_FRONTEND 0
#endif
#ifndef BCHP_PWR_RESOURCE_XPT
#define BCHP_PWR_RESOURCE_XPT 0
#endif
#ifndef BCHP_PWR_RESOURCE_DMA
#define BCHP_PWR_RESOURCE_DMA 0
#endif
#ifndef BCHP_PWR_RESOURCE_HSM
#define BCHP_PWR_RESOURCE_HSM 0
#endif
#ifndef BCHP_PWR_RESOURCE_M2MC
#define BCHP_PWR_RESOURCE_M2MC 0
#endif
#ifndef BCHP_PWR_RESOURCE_M2MC_SRAM
#define BCHP_PWR_RESOURCE_M2MC_SRAM 0
#endif
#ifndef BCHP_PWR_RESOURCE_GRAPHICS3D
#define BCHP_PWR_RESOURCE_GRAPHICS3D 0
#endif
#ifndef BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM
#define BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM 0
#endif
#ifndef BCHP_PWR_RESOURCE_SID
#define BCHP_PWR_RESOURCE_SID 0
#endif
#ifndef BCHP_PWR_RESOURCE_SID_SRAM
#define BCHP_PWR_RESOURCE_SID_SRAM 0
#endif
#ifndef BCHP_PWR_RESOURCE_IMAGE_INPUT
#define BCHP_PWR_RESOURCE_IMAGE_INPUT 0
#endif
#ifndef BCHP_PWR_RESOURCE_RFM
#define BCHP_PWR_RESOURCE_RFM 0
#endif
#ifndef BCHP_PWR_RESOURCE_DFE
#define BCHP_PWR_RESOURCE_DFE 0
#endif
#ifndef BCHP_PWR_RESOURCE_DS_TOP
#define BCHP_PWR_RESOURCE_DS_TOP 0
#endif
#ifndef BCHP_PWR_RESOURCE_THD_TOP
#define BCHP_PWR_RESOURCE_THD_TOP 0
#endif

#if NEXUS_POWER_MANAGEMENT
static const unsigned g_powerId[NEXUS_PowerManagementCore_eMax] = {
    BCHP_PWR_RESOURCE_VBI_DEC, /* NEXUS_PowerManagementCore_eVbiDecoder */
    BCHP_PWR_RESOURCE_VDEC, /* NEXUS_PowerManagementCore_eVdec */
    BCHP_PWR_RESOURCE_AVD, /* NEXUS_PowerManagementCore_eVideoDecoder */
    BCHP_PWR_RESOURCE_CCIR656_INPUT, /* NEXUS_PowerManagementCore_eCcir656Input */
    BCHP_PWR_RESOURCE_HD_DVI_INPUT, /* NEXUS_PowerManagementCore_eHdDviInput */
    BCHP_PWR_RESOURCE_HDMI_INPUT, /* NEXUS_PowerManagementCore_eHdmiInput */
    BCHP_PWR_RESOURCE_ANALOG_FRONTEND, /* NEXUS_PowerManagementCore_eAnalogFrontend */
    BCHP_PWR_RESOURCE_DIGITAL_FRONTEND, /* NEXUS_PowerManagementCore_eDigitalFrontend */
    BCHP_PWR_RESOURCE_XPT, /* NEXUS_PowerManagementCore_eTransport */
    BCHP_PWR_RESOURCE_DMA, /* NEXUS_PowerManagementCore_eDma */
    BCHP_PWR_RESOURCE_HSM, /* NEXUS_PowerManagementCore_eHsm */
    BCHP_PWR_RESOURCE_M2MC, /* NEXUS_PowerManagementCore_eGraphics2D */
    BCHP_PWR_RESOURCE_M2MC_SRAM, /* NEXUS_PowerManagementCore_eGraphics2DMemory */
    BCHP_PWR_RESOURCE_GRAPHICS3D, /* NEXUS_PowerManagementCore_eGraphics3D */
    BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM, /* NEXUS_PowerManagementCore_eGraphics3DMemory */
    BCHP_PWR_RESOURCE_SID, /* NEXUS_PowerManagementCore_ePictureDecoder */
    BCHP_PWR_RESOURCE_SID_SRAM, /* NEXUS_PowerManagementCore_ePictureDecoderMemory */
    BCHP_PWR_RESOURCE_IMAGE_INPUT, /* NEXUS_PowerManagementCore_eImageInput */
    BCHP_PWR_RESOURCE_RFM /* NEXUS_PowerManagementCore_eRfm */
};
#endif

void NEXUS_PowerManagement_SetCoreState( NEXUS_PowerManagementCore core, bool poweredUp )
{
    NEXUS_LockModule();
#if NEXUS_POWER_MANAGEMENT
    if (core >= NEXUS_PowerManagementCore_eMax) {
        BDBG_ERR(("Invalid NEXUS_PowerManagementCore %d", core));
    }
    else if (poweredUp) {
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, g_powerId[core]);
    }
    else {
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, g_powerId[core]);
    }
#else
    BSTD_UNUSED(core);
    BSTD_UNUSED(poweredUp);
#endif
    NEXUS_UnlockModule();
}
