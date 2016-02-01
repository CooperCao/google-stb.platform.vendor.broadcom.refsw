/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
*   Module name: SPDIFFM
*   This file contains the implementation of all PIs for the SPDIF
*   Formatter abstraction. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_spdiffm);

#define BRAP_SPDIFFM_PLAYBACK_MS0_CBIT_BUFFER 0
#define BRAP_SPDIFFM_PLAYBACK_MS1_CBIT_BUFFER 12


static BERR_Code 
BRAP_SPDIFFM_P_HWConfig ( 
	BRAP_ChannelHandle 		hRapCh,		/* [in] Rap channel handle */	
    BRAP_SPDIFFM_P_Handle   hSpdifFm    /* [in] SPDIF Formatter handle */
);


static BERR_Code 
BRAP_SPDIFFM_P_PrepareCBITData_isr(
	uint32_t *pui32CBitData, 			/* [out] CBit buffer */
	BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams,
										/* SPDIF Channel status params */
	BAVC_AudioSamplingRate eOutputSamplingRate,
										/* Output sampling rate */
    bool                   bCompressed  /* TRUE: input data is compressed */
);

/* Default Settings and Parameters */
static const BRAP_SPDIFFM_P_Params defSpdifFmParams =
{
    {
        true    /* sExtParams.bSpdifFormat */
    },
    true,       /* bCompressed */
    false,      /* bSeparateLRChanNum */
    false,		/* bUseSpdifPackedChanStatusBits */
	{	/* sSpdifPackedChanStatusBits */
		{
			0x0,
			0x0
        	}
	},
    BRAP_SPDIFFM_CP_TOGGLE_RATE_DISABLED, /* uiCpToggleRate */
	{			
            /* sChanStatusParams */
            false,  /* bProfessionalMode = Consumer Mode */
            false,  /* bSwCopyRight = Not asserted */
            0,      /* ui16CategoryCode = 0 */
            0,      /* bSeparateLRChanNum = 0 =>Left=right channel num=0000*/
            0,        /* ui16ClockAccuracy = 0 (Level II) */
            0,                   /*uiCGMS_A*/
            16                  /*ui16SpdifPcmWordWidth*/
    },
	BRAP_SPDIFFM_P_BurstRepPeriod_eNone, /* eBurstRepPeriod */
	BAVC_AudioSamplingRate_e48k,
	false    /*bUseHwCBit */
};

static const BRAP_SPDIFFM_P_Settings defSpdifFmSettings =
{
    {
        true,                           /* sExtSettings.bEnableDither */
        BRAP_SPDIFFM_BurstType_ePause   /* sExtSettings.eBurstType */
    }
};

