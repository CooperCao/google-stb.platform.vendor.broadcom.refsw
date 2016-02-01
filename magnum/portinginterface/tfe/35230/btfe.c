/***************************************************************************
 *     Copyright (c) 2004-2011, Broadcom Corporation
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
#include "btfe_scd_priv.h"
#include "bchp_dfe_ucdec.h"
#include "btfe_scd_reg35230_hi_priv.h"


BDBG_MODULE(btfe);

static const BTFE_Settings defDevSettings =
{
   {0xFFFC, /* BTFE_ConfigGPIO.ownershipMask */
    0xFFFC, /* BTFE_ConfigGPIO.inputMask */
    0, /* BTFE_ConfigGPIO.outputType */
    0} /* BTFE_ConfigGPIO.i2cSpeedSelect */
};


static const BTFE_ChannelSettings defChnSettings =
{
   {0, /* BTFE_ConfigBERT.headerRemoval */
    BTFE_BERTInput_eFAT, /* BTFE_ConfigBERT.inputSelect */
    false, /* BTFE_ConfigBERT.bPNInversion */
    false, /* BTFE_ConfigBERT.bPNSelection */
    false, /* BTFE_ConfigBERT.bONFlag */
    0, /* BTFE_ConfigBERT.syncErrorThreshold */
    0, /* BTFE_ConfigBERT.syncAcquireCounter */
    0, /* BTFE_ConfigBERT.syncLossCounter */
    0}, /* BTFE_ConfigBERT.windowSize */
   { 
    BTFE_SignalPolarity_eNoInvert, /* BTFE_ConfigFATData.dataPolarity} */
    BTFE_SignalPolarity_eNoInvert, /* BTFE_ConfigFATData.errorPolarity} */
    BTFE_SignalPolarity_eNoInvert, /* BTFE_ConfigFATData.clockPolarity */
    BTFE_SignalPolarity_eNoInvert, /* BTFE_ConfigFATData.syncPolarity */
    BTFE_SignalPolarity_eNoInvert, /* BTFE_ConfigFATData.validPolarity */
    BTFE_BurstMode_eBurstOn, /* BTFE_BurstMode.BTFE_BurstMode_eBurstOff */
    false, /* BTFE_ConfigFATData.bGatedClockEnable */
    false, /* BTFE_ConfigFATData.bParallelOutputEnable */
    true, /* BTFE_ConfigFATData.bHeaderEnable */
    false, /* BTFE_ConfigFATData.bCableCardBypassEnable */
    false, /* BTFE_ConfigFATData.bFlipOrder */
    true, /* BTFE_ConfigFATData.bMpegOutputEnable */
    BTFE_SignalStrength_eDefault, /* BTFE_ConfigFATData.dataStrength */
    BTFE_SignalStrength_eDefault, /* BTFE_ConfigFATData.errorStrength */
    BTFE_SignalStrength_eDefault, /* BTFE_ConfigFATData.clockStrength */
    BTFE_SignalStrength_eDefault, /* BTFE_ConfigFATData.syncStrength */
    BTFE_SignalStrength_eDefault}, /* BTFE_ConfigFATData.validStrength*/
   {BTFE_SignalPolarity_eInvert, /* BTFE_ConfigFATAGC.agcSdm1} */
    BTFE_SignalPolarity_eInvert, /* BTFE_ConfigFATAGC.agcSdm2 */
    BTFE_SignalPolarity_eInvert, /* BTFE_ConfigFATAGC.agcSdmX */
    BTFE_SignalPolarity_eInvert}, /* BTFE_ConfigFATAGC.agcSdmA */     
   {NULL}, /* BTFE_ConfigAGCScript.pData */
   {false, /* BTFE_ConfigSetTunerIF.bOverrideDefault */
    0, /* BTFE_ConfigSetTunerIF.center */
    0, /* BTFE_ConfigSetTunerIF.shift */
    false}, /* BTFE_ConfigSetTunerIF.bSpectInvertMode */
   {BTFE_AcquireConfig_eDirectedAcquire, /* BTFE_ConfigAcquisition.acqConfig */
	 BTFE_Bandwidth_eUndefined, /* BTFE_ConfigAcquisition.bandwidthConfig */
    false, /* BTFE_ConfigAcquisition.bSpectrumInversion */
    false, /* BTFE_ConfigAcquisition.bSpectrumAutoDetect */
    10, /* BTFE_ConfigAcquisition.agcDelay */
    false, /* BTFE_ConfigAcquisition.bCoChannelRejection */
    false, /* BTFE_ConfigAcquisition.bAdjChannelRejection */
    false, /* BTFE_ConfigAcquisition.bMobileMode */
    false, /* BTFE_ConfigAcquisition.bEnhancedMode */
    false, /* BTFE_ConfigAcquisition.bLowPriority */
    0}, /* BTFE_ConfigAcquisition.uIfFrequency */ 
   {BTFE_J83_Mode_eB, /* BTFE_ConfigJ83abc.mode */
    BTFE_J83_Constellation_e64Qam, /* BTFE_ConfigJ83abc.constellation */
    5056941}, /* BTFE_ConfigJ83abc.symbolRate */
    {BTFE_Cofdm_Standard_eDvbT,
      BTFE_Cofdm_CciMode_eNone,
      BTFE_Cofdm_AciMode_eNone,
      BTFE_Cofdm_MobileMode_eAuto,
      BTFE_Cofdm_Priority_eHigh,
      BTFE_Cofdm_CarrierRange_eNarrow,
      BTFE_Cofdm_Impulse_eNone,
      BTFE_Cofdm_RsLayer_eNormal,
      /* 8 - 15 */
      BTFE_Cofdm_ModeGuard_eAuto,
      BTFE_Cofdm_Mode_e2K,
      BTFE_Cofdm_Guard_e1_32,
      BTFE_Cofdm_TpsMode_eAuto,
      BTFE_Cofdm_CodeRate_e1_2,
      BTFE_Cofdm_CodeRate_e1_2,
      BTFE_Cofdm_Hierarchy_eNone,
      BTFE_Cofdm_Constellation_eQpsk, /*modulation */
      /* 16-23 */
      BTFE_Cofdm_Constellation_eQpsk, /* modulationLayerA */
      BTFE_Cofdm_Constellation_eQpsk,
      BTFE_Cofdm_Constellation_eQpsk,
      BTFE_Cofdm_CodeRate_e1_2, /* codeRateLayerA */
      BTFE_Cofdm_CodeRate_e1_2,
      BTFE_Cofdm_CodeRate_e1_2,
      13, /* segmentLayerA */
      0,
      /* 24 - 28 */
      0,
      0, /* timeInterleaveLayerA */
      0,
      0,
      true},
   {0x00000000} /* BTFE_ConfigIsdbtBuffer */
};


