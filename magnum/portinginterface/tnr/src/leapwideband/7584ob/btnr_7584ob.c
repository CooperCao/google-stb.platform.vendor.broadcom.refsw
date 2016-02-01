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
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7584ob.h"
#include "btnr_7584ob_priv.h"

BDBG_MODULE(btnr_7584ob);

#define	DEV_MAGIC_ID					((BERR_TNR_ID<<16) | 0xFACE)
#define	BTNR_7584Ob_SETTINGS_I2CADDR	(0x66)			/* 7bit addr */
#define	BTNR_7584Ob_IF_FREQ	            44000000			/* 7bit addr */

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_7584Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BTNR_7584Ob_Settings *pSettings, /* [Input] settings structure */
	BHAB_Handle hHab                  /* [Input] Hab Handle */
	)
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7584Ob_Handle h7584ObDev;
    BTNR_P_7584Ob_Settings *pTnrImplData;
    BTNR_Handle hDev;
    uint16_t chipVer;
    uint32_t familyId, chipId;
    uint8_t apVer, minApVer, i=0;
    uint8_t hab[9] = HAB_MSG_HDR(BTNR_OOB_RF_INPUT_MODE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_Capabilities capabilities={0, NULL};
    bool isOobSupported = false;

    BDBG_ENTER(BTNR_7584Ob_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h7584ObDev = (BTNR_7584Ob_Handle) BKNI_Malloc( sizeof( BTNR_P_7584Ob_Handle ) );
    if( h7584ObDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h7584ObDev, 0x00, sizeof( BTNR_P_7584Ob_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584Ob_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h7584ObDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7584ObDev->magicId = DEV_MAGIC_ID;
    h7584ObDev->bPowerdown = true;
    pTnrImplData = &h7584ObDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
    pTnrImplData->ifFreq = pSettings->ifFreq;

    h7584ObDev->hHab = hHab;
    hDev->hDevImpl = (void *) h7584ObDev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_7584Ob_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_7584Ob_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_7584Ob_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_7584Ob_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_7584Ob_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_7584Ob_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_7584Ob_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_7584Ob_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_7584Ob_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_7584Ob_SetSettings;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_7584Ob_GetVersionInfo;

    hab[4] = pSettings->inputMode;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(h7584ObDev->hHab, hab, 9, hab, 0, false, true, 9));

    CHK_RETCODE(retCode, BHAB_GetApVersion(h7584ObDev->hHab, &familyId, &chipId, &chipVer, &apVer, &minApVer));

    if((chipId == 0x00) && (familyId == 0x7584))
        chipId = 0x75840;

    retCode =  BHAB_GetTunerChannels(h7584ObDev->hHab, &capabilities.totalTunerChannels);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}

    capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc(capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
    if(!capabilities.channelCapabilities){
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
    }

    retCode =  BHAB_GetCapabilities(h7584ObDev->hHab, &capabilities);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}
    for(i=0; i<capabilities.totalTunerChannels; i++)
    {
        if(capabilities.channelCapabilities[i].demodCoreType.aob) {
            isOobSupported = true;
            break;
        }
    }
    if(isOobSupported)
        *phDev = hDev;
    else
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("AOB not supported on %x", chipId));
    }

done:
    if(capabilities.channelCapabilities)
        BKNI_Free(capabilities.channelCapabilities);
    capabilities.channelCapabilities = NULL;
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *phDev = NULL;
    }
    BDBG_LEAVE(BTNR_7584Ob_Open);
    return( retCode );
}

BERR_Code BTNR_7584Ob_GetDefaultSettings(
	BTNR_7584Ob_Settings *pDefSettings	/* [out] Returns default setting */
	)
{
	BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
	pDefSettings->i2cAddr = BTNR_7584Ob_SETTINGS_I2CADDR;
	pDefSettings->ifFreq = BTNR_7584Ob_IF_FREQ;

	return BERR_SUCCESS;
}

