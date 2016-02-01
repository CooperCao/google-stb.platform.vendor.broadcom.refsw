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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "btfe.h"
#include "btfe_priv.h"
#if (BCHP_CHIP==7543)
#include "btfe_scd_priv.h"
#include "bchp_dfe_ucdec.h"
#include "btfe_scd_reg_hi_priv.h"
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#ifdef BTFE_SUPPORTS_SHARED_MEMORY
#include "bchp_sun_top_ctrl.h"
#endif
#define BTFE_CORE_ID		0x0
#define BTFE_CORE_TYPE		0x8
#define CORE_TYPE_GLOBAL	0x0
/* Defines raw HAB test mesg hdr (struct) */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)(((N)>>4)  & 0x3F))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))) }

#define CHK_RETCODE( rc, func )             \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#undef BTFE_SUPPORTS_SHARED_MEMORY
typedef enum BTFE_OpCodesDS{
        BTFE_eAcquire = 0x10,
        BTFE_eAcquireParamsWrite = 0x11,
        BTFE_eAcquireParamsRead = 0x91,
        BTFE_eResetStatus = 0x15,
        BTFE_eRequestAsyncStatus = 0x16,
        BTFE_eGetAsyncStatus = 0x96,
        BTFE_eGetScanStatus = 0x99,
        BTFE_eGetConstellation = 0xA3,
        BTFE_eGetVersion = 0xB9,
        BTFE_eGetVersionInfo = 0xBA,
        BTFE_ePowerCtrlOn = 0x19,
        BTFE_ePowerCtrlOff = 0x18,
        BTFE_ePowerCtrlRead = 0x98,
        BTFE_eResetSelectiveAsyncStatus = 0x55,
        BTFE_eRequestSelectiveAsyncStatus = 0x56,
        BTFE_eGetSelectiveAsyncStatusReadyType = 0xD7,
        BTFE_eGetSelectiveAsyncStatus = 0xD6
}BTFE_OpCodesDS;

BDBG_MODULE(btfe);

BDBG_OBJECT_ID(BTFE);
BDBG_OBJECT_ID(BTFE_Channel);

static const BTFE_Settings defDevSettings =
{
    NULL
};

static const BTFE_ChannelSettings defChnSettings =
{
    0
};

static const BTFE_AcquireParams defAcquireParams =
{
    BTFE_ModulationFormat_eVSB,
    true,
    false,
    0,
   {false, /* BTFE_ConfigSetTunerIF.bOverrideDefault */
    13500000, /* BTFE_ConfigSetTunerIF.center */
    0} /* BTFE_ConfigSetTunerIF.shift */
};

BERR_Code BTFE_P_EventCallback_isr(
    void * pParam1, int param2
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTFE_ChannelHandle hTFEChan = (BTFE_ChannelHandle) pParam1;
    BHAB_InterruptType event = (BHAB_InterruptType) param2;

    BDBG_ENTER(BTFE_P_EventCallback_isr);
    BDBG_OBJECT_ASSERT( hTFEChan, BTFE_Channel );

    switch (event) {
        case BHAB_Interrupt_eLockChange:
            {
                if( hTFEChan->pCallback[BTFE_Callback_eLockChange] != NULL )
                {
                    (hTFEChan->pCallback[BTFE_Callback_eLockChange])(hTFEChan->pCallbackParam[BTFE_Callback_eLockChange] );
                }
            }
            break;
        case BHAB_Interrupt_eNoSignal:
            {
                if( hTFEChan->pCallback[BTFE_Callback_eNoSignal] != NULL )
                {
                    (hTFEChan->pCallback[BTFE_Callback_eNoSignal])(hTFEChan->pCallbackParam[BTFE_Callback_eNoSignal] );
                }
            }
            break;
        case BHAB_Interrupt_eOdsAsyncStatusReady:
            {
                if( hTFEChan->pCallback[BTFE_Callback_eAsyncStatusReady] != NULL )
                {
                    (hTFEChan->pCallback[BTFE_Callback_eAsyncStatusReady])(hTFEChan->pCallbackParam[BTFE_Callback_eAsyncStatusReady] );
                }
            }
            break;
        default:
            BDBG_WRN((" unknown event code from leap"));
            break;
    }

    BDBG_LEAVE(BTFE_P_EventCallback_isr);
    return retCode;
}

#if (BCHP_CHIP==7543)
static const struct
{
   BTFE_ModulationFormat mformat;
   SCD_MOD_FORMAT nformat;
   } bformats[] = {
   {BTFE_ModulationFormat_eUnknown , SCD_MOD_FORMAT__UNKNOWN},
   {BTFE_ModulationFormat_eLast , SCD_MOD_FORMAT__LAST},
   {BTFE_ModulationFormat_eVSB , SCD_MOD_FORMAT__FAT_VSB},
   {BTFE_ModulationFormat_eAuto , SCD_MOD_FORMAT__FAT_AUTO},
   {BTFE_ModulationFormat_eQAM64 , SCD_MOD_FORMAT__FAT_QAM64},
   {BTFE_ModulationFormat_eQAM256 , SCD_MOD_FORMAT__FAT_QAM256}
};