static const struct 
{
   BTFE_ModulationFormat mformat;
   SCD_MOD_FORMAT nformat;
   } bformats[] = {
   {BTFE_ModulationFormat_eUnknown , SCD_MOD_FORMAT__UNKNOWN},
   {BTFE_ModulationFormat_eLast , SCD_MOD_FORMAT__LAST},
   {BTFE_ModulationFormat_eVSB , SCD_MOD_FORMAT__FAT_VSB},
   {BTFE_ModulationFormat_eJ83ABC, SCD_MOD_FORMAT__FAT_J83ABC},
   {BTFE_ModulationFormat_eAuto , SCD_MOD_FORMAT__FAT_AUTO},
   {BTFE_ModulationFormat_eNTSC_M , SCD_MOD_FORMAT__FAT_NTSC_M},
   {BTFE_ModulationFormat_eNTSC_N , SCD_MOD_FORMAT__FAT_NTSC_N},
   {BTFE_ModulationFormat_eNTSC_J , SCD_MOD_FORMAT__FAT_NTSC_J},
   {BTFE_ModulationFormat_eNTSC_443 , SCD_MOD_FORMAT__FAT_NTSC_443},
   {BTFE_ModulationFormat_ePAL_I , SCD_MOD_FORMAT__FAT_PAL_I},
   {BTFE_ModulationFormat_ePAL_B , SCD_MOD_FORMAT__FAT_PAL_B},
   {BTFE_ModulationFormat_ePAL_B1 , SCD_MOD_FORMAT__FAT_PAL_B1},
   {BTFE_ModulationFormat_ePAL_G , SCD_MOD_FORMAT__FAT_PAL_G},
   {BTFE_ModulationFormat_ePAL_H , SCD_MOD_FORMAT__FAT_PAL_H},
   {BTFE_ModulationFormat_ePAL_D , SCD_MOD_FORMAT__FAT_PAL_D},
   {BTFE_ModulationFormat_ePAL_K , SCD_MOD_FORMAT__FAT_PAL_K},
   {BTFE_ModulationFormat_ePAL_60 , SCD_MOD_FORMAT__FAT_PAL_60},
   {BTFE_ModulationFormat_ePAL_M , SCD_MOD_FORMAT__FAT_PAL_M},
   {BTFE_ModulationFormat_ePAL_N , SCD_MOD_FORMAT__FAT_PAL_N},
   {BTFE_ModulationFormat_ePAL_NC , SCD_MOD_FORMAT__FAT_PAL_NC},
   {BTFE_ModulationFormat_eSECAM_B , SCD_MOD_FORMAT__FAT_SECAM_B},
   {BTFE_ModulationFormat_eSECAM_D , SCD_MOD_FORMAT__FAT_SECAM_D},
   {BTFE_ModulationFormat_eSECAM_G , SCD_MOD_FORMAT__FAT_SECAM_G},
   {BTFE_ModulationFormat_eSECAM_H , SCD_MOD_FORMAT__FAT_SECAM_H},
   {BTFE_ModulationFormat_eSECAM_K , SCD_MOD_FORMAT__FAT_SECAM_K},
   {BTFE_ModulationFormat_eSECAM_K1 , SCD_MOD_FORMAT__FAT_SECAM_K1},
   {BTFE_ModulationFormat_eSECAM_L , SCD_MOD_FORMAT__FAT_SECAM_L},
   {BTFE_ModulationFormat_eSECAM_L1 , SCD_MOD_FORMAT__FAT_SECAM_L1},
   {BTFE_ModulationFormat_eUNIFIED_COFDM, SCD_MOD_FORMAT__FAT_UNIFIED_COFDM}  
};

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
   BTFE_Handle hDev = NULL;
   int i;
   
   BDBG_ASSERT(hChip);
   BDBG_ASSERT(hInterrupt);
   BDBG_ASSERT(hReg);
   BDBG_ASSERT(pDefSettings);
   BDBG_ASSERT(hTFE);

   /* allocate memory for the handle */
   hDev = (BTFE_Handle)BKNI_Malloc(sizeof(BTFE_P_Handle));
   
   BKNI_Memset(hDev, 0, sizeof(*hDev));
   /* initialize our handle */
   hDev->hRegister = hReg;
   hDev->hInterrupt = hInterrupt;
   hDev->hChip = hChip;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BTFE_Settings));
   
   hDev->pChannels = (BTFE_P_ChannelHandle **)BKNI_Malloc(BTFE_35230_MAX_CHANNELS * sizeof(BTFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   hDev->totalChannels = BTFE_35230_MAX_CHANNELS;
   for (i = 0; i < BTFE_35230_MAX_CHANNELS; i++)
      hDev->pChannels[i] = NULL;
     
   *hTFE = hDev;
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_Close()
******************************************************************************/
BERR_Code BTFE_Close(
   BTFE_Handle hTFE /* [in] BTFE handle */
)
{
   BTFE_P_Handle *hDev;
   
   BDBG_ASSERT(hTFE);
   BDBG_ENTER(BTFE_Close);
   
   hDev = (BTFE_P_Handle *)(hTFE->pChannels);
   
   BTFE_P_ScdCleanup();   
   
   BKNI_Free((void*)hTFE->pChannels);
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
   *totalChannels = BTFE_35230_MAX_CHANNELS;
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
   
   BDBG_ASSERT(hTFE);
   BDBG_ASSERT(chnNo < BTFE_35230_MAX_CHANNELS);   
   
   if (pSettings == NULL)
      BTFE_GetChannelDefaultSettings(hTFE, chnNo, &cs);
   else
      cs = *pSettings;
     
   /* allocate memory for the channel handle */
   ch = (BTFE_P_ChannelHandle *)BKNI_Malloc(sizeof(BTFE_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memset((void*)ch, 0, sizeof(BTFE_P_ChannelHandle));
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BTFE_ChannelSettings)); 
   
   ch->channel = (uint8_t)chnNo;
   ch->pDevice = hTFE;
   ch->pFatHandle = NULL;
   hTFE->pChannels[chnNo] = ch;
   
   /* create events */
   retCode = BKNI_CreateEvent(&(ch->hLockStateChangeEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(ch->hAudioMaxThresholdEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(ch->hScanEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   retCode = BINT_CreateCallback(&(ch->hLockStatusChangeCb), hTFE->hInterrupt, BCHP_INT_ID_DFE_LOCK_STATUS_CHANGE, BTFE_P_LockStatusChange_isr, (void*)ch, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(ch->hAudioClippedCb), hTFE->hInterrupt, BCHP_INT_ID_DFE_AUDIO_CLIPPED, BTFE_P_AudioClipped_isr, (void*)ch, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(ch->hScanCb), hTFE->hInterrupt, BCHP_INT_ID_DFE_SCAN, BTFE_P_Scan_isr, (void*)ch, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
    
   *pChannelHandle = ch;
   
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
    
   BDBG_ASSERT(hTFEChan);
   BDBG_ENTER(BTFE_CloseChannel);
   
   if (BTFE_P_ScdStop(hTFEChan->pFatHandle) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
   
   if (BTFE_P_ScdCloseFat(hTFEChan->pFatHandle) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR); 
   
   BKNI_DestroyEvent(hTFEChan->hLockStateChangeEvent);
   BKNI_DestroyEvent(hTFEChan->hAudioMaxThresholdEvent);   
   BKNI_DestroyEvent(hTFEChan->hScanEvent);   
   
   BINT_DestroyCallback(hTFEChan->hLockStatusChangeCb);
   BINT_DestroyCallback(hTFEChan->hAudioClippedCb);
   BINT_DestroyCallback(hTFEChan->hScanCb);
   
   for (i = 0; i < BTFE_35230_MAX_CHANNELS; i++) 
   {
      if(hTFEChan->pDevice->pChannels[i] == hTFEChan) 
      {
        hTFEChan->pDevice->pChannels[i] = NULL;
        BKNI_Free(hTFEChan);
      }
   }
   
   BDBG_LEAVE(BTFE_CloseChannel);
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_Initialize()
******************************************************************************/
BERR_Code BTFE_Initialize(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   void *pImage /* [in] pointer to 8051  microcode image */
)
{
   BERR_Code retCode;
   BTFE_ChannelHandle pChn;
   BTFE_DataGPIO gpioState;
   int i;
   
   BSTD_UNUSED(pImage); /* TBD... */
   BDBG_ASSERT(hTFE);
          
   /*scd initialize*/
   if (BTFE_P_ScdInitialize(0, hTFE->hRegister) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR); 
		 
   /*do the necessary init for scd and save scd handles in pChn struct*/
   for (i = 0; i < BTFE_35230_MAX_CHANNELS; i++)
   {
      pChn = hTFE->pChannels[i];
      
      BTFE_P_DisableIrq(pChn, BTFE_IRQ_ALL);
      
      if (BTFE_P_ScdOpenFat(0, &pChn->pFatHandle) != BERR_SUCCESS)
         return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         
      if (i == 0)
      {
         retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eGPIO, (void*)(&(hTFE->settings.gpio)));
         if (retCode != BERR_SUCCESS)
         {
            BDBG_ERR(("unable to set BTFE_ConfigItem_eGPIO"));
            return BERR_TRACE(retCode);
         }
	  
         gpioState.gpioData = hTFE->settings.gpio.outputType;
         retCode = BTFE_SetGpio(hTFE, hTFE->settings.gpio.ownershipMask & ~(hTFE->settings.gpio.inputMask), 
                                gpioState);
         if (retCode != BERR_SUCCESS)
         {
            BDBG_ERR(("BTFE_SetGpio() failed"));         
            return BERR_TRACE(retCode);
         }
      }
	  
      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eFATData, (void*)(&(pChn->settings.fatData)));
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set BTFE_ConfigItem_eFATData"));
         return BERR_TRACE(retCode);
      }
        
      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eAcquisition, (void*)(&(pChn->settings.acquisition)));
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set BTFE_ConfigItem_eAcquisition"));
         return BERR_TRACE(retCode);
      }
        
      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eJ83ABC, (void*)(&(pChn->settings.j83abc)));
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set BTFE_ConfigItem_eJ83ABC"));
         return BERR_TRACE(retCode);
      }

      retCode = BTFE_SetConfig(pChn, BTFE_ConfigItem_eUNIFIED_COFDM, (void*)(&(pChn->settings.unifiedCofdm)));
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set BTFE_ConfigItem_eUNIFIED_COFDM"));
         return BERR_TRACE(retCode);
      }
   }
	  
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetVersion()
******************************************************************************/
BERR_Code BTFE_GetVersion(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t *majorTFE, /* [out] major version */
   uint32_t *minorTFE, /* [out] minor version */
   uint32_t *majorFW, /* [out] major version */
   uint32_t *minorFW, /* [out] minor version */
   uint32_t *fwCRC, /* [out] firmware CRC */
   uint32_t *customer /* [out] customer version/name */
)
{
   SCD_VERSION version;

   BDBG_ASSERT(hTFE);

   if (BTFE_P_ScdGetVersion(SCD_ITEM__FIRMWARE, 0, &version) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
   
   if (majorTFE)
      *majorTFE = BTFE_MAJOR_VERSION;

   if (minorTFE)
      *minorTFE = BTFE_MINOR_VERSION;

   if (majorFW)
      *majorFW = version.major;

   if (minorFW)
      *minorFW = version.minor;
     
   if (fwCRC)
      *fwCRC = version.device_id;

   if (customer)
      *customer = version.customer;

   return BERR_SUCCESS; 
}


/******************************************************************************
 BTFE_Acquire()
******************************************************************************/
BERR_Code BTFE_Acquire(
   BTFE_ChannelHandle hTFEChan, /* [in] BVSB handle */
   BTFE_ModulationFormat format /* [in] acquisition parameters */
)
{
   SCD_MOD_FORMAT scd_format;

   if ((format == 0) || (format > BTFE_ModulationFormat_eUNIFIED_COFDM))
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(hTFEChan);
   BDBG_ASSERT(hTFEChan->pFatHandle);

   scd_format = bformats[format].nformat;
   if (scd_format == SCD_MOD_FORMAT__UNKNOWN)
      return BERR_TRACE(BERR_NOT_SUPPORTED);

   /* stop current acquisition */
   BTFE_AbortAcq(hTFEChan);

   /* if acquire mode is search scan, turn on interrupt, and clear operationDone */
   if (hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eSearchScan)
   {
      uint8_t operationDone;

      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);

      operationDone = 0;
      if (format == BTFE_ModulationFormat_eVSB)
      {
         if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_VSB_STATUS_AREA+8, 1, &operationDone) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
      }
      else if (format == BTFE_ModulationFormat_eUNIFIED_COFDM)
      {
         if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_VSB_STATUS_AREA+60, 1, &operationDone) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
      }
   }

   if (BTFE_P_ScdStart(hTFEChan->pFatHandle,  scd_format) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);

   if ((hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eDirectedAcquire) || 
            (hTFEChan->settings.acquisition.acqConfig == BTFE_AcquireConfig_eFullAcquire))
      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_LOCK_CHANGE);

   if (BTFE_IS_ANALOG_FAT_MOD_FORMAT(format))
      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_AUDIO_CLIPPED);
   else
      BTFE_P_DisableIrq(hTFEChan, BTFE_IRQ_AUDIO_CLIPPED);
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_AbortAcq()
******************************************************************************/
BERR_Code BTFE_AbortAcq(
   BTFE_ChannelHandle hTFEChan /* [in] BTFE Handle */
)
{
   BDBG_ASSERT(hTFEChan);
   BDBG_ASSERT(hTFEChan->pFatHandle);
   
   /* disable lock change interrupt */
   BTFE_P_DisableIrq(hTFEChan, BTFE_IRQ_LOCK_CHANGE | BTFE_IRQ_AUDIO_CLIPPED | BTFE_IRQ_SCAN);
   
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
   SCD_STATUS__PSD s_psd;
   SCD_STATUS__EQ_TAPS_PLUS s_eq_taps;
   SCD_STATUS__CONSTELLATION_DATA s_constellation_data;
   SCD_STATUS__MEMORY_READ s_memory_read;
   uint32_t i;
   uint8_t buffer[28];
  
      
   BDBG_ASSERT(hTFEChan);
   BDBG_ASSERT(hTFEChan->pFatHandle);
   BDBG_ASSERT(pData != NULL);
   
   switch(item)
   {
      case BTFE_StatusItem_eJ83ABC:
         {
            BTFE_StatusJ83ABC *pStatusJ83ABC = (BTFE_StatusJ83ABC *)pData;
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA, 28, buffer) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusJ83ABC->reacqCounter = buffer[0];
            pStatusJ83ABC->acqConfig = (BTFE_AcquireConfig)buffer[1];
            pStatusJ83ABC->bandwidthConfig = (BTFE_Bandwidth)buffer[2];
            pStatusJ83ABC->bSpectralInversion = buffer[3] ? true : false;
            pStatusJ83ABC->mode = (BTFE_J83_Mode)buffer[4];
            pStatusJ83ABC->constellation = (BTFE_J83_Constellation)buffer[5];            
            pStatusJ83ABC->bOperationDone = buffer[12] ? true : false;
            pStatusJ83ABC->bSignalDetected = buffer[13] ? true : false;
            pStatusJ83ABC->bandEdgePosPower = (int8_t)buffer[14];
            pStatusJ83ABC->bandEdgeNegPower = (int8_t)buffer[15];
            pStatusJ83ABC->IFNomRate = (uint32_t)((buffer[16] << 24) | (buffer[17] << 16) | (buffer[18] << 8) | buffer[19]);
            pStatusJ83ABC->baudRateDetected = (uint32_t)((buffer[20] << 24) | (buffer[21] << 16) | (buffer[22] << 8) | buffer[23]);
            pStatusJ83ABC->rateNomFinal = (uint32_t)((buffer[24] << 24) | (buffer[25] << 16) | (buffer[26] << 8) | buffer[27]);
            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eVSB:
         {
            BTFE_StatusVSB *pStatusVSB = (BTFE_StatusVSB *)pData;
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_VSB_STATUS_AREA, 10, buffer) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusVSB->operationDone= buffer[8];
            pStatusVSB->confirmVSB = buffer[9] ? true: false;

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eDVBT:
         {
            BTFE_StatusDVBT *pStatusDVBT = (BTFE_StatusDVBT *)pData;
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_UNIFIED_COFDM_STATUS_AREA+60, 9, buffer) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            pStatusDVBT->operationDone= buffer[0];
            pStatusDVBT->scanResult= buffer[1];
            pStatusDVBT->spectraInverted= buffer[2]? true:false;
            pStatusDVBT->carrierOffset= (int32_t)((buffer[3] << 24) | (buffer[4] << 16) | (buffer[5] << 8) | buffer[6]);			
            pStatusDVBT->timingOffset=  (int16_t)((buffer[7] << 8) | buffer[8]);			

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
      case BTFE_StatusItem_ePSD: 
         {
            BTFE_StatusPSD *pStatusPSD = (BTFE_StatusPSD*)pData;
            
            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__PSD_FRONTEND, (void*)&s_psd, sizeof(s_psd)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);

            for(i=0; i<TFE_PSDARRAYSIZE; i++)
            {
               pStatusPSD->powerSpectrumData[i] =    s_psd.power_spectrum_data[i];   
            }            

            return BERR_SUCCESS;
         }
      case BTFE_StatusItem_eEQTaps: /*Remove avgnorm from pStatusEQTaps and add EQTapsPlus if needed*/
         {
            BTFE_StatusEQTaps *pStatusEQTaps = (BTFE_StatusEQTaps*)pData;
         
            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__EQ_TAPS_PLUS, (void*)&s_eq_taps, sizeof(s_eq_taps)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
            
            for(i=0; i<TFE_TAPARRAYSIZE; i++)
            {
               s_eq_taps.taps[i] = pStatusEQTaps->taps[i];
               s_eq_taps.adjustment[i] = pStatusEQTaps->adjustment[i];
               s_eq_taps.avgnorm[i] = pStatusEQTaps->avgnorm[i];
            }
            
            return BERR_SUCCESS;
         }
     case BTFE_StatusItem_eConstellationData: 
         {
            BTFE_StatusConstellationData *pStatusConstellationData = (BTFE_StatusConstellationData*)pData;
         
            if (BTFE_P_ScdGetStatus(hTFEChan->pFatHandle, SCD_STATUS_ITEM__CONSTELLATION_DATA, (void*)&s_constellation_data, sizeof(s_constellation_data)) != SCD_RESULT__OK)
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
            
            s_constellation_data.constX = pStatusConstellationData->constX;
            s_constellation_data.constY = pStatusConstellationData->constY;
      
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
            pStatusFat->rSUncorrectableErrorsB = s_fat_demod.RSUncorrectableErrorsB;
            pStatusFat->rSCorrectableErrorsB = s_fat_demod.RSCorrectableErrorsB;
            pStatusFat->numRSpacketsB = s_fat_demod.NumRSpacketsB;
            pStatusFat->rSUncorrectableErrorsC = s_fat_demod.RSUncorrectableErrorsC;
            pStatusFat->rSCorrectableErrorsC = s_fat_demod.RSCorrectableErrorsC;
            pStatusFat->numRSpacketsC = s_fat_demod.NumRSpacketsC;
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
            pStatusFat->cofdmModFormat = s_fat_demod.COFDMModFormat;
            pStatusFat->cofdmHierarchy = s_fat_demod.COFDMHierarchy;
            pStatusFat->cofdmMode = s_fat_demod.COFDMMode;
            pStatusFat->cofdmGuardInt = s_fat_demod.COFDMGuardInt;
            pStatusFat->cofdmCodeRate = s_fat_demod.COFDMCodeRate;
            /* ISDBT */
            pStatusFat->cofdmModFormatA = s_fat_demod.COFDMModFormatA;
            pStatusFat->cofdmModFormatB = s_fat_demod.COFDMModFormatB;
            pStatusFat->cofdmModFormatC = s_fat_demod.COFDMModFormatC;

            pStatusFat->cofdmCodeRateA = s_fat_demod.COFDMCodeRateA;
            pStatusFat->cofdmCodeRateB = s_fat_demod.COFDMCodeRateB;
            pStatusFat->cofdmCodeRateC = s_fat_demod.COFDMCodeRateC;

            pStatusFat->cofdmTdiA = s_fat_demod.COFDMTdiA;
            pStatusFat->cofdmTdiB = s_fat_demod.COFDMTdiB;
            pStatusFat->cofdmTdiC = s_fat_demod.COFDMTdiC;

            pStatusFat->cofdmSegA = s_fat_demod.COFDMSegA;
            pStatusFat->cofdmSegB = s_fat_demod.COFDMSegB;
            pStatusFat->cofdmSegC = s_fat_demod.COFDMSegC;
			/* end of ISDBT */
            pStatusFat->normalizedIF = s_fat_demod.NormalizedIF;
            pStatusFat->cofdmBerErrCnt = s_fat_demod.COFDMBerErrCnt;
            pStatusFat->cofdmBerPktCnt = s_fat_demod.COFDMBerPktCnt;

            pStatusFat->ews = s_fat_demod.ews;
            pStatusFat->partialReception = s_fat_demod.partialReception;
            pStatusFat->cellId = s_fat_demod.cellId;
            pStatusFat->demodSpectrum = s_fat_demod.demodSpectrum;	
            pStatusFat->ifdLockStatus= s_fat_demod.IFDlockStatus;	
                                  
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
   SCD_CONFIG__AGC_SCRIPT agc_script;
   SCD_XPROP_TUNER_IF__DATA if_data;
   SCD_CONFIG__ACQUISITION acquisition;
   SCD_CONFIG__J83ABC j83abc;
   SCD_CONFIG__UNIFIED_COFDM unifiedCofdm;
   SCD_CONFIG__ISDBT_BUFFER isdbtBuffer;
   uint32_t val32 = 0;
     
   BDBG_ASSERT(hTFEChan);
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
      case BTFE_ConfigItem_eAGCScript: 
         {
            BTFE_ConfigAGCScript *pConfigData = (BTFE_ConfigAGCScript*)pData;
            agc_script.pdata =  pConfigData->pData;  
                     
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__AGC_SCRIPT, (void*)&agc_script, sizeof(agc_script)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.agcScript = *pConfigData;
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
      case BTFE_ConfigItem_eJ83ABC:   
         {
            BTFE_ConfigJ83abc *pConfigJ83ABC = (BTFE_ConfigJ83abc*)pData;   

		   	/* validate parameters */
            if (pConfigJ83ABC->mode == BTFE_J83_Mode_eB)
            {
			      if ((pConfigJ83ABC->constellation != BTFE_J83_Constellation_e64Qam) &&
			          (pConfigJ83ABC->constellation != BTFE_J83_Constellation_e256Qam))
			      {
			         return BERR_TRACE(BTFE_ERR_INVALID_PARAM_COMBO);
			      }
            }
			
            j83abc.j83abcMode = pConfigJ83ABC->mode;
            j83abc.qamMode = pConfigJ83ABC->constellation;
            j83abc.symbolRate = pConfigJ83ABC->symbolRate;   
         
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83ABC, (void*)&j83abc, sizeof(j83abc)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.j83abc = *pConfigJ83ABC;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_eIsdbtBuffer:
         {
            BTFE_ConfigIsdbtBuffer *pConfigIsdbtBuffer = (BTFE_ConfigIsdbtBuffer *)pData;
            isdbtBuffer.bufferPtr = pConfigIsdbtBuffer->address;
            isdbtBuffer.alignment = 0x00400000;           
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__ISDBT_BUFFER, (void*)&isdbtBuffer, sizeof(isdbtBuffer)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.isdbtBuffer= *pConfigIsdbtBuffer;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);            
         }
         
      case BTFE_ConfigItem_ePad:
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
         break;
         
      case BTFE_ConfigItem_eUNIFIED_COFDM:   
         {
            BTFE_ConfigCofdm *pConfigCofdm = (BTFE_ConfigCofdm*)pData;   
			
            unifiedCofdm.ofdmStandard = pConfigCofdm->ofdmStandard;
            unifiedCofdm.cciEnable = pConfigCofdm->cci;
            unifiedCofdm.aciEnable = pConfigCofdm->aci;
            unifiedCofdm.mobilMode = pConfigCofdm->mobile;
            unifiedCofdm.priorityMode = pConfigCofdm->priority;
            unifiedCofdm.carrierRange = pConfigCofdm->carrierRange;
            unifiedCofdm.impluse = pConfigCofdm->impulse; 
            unifiedCofdm.rsLayer = pConfigCofdm->rsLayer;

            unifiedCofdm.modeGuard = pConfigCofdm->modeGuard;
            unifiedCofdm.mode = pConfigCofdm->mode;
            unifiedCofdm.guard = pConfigCofdm->guard;
            unifiedCofdm.tpsMode = pConfigCofdm->tps;
            unifiedCofdm.codeLP = pConfigCofdm->codeLP;
            unifiedCofdm.codeHP = pConfigCofdm->codeHP;
            unifiedCofdm.hierarchy = pConfigCofdm->hierarchy;
            unifiedCofdm.modulation = pConfigCofdm->modulation;

            unifiedCofdm.modulationLayerA = pConfigCofdm->modulationLayerA;
            unifiedCofdm.modulationLayerB = pConfigCofdm->modulationLayerB;
            unifiedCofdm.modulationLayerC = pConfigCofdm->modulationLayerC;
            unifiedCofdm.codeRateLayerA = pConfigCofdm->codeRateLayerA;
            unifiedCofdm.codeRateLayerB = pConfigCofdm->codeRateLayerB;
            unifiedCofdm.codeRateLayerC = pConfigCofdm->codeRateLayerC;
            unifiedCofdm.segmentLayerA = pConfigCofdm->segmentLayerA;
            unifiedCofdm.segmentLayerB = pConfigCofdm->segmentLayerB;
			
            unifiedCofdm.segmentLayerC = pConfigCofdm->segmentLayerC;
            unifiedCofdm.timeInterleaveLayerA = pConfigCofdm->timeInterleaveLayerA;
            unifiedCofdm.timeInterleaveLayerB = pConfigCofdm->timeInterleaveLayerB;
            unifiedCofdm.timeInterleaveLayerC = pConfigCofdm->timeInterleaveLayerC;
            unifiedCofdm.partialReception = pConfigCofdm->partialReception;

            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__UNIFIED_COFDM, (void*)&unifiedCofdm, sizeof(unifiedCofdm)) == SCD_RESULT__OK)
            {
               hTFEChan->settings.unifiedCofdm= *pConfigCofdm;
               return BERR_SUCCESS;
            }
            else
               return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         }
      case BTFE_ConfigItem_ePowerLevel: 
         {
            int16_t val16 = *((int16_t *)pData);       
                     
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__POWER_LEVEL, (void*)&val16, sizeof(val16)) == SCD_RESULT__OK)
               return BERR_SUCCESS;
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
   
   BDBG_ASSERT(hTFEChan);
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
      case BTFE_ConfigItem_eJ83ABC:
      {
         *(BTFE_ConfigJ83abc*)pData = hTFEChan->settings.j83abc;
         return BERR_SUCCESS;
      }
      case BTFE_ConfigItem_eUNIFIED_COFDM:
      {
         *(BTFE_ConfigCofdm*)pData = hTFEChan->settings.unifiedCofdm;
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
   BDBG_ASSERT(hTFE);

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
   BDBG_ASSERT(hTFE);

   hTFEChan = hTFE->pChannels[0];
   scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);   

   if (BTFE_P_ScdReadGpio(scd_handle, (uint32_t) mask, (uint32_t*) &pstate->gpioData) != BERR_SUCCESS)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
     
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_WriteMi2c()
******************************************************************************/
BERR_Code BTFE_WriteMi2c(
   BTFE_Handle hTFE, /* [in] BTFE Handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *pBuf, /* [in] specifies the data to transmit */
   uint32_t n /* [in] number of bytes to transmit after the i2c slave address */
)
{
   SCD_HANDLE scd_handle;
   BTFE_ChannelHandle hTFEChan;
   uint32_t bufLen, bytesLeft;
   uint8_t *p = pBuf;
   uint8_t subaddr = 0, buf[16];
   
   BDBG_ASSERT(hTFE);
   BDBG_ASSERT(n > 0);
   BDBG_ASSERT(pBuf);
   
   hTFEChan = hTFE->pChannels[0];
   if (hTFEChan)
   {
      scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);   
    
      /* 8051 can only transmit 15 bytes at a time (including slave addr) */
      /* therefore, we must send the buffer in 15 byte chunks */
      bytesLeft = n;
      while (bytesLeft > 0)
      {
         if (bytesLeft == n)
         {
            subaddr = pBuf[0];
            bytesLeft--;
            p++;            
            if (n <= 14)
               bufLen = n-1;
            else
               bufLen = 13;
         }
         else
         {
            if (bytesLeft < 14)
               bufLen = bytesLeft;
            else
               bufLen = 13;
         }
            
         buf[0] = subaddr;
         if (bufLen)
            BKNI_Memcpy(&buf[1], p, bufLen);
         
         if (BTFE_P_ScdWriteI2C(scd_handle, (uint32_t)slave_addr, 0, 0, bufLen+1, buf) != BERR_SUCCESS)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
            
         bytesLeft -= bufLen;
         p += bufLen;
         subaddr += bufLen;
      }
   }
 
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_ReadMi2c()
******************************************************************************/
BERR_Code BTFE_ReadMi2c(
   BTFE_Handle hTFE, /* [in] BTFE Handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf, /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n, /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf, /* [out] holds the data read */
   uint8_t in_n /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   SCD_HANDLE scd_handle;
   BTFE_ChannelHandle hTFEChan;
   uint32_t subaddr, subaddr_len, bytesLeft, n, addr;
   uint8_t *pInBuf;

   BDBG_ASSERT(hTFE);
   BDBG_ASSERT(in_buf);
   BDBG_ASSERT(in_n > 0);

   if(out_n)
      BDBG_ASSERT(out_buf);
   
   if (out_n > 4)
      return BERR_TRACE(BERR_NOT_SUPPORTED);
     
   subaddr_len = (uint32_t)out_n;
   if (subaddr_len == 4)
      subaddr = (uint32_t)((out_buf[0] << 24) | (out_buf[1] << 16) | (out_buf[2] << 8) | out_buf[3]);
   else if (subaddr_len == 3)
      subaddr = (uint32_t)((out_buf[0] << 16) | (out_buf[1] << 8) | out_buf[2]);   
   else if (subaddr_len == 2)
      subaddr = (uint32_t)((out_buf[0] << 8) | out_buf[1]);   
   else if (subaddr_len == 1)
      subaddr = (uint32_t)out_buf[0];
   else
      subaddr = 0;

   hTFEChan = hTFE->pChannels[0];
   
   if (hTFEChan)
   {
      scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);   

      /* do multiple reads since 8051 can only read in 32 bytes at a time */
      bytesLeft = in_n;
      pInBuf = in_buf;
      addr = subaddr;
      while (bytesLeft)
      {
         if (bytesLeft > 32)
            n = 32;
         else
            n = bytesLeft;
         if (BTFE_P_ScdReadI2C(scd_handle, (uint32_t)slave_addr, addr, subaddr_len, n, pInBuf) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
         pInBuf += n;
         addr += n;
         bytesLeft -= n;
      }
   }
      
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetAudioMaxThresholdEventHandle()
******************************************************************************/
BERR_Code BTFE_GetAudioMaxThresholdEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */   
   BKNI_EventHandle *hEvent /* [out] event handle */
)
{
   BDBG_ASSERT(hTFEChan);
   *hEvent = hTFEChan->hAudioMaxThresholdEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_SetAudioMagShift()
******************************************************************************/
BERR_Code BTFE_SetAudioMagShift(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFEHandle */
    uint8_t magshift /* [in] mag_shift value */
)
{
   SCD_HANDLE scd_handle;
   
   BDBG_ASSERT(hTFEChan);  
   scd_handle = (SCD_HANDLE)(hTFEChan->pFatHandle);
   if (BTFE_P_ScdSetConfig(scd_handle, SCD_CONFIG_ITEM__AUDIO_MAG_SHIFT, (void*)&magshift, sizeof(magshift)) == SCD_RESULT__OK)
      return BERR_SUCCESS;
   else
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);  
}


