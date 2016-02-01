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
* Module Description:
*   Module name: HIFIDAC
*   This file contains the implementation of all PIs for HIFI DAC output. 
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_hifidac);

#define   BRAP_DAC_P_DEFAULT_VOLUME			0 /* in dB. 0 dB => 0x8000 => full volume */
#define	BRAP_DAC_P_MAX_1FFFF_DB_VOLUME		101
#define	BRAP_DAC_P_FRACTION_IN_DB			100


static const unsigned int	BRAP_DacVol1FFFF_DB[] = 

{
       0x1ffff,        0x1c852,        0x196b2,        0x16a78,
       0x1430d,        0x11feb,        0x1009c,        0x0e4b4,
       0x0cbd5,        0x0b5aa,        0x0a1e9,        0x0904d,
       0x0809c,        0x072a0,        0x06629,        0x05b0d,
       0x05126,        0x04853,        0x04075,        0x03973,
       0x03334,        0x02da2,        0x028ac,        0x02440,
       0x0204f,        0x01ccb,        0x019aa,        0x016df,
       0x01463,        0x0122b,        0x01031,        0x00e6f,
       0x00cdd,        0x00b77,        0x00a38,        0x0091b,
       0x0081e,        0x0073c,        0x00673,        0x005bf,
       0x0051f,        0x00491,        0x00412,        0x003a0,
       0x0033c,        0x002e2,        0x00291,        0x0024a,
       0x0020a,        0x001d2,        0x0019f,        0x00172,
       0x0014a,        0x00126,        0x00106,        0x000ea,
       0x000d0,        0x000ba,        0x000a6,        0x00094,
       0x00084,        0x00075,        0x00069,        0x0005d,
       0x00053,        0x0004a,        0x00042,        0x0003b,
       0x00035,        0x0002f,        0x0002a,        0x00025,
       0x00021,        0x0001e,        0x0001b,        0x00018,
       0x00015,        0x00013,        0x00011,        0x0000f,
       0x0000e,        0x0000c,        0x0000b,        0x0000a,
       0x00009,        0x00008,        0x00007,        0x00006,
       0x00006,        0x00005,        0x00005,        0x00004,
       0x00004,        0x00003,        0x00003,        0x00003,
       0x00003,        0x00002,        0x00002,        0x00002,
       0x00002,        0x00000
};


static const BRAP_OP_DacToneInfo defDacTestToneInfo = 
{
  {
	0,68433,135695,200636,262143,319166,370727,415945,
	454046,484378,506422,519802,524287,519802,506422,484378,
	454046,415945,370727,319166,262143,200636,135695,68433,
	0,-68433,-135695,-200636,-262143,-319166,-370727,-415945,
	-454046,-484378,-506422,-519802,-524287,-519802,-506422,-484378,
	-454046,-415945,-370727,-319166,-262144,-200636,-135695,-68433,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
  },
  BRAP_OP_DacLROutputCtrl_eNoZeros,
  true,
  0,
  48,
  BAVC_AudioSamplingRate_e48k
};

