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
*      Module name: FMM
*      This file contains the implementation of all PIs for the top level
*      FMM abstraction. PIs for all submodules like the RBUF, MIXER etc 
*      are in separate files.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/


#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_fmm);


static void BRAP_FMM_P_RBufRegInit (BRAP_FMM_P_Handle hFmm);
static void BRAP_FMM_P_DpRegInit (BRAP_FMM_P_Handle hFmm);
static void BRAP_FMM_P_MsRegInit (BRAP_FMM_P_Handle hFmm);
static void BRAP_FMM_P_HiFiDacInit (BRAP_FMM_P_Handle hFmm);


static BERR_Code BRAP_FMM_P_I2sMultiInit(BRAP_FMM_P_Handle hFmm);

#if (BRAP_7550_FAMILY != 1)
static BERR_Code BRAP_FMM_P_SrcInit (BRAP_FMM_P_Handle hFmm);

/*#if (((BCHP_CHIP == 7440)&&(BCHP_VER != BCHP_VER_A0)) )*/
#if (BRAP_7405_FAMILY == 1)
#define BRAP_P_MAX_CAPPORTS     5

static BERR_Code BRAP_FMM_P_CapPortInit(BRAP_FMM_P_Handle hFmm);
#endif
#endif

/* The MicroSequencer ie SPDIF Formatter Code */
extern uint32_t BRAP_SPDIFFM_P_fw_array[];

/* The SRC 2X and 4X Coefficients */
extern uint32_t BRAP_SRC_P_2x_Coeff_Array[];
extern uint32_t BRAP_SRC_P_4x_Coeff_Array[];
extern uint32_t BRAP_MIXER_P_Soft_Coeff_Array[];


BERR_Code BRAP_FMM_P_Open (
    BRAP_Handle					hRap,        /* [in] Audio Device Handle */ 
    BRAP_FMM_P_Handle *         phFmm,        /* [out] Pointer to FMM Handle*/
    unsigned int                uiFmmIndex,  /* [in] FMM Index */
    const BRAP_FMM_P_Settings * pSettings   /* [in] FMM Settings */
)
{
    
    BERR_Code  ret = BERR_SUCCESS;
    uint32_t  reg;
    BRAP_FMM_P_Handle  hFmm;
    uint32_t  i=0;
    uint32_t  j=0;


    BDBG_ENTER (BRAP_FMM_P_Open);

    /* 1. Check all input parameters to the function. Return error if
      - Audio Manager handle is NULL
      - Given index exceeds maximum no. of FMMs
      - Pointer to Settings structure is NULL
      - Pointer to FMM handle is NULL     */
    BDBG_ASSERT (hRap);
    BDBG_ASSERT (phFmm);
    BSTD_UNUSED (pSettings);
	
    if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == false)
    {
        BDBG_ASSERT (pSettings);
    }

    if ( uiFmmIndex > (BRAP_RM_P_MAX_FMMS -1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }      

    if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == false)
    {   /* If not in WatchDog recovery */

        if (hRap->hFmm[uiFmmIndex] !=NULL )
        {
            BDBG_ERR(("BRAP_FMM_P_Open: FMM %d is already open", uiFmmIndex));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* 2. Allocate memory for the FMM handle. Fill in parameters in the 
           FMM handle. */

        /* Allocate FMM handle */
        hFmm = (BRAP_FMM_P_Handle) BKNI_Malloc (sizeof(BRAP_FMM_P_Object));
        if (hFmm == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }

        /* Clear the handle memory*/
        BKNI_Memset (hFmm, 0, sizeof(BRAP_FMM_P_Object));

        /* Initialise known elements in FMM handle */
        hFmm->hChip = hRap->hChip;
        hFmm->hRegister = hRap->hRegister;
        hFmm->hHeap = hRap->hHeap;
        hFmm->hInt = hRap->hInt;
        hFmm->hRap = hRap;
        hFmm->uiIndex = uiFmmIndex;
        hFmm->ui32Offset = 0;
        for ( i = 0; i < BRAP_FMM_P_MAX_EXT_MCLK; i++ )
        {
        	hFmm->sExtMClkSettings[i].ePll = BRAP_OP_Pll_ePll0;
            hFmm->sExtMClkSettings[i].eMClockRate = BRAP_OP_MClkRate_e256Fs;
        }


        /* For 7440 B0 based chips */
    for(i=0; i<BRAP_RM_P_MAX_DP_BLCK;i++)
    {
        for(j=0;j<BRAP_RM_P_MAX_MIXER_PB_FCI_ID;j++)
        {
            hFmm->ui32MixerPbFciId[i][j] = 0xFFFFFFFF;
            hFmm->ui32MixerPbFciIdCount[i][j] = 0;
        }
    }


    } /* End: If not in WatchDog recovery */
    else
    {
        hFmm = *phFmm;
    }
    
    /* Copy the FMM Settings Structure */
    /* The FMM Settings Structure is currently empty */
  
    /* 3. Initialise certain control registers for FMM and various submodules*/

    BDBG_MSG(("Resetting FMM bus logic"));
#if 1
    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
        reg = 0;

    /*Watchdog should not reset Plls because it is getting used by Smart card module*/
        if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == true)
        {
             reg |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) ;
        }
    
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);
#else
    if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == false)
    {
        BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);
    }