/******************************************************************************
 BTFE_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BTFE_GetLockStateChangeEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BKNI_EventHandle *hEvent /* [out] lock event handle */
)
{
   BDBG_ASSERT(hTFEChan);
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
   BDBG_ASSERT(hTFEChan);
   *hEvent = hTFEChan->hScanEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_GetCofdmMemoryRequirement()
******************************************************************************/
BERR_Code BTFE_GetCofdmMemoryRequirement(
   uint32_t *memSize,  /* [out] required memory size in bytes */
   uint32_t *alignment /* [out] byte alignment */
)
{
   BDBG_ASSERT(memSize);
   BDBG_ASSERT(alignment);
   
   *memSize = 0x00276000; /* 2520 KB */
   *alignment = 0x00400000; /* 4MB alignment */
   return BERR_SUCCESS;
}


/******************************************************************************
 BTFE_TryNextScan()
******************************************************************************/
BERR_Code BTFE_TryNextScan(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   int32_t carrierOffset
)
{
   uint8_t waitOnStart;
   int32_t scdCarrirOffset;

   scdCarrirOffset = carrierOffset;
   if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__TRY_NEXT_SCAN, &scdCarrirOffset, sizeof(scdCarrirOffset)) != SCD_RESULT__OK)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);   
   
   BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
 
   waitOnStart = 1;
   if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);     

   return BERR_SUCCESS;   
}


