/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bads.h"
#include "bads_priv.h"
#include "brpc.h"
#include "brpc_plat.h"
#include "brpc_3255.h"
#include "bads_3255.h"
#include "bads_docsis_priv.h"

BDBG_MODULE(bads_docsis);

#define sizeInLong(x)	(sizeof(x)/sizeof(uint32_t))
#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	MX_ADS_CHANNELS			(24)



#define	DEV_MAGIC_ID			((BERR_ADS_ID<<16) | 0xFACE)

#define	ONE_TO_MEGA(x)			((float) ((double) x / 1000000.0))
#define	MEGA_TO_ONE(x)			(x * 1000000)
#define	HERTZ_TO_MHZ(x)			ONE_TO_MEGA(x)
#define	MHZ_TO_HERTZ(x)			MEGA_TO_ONE(x)




/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/
typedef struct BADS_P_docsis_Handle				*BADS_docsis_Handle;
typedef struct BADS_P_docsis_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle hChip;
	BREG_Handle hRegister;
	BINT_Handle hInterrupt;
	BRPC_DevId devId;
	BRPC_Handle hRpc;
	BADS_Version verInfo;
	unsigned int mxChnNo;				/* Number of total channels*/
	unsigned int BondedChnNo;			/* Number of bonded channels*/
	unsigned int num2Reserve;			/* Number of bonded channels to reserve*/
	BADS_ChannelHandle hAdsChn[MX_ADS_CHANNELS];
} BADS_P_docsis_Handle;

typedef struct BADS_P_docsis_ChannelHandle			*BADS_docsis_ChannelHandle;
typedef struct BADS_P_docsis_ChannelHandle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BADS_Handle hAds;
	unsigned int chnNo;
	BRPC_DevId devId;
	BRPC_Handle hRpc;
	BADS_CallbackFunc pCallback[BADS_Callback_eLast];
	void *pCallbackParam[BADS_Callback_eLast];
	BADS_ChannelSettings settings;
	BADS_LockStatus lockStatus;			/* current lock status */
	BKNI_MutexHandle mutex;				/* mutex to protect lock status*/
	uint32_t accCorrectedCount;         /* Accumulated corrected block count. Reset on every reset status */
	uint32_t accUncorrectedCount;       /* Accumulated un corrected block count. Reset on every reset status */
} BADS_P_docsis_ChannelHandle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/

static const BADS_Settings defDevSettings =
{
    BRPC_DevId_3255,						/* TODO: Is this required? */
    NULL,									/* BRPC handle, must be provided by application*/
    {
        BADS_docsis_Open,
        BADS_docsis_Close,
        BADS_docsis_Init,
        BADS_docsis_GetVersion,
        NULL, /* BADS_GetVersionInfo */        
        BADS_docsis_GetBondingCapability,
        BADS_docsis_GetTotalChannels,
        BADS_docsis_OpenChannel,
        BADS_docsis_CloseChannel,
        BADS_docsis_GetDevice,
        BADS_docsis_GetChannelDefaultSettings,
        BADS_docsis_GetStatus,
        BADS_docsis_GetLockStatus,
        BADS_docsis_GetSoftDecision,
        BADS_docsis_InstallCallback,
        NULL, /* BADS_GetDefaultAcquireParams */
        NULL, /* BADS_SetAcquireParams */
        NULL, /* BADS_GetAcquireParams */       
        BADS_docsis_Acquire,
        BADS_docsis_EnablePowerSaver,
        NULL, /* DisablePowerSaver */
        BADS_docsis_ProcessNotification,
        NULL, /* SetDaisyChain */
        NULL, /* GetDaisyChain */
        BADS_docsis_ResetStatus,
        NULL, /* GetInterruptEventHandle */
        NULL, /* ProcessInterruptEvent */
        NULL, /* Untune */
        NULL, /* RequestAsyncStatus */
        NULL, /* GetAsyncStatus */
        NULL, /* GetScanStatus */
        NULL, /* ReadSlave */
        NULL,  /* WriteSlave */
        NULL, /*SetScanParam*/
        NULL, /* GetScanParam*/
        NULL, /* BADS_RequestSpectrumAnalyzerData */
        NULL /* BADS_GetSpectrumAnalyzerData  */
    },
    false,
    BADS_TransportData_eSerial,
    NULL,
    NULL
};