static BERR_Code BRAP_OP_P_GetDacRateMgrSR(
    BAVC_AudioSamplingRate      eSR,
    BRAP_OP_P_DacRateMangerSR   *pDacRateMangerSR
    )
{
    BERR_Code ret=BERR_SUCCESS;

    BDBG_ENTER(BRAP_OP_P_GetDacRateMgrSR);

    BDBG_ASSERT(pDacRateMangerSR);
    
    switch (eSR)
    {
        case BAVC_AudioSamplingRate_e32k:
            pDacRateMangerSR->eSamplingRate = eSR;
            pDacRateMangerSR->uiDenominator = 1024;
            pDacRateMangerSR->uiNumerator = 303;
            pDacRateMangerSR->uiSampleInc = 3;
            pDacRateMangerSR->uiPhaseInc = 0x0009b583;
            break;
        case BAVC_AudioSamplingRate_e44_1k:
            pDacRateMangerSR->eSamplingRate = eSR;
            pDacRateMangerSR->uiDenominator = 784;
            pDacRateMangerSR->uiNumerator = 307;
            pDacRateMangerSR->uiSampleInc = 2;
            pDacRateMangerSR->uiPhaseInc = 0x000d6159;
            break;
        case BAVC_AudioSamplingRate_e48k:
            pDacRateMangerSR->eSamplingRate = eSR;
            pDacRateMangerSR->uiDenominator = 512;
            pDacRateMangerSR->uiNumerator = 101;
            pDacRateMangerSR->uiSampleInc = 2;
            pDacRateMangerSR->uiPhaseInc = 0x000e9045;
            break;
        case BAVC_AudioSamplingRate_e96k:
            pDacRateMangerSR->eSamplingRate = eSR;
            pDacRateMangerSR->uiDenominator = 256;
            pDacRateMangerSR->uiNumerator = 101;
            pDacRateMangerSR->uiSampleInc = 4;
            pDacRateMangerSR->uiPhaseInc = 0x00074823;
            break;
        case BAVC_AudioSamplingRate_e192k:
            pDacRateMangerSR->eSamplingRate = eSR;
            pDacRateMangerSR->uiDenominator = 512;
            pDacRateMangerSR->uiNumerator = 101;
            pDacRateMangerSR->uiSampleInc = 2;
            pDacRateMangerSR->uiPhaseInc = 0x00e9045;
            break;
        default:
            BDBG_ERR(("BRAP_OP_P_GetDacRateMgrSR: Not a Supported SR = %d for HiFiDAC",eSR));
            pDacRateMangerSR->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
            ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
    }

    BDBG_LEAVE(BRAP_OP_P_GetDacRateMgrSR);
    return ret;
}

 uint32_t BRAP_OP_P_GetDacOffset(BRAP_OutputPort eOpType, uint32_t *pOffset)
{
    switch ( eOpType )
    {
    case BRAP_OutputPort_eDac0:
        *pOffset = 0;
        break;
#ifdef BCHP_HIFIDAC_CTRL1_REVISION 
    case BRAP_OutputPort_eDac1:
        *pOffset = (BCHP_HIFIDAC_CTRL1_REVISION -  BCHP_HIFIDAC_CTRL0_REVISION);
        break;
#endif
#ifdef BCHP_HIFIDAC_CTRL2_REVISION 
    case BRAP_OutputPort_eDac2:
        *pOffset = (BCHP_HIFIDAC_CTRL2_REVISION -  BCHP_HIFIDAC_CTRL0_REVISION);
        break;
#endif
    default:
        BDBG_ERR(("Invalid DAC output port %d", eOpType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}

BERR_Code
BRAP_OP_P_DacOpen (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_DacSettings * pSettings  /*[in] Open time parameters */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0;	
	uint32_t	i32Count = 0;
	uint32_t    ui32Temp=0, ui32AttenDb=0, ui32Delta=0;

	BDBG_ENTER (BRAP_OP_P_DacOpen);
	BDBG_ASSERT (hOp);
    BSTD_UNUSED(pSettings);

#if ((BCHP_CHIP==7420) && (BCHP_VER == A0))
        /* 7420 DAC ISSUE::HACK::*/
        /* 7420 Bringup: To get audio on one DAC, other DAC needs to be powered down.
            Powering down DAC1 always for bringup purpose. This should be fixed properly
            later. DAC1 will not work with this hack. For now, printing warning from function
            BRAP_RM_P_IsOutputPortSupported() if DAC1 is added. */
            if(BRAP_OutputPort_eDac0==hOp->eOutputPort)
            {
                ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_AIO_MISC_PWRDOWN);
                /* Power down DAC1 */
                ui32RegValue |= BCHP_MASK(AIO_MISC_PWRDOWN, DAC1);
                BRAP_Write32(hOp->hRegister, BCHP_AIO_MISC_PWRDOWN, ui32RegValue);
            }
#endif            

	/* Program these registers properly by reading
	the BRAP_OP_P_DacSettings fields */

	/* According to the info from Keith the test samples buffers must be made zero
	to avoid non zero samples getting out after power up */

    	/* Program HIFIDAC_CTRL0_TEST: RAM_BUF_SEL to  samples */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,RAM_BUF_SEL));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
                                            RAM_BUF_SEL, Samples ));
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset,
		ui32RegValue);

    	/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  enable */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_ENABLE));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
                                            TESTTONE_ENABLE, Enable ));
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset,
		ui32RegValue);

	/* Initialize the test samples to Zero */
	for ( i32Count = 0; i32Count <= BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_END; i32Count++ )
	{
		BRAP_Write32 (hOp->hRegister, 
			BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + hOp->ui32Offset + ( i32Count*4),
			0);
	}

    	/* Program HIFIDAC_CTRL0_TEST: RAM_BUF_SEL to  Pingpong */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,RAM_BUF_SEL));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
                                            RAM_BUF_SEL, Pingpong ));
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset,
		ui32RegValue);

	/* Initialize the test samples to Zero */
	for ( i32Count = 0; i32Count <= BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_END; i32Count++ )
	{
		BRAP_Write32 (hOp->hRegister, 
			BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + hOp->ui32Offset + ( i32Count*4),
			0);
	}

    	/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  Normal_operation */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_ENABLE));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
                                            TESTTONE_ENABLE, Normal_operation ));
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_TEST + hOp->ui32Offset,
		ui32RegValue);

	/* By Default unmute the RF mod output while opening a DAC.
	* This is based on the request from Jon.
	*/
	ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
					RFMOD_MUTE, Normal_operation ));    
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset,
		ui32RegValue);

	/*  program  MAPPER_SOFTMUTE to normal operation
	* Byt default it is in mute state 
	*/
    	/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  Normal_operation */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG,MAPPER_SOFTMUTE));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SOFTMUTE, Normal_operation ));
	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset,
		ui32RegValue);	

        if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
        {
        /* Set default volume */
        ui32AttenDb = BRAP_DAC_P_DEFAULT_VOLUME
                        / BRAP_DAC_P_FRACTION_IN_DB;

        
	ui32Temp = BRAP_DacVol1FFFF_DB[ui32AttenDb];   
	ui32Delta = ui32Temp - BRAP_DacVol1FFFF_DB[ui32AttenDb+1];
	ui32Delta = (ui32Delta * 
			(BRAP_DAC_P_DEFAULT_VOLUME%BRAP_DAC_P_FRACTION_IN_DB))
			/(BRAP_DAC_P_FRACTION_IN_DB);
	ui32Temp -= ui32Delta;        
    
	BDBG_MSG (("BRAP_OP_P_DacOpen:" 
			" BRAP_DAC_P_DEFAULT_VOLUME=%d dB, converted reg val =0x%x",
			BRAP_DAC_P_DEFAULT_VOLUME, ui32Temp));     
        }
        else
        {
            /* Set default volume */
            ui32AttenDb = (hOp->ui32DacVolume)
                                / BRAP_DAC_P_FRACTION_IN_DB;


            ui32Temp = BRAP_DacVol1FFFF_DB[ui32AttenDb];   
            ui32Delta = ui32Temp - BRAP_DacVol1FFFF_DB[ui32AttenDb+1];
            ui32Delta = (ui32Delta * 
            ((hOp->ui32DacVolume)%BRAP_DAC_P_FRACTION_IN_DB))
                                /(BRAP_DAC_P_FRACTION_IN_DB);
            ui32Temp -= ui32Delta;        

            BDBG_MSG (("BRAP_OP_P_DacOpen:" 
                                " BRAP_DAC_P_DEFAULT_VOLUME=%d dB, converted reg val =0x%x",
                                (hOp->ui32DacVolume), ui32Temp));     
        }

	ui32RegValue = 0;
	ui32RegValue |= (BCHP_FIELD_DATA (    
		HIFIDAC_CTRL0_DAC_VOLUME, 
		DAC_VOL, 
		ui32Temp));  

	BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hOp->ui32Offset,
		ui32RegValue);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {
	hOp->bDacMuteState = false; /* It is set to unmute state */
	hOp->ui32DacVolume = BRAP_DAC_P_DEFAULT_VOLUME;    
    }
	
	BDBG_LEAVE (BRAP_OP_P_DacOpen);
	return ret;
}

BERR_Code BRAP_OP_P_DacSetTimebase(
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
	BAVC_Timebase		  eTimebase			 /*[in] time base to program */
)
{
	BERR_Code ret = BERR_SUCCESS;
	
	BDBG_ENTER (BRAP_OP_P_DacSetTimebase);

    BKNI_EnterCriticalSection();
    ret = BRAP_OP_P_DacSetTimebase_isr(hOp, eTimebase);
    BKNI_LeaveCriticalSection();
    
 	BDBG_LEAVE(BRAP_OP_P_DacSetTimebase);
	return ret;
}

BERR_Code BRAP_OP_P_DacSetTimebase_isr(
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
	BAVC_Timebase		  eTimebase			 /*[in] time base to program */
)
{
	uint32_t ui32RegValue;
	BERR_Code ret = BERR_SUCCESS;
	
	BDBG_ENTER (BRAP_OP_P_DacSetTimebase_isr);
	BDBG_ASSERT (hOp);

	/* Program the TimeBase Value */
	ui32RegValue = BRAP_Read32_isr (hOp->hRegister, BCHP_HIFIDAC_RM0_CONTROL + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_RM0_CONTROL,TIMEBASE));

	switch (eTimebase)
	{
		case BAVC_Timebase_e0:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_RM0_CONTROL, 
					TIMEBASE, TIMEBASE_0));
			break;
		case BAVC_Timebase_e1:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_RM0_CONTROL, 
					TIMEBASE, TIMEBASE_1));			
			break;
		case BAVC_Timebase_e2:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_RM0_CONTROL, 
					TIMEBASE, TIMEBASE_2));
			break;
		case BAVC_Timebase_e3:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_RM0_CONTROL, 
					TIMEBASE, TIMEBASE_3));
			break;
		default:
			BDBG_ERR (("BRAP_OP_P_DacSetTimebase_isr(): Unsupported Time Base %d", 
                     eTimebase));
       		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}
	
	BRAP_Write32_isr (hOp->hRegister, 
		BCHP_HIFIDAC_RM0_CONTROL + hOp->ui32Offset,
		ui32RegValue);

 	BDBG_LEAVE(BRAP_OP_P_DacSetTimebase_isr);
	return ret;
}