#endif    
    /* UnReset Buffer Block, Data Path and MicroSequencer */
    reg = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET);
    
    reg |=  (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_TOP_LOGIC_B, Inactive)) |
#if ( BRAP_P_HAS_META_BUFFER == 1 )
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MB_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MB_REGS_B, Inactive)) |
#endif
#if ( BRAP_P_HAS_SPDIF_RECIEVER == 1 )
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_REGS_B, Inactive)) |
#endif
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive)) |
#if (BRAP_7550_FAMILY != 1)            
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_REGS_B, Inactive)) |
#endif            
#if (BRAP_3548_FAMILY == 1)
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_ADC_CIC_REGS_B, Inactive)) |
#endif
             (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) |

            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_REGS_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_REGS_B, Inactive));

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);

    /* Initialize Ring buffers Registers */
    BDBG_MSG(("Clearing ringbuffer registers"));
    BRAP_FMM_P_RBufRegInit (hFmm);

    /* Initialize Data Path Registers */
    BDBG_MSG(("Clearing DP registers"));
    BRAP_FMM_P_DpRegInit (hFmm);
    
    /* Initialize Micro Sequencer Registers */
    BDBG_MSG(("calling BRAP_FMM_P_MsRegInit"));
    BRAP_FMM_P_MsRegInit (hFmm);

    /* Initialize HiFiDACs Registers */
    BDBG_MSG(("calling BRAP_FMM_P_HiFiDacInit"));
    BRAP_FMM_P_HiFiDacInit (hFmm);
    /* Initialize I2sMulti Registers */
    BDBG_MSG(("calling BRAP_FMM_P_I2sMultiInit"));
    BRAP_FMM_P_I2sMultiInit (hFmm);

#if(BRAP_7550_FAMILY != 1)
#if(BRAP_3548_FAMILY != 1)
    BRAP_FMM_P_CapPortInit(hFmm);
#endif 

    /* Intialize SRC */
    BDBG_MSG(("calling BRAP_FMM_P_SrcInit"));
    ret = BRAP_FMM_P_SrcInit(hFmm);
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("BRAP_FMM_P_Open:SRC Init Failed"));
        BKNI_Free(hFmm);
        return ret;
    }
#endif
#if(BRAP_3548_FAMILY != 1)
    for(i = 0 ; i < BRAP_RM_P_MAX_IOP_FCI_CFG; i++)
    {
        reg = BRAP_Read32(hRap->hRegister , 
                    (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (i*4)));
        
        reg &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID));
        reg |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID,0x3FF);
        
        BRAP_Write32(hRap->hRegister , (BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + 
                                (i*4)),reg);
    }
#endif    


    if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == false)
    { /* If not in WatchDog recovery */    
        /* 4. Store and return the FMM handle */
        hRap->hFmm[uiFmmIndex] = hFmm;
        *phFmm = hFmm;
    } /* End: If not in WatchDog recovery */

    BDBG_LEAVE (BRAP_FMM_P_Open);
    return ret;
}

BERR_Code BRAP_FMM_P_Close (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i = 0;


    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_Close);

    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);
    
    /* Clear all referrences to this FMM */ 
    for(i=0; i < BRAP_RM_P_MAX_SRC_BLCK; i++)
    {
        if(NULL != hFmm->pSrcMemLocationInfo[i])
        {
            BKNI_Free(hFmm->pSrcMemLocationInfo[i]);
        }
    }


    hFmm->hRap->hFmm[hFmm->uiIndex] = NULL;

    
    /* Free the FMM Handle memory*/
    BKNI_Free (hFmm); 
                 
    BDBG_LEAVE (BRAP_FMM_P_Close);
    return ret;
}