/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/



/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/



/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BADS_docsis_Open(
	BADS_Handle *pAds,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const struct BADS_Settings *pDefSettings	/* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BADS_Handle hDev;
	unsigned int chnIdx;
	BADS_docsis_Handle hImplDev = NULL;


	BDBG_ENTER(BADS_docsis_Open);

	/* Alloc memory from the system heap */
	hDev = (BADS_Handle) BKNI_Malloc( sizeof( BADS_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BADS_P_Handle ) );

	hDev->magicId = DEV_MAGIC_ID;
	hDev->settings = *pDefSettings;

	hImplDev = (BADS_docsis_Handle) BKNI_Malloc( sizeof( BADS_P_docsis_Handle ) );
	if( hImplDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BADS_Open: BKNI_malloc() failed, impl"));
		goto done;
	}
	BKNI_Memset( hImplDev, 0x00, sizeof( BADS_P_docsis_Handle ) );


	hImplDev->magicId = DEV_MAGIC_ID;
	hImplDev->hChip = hChip;
	hImplDev->hRegister = hRegister;
	hImplDev->hInterrupt = hInterrupt;
	hImplDev->hRpc = pDefSettings->hGeneric;	/* For this device, we need the RPC handle */
	hImplDev->devId = pDefSettings->devId;
	hImplDev->mxChnNo = 0;	/* wait until the 3255 informs us */
	for( chnIdx = 0; chnIdx < MX_ADS_CHANNELS; chnIdx++ )
	{
		hImplDev->hAdsChn[chnIdx] = NULL;
	}
	hDev->pImpl = hImplDev;
	*pAds = hDev;


done:
	if( retCode != BERR_SUCCESS )
	{
		if( hDev != NULL )
		{
			BKNI_Free( hDev );
		}
		*pAds = NULL;
	}
	BDBG_LEAVE(BADS_docsis_Open);
	return( retCode );
}

BERR_Code BADS_docsis_Close(
	BADS_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BADS_docsis_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BKNI_Free( (void *) hDev->pImpl );
	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BADS_docsis_Close);
	return( retCode );
}


BERR_Code BADS_docsis_Init(
	BADS_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BERR_Code retVal;
	BADS_docsis_Handle hImplDev;
	BRPC_Param_ADS_GetTotalChannels outChnNoParam;
	BRPC_Param_ADS_GetVersion outVerParam;
	BRPC_Param_XXX_Get Param;

	BDBG_ENTER(BADS_docsis_Init);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	/* Get the version information for this device */
	CHK_RETCODE( retCode, BRPC_CallProc(hImplDev->hRpc, BRPC_ProcId_ADS_GetVersion, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outVerParam, sizeInLong(outVerParam), &retVal) );
	CHK_RETCODE( retCode, retVal );
	hImplDev->verInfo.majVer = outVerParam.majVer;
	hImplDev->verInfo.minVer = outVerParam.minVer;
	BDBG_MSG((" 3255 return version majVer 0x%x minVer 0x%x",hImplDev->verInfo.majVer, hImplDev->verInfo.minVer));

	/* Get the number of In-Band Downstream channels available for Video use */
	CHK_RETCODE( retCode, BRPC_CallProc(hImplDev->hRpc, BRPC_ProcId_ADS_GetTotalChannels, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outChnNoParam, sizeInLong(outChnNoParam), &retVal) );
	CHK_RETCODE( retCode, retVal );
	if (hImplDev->hRpc->rpc_disabled == true) /*return dummy channel num*/
		hImplDev->mxChnNo = 2;
	else
		hImplDev->mxChnNo = outChnNoParam.totalChannels;

	BDBG_MSG((" 3255 return total channels is %d", hImplDev->mxChnNo));

done:
	BDBG_LEAVE(BADS_docsis_Init);
	return( retCode );
}

BERR_Code BADS_3255_GetDefaultSettings(
	BADS_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BADS_docsis_GetDefaultSettings);

	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BADS_docsis_GetDefaultSettings);
	return( retCode );
}

