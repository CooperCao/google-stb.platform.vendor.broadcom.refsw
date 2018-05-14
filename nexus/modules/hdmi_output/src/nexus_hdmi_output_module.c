/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_hdmi_output_module.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_memory.h"

#include "nexus_hdmi_output_init.h"

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
#include "nexus_sage.h"
#include "bsagelib.h"
#include "priv/nexus_sage_priv.h" /* get access to NEXUS_Sage_GetSageLib_priv() */
#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#endif
#endif
#include "bchp_pwr.h"

BDBG_MODULE(nexus_hdmi_output);


/* global module handle & data */
NEXUS_ModuleHandle g_NEXUS_hdmiOutputModule;
NEXUS_HdmiOutputModuleSettings g_NEXUS_hdmiOutputModuleSettings;
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
NEXUS_HdmiOutput_SageData g_NEXUS_hdmiOutputSageData;
NEXUS_HdmiOutputMemoryBlock g_hdcpTABlock;

static NEXUS_Error NEXUS_HdmiOutputModule_P_LoadTA(void);
#endif


void NEXUS_HdmiOutputModule_GetDefaultSettings(
    NEXUS_HdmiOutputModuleSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_HdmiOutputModule_Init(
    const NEXUS_HdmiOutputModuleSettings *pSettings  /* NULL will use default settings */
    )
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_Error errCode;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    NEXUS_SageStatus sageStatus;
#endif

    BDBG_ASSERT(NULL == g_NEXUS_hdmiOutputModule);

    #if NEXUS_HAS_SECURITY
    if (!pSettings || !pSettings->modules.security) {
        BDBG_WRN(("security module handle required"));
        return NULL;
    }
    #else
    if (!pSettings) {
        NEXUS_HdmiOutputModule_GetDefaultSettings(&g_NEXUS_hdmiOutputModuleSettings);
        pSettings = &g_NEXUS_hdmiOutputModuleSettings;
    }
    #endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* hdmi interface is very slow */
    moduleSettings.dbgPrint = NEXUS_HdmiOutputModule_Print;
    moduleSettings.dbgModules = "nexus_hdmi_output_debug";
    g_NEXUS_hdmiOutputModule = NEXUS_Module_Create("hdmi_output", &moduleSettings);
    if ( NULL == g_NEXUS_hdmiOutputModule )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }

    if (pSettings != &g_NEXUS_hdmiOutputModuleSettings) {
        g_NEXUS_hdmiOutputModuleSettings = *pSettings;
    }

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    /* Delay the process of populating until Sage is up */
    BKNI_Memset(&g_NEXUS_hdmiOutputSageData, 0, sizeof(g_NEXUS_hdmiOutputSageData));

    /* get status so we block until Sage is running */
    errCode = NEXUS_Sage_GetStatus(&sageStatus);
    if (errCode)
	{
		errCode = BERR_TRACE(errCode);
		goto error;
    }

    /************* load hdcp22 TA to memory*******************/
    errCode = NEXUS_HdmiOutputModule_P_LoadTA();
    if (errCode != NEXUS_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto error;
    }

#if NEXUS_HAS_HDMI_INPUT
    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
    errCode = NEXUS_HdmiInput_LoadHdcpTA_priv(g_hdcpTABlock.buf, g_hdcpTABlock.len);
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
    if (errCode != NEXUS_SUCCESS)
    {
        errCode = BERR_TRACE(errCode);
        goto error;
    }
#endif
#endif

    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_edid", "BHDM_EDID", NEXUS_HdmiOutput_PrintRxEdid);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_audio", "BAVC_HDMI_DEBUG", NEXUS_HdmiOutput_PrintAudioInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_avi", "BAVC_HDMI_DEBUG", NEXUS_HdmiOutput_PrintAviInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_vsi", "BAVC_HDMI_DEBUG", NEXUS_HdmiOutput_PrintVendorSpecificInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_drm", "BAVC_HDMI_DEBUG", NEXUS_HdmiOutput_PrintDrmInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_acr", "BHDM_PACKET_ACR_PRIV", NEXUS_HdmiOutput_PrintAcrPacket);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
error:
#endif
    BSTD_UNUSED(errCode);


    /* Success */
    return g_NEXUS_hdmiOutputModule;
}

void NEXUS_HdmiOutputModule_Uninit(void)
{
    BDBG_ASSERT(NULL != g_NEXUS_hdmiOutputModule);

    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_edid");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_packet_audio");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_packet_avi");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_packet_vsi");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_packet_drm");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "bhdm_packet_acr");

    NEXUS_Module_Destroy(g_NEXUS_hdmiOutputModule);
    BKNI_Memset(&g_NEXUS_hdmiOutputModuleSettings, 0, sizeof(g_NEXUS_hdmiOutputModuleSettings));

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    if (g_hdcpTABlock.buf != NULL)
    {
        /* memory allocated using NEXUS_Sage_Malloc_priv() can be freed by NEXUS_Memory_Free() */
        NEXUS_Memory_Free(g_hdcpTABlock.buf);
        g_hdcpTABlock.buf = NULL;
        g_hdcpTABlock.len = 0;
    }