/*******************************************************************************
*   BTFE_P_TimerFunc
*******************************************************************************/
BERR_Code BTFE_P_TimerFunc(void *myParam1, int myParam2)
{
    BERR_Code retCode = BERR_SUCCESS;
    BTFE_Handle hDev;
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
    BTFE_ConfigSetTunerIF ifData;
    BTFE_AcquireParams acquireParams;
#endif
    BSTD_UNUSED(myParam2);

    BDBG_ENTER(BTFE_P_TimerFunc);
    BDBG_ASSERT( myParam1 );
	hDev = (BTFE_Handle) myParam1;

#ifdef BTFE_SUPPORTS_SHARED_MEMORY
    if( hDev->pChannels[0]->acqParams->commandType ) {
        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eGetFatStatus )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Get FAT Status "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eGetFatStatus);
            hDev->pChannels[0]->fatStatus->flags = 0xffffffff;
            BTFE_GetStatus(hDev->pChannels[0], BTFE_StatusItem_eFAT, hDev->pChannels[0]->fatStatus);
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eAbortAcquire )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Aborting Acquire "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eAbortAcquire);
            BTFE_AbortAcq(hDev->pChannels[0]);
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eGetAcquireParams )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Get Acquire Parameters "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eGetAcquireParams);
            BTFE_GetAcquireParams(hDev->pChannels[0], &acquireParams);
            hDev->pChannels[0]->acqParams->acqConfig = acquireParams.acqConfig;
            hDev->pChannels[0]->acqParams->bandwidthConfig = acquireParams.bandwidthConfig;
            hDev->pChannels[0]->acqParams->bSpectrumInversion = acquireParams.bSpectrumInversion;
            hDev->pChannels[0]->acqParams->bSpectrumAutoDetect = acquireParams.bSpectrumAutoDetect;
            hDev->pChannels[0]->acqParams->agcDelay = acquireParams.agcDelay;
            hDev->pChannels[0]->acqParams->bCoChannelRejection = acquireParams.bCoChannelRejection;
            hDev->pChannels[0]->acqParams->bAdjChannelRejection = acquireParams.bAdjChannelRejection;
            hDev->pChannels[0]->acqParams->bMobileMode = acquireParams.bMobileMode;
            hDev->pChannels[0]->acqParams->bEnhancedMode = acquireParams.bEnhancedMode;
            hDev->pChannels[0]->acqParams->bLowPriority = acquireParams.bLowPriority;
            hDev->pChannels[0]->acqParams->uIfFrequency = acquireParams.uIfFrequency;
            hDev->pChannels[0]->acqParams->dataPolarity = acquireParams.dataPolarity;
            hDev->pChannels[0]->acqParams->errorPolarity = acquireParams.errorPolarity;
            hDev->pChannels[0]->acqParams->clockPolarity = acquireParams.clockPolarity;
            hDev->pChannels[0]->acqParams->syncPolarity = acquireParams.syncPolarity;
            hDev->pChannels[0]->acqParams->validPolarity = acquireParams.validPolarity;
            hDev->pChannels[0]->acqParams->burstMode = acquireParams.burstMode;
            hDev->pChannels[0]->acqParams->bGatedClockEnable = acquireParams.bGatedClockEnable;
            hDev->pChannels[0]->acqParams->bParallelOutputEnable = acquireParams.bParallelOutputEnable;
            hDev->pChannels[0]->acqParams->bHeaderEnable = acquireParams.bHeaderEnable;
            hDev->pChannels[0]->acqParams->bCableCardBypassEnable = acquireParams.bCableCardBypassEnable;
            hDev->pChannels[0]->acqParams->bFlipOrder = acquireParams.bFlipOrder;
            hDev->pChannels[0]->acqParams->bMpegOutputEnable = acquireParams.bMpegOutputEnable;
            hDev->pChannels[0]->acqParams->dataStrength = acquireParams.dataStrength;
            hDev->pChannels[0]->acqParams->errorStrength = acquireParams.errorStrength;
            hDev->pChannels[0]->acqParams->clockStrength = acquireParams.clockStrength;
            hDev->pChannels[0]->acqParams->syncStrength = acquireParams.syncStrength;
            hDev->pChannels[0]->acqParams->validStrength = acquireParams.validStrength;
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eSetAcquireParams )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Set Acquire Parameters "));
            acquireParams.acqConfig = hDev->pChannels[0]->acqParams->acqConfig;
            acquireParams.bandwidthConfig = hDev->pChannels[0]->acqParams->bandwidthConfig;
            acquireParams.bSpectrumInversion = hDev->pChannels[0]->acqParams->bSpectrumInversion;
            acquireParams.bSpectrumAutoDetect = hDev->pChannels[0]->acqParams->bSpectrumAutoDetect;
            acquireParams.agcDelay = hDev->pChannels[0]->acqParams->agcDelay;
            acquireParams.bCoChannelRejection = hDev->pChannels[0]->acqParams->bCoChannelRejection;
            acquireParams.bAdjChannelRejection = hDev->pChannels[0]->acqParams->bAdjChannelRejection;
            acquireParams.bMobileMode = hDev->pChannels[0]->acqParams->bMobileMode;
            acquireParams.bEnhancedMode = hDev->pChannels[0]->acqParams->bEnhancedMode;
            acquireParams.bLowPriority = hDev->pChannels[0]->acqParams->bLowPriority;
            acquireParams.uIfFrequency = hDev->pChannels[0]->acqParams->uIfFrequency;
            acquireParams.dataPolarity = hDev->pChannels[0]->acqParams->dataPolarity;
            acquireParams.errorPolarity = hDev->pChannels[0]->acqParams->errorPolarity;
            acquireParams.clockPolarity = hDev->pChannels[0]->acqParams->clockPolarity;
            acquireParams.syncPolarity = hDev->pChannels[0]->acqParams->syncPolarity;
            acquireParams.validPolarity = hDev->pChannels[0]->acqParams->validPolarity;
            acquireParams.burstMode = hDev->pChannels[0]->acqParams->burstMode;
            acquireParams.bGatedClockEnable = hDev->pChannels[0]->acqParams->bGatedClockEnable;
            acquireParams.bParallelOutputEnable = hDev->pChannels[0]->acqParams->bParallelOutputEnable;
            acquireParams.bHeaderEnable = hDev->pChannels[0]->acqParams->bHeaderEnable;
            acquireParams.bCableCardBypassEnable = hDev->pChannels[0]->acqParams->bCableCardBypassEnable;
            acquireParams.bFlipOrder = hDev->pChannels[0]->acqParams->bFlipOrder;
            acquireParams.bMpegOutputEnable = hDev->pChannels[0]->acqParams->bMpegOutputEnable;
            acquireParams.dataStrength = hDev->pChannels[0]->acqParams->dataStrength;
            acquireParams.errorStrength = hDev->pChannels[0]->acqParams->errorStrength;
            acquireParams.clockStrength = hDev->pChannels[0]->acqParams->clockStrength;
            acquireParams.syncStrength = hDev->pChannels[0]->acqParams->syncStrength;
            acquireParams.validStrength = hDev->pChannels[0]->acqParams->validStrength;
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eSetAcquireParams);
            BTFE_SetAcquireParams(hDev->pChannels[0], &acquireParams);
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eAcquire )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Acquiring "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eAcquire);
            BTFE_Acquire(hDev->pChannels[0], (BTFE_ModulationFormat) hDev->pChannels[0]->acqParams->modulationFormat);
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eGetonstellationData )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Get Constellation Data "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eGetonstellationData);
            BTFE_GetStatus(hDev->pChannels[0], BTFE_StatusItem_eConstellationData, hDev->pChannels[0]->constellationData);
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eGetTunerIF )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Get Tuner IF "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eGetTunerIF);
            BTFE_GetConfig(hDev->pChannels[0], BTFE_ConfigItem_eSetTunerIF, &ifData);
            hDev->pChannels[0]->acqParams->bOverrideDefault = ifData.bOverrideDefault;
            hDev->pChannels[0]->acqParams->center = ifData.center;
            hDev->pChannels[0]->acqParams->shift = ifData.shift;
            hDev->pChannels[0]->acqParams->bSpectInvertMode = ifData.bSpectInvertMode;
        }

        if( hDev->pChannels[0]->acqParams->commandType & BTFE_BBS_CommandType_eSetTunerIF )
        {
            BDBG_MSG(("BTFE_P_TimerFunc: Set Tuner IF  "));
            hDev->pChannels[0]->acqParams->commandType &= (~BTFE_BBS_CommandType_eSetTunerIF);
            ifData.bOverrideDefault = hDev->pChannels[0]->acqParams->bOverrideDefault;
            ifData.center = hDev->pChannels[0]->acqParams->center;
            ifData.shift = hDev->pChannels[0]->acqParams->shift;
            ifData.bSpectInvertMode = hDev->pChannels[0]->acqParams->bSpectInvertMode;
            BTFE_SetConfig(hDev->pChannels[0], BTFE_ConfigItem_eSetTunerIF, &ifData);
        }
    }
	BDBG_LEAVE(BTFE_P_TimerFunc);
#endif
	return retCode;
}
#endif