/***************************************************************************
Summary:
    Initialize the Ring buffer and DMA engine

Description:
    

Returns:
    Nothing

See Also:
    
**************************************************************************/
static void 
BRAP_FMM_P_RBufRegInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
    int i;
    unsigned int uiSrcChId = 0;
    uint32_t ui32Offset = 0;


    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_RBufRegInit);

    /* Clear all ring buffer register RAM for safe operations.
       From AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR 
       To AUD_FMM_BF_CTRL_FMM_BF_RESERVE_ECO_25 */
#if(BRAP_7550_FAMILY == 1)
    for (i=0;i<(BCHP_AUD_FMM_BF_CTRL_RINGBUF_17_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#elif ((BRAP_7405_FAMILY == 1) || (BRAP_3548_FAMILY == 1))
    for (i=0;i<(BCHP_AUD_FMM_BF_CTRL_RINGBUF_23_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#endif
    {
        BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + (i*4), 0);
    }
    /* No grouping */
    for(uiSrcChId = 0; uiSrcChId < BRAP_RM_P_MAX_SRC_CHANNELS; uiSrcChId++)
    {
        ui32Offset = uiSrcChId * BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_ELEMENT_SIZE/8;
        BRAP_Write32(hFmm->hRegister,
                     BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + ui32Offset,
                     uiSrcChId);
    }


    BDBG_LEAVE (BRAP_FMM_P_RBufRegInit);
    return;
}

/***************************************************************************
Summary:
    Initialize the data path block.

Description:
    

Returns:
    BERR_SUCCESS - If successful

See Also:
    
**************************************************************************/
static void 
BRAP_FMM_P_DpRegInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
    unsigned int uiMixerId = 0, i = 0;
    uint32_t ui32MixerOffset = 0, ui32RegVal = 0;

    /* Initialize the DP RAM to all 0. Note that there is a address discontinuity between
         registers BCHP_AUD_FMM_DP_CTRL0_DP_SM and 
         BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE. Hence this
         initialization is splitted into two for loops. */
    for(i=0; 
        i < (BCHP_AUD_FMM_DP_CTRL0_DP_SM - 
            BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA + 4); 
        i=i+4)
    {
        BRAP_Write32(hFmm->hRegister,
            (BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA + i), 0);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
        BRAP_Write32(hFmm->hRegister,
            (BCHP_AUD_FMM_DP_CTRL1_MIXER_OUTPUT_ENA + i), 0);
#endif
    }/* for i */
        
    for(i=(BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE -
		BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA ); 
        i < (BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_1_25 - 
            BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA + 4); 
        i=i+4)
    {
        BRAP_Write32(hFmm->hRegister,
            (BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA + i), 0);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
        BRAP_Write32(hFmm->hRegister,
            (BCHP_AUD_FMM_DP_CTRL1_MIXER_OUTPUT_ENA + i), 0);
#endif
    }/* for i */
	
    /* No grouping */
    for(uiMixerId=0, ui32RegVal=0; 
        uiMixerId < BRAP_RM_P_MAX_MIXER_PER_DP_BLCK; 
        uiMixerId++, ui32RegVal=0)
    {
        ui32MixerOffset = (BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT10_CONFIG -
                           BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG ) 
                           * uiMixerId;

        ui32RegVal = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, 
                                    MIXER_GROUP_BEGIN, uiMixerId);  
        BRAP_Write32 (hFmm->hRegister, 
                      BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG + ui32MixerOffset, 
                      ui32RegVal);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
        BRAP_Write32 (hFmm->hRegister, 
                      BCHP_AUD_FMM_DP_CTRL1_MIXER0_CONFIG + ui32MixerOffset, 
                      ui32RegVal);
#endif
    }/* for uiMixerId */

    /* Program : 
       AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEP[0..255]: COEFFICIENT_RAMP 
       AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEP[0..255]: COEFFICIENT_RAMP 
       to 0x00100000 */
    for(i=0; i<=BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_END; i++)
    {
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            hFmm->hRap->uiScalingCoefRampStepSize);
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            hFmm->hRap->uiScalingCoefRampStepSize);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)            
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_PING_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            0x00100000);
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_PONG_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            0x00100000);
#endif
    }

#if (BRAP_7405_FAMILY ==1)
    ui32RegVal = BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, 0x0);
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, ui32RegVal);
#else
    /* AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP : VOLUME_RAMP_STEP */
    ui32RegVal = BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, 
                                    BRAP_P_DEFAULT_RAMP_STEP_SIZE);
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, ui32RegVal);
#endif    

    /* Program Mixer Soft Coefficients */
    for (i=0;i<=BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_END;i++)
    {
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_BASE + (4*i),
            BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)            
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFCi_ARRAY_BASE + (4*i),
            BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    }

    /* Program other soft coefficients */
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFA,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFA,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFB,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFB,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFD,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFD,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFE,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFE,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFF,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFF,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_0_5,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_0_5,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_0_8,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_0_8,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_1_25,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_RM_P_MAX_DP_BLCK == 2)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_1_25,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif

    /* Commenting out DP_PACING_FIX enable code, as BD 96kHz streams were failing with this change */
    return;
}