BERR_Code BADS_docsis_GetVersion(
	BADS_Handle hDev,					/* [in] Device handle */
	BADS_Version *pVersion				/* [out] Returns version */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_Handle hImplDev;


	BDBG_ENTER(BADS_docsis_GetVersion);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	/* use saved data */
	*pVersion = hImplDev->verInfo;

	BDBG_LEAVE(BADS_docsis_GetVersion);
	return( retCode );
}

BERR_Code BADS_docsis_GetTotalChannels(
	BADS_Handle hDev,					/* [in] Device handle */
	unsigned int *totalChannels			/* [out] Returns total number downstream channels supported */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_Handle hImplDev;


	BDBG_ENTER(BADS_docsis_GetTotalChannels);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	/* use saved data */
	*totalChannels = hImplDev->mxChnNo;

	BDBG_LEAVE(BADS_docsis_GetTotalChannels);
	return( retCode );
}


BERR_Code BADS_docsis_GetBondingCapability(
	BADS_Handle hDev,					/* [in] Device handle */
	unsigned int *num			/* [out] Returns number of current bonded channels */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BERR_Code retVal;
	BADS_docsis_Handle hImplDev;
	BRPC_Param_ADS_GetBondingCapability outParam;
	BRPC_Param_XXX_Get Param;

	BDBG_ENTER(BADS_docsis_GetBondingCapability);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	/* Get the bonding capability of 3255 */
	CHK_RETCODE( retCode, BRPC_CallProc(hImplDev->hRpc, BRPC_ProcId_ADS_GetBondingCapability, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal) );
	CHK_RETCODE( retCode, retVal );

	*num = hImplDev->BondedChnNo = outParam.maxNum;
	BDBG_MSG((" 3255 BondingCapability: number of bonded channel is %d", hImplDev->BondedChnNo ));

done:
	BDBG_LEAVE(BADS_docsis_GetBondingCapability);
	return( retCode );
}


BERR_Code BADS_docsis_GetChannelDefaultSettings(
	BADS_Handle hDev,					/* [in] Device handle */
	unsigned int channelNo,				/* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_Handle hImplDev;

	BDBG_ENTER(BADS_docsis_GetChannelDefaultSettings);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	if( channelNo < hImplDev->mxChnNo )
	{
		pChnDefSettings->ifFreq = BADS_SETTINGS_IFFREQ;
		pChnDefSettings->autoAcquire = false;
		pChnDefSettings->fastAcquire = false;
	} else		/* fall through the error state */
		retCode = BERR_TRACE(BERR_INVALID_PARAMETER);

	BDBG_LEAVE(BADS_docsis_GetChannelDefaultSettings);
	return( retCode );
}

