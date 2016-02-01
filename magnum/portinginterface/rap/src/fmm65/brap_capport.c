/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
*   Module name: CAPPORT
*   This file contains the implementation of all APIs for the Capture Port
*   abstraction.
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_capport);

static const BRAP_CAPPORT_P_Params defCapPortParam = 
{
    BRAP_INVALID_VALUE,             /* uiInputFciId */
    BRAP_InputBitsPerSample_e16,    /* eInputBitsPerSample */
    BRAP_INVALID_VALUE,             /* uiGrpId */
    BRAP_RM_P_INVALID_INDEX,        /* uiFsTmgSrc */
    BRAP_OP_Pll_eMax,               /* ePll */
    BAVC_AudioSamplingRate_eUnknown /* eSamplingRate */
};

/***************************************************************************
Summary:
    This function programs the Clock to drive the Capture Port.

Description:
    The Sample Rate of the loopback capture(Internal Capture port) is set by 
    programming the PLL registers.
    
Returns:
    BERR_SUCCESS else error
    
See Also:
    BRAP_OP_P_ProgramOutputClock

**************************************************************************/
static BERR_Code 
BRAP_CAPPORT_P_ProgramCaptureClock(
    BRAP_CAPPORT_P_Handle   hCapPort,      /* [in] Capture Port Handle */
    unsigned int            uiFsTmgSrcId,   /* [in] FS Timing Source Id */
    BRAP_OP_Pll 			ePll,           /* [in] PLL to be associated */
    BAVC_AudioSamplingRate 	eSamplingRate   /* [in] Sampling rate of the data at 
                                               CapPort or FCI sinker driven by 
                                               this FS timing source */
);

/***************************************************************************
Summary:
    Returns the Default Params for Capture Port.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all Capture Ports.
    
Returns:
    BERR_SUCCESS else error
    
See Also:

**************************************************************************/
BERR_Code 
BRAP_CAPPORT_P_GetDefaultParams ( 
    BRAP_CAPPORT_P_Params    *pDefParams    /* Pointer to memory where default
                                               settings should be written */    
)
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_CAPPORT_P_GetDefaultParams);
    BDBG_ASSERT (pDefParams);

    *pDefParams = defCapPortParam;

    BDBG_LEAVE(BRAP_CAPPORT_P_GetDefaultParams);
    return ret;    
}

/***************************************************************************
Summary:
    Opens a Capture port
    
Description:
    Initializes the Capture port and returns a Capture Port handle.The handle 
    can then be used for all other function calls.

Returns:
    BERR_SUCCESS else error

See Also:
    BRAP_CAPPORT_P_Close
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Open (
    BRAP_FMM_P_Handle           hFmm,           /* [in] Parent FMM handle */
    BRAP_CAPPORT_P_Handle       *phCapPort,     /* [out] Pointer to Capture Port handle */
    BRAP_CapInputPort           eCapPort,       /* [in] Capture Port index */
    const BRAP_CAPPORT_P_Settings *pSettings      /* [in] Capture Port settings */
)
{
	BERR_Code               ret = BERR_SUCCESS;
    BRAP_CAPPORT_P_Handle   hCapPort = NULL;

    BDBG_ENTER (BRAP_CAPPORT_P_Open);
    
    BDBG_MSG (("BRAP_CAPPORT_P_Open:hFmm=0x%x, eCapPort=%d,No settings",
                                                               hFmm, eCapPort));

    /* Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of Capture Port
     * - Pointer to Settings structure is NULL
     * - Pointer to Capture Port handle is NULL     */
     
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phCapPort);
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */    
        BDBG_ASSERT (pSettings);
    }
    BSTD_UNUSED(pSettings);
    if (eCapPort >= BRAP_CapInputPort_eMax)
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }       

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   
        /* If not in WatchDog recovery */  
        if(BRAP_P_IsPointerValid((void *)hFmm->hCapPort[eCapPort]))
        {
            /* This capport was previoulsy opened by another audio channel.
            So this time, just increment the open count. Dont change the 
            settings etc. */
            hCapPort = hFmm->hCapPort[eCapPort];
     
            *phCapPort = hCapPort;
            hCapPort->uiOpenCnt++;
            BDBG_MSG (("BRAP_CAPPORT_P_Open: Capture port %d was already open." 
                        "New open count = %d", eCapPort, hCapPort->uiOpenCnt));
            BDBG_LEAVE (BRAP_CAPPORT_P_Open);
            return ret;
        }
    
        /* Allocate memory for the Capture Port handle. Fill in params 
           in the Capture Port handle. */
    
        /* Allocate Capture port handle */
        hCapPort = (BRAP_CAPPORT_P_Handle) BKNI_Malloc (sizeof(BRAP_CAPPORT_P_Object));
        if ( NULL==hCapPort)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset (hCapPort, 0, sizeof(BRAP_CAPPORT_P_Object));

        /* Initialise known elements in Capture Port handle */
        hCapPort->hChip = hFmm->hChip;
        hCapPort->hRegister = hFmm->hRegister;
        hCapPort->hHeap = hFmm->hHeap;
        hCapPort->hInt = hFmm->hInt;
        hCapPort->hFmm = hFmm;
        hCapPort->eCapPort = eCapPort;
        hCapPort->uiOpenCnt = 1;
        hCapPort->uiStartCnt = 0;        
    }
    else
    {
        hCapPort = *phCapPort;
    }

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm) == false)
    {   /* If not in WatchDog recovery */ 

        /* Store Capture Port handle inside the FMM handle */
        hFmm->hCapPort[eCapPort] = hCapPort;
    
        *phCapPort = hCapPort;
    }   /* End: If not in WatchDog recovery */        
    
    BDBG_MSG(("BRAP_CAPPORT_P_Open: handle=0x%x", hCapPort)); 
    BDBG_LEAVE (BRAP_CAPPORT_P_Open);
    return ret;
}