/***************************************************************************
Summary:
    Initializes Microsequencer Block 

Description:
    This function initializes the Microsequencer F/W registers, downloads 
    the microcode and does other necessary initialization of the 
    micro-sequencer block. 

Returns:
    Nothing

See Also:
    
    
**************************************************************************/ 
static void 
BRAP_FMM_P_MsRegInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{ 
    uint32_t i = 0;
    uint32_t ui32RegValue = 0;
    uint32_t ui32MsFwRegs = 0;


#if (BRAP_7405_FAMILY == 1)&&(BRAP_7550_FAMILY != 1)
    unsigned int reg=0;
#endif

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_MsRegInit);
  

    /* 1. Initialize the Firmware registers to 0 */ 

    /* There are 2 discontinuous register sets */
    /* a) starts at BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 */
    ui32MsFwRegs = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                       (BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END + 1)*4- 
                   BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;
    for ( i = 0; i< ui32MsFwRegs; i+=4 )
    {
        BRAP_Write32 (hFmm->hRegister, 
                        BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + i,  
                        0);
    }

    /* b) starts at BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE */
    for ( i = 0; i< BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++)
    {
        BRAP_Write32 (hFmm->hRegister, 
                        BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + i*4,  
                        0);
    }
    if (BRAP_P_GetWatchdogRecoveryFlag(hFmm->hRap) == false)
    {    
#if (BRAP_7405_FAMILY == 1)&&(BRAP_7550_FAMILY != 1)
    reg = (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_0));            
    BRAP_Write32_isr(hFmm->hRegister, BCHP_AUD_FMM_PLL0_CONTROL, reg);
#if (BRAP_PLL1_SUPPORTED == 1)   
    BRAP_Write32_isr(hFmm->hRegister, BCHP_AUD_FMM_PLL1_CONTROL, reg);    
#endif
#endif    
    }


    /* 2. Download the firmware */

    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to  3 */
    ui32RegValue = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);    
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL));
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, ui32RegValue);
    
    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 1 */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 1);
    
    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 2 
             to enable internal access to instruction memory */
    ui32RegValue = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);    
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL));
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Instruction_memory));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, ui32RegValue);
    
    /* Copy the microcode to the F/W instruction memory */
    for (i=0; i <= BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++ )
    {
        ui32RegValue = BRAP_SPDIFFM_P_fw_array[i];
        BRAP_Write32 (hFmm->hRegister, 
                       ( BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE 
                       + BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_START
                       + (i*4)), 
                      ui32RegValue);

    }

    /* Now Reset Pico */
    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to  3 */
    ui32RegValue = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);    
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL));
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, ui32RegValue);
    
    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 0 */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 0);
    
    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 0 
             to enable internal access to block external access */
    ui32RegValue = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);    
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL));
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, No_external_access));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, ui32RegValue);

#if (BRAP_7405_FAMILY == 1)
/*Initializing cBits*/
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE,  0x4);
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 4, 0x200);
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BRAP_SPDIFFM_CBIT_BUFFER_SIZE *4,   0x4);
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BRAP_SPDIFFM_CBIT_BUFFER_SIZE * 4 + 4, 0x200);        

    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BRAP_SPDIFFM_CBIT_BUFFER_SIZE *4,   0x4);
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BRAP_SPDIFFM_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BRAP_SPDIFFM_CBIT_BUFFER_SIZE *4,   0x4);
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BRAP_SPDIFFM_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        

    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0,   0x50);
    BRAP_Write32 (hFmm->hRegister,  BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1,   0x68);

    /*Initializing ramp amount*/
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0,  0x0);           
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_1,  0x0);           

        /*Initializing Stream CTRL*/
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 0x112);                   
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 0x112);       

        /*Initializing burst*/
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0, 0x0);       
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_BURST_1, 0x0);       

        /*Initializing Bypass and Enable reg.*/
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, 0x0);       

BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0 , 0x10);
        