/******************************************************************************
 BTFE_J83NextScan()
******************************************************************************/

BERR_Code BTFE_J83NextScan(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BTFE_J83ChannelScan *pChannelScan
)
{
   BERR_Code retCode;
   SCD_CONFIG_J83_NEXT_SCAN j83ScanData;
   uint8_t waitOnStart;
   uint8_t buffer[32];
   bool operationDone = false, signalDetected = false;
   uint32_t curSymbolRate = 0;
   int32_t dcStep, dcOffset;
   int8_t stepArray[15] = {0, 4, -4, 2, 6, -2, -6, 7, 5, 3, 1, -7, -5, -3, -1};
   uint16_t tempu16;
   int32_t temp32 = 0;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, val;
   int32_t carrierLoop;
   int i;
   uint8_t digitalModeArray[6] = {QAM_MODE_DIGITAL_32QAM,
	  	                                  QAM_MODE_DIGITAL_16QAM,
	  	                                  QAM_MODE_DIGITAL_128QAM,
	  	                                  QAM_MODE_DIGITAL_64QAM,
	  	                                  QAM_MODE_DIGITAL_256QAM,
	  	                                  QAM_MODE_DIGITAL_UNDEFINED};

   if ((pChannelScan->mode != BTFE_J83_Mode_eA) && (pChannelScan->mode != BTFE_J83_Mode_eB) && (pChannelScan->mode != BTFE_J83_Mode_eC))
      return BERR_INVALID_PARAMETER;

   if (pChannelScan->mode == BTFE_J83_Mode_eB)
   {
      uint32_t j83bSymRateArray[2] = {QAM256_SYMBOL_RATE, QAM64_SYMBOL_RATE};

      /* symbol rate verify */
      j83ScanData.carrierOffset = 0;
      j83ScanData.bandwidth= SCD_BW_6MHZ;	  
      j83ScanData.j83abcMode = J83B;
      j83ScanData.acqMode = SCD_SYMBOL_RATE_VERIFY;
      for (i = 0; i < (int)(sizeof(j83bSymRateArray)/sizeof(uint32_t)); ++i)
      {
         j83ScanData.symbolRate = j83bSymRateArray[i];
         j83ScanData.qamMode = (i == 0) ? QAM_MODE_DIGITAL_256QAM : QAM_MODE_DIGITAL_64QAM;
         if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
         BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
         BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
         waitOnStart = 1;
         if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

         /* wait for symbol rate verify to complete */
         retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 500);
         if (retCode == BERR_TIMEOUT)
         {
            BDBG_WRN(("J83B symbol rate verify has timed out"));
            break;
         }
		 
         if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
         operationDone = buffer[0] ? true : false;
         signalDetected = buffer[1] ? true : false;

         if (operationDone && signalDetected)
         {
            /* printf("J83B signal detect i=%d\n", i); */
            break;
         }
      }

      pChannelScan->bSignalDetected = signalDetected;
      pChannelScan->bOperationDone= operationDone;
      if (!signalDetected)
         return BERR_SUCCESS;

      curSymbolRate =  j83bSymRateArray[i];
      if (i == 0)
      {
          pChannelScan->constellation = BTFE_J83_Constellation_e256Qam;
          digitalModeArray[0] = QAM_MODE_DIGITAL_256QAM;
          digitalModeArray[1] = QAM_MODE_DIGITAL_UNDEFINED;		 
      }
      else 
      {
          pChannelScan->constellation = BTFE_J83_Constellation_e64Qam;
          digitalModeArray[0] = QAM_MODE_DIGITAL_64QAM;
          digitalModeArray[1] = QAM_MODE_DIGITAL_UNDEFINED;	
      }
   }
   else /* BTFE_J83_Mode_eA or BTFE_J83_Mode_eC */
   {
      uint32_t j83aSymRateArray[6] = {6000000, 6952000, 6950000, 6900000, 6875000, 6125000};

      /* symbol rate verify */
      j83ScanData.carrierOffset = 0;
      j83ScanData.bandwidth= SCD_BW_8MHZ;
      j83ScanData.j83abcMode = J83A;
      j83ScanData.acqMode = SCD_SYMBOL_RATE_VERIFY;

      for (i = 0; i < (int)(sizeof(j83aSymRateArray)/sizeof(uint32_t)); ++i)
      {
         j83ScanData.symbolRate = j83aSymRateArray[i];
         j83ScanData.qamMode = 0; /* XXX */

/* printf("J83A symbol %d\n", j83ScanData.symbolRate); */
         if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
         BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
         BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
         waitOnStart = 1;
         if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

         /* wait for symbol rate verify to complete */
         retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 500);
         if (retCode == BERR_TIMEOUT)
         {
            BDBG_WRN(("J83A symbol rate verify has timed out"));
            break;
         }
		 
         if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
         operationDone = buffer[0] ? true : false;
         signalDetected = buffer[1] ? true : false;

         if (operationDone && signalDetected)
         {
            /* printf("J83A signal detect i=%d\n", i); */
            break;
         }
      }
      pChannelScan->bSignalDetected = signalDetected;
      pChannelScan->bOperationDone= operationDone;	  
      if (!signalDetected)
         return BERR_SUCCESS;	  

      curSymbolRate =  j83aSymRateArray[i];
   }

   /* 
    * constellation search
    */
   if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+13, 6, digitalModeArray) != SCD_RESULT__OK)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR); 
	  
   dcStep = (curSymbolRate * 25) /4000; /* symbolRate/4000000 * 25000 */
   j83ScanData.acqMode = SCD_CONST_SEARCH;
   j83ScanData.symbolRate = curSymbolRate;

   for (i = 0; i < (int)(sizeof(stepArray)/sizeof(int8_t)); ++i)
   {

      j83ScanData.carrierOffset = dcOffset = (int32_t) (dcStep * stepArray[i]);

/* printf("J83 dc_offset %d\n", j83ScanData.carrierOffset); */
      if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
         return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
      BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
      BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
      waitOnStart = 1;
      if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
         return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

      /* wait for symbol rate verify to complete */
      retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 1000);
      if (retCode == BERR_TIMEOUT)
      {
         BDBG_WRN(("J83 constellation search has timed out!"));
         break;
      }
		 
      if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
         return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
      operationDone = buffer[0] ? true : false;
      signalDetected = buffer[1] ? true : false;

      if (operationDone && signalDetected)
      {
         /* printf("J83 constellation detect i=%d\n", i); */
         break;
      }
   }

   pChannelScan->bSignalDetected = signalDetected;
   pChannelScan->bOperationDone= operationDone;	 
   if (!signalDetected)
      return BERR_SUCCESS;

   if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA, 30, buffer) != SCD_RESULT__OK)
      return BERR_TRACE(BTFE_ERR_SCD_ERROR);
   
   tempu16 = (buffer[28] << 8) | (buffer[29]);
   if(tempu16 & 0x8000)
   {
      temp32 = (int32_t) (tempu16 - 0x10000);
      val = ~temp32 + 1;
   }
   else
      val = (uint32_t) tempu16;

   BTFE_MultU32U32(val, curSymbolRate, &P_hi, &P_lo);
   BTFE_DivU64U32(P_hi, P_lo, 524288, &Q_hi, &Q_lo); /* divide by 2^19 */
   if (temp32 & 0x80000000)
      carrierLoop = (int32_t) -Q_lo;
   else 
      carrierLoop = (int32_t) Q_lo;