BERR_Code BRAP_OP_P_DacHWConfig (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_DacSettings * pSettings /*[in] Parameters */
)
{
    uint32_t  ui32RegValue = 0,ui32RegValue1 = 0,ui32DacVol =0;
    BRAP_OP_P_DacSettings sSettings;
    unsigned int i=0;
    unsigned int ui32MapperLeft=0,ui32MapperRight=0;
    bool    bMuteRequired=false;
    BDBG_ENTER (BRAP_OP_P_DacHWConfig);
    BDBG_ASSERT (hOp);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {   /* If not in WatchDog recovery */  
        BDBG_ASSERT (pSettings);
        sSettings = *pSettings;
    }
    else
    {
        sSettings = hOp->uOpSettings.sDac;
    }
    BDBG_MSG(("BRAP_OP_P_DacHWConfig: Settings:"));
    BDBG_MSG(("ChannelOrder = %d", sSettings.sExtSettings.eChannelOrder));
    BDBG_MSG(("MuteType = %d", sSettings.sExtSettings.eMuteType));
    BDBG_MSG(("RightDacOutputVal = %d", sSettings.sExtSettings.ui32RightDacOutputVal));
    BDBG_MSG(("LeftDacOutputVal = %d", sSettings.sExtSettings.ui32LeftDacOutputVal));
    BDBG_MSG(("Scale = %d", sSettings.sExtSettings.ui32Scale));

	/* Program the Mute Type */
	ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset);

#if  (BRAP_7405_FAMILY == 1) || (BRAP_3548_FAMILY == 1)
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_MUTETYPE));

	switch (sSettings.sExtSettings.eMuteType)
	{
		case BRAP_OP_DacMuteType_Drive_aaaa:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_MUTETYPE, Drive_aaaa));
			break;     
		case BRAP_OP_DacMuteType_Drive_5555:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_MUTETYPE, Drive_5555));
			break;     
		case BRAP_OP_DacMuteType_Use_reg_val:
			ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_MUTETYPE, Use_reg_val));
                    ui32RegValue1 = 0;
                    ui32RegValue1 |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_DAC_CONST_VAL, 
                                            RIGHT,sSettings.sExtSettings.ui32RightDacOutputVal));
                    ui32RegValue1 |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_DAC_CONST_VAL, 
                                            LEFT,sSettings.sExtSettings.ui32LeftDacOutputVal));
                    BRAP_Write32 (hOp->hRegister, 
                                           BCHP_HIFIDAC_CTRL0_DAC_CONST_VAL + hOp->ui32Offset,
                                             ui32RegValue1);                    
            
			break;                 
		case BRAP_OP_DacMuteType_ConstantHigh:
		case BRAP_OP_DacMuteType_SquareWaveOpp:          
		case BRAP_OP_DacMuteType_SquareWaveSame:
			break;     
		default:
			BDBG_ERR (("BRAP_OP_P_DacHWConfig(): Unsupported Mute Type %d", 
                     sSettings.sExtSettings.eMuteType));
            		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}     
#endif


    ui32RegValue1 = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset);
    ui32MapperLeft = BCHP_GET_FIELD_DATA(ui32RegValue1,HIFIDAC_CTRL0_CONFIG,MAPPER_SELECT_LEFT);
    ui32MapperRight = BCHP_GET_FIELD_DATA(ui32RegValue1,HIFIDAC_CTRL0_CONFIG,MAPPER_SELECT_RIGHT);        
        
    /* Set Channel Order */
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT));
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT));
    switch (sSettings.sExtSettings.eChannelOrder)
    {
        case BRAP_OP_ChannelOrder_eNormal:
            if(!((ui32MapperLeft ==0)&&(ui32MapperRight==1)))
            {
                bMuteRequired =true;
            }
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_LEFT, Left));
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_RIGHT, Right));            
            break;
        case BRAP_OP_ChannelOrder_eBothLeft:
            if(!((ui32MapperLeft ==0)&&(ui32MapperRight==0)))
            {
                bMuteRequired =true;
            }            
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_LEFT, Left));
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_RIGHT, Left));   
            break;
        case BRAP_OP_ChannelOrder_eBothRight:
            if(!((ui32MapperLeft ==1)&&(ui32MapperRight==1)))
            {
                bMuteRequired =true;
            }                        
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_LEFT, Right));
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_RIGHT, Right));   
            break;
        case BRAP_OP_ChannelOrder_eSwap:
            if(!((ui32MapperLeft ==1)&&(ui32MapperRight==0)))
            {
                bMuteRequired =true;
            }                        
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_LEFT, Right));
            ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                                            MAPPER_SELECT_RIGHT, Left));   
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_DacHWConfig(): Incorrect Channel Order %d", 
                        sSettings.sExtSettings.eChannelOrder));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }      

    if(bMuteRequired == true)
    {
        /*Muting  DAC before changing Channel Order to prevent audio Pop*/
        ui32RegValue1 =0;
        ui32RegValue1 = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hOp->ui32Offset);
        ui32DacVol = ui32RegValue1;
        BRAP_Write32 (hOp->hRegister, 
                      BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hOp->ui32Offset,
                      0);
        /*Waiting for Ramp down to happen*/
        for(i = 0 ; i < 1000 ; i++)
        {
            ui32RegValue1 = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_STATUS + hOp->ui32Offset);
            ui32RegValue1 = BCHP_GET_FIELD_DATA(ui32RegValue1,HIFIDAC_CTRL0_STATUS,VOLUME_AT_TARGET);
            if(ui32RegValue1 != 0)
                break;
            BKNI_Sleep(1);            
        }
        if(i>=1000)
        {
            BDBG_WRN(("Volume didn't ramped down"));
        }
    }

    /* Programming Channel Order */
	BRAP_Write32 (hOp->hRegister, 
                  BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset,
                  ui32RegValue);
    
    if(bMuteRequired == true)
    {
        /*Restoring the Volume for DAC*/
    	BRAP_Write32 (hOp->hRegister, 
                      BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hOp->ui32Offset,
                      ui32DacVol);
        /*Waiting for Ramp up to happen*/
        for(i = 0 ; i < 1000 ; i++)
        {
            ui32RegValue1 = BRAP_Read32(hOp->hRegister, BCHP_HIFIDAC_CTRL0_STATUS + hOp->ui32Offset);
            ui32RegValue1 = BCHP_GET_FIELD_DATA(ui32RegValue1,HIFIDAC_CTRL0_STATUS,VOLUME_AT_TARGET);            
            if(ui32RegValue1 != 0)
                break;
            BKNI_Sleep(1);            
        }
        if(i>=1000)
        {
            BDBG_WRN(("Volume didn't ramped up"));
        }
    }

    /* Programming the given scale  parameter */ 
	ui32RegValue = 0;
	ui32RegValue |= ( BCHP_FIELD_DATA (HIFIDAC_CTRL0_SCALE, SCALE, 
                    pSettings->sExtSettings.ui32Scale));
    BRAP_Write32 (hOp->hRegister, 
                    BCHP_HIFIDAC_CTRL0_SCALE + hOp->ui32Offset, ui32RegValue);
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {  
        /* store the settings in hOp */
        hOp->uOpSettings.sDac = sSettings;
    }
    return BERR_SUCCESS;    
}