#endif

    BDBG_LEAVE (BRAP_FMM_P_MsRegInit);
    return;
}
#if(BCHP_CHIP != 7601)
static void 
BRAP_FMM_P_HiFiDacInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_HiFiDacInit);
    /* Reset the HiFiDAC 0 by writing all 1's to HIFIDAC_CTRL0_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL0_RESET, 1);

    /* Bring HiFiDAC 0 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL0_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL0_RESET, 0);

#ifdef BCHP_HIFIDAC_CTRL1_REVISION 
    /* Reset the HiFiDAC 1 by writing all 1's to HIFIDAC_CTRL1_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL1_RESET, 1);

    /* Bring HiFiDAC 1 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL1_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL1_RESET, 0);
#endif 

#ifdef BCHP_HIFIDAC_CTRL2_REVISION 
    /* Reset the HiFiDAC 2 by writing all 1's to HIFIDAC_CTRL2_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL2_RESET, 1);

    /* Bring HiFiDAC 2 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL2_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL2_RESET, 0);
#endif

    BDBG_LEAVE (BRAP_FMM_P_HiFiDacInit);
    return;
}
#endif

bool BRAP_FMM_P_GetWatchdogRecoveryFlag (
        BRAP_FMM_P_Handle hFmm       /* [in] FMM Device Handle */
)
{
	return BRAP_P_GetWatchdogRecoveryFlag (hFmm->hRap);
}

void BRAP_FMM_P_ResetHardware (
	BRAP_FMM_P_Handle hFmm /* [in] FMM Device Handle */
)
{
    uint32_t reg = 0;

#if 1
    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
        reg = 0;

 /*Watchdog should not reset Plls because it is getting used by Smart card module*/
         reg |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
        (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) ;
 
        BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);
#else
    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);
#endif        

    reg = 0;
    reg |=  (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_TOP_LOGIC_B, Inactive)) |
#if ( BRAP_P_HAS_META_BUFFER == 1 )
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MB_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MB_REGS_B, Inactive)) |
#endif
#if ( BRAP_P_HAS_SPDIF_RECIEVER == 1 )
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_REGS_B, Inactive)) |
#endif
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive)) |
#if (BRAP_7550_FAMILY != 1)            
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_REGS_B, Inactive)) |
#endif            
#if (BRAP_3548_FAMILY == 1)
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_ADC_CIC_REGS_B, Inactive)) |
#endif
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_REGS_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_REGS_B, Inactive));

    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);    
}


static BERR_Code
BRAP_FMM_P_I2sMultiInit(
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
    )
{
    BERR_Code   ret = BERR_SUCCESS;    
    BDBG_ASSERT(hFmm);
    BDBG_ENTER(BRAP_FMM_P_I2sMultiInit);
#if ((BRAP_3548_FAMILY == 1))
    /* Clear the OP_CTRL_I2SM_GROUPING */
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_I2SM_GROUPING, 0);
#else

#if (BRAP_7420_FAMILY == 1)|| (BRAP_7550_FAMILY == 1)
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_MAI_MULTI_GROUPING, 0);
#else
    /* Clear the OP_CTRL_I2SM_GROUPING */
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_I2SM0_GROUPING, 0);
#endif
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE, 0x100);
#endif

    BDBG_LEAVE(BRAP_FMM_P_I2sMultiInit);
    return ret;
}

#if ((BRAP_3548_FAMILY != 1)&&(BRAP_7550_FAMILY != 1))
static BERR_Code BRAP_FMM_P_CapPortInit(BRAP_FMM_P_Handle hFmm)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    unsigned int    uiCapPort = 0;
    
    BDBG_ASSERT(hFmm);
    BDBG_ENTER(BRAP_FMM_P_I2sMultiInit);

    for (uiCapPort =0 ; uiCapPort < BRAP_P_MAX_CAPPORTS ; uiCapPort++)
    {
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP));
        ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP,uiCapPort);
        
        BRAP_Write32 (hFmm->hRegister, (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+
                                            (uiCapPort*4)), ui32RegVal);
    }
    return ret;
}
#endif 

#if (BRAP_7550_FAMILY != 1)
static BERR_Code
BRAP_FMM_P_SrcInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
    BERR_Code                       ret = BERR_SUCCESS;    
    BRAP_SRC_P_SramMemLocationInfo  *pMemLocInfo_0 = NULL;
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )	
    BRAP_SRC_P_SramMemLocationInfo  *pMemLocInfo_1 = NULL;