/***************************************************************************
Summary:
    Releases all the resources associated with this Capture port and frees 
    the handles.
    
Description:

Returns:
    BERR_SUCCESS else error

See Also:
BRAP_CAPPORT_P_Open
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Close ( 
    BRAP_CAPPORT_P_Handle   hCapPort      /* [in] Capture Port Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_CAPPORT_P_Close);
    BDBG_ASSERT (hCapPort);
    
    BDBG_MSG (("BRAP_CAPPORT_P_Close(): hCapPort=0x%x, eCapPort=%d ", 
        hCapPort, hCapPort->eCapPort));

    if(0 == hCapPort->uiOpenCnt)
    { 
        /* This should never happen. If it does, it means the system
           has gone into an invalid state!!!!*/
        BDBG_ERR (("BRAP_CAPPORT_P_Open: Capture port %d has NO open"
            " audio channels!!!", hCapPort->eCapPort));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    hCapPort->uiOpenCnt--;

    if(0 == hCapPort->uiOpenCnt)
    {   
        /* If this was the last open audio channel on this capture port,
           free the handle etc */
        /* Remove referrence to this Capture Port from the parent FMM */ 
        hCapPort->hFmm->hCapPort[hCapPort->eCapPort] = NULL;

        /* Free the Capture Port Handle memory*/
        BKNI_Free (hCapPort); 
    }             

    BDBG_LEAVE (BRAP_CAPPORT_P_Close);
    return ret;
}

#if ((BRAP_3548_FAMILY == 1))
/***************************************************************************
Summary:
    Configures the HW registers for the Capture Port

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CAPPORT_P_Start
**************************************************************************/
static BERR_Code BRAP_CAPPORT_P_HWConfig ( 
    BRAP_CAPPORT_P_Handle   hCapPort      /* [in] Capture Port Handle */)
{
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            ui32RegVal = 0;
    BREG_Handle         hRegister = NULL;
    BRAP_CAPPORT_P_Params sParams;

#if(BRAP_48KHZ_RINGBUFFER_DESTINATION == 1) 
    uint32_t            ui32FsOffset = 0;
#endif

    BDBG_ENTER (BRAP_CAPPORT_P_HWConfig);
    BDBG_ASSERT (hCapPort);

    BDBG_MSG (("BRAP_CAPPORT_P_HWConfig(): hCapPort=0x%x, Index=%d \n", 
                                hCapPort, hCapPort->eCapPort));
  
    hRegister = hCapPort->hRegister;
    sParams = hCapPort->sParams;    
    
    switch(hCapPort->eCapPort)
    {
#if ((BRAP_3548_FAMILY == 1) )
        case BRAP_CapInputPort_eSpdif:
        case BRAP_CapInputPort_eHdmi:            
            ret = BRAP_SPDIFRX_P_Start ( hCapPort->hFmm->hRap );
            if( BERR_SUCCESS!=ret )
                return BERR_TRACE( ret );
            break;

        case BRAP_CapInputPort_eRfAudio:
            ret = BRAP_RFAUDIO_P_Start( hCapPort );
            if( BERR_SUCCESS!=ret )
                return BERR_TRACE( ret );
            break;

        case BRAP_CapInputPort_eAdc:
            /* Program the PLL1 which is specific to ADC */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_VCXO_CTL_MISC_AC1_MACRO);
            ui32RegVal &= ~(BCHP_MASK (VCXO_CTL_MISC_AC1_MACRO, MACRO_SELECT));
            switch (sParams.eSamplingRate)
            {
                case BAVC_AudioSamplingRate_e32k:
                case BAVC_AudioSamplingRate_e64k:
                case BAVC_AudioSamplingRate_e128k:
                    /* All 32kHz family of sampling rates are achieved using 32kHz 
                       as the base sampling rate. Using PLL channel select 1x, 2x or 3x 
                       to achieve 32kHz, 64kHz or 128kHz */
                    ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC1_MACRO, MACRO_SELECT, Mult_of_32000));
                    break;
                    
                case BAVC_AudioSamplingRate_e44_1k:
                case BAVC_AudioSamplingRate_e88_2k:
                case BAVC_AudioSamplingRate_e176_4k:
                    /* All 44.1kHz family of sampling rates are achieved using 44.1kHz 
                       as the base sampling rate. Using PLL channel select 1x, 2x or 3x 
                       to achieve 44.1kHz, 88.2kHz or 176.4kHz */
                    ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC1_MACRO, MACRO_SELECT, Mult_of_44100));
                    break;
                    
                case BAVC_AudioSamplingRate_e48k:
                case BAVC_AudioSamplingRate_e96k:
                case BAVC_AudioSamplingRate_e192k:                    
                    /* All 48kHz family of sampling rates are achieved using 48kHz as 
                       the base sampling rate. Using PLL channel select 1x, 2x or 3x to 
                       achieve 48kHz*/
                    ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC1_MACRO, MACRO_SELECT, Mult_of_48000));
                    break;
                    
                default:
                    BDBG_MSG (("Incorrect Sampling Rate %d supplied. Forcing it to 48kHz", sParams.eSamplingRate));
                    sParams.eSamplingRate = BAVC_AudioSamplingRate_e48k;
                    ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC1_MACRO, MACRO_SELECT, Mult_of_48000));
            }
            BRAP_Write32(hRegister,BCHP_VCXO_CTL_MISC_AC1_MACRO, ui32RegVal);

            ui32RegVal = BRAP_Read32(hRegister,BCHP_VCXO_CTL_MISC_AC1_CONTROL);
            ui32RegVal &= ~(BCHP_MASK (VCXO_CTL_MISC_AC1_CONTROL, REFERENCE_SELECT));
            ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC1_CONTROL, REFERENCE_SELECT, VCXO_1));
            BRAP_Write32(hRegister,BCHP_VCXO_CTL_MISC_AC1_CONTROL, ui32RegVal);            
            
            /* ADCCIC needs to be put in Normal Operation */
            ui32RegVal = BRAP_Read32 (hRegister, BCHP_ADCCIC_CTRL_RESET);
            ui32RegVal &= ~( (BCHP_MASK (ADCCIC_CTRL_RESET, SYNC_RESET_FIFO)) |
                             (BCHP_MASK (ADCCIC_CTRL_RESET, SYNC_RESET))); 
            ui32RegVal |= (BCHP_FIELD_ENUM (ADCCIC_CTRL_RESET, SYNC_RESET_FIFO, Normal_operation));
            ui32RegVal |= (BCHP_FIELD_ENUM (ADCCIC_CTRL_RESET, SYNC_RESET, Normal_operation));            
            BRAP_Write32 (hRegister, BCHP_ADCCIC_CTRL_RESET, ui32RegVal);            
            
            ui32RegVal = BRAP_Read32(hRegister,BCHP_ADCCIC_CTRL_ADC_MUX_SEL);
            ui32RegVal &= ~(BCHP_MASK (ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL)); 

            switch (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sAdcParams.uiInputSelect)
            {
                case 0:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_1));        
                    break;

                case 1:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_2));        
                    break;

                case 2:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_3));        
                    break;

                case 3:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_4));        
                    break;

                case 4:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_5));        
                    break;

                case 5:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel_6));        
                    break;

                case 6:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel5));        
                    break;

                case 7:
                    ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_ADC_MUX_SEL, MUX_SEL, Channel6));        
                    break;

                default:
					BDBG_ERR (("BRAP_CAPPORT_P_HWConfig: "
							   "ADC uiInputSelect %d is not supported", 
							   hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sAdcParams.uiInputSelect));
					return BERR_TRACE (BERR_NOT_SUPPORTED);   
            }
            BRAP_Write32(hRegister,BCHP_ADCCIC_CTRL_ADC_MUX_SEL, ui32RegVal);

            /* Start the ADC */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_ADCCIC_CTRL_CONFIG);
            ui32RegVal &= ~(BCHP_MASK (ADCCIC_CTRL_CONFIG, 	ADC_CIC_OUTFIFO_ENA)); 
            ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_CONFIG, ADC_CIC_OUTFIFO_ENA, enable));
            BRAP_Write32(hRegister,BCHP_ADCCIC_CTRL_CONFIG,ui32RegVal);            
            break;
            