BERR_Code 
BRAP_SPDIFFM_P_GetDefaultParams ( 
    BRAP_SPDIFFM_P_Params    *pDefParams   /* Pointer to memory where default
                                              settings should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_SPDIFFM_P_GetDefaultParams);

    BDBG_ASSERT (pDefParams);            
    
    *pDefParams = defSpdifFmParams;
    BDBG_LEAVE (BRAP_SPDIFFM_P_GetDefaultParams);
    return ret;
}

BERR_Code 
BRAP_SPDIFFM_P_GetDefaultSettings ( 
    BRAP_SPDIFFM_P_Settings *pDefSettings   /* Pointer to memory where default
                                              settings should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_SPDIFFM_P_GetDefaultSettings);

    BDBG_ASSERT (pDefSettings);            
    
    *pDefSettings = defSpdifFmSettings;
    BDBG_LEAVE (BRAP_SPDIFFM_P_GetDefaultSettings);
    return ret;
}

BERR_Code 
BRAP_SPDIFFM_P_GetCurrentParams (
	BRAP_SPDIFFM_P_Handle  hSpdifFm, /* [in] SPDIF Formatter handle */
    BRAP_SPDIFFM_P_Params    *pCurParams   /* Pointer to memory where current 
                                              parameters should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER ( BRAP_SPDIFFM_P_GetCurrentParams );
	BDBG_ASSERT( hSpdifFm );
	BDBG_ASSERT( pCurParams );

	*pCurParams = hSpdifFm->sParams;

    BDBG_LEAVE ( BRAP_SPDIFFM_P_GetCurrentParams );
    return ret;
}

/***************************************************************************
Summary:
    Changes Burst repetition period for MPEG-2 as per IEC-61937-1. 
Description:
    If sampling rate of an MPEG-2 stream is < 32kHz then burst repetition 
    rate is set to PER_64 else it is set to PER_32. Applied only if the 
    data out is compressed from SPDIFFM. 
Returns:
    BERR_SUCCESS on success
See Also:

**************************************************************************/
BERR_Code BRAP_SPDIFFM_P_ChangeBurstRepPeriodForMPEG2_isr(
    BRAP_SPDIFFM_P_Handle   hSpdifFm   /* [in] SPDIF Formatter handle */
    )
{
    uint32_t ui32RegValue = 0;
    BERR_Code ret = BERR_SUCCESS;
    unsigned int uiSR = 0;

    BDBG_ENTER(BRAP_SPDIFFM_P_ChangeBurstRepPeriodForMPEG2_isr);
    BDBG_ASSERT(hSpdifFm);
    
    /* Change Burst repetition period only for a compressed stream */
  if( (hSpdifFm->sParams.bCompressed) && 
	(hSpdifFm->sSettings.sExtSettings.eBurstType != BRAP_SPDIFFM_BurstType_eNone) )
    {
        ui32RegValue = BRAP_Read32_isr (hSpdifFm->hRegister, 
                    BCHP_AUD_FMM_MS_CTRL_FW_BURST_0 + hSpdifFm->ui32Offset);
        
        ui32RegValue &= ~( BCHP_MASK (AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD));    

        /* Convert eSR to uiSR */
        ret = BRAP_P_ConvertSR(hSpdifFm->sParams.eSamplingRate, &uiSR);
        if(ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            return ret;
        }
        
        if(uiSR < 32000)
        {
            BDBG_MSG(("ChangeBurstRepPeriodForMPEG2_isr: burst rep_period = 64"));
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                                          REP_PERIOD, PER_64));
        }
        else
        {
            BDBG_MSG(("ChangeBurstRepPeriodForMPEG2_isr: burst rep_period = 32"));
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0,   
                                                          REP_PERIOD, PER_32));
        }

        BRAP_Write32_isr (hSpdifFm->hRegister, 
                      hSpdifFm->ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_BURST_0,
                      ui32RegValue); 
    }

    BDBG_LEAVE(BRAP_SPDIFFM_P_ChangeBurstRepPeriodForMPEG2_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Programs dither and burst for SPDIFFM.
Description:
    This internal routine does the register programming for dither and burst
    for SPDIFFM.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	
**************************************************************************/
BERR_Code
BRAP_SPDIFFM_P_ProgramDitherAndBurst(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    BRAP_SPDIFFM_P_Settings *pSpdiffmSettings,/* [in] SPDIFFM settings */
    BRAP_SPDIFFM_P_BurstRepPeriod eBurstRepPeriod 
                                              /* [in] Burst repetition period */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32Offset = 0;
    uint32_t ui32RegValue = 0;
    
    BDBG_ENTER(BRAP_SPDIFFM_P_ProgramDitherAndBurst);

    /* Validate input params */
    BDBG_ASSERT(pSpdiffmSettings);
    BDBG_ASSERT(hSpdifFm);

    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: hSpdifFm->sSettings:"));
    BDBG_MSG(("\t bEnableDither=%d", hSpdifFm->sSettings.sExtSettings.bEnableDither));    
    BDBG_MSG(("\t eBurstType=%d", hSpdifFm->sSettings.sExtSettings.eBurstType));        

    ui32Offset = hSpdifFm->ui32Offset;

    /* Program the DITHER_ENA */
    ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + ui32Offset);
    ui32RegValue &= ~( BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA) );    
    if(hSpdifFm->sParams.bCompressed == false)
    {
        BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: bComp == false bEnaDither = %d", 
                pSpdiffmSettings->sExtSettings.bEnableDither));
        /* For uncompressed, do as per the user settings */    
        if(pSpdiffmSettings->sExtSettings.bEnableDither)
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Enable));
        else
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Disable));
    }
    else
    {
        BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: bComp = true disabling dither"));
        /* For compressed, always disable dither */
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Disable));
    }
    
    BRAP_Write32 (hSpdifFm->hRegister, 
                  ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0,
                  ui32RegValue); 
        
    /* Program the BURST details */
    ui32RegValue = 0;
    ui32RegValue &= ~( BCHP_MASK (AUD_FMM_MS_CTRL_FW_BURST_0, TYPE) |
                       BCHP_MASK (AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD) |
                       BCHP_MASK (AUD_FMM_MS_CTRL_FW_BURST_0, STOP));    

    if(hSpdifFm->sParams.bCompressed == false)
    {
        BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: bComp = false, disabling burst "));
        /* For uncompressed, always disable the burst */
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, None));
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          STOP, Disable));
    }
    else
    {
        /* For compressed, do as per the user settings */
        switch(pSpdiffmSettings->sExtSettings.eBurstType)
        {
            case BRAP_SPDIFFM_BurstType_eNone:
                BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst None"));
                /* Insert nither pause nor null burst */
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, None));
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          STOP, Disable));
                break;
                
            case BRAP_SPDIFFM_BurstType_eNull:
                BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst Null"));
                /* Type = Null burst */
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          TYPE, Null));
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          STOP, Enable));
                break;
                
            case BRAP_SPDIFFM_BurstType_ePause:
                BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst Pause"));
                /* Type = Pause burst */
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          TYPE, Pause));
                ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          STOP, Enable));
                break;

            default:
                BDBG_ERR(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: Illegal eBurstType(%d)",
                    pSpdiffmSettings->sExtSettings.eBurstType));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }/* switch eBurstType */

        /* Only for Null or Pause burst type, program the repetition period */
        if(pSpdiffmSettings->sExtSettings.eBurstType == BRAP_SPDIFFM_BurstType_eNull ||
           pSpdiffmSettings->sExtSettings.eBurstType == BRAP_SPDIFFM_BurstType_ePause )
        {
            switch (eBurstRepPeriod)
            {
                case BRAP_SPDIFFM_P_BurstRepPeriod_eNone:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period : none"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, None));
                    break;            
                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer3:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 3"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, PER_3));
                    break;
                    
                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer4:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 4"));
                    ui32RegValue |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, 2));
                    break;                                            
                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer32:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 32"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, PER_32));
                    break;

                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer64:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 64"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, PER_64));
                    break;

                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer1024:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 1024"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, PER_1024));
                    break;

                case BRAP_SPDIFFM_P_BurstRepPeriod_ePer4096:
                    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: burst rep period 4096"));
                    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, 
                                          REP_PERIOD, PER_4096));
                    break;

                default:
                    BDBG_ERR(("BRAP_SPDIFFM_P_ProgramDitherAndBurst: Invalid eBurstRepPeriod(%d)",
                        eBurstRepPeriod));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }/* switch eBurstRepPeriod */
        }
    }
        
    BRAP_Write32 (hSpdifFm->hRegister, 
                  ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_BURST_0,
                  ui32RegValue); 

    BDBG_LEAVE(BRAP_SPDIFFM_P_ProgramDitherAndBurst);
    return ret;
}


/***************************************************************************
Summary:
    This PI sets/clears the OVERWRITE_DATA flag which determines 
    whether or not the data from the mixer will be overwritten.
    When this bit is set, by a Dither/Null 
    Burst/P
    Must be called only after the output port has been configured and opened.
    
Description:

For compressed data:
REP_PERIOD   OVERWRITE_DATA
    0         0     Pause/null/zero never inserted. Samples sent out as is.
    0         1     Overwrite sample from mixer with zeros
  non-0       0     Insert pause/null on underflow.
  non-0       1     Overwrite samples from mixer with null/pause burst.

For PCM data:
DITHER_ENA  OVERWRITE_DATA
    0         0     Nothing inserted.
    0         1     Overwrite sample from mixer with zeros
    1         0     Insert dither on underflow.
    1         1     Overwrite samples from mixer with dither signal

Returns:
    BERR_SUCCESS on success
    Error code on failure
    
See Also:
	

**************************************************************************/
BERR_Code
BRAP_SPDIFFM_P_InsertBurstOnMute (
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    bool                    bOverwrite  /* TRUE: overwrite data from mixer
                                           FALSE: Don't overwrite data from 
                                           mixer */
)               
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegVal = 0;
    
    BDBG_ENTER(BRAP_SPDIFFM_P_InsertBurstOnMute);

    BDBG_ASSERT(hSpdifFm);
    
    BDBG_MSG(("BRAP_SPDIFFM_P_InsertBurstOnMute: eBurstType=%d,hSpdifFm->sParams.bUseHwCBit =%d", hSpdifFm->sSettings.sExtSettings.eBurstType
        ,hSpdifFm->sParams.bUseHwCBit ));  

    ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset);
    
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA));
#if (BRAP_7405_FAMILY == 1)     
    if(hSpdifFm->sParams.bUseHwCBit ==true)
    {
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA));        
    }    
#endif
    if (bOverwrite == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA, 1));            