#endif
    unsigned int                    uiSizeOfArray = 0;
    unsigned int                    uiBaseAddr = 0;
    unsigned int                    uiBlkId = 0;
    unsigned int                    uiSrcId = 0;
    unsigned int                    uiBlkOffset = 0;
    uint32_t                        ui32RegVal = 0;
    unsigned int                    i = 0;

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_SrcInit);
    
    /* For SRC BLK 0 , find the size of possible array */
    uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);

    if (BRAP_P_GetWatchdogRecoveryFlag(hFmm->hRap) == false)
    {
    /* Allocate the Memory to the pointer to the array */
    pMemLocInfo_0 = (BRAP_SRC_P_SramMemLocationInfo *)BKNI_Malloc
                        (sizeof(BRAP_SRC_P_SramMemLocationInfo)* uiSizeOfArray);
        if(NULL == pMemLocInfo_0)
        {
            BDBG_ERR(("BRAP_FMM_P_SrcInit: Could not allocate Memory"));
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_Memset (pMemLocInfo_0, 0, sizeof(BRAP_SRC_P_SramMemLocationInfo)* uiSizeOfArray);        
    }
    else
    {
        pMemLocInfo_0 = hFmm->pSrcMemLocationInfo[0];
    }

    /* Intialize the Array for multiple of 128 Words */
    for (i=0;
         i<(BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0/BRAP_P_SRAM_MEMORY_REQUIRED_UPx);
         i++)
    {
        pMemLocInfo_0[i].bAllocated = false;
        pMemLocInfo_0[i].uiBaseAddr= uiBaseAddr;
        pMemLocInfo_0[i].uiSize = BRAP_P_SRAM_MEMORY_REQUIRED_UPx;

        uiBaseAddr += BRAP_P_SRAM_MEMORY_REQUIRED_UPx;
    }

    /* Intialize the Array for multiple of 4 Words for Linear Interpolation */
    for (;i<uiSizeOfArray;i++)
    {
        pMemLocInfo_0[i].bAllocated = false;
        pMemLocInfo_0[i].uiBaseAddr = uiBaseAddr;
        pMemLocInfo_0[i].uiSize = BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT;

        uiBaseAddr += BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT;
    }

    for(i = 0; i < 360; i++)
    {
        hFmm->bSrcCoeffUsed[0][i] = false;
    }
    
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )
    /* Reset the variables for SRC BLK 1*/
    uiSizeOfArray = 0;
    uiBaseAddr = 0;
    
    /* For SRC BLK 1 */
    uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);

    /* Allocate the Memory to the pointer to the array */
    if (BRAP_P_GetWatchdogRecoveryFlag(hFmm->hRap) == false)
    {
    pMemLocInfo_1 = (BRAP_SRC_P_SramMemLocationInfo *)BKNI_Malloc
                        (sizeof(BRAP_SRC_P_SramMemLocationInfo)* uiSizeOfArray);
        if(NULL == pMemLocInfo_1)
        {
            BDBG_ERR(("BRAP_FMM_P_SrcInit: Could not allocate Memory"));
            BKNI_Free(pMemLocInfo_0);
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_Memset (pMemLocInfo_1, 0, sizeof(BRAP_SRC_P_SramMemLocationInfo)* uiSizeOfArray);                
    }
    else
    {
        pMemLocInfo_1 = hFmm->pSrcMemLocationInfo[1];
    }

    /* Intialize the Array for multiple of 128 Words */
    for (i=0;
         i<(BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1/BRAP_P_SRAM_MEMORY_REQUIRED_UPx);
         i++)
    {
        pMemLocInfo_1[i].bAllocated = false;
        pMemLocInfo_1[i].uiBaseAddr = uiBaseAddr ;
        pMemLocInfo_1[i].uiSize = BRAP_P_SRAM_MEMORY_REQUIRED_UPx;

        uiBaseAddr += BRAP_P_SRAM_MEMORY_REQUIRED_UPx;
    }

    /* Intialize the Array for multiple of 4 Words for Linear Interpolation */
    for (;i<uiSizeOfArray;i++)
    {
        pMemLocInfo_1[i].bAllocated = false;
        pMemLocInfo_1[i].uiBaseAddr = uiBaseAddr ;
        pMemLocInfo_1[i].uiSize = BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT;

        uiBaseAddr += BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT;
    }

    for(i = 0; i < 360; i++)
    {
        hFmm->bSrcCoeffUsed[1][i] = false;
    }    
#endif

    /* Save the Array in hFmm Handle. Whoevr wants can use/update in hFmm */
    hFmm->pSrcMemLocationInfo[0]=pMemLocInfo_0;
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )
    hFmm->pSrcMemLocationInfo[1]=pMemLocInfo_1;
#endif