/* printf(" temp16 %d, 0x%x, dc_offset %d, carrier_loop %d\n", tempu16, (uint16_t) tempu16, dcOffset, carrierLoop); */

   pChannelScan->carrierOffset = -(dcOffset+carrierLoop);
   pChannelScan->bSpectralInversion = buffer[3] ? true : false;
   pChannelScan->constellation = buffer[5];
   pChannelScan->symbolRate = curSymbolRate;

   /* printf("constellation %d, bSpectralInversion %d, carrierOffset %d\n", 
   	pChannelScan->constellation, pChannelScan->bSpectralInversion, pChannelScan->carrierOffset); */

   return BERR_SUCCESS;  
}


/******************************************************************************
 BTFE_DvbcChannelScan()
******************************************************************************/

BERR_Code BTFE_DvbcChannelScan(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BTFE_ChannelScan *pChannelScan
)
{
    BERR_Code retCode;
    BTFE_ConfigAcquisition acquisition;
    SCD_CONFIG_J83_NEXT_SCAN j83ScanData;
    BTFE_DvbcScanData *pScanData;
    uint8_t waitOnStart;
    uint8_t buffer[32];
    bool operationDone = false, signalDetected = false;
    int32_t curOffset = 0;
    int i, j;

    BDBG_ASSERT(hTFEChan);
    BDBG_ASSERT(pChannelScan);
    BDBG_ASSERT(pChannelScan->pScanData);
	
    if ((pChannelScan->numScanData == 0) || (pChannelScan->numScanData > 16))
        return BERR_INVALID_PARAMETER;

    pScanData = pChannelScan->pScanData;
		
    /* 
     * Step 1: Check DVB-C signal presence
     */	

    /* set up the acquisition */
    retCode = BTFE_GetConfig(hTFEChan, BTFE_ConfigItem_eAcquisition, &acquisition);
    if (retCode != BERR_SUCCESS)	
        return BERR_TRACE(retCode);

   /* acquisition.bSpectrumInversion and agcDelay are set by application */
    acquisition.acqConfig = BTFE_AcquireConfig_eSymbolRateVerify;
    acquisition.bandwidthConfig = BTFE_Bandwidth_e8MHZ; 
    retCode = BTFE_SetConfig(hTFEChan, BTFE_ConfigItem_eAcquisition, &acquisition);
    if (retCode != BERR_SUCCESS)
        return BERR_TRACE(retCode);

    retCode = BTFE_Acquire(hTFEChan, BTFE_ModulationFormat_eJ83ABC);
    if (retCode != BERR_SUCCESS)
        return BERR_TRACE(retCode);

    j83ScanData.acqMode = SCD_SYMBOL_RATE_VERIFY;	
    j83ScanData.bandwidth= SCD_BW_8MHZ;
    j83ScanData.j83abcMode = J83A;
    j83ScanData.qamMode = 0; /* not used, carrierOffset/symbolRate will be adjusted */
    signalDetected = false;

    /* loop on symbol rate */
    for (i = 0; (signalDetected == false) && (i < (int)(pChannelScan->numScanData)); ++i)
    {  
        j83ScanData.symbolRate =pScanData->symbolRate;
        /* printf("DVB-C symbol %d\n", j83ScanData.symbolRate); */
		
        if (j83ScanData.symbolRate > 7000000)
	    j83ScanData.bandwidth= SCD_BW_9MHZ;
        else if (j83ScanData.symbolRate > 6100000)
	    j83ScanData.bandwidth= SCD_BW_8MHZ;		
        else if (j83ScanData.symbolRate > 5220000)
	    j83ScanData.bandwidth= SCD_BW_7MHZ;
        else if (j83ScanData.symbolRate > 4350000)
	    j83ScanData.bandwidth= SCD_BW_6MHZ;
        else if (j83ScanData.symbolRate > 3480000)
	    j83ScanData.bandwidth= SCD_BW_5MHZ;
        else if (j83ScanData.symbolRate > 2610000)
	    j83ScanData.bandwidth= SCD_BW_4MHZ;
        else if (j83ScanData.symbolRate > 1740000)
	    j83ScanData.bandwidth= SCD_BW_3MHZ;
        else
	    j83ScanData.bandwidth= SCD_BW_2MHZ;

       /* loop on freq span for a symbol rate */
       for (j = 0; j < (int)(pScanData->numFeqSpan); ++j)
       {
            operationDone = signalDetected = false;
            j83ScanData.carrierOffset = pScanData->freqSpanArray[j];
	     /* set to different carrier offset, symbol rate */
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
            BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
            waitOnStart = 1;
            if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

            /* wait for symbol rate verify to complete */
            retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 500);
            if (retCode == BERR_TIMEOUT)
            {
                BDBG_WRN(("DVB-C BTFE_DvbcChannelScan time out"));
                break;
            }
		 
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            operationDone = buffer[0] ? true : false;
            signalDetected = buffer[1] ? true : false;

            if (operationDone && signalDetected)
            {
                /* printf("DVB-C signal detect i=%d, j=%d\n", i, j); */
                curOffset = pScanData->freqSpanArray[j];
                pChannelScan->symbolRate =  pScanData->symbolRate;				
                break;
            }
       }

        /* try with next symbol rate */
	if (signalDetected == false)
	    pScanData++;
    }

    /* printf("signal detect %d, i = %d, curOffset %d, symbolrate %d\n", signalDetected, i, curOffset, pScanData->symbolRate); */

    pChannelScan->bSignalDetected = signalDetected;	  
    if (!signalDetected)
        return BERR_SUCCESS;	  

    /* 
     * Step 2: Estimate coarse CR offset with a series of SRVs
     */