#endif
        case BRAP_CapInputPort_eExtI2s0:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, BITS_PER_SAMPLE)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_JUSTIFICATION)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_ALIGNMENT)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, LRCK_POLARITY)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, SCLK_POLARITY)));

            switch(sParams.eInputBitsPerSample)
			{
				case BRAP_InputBitsPerSample_e16:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth16));
					break;
				case BRAP_InputBitsPerSample_e18:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth18));
					break;
				case BRAP_InputBitsPerSample_e20:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth20));
					break;
				case BRAP_InputBitsPerSample_e24:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth24));
					break;
				case BRAP_InputBitsPerSample_e32:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth32));
					break;
				default:
					BDBG_ERR (("BRAP_CAPPORT_P_HWConfig: "
							   "I2S Input Bits/Sample %d is not supported", 
							   sParams.eInputBitsPerSample));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
					break;
			}

			/* Data Justification */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLsbAtLRClk == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_JUSTIFICATION, LSB));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_JUSTIFICATION, MSB));
			}

			/* Data Alignment */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk == false)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_ALIGNMENT, Delayed));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_ALIGNMENT, Aligned));
			}

			/* S Clock Polarity to set to Falling_aligned_with_sdata */
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
							SCLK_POLARITY, Falling_aligned_with_sdata));
			
			/* LR Clock Polarity */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLRClkPolarity == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												LRCK_POLARITY, High_for_left));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												LRCK_POLARITY, Low_for_left));
			}

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0,ui32RegVal);

            break;
            
        case BRAP_CapInputPort_eIntCapPort0:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, LOOPBACK0_INS)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL0)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, IGNORE_FIRST_UNDERFLOW0)));
            /* Program LOOPBACK0_INS, TMG_SRC_SEL0, IGNORE_FIRST_UNDERFLOW0 */

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG,ui32RegVal);

#if (BRAP_3548_FAMILY == 1)
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM9);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM9, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, ui32RegVal);

#else
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM8);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM8, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, ui32RegVal);            
#endif            
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, LOOPBACK1_INS)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL1)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, IGNORE_FIRST_UNDERFLOW1)));
            /* Program LOOPBACK1_INS, TMG_SRC_SEL1, IGNORE_FIRST_UNDERFLOW1 */

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG,ui32RegVal);

#if (BRAP_3548_FAMILY == 1)
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM10);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM10, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, ui32RegVal);
#else
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM9);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, STREAM9, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP98, ui32RegVal);
#endif            
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, LOOPBACK2_INS)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL2)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, IGNORE_FIRST_UNDERFLOW2)));
            /* Program LOOPBACK2_INS, TMG_SRC_SEL2, IGNORE_FIRST_UNDERFLOW2 */

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG,ui32RegVal);

#if (BRAP_3548_FAMILY == 1)
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM11);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM11, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, ui32RegVal);
#else
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM10);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM10, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, ui32RegVal);
#endif            
            
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, LOOPBACK3_INS)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL3)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFG, IGNORE_FIRST_UNDERFLOW3)));
            /* Program LOOPBACK3_INS, TMG_SRC_SEL3, IGNORE_FIRST_UNDERFLOW3 */

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG,ui32RegVal);

#if (BRAP_3548_FAMILY == 1)
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, STREAM12);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, STREAM12, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPdc, ui32RegVal);
#else
            /* Program the Input FCI Id */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba);
            ui32RegVal &= ~ BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM11);
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, STREAM11, sParams.uiInputFciId);
            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAPba, ui32RegVal);