BERR_Code BADS_docsis_OpenChannel(
	BADS_Handle hDev,					/* [in] Device handle */
	BADS_ChannelHandle *phChn,			/* [out] Returns channel handle */
	unsigned int channelNo,					/* [in] Channel number to open */
	const struct BADS_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_Handle hImplDev;
	BADS_docsis_ChannelHandle hImplChnDev = NULL;
 	BADS_ChannelHandle hChnDev;
	BRPC_Param_ADS_OpenChannel	Param;
	BERR_Code retVal;


	BDBG_ENTER(BADS_docsis_OpenChannel);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );


	hImplDev = (BADS_docsis_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_ASSERT( hImplDev->hRpc );

	hChnDev = NULL;
	if( channelNo < hImplDev->mxChnNo )
	{
		if( hImplDev->hAdsChn[channelNo] == NULL )
		{
			/* Alloc memory from the system heap */
			hChnDev = (BADS_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_ChannelHandle ) );
			if( hChnDev == NULL )
			{
				retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
				BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed"));
				goto done;
			}
			BKNI_Memset( hChnDev, 0x00, sizeof( BADS_P_ChannelHandle ) );

			hChnDev->magicId = DEV_MAGIC_ID;
			hChnDev->hAds = hDev;

			hImplChnDev = (BADS_docsis_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_docsis_ChannelHandle ) );
			if( hImplChnDev == NULL )
			{
				retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
				BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed, impl"));
				goto done;
			}

			/* RPC open channel*/
			Param.devId = ((hImplDev->verInfo.minVer <= 0x9) ? BRPC_DevId_3255_DS0 : BRPC_DevId_ECM_DS0) + channelNo;
			if (pChnDefSettings) Param.ifFreq = pChnDefSettings->ifFreq;

			CHK_RETCODE( retCode, BRPC_CallProc(hImplDev->hRpc, BRPC_ProcId_ADS_OpenChannel, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
			CHK_RETCODE( retCode, retVal );


			hImplChnDev->chnNo = channelNo;
			if (pChnDefSettings) hImplChnDev->settings = *pChnDefSettings;
			/* Save copies for quicker access */
			hImplChnDev->devId = ((hImplDev->verInfo.minVer <= 0x9) ? BRPC_DevId_3255_DS0 : BRPC_DevId_ECM_DS0) + hImplChnDev->chnNo;
			hImplChnDev->hRpc = hImplDev->hRpc;
			CHK_RETCODE(retCode, BKNI_CreateMutex(&hImplChnDev->mutex));
			hImplDev->hAdsChn[channelNo] = hChnDev;
			hChnDev->pImpl = hImplChnDev;

			*phChn = hChnDev;
		}
		else
		{
			retCode = BERR_TRACE(BADS_ERR_NOTAVAIL_CHN_NO);
		}
	}
	else
	{
		retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

done:
	if( retCode != BERR_SUCCESS )
	{
		if( hChnDev != NULL )
		{
			BKNI_Free( hChnDev );
			hImplDev->hAdsChn[channelNo] = NULL;
		}
		if( hImplChnDev != NULL )
		{
			BKNI_Free( hImplChnDev );
		}
		*phChn = NULL;
	}
	BDBG_LEAVE(BADS_docsis_OpenChannel);
	return( retCode );
}

BERR_Code BADS_docsis_CloseChannel(
	BADS_ChannelHandle hChn				/* [in] Device channel handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_Handle hImplDev;
	BADS_docsis_ChannelHandle hImplChnDev;
	BADS_Handle hAds;
	unsigned int chnNo;
	BRPC_Param_ADS_CloseChannel Param;
	BERR_Code retVal;


	BDBG_ENTER(BADS_docsis_CloseChannel);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	Param.devId = hImplChnDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_CloseChannel, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

	hAds = hChn->hAds;
	hImplDev = (BADS_docsis_Handle) hAds->pImpl;
	BDBG_ASSERT( hImplDev );

	BKNI_DestroyMutex(hImplChnDev->mutex);
	chnNo = hImplChnDev->chnNo;
	hChn->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( hChn->pImpl );
	BKNI_Free( hChn );
	hImplDev->hAdsChn[chnNo] = NULL;

done:
	BDBG_LEAVE(BADS_docsis_CloseChannel);
	return( retCode );
}

BERR_Code BADS_docsis_GetDevice(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
	BADS_Handle *phDev					/* [out] Returns Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BADS_docsis_GetDevice);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	*phDev = hChn->hAds;

	BDBG_LEAVE(BADS_docsis_GetDevice);
	return( retCode );
}

BERR_Code BADS_docsis_Acquire(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
	BADS_InbandParam *ibParam			/* [in] Inband Parameters to use */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
	BRPC_Param_ADS_Acquire Param;
	BERR_Code retVal;


	BDBG_ENTER(BADS_docsis_Acquire);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	BKNI_AcquireMutex(hImplChnDev->mutex);
	hImplChnDev->lockStatus = BADS_LockStatus_eUnlocked;
	BKNI_ReleaseMutex(hImplChnDev->mutex);

	hImplChnDev->accCorrectedCount = 0;		/* Reset accumulated counts. */
	hImplChnDev->accUncorrectedCount = 0;

	BDBG_MSG(("%s: modType=%d, symbolRate=%d", BSTD_FUNCTION, ibParam->modType, ibParam->symbolRate));

	Param.devId = hImplChnDev->devId;
	Param.modType = ibParam->modType;
	Param.symbolRate = ibParam->symbolRate;
	Param.autoAcquire = hImplChnDev->settings.autoAcquire;
	Param.fastAcquire = hImplChnDev->settings.fastAcquire;
	CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_Acquire, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

done:
	BDBG_LEAVE(BADS_docsis_Acquire);
	return( retCode );
}

BERR_Code BADS_docsis_GetStatus(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
	BADS_Status *pStatus				/* [out] Returns status */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
	BRPC_Param_ADS_GetStatus outParam;
	BRPC_Param_XXX_Get Param;
	BERR_Code retVal;


	BDBG_ENTER(BADS_docsis_GetStatus);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	pStatus->isFecLock = pStatus->isQamLock = false;
	Param.devId = hImplChnDev->devId;

	CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_GetStatus, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal) );
	CHK_RETCODE( retCode, retVal);

	hImplChnDev->accCorrectedCount += outParam.correctedCount;
	hImplChnDev->accUncorrectedCount += outParam.uncorrectedCount;

	pStatus->isPowerSaverEnabled = outParam.isPowerSaverEnabled;
	pStatus->modType = outParam.modType;
	pStatus->ifFreq = outParam.ifFreq;
	pStatus->symbolRate = outParam.symbolRate;
	pStatus->isFecLock = outParam.isFecLock;
	pStatus->isQamLock = outParam.isQamLock;
	pStatus->correctedCount = outParam.correctedCount;
	pStatus->uncorrectedCount = outParam.uncorrectedCount;
	pStatus->snrEstimate = outParam.snrEstimate;
	pStatus->agcIntLevel = outParam.agcIntLevel;
	pStatus->agcExtLevel = outParam.agcExtLevel;
	pStatus->carrierFreqOffset = outParam.carrierFreqOffset;
	pStatus->carrierPhaseOffset = outParam.carrierPhaseOffset;
	pStatus->rxSymbolRate = outParam.rxSymbolRate;
	pStatus->interleaveDepth = outParam.interleaveDepth;
	pStatus->goodRsBlockCount = outParam.goodRsBlockCount;
	pStatus->berRawCount = outParam.berRawCount;
	pStatus->dsChannelPower = outParam.dsChannelPower;
	pStatus->mainTap = outParam.mainTap;
	pStatus->equalizerGain = outParam.equalizerGain*100;
	pStatus->postRsBER = outParam.postRsBER;
	pStatus->elapsedTimeSec = outParam.elapsedTimeSec;
	pStatus->isSpectrumInverted = outParam.spectralInversion;
	pStatus->accCorrectedCount = hImplChnDev->accCorrectedCount;
	pStatus->accUncorrectedCount = hImplChnDev->accUncorrectedCount;