#if (BRAP_7405_FAMILY == 1)        
        if(hSpdifFm->sParams.bUseHwCBit ==true)
        {
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, 1));
        }
#endif        
        BRAP_Write32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset, 
                      ui32RegVal);     
#if (BRAP_7405_FAMILY == 1)                
        if(hSpdifFm->sParams.bUseHwCBit ==true)
        {        
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);       
            if(hSpdifFm->uiStreamIndex == 0)
            {
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0));
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, 0));                    
            }
            else  if(hSpdifFm->uiStreamIndex == 1)
            {
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1));
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, 0));                                    
            }

            BRAP_Write32 (hSpdifFm->hRegister, 
                          BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS , 
                          ui32RegVal);          
        }            
#endif        
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA, 0));            
#if (BRAP_7405_FAMILY == 1)                
        if(hSpdifFm->sParams.bUseHwCBit ==true)
        {
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, 0));       
        }
#endif        
        BRAP_Write32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset, 
                      ui32RegVal);    
#if (BRAP_7405_FAMILY == 1)                
        if(hSpdifFm->sParams.bUseHwCBit ==true)
        {            
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);       
            if(hSpdifFm->uiStreamIndex == 0)
            {
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0));
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, 1));                    
            }
            else  if(hSpdifFm->uiStreamIndex == 1)
            {
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1));
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, 1));                                    
            }

            BRAP_Write32 (hSpdifFm->hRegister, 
                          BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS , 
                          ui32RegVal);          
        }
#endif        
        
    }        

    BDBG_LEAVE (BRAP_SPDIFFM_P_InsertBurstOnMute);
    return ret;       
}

BERR_Code 
BRAP_SPDIFFM_P_Open (
    BRAP_FMM_P_Handle         hFmm,             /* [in] Parent FMM handle */
    BRAP_SPDIFFM_P_Handle *   phSpdifFm,        /* [out] SPDIFFM stream handle */ 
    unsigned int              uiStreamIndex,    /* [in] SPDIFFM Stream index */
    const BRAP_SPDIFFM_P_Settings * pSettings   /* [in] The SPDIFFM stream settings*/                                         
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_SPDIFFM_P_Handle hSpdifFm;
    uint32_t ui32RegValue=0;

    BDBG_ENTER (BRAP_SPDIFFM_P_Open);

    /* 1. Check all input parameters to the function. Return error if
      - FMM handle is NULL
      - Given index exceeds maximum no. of SPDIF Formatters
      - Pointer to Settings structure is NULL
      - Pointer to SPDIF Formatter handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phSpdifFm);
    if (uiStreamIndex > (BRAP_RM_P_MAX_SPDIFFM_STREAMS - 1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }    
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  

        BDBG_ASSERT (pSettings);
        if (hFmm->hSpdifFm[uiStreamIndex] != NULL)
        {

            /* This SPDIFFM stream was previoulsy opened by another audio channel.
            So this time, just increment the open count. Dont change the 
            settings etc. */
            /* OpenCount cannot exceed the no. of Mixer inputs!! */
            hSpdifFm = hFmm->hSpdifFm[uiStreamIndex];

            /* In 7440, an SPDIFFM can be opened more than the number of mixer 
               inputs */
            if (hSpdifFm->uiOpenCnt >= BRAP_RM_P_MAX_MIXER_INPUTS)
            {
                BDBG_ERR (("BRAP_SPDIFFM_P_Open: SpdifFm stream %d already has %d open"
                        "audio channels. Cannot add another!!!",
                        uiStreamIndex, hSpdifFm->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }

            *phSpdifFm = hSpdifFm;
            hSpdifFm->uiOpenCnt ++;
        
            
            BDBG_MSG (("BRAP_SPDIFFM_P_Open: SpdifFm stream %d was already open." 
                    "New open count = %d", 
                        uiStreamIndex, hSpdifFm->uiOpenCnt));
            BDBG_LEAVE (BRAP_SPDIFFM_P_Open);
            return ret;
        }
      
        /* 2. Allocate memory for the SPDIF Formatter handle. Fill in parameters 
        in the SPDIF Formatter handle. */
    
        /* Allocate Mixer handle */
        hSpdifFm = (BRAP_SPDIFFM_P_Handle) BKNI_Malloc (sizeof(BRAP_SPDIFFM_P_Object));
        if (hSpdifFm == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset(hSpdifFm, 0, sizeof(BRAP_SPDIFFM_P_Object));

        /* Initialise known elements in Source Channel handle */
        hSpdifFm->hChip = hFmm->hChip;
        hSpdifFm->hRegister = hFmm->hRegister;
        hSpdifFm->hHeap = hFmm->hHeap;
        hSpdifFm->hInt = hFmm->hInt;
        hSpdifFm->hFmm = hFmm;
        hSpdifFm->uiStreamIndex = uiStreamIndex;
        hSpdifFm->uiOpenCnt = 1;
        hSpdifFm->uiStartCnt = 0;
        
        BRAP_RM_P_GetSpdifFmId (uiStreamIndex, &(hSpdifFm->uiIndex));

        /* SPDIF Formatter register offset */
        hSpdifFm->ui32Offset = (BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1 - 
                          BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0)
                          * hSpdifFm->uiStreamIndex;
      
        /* Fill up the Settings Structure */
        hSpdifFm->sSettings = *pSettings;


    }   /* End: If not in WatchDog recovery */
    else
    {
        hSpdifFm = *phSpdifFm; 
    }

    /* dither */
    ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset);
    ui32RegValue &= ~( (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA))
                       |(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA))
#if (BRAP_7405_FAMILY == 1)    
                       |(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM))
#endif
                        );

    if(hSpdifFm->sSettings.sExtSettings.bEnableDither)
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Enable));
    else
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Disable));

    /* Insert dither signal even when STREAM_ENA is off */
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      INSERT_WHEN_DISA, Insert));
#if (BRAP_7405_FAMILY == 1)    
        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      SPDIF_OR_PCM, SPDIF));