#endif            
            
            break;
        default:
            BDBG_ASSERT (0);
    }

    /* Clock programming for Internal Capture ports */
    switch(hCapPort->eCapPort)
    {            
        case BRAP_CapInputPort_eIntCapPort0:
        case BRAP_CapInputPort_eIntCapPort1:
        case BRAP_CapInputPort_eIntCapPort2:
        case BRAP_CapInputPort_eIntCapPort3:

#if (BRAP_3548_FAMILY == 1)
            /* Internal capture ports require VCXO_0 to be programmed in PLL0 */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_VCXO_CTL_MISC_AC0_CONTROL);
            ui32RegVal &= ~(BCHP_MASK (VCXO_CTL_MISC_AC0_CONTROL, REFERENCE_SELECT));
            ui32RegVal |= (BCHP_FIELD_ENUM (VCXO_CTL_MISC_AC0_CONTROL, REFERENCE_SELECT, VCXO_0));
            BRAP_Write32(hRegister,BCHP_VCXO_CTL_MISC_AC0_CONTROL, ui32RegVal);  

#if(BRAP_48KHZ_RINGBUFFER_DESTINATION == 1) 
            /* If this macro==0 clk programming is done by FW */
            ui32FsOffset = 4 * hCapPort->sParams.uiFsTmgSrc;

            ui32RegVal = 0;
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, MCLK_RATE, MCLK_is_256fs);            
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, PLLCLKSEL, Mclk_gen0);
            BRAP_Write32 (hRegister, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_BASE + ui32FsOffset, 
                          ui32RegVal);            

            ui32RegVal = 0;
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_GEN_CFGi, TB_SEL, Timebase_0);            
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_GEN_CFGi, NCO_DELTA, 0x200);            
            ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_GEN_CFGi, NCO_MODULUS, 0x465);
            BRAP_Write32 (hRegister, BCHP_AUD_FMM_OP_CTRL_MCLK_GEN_CFGi_ARRAY_BASE + ui32FsOffset, 
                          ui32RegVal);    
#endif

#endif
            break;
        default:
            /* Do nothing */
            break;
    }/* switch eCapPort */      


    BDBG_LEAVE (BRAP_CAPPORT_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
    Enables a Capture Port.

Description:
    This function should be called for enabling the capture port to get the 
    date from its input.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CAPPORT_P_Stop, BRAP_CAPPORT_P_HWConfig
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Start ( 
	BRAP_CAPPORT_P_Handle       hCapPort,   /* [in] Capture Port Handle */
    const BRAP_CAPPORT_P_Params *pParams    /* [in] Pointer to start parameters 
                                               for this Capture Port */ 
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER (BRAP_CAPPORT_P_Start);
    BDBG_ASSERT (hCapPort);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hCapPort->hFmm) == false)
    {   
        /* If not in WatchDog recovery */  
        BDBG_ASSERT (pParams);

        if (hCapPort->uiStartCnt > 0)
        {
            /* This Outport was previoulsy started by another audio channel.
               So this time, just increment the start count. Dont change the 
               settings etc. */
            /* Also, start count cannot exceed the open count!! */
            if (hCapPort->uiStartCnt >= hCapPort->uiOpenCnt)
            {
                BDBG_ERR (("BRAP_CAPPORT_P_Start: Cannot start capport %d. "
                        "Start Count (%d) >= Open Count (%d)",
                        hCapPort->eCapPort, hCapPort->uiStartCnt, hCapPort->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
        
            hCapPort->uiStartCnt ++;
            BDBG_MSG (("BRAP_CAPPORT_P_Start: CapPort %d was already started." 
                        "New start count = %d", 
                        hCapPort->eCapPort, hCapPort->uiStartCnt));

            BDBG_LEAVE (BRAP_CAPPORT_P_Start);
            return ret;
        }
    
        /* Store the start parameters inside the handle */
        hCapPort->sParams = *pParams;

        BDBG_MSG (("BRAP_CAPPORT_P_Start(): hCapPort=0x%x, Type=%d, \n"
				"hCapPort->sParams.sExtParams.bLsbAtLRClk=%d\n"
				"hCapPort->sParams.sExtParams.bAlignedToLRClk=%d\n"
                "hCapPort->sParams.sExtParams.bLRClkPolarity=%d\n"
                "hCapPort->sParams.uiInputFciId =%d\n"
                "hCapPort->sParams.eInputBitsPerSample=%d\n",
                hCapPort, hCapPort->eCapPort,
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLsbAtLRClk, 
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk, 
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLRClkPolarity, 
				hCapPort->sParams.uiInputFciId, 				
				hCapPort->sParams.eInputBitsPerSample)); 
        

        BDBG_MSG (("hCapPort->sParams.uiGrpId = %d",hCapPort->sParams.uiGrpId));

#if ((BRAP_7405_FAMILY == 1))
        BDBG_MSG (("hCapPort->sParams.uiFsTmgSrc = %d \n"
                   "hCapPort->sParams.ePll = %d\n"
                   "hCapPort->sParams.eSamplingRate = %d \n",
                   hCapPort->sParams.uiFsTmgSrc, 
                   hCapPort->sParams.ePll,
                   hCapPort->sParams.eSamplingRate));
#endif /* 7405 */        
    
    }   /* End: If not in WatchDog recovery */
   
    hRegister = hCapPort->hRegister;

    /* Configure Capture Port Hardware */
    BRAP_CAPPORT_P_HWConfig (hCapPort);
    
	/* Enable Capture Port */
	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_IOP_CTRL_CAP_CFG );

    switch(hCapPort->eCapPort)
    {
#if ( (BRAP_3548_FAMILY == 1) )
        case BRAP_CapInputPort_eRfAudio:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW5))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA5)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW5,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA5,Enable);
            break;

        case BRAP_CapInputPort_eSpdif:
        case BRAP_CapInputPort_eHdmi:            
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW6))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA6)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW6,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA6,Enable);
            break;
            
        case BRAP_CapInputPort_eAdc:
#if (BCHP_VER==BCHP_VER_A0)
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW7))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA7)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW7,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA7,Enable);
#else
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW10))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA10)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW10,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA10,Enable);

#endif
            break;            
#endif
        case BRAP_CapInputPort_eExtI2s0:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW4))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA4)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW4,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA4,Enable);
            break;
        case BRAP_CapInputPort_eIntCapPort0:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW0))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA0)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW0,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA0,Enable);
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW1))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA1)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW1,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA1,Enable);
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW2))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA2)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW2,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA2,Enable);
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,IGNORE_FIRST_OVERFLOW3))
                            |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA3)));

            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,
                                            IGNORE_FIRST_OVERFLOW3,Ignore);
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA3,Enable);
            break;
        default:
            BDBG_ASSERT(0);
    }
    
	BRAP_Write32 (hRegister, BCHP_AUD_FMM_IOP_CTRL_CAP_CFG, ui32RegVal);

    /* Set the start count to 1 */
    hCapPort->uiStartCnt = 1;

    BDBG_LEAVE (BRAP_CAPPORT_P_Start);
    return ret;
}

/***************************************************************************
Summary:
    Disables the Capture port.

Description:
    This function disables the capture port.
Returns:
    BERR_Success else Error
See Also:
    BRAP_CAPPORT_P_Start
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Stop ( 
    BRAP_CAPPORT_P_Handle   hCapPort    /* [in] Capture Port Handle */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER(BRAP_CAPPORT_P_Stop);
    BDBG_ASSERT (hCapPort);

    BDBG_MSG (("BRAP_CAPPORT_P_Stop(): hCapPort=0x%x, eCapPort=%d ", 
        hCapPort, hCapPort->eCapPort));

    if(0 == hCapPort->uiStartCnt)
    { 
        BDBG_WRN (("BRAP_CAPPORT_P_Stop: CapPort %d has NO active"
            "audio channels. Ignoring this call to _Stop()!",
            hCapPort->eCapPort));
        BDBG_LEAVE (BRAP_CAPPORT_P_Stop);
        return ret;
    }
    
    hCapPort->uiStartCnt --;
    BDBG_MSG (("BRAP_CAPPORT_P_Stop: CapPort %d new start count =%d",
        hCapPort->eCapPort, hCapPort->uiStartCnt));
    if (hCapPort->uiStartCnt != 0)
    {
        BDBG_MSG(("BRAP_CAPPORT_P_Stop: So do nothing!!", 
            hCapPort->eCapPort, hCapPort->uiStartCnt));        
        BDBG_LEAVE (BRAP_OP_P_Stop);
        return ret;
    }

    hRegister = hCapPort->hRegister;


    /* Disable Capture Port */
	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_IOP_CTRL_CAP_CFG );

    switch(hCapPort->eCapPort)
    {
#if ((BRAP_3548_FAMILY == 1) )
        case BRAP_CapInputPort_eRfAudio:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA5));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA5,Disable);
            
            ret = BRAP_RFAUDIO_P_Stop( hCapPort );
            if( BERR_SUCCESS!=ret )
                return BERR_TRACE( ret );
            break;

        case BRAP_CapInputPort_eSpdif:
        case BRAP_CapInputPort_eHdmi:            
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA6));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA6,Disable);

            ret = BRAP_SPDIFRX_P_Stop (hCapPort->hFmm->hRap);
            if( BERR_SUCCESS!=ret )
                return BERR_TRACE( ret );            
            break;   

        case BRAP_CapInputPort_eAdc:
            /* Stop the ADC */
            ui32RegVal = BRAP_Read32(hRegister,BCHP_ADCCIC_CTRL_CONFIG);
            ui32RegVal &= ~(BCHP_MASK (ADCCIC_CTRL_CONFIG, 	ADC_CIC_OUTFIFO_ENA)); 
            ui32RegVal |= (BCHP_FIELD_ENUM(ADCCIC_CTRL_CONFIG, ADC_CIC_OUTFIFO_ENA, disable));
            BRAP_Write32(hRegister,BCHP_ADCCIC_CTRL_CONFIG,ui32RegVal);          
            
            ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_IOP_CTRL_CAP_CFG );
#if (BCHP_VER==BCHP_VER_A0)
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA7));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA7,Disable);
#else
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA10));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA10,Disable);
#endif
            break;              
#endif
        case BRAP_CapInputPort_eExtI2s0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA4));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA4,Disable);
            break;
        case BRAP_CapInputPort_eIntCapPort0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA0));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA0,Disable);
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA1));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA1,Disable);
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA2));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA2,Disable);
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFG,ENA3));
            ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFG,ENA3,Disable);
            break;
        default:
            BDBG_ASSERT(0);
    }
    
	BRAP_Write32 (hRegister, BCHP_AUD_FMM_IOP_CTRL_CAP_CFG, ui32RegVal);

    BDBG_LEAVE (BRAP_CAPPORT_P_Stop);
    return ret;
}
#else