#if (BRAP_3548_FAMILY == 1) 
    /* SRC block for 3548 family has changed a lot compared to 3563 or 7405 family */
    for(uiBlkId = 0; uiBlkId < BRAP_RM_P_MAX_SRC_BLCK; uiBlkId++)
    {
        uiBlkOffset = (BCHP_AUD_FMM_SRC_CTRL1_RAMP_STEP - BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP) * uiBlkId;
        
        for(uiSrcId = 0, ui32RegVal = 0; uiSrcId < BRAP_RM_P_MAX_SRC_PER_SRC_BLCK; uiSrcId++, ui32RegVal = 0)        
        {
            ui32RegVal = (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP ,uiSrcId));
            BRAP_Write32(hFmm->hRegister, 
                            uiBlkOffset +
                            (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                            ((BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE>>3) * uiSrcId)),
                            ui32RegVal);

            /* Debug print */
            BDBG_MSG(("uiSrcId = %d uiBlkOffset = 0x%x addr[0x%x] -> 0x%x", uiSrcId, uiBlkOffset,
                            uiBlkOffset +
                            (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                            (4* uiSrcId)),
                            ui32RegVal));      

            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, FCI_ID ,uiSrcId));
            BRAP_Write32(hFmm->hRegister, 
                            uiBlkOffset +
                            (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                            ((BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE>>3) * uiSrcId)),
                            ui32RegVal);       

            BRAP_Write32 (hFmm->hRegister,
                            uiBlkOffset +
                            (BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE +
                            ((BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_ELEMENT_SIZE>>3) * uiSrcId)),
                            0);

            BRAP_Write32 (hFmm->hRegister,
                            uiBlkOffset +
                            (BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
                            ((BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_ELEMENT_SIZE>>3) * uiSrcId)),
                            0);            
        }

#if (BRAP_7405_FAMILY==1)
        BRAP_Write32(hFmm->hRegister, (BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset), 0x0);   
#else
        BRAP_Write32(hFmm->hRegister, (BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset),
                                        BRAP_P_DEFAULT_RAMP_STEP_SIZE);   
#endif        
        for (i=BCHP_AUD_FMM_SRC_CTRL0_RD_BANK_SEL; i<=BCHP_AUD_FMM_SRC_CTRL0_DIAG; i+=4)
        {
            BRAP_Write32(hFmm->hRegister, (i + uiBlkOffset), 0);   
        }

        /* 
            Filling FIR coefficients.
            In the 3563 and 7405 designs,  the 2X and 4X coefficients have 1.24 and 1.25 implicit formats respectively 
            We load them as 24bits. In the 3548 design, these coefficients need to be loaded as 2.26 format explicitly.
            Since we still use the same coefficients array, we have to do the following
            For 24bit 2X coefficients , shift left 2 position and fill bit 0 and 1 with zeros. Copy bit 25 to bit 26 and 27. Fill bit 28 through 31 with 0. 
            For 24bit 4X coefficients , shift left 1 position and fill bit 0 with 0 , Copy bit 24 to bit 25, 26 and 27. Fill bit 28 through31 with 0.
        */

        for (i=0; i<360; i++)
        {
            BRAP_Write32(hFmm->hRegister,
                                     uiBlkOffset +
                                     BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                                     0);            
        }
        if(0 == uiBlkId)
        {
            for(i=0; i<128 ;i++)
            {
                ui32RegVal = BRAP_SRC_P_4x_Coeff_Array[i];

                ui32RegVal <<= 1;            
                if (0 != (ui32RegVal & 0x1000000) ) 
                {
                    ui32RegVal |= 0x0E000000;
                }

                BRAP_Write32(hFmm->hRegister,
                             uiBlkOffset +
                             BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + /*(4*64)*/ + (i*4),
                             ui32RegVal);
            }   
            for (i=200; i<360; i++)
            {
                if (0 == (i%5))
                {
                    BRAP_Write32(hFmm->hRegister,
                                 uiBlkOffset +
                                 BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                                 0x4000000);
                    
                }
                else
                {
                    BRAP_Write32(hFmm->hRegister,
                                 uiBlkOffset +
                                 BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                                 0x0);
                }
            }
        }
        else
        {
            /* In SRC1 block entire coefficient memory is available for IIR filter */
            for (i=0; i<360; i++)
            {
                if (0 == (i%5))
                {
                    BRAP_Write32(hFmm->hRegister,
                                 uiBlkOffset +
                                 BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                                 0x4000000);
                    
                }
                else
                {
                    BRAP_Write32(hFmm->hRegister,
                                 uiBlkOffset +
                                 BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                                 0x0);
                }
            }        
        } /* if(0 == uiBlkId) */
    }

#else /* not 3548 family*/
    /* Initialize the SRC Reg to all 0. */
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, 0);

	/* No grouping */
    for(uiBlkId = 0; uiBlkId < BRAP_RM_P_MAX_SRC_BLCK; uiBlkId++)
    {
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )    
        uiBlkOffset = (BCHP_AUD_FMM_SRC_CTRL1_STRM_ENA - BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA) * uiBlkId;