#endif
    
    BRAP_Write32 (hSpdifFm->hRegister,
                  hSpdifFm->ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0,
                  ui32RegValue); 

    /* Disable BURST - insert neither pause nor null burst */
    ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0 + hSpdifFm->ui32Offset);
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD));     
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD, None));
    BRAP_Write32 (hSpdifFm->hRegister, 
                  hSpdifFm->ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_BURST_0,
                  ui32RegValue);     

    /* PR 18296: if dither is enabled, we want the SPDIFFM to be alive always... and not disable it at stert/stop.
        So, we just enable it at opne time itself */

    /* Enable the SPDIF Formatter output */
    ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);
	

    /* Program STREAMX_ENA field to 'enable' depending upon which 
       SPDIF Formatter Input is used */
    switch (hSpdifFm->uiStreamIndex)
    {
        case 0:
            ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA));
            ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                             STREAM0_ENA, Enable);
            break;
        case 1:
            ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA));
            ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                             STREAM1_ENA, Enable);
            break;
        default:
            BDBG_ERR (("BRAP_SPDIFFM_P_Open(): Incorrect SPDIFFM stream index %d", 
                        hSpdifFm->uiStreamIndex));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    BRAP_Write32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA,
                  ui32RegValue);

    BDBG_MSG(("hSpdifFm->sSettings:"));
    BDBG_MSG(("\t bEnableDither=%d", hSpdifFm->sSettings.sExtSettings.bEnableDither));    
    BDBG_MSG(("\t eBurstType=%d", hSpdifFm->sSettings.sExtSettings.eBurstType));        
    
    /* Configure SPDIF Formatter Hardware if reqd*/
        
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  

        /* Store the SPDIF Formatter handle inside the FMM handle */
        hFmm->hSpdifFm[uiStreamIndex] = hSpdifFm;
 
        /* Return the filled handle */
        *phSpdifFm = hSpdifFm;
    }   /* End: If not in WatchDog recovery */
    
    BDBG_LEAVE(BRAP_SPDIFFM_P_Open);
    return ret;

}




BERR_Code 
BRAP_SPDIFFM_P_Close ( 
    BRAP_SPDIFFM_P_Handle  hSpdifFm        /* [in] SPDIF Formatter Stream handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegValue = 0;
    
    BDBG_ENTER (BRAP_SPDIFFM_P_Close);
    BDBG_ASSERT (hSpdifFm);
    if (hSpdifFm->uiOpenCnt == 0)
    { 
        /* This should never happen. If it does, it means the system
           has gone into an invalid state!!!!*/
            BDBG_ERR (("BRAP_SPDIFFM_P_Close: SPDIFFM Stream %d has NO open"
                        "audio channels!!!", hSpdifFm->uiStreamIndex));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    hSpdifFm->uiOpenCnt --;

    if (hSpdifFm->uiOpenCnt == 0)
    {   
        /* If this was the last open audio channel,
           free the handle etc */

        /* PR 18296: if dither is enabled, we want the SPDIFFM to be alive always... and not disable it at start/stop.
        So, we just enable it at opne time itself and disable at close */
        
        ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, 
                BCHP_AUD_FMM_MS_CTRL_STRM_ENA);

        /* Program STREAMX_ENA field to 'disable' depending upon which 
           SPDIF Formatter is used */
#if (BRAP_7405_FAMILY != 1)
          
        switch (hSpdifFm->uiStreamIndex)
        {
            case 0:
                ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA));
                ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                                 STREAM0_ENA, Disable);
                break;
            case 1:
                ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA));
                ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                                 STREAM1_ENA, Disable);
                break;
            default:
                BDBG_ERR (("BRAP_SPDIFFM_P_Stop(): Invalid input index 0x%08x ", 
                            hSpdifFm->uiStreamIndex));    
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        }

        BRAP_Write32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_STRM_ENA,
                      ui32RegValue);

#endif
        /* Remove referrence to this SPDIF Formatter from the parent FMM */ 
        hSpdifFm->hFmm->hSpdifFm[hSpdifFm->uiStreamIndex] = NULL;

	BDBG_MSG (("BRAP_SPDIFFM_P_Close: SPDIFFM Stream %d closed. hSpdifFm->uiOpenCnt=%d",
		hSpdifFm->uiStreamIndex, hSpdifFm->uiOpenCnt));

        /* Free the SPDIF Formatter Handle memory*/
        BKNI_Free (hSpdifFm); 
    }      
    BDBG_LEAVE (BRAP_SPDIFFM_P_Close);
    return ret;
}