/***************************************************************************
Summary:
    Configures the HW registers for the Capture Port

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CAPPORT_P_Start
**************************************************************************/
static BERR_Code BRAP_CAPPORT_P_HWConfig ( 
    BRAP_CAPPORT_P_Handle   hCapPort      /* [in] Capture Port Handle */)
{
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            ui32RegVal = 0, ui32LoopBackStreamId =0;
    uint32_t            ui32FciStreamId = 0;
    BREG_Handle         hRegister = NULL;
    BRAP_CAPPORT_P_Params sParams;

    BDBG_ENTER (BRAP_CAPPORT_P_HWConfig);
    BDBG_ASSERT (hCapPort);

    BDBG_MSG (("BRAP_CAPPORT_P_HWConfig(): hCapPort=0x%x, Index=%d \n", 
                                hCapPort, hCapPort->eCapPort));
  
    hRegister = hCapPort->hRegister;
    sParams = hCapPort->sParams;    
    
    switch(hCapPort->eCapPort)
    {
        case BRAP_CapInputPort_eExtI2s0:
            ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0);

            ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, BITS_PER_SAMPLE)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_JUSTIFICATION)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_ALIGNMENT)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, LRCK_POLARITY)) 
                     | (BCHP_MASK (AUD_FMM_IOP_CTRL_I2SIN_CFG0, SCLK_POLARITY)));

            switch(sParams.eInputBitsPerSample)
			{
				case BRAP_InputBitsPerSample_e16:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth16));
					break;
				case BRAP_InputBitsPerSample_e18:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth18));
					break;
				case BRAP_InputBitsPerSample_e20:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth20));
					break;
				case BRAP_InputBitsPerSample_e24:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth24));
					break;
				case BRAP_InputBitsPerSample_e32:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
											BITS_PER_SAMPLE, Bitwidth32));
					break;
				default:
					BDBG_ERR (("BRAP_CAPPORT_P_HWConfig: "
							   "I2S Input Bits/Sample %d is not supported", 
							   sParams.eInputBitsPerSample));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
					break;
			}

			/* Data Justification */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLsbAtLRClk == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_JUSTIFICATION, LSB));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_JUSTIFICATION, MSB));
			}

			/* Data Alignment */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk == false)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_ALIGNMENT, Delayed));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												DATA_ALIGNMENT, Aligned));
			}

			/* S Clock Polarity to set to Falling_aligned_with_sdata */
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
							SCLK_POLARITY, Falling_aligned_with_sdata));
			
			/* LR Clock Polarity */
			if (hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLRClkPolarity == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												LRCK_POLARITY, High_for_left));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_IOP_CTRL_I2SIN_CFG0, 
												LRCK_POLARITY, Low_for_left));
			}

            BRAP_Write32(hRegister,BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0,ui32RegVal);

            break;
#if ((BRAP_7420_FAMILY==1) && (!((BCHP_CHIP == 7420) && (BCHP_VER == A0))))
        case BRAP_CapInputPort_eIntCapPort0:
            ui32LoopBackStreamId = 0;
            ui32FciStreamId = 10;
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32LoopBackStreamId = 1;
            ui32FciStreamId = 11;
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32LoopBackStreamId = 2;
            ui32FciStreamId = 12;
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32LoopBackStreamId = 3;
            ui32FciStreamId = 13;
            break;
#else
        case BRAP_CapInputPort_eIntCapPort0:
            ui32LoopBackStreamId = 0;
            ui32FciStreamId = 9;
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32LoopBackStreamId = 1;
            ui32FciStreamId = 10;
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32LoopBackStreamId = 2;
            ui32FciStreamId = 11;
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32LoopBackStreamId = 3;
            ui32FciStreamId = 12;
            break;
#endif
#if (BRAP_7405_FAMILY != 1)            
        case BRAP_CapInputPort_eIntCapPort4:
            ui32LoopBackStreamId = 4;
            ui32FciStreamId = 13;
            break;
        case BRAP_CapInputPort_eIntCapPort5:
            ui32LoopBackStreamId = 5;
            ui32FciStreamId = 14;
            break;
        case BRAP_CapInputPort_eIntCapPort6:
            ui32LoopBackStreamId = 6;
            ui32FciStreamId = 15;
            break;
        case BRAP_CapInputPort_eIntCapPort7:
            ui32LoopBackStreamId = 7;
            ui32FciStreamId = 16;
            break;
#endif            
        default:
            BDBG_ASSERT (0);
    }

    if(BRAP_CapInputPort_eExtI2s0 != hCapPort->eCapPort)
    {
        ui32RegVal = BRAP_Read32(hRegister,(BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_BASE+(ui32LoopBackStreamId*4)));

        ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, INS_NOACK)) 
                 | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, INSERT_CTL))
                 | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, TMG_SRC_SEL)) 
                 | (BCHP_MASK (AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, IGNORE_FIRST_UNDERFLOW)));
        /* Program LOOPBACK0_INS, TMG_SRC_SEL, IGNORE_FIRST_UNDERFLOW0 */


#if (BRAP_7405_FAMILY != 1)
/*On 7405 platform, Enabling this leads to movement of DSTCH Fifo ptrs, 
even if data is not available. This leads initial zeros.*/
        ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, INS_NOACK, Enable);