#endif

    g_NEXUS_hdmiOutputModule = NULL;

    return;
}

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)


/* Load a hdcp22 TA binary located on the file system into memory */
static NEXUS_Error NEXUS_HdmiOutputModule_P_LoadTA(void)
{
    BSAGElib_Handle sagelibHandle;
    BSAGElib_ChipInfo chipInfo;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SageMemoryBlock blk = {0};
    NEXUS_SageImageHolder holder = {"HDCP2.2 TA", SAGE_IMAGE_FirmwareID_eSage_TA_HDCP22, NULL};

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    return NEXUS_SUCCESS;
#endif

    /* retrieve Sagelib Handle */
    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
    NEXUS_Sage_GetSageLib_priv(&sagelibHandle);

    /* retrieve chip type */
    /* NOTE: Function name doesn't make it clear the sage module lock isn't used...
    *        Potential for deadlock if this function changes */
    BSAGElib_GetChipInfo(sagelibHandle, &chipInfo);

    /* Specify which version of TA to load based on the type of chip on the board */
    if (chipInfo.chipType == BSAGElib_ChipType_eZS) {
        holder.id = SAGE_IMAGE_FirmwareID_eSage_TA_HDCP22_Development;
    }
    else {
        holder.id = SAGE_IMAGE_FirmwareID_eSage_TA_HDCP22;
    }
    holder.raw = &blk;

    rc = NEXUS_Sage_LoadImage_priv(&holder);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    g_hdcpTABlock.buf = blk.buf;
    g_hdcpTABlock.len = blk.len;

done:
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);

    return rc;
}
#endif

/* Update Rx supported hdcp version - use hdcp 2.2 if support */
void NEXUS_HdmiOutput_P_CheckHdcpVersion(NEXUS_HdmiOutputHandle output)
{
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    BHDM_HDCP_Version eHdcpVersion = output->eHdcpVersion;
    BERR_Code rc = BERR_SUCCESS;

    if ( output->rxState != NEXUS_HdmiOutputState_ePoweredOn ) {
        return;
    }

    if (output->hdcpVersionSelect == NEXUS_HdmiOutputHdcpVersion_e1_x) {
        eHdcpVersion = BHDM_HDCP_Version_e1_1;
    }
    else {
        rc = BHDM_HDCP_GetHdcpVersion(output->hdmHandle, &eHdcpVersion);
        /* default to HDCP 1.x if cannot read HDCP Version */
        if (rc != BERR_SUCCESS) {
            eHdcpVersion = BHDM_HDCP_Version_e1_1;
        }
    }

    /* close/re-open hdcplib handle for diff hdcp version if needed */
    if (output->eHdcpVersion != eHdcpVersion)
    {
        NEXUS_HdmiOutput_P_UninitHdcp(output);

        output->eHdcpVersion = eHdcpVersion;
        rc = NEXUS_HdmiOutput_P_InitHdcp(output);
        if (rc)
        {
            rc = BERR_TRACE(rc);
        };
    }
#else
    BSTD_UNUSED(output);
#endif
}

NEXUS_Error NEXUS_HdmiOutputModule_GetStatus_priv(NEXUS_HdmiOutputModuleStatus *pStatus)
{
    unsigned i;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    for (i=0;i<NEXUS_MAX_HDMI_OUTPUTS;i++) {
        unsigned clk=0, phy=0;
#if BCHP_PWR_RESOURCE_HDMI_TX_SRAM || BCHP_PWR_RESOURCE_HDMI_TX_1_SRAM
        unsigned sram=0;
#endif
        switch(i) {
            case 0:
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
                clk = BCHP_PWR_RESOURCE_HDMI_TX_CLK;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_PHY
                phy = BCHP_PWR_RESOURCE_HDMI_TX_PHY;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_SRAM
                sram = BCHP_PWR_RESOURCE_HDMI_TX_SRAM;
#endif
                break;
            case 1:
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
                clk = BCHP_PWR_RESOURCE_HDMI_TX_1_CLK;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
                phy = BCHP_PWR_RESOURCE_HDMI_TX_1_PHY;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_SRAM
                sram = BCHP_PWR_RESOURCE_HDMI_TX_1_SRAM;
#endif
                break;
        }
        if (clk) {
            pStatus->power.core[i].clock = BCHP_PWR_ResourceAcquired(g_pCoreHandles->chp, clk)?NEXUS_PowerState_eOn:NEXUS_PowerState_eOff;
        }
        if (phy) {
            pStatus->power.core[i].phy = BCHP_PWR_ResourceAcquired(g_pCoreHandles->chp, phy)?NEXUS_PowerState_eOn:NEXUS_PowerState_eOff;
        }
#if BCHP_PWR_RESOURCE_HDMI_TX_SRAM || BCHP_PWR_RESOURCE_HDMI_TX_1_SRAM
        if (sram) {
            pStatus->power.core[i].sram = BCHP_PWR_ResourceAcquired(g_pCoreHandles->chp, sram)?NEXUS_PowerState_eOn:NEXUS_PowerState_eOff;
        }
#endif
    }

    return NEXUS_SUCCESS;
}