BERR_Code
BRAP_OP_P_DacStart (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_DacParams * pParams  /*[in] Open time parameters */
)
{

/*     Algo:
    1. Configure the DAC port ie program register BCHP_HIFIDAC_CTRL0_CONFIG
        with the settings saved in hRap ie
        hRap->sOutputSettings[BRAP_OutputPort_eDacX].uOutputPortSettings.sDacSettings
        by calling BRAP_OP_P_DacHWConfig()  
    2. Cofigure Timebase ie BCHP_HIFIDAC_RM0_CONTROL
    3. Enable upstream request. 
    4. set volume
    5. If the port was unmuted, enable data flow.
*/    

	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0;
	uint32_t ui32Temp=0, ui32AttenDb=0, ui32Delta=0;
	BRAP_OP_P_DacParams sParams;
	BRAP_OP_P_DacSettings sSettings;    
#if ((BRAP_3548_FAMILY != 1))
    uint32_t ui32StreamId =0;
#endif


	BDBG_ENTER (BRAP_OP_P_DacStart);
	BDBG_ASSERT (hOp);

	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{ 
		BDBG_ASSERT (pParams);
		hOp->uOpParams.sDac= *pParams;
		sParams = *pParams;
	}
	else
	{
	    sParams = hOp->uOpParams.sDac; 
	}	

	/* Program these registers properly by reading
	the BRAP_OP_P_DacParams fields */
	BDBG_MSG (("BRAP_OP_P_DacStart: eTimeBase=%d", sParams.eTimebase));

    /* Configure the Dac port */
    sSettings.sExtSettings = 
        hOp->hFmm->hRap->sOutputSettings[hOp->eOutputPort].uOutputPortSettings.sDacSettings;    
    BRAP_OP_P_DacHWConfig (hOp, &sSettings);

/* TODO: Check the logic */
    /* Program the FCI ID of the Input Mixer */
    ui32RegValue = 0;
#if ((BRAP_3548_FAMILY == 1))
    if (BRAP_OutputPort_eDac0 ==  hOp->eOutputPort)
    {
        ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP32);
        ui32RegValue &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP32, STREAM2));
        ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP32, STREAM2, hOp->uOpParams.sDac.ui32InputFciId);
        BRAP_Write32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP32, ui32RegValue);
    } 
    else if (BRAP_OutputPort_eDac1 ==  hOp->eOutputPort)
    {
        ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98);
        ui32RegValue &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM8));
        ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM8, hOp->uOpParams.sDac.ui32InputFciId);
        BRAP_Write32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, ui32RegValue);
    }
    else if (BRAP_OutputPort_eDac2 ==  hOp->eOutputPort)
    {
        ui32RegValue = BRAP_Read32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc);
        ui32RegValue &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, STREAM13));
        ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, STREAM13, hOp->uOpParams.sDac.ui32InputFciId);
        BRAP_Write32(hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, ui32RegValue);
    }
	
#if (BRAP_P_RESET_IOP_SM==1)
	/* To prevent last sample from previous run getting played again, reset HW internal state
	 * machine by writing 1 to INIT_SM bit corresponding to this output port. Reset this bit immediately */
	ui32RegValue = BRAP_Read32( hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL );
	ui32RegValue &= ~( BCHP_MASK( AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, INIT_SM2 ));
	ui32RegValue |= BCHP_FIELD_ENUM( AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, INIT_SM2, Init );
	BRAP_Write32( hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, ui32RegValue );
	/* Reset immediately */
	ui32RegValue &= ~( BCHP_MASK( AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, INIT_SM2 ));
	ui32RegValue |= BCHP_FIELD_ENUM( AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, INIT_SM2, Normal );
	BRAP_Write32( hOp->hRegister, BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_FCI_CTRL, ui32RegValue );
#endif

#else 

    switch ( hOp->eOutputPort )
    {
    case BRAP_OutputPort_eDac0:
        ui32StreamId = 2;
        break;
#ifdef BCHP_HIFIDAC_CTRL1_REVISION 
    case BRAP_OutputPort_eDac1:
#if (BRAP_7420_FAMILY == 1)        
        ui32StreamId = 9;
#endif
        break;
#endif
#ifdef BCHP_HIFIDAC_CTRL2_REVISION 
    case BRAP_OutputPort_eDac2:
        break;
#endif
    default:
        BDBG_ERR(("Invalid DAC output port %d", hOp->eOutputPort));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    ui32RegValue = BRAP_Read32(hOp->hRegister , 
                (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (ui32StreamId*4)));

    ui32RegValue &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID));
    ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID,hOp->uOpParams.sDac.ui32InputFciId);

    BDBG_MSG(("DAC InputFciId = 0x%x\n",hOp->uOpParams.sDac.ui32InputFciId));
    BRAP_Write32(hOp->hRegister , (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + 
                            (ui32StreamId*4)),ui32RegValue);