done:
	BDBG_LEAVE(BADS_docsis_GetStatus);
	return( retCode );
}

BERR_Code BADS_docsis_ResetStatus(
	BADS_ChannelHandle hChn				/* [in] Device channel handle */
	)
{
	BADS_docsis_ChannelHandle hImplChnDev;

	BDBG_ENTER(BADS_docsis_ResetStatus);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	hImplChnDev->accCorrectedCount = 0;
	hImplChnDev->accUncorrectedCount = 0;
	BDBG_LEAVE(BADS_docsis_ResetStatus);
	return( BERR_SUCCESS );
}

BERR_Code BADS_docsis_GetLockStatus(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
#ifdef USE_RPC_GET_LOCK_STATUS
	BRPC_Param_ADS_GetLockStatus outParam;
	BRPC_Param_XXX_Get Param;
	BERR_Code retVal;
#endif

	BDBG_ENTER(BADS_docsis_GetLockStatus);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );
#ifndef USE_RPC_GET_LOCK_STATUS
	BKNI_AcquireMutex(hImplChnDev->mutex);
	*pLockStatus = hImplChnDev->lockStatus;
	BKNI_ReleaseMutex(hImplChnDev->mutex);
#else
	*pLockStatus = false;
	Param.devId = hImplChnDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_GetLockStatus, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal) );
	CHK_RETCODE( retCode, retVal );
	*pLockStatus = (outParam.isQamLock == true) && (outParam.isFecLock == true);
done:
#endif
	BDBG_LEAVE(BADS_docsis_GetLockStatus);
	return( retCode );

}