#endif
        ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, INSERT_CTL, No_insert);
        ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, IGNORE_FIRST_UNDERFLOW, Ignore);
        switch(sParams.uiFsTmgSrc)
        {
            case 0:
                ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, 
                    TMG_SRC_SEL, Fs0);
                break;
            case 1:
                ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, 
                    TMG_SRC_SEL, Fs1);
                break;
            case 2:
                ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, 
                    TMG_SRC_SEL, Fs2);
                break;
            case 3:
                ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, 
                    TMG_SRC_SEL, Fs3);
                break;                    
            default:
                BDBG_ASSERT(0);                
        }/* switch uiFsTmgSrc */

        BRAP_Write32(hRegister,(BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_BASE+(ui32LoopBackStreamId*4)),ui32RegVal);

        /* Program the Input FCI Id */
        ui32RegVal = BRAP_Read32(hRegister , 
            (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (ui32FciStreamId*4)));

        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID));
        ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID,sParams.uiInputFciId);

        BRAP_Write32(hRegister , (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + 
                                (ui32FciStreamId*4)),ui32RegVal);


        /* Program the clock that drives the external capture port */
        ret = BRAP_CAPPORT_P_ProgramCaptureClock(hCapPort, 
                                       sParams.uiFsTmgSrc, 
                                       sParams.ePll, 
                                       sParams.eSamplingRate);
        if(BERR_SUCCESS != ret)
        {
            return BERR_TRACE(ret);
        }

    }

    BDBG_LEAVE (BRAP_CAPPORT_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
    Enables a Capture Port.

Description:
    This function should be called for enabling the capture port to get the 
    date from its input.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CAPPORT_P_Stop, BRAP_CAPPORT_P_HWConfig
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Start ( 
	BRAP_CAPPORT_P_Handle       hCapPort,   /* [in] Capture Port Handle */
    const BRAP_CAPPORT_P_Params *pParams    /* [in] Pointer to start parameters 
                                               for this Capture Port */ 
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0, ui32StreamId = 0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER (BRAP_CAPPORT_P_Start);
    BDBG_ASSERT (hCapPort);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hCapPort->hFmm) == false)
    {   
        /* If not in WatchDog recovery */  
        BDBG_ASSERT (pParams);

        if (hCapPort->uiStartCnt > 0)
        {
            /* This Outport was previoulsy started by another audio channel.
               So this time, just increment the start count. Dont change the 
               settings etc. */
            /* Also, start count cannot exceed the open count!! */
            if (hCapPort->uiStartCnt >= hCapPort->uiOpenCnt)
            {
                BDBG_ERR (("BRAP_CAPPORT_P_Start: Cannot start capport %d. "
                        "Start Count (%d) >= Open Count (%d)",
                        hCapPort->eCapPort, hCapPort->uiStartCnt, hCapPort->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
        
            hCapPort->uiStartCnt ++;
            BDBG_MSG (("BRAP_CAPPORT_P_Start: CapPort %d was already started." 
                        "New start count = %d", 
                        hCapPort->eCapPort, hCapPort->uiStartCnt));

            BDBG_LEAVE (BRAP_CAPPORT_P_Start);
            return ret;
        }
        
        /* Store the start parameters inside the handle */
        hCapPort->sParams = *pParams;

        BDBG_MSG (("BRAP_CAPPORT_P_Start(): hCapPort=0x%x, Type=%d, \n"
				"hCapPort->sParams.sExtParams.bLsbAtLRClk=%d\n"
				"hCapPort->sParams.sExtParams.bAlignedToLRClk=%d\n"
                "hCapPort->sParams.sExtParams.bLRClkPolarity=%d\n"
                "hCapPort->sParams.uiInputFciId =%d\n"
                "hCapPort->sParams.eInputBitsPerSample=%d\n"
                "hCapPort->sParams.uiFsTmgSrc = %d \n"
                "hCapPort->sParams.ePll = %d\n"
                "hCapPort->sParams.eSamplingRate = %d \n",
                hCapPort, hCapPort->eCapPort,
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLsbAtLRClk, 
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk, 
				hCapPort->hFmm->hRap->sInputSettings[hCapPort->eCapPort].sCapPortParams.uCapPortParams.sInputI2sParams.bLRClkPolarity, 
				hCapPort->sParams.uiInputFciId, 				
				hCapPort->sParams.eInputBitsPerSample,
                hCapPort->sParams.uiFsTmgSrc, 
                hCapPort->sParams.ePll,
                hCapPort->sParams.eSamplingRate));
    }   /* End: If not in WatchDog recovery */
   
    hRegister = hCapPort->hRegister;

    /* Configure Capture Port Hardware */
    BRAP_CAPPORT_P_HWConfig (hCapPort);
    
	/* Enable Capture Port */
    switch(hCapPort->eCapPort)
    {
        case BRAP_CapInputPort_eExtI2s0:
#if (BRAP_7405_FAMILY == 1)
            ui32StreamId = 4;
#else
            ui32StreamId = 8;
#endif
            break;
        case BRAP_CapInputPort_eIntCapPort0:
            ui32StreamId = 0;
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32StreamId = 1;
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32StreamId = 2;
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32StreamId = 3;
            break;
#if (BRAP_7405_FAMILY != 1)
        case BRAP_CapInputPort_eIntCapPort4:
            ui32StreamId = 4;
            break;
        case BRAP_CapInputPort_eIntCapPort5:
            ui32StreamId = 5;
            break;
        case BRAP_CapInputPort_eIntCapPort6:
            ui32StreamId = 6;
            break;
        case BRAP_CapInputPort_eIntCapPort7:
            ui32StreamId = 7;
            break;
#endif            
        default:
            BDBG_ASSERT(0);
    }

    ui32RegVal = BRAP_Read32 (hRegister, 
              (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+(ui32StreamId*4)));
    
    ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,IGNORE_FIRST_OVERFLOW))
                    |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,ENA))
                    |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP)));

    ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFGi,
                                    IGNORE_FIRST_OVERFLOW,Ignore);

    /* Grouping of cap ports */
    ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP,
                                                          pParams->uiGrpId); 
    ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFGi,ENA,Enable);
	BRAP_Write32 (hRegister, (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+
                                            (ui32StreamId*4)), ui32RegVal);

    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hCapPort->hFmm) == false)
    {   
        /* Set the start count to 1 */
        hCapPort->uiStartCnt = 1;
    }

    BDBG_LEAVE (BRAP_CAPPORT_P_Start);
    return ret;
}

