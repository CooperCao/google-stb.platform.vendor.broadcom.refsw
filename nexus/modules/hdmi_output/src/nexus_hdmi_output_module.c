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
#include "nexus_hdmi_output_image.h"

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
#include "nexus_sage.h"
#include "bsagelib.h"
#include "priv/nexus_sage_priv.h" /* get access to NEXUS_Sage_GetSageLib_priv() */
#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#endif
#endif

BDBG_MODULE(nexus_hdmi_output);


/* global module handle & data */
NEXUS_ModuleHandle g_NEXUS_hdmiOutputModule;
NEXUS_HdmiOutputModuleSettings g_NEXUS_hdmiOutputModuleSettings;
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
NEXUS_HdmiOutput_SageData g_NEXUS_hdmiOutputSageData;
NEXUS_HdmiOutputMemoryBlock g_hdcpTABlock;

typedef struct NEXUS_HdmiOutputImageHolder {
    const char *name;         /* printable name */
    uint8_t id;         /* firmware Id */
    NEXUS_HdmiOutputMemoryBlock *raw;
} NEXUS_HdmiOutputImageHolder;

static NEXUS_Error NEXUS_HdmiOutputModule_P_Img_Create(const char *id, void **ppContext, BIMG_Interface  *pInterface);
static void NEXUS_HdmiOutputModule_P_Img_Destroy(void *pContext);
static NEXUS_Error NEXUS_HdmiOutputModule_P_LoadTA(NEXUS_HdmiOutputImageHolder *holder, BIMG_Interface *img_interface, void *img_context);
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
    BSAGElib_Handle sagelibHandle;
	BSAGElib_ChipInfo chipInfo;
    NEXUS_SageStatus sageStatus;

    NEXUS_HdmiOutputImageHolder hdcpTAImg =
        {"HDCP2.2 TA", HDMI_OUTPUT_IMG_ID_Production, NULL};

    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;
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

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    errCode = NEXUS_HdmiOutputModule_P_Img_Create(NEXUS_CORE_IMG_ID_HDCP, &img_context, &img_interface);
    if (errCode != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot use IMG interface", __FUNCTION__));
        errCode = BERR_TRACE(errCode);
        if (img_context) {
            NEXUS_HdmiOutputModule_P_Img_Destroy(img_context);
        }
    }

    /* get status so we block until Sage is running */
    errCode = NEXUS_Sage_GetStatus(&sageStatus);
    if (errCode)
	{
		errCode = BERR_TRACE(errCode);
		goto error;
    }

    /* retrieve Sagelib Handle */
    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
    NEXUS_Sage_GetSageLib_priv(&sagelibHandle);
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);

	/* retrieve chip type */
	BSAGElib_GetChipInfo(sagelibHandle, &chipInfo);

    /* Specify which version of TA to load based on the type of chip on the board */
    if (chipInfo.chipType == BSAGElib_ChipType_eZS) {
        hdcpTAImg.id = HDMI_OUTPUT_IMG_ID_Development;
    }
	else {
		hdcpTAImg.id = HDMI_OUTPUT_IMG_ID_Production;
	}

    /************* load hdcp22 TA to memory*******************/
    hdcpTAImg.raw = &g_hdcpTABlock;
    errCode = NEXUS_HdmiOutputModule_P_LoadTA(&hdcpTAImg, &img_interface, img_context);
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
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_audio", "BHDM_PACKET_AUDIO", NEXUS_HdmiOutput_PrintAudioInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_avi", "BHDM_PACKET_AVI", NEXUS_HdmiOutput_PrintAviInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_vsi", "BHDM_PACKET_VSI", NEXUS_HdmiOutput_PrintVendorSpecificInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_drm", "BHDM_PACKET_DRM", NEXUS_HdmiOutput_PrintDrmInfoFramePacket);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "bhdm_packet_acr", "BHDM_PACKET_ACR_PRIV", NEXUS_HdmiOutput_PrintAcrPacket);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
error:
    if (img_context) {
        NEXUS_HdmiOutputModule_P_Img_Destroy(img_context);
    }