/******************************************************************************
 BTFE_Open()
******************************************************************************/
BERR_Code BTFE_Open(
   BTFE_Handle *hTFE, /* [out] BTFE handle */
   BCHP_Handle hChip, /* [in] chip handle */
   BREG_Handle hReg, /* [in] register handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BTFE_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BTFE_Handle hDev = NULL;
   int i;
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
   BTMR_Settings sTimerSettings;
#endif

   BDBG_ENTER(BTFE_Open);

   BDBG_ASSERT(hChip);
   BDBG_ASSERT(hInterrupt);
   BDBG_ASSERT(hReg);
   BDBG_ASSERT(pDefSettings);
   BDBG_ASSERT(hTFE);

   /* allocate memory for the handle */
   hDev = (BTFE_Handle)BKNI_Malloc(sizeof(BTFE_P_Handle));

   BKNI_Memset(hDev, 0, sizeof(*hDev));
   BDBG_OBJECT_SET(hDev, BTFE);
   /* initialize our handle */
   hDev->hRegister = hReg;
   hDev->hInterrupt = hInterrupt;
   hDev->hChip = hChip;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)&pDefSettings, sizeof(BTFE_Settings));

   hDev->pChannels = (BTFE_P_ChannelHandle **)BKNI_Malloc(BTFE_MAX_CHANNELS * sizeof(BTFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   hDev->totalChannels = BTFE_MAX_CHANNELS;
   hDev->hHab = (BHAB_Handle) pDefSettings->hGeneric;    /* For this device, we need the HAB handle */
   hDev->devId = BHAB_DevId_eVSB0; /* Here the device id is always defaulted to channel 0. */
   for (i = 0; i < BTFE_MAX_CHANNELS; i++)
      hDev->pChannels[i] = NULL;

#ifdef BTFE_SUPPORTS_SHARED_MEMORY
	/* Create timer for status lock check */

   sTimerSettings.type = BTMR_Type_ePeriodic;
   sTimerSettings.cb_isr = (BTMR_CallbackFunc)BTFE_P_TimerFunc;
   sTimerSettings.pParm1 = (void*)hDev;
   sTimerSettings.parm2 = 0;
   sTimerSettings.exclusive = true;
   retCode = BTMR_CreateTimer (pDefSettings->hTmr, &hDev->hTimer, &sTimerSettings);
   if ( retCode != BERR_SUCCESS )
   {
      BDBG_ERR(("BTFE_Open: Create Timer Failed"));
      goto done;
   }
   BTMR_StartTimer(hDev->hTimer, 25000);   /* the timer is in Micro second */
#endif

   *hTFE = hDev;

   BDBG_LEAVE(BTFE_Open);
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
done:
#endif
   return retCode;
}


/******************************************************************************
 BTFE_Close()
******************************************************************************/
BERR_Code BTFE_Close(
   BTFE_Handle hTFE /* [in] BTFE handle */
)
{
   BTFE_P_Handle *hDev;

   BDBG_OBJECT_ASSERT(hTFE, BTFE);
   BDBG_ENTER(BTFE_Close);

   hDev = (BTFE_P_Handle *)(hTFE->pChannels);
#if (BCHP_CHIP==7543)
   BTFE_P_ScdCleanup();
#endif
   BKNI_Free((void*)hTFE->pChannels);
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
   BTMR_StopTimer(hTFE->hTimer);
   BTMR_DestroyTimer(hTFE->hTimer);
#endif

   BDBG_OBJECT_DESTROY(hTFE, BTFE);
   BKNI_Free((void*)hTFE);
   hTFE = NULL;

   BDBG_LEAVE(BTFE_Close);

   return BERR_SUCCESS;
}

/******************************************************************************
 BTFE_GetDefaultSettings()
******************************************************************************/
BERR_Code BTFE_GetDefaultSettings(
   BTFE_Settings *pChnDefSettings /* [out] default channel settings */
)
{
   *pChnDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetTotalChannels()
******************************************************************************/
BERR_Code BTFE_GetTotalChannels(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t *totalChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(hTFE);
   *totalChannels = BTFE_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BTFE_GetChannelDefaultSettings(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t chnNo, /* [in] channel number */
   BTFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(hTFE);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_OpenChannel()
******************************************************************************/
BERR_Code BTFE_OpenChannel(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   BTFE_ChannelHandle *pChannelHandle, /* [out] BTFE channel handle */
   uint32_t chnNo, /* [in] channel number */
   const BTFE_ChannelSettings *pSettings /* [in] channel settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BTFE_ChannelSettings cs;
   BTFE_P_ChannelHandle *ch;
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
   uint32_t bufSrc;
   void *cached_ptr, *tmpAddress;
#endif
    unsigned int event=0;

   BDBG_OBJECT_ASSERT(hTFE, BTFE);
   BDBG_ASSERT(chnNo < BTFE_MAX_CHANNELS);

   if (pSettings == NULL)
      BTFE_GetChannelDefaultSettings(hTFE, chnNo, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BTFE_P_ChannelHandle *)BKNI_Malloc(sizeof(BTFE_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memset((void*)ch, 0, sizeof(BTFE_P_ChannelHandle));
   BDBG_OBJECT_SET(ch, BTFE_Channel);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BTFE_ChannelSettings));

   ch->channel = (uint8_t)chnNo;
   ch->pDevice = hTFE;
   ch->hHab = hTFE->hHab;
   ch->pFatHandle = NULL;
   hTFE->pChannels[chnNo] = ch;

   ch->devId = (BHAB_DevId) BHAB_DevId_eVSB0;
   BHAB_InstallInterruptCallback( ch->hHab,  ch->devId, BTFE_P_EventCallback_isr , (void *)ch, event);
   /* create events */
   retCode = BKNI_CreateEvent(&(ch->hLockStateChangeEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(ch->hScanEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);

#ifdef BCHP_PWR_RESOURCE_DFE
   BCHP_PWR_AcquireResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
#if (BCHP_CHIP==7543)
   retCode = BINT_CreateCallback(&(ch->hLockStatusChangeCb), hTFE->hInterrupt, BCHP_INT_ID_DFE_LOCK_STATUS_CHANGE, BTFE_P_LockStatusChange_isr, (void*)ch, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(ch->hScanCb), hTFE->hInterrupt, BCHP_INT_ID_DFE_SCAN, BTFE_P_Scan_isr, (void*)ch, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
#endif
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
   tmpAddress = (BTFE_BBS_SharedMemory *)BMEM_AllocAligned(hTFE->settings.hHeap, sizeof(BTFE_BBS_SharedMemory), 0, 0 );
   if( tmpAddress == NULL )
   {
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BTFE_OpenChannel: BMEM_AllocAligned() failed"));
      goto done;
   }
   BMEM_ConvertAddressToCached(hTFE->settings.hHeap, tmpAddress, &cached_ptr);
   BKNI_Memset(cached_ptr, 0x00, sizeof( BTFE_BBS_SharedMemory ) );
   ch->sharedMemory = cached_ptr;
   BMEM_FlushCache(hTFE->settings.hHeap, ch->sharedMemory, sizeof( BTFE_BBS_SharedMemory ) );
   BMEM_ConvertAddressToOffset(hTFE->settings.hHeap, tmpAddress, &bufSrc );
   BREG_Write32(hTFE->hRegister, BCHP_SUN_TOP_CTRL_SPARE_CTRL, bufSrc);

   tmpAddress = (BTFE_BBS_AcquireParams *)BMEM_AllocAligned(hTFE->settings.hHeap, sizeof(BTFE_BBS_AcquireParams), 0, 0 );
   if( tmpAddress == NULL )
   {
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BTFE_OpenChannel: BMEM_AllocAligned() failed"));
      goto done;
   }
   BMEM_ConvertAddressToCached(hTFE->settings.hHeap, tmpAddress, &cached_ptr);
   BKNI_Memset(cached_ptr, 0x00, sizeof( BTFE_BBS_AcquireParams ) );
   ch->acqParams = cached_ptr;
   BMEM_FlushCache(hTFE->settings.hHeap, ch->acqParams, sizeof( BTFE_BBS_AcquireParams ) );
   BMEM_ConvertAddressToOffset(hTFE->settings.hHeap, tmpAddress, &bufSrc );
   ch->sharedMemory->acquireParams = bufSrc;

   tmpAddress = (BTFE_StatusFAT *)BMEM_AllocAligned(hTFE->settings.hHeap, sizeof(BTFE_StatusFAT), 0, 0 );
   if( tmpAddress == NULL )
   {
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BTFE_OpenChannel: BMEM_AllocAligned() failed"));
      goto done;
   }

   BMEM_ConvertAddressToCached(hTFE->settings.hHeap, tmpAddress, &cached_ptr);
   BKNI_Memset(cached_ptr, 0x00, sizeof( BTFE_StatusFAT ) );
   ch->fatStatus = cached_ptr;
   BMEM_FlushCache(hTFE->settings.hHeap, ch->fatStatus, sizeof( BTFE_StatusFAT ) );
   BMEM_ConvertAddressToOffset(hTFE->settings.hHeap, tmpAddress, &bufSrc );
   ch->sharedMemory->fatStatus = bufSrc;

   tmpAddress = (BTFE_StatusConstellationData *)BMEM_AllocAligned(hTFE->settings.hHeap, sizeof(BTFE_StatusConstellationData), 0, 0 );
   if( tmpAddress == NULL )
   {
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BTFE_OpenChannel: BMEM_AllocAligned() failed"));
      goto done;
   }

   BMEM_ConvertAddressToCached(hTFE->settings.hHeap, tmpAddress, &cached_ptr);
   BKNI_Memset(cached_ptr, 0x00, sizeof( BTFE_StatusConstellationData ) );
   ch->constellationData = cached_ptr;
   BMEM_FlushCache(hTFE->settings.hHeap, ch->constellationData, sizeof( BTFE_StatusConstellationData ) );
   BMEM_ConvertAddressToOffset(hTFE->settings.hHeap, tmpAddress, &bufSrc );
   ch->sharedMemory->constellationData = bufSrc;
#endif
   ch->previousAcquireParams = defAcquireParams;
   ch->bPowerdown = true;
   *pChannelHandle = ch;

#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif

#ifdef BTFE_SUPPORTS_SHARED_MEMORY
done:
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_CloseChannel()
******************************************************************************/
BERR_Code BTFE_CloseChannel(
   BTFE_ChannelHandle hTFEChan /* [in] BTFE channel handle */
)
{
   int i;

   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ENTER(BTFE_CloseChannel);
#if (BCHP_CHIP==7543)
   if (BTFE_P_ScdStop(hTFEChan->pFatHandle) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

   if (BTFE_P_ScdCloseFat(hTFEChan->pFatHandle) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
#endif
   BKNI_DestroyEvent(hTFEChan->hLockStateChangeEvent);
   BKNI_DestroyEvent(hTFEChan->hScanEvent);
#if (BCHP_CHIP==7543)
   BINT_DestroyCallback(hTFEChan->hLockStatusChangeCb);
   BINT_DestroyCallback(hTFEChan->hScanCb);
#endif
   BHAB_UnInstallInterruptCallback(hTFEChan->hHab, hTFEChan->devId);

   for (i = 0; i < BTFE_MAX_CHANNELS; i++)
   {
      if(hTFEChan->pDevice->pChannels[i] == hTFEChan)
      {
        hTFEChan->pDevice->pChannels[i] = NULL;
        BDBG_OBJECT_DESTROY(hTFEChan, BTFE_Channel);
        BKNI_Free(hTFEChan);
      }
   }

   BDBG_LEAVE(BTFE_CloseChannel);
   return BERR_SUCCESS;
}

BERR_Code BTFE_InstallCallback(
     BTFE_ChannelHandle hTFEChan,   /* [in] Device channel handle */
     BTFE_Callback callbackType,    /* [in] Type of callback */
     BTFE_CallbackFunc pCallbackFunc,   /* [in] Function Ptr to callback */
     void *pParam                   /* [in] Generic parameter send on callback */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ENTER(BTFE_InstallCallback);

    switch( callbackType )
    {
        case BTFE_Callback_eLockChange:
            hTFEChan->pCallback[callbackType] = pCallbackFunc;
            hTFEChan->pCallbackParam[callbackType] = pParam;
            break;
        case BTFE_Callback_eNoSignal:
            hTFEChan->pCallback[callbackType] = pCallbackFunc;
            hTFEChan->pCallbackParam[callbackType] = pParam;
            break;
        case BTFE_Callback_eAsyncStatusReady:
            hTFEChan->pCallback[callbackType] = pCallbackFunc;
            hTFEChan->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

   BDBG_LEAVE(BTFE_InstallCallback);
   return retCode;
}

/******************************************************************************
 BTFE_Initialize()
******************************************************************************/
BERR_Code BTFE_Initialize(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   void *pImage /* [in] pointer to 8051  microcode image */
)
{
   BERR_Code retCode = BERR_SUCCESS;
#if (BCHP_CHIP==7543)
   BTFE_ChannelHandle pChn;
   int i;
#endif

   BSTD_UNUSED(pImage); /* TBD... */
   BDBG_OBJECT_ASSERT(hTFE, BTFE);
#if (BCHP_CHIP==7543)
#ifdef BCHP_PWR_RESOURCE_DFE
   BCHP_PWR_AcquireResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
#if (BCHP_CHIP==7543)
   /*scd initialize*/
   if (BTFE_P_ScdInitialize(0, hTFE->hRegister) != BERR_SUCCESS)
   {
#ifdef BCHP_PWR_RESOURCE_DFE
      BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
   }
#endif
   /*do the necessary init for scd and save scd handles in pChn struct*/
   for (i = 0; i < BTFE_MAX_CHANNELS; i++)
   {
      pChn = hTFE->pChannels[i];

      BTFE_P_DisableIrq(pChn, BTFE_IRQ_ALL);
#if (BCHP_CHIP==7543)
      if (BTFE_P_ScdOpenFat(0, &pChn->pFatHandle) != BERR_SUCCESS)
      {
#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
         return BERR_TRACE(BTFE_ERR_SCD_ERROR);
      }

      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eFATData, (void*)(&(pChn->settings.fatData)));
      if (retCode != BERR_SUCCESS)
      {
#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
         BDBG_ERR(("unable to set BTFE_ConfigItem_eFATData"));
         return BERR_TRACE(retCode);
      }

      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eAcquisition, (void*)(&(pChn->settings.acquisition)));
      if (retCode != BERR_SUCCESS)
      {
#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
         BDBG_ERR(("unable to set BTFE_ConfigItem_eAcquisition"));
         return BERR_TRACE(retCode);
      }
#endif
   }

#ifdef BCHP_PWR_RESOURCE_DFE
   BCHP_PWR_ReleaseResource(hTFE->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
#endif
   return retCode;
}

/******************************************************************************
 BTFE_GetVersion()
******************************************************************************/
BERR_Code BTFE_GetVersion(
    BTFE_Handle hTFE, /* [in]  TFE handle */
    BFEC_VersionInfo *pVersion /* [out]  Returns FW version information */
)
{
#if (BCHP_CHIP==7543)
   SCD_VERSION version;
#endif
   BDBG_OBJECT_ASSERT(hTFE, BTFE);
   BKNI_Memset(pVersion, 0x00, sizeof( BFEC_VersionInfo ) );
#if (BCHP_CHIP==7543)
   if (BTFE_P_ScdGetVersion(SCD_ITEM__FIRMWARE, 0, &version) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

    pVersion->majorVersion = (uint32_t)version.major;
    pVersion->minorVersion = (uint32_t)version.minor;
    pVersion->buildType = (uint32_t)(version.customer >> 4);
    pVersion->buildId = (uint32_t)(version.customer & 0xF);
#endif
   return BERR_SUCCESS;
}

/******************************************************************************
 BTFE_GetDefaultAcquireParams()
******************************************************************************/
BERR_Code BTFE_GetDefaultAcquireParams(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_AcquireParams *acquireParams    /* [out] DefaultAcquire Parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(acquireParams);

   *acquireParams = defAcquireParams;

   return retCode;
}

/******************************************************************************
 BTFE_GetAcquireParams()
******************************************************************************/
BERR_Code BTFE_GetAcquireParams(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_AcquireParams *acquireParams    /* [out] Acquire Parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(acquireParams);

   *acquireParams = hTFEChan->previousAcquireParams;

   return retCode;
}

/******************************************************************************
 BTFE_SetAcquireParams()
******************************************************************************/
BERR_Code BTFE_SetAcquireParams(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   const BTFE_AcquireParams *acquireParams  /* [in] Acquire Parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[31] = HAB_MSG_HDR(BTFE_eAcquireParamsWrite, 0x1A, BTFE_CORE_TYPE);
#if (BCHP_CHIP==7543)
   SCD_CONFIG__FAT_DATA fat_data;
   SCD_CONFIG__ACQUISITION acquisition;
#endif
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(acquireParams);
#if (BCHP_CHIP==7543)
   BDBG_ASSERT(hTFEChan->pFatHandle);
   fat_data.dataPolarity =  acquireParams->dataPolarity;
   fat_data.errorPolarity =  acquireParams->errorPolarity;
   fat_data.clockPolarity =  acquireParams->clockPolarity;
   fat_data.syncPolarity =  acquireParams->syncPolarity;
   fat_data.validPolarity =  acquireParams->validPolarity;
   fat_data.BurstMode =  acquireParams->burstMode;
   fat_data.GatedClockEnable =  acquireParams->bGatedClockEnable;
   fat_data.ParallelOutputEnable =  acquireParams->bParallelOutputEnable;
   fat_data.HeaderEnable =  acquireParams->bHeaderEnable;
   fat_data.CableCardBypassEnable =  acquireParams->bCableCardBypassEnable;
   fat_data.FlipOrder =  acquireParams->bFlipOrder;
   fat_data.MpegOutputEnable =  acquireParams->bMpegOutputEnable;
   fat_data.dataStrength =  acquireParams->dataStrength;
   fat_data.errorStrength =  acquireParams->errorStrength;
   fat_data.clockStrength =  acquireParams->clockStrength;
   fat_data.syncStrength =  acquireParams->syncStrength;
   fat_data.validStrength =  acquireParams->validStrength;

   if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__FAT_DATA, (void*)&fat_data, sizeof(fat_data)) == SCD_RESULT__OK)
      retCode = BERR_SUCCESS;
   else
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

    acquisition.acqConfig = acquireParams->acqConfig;
    acquisition.bandWidthConfig = acquireParams->bandwidthConfig;
    acquisition.agcDelay = acquireParams->agcDelay;
    acquisition.TuneMode = 0; /*not used*/
    acquisition.bSpectrumInversion = acquireParams->bSpectrumInversion;
    acquisition.bSpectrumAutoDetect= acquireParams->bSpectrumAutoDetect;
    acquisition.bCoChannelRejection = acquireParams->bCoChannelRejection;
    acquisition.bAdjChannelRejection = acquireParams->bAdjChannelRejection;
    acquisition.bMobileMode = acquireParams->bMobileMode;
    acquisition.bEnhancedMode = acquireParams->bEnhancedMode;
    acquisition.bLowPriority = acquireParams->bLowPriority;
    acquisition.ifFrequency = acquireParams->uIfFrequency;
    acquisition.bLegacyAGC = 0; /*not used*/
    acquisition.TuneMode = SCD_TUNE_MODE__APP;

    if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__ACQUISITION, (void*)&acquisition, sizeof(acquisition)) == SCD_RESULT__OK)
       retCode = BERR_SUCCESS;
    else
       return BERR_TRACE(BTFE_ERR_SCD_ERROR);
#endif
    buf[5] = (acquireParams->modulationFormat<<4) | (acquireParams->spectrumInversion << 1) | acquireParams->spectrumAutoDetect;
    buf[7] = acquireParams->tunerIf.overrideDefault;
    buf[8] = (uint32_t)(acquireParams->tunerIf.center>>24);
    buf[9] = (uint32_t)(acquireParams->tunerIf.center>>16);
    buf[0xA] = (uint32_t)(acquireParams->tunerIf.center>>8);
    buf[0xB] = (uint32_t)acquireParams->tunerIf.center;
    buf[0xC] = (uint32_t)(acquireParams->tunerIf.shift>>24);
    buf[0xD] = (uint32_t)(acquireParams->tunerIf.shift>>16);
    buf[0xe] = (uint32_t)(acquireParams->tunerIf.shift>>8);
    buf[0xF] = (uint32_t)acquireParams->tunerIf.shift;
    buf[0x14] = (uint32_t)(acquireParams->ifFrequency>>24);
    buf[0x15] = (uint32_t)(acquireParams->ifFrequency>>16);
    buf[0x16] = (uint32_t)(acquireParams->ifFrequency>>8);
    buf[0x17] = (uint32_t)acquireParams->ifFrequency;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 31, buf, 0, false, true, 31));
    hTFEChan->previousAcquireParams = *acquireParams;
done:
    return retCode;
}

BERR_Code BTFE_Acquire(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
    BTFE_AcquireParams *acquireParams     /* [out] Acquire Parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[5] = HAB_MSG_HDR(BTFE_eAcquire, 0, BTFE_CORE_TYPE);
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ENTER(BTFE_Acquire);

   BSTD_UNUSED(acquireParams);

   CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 0, false, true, 5 ));
   CHK_RETCODE(retCode, BHAB_EnableLockInterrupt(hTFEChan->hHab, hTFEChan->devId, true));
   BDBG_LEAVE(BTFE_Acquire);

done:
   return retCode;
}

#if (BCHP_CHIP==7543)
/******************************************************************************
 BTFE_Acquire()
******************************************************************************/
BERR_Code BTFE_Acquire(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_ModulationFormat format /* [in] acquisition parameters */
)
{
#if (BCHP_CHIP==7543)
   SCD_MOD_FORMAT scd_format;
#endif
   if ((format == 0) || (format > BTFE_ModulationFormat_eQAM256))
      return BERR_INVALID_PARAMETER;

   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(hTFEChan->pFatHandle);
#if (BCHP_CHIP==7543)
   scd_format = bformats[format].nformat;
   if (scd_format == SCD_MOD_FORMAT__UNKNOWN)
      return BERR_TRACE(BERR_NOT_SUPPORTED);

   /* stop current acquisition */
   BTFE_AbortAcq(hTFEChan);
#endif
   /* if acquire mode is search scan, turn on interrupt, and clear operationDone */
   if (hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eSearchScan)
   {
      uint8_t operationDone;

      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);

      operationDone = 0;
#if (BCHP_CHIP==7543)
      if (format == BTFE_ModulationFormat_eVSB)
      {
         if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_VSB_STATUS_AREA+32, 1, &operationDone) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
      }
#endif
   }
#if (BCHP_CHIP==7543)
   if (BTFE_P_ScdStart(hTFEChan->pFatHandle,  scd_format) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
#endif
   if ((hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eDirectedAcquire) ||
            (hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eFullAcquire))
      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_LOCK_CHANGE);

   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_AbortAcq()
******************************************************************************/
BERR_Code BTFE_AbortAcq(
   BTFE_ChannelHandle hTFEChan /* [in] BTFE Handle */
)
{
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(hTFEChan->pFatHandle);

   /* disable lock change interrupt */
   BTFE_P_DisableIrq(hTFEChan, BTFE_IRQ_LOCK_CHANGE | BTFE_IRQ_SCAN);

   if (BTFE_P_ScdStop(hTFEChan->pFatHandle) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

   return BERR_SUCCESS;
}


/***************************************************************************
 BTFE_GetStatus()
****************************************************************************/
BERR_Code BTFE_GetStatus(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_StatusItem item, /* [in] status item to read */
   void *pData /* [out] status to read */
)
{
   SCD_STATUS__AGC_INDICATOR s_fat_agc;
   SCD_STATUS__FAT s_fat_demod;
   SCD_STATUS__TUNER_AGC s_tuner_agc;
   SCD_STATUS__BERT s_bert;
   SCD_STATUS__CONSTELLATION_DATA s_constellation_data;
   SCD_STATUS__MEMORY_READ s_memory_read;
   uint8_t buffer[28];


   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(hTFEChan->pFatHandle);
   BDBG_ASSERT(pData != NULL);

   switch(item)
   {
      case BTFE_StatusItem_eVSB:
         {
            BTFE_StatusVSB *pStatusVSB = (BTFE_StatusVSB *)pData;
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_VSB_STATUS_AREA, 10, buffer) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusVSB->operationDone= buffer[8];
            pStatusVSB->confirmVSB = buffer[9] ? true: false;

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eAGCIndicator:
         {
            BTFE_StatusAGCIndicator *pStatusAGCIndicator = (BTFE_StatusAGCIndicator*)pData;

            s_fat_agc.Flags = pStatusAGCIndicator->flags;

            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__AGC_INDICATOR, (void*)&s_fat_agc, sizeof(s_fat_agc)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusAGCIndicator->flags = s_fat_agc.Flags;
            pStatusAGCIndicator->sdm1 = s_fat_agc.SDM1;
            pStatusAGCIndicator->sdm2 = s_fat_agc.SDM2;
            pStatusAGCIndicator->sdmx = s_fat_agc.SDMX;
            pStatusAGCIndicator->adcMin = s_fat_agc.AdcMin;
            pStatusAGCIndicator->adcMax = s_fat_agc.AdcMax;
            pStatusAGCIndicator->adcPower = s_fat_agc.AdcPower;
            pStatusAGCIndicator->pdetPower = s_fat_agc.PdetPower;
            pStatusAGCIndicator->vidPower = s_fat_agc.VidPower;
            pStatusAGCIndicator->vdcLevel = s_fat_agc.vdcLevel;

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eTunerAGC:
         {
            BTFE_StatusTunerAGC *pStatusTunerAGC = (BTFE_StatusTunerAGC*)pData;

            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__TUNER_AGC, (void*)&s_tuner_agc, sizeof(s_tuner_agc)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusTunerAGC->piAGCData =  s_tuner_agc.agcData;

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eBERT:
         {
            BTFE_StatusBERT *pStatusBERT = (BTFE_StatusBERT*)pData;

            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__BERT, (void*)&s_bert, sizeof(s_bert)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            s_bert.LockStatus = pStatusBERT->lockStatus;
            s_bert.ErrorCount = pStatusBERT->errorCount;

            return BERR_SUCCESS;
         }
     case BTFE_StatusItem_eConstellationData:
         {
            BTFE_StatusConstellationData *pStatusConstellationData = (BTFE_StatusConstellationData*)pData;

            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__CONSTELLATION_DATA, (void*)&s_constellation_data, sizeof(s_constellation_data)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusConstellationData->constX = s_constellation_data.constX;
            pStatusConstellationData->constY = s_constellation_data.constY;

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eFAT:
         {
            BTFE_StatusFAT *pStatusFat = (BTFE_StatusFAT*)pData;

            s_fat_demod.Flags = pStatusFat->flags;
            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__FAT, (void*)&s_fat_demod, sizeof(s_fat_demod)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusFat->flags = s_fat_demod.Flags;
            pStatusFat->bStarted = s_fat_demod.Started;
            pStatusFat->lockStatus = s_fat_demod.LockStatus;
            pStatusFat->demodulationFormat = s_fat_demod.DemodulationFormat;
            pStatusFat->recommendedTimeoutValue = s_fat_demod.RecommendedTimeoutValue;
            pStatusFat->spectrumPolarity = s_fat_demod.SpectrumPolarity;
            pStatusFat->equalizerSNR = s_fat_demod.EqualizerSNR;
            pStatusFat->timingOffset = s_fat_demod.TimingOffset;
            pStatusFat->pilotOffset = s_fat_demod.PilotOffset;
            pStatusFat->rSUncorrectableErrorsA = s_fat_demod.RSUncorrectableErrorsA;
            pStatusFat->rSCorrectableErrorsA = s_fat_demod.RSCorrectableErrorsA;
            pStatusFat->numRSpacketsA = s_fat_demod.NumRSpacketsA;
            pStatusFat->coarseOffset = s_fat_demod.CoarseOffset;
            pStatusFat->iAGCGain = s_fat_demod.IAGCGain;
            pStatusFat->dUR = s_fat_demod.DUR;
            pStatusFat->pilotAmplitude = s_fat_demod.PilotAmplitude;
            pStatusFat->eqCursor = s_fat_demod.EqCursor;
            pStatusFat->pilotEstimate = s_fat_demod.PilotEstimate;
            pStatusFat->aTSMstate = s_fat_demod.ATSMstate;
            pStatusFat->dFSstate = s_fat_demod.DFSstate;
            pStatusFat->dFSpolarity = s_fat_demod.DFSpolarity;
            pStatusFat->qAMinterleaverMode = s_fat_demod.QAMinterleaverMode;
            pStatusFat->acbState = s_fat_demod.ACBState;
            pStatusFat->acbStatus = s_fat_demod.ACBStatus;
            pStatusFat->acbTimer = s_fat_demod.ACBTimer;
            pStatusFat->acbAcqTime = s_fat_demod.ACBAcqTime;
            pStatusFat->acbNumReacqs = s_fat_demod.ACBNumReacqs;
            pStatusFat->carrierOffset = s_fat_demod.CarrierOffset;
            pStatusFat->agcSettleTime = s_fat_demod.AgcSettleTime;
            pStatusFat->sampleFrequency = s_fat_demod.SampleFrequency;
            pStatusFat->targetIfFrequency = s_fat_demod.TargetIfFrequency;
            pStatusFat->symbolRate = s_fat_demod.SymbolRate;
            pStatusFat->normalizedIF = s_fat_demod.NormalizedIF;
            pStatusFat->ews = s_fat_demod.ews;
            pStatusFat->partialReception = s_fat_demod.partialReception;
            pStatusFat->cellId = s_fat_demod.cellId;
            pStatusFat->demodSpectrum = s_fat_demod.demodSpectrum;
            pStatusFat->SignalQualityIndex = s_fat_demod.SignalQualityIndex;
            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eMemoryRead:
         {
            BTFE_StatusMemoryRead *pStatusMemoryRead = (BTFE_StatusMemoryRead*)pData;

            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__MEMORY_READ, (void*)&s_memory_read, sizeof(s_memory_read)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            s_memory_read.offset = pStatusMemoryRead->offset;
            s_memory_read.size = pStatusMemoryRead->size;
            s_memory_read.values = pStatusMemoryRead->uiValues;

            return BERR_SUCCESS;
         }
      default:
         BDBG_ERR(("Invalid Status Item\n"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   return BERR_SUCCESS;
}


/***************************************************************************
 BTFE_SetConfig()
****************************************************************************/
BERR_Code BTFE_SetConfig(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_ConfigItem item, /* [in] specifies which config item to set */
   void *pData /* [in] pointer to configuration data structure */
)
{
   SCD_CONFIG__BERT bert;
   SCD_CONFIG__GPIO gpio;
   SCD_CONFIG__FAT_DATA fat_data;
   SCD_CONFIG__FAT_AGC fat_agc;
   SCD_XPROP_TUNER_IF__DATA if_data;
   SCD_CONFIG__ACQUISITION acquisition;
   SCD_CONFIG__RF_OFFSET rfOffset;
   uint32_t val32 = 0;

   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(hTFEChan->pFatHandle);
   BDBG_ASSERT(pData != NULL);

   switch(item)
   {
      case BTFE_ConfigItem_eBERT:
         {
            BTFE_ConfigBERT *pConfigBERT = (BTFE_ConfigBERT*)pData;
            bert.HeaderRemoval =  pConfigBERT->headerRemoval;
            bert.InputSelect =  pConfigBERT->inputSelect;
            bert.PN_Inversion =  pConfigBERT->bPNInversion;
            bert.PN_Selection =  pConfigBERT->bPNSelection;
            bert.ON_Flag =  pConfigBERT->bONFlag;
            bert.SyncErrorThreshold =  pConfigBERT->syncErrorThreshold;
            bert.SyncAcquireCounter =  pConfigBERT->syncAcquireCounter;
            bert.SyncLossCounter =  pConfigBERT->syncLossCounter;
            bert.WindowSize =  pConfigBERT->windowSize;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__BERT, (void*)&bert, sizeof(bert)) == SCD_RESULT__OK)
            {
                  hTFEChan->settings.bert = *pConfigBERT;
                  return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eGPIO:
         {
            BTFE_ConfigGPIO *pConfigGPIO = (BTFE_ConfigGPIO*)pData;
            gpio.ownershipMask =  pConfigGPIO->ownershipMask;
            gpio.inputMask =  pConfigGPIO->inputMask;
            gpio.outputType =  pConfigGPIO->outputType;

            val32 = (uint32_t)(pConfigGPIO->i2cSpeedSelect & 0x7F);
            BREG_Write32(hTFEChan->pDevice->hRegister, BCHP_DFE_UCDEC_UC_CONTROL, val32);

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__GPIO, (void*)&gpio, sizeof(gpio)) == SCD_RESULT__OK)
            {
               hTFEChan->pDevice->settings.gpio = *pConfigGPIO;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eFATData:
         {
            BTFE_ConfigFATData *pConfigFATData = (BTFE_ConfigFATData*)pData;
            fat_data.dataPolarity =  pConfigFATData->dataPolarity;
            fat_data.errorPolarity =  pConfigFATData->errorPolarity;
            fat_data.clockPolarity =  pConfigFATData->clockPolarity;
            fat_data.syncPolarity =  pConfigFATData->syncPolarity;
            fat_data.validPolarity =  pConfigFATData->validPolarity;
            fat_data.BurstMode =  pConfigFATData->burstMode;
            fat_data.GatedClockEnable =  pConfigFATData->bGatedClockEnable;
            fat_data.ParallelOutputEnable =  pConfigFATData->bParallelOutputEnable;
            fat_data.HeaderEnable =  pConfigFATData->bHeaderEnable;
            fat_data.CableCardBypassEnable =  pConfigFATData->bCableCardBypassEnable;
            fat_data.FlipOrder =  pConfigFATData->bFlipOrder;
            fat_data.MpegOutputEnable =  pConfigFATData->bMpegOutputEnable;
            fat_data.dataStrength =  pConfigFATData->dataStrength;
            fat_data.errorStrength =  pConfigFATData->errorStrength;
            fat_data.clockStrength =  pConfigFATData->clockStrength;
            fat_data.syncStrength =  pConfigFATData->syncStrength;
            fat_data.validStrength =  pConfigFATData->validStrength;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__FAT_DATA, (void*)&fat_data, sizeof(fat_data)) == SCD_RESULT__OK)
               {
                  hTFEChan->settings.fatData = *pConfigFATData;
                  return BERR_SUCCESS;
               }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eFATAGC:
         {
            BTFE_ConfigFATAGC *pConfigFATAGC = (BTFE_ConfigFATAGC*)pData;
            fat_agc.agcSdm1 =  pConfigFATAGC->agcSdm1;
            fat_agc.agcSdm2 =  pConfigFATAGC->agcSdm2;
            fat_agc.agcSdmX =  pConfigFATAGC->agcSdmX;
            fat_agc.agcSdmA =  pConfigFATAGC->agcSdmA;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__FAT_AGC, (void*)&fat_agc, sizeof(fat_agc)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.fatAGC = *pConfigFATAGC;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eSetTunerIF:
         {
            BTFE_ConfigSetTunerIF *pConfigTunerIF = (BTFE_ConfigSetTunerIF*)pData;
            if_data.current_mod = 0; /* not used */
            if_data.tagSize = sizeof(if_data);
            if_data.centerIF = pConfigTunerIF->center;
            if_data.IFshift = pConfigTunerIF->shift;
            if_data.spectrumInv = pConfigTunerIF->bSpectInvertMode;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__SET_IF, (void*)&if_data, sizeof(if_data)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.setTunerIF = *pConfigTunerIF;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eAcquisition:
         {
            BTFE_ConfigAcquisition *pConfigAcquisition = (BTFE_ConfigAcquisition*)pData;
            acquisition.acqConfig = pConfigAcquisition->acqConfig;
            acquisition.bandWidthConfig = pConfigAcquisition->bandwidthConfig;
            acquisition.agcDelay = pConfigAcquisition->agcDelay;
            acquisition.TuneMode = 0; /*not used*/
            acquisition.bSpectrumInversion = pConfigAcquisition->bSpectrumInversion;
            acquisition.bSpectrumAutoDetect= pConfigAcquisition->bSpectrumAutoDetect;
            acquisition.bCoChannelRejection = pConfigAcquisition->bCoChannelRejection;
            acquisition.bAdjChannelRejection = pConfigAcquisition->bAdjChannelRejection;
            acquisition.bMobileMode = pConfigAcquisition->bMobileMode;
            acquisition.bEnhancedMode = pConfigAcquisition->bEnhancedMode;
            acquisition.bLowPriority = pConfigAcquisition->bLowPriority;
            acquisition.ifFrequency = pConfigAcquisition->uIfFrequency;
            acquisition.bLegacyAGC = 0; /*not used*/
            acquisition.TuneMode = SCD_TUNE_MODE__APP;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__ACQUISITION, (void*)&acquisition, sizeof(acquisition)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.acquisition = *pConfigAcquisition;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_ePad:
#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_AcquireResource(hTFEChan->pDevice->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
         val32 = BREG_Read32(hTFEChan->pDevice->hRegister, BCHP_DFE_MISCDEC_MISC_CORE_PADS_OE_CNTRL);
         if (((BTFE_ConfigPad *)pData)->bAgcEnable)
         {
            /* enable AGC pads */
            val32 &= ~0x10; /* MISC_CORE_PADS_OE_CNTRL.pgm_agc_pads_oen = 0 */
         }
         else
         {
            /* disable AGC pads */
            val32 |= 0x10; /* MISC_CORE_PADS_OE_CNTRL.pgm_agc_pads_oen = 1 */
         }
         BREG_Write32(hTFEChan->pDevice->hRegister, BCHP_DFE_MISCDEC_MISC_CORE_PADS_OE_CNTRL, val32);

#ifdef BCHP_PWR_RESOURCE_DFE
         BCHP_PWR_ReleaseResource(hTFEChan->pDevice->hChip, BCHP_PWR_RESOURCE_DFE);
#endif
         break;
      case BTFE_ConfigItem_ePowerLevel:
         {
            int16_t val16 = *((int16_t *)pData);

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__POWER_LEVEL, (void*)&val16, sizeof(val16)) == SCD_RESULT__OK)
               return BERR_SUCCESS;
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eRfOffset:
         {
            BTFE_ConfigRfOffset *pConfigRfOffset = (BTFE_ConfigRfOffset *)pData;

            rfOffset.freqOffset =  pConfigRfOffset->freqOffset;
            rfOffset.symbolRate =  pConfigRfOffset->symbolRate;
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__RF_OFFSET, (void*)&rfOffset, sizeof(rfOffset)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.rfOffset= *pConfigRfOffset;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_ePowerSaving:
         {
            int32_t val32 = *((int32_t *)pData);

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__POWER_SAVING, (void*)&val32, sizeof(val32)) == SCD_RESULT__OK)
               return BERR_SUCCESS;
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      default:
         BDBG_ERR(("Invalid Config Item\n"));
         return BERR_INVALID_PARAMETER;
         break;
   }

   return BERR_SUCCESS;
}


/***************************************************************************
 BTFE_GetConfig()
****************************************************************************/
BERR_Code BTFE_GetConfig(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_ConfigItem item, /* [in] specifies which config item to get */
   void *pData /* [out] pointer to configuration data structure */
)
{

   BTFE_ConfigPad *pConfigPad;
   uint32_t val32;

   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BDBG_ASSERT(hTFEChan->pFatHandle);
   BDBG_ASSERT(pData != NULL);

   switch(item)
   {
      case BTFE_ConfigItem_eBERT:
      {
         *(BTFE_ConfigBERT*)pData = hTFEChan->settings.bert;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eGPIO:
      {
         *(BTFE_ConfigGPIO*)pData = hTFEChan->pDevice->settings.gpio;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eFATData:
      {
         *(BTFE_ConfigFATData*)pData = hTFEChan->settings.fatData;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eFATAGC:
      {
         *(BTFE_ConfigFATAGC*)pData = hTFEChan->settings.fatAGC;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eAGCScript:
      {
         *(BTFE_ConfigAGCScript*)pData = hTFEChan->settings.agcScript;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eSetTunerIF:
      {
         *(BTFE_ConfigSetTunerIF*)pData = hTFEChan->settings.setTunerIF;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eAcquisition:
      {
         *(BTFE_ConfigAcquisition*)pData = hTFEChan->settings.acquisition;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_ePad:
      {
         pConfigPad = (BTFE_ConfigPad*)pData;
         val32 = BREG_Read32(hTFEChan->pDevice->hRegister, BCHP_DFE_MISCDEC_MISC_CORE_PADS_OE_CNTRL);
         if (val32 & 0x10)
            pConfigPad->bAgcEnable = false;
         else
            pConfigPad->bAgcEnable = true;
         return BERR_SUCCESS;
      }
      default:
         BDBG_ERR(("Invalid Config Item\n"));
         return BERR_INVALID_PARAMETER;
         break;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_SetGpio()
******************************************************************************/
BERR_Code BTFE_SetGpio(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t mask, /* [in] selects which GPIO pins to set */
   BTFE_DataGPIO state /* [in] state of GPIO pins */
)
{
   SCD_HANDLE scd_handle;
   BTFE_ChannelHandle hTFEChan;
   BDBG_OBJECT_ASSERT(hTFE, BTFE);

   hTFEChan = hTFE->pChannels[0];
   scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);

   if (BTFE_P_ScdWriteGpio(scd_handle, (uint32_t) mask, (uint32_t) state.gpioData) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetGpio()
******************************************************************************/
BERR_Code BTFE_GetGpio(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t mask, /* [in] selects which GPIO pins to read */
   BTFE_DataGPIO *pstate /* [out] state of selected GPIO pins */
)
{
   SCD_HANDLE scd_handle;
   BTFE_ChannelHandle hTFEChan;
   BDBG_OBJECT_ASSERT(hTFE, BTFE);

   hTFEChan = hTFE->pChannels[0];
   scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);

   if (BTFE_P_ScdReadGpio(scd_handle, (uint32_t) mask, (uint32_t*) &pstate->gpioData) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BTFE_GetLockStateChangeEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BKNI_EventHandle *hEvent /* [out] lock event handle */
)
{
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   *hEvent = hTFEChan->hLockStateChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetScanEventHandle()
******************************************************************************/
BERR_Code BTFE_GetScanEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BKNI_EventHandle *hEvent /* [out] lock event handle */
)
{
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   *hEvent = hTFEChan->hScanEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_MultU32U32() - private function for fixed point calculations
******************************************************************************/
void BTFE_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo)
{
   uint32_t A_lo = A & 0xFFFF;
   uint32_t A_hi = (A >> 16) & 0xFFFF;
   uint32_t B_lo = B & 0xFFFF;
   uint32_t B_hi = (B >> 16) & 0xFFFF;
   uint32_t P, P0, P1, P2, P3, c;

   P = B_lo * A_lo;
   P0 = P & 0xFFFF;
   P1 = (P >> 16) & 0xFFFF;

   P = B_hi * A_hi;
   P2 = P & 0xFFFF;
   P3 = (P >> 16) & 0xFFFF;

   P = B_lo * A_hi;
   P1 += (P & 0xFFFF);
   P2 += ((P >> 16) & 0xFFFF);

   P = B_hi * A_lo;
   P1 += (P & 0xFFFF);
   P2 += ((P >> 16) & 0xFFFF);

   c = (P1 >> 16) & 0xFFFF;
   if (c)
   {
      P1 &= 0xFFFF;
      P2 += c;
   }

   c = (P2 >> 16) & 0xFFFF;
   if (c)
   {
      P2 &= 0xFFFF;
      P3 += c;
   }

   P3 &= 0xFFFF;
   *P_hi = P2 | (P3 << 16);
   *P_lo = P0 | (P1 << 16);
}


/******************************************************************************
 BTFE_DivU64U32() - private function for fixed point calculations
******************************************************************************/
void BTFE_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo)
{
   uint32_t X;
   int i;

   X = *Q_hi = *Q_lo = 0;
   for (i = 63; i >= 0; i--)
   {
      X = (X << 1);
      if (i >= 32)
      {
         *Q_hi = *Q_hi << 1;
         X |= ((A_hi & (1 << (i - 32))) ? 1 : 0);
      }
      else
      {
         *Q_lo = *Q_lo << 1;
         X |= ((A_lo & (1 << i)) ? 1 : 0);
      }

      if (X >= B)
      {
         if (i >= 32)
            *Q_hi |= 1;
         else
            *Q_lo |= 1;
         X -= B;
      }
   }
}


/******************************************************************************
 BTFE_P_LockStatusChange_isr()
******************************************************************************/
void BTFE_P_LockStatusChange_isr(void *p, int param)
{
   BTFE_ChannelHandle hTFEChan = (BTFE_ChannelHandle)p;

   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);
   BSTD_UNUSED(param);

   BDBG_MSG(("in BTFE_P_LockStatusChange_isr()"));

   BKNI_SetEvent(hTFEChan->hLockStateChangeEvent);
}


/******************************************************************************
 BTFE_P_Scan_isr()
******************************************************************************/
void BTFE_P_Scan_isr(void *p, int param)
{
   BTFE_ChannelHandle hTFEChan = (BTFE_ChannelHandle)p;

   BSTD_UNUSED(param);

   /* BDBG_MSG(("in BTFE_P_Scan_isr()")); */
   BKNI_SetEvent(hTFEChan->hScanEvent);
   BINT_DisableCallback_isr(hTFEChan->hScanCb);
}
#endif
/******************************************************************************
 BTFE_P_EnableIrq()
******************************************************************************/
void BTFE_P_EnableIrq(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   uint32_t           irq       /* [in] DFE irq(s) to enable */
)
{
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

   irq &= BTFE_IRQ_ALL;
   if (irq)
   {
      if (irq & BTFE_IRQ_LOCK_CHANGE)
	   {
	     BINT_ClearCallback(hTFEChan->hLockStatusChangeCb);
	     BINT_EnableCallback(hTFEChan->hLockStatusChangeCb);
	   }

	   if (irq & BTFE_IRQ_SCAN)
	   {
	     BINT_ClearCallback(hTFEChan->hScanCb);
	     BINT_EnableCallback(hTFEChan->hScanCb);
	   }
   }
}


/******************************************************************************
 BTFE_P_DisableIrq()
******************************************************************************/
void BTFE_P_DisableIrq(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   uint32_t           irq       /* [in] DFE irq(s) to disable */
)
{
   BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

   irq &= BTFE_IRQ_ALL;
   if (irq)
   {
      if (irq & BTFE_IRQ_LOCK_CHANGE)
	   {
	     BINT_DisableCallback(hTFEChan->hLockStatusChangeCb);
	   }


	   if (irq & BTFE_IRQ_SCAN)
	   {
	     BINT_DisableCallback(hTFEChan->hScanCb);
	   }
   }
}

/******************************************************************************
 BTFE_ResetStatus()
******************************************************************************/
BERR_Code BTFE_ResetStatus(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t reset[9] = HAB_MSG_HDR(BTFE_eResetSelectiveAsyncStatus, 0x4, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_ResetStatus);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        reset[3] = hTFEChan->channel;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, reset, 9, reset, 0, false, true, 9));
    }

done:
    BDBG_LEAVE(BTFE_ResetStatus);
    return retCode;
}

/******************************************************************************
 BTFE_RequestSelectiveAsyncStatus()
******************************************************************************/
BERR_Code BTFE_RequestSelectiveAsyncStatus(
     BTFE_ChannelHandle hTFEChan,                      /* [in] Device channel handle */
     BTFE_SelectiveAsyncStatusType type         /* [in] Device channel handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTFE_eRequestSelectiveAsyncStatus, 0x4, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_RequestSelectiveAsyncStatus);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hTFEChan->channel;
        buf[5]=type;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 9, buf, 0, false, true, 9));
    }

done:
    BDBG_LEAVE(BTFE_RequestSelectiveAsyncStatus);
    return retCode;
}

/******************************************************************************
 BTFE_GetSelectiveAsyncStatusReadyType()
******************************************************************************/
BERR_Code BTFE_GetSelectiveAsyncStatusReadyType(
     BTFE_ChannelHandle hTFEChan,                          /* [in] Device channel handle */
     BTFE_SelectiveAsyncStatusReadyType *ready    /* [in] Device channel handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTFE_eGetSelectiveAsyncStatusReadyType, 0, BTFE_CORE_TYPE);
    uint8_t i, statusReady = 0;

    BDBG_ENTER(BTFE_GetSelectiveAsyncStatusReadyType);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hTFEChan->channel;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 9, false, true, 9));
        for(i=0; i< 2; i++)
        {
            if((((buf[6] << 8) | buf[7]) >> i) & 0x1)
                statusReady = i;
            switch(statusReady)
            {
                case 0:
                    ready->demod = 1;
                    break;
                case 1:
                    ready->agcIndicator = 1;
                    break;
                default:
                    retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                    break;
            }
        }
    }

done:
    BDBG_LEAVE(BTFE_GetSelectiveAsyncStatusReadyType);
    return retCode;
}

/******************************************************************************
 BTFE_GetSelectiveAsyncStatus()
******************************************************************************/
BERR_Code BTFE_GetSelectiveAsyncStatus(
     BTFE_ChannelHandle hTFEChan,
     BTFE_SelectiveAsyncStatusType type,             /* [in] Device channel handle */
     BTFE_SelectiveStatus *pStatus                     /* [out] */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[57] = HAB_MSG_HDR(BTFE_eGetSelectiveAsyncStatus, 0x8, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_GetSelectiveAsyncStatus);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        if(type == BTFE_SelectiveAsyncStatusType_eDemod){
            buf[3] = hTFEChan->channel;
            CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 13, buf, 57, false, true, 57));
            switch(buf[9])
            {
                case 0:
                    pStatus->type=BTFE_SelectiveAsyncStatusType_eDemod;
                    pStatus->status.demodStatus.lock=(buf[0xC]&0x40) >> 6;
                    pStatus->status.demodStatus.spectrumPolarity=(buf[0xc]&0x20) >> 5;
                    pStatus->status.demodStatus.modulationFormat=buf[0xC]&0xF;
                    pStatus->status.demodStatus.reacqCount=buf[0xD];
                    pStatus->status.demodStatus.equalizerSNR=((buf[0xE] << 8) | buf[0xF])*100/256;
                    pStatus->status.demodStatus.carrierOffset=(buf[0x10] << 24) | (buf[0x11] << 16) | (buf[0x12] << 8) | buf[0x13];
                    pStatus->status.demodStatus.timingOffset=(buf[0x14] << 24) | (buf[0x15] << 16) | (buf[0x16] << 8) | buf[0x17];
                    pStatus->status.demodStatus.rSUncorrectableErrors=(buf[0x18] << 24) | (buf[0x19] << 16) | (buf[0x1A] << 8) | buf[0x1B];
                    pStatus->status.demodStatus.rSCorrectableErrors=(buf[0x1C] << 24) | (buf[0x1D] << 16) | (buf[0x1E] << 8) | buf[0x1F];
                    pStatus->status.demodStatus.numRSpacketsTotal=(buf[0x20] << 24) | (buf[0x21] << 16) | (buf[0x22] << 8) | buf[0x23];
                    pStatus->status.demodStatus.ifFrequency=(buf[0x24] << 24) | (buf[0x25] << 16) | (buf[0x26] << 8) | buf[0x27];
                    pStatus->status.demodStatus.signalQualityIndex=(buf[0x28] << 24) | (buf[0x29] << 16) | (buf[0x2A] << 8) | buf[0x2B];
                    break;
                default:
                    retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                    break;
            }
        }
    }

done:
    BDBG_LEAVE(BTFE_GetSelectiveAsyncStatus);
    return retCode;
}

/******************************************************************************
 BTFE_GetScanStatus()
******************************************************************************/
BERR_Code BTFE_GetScanStatus(
     BTFE_ChannelHandle hTFEChan,                /* [in] Device channel handle */
     BTFE_ScanStatus *pScanStatus                     /* [out] Returns status */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTFE_eGetScanStatus, 0, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_GetScanStatus);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hTFEChan->channel;

        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 9, false, true, 9));
        pScanStatus->operationDone= buf[4]&0x1;
        pScanStatus->confirmVSB= (buf[4]&0x2) >> 1;
    }

done:
    BDBG_LEAVE(BTFE_GetScanStatus);
    return retCode;
}

/******************************************************************************
 BTFE_GetSoftDecision()
******************************************************************************/
BERR_Code BTFE_GetSoftDecision(
     BTFE_ChannelHandle hTFEChan,                /* [in] Device channel handle */
     int16_t nbrToGet,                         /* [in] Number values to get */
     int16_t *iVal,                             /* [out] Ptr to array to store output I soft decision */
     int16_t *qVal,                             /* [out] Ptr to array to store output Q soft decision */
     int16_t *nbrGotten                        /* [out] Number of values gotten/read */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t i;
    uint8_t buf[0x7D] = HAB_MSG_HDR(BTFE_eGetConstellation, 0, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_GetSoftDecision);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        BDBG_ERR(("TFE core %d Powered Off", hTFEChan->channel));
        retCode = BERR_TRACE(BTFE_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hTFEChan->channel;

        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 0x7D, false, true, 0x7D));

        for (i = 0; i < 30 && i < nbrToGet; i++)
        {
            iVal[i] = (buf[4+(4*i)] << 8) | (buf[5+(4*i)]);
            qVal[i] = (buf[6+(4*i)] << 8) | (buf[7+(4*i)]);
        }

        *nbrGotten = i;
    }

done:
    BDBG_LEAVE(BTFE_GetSoftDecision);
    return retCode;
}

/******************************************************************************
 BTFE_EnablePowerSaver()
******************************************************************************/
BERR_Code BTFE_EnablePowerSaver(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BTFE_ePowerCtrlOff, 0, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_EnablePowerSaver);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(!hTFEChan->bPowerdown)
    {
        buf[3] = hTFEChan->channel;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 0, false, true, 5));
        hTFEChan->bPowerdown = true;
    }

done:
    BDBG_LEAVE(BTFE_EnablePowerSaver);
    return retCode;
}


/******************************************************************************
 BTFE_DisablePowerSaver()
******************************************************************************/
BERR_Code BTFE_DisablePowerSaver(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BTFE_ePowerCtrlOn, 0, BTFE_CORE_TYPE);

    BDBG_ENTER(BTFE_DisablePowerSaver);
    BDBG_OBJECT_ASSERT(hTFEChan, BTFE_Channel);

    if(hTFEChan->bPowerdown)
    {
        buf[3] = hTFEChan->channel;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hTFEChan->hHab, buf, 5, buf, 0, false, true, 5));
        hTFEChan->bPowerdown = false;
    }

done:
    BDBG_LEAVE(BTFE_DisablePowerSaver);
    return retCode;
}
