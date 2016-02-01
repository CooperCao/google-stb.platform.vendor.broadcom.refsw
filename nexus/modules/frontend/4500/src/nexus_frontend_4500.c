/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
* API Description:
*   API name: Frontend 4500
*    APIs for a BCM4500 Satellite Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_sds.h"
#include "blst_slist.h"
#include "priv/nexus_i2c_priv.h"
#include "bchp_4500.h"
#include "bcm4500ap.h"
#include "bsds.h"

BDBG_MODULE(nexus_frontend_4500);

BDBG_OBJECT_ID(NEXUS_4500Device);

typedef struct NEXUS_4500Device
{
    BDBG_OBJECT(NEXUS_4500Device)
    BLST_S_ENTRY(NEXUS_4500Device) node;
    NEXUS_4500Settings settings;
	BSDS_Handle sdsHandle;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    NEXUS_FrontendHandle handle;
} NEXUS_4500Device;

static BLST_S_HEAD(devList, NEXUS_4500Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

static void NEXUS_Frontend_P_4500_IsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4500_L1_isr(void *param1, int param2);
static void NEXUS_Frontend_P_4500_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_4500_DestroyDevice(NEXUS_4500Device *pDevice);
static void NEXUS_Frontend_P_4500_IsrCallback(void *pParam);

/***************************************************************************
Summary:
    Get the default settings for a BCM4500 frontend
See Also:
    NEXUS_Frontend_Open4500
 ***************************************************************************/
void NEXUS_Frontend_GetDefault4500Settings(
    NEXUS_4500Settings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = 0x7E;
	pSettings->diseqcI2cAddr = 0x08;	/* ISL6421 I2C address on BCM9740x */
	pSettings->diseqcChip = 6421;
}

/***************************************************************************
Summary:
    Open a handle to a BCM4500 device.
See Also:
    NEXUS_Frontend_Close4500
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open4500(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_4500Settings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_4500Device *pDevice=NULL;
    NEXUS_FrontendSdsSettings nsdsSettings;

    if (!pSettings || !pSettings->i2cDevice) {
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    for ( pDevice = BLST_S_FIRST(&g_deviceList);
          NULL != pDevice;
          pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);
        if ( pSettings->i2cDevice == pDevice->settings.i2cDevice &&
             pSettings->i2cAddr == pDevice->settings.i2cAddr )
        {
            break;
        }
    }

    if ( NULL == pDevice )
    {
        BSDS_Settings bsdsSettings;
        BREG_I2C_Handle i2cHandle;

        BDBG_MSG(("Opening new 4500 device"));
        pDevice = BKNI_Malloc(sizeof(NEXUS_4500Device));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_4500Device);
        pDevice->settings = *pSettings;
		
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);

		BSDS_GetDefaultSettings(&bsdsSettings);
		bsdsSettings.chipAddr = pSettings->i2cAddr;
		bsdsSettings.interruptEnableFunc = NEXUS_Frontend_P_4500_IsrControl_isr;;
		bsdsSettings.interruptEnableFuncParam =(void *)pDevice;
		errCode = BSDS_Open(&pDevice->sdsHandle,  NULL /* chip */, i2cHandle, &bsdsSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }

        /* Success opeining the SDS Device.  Connect Interrupt */
        errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                              NEXUS_Frontend_P_4500_L1_isr,
                                              (void *)pDevice,
                                              (int)pDevice->sdsHandle);
        if ( errCode != BERR_SUCCESS )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }

        BDBG_WRN(("Initializating 4500 core..."));
        /* Init the acquisition processor */
        errCode = BSDS_InitAp(pDevice->sdsHandle, bcm4500_ap_image);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }
        BDBG_WRN(("Initializating 4500 core... Done"));
		errCode = BSDS_SetGpioMode(pDevice->sdsHandle, false, false, false);
		if ( errCode )
		{
			errCode = BERR_TRACE(errCode);
			goto destroy_device;
		}

		/* must be done after InitAp because it uses the event */
        errCode = BSDS_GetInterruptEventHandle(pDevice->sdsHandle, &pDevice->isrEvent);
		if ( errCode )
		{
			errCode = BERR_TRACE(errCode);
			goto destroy_device;
		}
        pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_4500_IsrCallback, pDevice);
        if ( NULL == pDevice->isrCallback )
        {
            errCode = BERR_TRACE(NEXUS_UNKNOWN);
            goto destroy_device;
        }

        /* Success, add to device list */
        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found existing device"));
    }

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);

	NEXUS_Frontend_P_Sds_GetDefaultSettings(&nsdsSettings);
	nsdsSettings.devSettings = pDevice->settings;
    nsdsSettings.sdsHandle = pDevice->sdsHandle;
    nsdsSettings.closeFunction = NEXUS_Frontend_P_4500_CloseCallback;
    nsdsSettings.pCloseParam = pDevice;

    pDevice->handle = NEXUS_Frontend_P_Sds_Create(&nsdsSettings);
    if ( NULL == pDevice->handle )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_4500_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }

    return pDevice->handle;