BERR_Code BADS_docsis_GetSoftDecision(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
	int16_t nbrToGet,					/* [in] Number values to get */
	int16_t *iVal,						/* [out] Ptr to array to store output I soft decision */
	int16_t *qVal,						/* [out] Ptr to array to store output Q soft decision */
	int16_t *nbrGotten					/* [out] Number of values gotten/read */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
	BRPC_Param_ADS_GetSoftDecisions outParam;
	BRPC_Param_XXX_Get Param;
	BERR_Code retVal;
	int cnt;
	unsigned int idx;

	BDBG_ENTER(BADS_docsis_GetSoftDecision);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	Param.devId = hImplChnDev->devId;
	*nbrGotten = 0;
	for( cnt = nbrToGet; cnt > 0; cnt -= MX_IQ_PER_GET )
	{

		CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_GetSoftDecision, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal ) );
		CHK_RETCODE( retCode, retVal );
		BDBG_MSG((" BRPC_ProcId_ADS_GetSoftDecision %d", outParam.nbrGotten));
		if (outParam.nbrGotten > (unsigned)cnt) outParam.nbrGotten = cnt;
		/* Copy one block at a time */
		for( idx = 0; idx < outParam.nbrGotten; idx++ )
		{
			*iVal++ = (int16_t) outParam.iVal[idx];
			*qVal++ = (int16_t) outParam.qVal[idx];
			*nbrGotten += 1;
		}
	}
done:
	BDBG_LEAVE(BADS_docsis_GetSoftDecision);
	return( retCode );
}

BERR_Code BADS_docsis_EnablePowerSaver(
	BADS_ChannelHandle hChn				/* [in] Device channel handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
	BRPC_Param_ADS_EnablePowerSaver Param;
	BERR_Code retVal;

	BDBG_ENTER(BADS_docsis_EnablePowerSaver);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	Param.devId = hImplChnDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hImplChnDev->hRpc, BRPC_ProcId_ADS_EnablePowerSaver, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

done:
	BDBG_LEAVE(BADS_docsis_EnablePowerSaver);
	return( retCode );
}


BERR_Code BADS_docsis_ProcessNotification(
	BADS_ChannelHandle hChn,				/* [in] Device channel handle */
	unsigned int event						/* [in] Event code and event data*/
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;
	BRPC_Notification_Event event_code;
	BADS_LockStatus lockStatus;

	BDBG_ENTER(BADS_docsis_ProcessNotification);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	BDBG_ASSERT( hImplChnDev->hRpc );

	event_code = event>>16;

	switch (event_code) {
		case BRPC_Notification_Event_LockStatusChanged:
			lockStatus = (event&BRPC_Qam_Lock) && (event&BRPC_Fec_Lock);
			BDBG_MSG((" ADS LockStatusChanged from 3255: lockStatus? %d", lockStatus));
			BKNI_AcquireMutex(hImplChnDev->mutex);
			hImplChnDev->lockStatus = lockStatus;
			BKNI_ReleaseMutex(hImplChnDev->mutex);
			{
				if( hImplChnDev->pCallback[BADS_Callback_eLockChange] != NULL )
				{
					(hImplChnDev->pCallback[BADS_Callback_eLockChange])(hImplChnDev->pCallbackParam[BADS_Callback_eLockChange] );
				}
			}
			break;
		default:
			BDBG_WRN((" unknown event code from 3255"));
			break;
	}

	BDBG_LEAVE(BADS_docsis_ProcessNotification);
	return( retCode );
}

BERR_Code BADS_docsis_InstallCallback(
	BADS_ChannelHandle hChn,			/* [in] Device channel handle */
	BADS_Callback callbackType,			/* [in] Type of callback */
	BADS_CallbackFunc pCallback,		/* [in] Function Ptr to callback */
	void *pParam						/* [in] Generic parameter send on callback */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BADS_docsis_ChannelHandle hImplChnDev;


	BDBG_ENTER(BADS_docsis_InstallCallback);
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hImplChnDev = (BADS_docsis_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );

	switch( callbackType )
	{
		case BADS_Callback_eLockChange:
			hImplChnDev->pCallback[callbackType] = pCallback;
			hImplChnDev->pCallbackParam[callbackType] = pParam;
			break;
		default:
			retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			break;
	}

	BDBG_LEAVE(BADS_docsis_InstallCallback);
	return( retCode );
}