/***************************************************************************
Summary:
    Disables the Capture port.

Description:
    This function disables the capture port.
Returns:
    BERR_Success else Error
See Also:
    BRAP_CAPPORT_P_Start
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Stop ( 
    BRAP_CAPPORT_P_Handle   hCapPort    /* [in] Capture Port Handle */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0, ui32StreamId =0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER(BRAP_CAPPORT_P_Stop);
    BDBG_ASSERT (hCapPort);

    BDBG_MSG (("BRAP_CAPPORT_P_Stop(): hCapPort=0x%x, eCapPort=%d ", 
        hCapPort, hCapPort->eCapPort));

    if(0 == hCapPort->uiStartCnt)
    { 
        BDBG_MSG (("BRAP_CAPPORT_P_Stop: CapPort %d has NO active"
            "audio channels. Ignoring this call to _Stop()!",
            hCapPort->eCapPort));
        BDBG_LEAVE (BRAP_CAPPORT_P_Stop);
        return ret;
    }
    
    hCapPort->uiStartCnt --;
    BDBG_MSG (("BRAP_CAPPORT_P_Stop: CapPort %d new start count =%d",
        hCapPort->eCapPort, hCapPort->uiStartCnt));
    if (hCapPort->uiStartCnt != 0)
    {
        BDBG_MSG(("BRAP_CAPPORT_P_Stop: So do nothing!!", 
            hCapPort->eCapPort, hCapPort->uiStartCnt));        
        BDBG_LEAVE (BRAP_OP_P_Stop);
        return ret;
    }
    hRegister = hCapPort->hRegister;

    /* Disable Capture Port */
	switch(hCapPort->eCapPort)
    {
        case BRAP_CapInputPort_eExtI2s0:
#if (BRAP_7405_FAMILY == 1)
            ui32StreamId = 4;
#else
            ui32StreamId = 8;
#endif
            break;
        case BRAP_CapInputPort_eIntCapPort0:
            ui32StreamId = 0;
            break;
        case BRAP_CapInputPort_eIntCapPort1:
            ui32StreamId = 1;
            break;
        case BRAP_CapInputPort_eIntCapPort2:
            ui32StreamId = 2;
            break;
        case BRAP_CapInputPort_eIntCapPort3:
            ui32StreamId = 3;
            break;
#if (BRAP_7405_FAMILY != 1)            
        case BRAP_CapInputPort_eIntCapPort4:
            ui32StreamId = 4;
            break;
        case BRAP_CapInputPort_eIntCapPort5:
            ui32StreamId = 5;
            break;
        case BRAP_CapInputPort_eIntCapPort6:
            ui32StreamId = 6;
            break;
        case BRAP_CapInputPort_eIntCapPort7:
            ui32StreamId = 7;
            break;
#endif            
        default:
            BDBG_ASSERT(0);
    }

    ui32RegVal = BRAP_Read32 (hRegister, 
              (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+(ui32StreamId*4)));
    
    ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,ENA))
                    |(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP)));

    /* Grouping of cap ports */
    ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP,ui32StreamId); 
    ui32RegVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_CAP_CFGi,ENA,Disable);
    
	BRAP_Write32 (hRegister, (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+
                                            (ui32StreamId*4)), ui32RegVal);

    BDBG_LEAVE (BRAP_CAPPORT_P_Stop);
    return ret;
}

#endif

/***************************************************************************
Summary: Refer to the Prototype
***************************************************************************/
static BERR_Code BRAP_CAPPORT_P_ProgramCaptureClock(
    BRAP_CAPPORT_P_Handle   hCapPort,      /* [in] Capture Port Handle */
    unsigned int            uiFsTmgSrcId,   /* [in] FS Timing Source Id */
    BRAP_OP_Pll 			ePll,           /* [in] PLL to be associated */
    BAVC_AudioSamplingRate 	eSamplingRate   /* [in] Sampling rate of the data at 
                                               CapPort or FCI sinker driven by 
                                               this FS timing source */
)
{
    BERR_Code       ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_CAPPORT_P_ProgramCaptureClock);
    BDBG_ASSERT(hCapPort);
    
    /* Program the MCLK and 1x, 2x or 4x for FS timing source */
    ret = BRAP_P_ConfigureFsTimingSource(hCapPort->hRegister, 
                                   uiFsTmgSrcId, 
                                   ePll, 
                                   eSamplingRate);
    if(BERR_SUCCESS != ret)
    {
        return BERR_TRACE(ret);
    }

    /* Pass the Capture Sample Rate Info to the upper SW layer */
    if(true == BRAP_P_IsInternalCapPort(hCapPort->eCapPort))
    {
        bool    bFound=false;
        unsigned uiAssocId=0, uiDstIndex=0;
        BRAP_OutputChannelPair  eChPair = BRAP_OutputChannelPair_eMax;
        
        for(uiAssocId=0; uiAssocId<BRAP_MAX_ASSOCIATED_GROUPS; uiAssocId++)
        {
            for(uiDstIndex=0; uiDstIndex < BRAP_P_MAX_DST_PER_RAPCH; uiDstIndex++)
            {
                if(BRAP_AudioDst_eRingBuffer == hCapPort->hFmm->hRap->sAssociatedCh[uiAssocId].sDstDetails[uiDstIndex].sExtDstDetails.eAudioDst)
                {
                    for(eChPair = 0; eChPair < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; eChPair++)
                    {
                        if(hCapPort->eCapPort == hCapPort->hFmm->hRap->sAssociatedCh[uiAssocId].sDstDetails[uiDstIndex].sExtDstDetails.uDstDetails.sRBufDetails.eCapPort[eChPair])
                        {
                            bFound = true;
                            if(hCapPort->hFmm->hRap->sAssociatedCh[uiAssocId].sDstDetails[uiDstIndex].bSampleRateChangeCallbackEnabled == true)
                            {
                                BKNI_EnterCriticalSection();
                                BRAP_P_DestinationSampleRateChange_isr(
                                    (void *)(&(hCapPort->hFmm->hRap->sAssociatedCh[uiAssocId].sDstDetails[uiDstIndex]))
                                    ,(unsigned int)eSamplingRate);
                                BKNI_LeaveCriticalSection();
                            }
                            break;                            
                        }
                    }
                }
                if(bFound == true)
                    break;
            }
            if(bFound == true)
                break;            
        }
    }
    
    BDBG_LEAVE (BRAP_CAPPORT_P_ProgramCaptureClock);
    return ret;    
}

/* End of File */