BERR_Code 
BRAP_SPDIFFM_P_Start ( 
	BRAP_ChannelHandle 		hRapCh,		  /* [in] Rap channel handle */	
    BRAP_SPDIFFM_P_Handle   hSpdifFm,     /* [in] SPDIF Formatter handle */
    const BRAP_SPDIFFM_P_Params * pParams /* [in] Pointer to start
                                                  parameters */ 
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_SPDIFFM_P_Start);
    BDBG_ASSERT (hSpdifFm);
    
    BDBG_MSG (("BRAP_SPDIFFM_P_Start(): hSpdifFm=0x%x SPDIFFM Stream index=%d ", hSpdifFm, hSpdifFm->uiStreamIndex)); 

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hSpdifFm->hFmm) == false)
    {   /* If not in WatchDog recovery */  
        BDBG_ASSERT (pParams);

        if (hSpdifFm->uiStartCnt > 0)
        {
            /* This hSpdifFm was previoulsy started by another audio channel.
            So this time, just increment the start count. Dont change the 
            settings etc. */
            /* start Count cannot exceed the open count!! */
            if (hSpdifFm->uiStartCnt >= hSpdifFm->uiOpenCnt)
            {
                BDBG_ERR (("BRAP_SPDIFFM_P_Start: Cannot start SPDIFFM Stream %d. "
                        "Start Count (%d) >= Open Count (%d)",
                        hSpdifFm->uiStreamIndex, hSpdifFm->uiStartCnt, hSpdifFm->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
        
            hSpdifFm->uiStartCnt ++;

            BDBG_MSG (("BRAP_SPDIFFM_P_Start: SPDIFFM Stream %d was already started." 
                    "New start count = %d", 
                    hSpdifFm->uiStreamIndex, hSpdifFm->uiStartCnt));
            return ret;
        }

        /* Store the start parameters */
        hSpdifFm->sParams = *pParams;   
    }

    /* Configure SPDIF Formatter Hardware */
    BRAP_SPDIFFM_P_HWConfig (hRapCh, hSpdifFm);
    if(ret != BERR_SUCCESS)
    {
    	BDBG_ERR(("BRAP_SPDIFFM_P_Start(): BRAP_SPDIFFM_P_HWConfig() failed(%d)",ret));
    	return BERR_TRACE(ret);
    }

    if ((BRAP_FMM_P_GetWatchdogRecoveryFlag (hSpdifFm->hFmm) == false)
          && (ret == BERR_SUCCESS))
    {   /* If not in WatchDog recovery */  
        hSpdifFm->uiStartCnt ++;
    }
    
    BDBG_LEAVE (BRAP_SPDIFFM_P_Start);
    return ret;
}


BERR_Code 
BRAP_SPDIFFM_P_Stop (
    BRAP_SPDIFFM_P_Handle  hSpdifFm       /* [in] SPDIF Formatter handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegValue;

    BDBG_ENTER (BRAP_SPDIFFM_P_Stop);
    BDBG_ASSERT (hSpdifFm);
  
    if (hSpdifFm->uiStartCnt == 0)
    {
        BDBG_WRN (("BRAP_SPDIFFM_P_Stop: SPDIFFM %d stream %d is not active!!"
                    "Ignoring this call to _Stop()!!",
                    hSpdifFm->uiIndex, hSpdifFm->uiStreamIndex));
        BDBG_LEAVE (BRAP_SPDIFFM_P_Stop);
        return BERR_SUCCESS;  
    }   
    hSpdifFm->uiStartCnt --;   

    BDBG_MSG (("BRAP_SPDIFFM_P_Stop: SPDIFFM Stream  %d new start count =%d",
                hSpdifFm->uiStreamIndex, hSpdifFm->uiStartCnt));
    if (hSpdifFm->uiStartCnt != 0)
    {
            BDBG_LEAVE (BRAP_SPDIFFM_P_Stop);
            return ret;
    }    

#if (BRAP_7405_FAMILY == 1)
    BRAP_Write32 (hSpdifFm->hRegister,
              BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0 + hSpdifFm->ui32Offset , 
              0x800); 
#endif


    ui32RegValue = BRAP_Read32 (hSpdifFm->hRegister, 
            BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset);
    ui32RegValue &= ~( BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA) );    
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      STREAM_ENA, Disable));
    BRAP_Write32 (hSpdifFm->hRegister, 
                  hSpdifFm->ui32Offset + BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0,
                  ui32RegValue); 

    BDBG_LEAVE (BRAP_SPDIFFM_P_Stop);
    return ret;
}



/***************************************************************************
Summary:
    Configures the HW registers for the SPDIF Formatter
    Must be called only after the output port has been configured and opened.    
Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
static BERR_Code 
BRAP_SPDIFFM_P_HWConfig ( 
	BRAP_ChannelHandle 		hRapCh,		/* [in] Rap channel handle */	
    BRAP_SPDIFFM_P_Handle   hSpdifFm    /* [in] SPDIF Formatter handle */
)
{
   
	BERR_Code ret = BERR_SUCCESS;
	BRAP_SPDIFFM_P_Params sParams;
	uint32_t  ui32RegVal;

	uint32_t  ui32SpdifChStatusBits[BRAP_SPDIFFM_CBIT_BUFFER_SIZE];
	unsigned int uiCnt = 0;

#if (BRAP_7405_FAMILY == 1)
        unsigned int    uiSpdiffmSR=0;
        uint32_t    ui32HwCbitLo = 0,ui32HwCbitHi =0;
#endif
	
    BDBG_ENTER (BRAP_SPDIFFM_P_HWConfig);
    BDBG_ASSERT (hSpdifFm);
    BDBG_MSG (("BRAP_SPDIFFM_P_HWConfig(): hSpdifFm=0x%x SPDIFFM Stream index=0x%08x ", hSpdifFm, hSpdifFm->uiStreamIndex));   
  
    sParams = hSpdifFm->sParams;    

    BDBG_MSG(("BRAP_SPDIFFM_P_HWConfig: hSpdifFm->sSettings:"));
    BDBG_MSG(("\t bEnableDither=%d", hSpdifFm->sSettings.sExtSettings.bEnableDither));    
    BDBG_MSG(("\t eBurstType=%d", hSpdifFm->sSettings.sExtSettings.eBurstType));        

    ret = BRAP_SPDIFFM_P_ProgramDitherAndBurst(hSpdifFm, 
                                               &hSpdifFm->sSettings,
                                               sParams.eBurstRepPeriod);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_SPDIFFM_P_HWConfig(): ProgramDitherAndBurst returned err(%d)",ret));
		return BERR_TRACE(ret);
	}
    
    ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset);
    
    ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, CHAN_OVERRIDE)) 
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, VALIDITY)) 
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, COMP_OR_LINEAR))
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM))
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, FLUSH_ON_UFLOW))
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_ON_UFLOW))
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA))      
                     | (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA)) );

    if (sParams.bSeparateLRChanNum == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, CHAN_OVERRIDE, 1));
    }
    if ((sParams.sExtParams.bSpdifFormat == true)
#if (BRAP_7405_FAMILY == 1)                
        && (false==hSpdifFm->sParams.bUseHwCBit)
#endif
      )

    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM, 1));
        if (sParams.bCompressed == true)
        {
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                            VALIDITY, 1));
        } 
        else  
        {
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                            VALIDITY, 0));
        }             
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM, 0));               
    }

    if (sParams.bCompressed == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, COMP_OR_LINEAR, 1));
    }    
	else  
	{
	    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, COMP_OR_LINEAR, 0));
    }    


/*	if(hRapCh->eChannelType != BRAP_ChannelType_eDecode)*/
	{
		ret = BRAP_SPDIFFM_P_ProgramChanStatusBits(hSpdifFm,hSpdifFm->sParams.eSamplingRate);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_SPDIFFM_P_HWConfig(): ProgramChanStatusBits returned err(%d)",ret));
			return BERR_TRACE(ret);
		}
	}
	if(( hRapCh->eChannelType == BRAP_ChannelType_eDecode ) 
		&&( true==hSpdifFm->sParams.bUseSpdifPackedChanStatusBits ))
	{
		for(uiCnt = 0; uiCnt < BRAP_SPDIFFM_CBIT_BUFFER_SIZE; uiCnt++)
		{
			ui32SpdifChStatusBits[uiCnt] = 0;
		}
		ui32SpdifChStatusBits[0] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0];
		ui32SpdifChStatusBits[1] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[1];
		/* Second argument to function BRAP_SPDIFFM_P_ProgramCBITBuffer_isr() should have
		 * C-Bit values initialized in lower 16-bits of the array elements. */
		ui32SpdifChStatusBits[0] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0] & 0xFFFF;
		ui32SpdifChStatusBits[1] = ( hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0] & 0xFFFF0000 ) >> 16;
		ui32SpdifChStatusBits[2] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[1] & 0xFFFF;
		
    		BKNI_EnterCriticalSection();
		ret = BRAP_SPDIFFM_P_ProgramCBITBuffer_isr(hSpdifFm, &(ui32SpdifChStatusBits[0] )); 
    		BKNI_LeaveCriticalSection();

		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_SPDIFFM_P_HWConfig(): ProgramCBITBuffer_isr returned err(%d)",ret));
			return BERR_TRACE(ret);
		}
	}

    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_ON_UFLOW, 1));
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, FLUSH_ON_UFLOW, 1));
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA, 1));


    BRAP_Write32 (hSpdifFm->hRegister,
                  BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0 + hSpdifFm->ui32Offset, 
                  0x0);    

    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, Enable));

   
    BRAP_Write32 (hSpdifFm->hRegister, 
                  BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + hSpdifFm->ui32Offset, 
                  ui32RegVal); 
    
    if (false==hSpdifFm->sParams.bUseHwCBit)
    {
        ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);

        if (0==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, No_bypass);
        }
        if (1==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, No_bypass);
        }
        BRAP_Write32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS,
                          ui32RegVal);
        