#endif
	/* Program the TimeBase Value */
	ret = BRAP_OP_P_DacSetTimebase(hOp, sParams.eTimebase);
    if(BERR_SUCCESS != ret) {return BERR_TRACE(ret);}
 	
	/* Enables the Stream2 for HifiDAC 0 output */
    ret = BRAP_OP_P_EnableStream (hOp, true);
    if(BERR_SUCCESS != ret) {return BERR_TRACE(ret);}

	/* Set the volume level stored in the handle */
        ui32AttenDb = hOp->ui32DacVolume / BRAP_DAC_P_FRACTION_IN_DB;

	ui32Temp = BRAP_DacVol1FFFF_DB[ui32AttenDb];   
	ui32Delta = ui32Temp - BRAP_DacVol1FFFF_DB[ui32AttenDb+1];
	ui32Delta = (ui32Delta * (hOp->ui32DacVolume %BRAP_DAC_P_FRACTION_IN_DB))
			/(BRAP_DAC_P_FRACTION_IN_DB);
	ui32Temp -= ui32Delta;        
    
	BDBG_MSG (("BRAP_OP_P_DacStart:" 
			" hOp->ui32DacVolume =%d dB, converted reg val =0x%x",
			hOp->ui32DacVolume , ui32Temp));     

	ui32RegValue = 0;
	ui32RegValue |= (BCHP_FIELD_DATA (    
		HIFIDAC_CTRL0_DAC_VOLUME, 
		DAC_VOL, 
		ui32Temp));  

        BRAP_Write32 (hOp->hRegister, 
                   BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hOp->ui32Offset,
                  ui32RegValue);

	/* Maintain the Mute status on channel change */
	if ( hOp->bDacMuteState == false )
	{
		/* Unmute the DAC port */
		ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ hOp->ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_unmute ));
		BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hOp->ui32Offset,
		ui32RegValue);
	}
	else
	{
		/* Mute the DAC port */
		ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ hOp->ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_mute ));
		BRAP_Write32 (hOp->hRegister, 
		BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hOp->ui32Offset,
		ui32RegValue);
	}

    /* Program the PEAK registers. This was required so that frequency response at high frequency does not roll off by more than -1dB */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_CONFIG + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_PEAK_CONFIG,PEAK_ENABLE));
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_PEAK_CONFIG,PEAK_ROUND_TYPE));    
#if (HIFIDAC_VERSION==130)
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_PEAK_CONFIG, PEAK_ENABLE, Disable));
#else
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_PEAK_CONFIG, PEAK_ENABLE, Enable));
#endif

	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_PEAK_CONFIG, PEAK_ROUND_TYPE, Magnitude_truncate));    
	BRAP_Write32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_CONFIG + hOp->ui32Offset, ui32RegValue);


	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_GAIN + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_PEAK_GAIN,PEAK_GAIN));

    if(hOp->uOpParams.sDac.bBtscOnDAC == false)
    {
#if (HIFIDAC_VERSION==130)
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_GAIN, PEAK_GAIN, 0x0));    
#else
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_GAIN, PEAK_GAIN, 0x3e539));    
#endif
    }
    else
    {
        ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_GAIN, PEAK_GAIN, 0xe100));        
    }

	BRAP_Write32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_GAIN + hOp->ui32Offset, ui32RegValue);


	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_A1 + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_PEAK_A1,PEAK_A1));

    if(hOp->uOpParams.sDac.bBtscOnDAC == false)
    {
#if (HIFIDAC_VERSION==130)   
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A1, PEAK_A1, 0x0));    
#else
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A1, PEAK_A1, 0x1904f));    
#endif
    }
    else
    {
    	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A1, PEAK_A1, 0x0));    
    }


	BRAP_Write32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_A1 + hOp->ui32Offset, ui32RegValue);

	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_A2 + hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_PEAK_A2,PEAK_A2));
    
    if(hOp->uOpParams.sDac.bBtscOnDAC == false)
    {    
#if (HIFIDAC_VERSION==130)   
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A2, PEAK_A2, 0x0));    
#else
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A2, PEAK_A2, 0x35088));    
#endif    
    }
    else
    {
    	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_PEAK_A2, PEAK_A2, 0x0));    
    }

	BRAP_Write32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_PEAK_A2 + hOp->ui32Offset, ui32RegValue);    
    

	/* unmute the DACALL Control port */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL+ hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL,MUTEALL));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL, 
                                            MUTEALL, Ramp_unmute ));
	BRAP_Write32 (hOp->hRegister, 
			BCHP_HIFIDAC_CTRL0_MUTECTRL + hOp->ui32Offset,
			ui32RegValue);

	BDBG_LEAVE (BRAP_OP_P_DacStart);
	return ret;
}

BERR_Code
BRAP_OP_P_DacStop (
    BRAP_OP_P_Handle      hOp
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0;	

	BDBG_ENTER (BRAP_OP_P_DacStop);
	BDBG_ASSERT (hOp);

	BDBG_MSG (("BRAP_OP_P_DacStop: DAC Stoped and muted"));

	/* Mute the DAC port */
	ui32RegValue = BRAP_Read32 (hOp->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ hOp->ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
	ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
	                                    MUTEDAC, Ramp_mute ));
	BRAP_Write32 (hOp->hRegister, 
	BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hOp->ui32Offset,
	ui32RegValue);

	/* Disables the Stream2 for HifiDAC 0 output */
       ret = BRAP_OP_P_EnableStream (hOp, false);
	
	BDBG_LEAVE (BRAP_OP_P_DacStop);
	return ret;
}