#endif


        for(uiSrcId = 0, ui32RegVal = 0;
            uiSrcId < BRAP_RM_P_MAX_SRC_PER_SRC_BLCK;
            uiSrcId++, ui32RegVal = 0)
        {
            ui32RegVal = (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP ,uiSrcId));
            BRAP_Write32(hFmm->hRegister, 
                         uiBlkOffset +
                         (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                         ((BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE>>3) * uiSrcId)),
                         ui32RegVal);

            /* Debug print */
            BDBG_MSG(("uiSrcId = %d uiBlkOffset = 0x%x addr[0x%x] -> 0x%x", uiSrcId, uiBlkOffset,
                uiBlkOffset +
                 (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                 (4* uiSrcId)),
                 ui32RegVal));
        }/* uiSrcId */
    }/* uiBlkId */

    for (uiBlkId = 0; uiBlkId < BRAP_RM_P_MAX_SRC_BLCK; uiBlkId++)
    {
        uiBlkOffset = 0;
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )    
        uiBlkOffset = (BCHP_AUD_FMM_SRC_CTRL1_STRM_ENA - BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA) * uiBlkId;
#endif

        /* 0 init */
        for(i=0; 
            i <= (BCHP_AUD_FMM_SRC_CTRL0_IDMAPba - BCHP_AUD_FMM_SRC_CTRL0_MISC_CFG); 
            i=i+4)
        {
            BRAP_Write32(hFmm->hRegister,
                (BCHP_AUD_FMM_SRC_CTRL0_MISC_CFG + i + uiBlkOffset), 0);
        }/* for i */

        for(i=0; 
            i <= (BCHP_AUD_FMM_SRC_CTRL0_STRM_RESET - BCHP_AUD_FMM_SRC_CTRL0_DIAG); 
            i=i+4)
        {
            BRAP_Write32(hFmm->hRegister,
                (BCHP_AUD_FMM_SRC_CTRL0_DIAG + i + uiBlkOffset), 0);
        }/* for i */

        /* Set RAMP step to default value */
#if (BRAP_7405_FAMILY==1)
        BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset, 0x0);
#else
        BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset,
                                        BRAP_P_DEFAULT_RAMP_STEP_SIZE);  
#endif          


        /* Default init IDMAP registers */
        for(i=0; i < BRAP_RM_P_MAX_SRC_PER_SRC_BLCK; i++)
        {
            switch(i%2)
            {
            case 0:
                ui32RegVal = BRAP_Read32 (hFmm->hRegister, ((i/2)*4) + uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10);
                ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM0));
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM0, i));
                BRAP_Write32 (hFmm->hRegister, ((i/2)*4) + uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10,ui32RegVal);
                break;

            case 1:
                ui32RegVal = BRAP_Read32 (hFmm->hRegister, ((i/2)*4) + uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10);
                ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM1));
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM1, i));
                BRAP_Write32 (hFmm->hRegister, ((i/2)*4) + uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10,ui32RegVal);
                break;
            }
        }/* for i */

        /* For SRC Coeff Init */
        for(i=0 ; i<= BCHP_AUD_FMM_SRC_CTRL0_COEFF2X_i_ARRAY_END ;i++)
        {
            ui32RegVal = BRAP_SRC_P_2x_Coeff_Array[i];
            
            BRAP_Write32(hFmm->hRegister,
                         uiBlkOffset +
                         BCHP_AUD_FMM_SRC_CTRL0_COEFF2X_i_ARRAY_BASE + (i*4),
                         ui32RegVal);
        }

        for(i=0 ; i<= BCHP_AUD_FMM_SRC_CTRL0_COEFF4X_i_ARRAY_END ;i++)
        {
            ui32RegVal = BRAP_SRC_P_4x_Coeff_Array[i];
            
            BRAP_Write32(hFmm->hRegister,
                         uiBlkOffset +
                         BCHP_AUD_FMM_SRC_CTRL0_COEFF4X_i_ARRAY_BASE + (i*4),
                         ui32RegVal);
        }

        /* 0 Init */
        for(i=0; 
            i <= (BCHP_AUD_FMM_SRC_CTRL0_LI_DEN11 - BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + 4); 
            i=i+4)
        {
            BRAP_Write32(hFmm->hRegister,
                (BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + i + uiBlkOffset), 0);
        }/* for i */       
    }/* for uiBlkId */
#endif
    BDBG_LEAVE (BRAP_FMM_P_SrcInit);
    return ret;
}
#endif /*end of #if (BRAP_7550_FAMILY != 1)*/
/* End of File */