/* printf("call BTFE_P_LogSearch: offset %d, logspan %d\n", curOffset, pScanData->logSweepFreq); */
    BTFE_P_LogSearch(hTFEChan, pScanData, curOffset, &(pChannelScan->carrierOffset));
	
   /* printf("constellation %d, bSpectralInversion %d, carrierOffset %d\n", 
   	pChannelScan->constellation, pChannelScan->bSpectralInversion, pChannelScan->carrierOffset); */

    return BERR_SUCCESS;  
}


/******************************************************************************
 BTFE_DvbcConstellationDetect()
******************************************************************************/

BERR_Code BTFE_DvbcConstellationDetect(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BTFE_ConstellationDetect *pConstellationDetect
)
{
    BERR_Code retCode;
    BTFE_ConfigAcquisition acquisition;
    BTFE_ConfigJ83abc j83abc;
    uint8_t buffer[32];
    bool operationDone = false, signalDetected = false;
    int i;
    /* extern uint32_t time_x1, time_x2, timer_tick; */

    BDBG_ASSERT(hTFEChan);
    BDBG_ASSERT(pConstellationDetect);
    BDBG_ASSERT(pConstellationDetect->numConstellation);

    retCode = BTFE_GetConfig(hTFEChan, BTFE_ConfigItem_eAcquisition, &acquisition);
    if (retCode != BERR_SUCCESS)
        return BERR_TRACE(retCode);
		
    acquisition.acqConfig = BTFE_AcquireConfig_eDirectedAcquire;
    acquisition.bandwidthConfig = BTFE_Bandwidth_e8MHZ; 
    retCode = BTFE_SetConfig(hTFEChan, BTFE_ConfigItem_eAcquisition, &acquisition);
    if (retCode != BERR_SUCCESS)
        return BERR_TRACE(retCode);

    j83abc.mode = BTFE_J83_Mode_eA;
    j83abc.symbolRate = pConstellationDetect->symbolRate;

    /* 
     * Can we move BTFE_ConfigItem_eJ83ABC, and startFat() here
     * Use waitOnStart and SCD_CONFIG_ITEM__J83_NEXT_SCAN to replace the following startFat()
     */
		
    pConstellationDetect->bConstellationDetected = false;
	
    for (i = 0; i < pConstellationDetect->numConstellation; ++i)
    {
        j83abc.constellation = pConstellationDetect->constellationArray[i];  
        retCode = BTFE_SetConfig(hTFEChan, BTFE_ConfigItem_eJ83ABC, &j83abc);
        if (retCode != BERR_SUCCESS)
            return BERR_TRACE(retCode);

        buffer[0] = 0; /* clear operationDone */
        if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 1, buffer) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);	

        /* stop current acquisition */
        BTFE_AbortAcq(hTFEChan);