/***************************************************************************
Summary:
	Set the new volume level for the selected HiFiDAC.

Description:
	This function sets audio volume for HiFi DAC
	HiFi DAC volume control is linear in hardware ranging from 0
	min to 1FFFF max volume.  This PI has done a mapping
	from this linear range to 1/100 of DB.
	This function gets values in 1/100 of DB from 0 max to 10200 1/100 DB min,
	and for all values above 10200 the volume will be set to 0 linear
	or -102 DB.  Note: For fractions in DB a linear interpolation is used.
	PI maintains this volume upon channel change.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_GetDacVolume()
	BRAP_MIXER_SetOutputVolume()

Note: 
    The function BRAP_MIXER_SetOutputVolume() can also be used to set the DAC 
    volume. It is a generic function which can be used for any output port. It 
    works by scaling the data inside the Raptor Audio core at the mixer level.
    BRAP_OP_SetDacVolume() is an extra level of volume control specifically for 
    the DACs. It does volume control by specifically programming the DAC 
    registers.
    
****************************************************************************/
BERR_Code
BRAP_OP_SetDacVolume (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    uint32_t ui32Volume			/* [in] volume attenuation in 1/100 dB*/
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32VolRegValue = 0;	
	uint32_t	ui32RegValue = 0;	
	uint32_t	ui32AttenDb=0, ui32Delta=0;

	BDBG_ENTER(BRAP_OP_SetDacVolume);
	BDBG_ASSERT (hRap);
	
	BDBG_MSG(("BRAP_OP_SetDacVolume(): eOpType=%d Vol=%d", eOpType, ui32Volume));

    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_SetDacVolume: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

    if( eOpType == BRAP_OutputPort_eDac0)
    {
        ret = BRAP_P_IsVolumeControlSupported(hRap,eOpType,BRAP_ProcessingType_eBtsc);
        if(ret != BERR_SUCCESS)
        {     
			BDBG_ERR(("BRAP_OP_SetDacVolume:BRAP_P_IsVolumeControlSupported returned "
				  "Error"));                
			return ret;                
        }        
    }
    
	if ( hRap->hFmm[0]->hOp[eOpType] == NULL )
	{
		BDBG_ERR (("BRAP_OP_SetDacVolume(): Output port is not yet opened!!!"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	ui32AttenDb = ui32Volume/BRAP_DAC_P_FRACTION_IN_DB;

	if ( ui32AttenDb >= BRAP_DAC_P_MAX_1FFFF_DB_VOLUME)
	{
		ui32Volume = BRAP_DAC_P_MAX_1FFFF_DB_VOLUME*BRAP_DAC_P_FRACTION_IN_DB;
		ui32VolRegValue =0;
	}
	else
	{
		ui32VolRegValue = BRAP_DacVol1FFFF_DB[ui32AttenDb];
		ui32Delta = ui32VolRegValue - BRAP_DacVol1FFFF_DB[ui32AttenDb+1];
		ui32Delta = (ui32Delta * (ui32Volume%BRAP_DAC_P_FRACTION_IN_DB))/(BRAP_DAC_P_FRACTION_IN_DB);
		ui32VolRegValue -=ui32Delta;
	}

	BDBG_MSG (("BRAP_OP_SetDacVolume:" 
			"ui32Volume =%d dB, converted reg val =0x%x",
			ui32Volume , ui32VolRegValue));     

	ui32RegValue = 0;
	ui32RegValue |= (BCHP_FIELD_DATA (    
		HIFIDAC_CTRL0_DAC_VOLUME, 
		DAC_VOL, 
		ui32VolRegValue));  

	BRAP_Write32 (hRap->hRegister, 
		BCHP_HIFIDAC_CTRL0_DAC_VOLUME + hRap->hFmm[0]->hOp[eOpType]->ui32Offset,
		ui32RegValue);

	hRap->hFmm[0]->hOp[eOpType]->ui32DacVolume = ui32Volume;
 
	BDBG_LEAVE(BRAP_OP_SetDacVolume);

	return ret;
}

/***************************************************************************
Summary:
	Retrieves the current volume level at the HifiDAC


Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_SetDacVolume
	BRAP_MIXER_GetOutputVolume()	
****************************************************************************/
BERR_Code
BRAP_OP_GetDacVolume (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    uint32_t *puiVolume			/* [out] volume attenuation in 1/100 dB*/
)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_OP_GetDacVolume);
	BDBG_ASSERT (hRap);
	BDBG_ASSERT (puiVolume);
	
	BDBG_MSG(("BRAP_OP_GetDacVolume(): eOpType=%d", eOpType));

    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_GetDacVolume: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

	if ( hRap->hFmm[0]->hOp[eOpType] == NULL )
	{
		BDBG_ERR (("BRAP_OP_GetDacVolume(): Output port is not yet opened!!!"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}
 
	*puiVolume = hRap->hFmm[0]->hOp[eOpType]->ui32DacVolume;
 
	BDBG_LEAVE(BRAP_OP_GetDacVolume);
	return ret;
}

/***************************************************************************
Summary:
	Mutes/Unmutes the DAC output

Description:
	This API is required to mute/unmute the output of the specified DAC.
	PI maintains this state upon channel change.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_P_DacGetMute
	
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetMute (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool bMute						/* [in] True = Mute; false = unmute*/
)
{
	BERR_Code     eStatus;
	BDBG_ENTER (BRAP_OP_P_DacSetMute);

	BKNI_EnterCriticalSection();
	eStatus = BRAP_OP_P_DacSetMute_isr (hRap, eOpType, bMute);
	BKNI_LeaveCriticalSection();
	BDBG_LEAVE (BRAP_OP_P_DacSetMute);
	return eStatus;

}


BERR_Code
BRAP_OP_P_DacSetMute_isr (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool bMute						/* [in] True = Mute; false = unmute*/
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0;

	BDBG_ENTER(BRAP_OP_P_DacSetMute_isr);
	BDBG_ASSERT (hRap);
	
	BDBG_MSG(("BRAP_OP_P_DacSetMute_isr(): eOpType=%d bMute=%d", eOpType,
		bMute));
	
    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_P_DacSetMute_isr: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

	if ( hRap->hFmm[0]->hOp[eOpType] == NULL )
	{
		BDBG_ERR (("BRAP_OP_P_DacSetMute_isr(): Output port is not yet opened!!!"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	/* Mute or Unmute the DAC */
	if ( bMute == false )
	{
		/* Unmute the DAC port */
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+
				hRap->hFmm[0]->hOp[eOpType]->ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_unmute ));
		BRAP_Write32_isr (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hRap->hFmm[0]->hOp[eOpType]->ui32Offset,
			ui32RegValue);
	}
	else
	{
		/* Mute the DAC port */
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+
			hRap->hFmm[0]->hOp[eOpType]->ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_mute ));
		BRAP_Write32_isr (hRap->hRegister, 
		BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hRap->hFmm[0]->hOp[eOpType]->ui32Offset,
		ui32RegValue);
	}

	/* Update it in output handle */
	hRap->hFmm[0]->hOp[eOpType]->bDacMuteState = bMute;

	BDBG_LEAVE(BRAP_OP_P_DacSetMute_isr);
	return ret;
}

/***************************************************************************
Summary:
	Retrieves the mute status

Description:
	This API is required to get mute/unmute status of the
	output of the specified DAC.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_DacSetMute
	
****************************************************************************/
BERR_Code
BRAP_OP_P_DacGetMute (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool *pbMute					/* [out]True = Mute; false = unmute*/
)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_OP_P_DacGetMute);
	BDBG_ASSERT (hRap);
	BDBG_ASSERT (pbMute);
	
	BDBG_MSG(("BRAP_OP_P_DacGetMute(): eOpType=%d", eOpType));

    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_P_DacGetMute: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

	if ( hRap->hFmm[0]->hOp[eOpType] == NULL )
	{
		BDBG_ERR (("BRAP_OP_P_DacGetMute(): Output port is not yet opened!!!"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	*pbMute = hRap->hFmm[0]->hOp[eOpType]->bDacMuteState;

	BDBG_LEAVE(BRAP_OP_P_DacGetMute);
	return ret;
}
/***************************************************************************
Summary:
	Sets the sample rate for the given DAC output

Description:
	This API is required to set the sampling rate of DAC.
	This is mainly used for playback of PCM buffers from
	memory. In decode mode the F/W sets it according to
	the frequency.

Returns:
	BERR_SUCCESS 

See Also:
		
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetSampleRate(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BAVC_AudioSamplingRate eSamplingRate	/* [in]The sampling rate to be programmed */
)
{
	BERR_Code ret = BERR_SUCCESS;
	BDBG_ENTER(BRAP_OP_P_DacSetSampleRate);

    BKNI_EnterCriticalSection();
    ret = BRAP_OP_P_DacSetSampleRate_isr(hRap, eOpType, eSamplingRate);
    BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_OP_P_DacSetSampleRate);
    return ret;
}