#if (!( (((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325)) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1)))|| (BRAP_3548_FAMILY == 1)))
        /*Disable hardware channel status for SPDIF*/
        if (0==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal=0x0;
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ENABLE));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ENABLE, Disable));

            BRAP_Write32 (hSpdifFm->hRegister,
                          BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ui32RegVal);
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, 0);    
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, 0);                
        }

        /*Disble hardware channel status for HDMI*/
        if (1==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal=0x0;
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE, Disable));

            BRAP_Write32 (hSpdifFm->hRegister,
                          BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ui32RegVal);
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, 0);    
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, 0);                
        }
#endif        
    }
    else
    {
    
#if ( (((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325)) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) || (BRAP_3548_FAMILY == 1))
        BDBG_ERR(("hSpdifFm->sParams.bUseHwCBit can't be true for 7405A0/7325A0"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
#else        
        ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);

        if (0==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, Bypass);
        }
        if (1==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, Bypass);
        }
        BRAP_Write32 (hSpdifFm->hRegister, 
                      BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS,
                      ui32RegVal);
        
        switch(hSpdifFm->sParams.eSamplingRate)
        {   
            case BAVC_AudioSamplingRate_e32k:
                uiSpdiffmSR = 3; /* 1100 = 12 */
                break;

            case BAVC_AudioSamplingRate_e44_1k:
                uiSpdiffmSR = 0; /* 0000 = 0 */
                break;

            case BAVC_AudioSamplingRate_e48k:
                uiSpdiffmSR = 2; /* 0100 = 4 */
                break;

            case BAVC_AudioSamplingRate_e96k:
                uiSpdiffmSR = 10; /* 0101 = 5 */
                break;

            case BAVC_AudioSamplingRate_e22_05k:
                uiSpdiffmSR = 4; /* 0010 = 2 */
                break;

            case BAVC_AudioSamplingRate_e88_2k:
                uiSpdiffmSR = 8; /* 0001 = 1 */
                break;

            case BAVC_AudioSamplingRate_e176_4k:
                uiSpdiffmSR = 12; /* 0011 = 3 */
                break;

            case BAVC_AudioSamplingRate_e24k:
                uiSpdiffmSR = 6; /* 0110 = 6 */
                break;

            case BAVC_AudioSamplingRate_e192k:
                uiSpdiffmSR = 14; /* 0111 = 7 */
                break;

            default:
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }/* switch */

        ui32HwCbitLo =  (hSpdifFm->sParams.bCompressed << 0x1) | ((hSpdifFm->sParams.sChanStatusParams.bSwCopyRight^1)<< 0x2)
        |(((uint8_t)hSpdifFm->sParams.sChanStatusParams.ui16CategoryCode) << 0x8) | (uiSpdiffmSR << 24)
        |(hSpdifFm->sParams.sChanStatusParams.ui16ClockAccuracy << 28) ;

        if(hSpdifFm->sParams.bCompressed == true)
        {
            ui32HwCbitHi=0;
        }
        else
        {
            ui32HwCbitHi= uiSpdiffmSR << 4;
        }   
        
        /*Enable hardware channel status for SPDIF*/
        if (0==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal=0x0;
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0);
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ENABLE, Enable));

            BRAP_Write32 (hSpdifFm->hRegister,
                          BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ui32RegVal);
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, ui32HwCbitLo);    
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, ui32HwCbitHi);               
        }

        /*Enable hardware channel status for HDMI*/
        if (1==hSpdifFm->uiStreamIndex)
        {
            ui32RegVal=0x0;
            ui32RegVal = BRAP_Read32 (hSpdifFm->hRegister, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1);
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE, Enable));

            BRAP_Write32 (hSpdifFm->hRegister,
                          BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ui32RegVal);
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, ui32HwCbitLo);    
            BRAP_Write32 (hSpdifFm->hRegister,
                BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, ui32HwCbitHi);                
        }