#endif


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
static NEXUS_Error NEXUS_HdmiOutputModule_P_Img_Create(
    const char *id,             /* Image Name */
    void **ppContext,           /* [out] Context */
    BIMG_Interface  *pInterface /* [out] Pointer to Image interface */
    )
{
    NEXUS_Error rc;
#if defined(NEXUS_MODE_driver)
    rc = Nexus_Core_P_Img_Create(id, ppContext, pInterface);
#else
    BSTD_UNUSED(id);
    *ppContext = HDMI_OUTPUT_IMAGE_Context;
    *pInterface = HDMI_OUTPUT_IMAGE_Interface;
    rc = NEXUS_SUCCESS;
#endif
    return rc;
}

static void NEXUS_HdmiOutputModule_P_Img_Destroy(
    void *pContext              /* Context returned by previous call */
    )
{
#if defined(NEXUS_MODE_driver)
    Nexus_Core_P_Img_Destroy(pContext);
#else
    BSTD_UNUSED(pContext);
#endif
}

/* Load a hdcp22 TA binary located on the file system into memory */
static NEXUS_Error NEXUS_HdmiOutputModule_P_LoadTA(
    NEXUS_HdmiOutputImageHolder *holder,
    BIMG_Interface *img_interface,
    void *img_context)
{
    void *image = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    BSTD_UNUSED(holder);
    BSTD_UNUSED(img_interface);
    BSTD_UNUSED(img_context);

    return NEXUS_SUCCESS;
#endif

    /* Prepare memory to load binfile */
    {
        uint32_t *size = NULL;

        /* Open file */
        rc = img_interface->open(img_context, &image, holder->id);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error opening HDCP_TA bin file (%s)", __FUNCTION__, holder->name));
            holder->raw->buf = NULL;
            holder->raw->len = 0;
            rc = NEXUS_SUCCESS;
            goto done;
        }

        /* Get buffer size */
        rc = img_interface->next(image, 0, (const void **)&size, sizeof(uint32_t));
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error while reading '%s' file to get size",
                      __FUNCTION__, holder->name));
            rc = BERR_TRACE(rc);
            goto done;
        }

        /* Allocate buffer to save data */
        {
            size_t alloc_size = (size_t)*size;

            BDBG_MSG(("alloc '%s' %u bytes", holder->name, (uint32_t) alloc_size));
            /* use SAGE allocator */
            NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
            holder->raw->buf = NEXUS_Sage_Malloc_priv(alloc_size);
            NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
            if (holder->raw->buf == NULL) {
                rc = BERR_OUT_OF_DEVICE_MEMORY;
                BDBG_ERR(("%s - Error allocating %u bytes memory for '%s' buffer",
                          __FUNCTION__, *size, holder->name));
                BERR_TRACE(rc);
                goto done;
            }

            holder->raw->len = *size;
        }
    }

    /* Load file into memory: read HDMI_OUTPUT_IMG_BUFFER_SIZE bytes at once */
    {
        uint32_t buff_size = holder->raw->len;
        uint8_t *buffer_ex = holder->raw->buf;
        unsigned chunk = 0;

        while (buff_size) {
            void *data = NULL;
            const uint16_t bytes_to_read =
                (buff_size >= HDMI_OUTPUT_IMG_BUFFER_SIZE) ? (HDMI_OUTPUT_IMG_BUFFER_SIZE - 1) : buff_size;

            rc = img_interface->next(image, chunk, (const void **)&data, bytes_to_read);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Error while reading '%s' file", __FUNCTION__, holder->name));
                rc = BERR_TRACE(rc);
                goto done;
            }

            BDBG_MSG(("%s - Read %u bytes from file (chunk: %u)", __FUNCTION__, bytes_to_read, chunk));
            BKNI_Memcpy(buffer_ex, data, bytes_to_read);
            buff_size -= bytes_to_read;
            buffer_ex += bytes_to_read;
            chunk++;
        }
    }

    /* Sync physical memory for all areas */
    NEXUS_Memory_FlushCache(holder->raw->buf, holder->raw->len);
    BDBG_MSG(("%s - '%s' Raw@%p,  size=%u", __FUNCTION__,
              holder->name, (void *)holder->raw->buf, (unsigned)holder->raw->len));

done:

    /* allocated memory block is freed in NEXUS_HdmiOutputModule_Uninit()? *****/
    if (image) {
        img_interface->close(image);
    }

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