/***************************************************************************
Summary:
	Sets the sample rate for the given DAC output

Description:
    ISR version of BRAP_OP_P_DacSetSampleRate().
    
Returns:
	BERR_SUCCESS 

See Also:
		
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetSampleRate_isr(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BAVC_AudioSamplingRate eSamplingRate	/* [in]The sampling rate to be programmed */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0,i=0,j=0;
	uint32_t	ui32Offset = 0;
    BRAP_OP_P_DacRateMangerSR    sDacRateMangerSR = {BAVC_AudioSamplingRate_e48k, 0, 0, 0, 0};

	BDBG_ENTER(BRAP_OP_P_DacSetSampleRate_isr);
	BDBG_ASSERT (hRap);
	
	BDBG_MSG(("BRAP_OP_P_DacSetSampleRate_isr(): eOpType=%d eSamplingRate=%d",
		eOpType, eSamplingRate));


    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_P_DacSetSampleRate_isr: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

    ret = BRAP_OP_P_GetDacOffset(eOpType, &ui32Offset);
    if( ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_DacSetSampleRate_isr: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
    }

	/* Program the FIR field in HIFIDAC_CTRL0_CONFIG register */
	if ( eSamplingRate > BAVC_AudioSamplingRate_e96k )
	{
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG+ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG,FIR_MODE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
			FIR_MODE, Audio_over100kHz));
		BRAP_Write32_isr (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_CONFIG + ui32Offset,
			ui32RegValue);
	}
	else
	{
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG+
			ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG,FIR_MODE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
			FIR_MODE, Audio_under100kHz));
		BRAP_Write32_isr (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_CONFIG + ui32Offset,
			ui32RegValue);
	}

    /* programming FIR mode to over 100K for BTSC encoder case*/
    for(i=0;i<BRAP_MAX_PP_SUPPORTED;i++)
    {
        if((hRap->hAudioProcessingStageHandle[i]!=NULL)&&
            (hRap->hAudioProcessingStageHandle[i]->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eBtsc))
        {
            for(j=0;j<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;j++)
            {                
                if((hRap->hAudioProcessingStageHandle[i]->hDestHandle[j]!=NULL)&&
                (hRap->hAudioProcessingStageHandle[i]->hDestHandle[j]->sExtDstDetails. \
                uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLR] == BRAP_OutputPort_eDac0))
                {
            		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG+ui32Offset);
            		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG,FIR_MODE));
            		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
            			FIR_MODE, Audio_over100kHz));
            		BRAP_Write32_isr (hRap->hRegister, 
            			BCHP_HIFIDAC_CTRL0_CONFIG + ui32Offset,
            			ui32RegValue);                
                }
            }
        }
    }

	/* Program the Oversampling frequency */
	if ( eSamplingRate <= BAVC_AudioSamplingRate_e48k )
	{
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_RATEMGRCFG+
			ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_RATEMGRCFG,OVERSAMPLE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_RATEMGRCFG, 
			OVERSAMPLE, Fs256));
		BRAP_Write32_isr (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_RATEMGRCFG + ui32Offset,
			ui32RegValue);
	}
	else
	{
		ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_HIFIDAC_CTRL0_RATEMGRCFG+
			ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_RATEMGRCFG,OVERSAMPLE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_RATEMGRCFG, 
			OVERSAMPLE, Fs64));
		BRAP_Write32_isr (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_RATEMGRCFG + ui32Offset,
			ui32RegValue);
	}

    /* Program the Rate Manager with the parameters related to sampling rate */

    /* Get the Rate Manager Values */
    ret = BRAP_OP_P_GetDacRateMgrSR(eSamplingRate, &sDacRateMangerSR);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_P_DacSetSampleRate_isr: BRAP_OP_P_GetDacRateMgrSR returned Error %d",ret));
        return BERR_TRACE(ret);
	}

    /* Program the denominator */
    

	ui32RegValue = BRAP_Read32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_RATE_RATIO +
			ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_RM0_RATE_RATIO, DENOMINATOR));
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_RM0_RATE_RATIO, DENOMINATOR, 
									  sDacRateMangerSR.uiDenominator));
	BRAP_Write32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_RATE_RATIO+
			ui32Offset, ui32RegValue);

	/* Program the numerator and the interger part (Sample Inc) of the ratio */
	ui32RegValue = BRAP_Read32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_SAMPLE_INC +
			ui32Offset);
	ui32RegValue &= ~((BCHP_MASK(HIFIDAC_RM0_SAMPLE_INC, NUMERATOR)) |
					  (BCHP_MASK(HIFIDAC_RM0_SAMPLE_INC, SAMPLE_INC)));
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_RM0_SAMPLE_INC, NUMERATOR, 
									  sDacRateMangerSR.uiNumerator)) |
					(BCHP_FIELD_DATA (HIFIDAC_RM0_SAMPLE_INC, SAMPLE_INC, 
									  sDacRateMangerSR.uiSampleInc));
	BRAP_Write32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_SAMPLE_INC +
			ui32Offset, ui32RegValue);

	/* Program the Phase Inc */
	ui32RegValue = BRAP_Read32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_PHASE_INC +
			ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(HIFIDAC_RM0_PHASE_INC, PHASE_INC));
	ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_RM0_PHASE_INC, PHASE_INC, 
									  sDacRateMangerSR.uiPhaseInc));
	BRAP_Write32_isr(hRap->hRegister, BCHP_HIFIDAC_RM0_PHASE_INC +
			ui32Offset, ui32RegValue);
	BDBG_LEAVE(BRAP_OP_P_DacSetSampleRate_isr);
	return ret;
}