destroy_device:
    NEXUS_Core_DisconnectInterrupt(pSettings->isrNumber);
    if (pDevice->sdsHandle) {
        BSDS_Close(pDevice->sdsHandle);
    }
	BKNI_Free(pDevice);

    return NULL;
}

/***************************************************************************
Summary:
    Probe to see if a BCM4500 device exists with the specified settings
See Also:
    NEXUS_Frontend_Open4500
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe4500(
    const NEXUS_4500Settings *pSettings,
    NEXUS_4500ProbeResults *pResults        /* [out] */
    )
{
    uint8_t sb;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BDBG_MSG(("Probing 4500 with I2C Address 0x%02x", pSettings->i2cAddr));

    /* read BCM4500_REVID register */
    sb = 1;
    if (NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr, BCM4500_HCTL1, &sb, 1) != BERR_SUCCESS)
        return NEXUS_INVALID_PARAMETER;

	if (NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr, BCM4500_REVID, &pResults->revision, 1) != BERR_SUCCESS)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	BDBG_WRN(("4500 detected with rev %02x", pResults->revision));
	return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_4500_L1_isr(void *param1, int param2)
{
    BSDS_Handle sdsHandle = (BSDS_Handle)param2;
    BSTD_UNUSED(param1);
    /* BDBG_MSG(("4500 L1 ISR")); */
    BSDS_HandleInterrupt_isr(sdsHandle);
}

static void NEXUS_Frontend_P_4500_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    NEXUS_4500Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);

    /* Mark handle as destroyed */
    if ( handle )
    {
		pDevice->handle = NULL;
    }

    NEXUS_Frontend_P_4500_DestroyDevice(pDevice);
}

static void NEXUS_Frontend_P_4500_DestroyDevice(NEXUS_4500Device *pDevice)
{
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);

	if ( NULL != pDevice->handle )
	{
		BDBG_ERR(("Frontend must be closed before destroying device"));
		BDBG_ASSERT(NULL == pDevice->handle);
	}

    BDBG_MSG(("Destroying 4500 device %p", (void *)pDevice));

    NEXUS_UnregisterEvent(pDevice->isrCallback);
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_4500Device, node);
    NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
	BSDS_Close(pDevice->sdsHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_4500Device);
    BKNI_Free(pDevice);
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 4500 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4500_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_4500Device *pDevice = (NEXUS_4500Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);
    if ( enable )
    {
        /* BDBG_MSG(("Enable 4500 Interrupt %u", pDevice->settings.isrNumber)); */
        NEXUS_Core_EnableInterrupt_isr(pDevice->settings.isrNumber);
    }
    else
    {
        /* BDBG_MSG(("Disable 4500 Interrupt %u", pDevice->settings.isrNumber)); */
        NEXUS_Core_DisableInterrupt_isr(pDevice->settings.isrNumber);
    }
}

static void NEXUS_Frontend_P_4500_IsrCallback(void *pParam)
{
    NEXUS_4500Device *pDevice = (NEXUS_4500Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4500Device);
	BSDS_ProcessInterruptEvent(pDevice->sdsHandle, NULL /* unused */);
}

NEXUS_Error NEXUS_Frontend_Read4500HostRegister( NEXUS_FrontendHandle handle, uint8_t address, uint32_t *pData )
{
    NEXUS_SdsDevice *sdsDevice = NEXUS_Frontend_P_GetSdsDevice(handle);
    return BSDS_ReadCoreRegister(sdsDevice->sdsHandle, address, false, pData);
}