#if 1	
        BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
        BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);

        if (BTFE_P_ScdStart(hTFEChan->pFatHandle,  BTFE_ModulationFormat_eJ83ABC) != BERR_SUCCESS)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
   
        /* time_x1 = timer_tick; */

        retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 5000);
        if (retCode == BERR_TIMEOUT)
        {
            BDBG_WRN(("DVB-C BTFE_DvbcConstellationDetect time out"));		
            /* break; */
        }

        /* time_x2 = timer_tick; printf("time %d\n", (time_x2-time_x1)); */

        if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA, 14, buffer) != SCD_RESULT__OK)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
        operationDone = buffer[12] ? true : false;
        signalDetected = buffer[13] ? true : false;
        /* printf("operationDone %d, signalDetected %d\n", operationDone, signalDetected); */

        if (operationDone && signalDetected)
        {
            pConstellationDetect->bConstellationDetected = true;
	     pConstellationDetect->constellation = j83abc.constellation;	
	     pConstellationDetect->bSpectralInversion = buffer[3] ? true : false;			 
            break;
        }
#else
{
        BTFE_StatusFAT sFatDemod;
        BKNI_WaitForEvent(hTFEChan->hLockStateChangeEvent, 0); 
        BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_LOCK_CHANGE);

        if (BTFE_P_ScdStart(hTFEChan->pFatHandle,  BTFE_ModulationFormat_eJ83ABC) != BERR_SUCCESS)
            return BERR_TRACE(BTFE_ERR_SCD_ERROR);

        retCode = BKNI_WaitForEvent(hTFEChan->hLockStateChangeEvent, 500);
        if (retCode == BERR_TIMEOUT)
        {
            BDBG_WRN(("DVB-C BTFE_DvbcConstellationDetect time out"));		
            /* break; */
        }

        if (retCode == BERR_SUCCESS)
        {
            sFatDemod.flags = SCD_STATUS_FAT__LOCK_STATUS;      
            retCode = BTFE_GetStatus(hTFEChan, BTFE_StatusItem_eFAT, &sFatDemod);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("unable to get BTFE_GetStatus"));
                return BERR_TRACE(retCode);
            }
     
            if (sFatDemod.lockStatus == 1)
            {
                pConstellationDetect->bConstellationDetected = true;
	         pConstellationDetect->constellation = j83abc.constellation;	
		  break;
            }
        }
}
#endif

    }  

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
 BTFE_P_EnableIrq()