#endif        
    
    }

    /* Note: Currently the DSP FW programs the CBIT buffers. If not done by 
       the DSP FW, PI should do it here. */

    BDBG_LEAVE (BRAP_SPDIFFM_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
    Prepares the CBIT buffer with the SPDIF channel status params
Description:
    NOTE: this PI should be called only by PB and CAP channels. 
    The DSP will program the CBIT buffers for a decode channel
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
	BRAP_SPDIFFM_P_ProgramCBITBuffer_isr
**************************************************************************/
static BERR_Code 
BRAP_SPDIFFM_P_PrepareCBITData_isr(
	uint32_t *pui32CBitData, 			/* [out] CBit buffer */
	BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams,
										/* SPDIF Channel status params */
	BAVC_AudioSamplingRate eOutputSamplingRate,
										/* Output sampling rate */
    bool                   bCompressed  /* TRUE: input data is compressed  */
)
{
	unsigned int uiSpdiffmSR;
	int count;
	bool bSwCopyRightAsserted;
	bool bProfMode, bSeparateLR;
	uint32_t ui32ChanDataLow, ui32ChanDataHigh,ui32CgmsDataField;
	uint32_t ui32CategoryCode, ui32ClockAccuracy,ui32Cgms;
	uint32_t *pui32ChanDataPtr = pui32CBitData;

    BDBG_ENTER (BRAP_SPDIFFM_P_PrepareCBITData_isr);
    BSTD_UNUSED (bCompressed);
    
	bProfMode = sSpdifChanStatusParams.bProfessionalMode;
	bSeparateLR = sSpdifChanStatusParams.bSeparateLRChanNum;
	ui32ClockAccuracy = sSpdifChanStatusParams.ui16ClockAccuracy;
        ui32Cgms = sSpdifChanStatusParams.uiCGMS_A;
	ui32CategoryCode = (uint32_t)sSpdifChanStatusParams.ui16CategoryCode;
	bSwCopyRightAsserted = sSpdifChanStatusParams.bSwCopyRight;
	bSwCopyRightAsserted ^= 1;
	
	/*  These frequency mapping has been defined based on relation between 
	 	BAVC_AudioSamplingRate and SPDIF sampling frequencies
        Refer IEC 60958-3.
		Note that these are to be stored in bit reversed order	*/
	switch(eOutputSamplingRate)
    {   
  	    case BAVC_AudioSamplingRate_e32k:
            uiSpdiffmSR = 3; /* 1100 = 12 */
            break;
            
  	    case BAVC_AudioSamplingRate_e44_1k:
            uiSpdiffmSR = 0; /* 0000 = 0 */
            break;
            
  	    case BAVC_AudioSamplingRate_e48k:
            uiSpdiffmSR = 2; /* 0100 = 4 */
            break;
            
        case BAVC_AudioSamplingRate_e96k:
            uiSpdiffmSR = 10; /* 0101 = 5 */
            break;
            
        case BAVC_AudioSamplingRate_e22_05k:
            uiSpdiffmSR = 4; /* 0010 = 2 */
            break;
            
        case BAVC_AudioSamplingRate_e88_2k:
            uiSpdiffmSR = 8; /* 0001 = 1 */
            break;
            
        case BAVC_AudioSamplingRate_e176_4k:
            uiSpdiffmSR = 12; /* 0011 = 3 */
            break;
            
        case BAVC_AudioSamplingRate_e24k:
            uiSpdiffmSR = 6; /* 0110 = 6 */
            break;
            
        case BAVC_AudioSamplingRate_e192k:
            uiSpdiffmSR = 14; /* 0111 = 7 */
            break;
            
        default:
            BDBG_ERR(("BRAP_SPDIFFM_P_PrepareCBITData_isr: Not supported eOutputSamplingRate(%d)",
                eOutputSamplingRate));    
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }/* switch */
    
 	if(bProfMode)
  	{
   		*pui32ChanDataPtr++ = (0xC000)|(uiSpdiffmSR<<8);
		for(count=0;count<11;count++)
		    *pui32ChanDataPtr++=0x00000000; 
	}
	else
  	{
   		ui32ChanDataLow = (ui32CategoryCode<<8)|(bSwCopyRightAsserted<<2)|
						  (bCompressed<<1);
        
   		ui32ChanDataHigh = (ui32ClockAccuracy<<12)|
						   (uiSpdiffmSR<<8)|
						   (bSeparateLR<<4);
               ui32CgmsDataField = (ui32Cgms << 8);
   		*pui32ChanDataPtr++ = ui32ChanDataLow;
   		*pui32ChanDataPtr++ = ui32ChanDataHigh; 
   		*pui32ChanDataPtr++ = ui32CgmsDataField;         

            BDBG_MSG(("BRAP_SPDIFFM_P_PrepareCBITData_isr: (%d)(%d)(%d)",ui32ChanDataLow,ui32ChanDataHigh,ui32CgmsDataField));

   		/* Remaining all 10 words are filled with zeros as per the specifications */
   		for(count=0;count<9;count++)
    		*pui32ChanDataPtr++=0x00000000; 
  	}

    BDBG_LEAVE(BRAP_SPDIFFM_P_PrepareCBITData_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Configures the HW registers with the CBIT buffer params
Description:
    ISR version of BRAP_SPDIFFM_P_ProgramCBITBuffer()
    NOTE: this PI should be called only by PB and CAP channels. 
    The DSP will program the CBIT buffers for a decode channel
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
	BRAP_SPDIFFM_P_PrepareCBITData_isr
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_ProgramCBITBuffer_isr( 
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    uint32_t			*pui32CBITBuffer /* [in] SPDIF bit buffer */
)
{
    uint32_t            	ui32CbitArrayOffset;
    uint32_t            	ui32Offset;
    uint32_t 				ui32RegValue; 
    unsigned int 			uiCnt = 0;
    uint32_t                ui32CpToggleRate = 0;
    BERR_Code               ret = BERR_SUCCESS;
    unsigned int            uiSR = 0;

#if (BRAP_7405_FAMILY == 1)    
    unsigned int ui32OrigRegVal=0;
#endif

    /* Validate input params */
    BDBG_ASSERT(hSpdifFm);
    BDBG_ASSERT(pui32CBITBuffer);

    /* Write to CBIT buffer */
    switch(hSpdifFm->uiStreamIndex)
    {
        case 0:
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: Stream Index 0"));
            ui32CbitArrayOffset = 
                (	BRAP_SPDIFFM_PLAYBACK_MS0_CBIT_BUFFER * 
                    BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_ELEMENT_SIZE / 4);
            break;
        case 1:
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: Stream Index 1"));
            ui32CbitArrayOffset = 
                (   BRAP_SPDIFFM_PLAYBACK_MS1_CBIT_BUFFER * 
                    BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_ELEMENT_SIZE / 4);
            break;
        default:
    		BDBG_ERR(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: Not supported index (%d)",
				hSpdifFm->uiIndex));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Populate the selected CBIT buffer */
        /*Program Ping CBIT Buffer*/
    for(uiCnt = 0; uiCnt < BRAP_SPDIFFM_CBIT_BUFFER_SIZE; uiCnt++)
    {
#if (BRAP_7405_FAMILY == 1) 
        ui32OrigRegVal = BRAP_Read32_isr(hSpdifFm->hRegister, 
                     BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (uiCnt*4));

        if(ui32OrigRegVal != pui32CBITBuffer[uiCnt])
#endif
        {
        BRAP_Write32_isr(hSpdifFm->hRegister, 
                     BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (uiCnt*4), 
                     pui32CBITBuffer[uiCnt]);
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: [%x]",BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (uiCnt*4)));
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr value[%d] : [%x]",uiCnt,pui32CBITBuffer[uiCnt]));	
    }
    }

    /*Program Pong CBIT Buffer*/
        for(uiCnt = 0; uiCnt < BRAP_SPDIFFM_CBIT_BUFFER_SIZE; uiCnt++)
    {
#if (BRAP_7405_FAMILY == 1) 
        ui32OrigRegVal = BRAP_Read32_isr(hSpdifFm->hRegister, 
                     BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (BRAP_SPDIFFM_CBIT_BUFFER_SIZE*4) + (uiCnt*4));

        if(ui32OrigRegVal != pui32CBITBuffer[uiCnt])
#endif
        {    
        BRAP_Write32_isr(hSpdifFm->hRegister, 
                     BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (BRAP_SPDIFFM_CBIT_BUFFER_SIZE*4) + (uiCnt*4), 
                     pui32CBITBuffer[uiCnt]);
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: [%x]",BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (BRAP_SPDIFFM_CBIT_BUFFER_SIZE*4) + (uiCnt*4)));
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr value[%d] : [%x]",uiCnt,pui32CBITBuffer[uiCnt]));	
    }
    }


    /* Program the CBIT Buffer Offset / 4 to the Offset field of 
       BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_X register */
    ui32Offset = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                ui32CbitArrayOffset - BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;

	ui32Offset = ui32Offset/4;

    ui32RegValue = BRAP_Read32_isr(hSpdifFm->hRegister,  
                        BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0 + 
                        hSpdifFm->ui32Offset);

    ui32RegValue &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, OFFSET));
	/* Though we dont ever write to CP_TOGGLE_RATE, make sure its cleared here 
    just in case it had a junk value earlier. */ 
    ui32RegValue &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, CP_TOGGLE_RATE));

    /* Program CP_TOGGLE_RATE only for 7440 based chips */
    if(hSpdifFm->sParams.uiCpToggleRate == BRAP_SPDIFFM_CP_TOGGLE_RATE_DISABLED)
    {
        /* CP_TOGGLE_RATE disabled - as in default */
        ui32CpToggleRate = 0;
    }
    else if((hSpdifFm->sParams.uiCpToggleRate >= BRAP_SPDIFFM_MIN_CP_TOGGLE_RATE_HZ) &&
       (hSpdifFm->sParams.uiCpToggleRate <= BRAP_SPDIFFM_MAX_CP_TOGGLE_RATE_HZ))
    {
        /* Convert eSR to uiSR */
        ret = BRAP_P_ConvertSR(hSpdifFm->sParams.eSamplingRate, &uiSR);
        if(ret != BERR_SUCCESS)
        {
            return BERR_TRACE(ret);
        }

        /* The Cp toggle rate to be programmed in the register equals 
           sample_rate/(192*cp_toggle_rate*2) */
        ui32CpToggleRate = (((uiSR / 192) / hSpdifFm->sParams.uiCpToggleRate) / 2);
        BDBG_MSG(("uiSR=%d hSpdifFm->sParams.uiCpToggleRate=%d ui32CpToggleRate=%d",
            uiSR, hSpdifFm->sParams.uiCpToggleRate, ui32CpToggleRate));
    }
    else
    {
        BDBG_ERR(("Valid Range (4-10). Invalid CpToggleRate = %d",hSpdifFm->sParams.uiCpToggleRate));
        return BERR_TRACE(BERR_NOT_SUPPORTED);    
    }

    ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, 
                    CP_TOGGLE_RATE, 
                    ui32CpToggleRate);
    
    ui32RegValue |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, 
                        OFFSET, 
                        ui32Offset);

	BRAP_Write32_isr(hSpdifFm->hRegister, 
                 BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0 + 
                 hSpdifFm->ui32Offset, 
             	 ui32RegValue);

	BDBG_LEAVE(BRAP_SPDIFFM_P_ProgramCBITBuffer_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Prepares and programs channel status bits. 
Description:
    NOTE: this PI should be called only by PB and CAP channels. 
    The DSP will program the CBIT buffers for a decode channel
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	BRAP_SPDIFFM_P_PrepareCBITData_isr, BRAP_SPDIFFM_P_ProgramCBITData_isr
**************************************************************************/
BERR_Code BRAP_SPDIFFM_P_ProgramChanStatusBits(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,       /* [in] SPDIF Formatter handle */
    BAVC_AudioSamplingRate  eSamplingRate   /* [in] SPDIF Output SR */
    ) 
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_SPDIFFM_P_ProgramChanStatusBits);

    BKNI_EnterCriticalSection();
    ret = BRAP_SPDIFFM_P_ProgramChanStatusBits_isr(hSpdifFm,eSamplingRate);
    BKNI_LeaveCriticalSection();

    BDBG_ENTER(BRAP_SPDIFFM_P_ProgramChanStatusBits);
    return ret;
}
/***************************************************************************
Summary:
    Prepares and programs channel status bits. 
Description:
    NOTE: this PI should be called only by PB and CAP channels. 
    The DSP will program the CBIT buffers for a decode channel
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	BRAP_SPDIFFM_P_PrepareCBITData_isr, BRAP_SPDIFFM_P_ProgramCBITData_isr
**************************************************************************/
BERR_Code BRAP_SPDIFFM_P_ProgramChanStatusBits_isr(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,       /* [in] SPDIF Formatter handle */
    BAVC_AudioSamplingRate  eSamplingRate   /* [in] SPDIF Output SR */
) 
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32CBitData[BRAP_SPDIFFM_CBIT_BUFFER_SIZE];
    uint32_t  ui32SpdifChStatusBits[BRAP_SPDIFFM_CBIT_BUFFER_SIZE];
    unsigned int uiCnt = 0;



    BDBG_ENTER(BRAP_SPDIFFM_P_ProgramChanStatusBits_isr);

    BDBG_ASSERT(hSpdifFm);

	if(true==hSpdifFm->sParams.bUseSpdifPackedChanStatusBits)
	{
		for(uiCnt = 0; uiCnt < BRAP_SPDIFFM_CBIT_BUFFER_SIZE; uiCnt++)
		{
			ui32SpdifChStatusBits[uiCnt] = 0;
		}
		ui32SpdifChStatusBits[0] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0];
		ui32SpdifChStatusBits[1] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[1];
		/* Second argument to function BRAP_SPDIFFM_P_ProgramCBITBuffer_isr() should have
		 * C-Bit values initialized in lower 16-bits of the array elements. */
		ui32SpdifChStatusBits[0] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0] & 0xFFFF;
		ui32SpdifChStatusBits[1] = ( hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[0] & 0xFFFF0000 ) >> 16;
		ui32SpdifChStatusBits[2] = hSpdifFm->sParams.sSpdifPackedChanStatusBits.ui32ChStatusBits[1] & 0xFFFF;
		
		ret = BRAP_SPDIFFM_P_ProgramCBITBuffer_isr(hSpdifFm, &(ui32SpdifChStatusBits[0] )); 

		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_SPDIFFM_P_ProgramChanStatusBits_isr(): ProgramCBITBuffer_isr returned err(%d)",ret));
			return BERR_TRACE(ret);
		}
	}
    else
    {
        BRAP_SPDIFFM_P_PrepareCBITData_isr(&ui32CBitData[0], 
									   hSpdifFm->sParams.sChanStatusParams,
									   eSamplingRate,
									   hSpdifFm->sParams.bCompressed);
        
        ret = BRAP_SPDIFFM_P_ProgramCBITBuffer_isr(hSpdifFm, &ui32CBitData[0]); 
        if(ret != BERR_SUCCESS)
        {
        	BDBG_ERR(("BRAP_SPDIFFM_P_ProgramChanStatusBits_isr: ProgramCBITBuffer returned err(%d)",ret));
        }
    }

    BDBG_ENTER(BRAP_SPDIFFM_P_ProgramChanStatusBits_isr);
    return ret;
}



/* End of File */