/***************************************************************************
Summary:
	Enable/Disable the Test tone

Description:
	This API plays the samples stored in the test tone buffer through
	the specified DAC. This can be used even before a decode channel is 
	opened and just after opening the raptor device. The samples for the
	test tone generaion is given by the application or if application has asked
	to use the default test-tone then PI uses that for test tone generation.
	If bEnable = False, it stops the enabled testtone.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code
BRAP_OP_DacEnableTestTone(
    BRAP_Handle     hRap,				/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,		/* [in] Output Type */
    bool	bEnable,					/* [in] True = Enable; False = Disable */
    const BRAP_OP_DacToneInfo *pToneInfo	/* [in] Test tone control structure */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t	ui32RegValue = 0;
	uint32_t	i32Count = 0;
	uint32_t	ui32Offset = 0;
	uint32_t ui32Temp=0, ui32AttenDb=0, ui32Delta=0;
	
	BDBG_ENTER(BRAP_OP_DacEnableTestTone);
	BDBG_ASSERT (hRap);
	BDBG_ASSERT (pToneInfo);
	
	BDBG_MSG(("BRAP_OP_DacEnableTestTone(): eOpType=%d bEnable=%d",
		eOpType, bEnable));
	BDBG_MSG(("TestToneInfo Settings:"));
	BDBG_MSG(("pToneInfo->bSampleBufferMode=%d",pToneInfo->bSampleBufferMode));
	BDBG_MSG(("pToneInfo->eLROutputControl=%d",pToneInfo->eLROutputControl));
	BDBG_MSG(("pToneInfo->uiLeftRepeat=%d",pToneInfo->uiLeftRepeat));
	BDBG_MSG(("pToneInfo->uiRightRepeat=%d",pToneInfo->uiRightRepeat));

    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_DacEnableTestTone: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

    ret = BRAP_OP_P_GetDacOffset(eOpType, &ui32Offset);
    if( ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_DacEnableTestTone: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
    }

	if ( bEnable == true )
	{
		/* Enable the sampling rate */
		BRAP_OP_P_DacSetSampleRate( hRap, eOpType,pToneInfo->eSamplingRate );
		
		/* Program REPEAT_RIGHT and REPEAT_LEFT fields */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_REPEAT_RIGHT));
		ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_TEST, 
                                            TESTTONE_REPEAT_RIGHT, pToneInfo->uiRightRepeat));
		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
			ui32RegValue);		

		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_REPEAT_LEFT));
		ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_TEST, 
                                            TESTTONE_REPEAT_LEFT, pToneInfo->uiLeftRepeat));
		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
			ui32RegValue);

		/* Program TESTTONE_SHARE field */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_SHARE));
		ui32RegValue |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_TEST, 
                                            TESTTONE_SHARE, pToneInfo->bSampleBufferMode));
		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
			ui32RegValue);

		switch ( pToneInfo->eLROutputControl )
		{
			case BRAP_OP_DacLROutputCtrl_eBothLRZeros:
				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_RIGHT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_RIGHT, Output_zero));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);		

				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_LEFT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_LEFT, Output_zero));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);
				break;
			case BRAP_OP_DacLROutputCtrl_eOnlyLZeros:
				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_RIGHT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_RIGHT, Output_testtone));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);		

				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_LEFT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_LEFT, Output_zero));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);
				break;
			case BRAP_OP_DacLROutputCtrl_eOnlyRZeros:
				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_RIGHT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_RIGHT, Output_zero));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);		

				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_LEFT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_LEFT, Output_testtone));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);
				break;
			case BRAP_OP_DacLROutputCtrl_eNoZeros:
				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_RIGHT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_RIGHT, Output_testtone));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);		

				ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
				ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_LEFT));
				ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_LEFT, Output_testtone));
				BRAP_Write32 (hRap->hRegister, 
						BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
						ui32RegValue);
				break;
			default:
				BDBG_ERR (("BRAP_OP_DacEnableTestTone(): Unsupported Output control %d", 
	                     		pToneInfo->eLROutputControl));
	            		return BERR_TRACE (BERR_INVALID_PARAMETER);
		}
		
	 	/* Program HIFIDAC_CTRL0_TEST: RAM_BUF_SEL to  samples */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST+ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,RAM_BUF_SEL));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
                                            RAM_BUF_SEL, Samples ));
		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
			ui32RegValue);

		/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  enable */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST + ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_ENABLE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_ENABLE, Enable ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
				ui32RegValue);

		/*program  MAPPER_SOFTMUTE to normal operation
		* By default it is in mute state 
		*/
	    	/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  Normal_operation */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_CONFIG + ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG,MAPPER_SOFTMUTE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
	                                            MAPPER_SOFTMUTE, Normal_operation ));
		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_CONFIG +ui32Offset,
			ui32RegValue);	

	        /* Set default volume */
	        ui32AttenDb = BRAP_DAC_P_DEFAULT_VOLUME/BRAP_DAC_P_FRACTION_IN_DB;
	        
		ui32Temp = BRAP_DacVol1FFFF_DB[ui32AttenDb];   
		ui32Delta = ui32Temp - BRAP_DacVol1FFFF_DB[ui32AttenDb+1];
		ui32Delta = (ui32Delta * (BRAP_DAC_P_DEFAULT_VOLUME%BRAP_DAC_P_FRACTION_IN_DB))
				/(BRAP_DAC_P_FRACTION_IN_DB);
		ui32Temp -= ui32Delta;        
	    
		BDBG_MSG (("BRAP_OP_P_DacOpen:" 
				"BRAP_OP_DacEnableTestTone=%d dB, converted reg val =0x%x",
				BRAP_DAC_P_DEFAULT_VOLUME, ui32Temp));     

		ui32RegValue = 0;
		ui32RegValue |= (BCHP_FIELD_DATA (    
			HIFIDAC_CTRL0_DAC_VOLUME, 
			DAC_VOL, 
			ui32Temp));  

		BRAP_Write32 (hRap->hRegister, 
			BCHP_HIFIDAC_CTRL0_DAC_VOLUME + ui32Offset,
			ui32RegValue);

		/* Initialize the test samples */
		for ( i32Count = 0; i32Count <= BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_END; i32Count++ )
		{
			BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + ui32Offset + ( i32Count*4),
				pToneInfo->aSample[i32Count]);
			BDBG_MSG(("pToneInfo->aSample[%d]=%d",i32Count,pToneInfo->aSample[i32Count]));
		}

		/* Unmute the DAC port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_unmute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + ui32Offset,
				ui32RegValue);

		/* unmute the DACALL Control port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL,MUTEALL));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL, 
                                            MUTEALL, Ramp_unmute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL + ui32Offset,
				ui32RegValue);
	}
	else
	{
		/* mute the DAC port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_mute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + ui32Offset,
				ui32RegValue);

		/* mute the DACALL Control port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL,MUTEALL));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL, 
                                            MUTEALL, Ramp_mute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL + ui32Offset,
				ui32RegValue);
		
		/* Program HIFIDAC_CTRL0_TEST: TESTTONE_ENABLE to  enable */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_TEST + ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST,TESTTONE_ENABLE));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_TEST, 
						TESTTONE_ENABLE, Normal_operation));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_TEST + ui32Offset,
				ui32RegValue);

		/* Unmute the DAC port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_unmute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + ui32Offset,
				ui32RegValue);

		/* Unmute the DACALL Control port */
		ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_HIFIDAC_CTRL0_MUTECTRL+ ui32Offset);
		ui32RegValue &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL,MUTEALL));
		ui32RegValue |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL, 
                                            MUTEALL, Ramp_unmute ));
		BRAP_Write32 (hRap->hRegister, 
				BCHP_HIFIDAC_CTRL0_MUTECTRL + ui32Offset,
				ui32RegValue);        
	}
	
	BDBG_LEAVE(BRAP_OP_DacEnableTestTone);
	return ret;
}

BERR_Code
BRAP_OP_DacGetDefaultTestTone(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BRAP_OP_DacToneInfo *pToneInfo		/* [out] Test tone control structure */
)
{
	BERR_Code ret = BERR_SUCCESS;
	
	BDBG_ENTER(BRAP_OP_DacGetDefaultTestTone);
	BDBG_ASSERT (hRap);
	BDBG_ASSERT (pToneInfo);

	BSTD_UNUSED(hRap);

    ret = BRAP_RM_P_IsOutputPortSupported(eOpType);
    if( ret != BERR_SUCCESS)
	{
        BDBG_ERR(("BRAP_OP_DacGetDefaultTestTone: invalid output port %d", eOpType));
        return BERR_TRACE(ret);
	}

	*pToneInfo = defDacTestToneInfo;

	BDBG_LEAVE(BRAP_OP_DacGetDefaultTestTone);
	return ret;
}