******************************************************************************/
void BTFE_P_EnableIrq(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   uint32_t           irq       /* [in] DFE irq(s) to enable */
)   
{
   BDBG_ASSERT(hTFEChan); 
   
   irq &= BTFE_IRQ_ALL;
   if (irq)
   {   
      if (irq & BTFE_IRQ_LOCK_CHANGE)
	   {
	     BINT_ClearCallback(hTFEChan->hLockStatusChangeCb);
	     BINT_EnableCallback(hTFEChan->hLockStatusChangeCb);
	   }
      
	   if (irq & BTFE_IRQ_AUDIO_CLIPPED)
	   {
	     BINT_ClearCallback(hTFEChan->hAudioClippedCb);	  
	     BINT_EnableCallback(hTFEChan->hAudioClippedCb);
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
   BDBG_ASSERT(hTFEChan); 
   
   irq &= BTFE_IRQ_ALL;
   if (irq)
   {   
      if (irq & BTFE_IRQ_LOCK_CHANGE)
	   {
	     BINT_DisableCallback(hTFEChan->hLockStatusChangeCb);
	   }
      
	   if (irq & BTFE_IRQ_AUDIO_CLIPPED)
	   {
	     BINT_DisableCallback(hTFEChan->hAudioClippedCb);
	   }
      
	   if (irq & BTFE_IRQ_SCAN)
	   {
	     BINT_DisableCallback(hTFEChan->hScanCb);
	   }      
   }
}


/******************************************************************************
 BTFE_P_LockStatusChange_isr()
******************************************************************************/
void BTFE_P_LockStatusChange_isr(void *p, int param)
{
   BTFE_ChannelHandle hTFEChan = (BTFE_ChannelHandle)p;

   BSTD_UNUSED(param);
 
   BDBG_MSG(("in BTFE_P_LockStatusChange_isr()"));  
   
   BKNI_SetEvent(hTFEChan->hLockStateChangeEvent);   
}


/******************************************************************************
 BTFE_P_AudioClipped_isr()
******************************************************************************/
void BTFE_P_AudioClipped_isr(void *p, int param)
{
   BTFE_ChannelHandle hTFEChan = (BTFE_ChannelHandle)p;

   BSTD_UNUSED(param);
   
   /* BDBG_MSG(("in BTFE_P_AudioClipped_isr()")); */
   BKNI_SetEvent(hTFEChan->hAudioMaxThresholdEvent);    
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

/******************************************************************************
 BTFE_P_LogSearch()
******************************************************************************/
BERR_Code BTFE_P_LogSearch(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
    BTFE_DvbcScanData *pScanData,
    int32_t defaultIFNomRate,
    int32_t *coarseOffset /* result */
)
{
    BERR_Code retCode;
    SCD_CONFIG_J83_NEXT_SCAN j83ScanData;
    uint8_t waitOnStart;
    uint8_t buffer[32];
    bool operationDone = false, signalDetected = false;
    int32_t rightEnd, leftEnd, dc_log_delta_step;
    int32_t ifNormRate, ifNormRatePast;

    BDBG_ASSERT(pScanData);
    BDBG_ASSERT(pScanData->logSweepFreq);

 
    j83ScanData.j83abcMode = J83A;
    j83ScanData.acqMode = SCD_SYMBOL_RATE_VERIFY;
    j83ScanData.qamMode = 0;
    j83ScanData.bandwidth= SCD_BW_8MHZ;	  
    j83ScanData.symbolRate= pScanData->symbolRate;

    rightEnd = -1;
    dc_log_delta_step = pScanData->logSweepFreq;
    ifNormRate = defaultIFNomRate;
    operationDone = signalDetected = false;
	
    do
    {
        ifNormRatePast = ifNormRate;
	 ifNormRate = ifNormRate + dc_log_delta_step;

	 /* printf("ifNormRate %d, dc_log_delta_step %d, ifNormRatePast %d, rightEnd %d\n", 
	 		ifNormRate, dc_log_delta_step, ifNormRatePast, rightEnd); */
	 
	 if (ifNormRate == rightEnd)
	 {
             signalDetected = false;
             operationDone = true;
	 }
	 else
	 {  
            j83ScanData.carrierOffset = ifNormRate;
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
            BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
            waitOnStart = 1;
            if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

            /* wait for symbol rate verify to complete */
            retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 500);
            if (retCode == BERR_TIMEOUT)
            {
                BDBG_WRN(("DVB-C symbol rate verify time out"));
                break;
            }
		 
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            operationDone = buffer[0] ? true : false;
            signalDetected = buffer[1] ? true : false;
	 }

        if (operationDone == false)
        {
            BDBG_WRN(("DVB-C operation done is false"));
            break;
        }
			
        if (operationDone && (signalDetected == false))
        {
            /* printf("	ifNormRate %d, FAIL\n", ifNormRate); */       
            rightEnd = ifNormRate;
            ifNormRate  = ifNormRatePast;		
            dc_log_delta_step = dc_log_delta_step/2;
        }
/*        else
            printf("	ifNormRate %d, PASS\n", ifNormRate); */
   } while (dc_log_delta_step > (pScanData->logSweepFreq /16));

/* printf("rightEnd %d\n", rightEnd); */

    leftEnd = -1;
    dc_log_delta_step = -(pScanData->logSweepFreq);
    ifNormRate = defaultIFNomRate;

    operationDone = signalDetected = false;
	
    do
    {
        ifNormRatePast = ifNormRate;
	 ifNormRate = ifNormRate + dc_log_delta_step;

	 /* printf("ifNormRate %d, dc_log_delta_step %d, ifNormRatePast %d, leftEnd %d\n", 
	 		ifNormRate, dc_log_delta_step, ifNormRatePast, leftEnd); */
	 
	 if (ifNormRate == leftEnd)
	 {
             signalDetected = false;
             operationDone = true;
	 }
	 else
	 {  
            j83ScanData.carrierOffset = ifNormRate;
            if (BTFE_P_ScdSetConfig(hTFEChan->pFatHandle, SCD_CONFIG_ITEM__J83_NEXT_SCAN, (void *)&j83ScanData, sizeof(j83ScanData)) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            BTFE_P_EnableIrq(hTFEChan, BTFE_IRQ_SCAN);
            BKNI_WaitForEvent(hTFEChan->hScanEvent, 0); 
		  
            waitOnStart = 1;
            if (BTFE_P_ScdWriteChip(hTFEChan->pFatHandle, 0, ixHI_J83_CONFIG_AREA+12, 1, &waitOnStart) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);  

            /* wait for symbol rate verify to complete */
            retCode = BKNI_WaitForEvent(hTFEChan->hScanEvent, 500);
            if (retCode == BERR_TIMEOUT)
            {
                BDBG_WRN(("DVB-C symbol rate verify time out"));
                break;
            }
		 
            if (BTFE_P_ScdReadChip(hTFEChan->pFatHandle, 0, ixHI_J83_STATUS_AREA+12, 2, buffer) != SCD_RESULT__OK)
                return BERR_TRACE(BTFE_ERR_SCD_ERROR);
		 
            operationDone = buffer[0] ? true : false;
            signalDetected = buffer[1] ? true : false;
	 }
        if (operationDone == false)
        {
            BDBG_WRN(("DVB-C operation done is false"));
            break;
        }
			
        if (operationDone && (signalDetected == false))
        {
            /* printf("	ifNormRate %d, FAIL\n", ifNormRate); */
            leftEnd = ifNormRate;
            ifNormRate  = ifNormRatePast;		
            dc_log_delta_step = dc_log_delta_step/2;
        }	
/*        else
            printf("	ifNormRate %d, PASS\n", ifNormRate); */
   } while ((-dc_log_delta_step) > (pScanData->logSweepFreq/16));


    *coarseOffset = (rightEnd+leftEnd)/2;
    /* printf("carrier offset %d, rightEnd %d, leftEnd %d\n", (rightEnd+leftEnd)/2, rightEnd, leftEnd); */
	
    return BERR_SUCCESS;  
}

