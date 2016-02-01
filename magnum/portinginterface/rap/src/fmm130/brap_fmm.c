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
static void BRAP_FMM_P_ProgramDMASlot (BRAP_FMM_P_Handle hFmm);
static void BRAP_FMM_P_DpRegInit (BRAP_FMM_P_Handle hFmm);
static void BRAP_FMM_P_MsRegInit (BRAP_FMM_P_Handle hFmm);
#if(BCHP_CHIP != 7601)
static void BRAP_FMM_P_HiFiDacInit (BRAP_FMM_P_Handle hFmm);
#endif
#if (BRAP_7440_ARCH == 1)
static BERR_Code BRAP_FMM_P_I2sMultiInit(BRAP_FMM_P_Handle hFmm);
static BERR_Code BRAP_FMM_P_SrcInit (BRAP_FMM_P_Handle hFmm);

/*#if (((BCHP_CHIP == 7440)&&(BCHP_VER != BCHP_VER_A0)) && (BCHP_CHIP != 3563))*/
#if (((BCHP_CHIP == 7440)&&(BCHP_VER > BCHP_VER_A0)) || (BCHP_CHIP == 7601))
#define BRAP_P_MAX_CAPPORTS     9

static BERR_Code BRAP_FMM_P_CapPortInit(BRAP_FMM_P_Handle hFmm);
#elif (BRAP_7405_FAMILY == 1)
#define BRAP_P_MAX_CAPPORTS     5

static BERR_Code BRAP_FMM_P_CapPortInit(BRAP_FMM_P_Handle hFmm);
#endif 

#endif

#if (BRAP_7440_ARCH==0)
/* Maximum MS F/W register memory locations in 32 bit words */ 
#define BRAP_P_MAX_MS_FW_REGS_MEM    (BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE  \
                                      + BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END + 1 \
                                      - BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0)
#endif

/* The MicroSequencer ie SPDIF Formatter Code */
extern uint32_t BRAP_SPDIFFM_P_fw_array[];

#if (BRAP_7440_ARCH == 1)
/* The SRC 2X and 4X Coefficients */
extern uint32_t BRAP_SRC_P_2x_Coeff_Array[];
extern uint32_t BRAP_SRC_P_4x_Coeff_Array[];
extern uint32_t BRAP_MIXER_P_Soft_Coeff_Array[];
#endif

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

#if ((BRAP_7401_FAMILY == 1) || (BCHP_CHIP == 7400))	
    uint32_t  ui32Offset=0;
    BRAP_OP_Pll  ePll = 0x1;
    uint32_t ui32CBitData[48];

	uint32_t            	ui32CbitArrayOffset=0x0;
    unsigned int 			uiCnt = 0;

	uint32_t ui32RegVal=0 ,ui32RegVal1=0,ui32RegValue=0;
#endif
	
#ifndef BCHP_7411_VER		
    uint32_t  i=0;
#endif

#if (BRAP_7440_ARCH == 1)
#if ((!((BCHP_CHIP == 7440)&&(BCHP_VER == BCHP_VER_A0)))&&(BCHP_CHIP != 3563))
    uint32_t  j=0;
#endif
#endif

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
#ifndef BCHP_7411_VER			
        for ( i = 0; i < BRAP_FMM_P_MAX_EXT_MCLK; i++ )
        {
        	hFmm->sExtMClkSettings[i].ePll = BRAP_OP_Pll_ePll0;
            hFmm->sExtMClkSettings[i].eMClockRate = BRAP_OP_MClkRate_e256Fs;
        }
#endif

#if (BRAP_7440_ARCH == 1)
        /* For 7440 B0 based chips */
#if ((!((BCHP_CHIP == 7440)&&(BCHP_VER == BCHP_VER_A0)))&&(BCHP_CHIP != 3563))
    for(i=0; i<BRAP_RM_P_MAX_DP_BLCK;i++)
    {
        for(j=0;j<BRAP_RM_P_MAX_MIXER_PB_FCI_ID;j++)
        {
            hFmm->ui32MixerPbFciId[i][j] = 0xFFFFFFFF;
            hFmm->ui32MixerPbFciIdCount[i][j] = 0;
        }
    }

#endif
#endif

    } /* End: If not in WatchDog recovery */
    else
    {
        hFmm = *phFmm;
    }
    
    /* Copy the FMM Settings Structure */
    /* The FMM Settings Structure is currently empty */
  
    /* 3. Initialise certain control registers for FMM and various submodules*/

    BDBG_MSG(("Resetting FMM bus logic"));
    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);
    
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
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive)) |
#if BRAP_7440_ARCH
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

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);

#if ((BRAP_7401_FAMILY == 0) || (BCHP_CHIP != 7400))	

    /* Initialize Ring buffers Registers */
    BDBG_MSG(("Clearing ringbuffer registers"));
    BRAP_FMM_P_RBufRegInit (hFmm);

    /* Initialize Data Path Registers */
    BDBG_MSG(("Clearing DP registers"));
    BRAP_FMM_P_DpRegInit (hFmm);
    
#endif
    /* Initialize Micro Sequencer Registers */
    BDBG_MSG(("calling BRAP_FMM_P_MsRegInit"));
    BRAP_FMM_P_MsRegInit (hFmm);


#if ((BRAP_7401_FAMILY == 1) || (BCHP_CHIP == 7400))	

#if (BRAP_7440_ARCH==0)
    /* Enable Buffer Block, Data Path and MicroSequencer */
    reg = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET);
    
    reg |=  (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_MS_OPER_ENABLE, Enable)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_DP_OPER_ENABLE, Enable)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_BF_OPER_ENABLE, Enable));
    
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);
#endif

    ui32Offset = (int)(ePll) * (BCHP_AUD_FMM_PLL1_MACRO - BCHP_AUD_FMM_PLL0_MACRO);    

    
    ui32RegVal1 = BRAP_Read32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL);
    ui32RegVal1 &= ~(BCHP_MASK (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT));
#if (BCHP_CHIP == 7400)
    ui32RegVal1 |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_0));
#else
    ui32RegVal1 |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27a));
#endif 
	BRAP_Write32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL , ui32RegVal1);


    ui32RegVal1 = 0x0;
    
    ui32RegVal1 = BRAP_Read32_isr (hRap->hRegister, 
                                BCHP_AUD_FMM_PLL0_MACRO);
    ui32RegVal1 &= ~(BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT));
    ui32RegVal1 |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_48000));

    BRAP_Write32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_MACRO , ui32RegVal1);


    ui32RegVal = 0x0;
    ui32RegVal1 = 0x0;
    
    ui32RegVal1 = BRAP_Read32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL+ ui32Offset);
    ui32RegVal1 &= ~(BCHP_MASK (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT));
#if (BCHP_CHIP == 7400)
    ui32RegVal1 |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_0));
#else
    ui32RegVal1 |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27a));
#endif
	BRAP_Write32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL+ ui32Offset , ui32RegVal1);



    ui32RegVal = BRAP_Read32_isr (hRap->hRegister, 
                                BCHP_AUD_FMM_PLL0_MACRO + ui32Offset);
    ui32RegVal &= ~(BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT));

    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_48000));

    BRAP_Write32_isr(hRap->hRegister, BCHP_AUD_FMM_PLL0_MACRO + ui32Offset , ui32RegVal);
    
    

    ui32RegVal = 0x0;

    ui32RegVal = BRAP_Read32 (hRap->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0); 

    ui32RegVal = ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                PLLCLKSEL, PLL1_ch2)) |
                   (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                MCLK_RATE, MCLK_is_256fs)));
    
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, ui32RegVal);





    ui32RegVal = 0x0;

    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                    STREAM2_ENA, 1));

    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                    STREAM0_ENA, 1));

    BRAP_Write32_isr (hRap->hRegister,BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, ui32RegVal);


    ui32RegVal = 0x0;

    /* Enable the clock while opening the output port. Never disable it */
    ui32RegVal = BRAP_Read32 (hRap->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0); 
    
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,CLOCK_ENABLE));  
    ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0,  
                                    CLOCK_ENABLE, Enable));
    ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0,  
                                    DATA_ENABLE, Enable));    
    
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    LR_SELECT, Normal));
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                        LIMIT_TO_16_BITS, Disable_limit));
    
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,PREAM_POL)); 
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    PREAM_POL, High));
    
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,SOFT_PARITY)); 
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    SOFT_PARITY, Hard));    

    
    BRAP_Write32 (hRap->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 , 
               ui32RegVal); 



    for(i=0;i<48;i++)
        ui32CBitData[i]=0;

    ui32CBitData[0]=0x4;
    ui32CBitData[1]=0x0000;   /* 0x200 for 48k*/
    ui32CBitData[12]=0x4;
    ui32CBitData[13]=0x0000;
    
    ui32CBitData[24]=0x4;
    ui32CBitData[25]=0x0000;    
    ui32CBitData[36]=0x4;
    ui32CBitData[37]=0x0000;    
        /* Write to CBIT buffer */

    BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: Stream Index 0"));
    ui32CbitArrayOffset =(0 * BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_ELEMENT_SIZE / 4);

    /* Populate the selected CBIT buffer */
    for(uiCnt = 0; uiCnt < 48 ; uiCnt++)
    {
        BRAP_Write32_isr(hRap->hRegister, 
                     BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (uiCnt*4), 
                     ui32CBitData[uiCnt]);
  	     BDBG_MSG(("BRAP_SPDIFFM_P_ProgramCBITBuffer_isr: [%x]",BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                     ui32CbitArrayOffset + (uiCnt*4)));		
    }




     ui32RegValue    =0;

    /* dither */
    ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 );
    ui32RegValue &= ~( (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA))
                       |(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA)));

        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      DITHER_ENA, Enable));

    /* Insert dither signal even when STREAM_ENA is off */
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                      INSERT_WHEN_DISA, Insert));
    
    ui32RegValue |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 
                                        INSERT_ON_UFLOW, 1));

    ui32RegValue |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM, 1));

        BRAP_Write32 (hRap->hRegister,
                  BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0,
                  ui32RegValue); 
        
     ui32RegValue    =0;

    /* dither */
    ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1 );
    ui32RegValue &= ~( (BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, DITHER_ENA))
                       |(BCHP_MASK (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, INSERT_WHEN_DISA)));

        ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 
                                      DITHER_ENA, Enable));

    /* Insert dither signal even when STREAM_ENA is off */
    ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 
                                      INSERT_WHEN_DISA, Insert));
    
    ui32RegValue |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 
                                        INSERT_ON_UFLOW, 1));

    ui32RegValue |= (BCHP_FIELD_DATA (AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, SPDIF_OR_PCM, 1));

        BRAP_Write32 (hRap->hRegister,
                  BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1,
                  ui32RegValue);
        
    

    /* Bypass  the MS */
    BRAP_Write32 (hRap->hRegister,
                  BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS,
                  0x0);


    ui32RegValue =0 ;

    /* Enable the SPDIF Formatter output */
    ui32RegValue = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);

    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA));
    ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                     STREAM0_ENA, Enable);

    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA));
    ui32RegValue |= BCHP_FIELD_ENUM (AUD_FMM_MS_CTRL_STRM_ENA, 
                                     STREAM1_ENA, Enable);
            

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA,
                  ui32RegValue);

    /* Initialize Ring buffers Registers */
    BDBG_MSG(("Clearing ringbuffer registers"));
    BRAP_FMM_P_RBufRegInit (hFmm);

    /* Initialize Data Path Registers */
    BDBG_MSG(("Clearing DP registers"));
    BRAP_FMM_P_DpRegInit (hFmm);

#endif

    /* Initialize HiFiDACs Registers */
    BDBG_MSG(("calling BRAP_FMM_P_HiFiDacInit"));
#if(BCHP_CHIP != 7601)    
    BRAP_FMM_P_HiFiDacInit (hFmm);
#endif
#if BRAP_7440_ARCH
    /* Initialize I2sMulti Registers */
    BDBG_MSG(("calling BRAP_FMM_P_I2sMultiInit"));
    BRAP_FMM_P_I2sMultiInit (hFmm);

#if ((!((BCHP_CHIP == 7440)&&(BCHP_VER == BCHP_VER_A0))) && (BCHP_CHIP != 3563) &&(BRAP_3548_FAMILY != 1))
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

#if (BRAP_7440_ARCH==0)
    /* Enable Buffer Block, Data Path and MicroSequencer */
    reg = BRAP_Read32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET);
    
    reg |=  (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_MS_OPER_ENABLE, Enable)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_DP_OPER_ENABLE, Enable)) |
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, FMM_BF_OPER_ENABLE, Enable));
    
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, reg);
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
#if (BRAP_7440_ARCH == 1)
    unsigned int i = 0;
#endif

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_Close);

    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);
    
#if (BRAP_7440_ARCH == 1)
    /* Clear all referrences to this FMM */ 
    for(i=0; i < BRAP_RM_P_MAX_SRC_BLCK; i++)
    {
        if(NULL != hFmm->pSrcMemLocationInfo[i])
        {
            BKNI_Free(hFmm->pSrcMemLocationInfo[i]);
        }
    }
#endif

    hFmm->hRap->hFmm[hFmm->uiIndex] = NULL;

    
    /* Free the FMM Handle memory*/
    BKNI_Free (hFmm); 
                 
    BDBG_LEAVE (BRAP_FMM_P_Close);
    return ret;
}


/***************************************************************************
Summary:
    Initializes the dma 

Description:
    This function must be called before starting a source channel 

Returns:
    Nothing

See Also:
    
**************************************************************************/
static void
BRAP_FMM_P_ProgramDMASlot (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
#if (BRAP_7440_ARCH == 0)
    uint32_t       ui32RegValue = 0;
#endif
    BREG_Handle    hRegister;

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_ProgramDMASlot);

    hRegister = hFmm->hRegister;

    /* Program BCHP_AUD_FMM_BF_CTRL_PROCSEQID_0_3 register (DMA Slots 0 to 3) */
#if (BRAP_7440_ARCH == 0)
    /* Make the PROCESS_SEQ_IDX_VALID = 1 (enabling DMA) and 
       BLOCKED_ACCESS_X_DISABLE = 0 (enabling blocked access) 
       and PROCESS_IDX_HIGH = 0 (no priority) for each DMA client. 
       Also program the DMA Process Sequence ID for each DMA Slot. */
    ui32RegValue =  (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID3_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, BLOCKED_ACCESS_3_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_ID3_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID_3, BRAP_SRCCH_P_DmaClientId_e3)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID2_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, BLOCKED_ACCESS_2_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_ID2_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID_2, BRAP_SRCCH_P_DmaClientId_e2)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID1_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, BLOCKED_ACCESS_1_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_ID1_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID_1, BRAP_SRCCH_P_DmaClientId_e1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID0_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, BLOCKED_ACCESS_0_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_ID0_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_0_3, PROCESS_SEQ_ID_0, BRAP_SRCCH_P_DmaClientId_e0));

    /* Write the register value to the BCHP_AUD_FMM_BF_CTRL_PROCSEQID_0_3 
       register to initialize DMA */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_PROCSEQID_0_3, ui32RegValue);
        
    /* Program BCHP_AUD_FMM_BF_CTRL_PROCSEQID_4_7 register (DMA Slots 4 to 7) */

    /* Make the PROCESS_SEQ_IDX_VALID = 1 (enabling DMA) and 
       BLOCKED_ACCESS_X_DISABLE = 0 (enabling blocked access) 
       and PROCESS_IDX_HIGH = 0 (no priority) for each DMA client. 
       Also program the DMA Process Sequence ID for each DMA Slot. */
    ui32RegValue = 0;
    ui32RegValue =  
#if defined ( BCHP_7411_VER ) 
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID7_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_7_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID7_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_7, BRAP_DSTCH_P_DmaClientId_e1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID6_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_6_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID6_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_6, BRAP_DSTCH_P_DmaClientId_e0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID5_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_5_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID5_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_5, BRAP_SRCCH_P_DmaClientId_e5)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID4_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_4_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID4_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_4, BRAP_SRCCH_P_DmaClientId_e4));
#elif ( BCHP_CHIP == 7400 )
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID7_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_7_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID7_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_7, BRAP_SRCCH_P_DmaClientId_e7)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID6_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_6_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID6_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_6, BRAP_SRCCH_P_DmaClientId_e6)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID5_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_5_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID5_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_5, BRAP_SRCCH_P_DmaClientId_e5)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID4_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_4_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID4_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_4, BRAP_SRCCH_P_DmaClientId_e4));
#elif (BRAP_7401_FAMILY == 1)
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID4_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, BLOCKED_ACCESS_4_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_ID4_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_4_7, PROCESS_SEQ_ID_4, BRAP_DSTCH_P_DmaClientId_e0));
#endif                    
    /* Write the register value to the BCHP_AUD_FMM_BF_CTRL_PROCSEQID_4_7 
       register to initialize DMA */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_PROCSEQID_4_7, ui32RegValue);

#if ( BCHP_CHIP == 7400 )
    ui32RegValue = 0;
    ui32RegValue =  
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_SEQ_ID9_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, BLOCKED_ACCESS_9_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_ID9_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_SEQ_ID_9, BRAP_DSTCH_P_DmaClientId_e1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_SEQ_ID8_VALID, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, BLOCKED_ACCESS_8_DISABLE, 1)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_ID8_HIGH, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_PROCSEQID_8_9, PROCESS_SEQ_ID_8, BRAP_DSTCH_P_DmaClientId_e0));

    			BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_PROCSEQID_8_9, ui32RegValue);
#endif

    /* Program DMA Block Count = 0 for all DMA clients */
    ui32RegValue = 0;
    ui32RegValue =  
#if defined ( BCHP_7411_VER ) ||( BCHP_CHIP == 7400 )
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_7, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_6, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_5, 0)) |
#endif                    
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_4, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_3, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_2, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_1, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, DMA_BLOCK_CNT_0, 0));
    
    /* Write the register value to the BCHP_AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7 register */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_0_7, ui32RegValue);


#if ( BCHP_CHIP == 7400 )
    ui32RegValue = 0;
    ui32RegValue =  
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_8_9, DMA_BLOCK_CNT_9, 0)) |
                    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_8_9, DMA_BLOCK_CNT_8, 0));

    /* Write the register value to the BCHP_AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_8_9 register */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DMA_BLOCK_CNT_8_9, ui32RegValue);
#endif
#endif /* (BRAP_7440_ARCH == 0) */
    BDBG_LEAVE (BRAP_FMM_P_ProgramDMASlot);
    return;
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
#if BRAP_7440_ARCH
    unsigned int uiSrcChId = 0;
    uint32_t ui32Offset = 0;
#endif /* 7440 */

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_RBufRegInit);

    /* Clear all ring buffer register RAM for safe operations.
       From AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR 
       To AUD_FMM_BF_CTRL_FMM_BF_RESERVE_ECO_25 */
#if defined ( BCHP_7411_VER ) 
    for (i=0; i<(BCHP_AUD_FMM_BF_CTRL_FMM_BF_RESERVE_ECO_25 - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4; i++)
#elif (BRAP_7401_FAMILY == 1)
    for (i=0; i<(BCHP_AUD_FMM_BF_CTRL_FMM_BF_RESERVE_ECO_15 - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4; i++)
#elif ( BCHP_CHIP == 7400 )
    for (i=0; i<(BCHP_AUD_FMM_BF_CTRL_RINGBUF_15_START_WRPOINT - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4; i++)
#elif ((BCHP_CHIP == 7440) && (BCHP_VER == BCHP_VER_A0))
    for (i=0;i<=(BCHP_AUD_FMM_BF_CTRL_RINGBUF_55_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#elif (((BCHP_CHIP == 7440) && (BCHP_VER != BCHP_VER_A0)) ||(BCHP_CHIP == 7601))
    for (i=0;i<=(BCHP_AUD_FMM_BF_CTRL_RINGBUF_63_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#elif (BCHP_CHIP == 3563)
    for (i=0;i<(BCHP_AUD_FMM_BF_CTRL_RINGBUF_23_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#elif ( (BRAP_7405_FAMILY == 1) || (BRAP_3548_FAMILY == 1))
    for (i=0;i<(BCHP_AUD_FMM_BF_CTRL_RINGBUF_23_MI_VALID - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)/4;i++)
#endif
    {
        BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + (i*4), 0);
    }
    /* Initialize the DMA module's registers */
    BRAP_FMM_P_ProgramDMASlot (hFmm);

#if BRAP_7440_ARCH
    /* No grouping */
    for(uiSrcChId = 0; uiSrcChId < BRAP_RM_P_MAX_SRC_CHANNELS; uiSrcChId++)
    {
        ui32Offset = uiSrcChId * BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_ELEMENT_SIZE/8;
        BRAP_Write32(hFmm->hRegister,
                     BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + ui32Offset,
                     uiSrcChId);
    }
#endif /* BRAP_7440_ARCH */

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
#if BRAP_7440_ARCH 
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
#if (BRAP_DVD_FAMILY == 1)
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
#if (BRAP_DVD_FAMILY == 1)
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
#if (BRAP_DVD_FAMILY == 1)
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
            0x00100000);
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            0x00100000);
#if (BRAP_DVD_FAMILY == 1)            
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_PING_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            0x00100000);
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_PONG_COEFF_RAMP_STEPi_ARRAY_BASE + (4 * i),
            0x00100000);
#endif
    }
    
    /* AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP : VOLUME_RAMP_STEP */
    ui32RegVal = BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, 0x200);
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, ui32RegVal);

    /* Program Mixer Soft Coefficients */
    for (i=0;i<=BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_END;i++)
    {
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_BASE + (4*i),
            BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)            
        BRAP_Write32 (hFmm->hRegister,
            BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFCi_ARRAY_BASE + (4*i),
            BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    }

    /* Program other soft coefficients */
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFA,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFA,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFB,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFB,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFD,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFD,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFE,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFE,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFF,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEFF,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_0_5,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_0_5,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_0_8,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_0_8,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif
    i++;

    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_SOFT_COEF_1_25,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL1_SOFT_COEF_1_25,
                      BRAP_MIXER_P_Soft_Coeff_Array[i]);
#endif

    /* Program 
            AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP : VOLUME_RAMP_STEP = 0x200,
            for both the DPs
    */
    ui32RegVal = (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP , VOLUME_RAMP_STEP, 0x200));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, ui32RegVal);

    /* Commenting out DP_PACING_FIX enable code, as BD 96kHz streams were failing with this change */
#if 0
    ui32RegVal = BRAP_Read32(hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_DP_MISC_CTRL);

    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_DP_MISC_CTRL , DP_PACING_FIX, Enable));
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_DP_MISC_CTRL, ui32RegVal);
#endif

#if (BRAP_DVD_FAMILY == 1)
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL1_VOLUME_RAMP_STEP , 0x200);

    /* Commenting out DP_PACING_FIX enable code, as BD 96kHz streams were failing with this change */
#if 0
    ui32RegVal = BRAP_Read32(hFmm->hRegister,BCHP_AUD_FMM_DP_CTRL0_DP_MISC_CTRL);
    ui32RegVal |= 0x00000040;
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL1_DP_MISC_CTRL, ui32RegVal);
#endif

#endif


#else
    int         i;
    uint32_t    ui32RegVal;
    
    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_DpRegInit);
    
    /* Initialize the DP RAM to all 0 */

    for(i=0; 
        i <  (BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP  - 
            BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG + 4); 
        i=i+4)

    {
        BRAP_Write32(hFmm->hRegister, (BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG  + i), 0);
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
        BRAP_Write32(hFmm->hRegister, (BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT0_CONFIG  + i), 0);
#endif        
    }
    /* Program 
            AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP : SCALE_RAMP_STEP_SIZE = 0x200,
            AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP : VOL_RAMP_STEP_SIZE = 0x200
            for both the DPs
    */
#if (BCHP_CHIP == 7400)    
    ui32RegVal = (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, SCALE_RAMP_STEP_SIZE, 0x00)) 
            | (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, VOL_RAMP_STEP_SIZE, 0x20));
#else
    ui32RegVal = (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, SCALE_RAMP_STEP_SIZE, 0x200)) 
            | (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, VOL_RAMP_STEP_SIZE, 0x200));
#endif

    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, ui32RegVal);
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_DP_CTRL1_FMM_SCALE_VOL_STEP, ui32RegVal);
#endif
    BDBG_LEAVE (BRAP_FMM_P_DpRegInit);
#endif/* BRAP_7440_ARCH */
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
#if BRAP_7440_ARCH    
    uint32_t ui32MsFwRegs = 0;
#endif

    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_MsRegInit);
  

    /* 1. Initialize the Firmware registers to 0 */ 
#if (BRAP_7440_ARCH==0)    
    for ( i = 0; i< BRAP_P_MAX_MS_FW_REGS_MEM; i+=4 )
    {
        /* Don't write to the address range from 0x638a00 to 0x638ffc*/
        if (((i + BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 ) >= 0x638a00) &&
            ((i + BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 ) <= 0x638ffc))
            continue;

        BRAP_Write32 (hFmm->hRegister, 
                        BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + i,  
                        0);
    }
#else
    /* There are 2 discontinuous register sets */
    /* a) starts at BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 */
    ui32MsFwRegs = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 
                   (BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END + 1)- 
                   BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;
    for ( i = 0; i< ui32MsFwRegs; i+=4 )
    {
        BRAP_Write32 (hFmm->hRegister, 
                        BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + i,  
                        0);
    }

    /* b) starts at BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE */
    for ( i = 0; i< BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i+=4)
    {
        BRAP_Write32 (hFmm->hRegister, 
                        BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + i,  
                        0);
    }
#endif

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

#if 0
        BDBG_MSG(("MS_INST addr=0x%x, inst=0x%x",   
                    ( BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE 
                       + BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_START
                       + (i*4)), 
                      ui32RegValue));
#endif 
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

    BDBG_LEAVE (BRAP_FMM_P_MsRegInit);
    return;
}
#if(BCHP_CHIP != 7601)
static void 
BRAP_FMM_P_HiFiDacInit (
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
)
{
#ifdef BCHP_7411_VER /* Only for the 7411 */ 
    BSTD_UNUSED(hFmm);
#else
    BDBG_ASSERT (hFmm);
    BDBG_ENTER (BRAP_FMM_P_HiFiDacInit);
    /* Reset the HiFiDAC 0 by writing all 1's to HIFIDAC_CTRL0_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL0_RESET, 1);

    /* Bring HiFiDAC 0 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL0_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL0_RESET, 0);

#if ( (BCHP_CHIP == 7400) || (BRAP_3548_FAMILY == 1) )
    /* Reset the HiFiDAC 1 by writing all 1's to HIFIDAC_CTRL1_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL1_RESET, 1);

    /* Bring HiFiDAC 1 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL1_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL1_RESET, 0);
#endif /* BCHP_CHIP == 7400 */

#if (BRAP_3548_FAMILY == 1)
    /* Reset the HiFiDAC 2 by writing all 1's to HIFIDAC_CTRL2_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL2_RESET, 1);

    /* Bring HiFiDAC 2 back to normal mode by writing all 0's to 
     * HIFIDAC_CTRL2_RESET register */
    BRAP_Write32 (hFmm->hRegister, BCHP_HIFIDAC_CTRL2_RESET, 0);
#endif

    BDBG_LEAVE (BRAP_FMM_P_HiFiDacInit);
#endif
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
    /* Disable/Reset Buffer Block, Data Path and MicroSequencer */
    BRAP_Write32 (hFmm->hRegister, BCHP_AUD_FMM_MISC_RESET, 0);

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
#if BRAP_7440_ARCH
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


#if (BRAP_7440_ARCH == 1)
static BERR_Code
BRAP_FMM_P_I2sMultiInit(
    BRAP_FMM_P_Handle hFmm        /* [in] FMM Device Handle */
    )
{
    BERR_Code   ret = BERR_SUCCESS;    
    BDBG_ASSERT(hFmm);
    BDBG_ENTER(BRAP_FMM_P_I2sMultiInit);
#if (((BCHP_CHIP == 7440)&&(BCHP_VER == BCHP_VER_A0)) ||(BCHP_CHIP == 3563)||(BRAP_3548_FAMILY == 1))
    /* Clear the OP_CTRL_I2SM_GROUPING */
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_I2SM_GROUPING, 0);
#else
    /* Clear the OP_CTRL_I2SM_GROUPING */
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_I2SM0_GROUPING, 0);
    BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE, 0);
#endif

    BDBG_LEAVE(BRAP_FMM_P_I2sMultiInit);
    return ret;
}

#if ((!((BCHP_CHIP == 7440)&&(BCHP_VER == BCHP_VER_A0))) && (BCHP_CHIP != 3563)&&(BRAP_3548_FAMILY != 1))
static BERR_Code BRAP_FMM_P_CapPortInit(BRAP_FMM_P_Handle hFmm)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    unsigned int    uiCapPort = 0;
    
    BDBG_ASSERT(hFmm);
    BDBG_ENTER(BRAP_FMM_P_I2sMultiInit);

    for (uiCapPort =0 ; uiCapPort < BRAP_P_MAX_CAPPORTS ; uiCapPort++)
    {
#if 0
        ui32RegVal = BRAP_Read32 (hFmm->hRegister, 
              (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+(uiCapPort*4)));
#endif
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP));
        ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi,GROUP,uiCapPort);
        
        BRAP_Write32 (hFmm->hRegister, (BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE+
                                            (uiCapPort*4)), ui32RegVal);
    }
    return ret;
}
#endif 

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

        BRAP_Write32(hFmm->hRegister, (BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset), 0x200);   
        
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
#if 0
        for(i=0; i<64 ;i++)
        {
            ui32RegVal = BRAP_SRC_P_2x_Coeff_Array[i];
            ui32RegVal <<= 2;

            if (0 != (ui32RegVal & 0x2000000) ) 
            {
                ui32RegVal |= 0x0C000000
            }
            
            BRAP_Write32(hFmm->hRegister,
                         uiBlkOffset +
                         BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (i*4),
                         ui32RegVal);
        }
#endif
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

        for (i=0; i<40; i++)
        {
            if (0 == (i%5))
            {
                BRAP_Write32(hFmm->hRegister,
                             uiBlkOffset +
                             BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (221*4) + (i*4),
                             0x4000000);
                
            }
            else
            {
                BRAP_Write32(hFmm->hRegister,
                             uiBlkOffset +
                             BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (221*4) + (i*4),
                             0x0);
            }
        }
        
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

#if BRAP_P_PR21368_WORKAROUND
        /* The work-around for PR21368, reserves the 12th SRC for dummy FCI 
           linking to unused mixer inputs. As a result, the effective usable
           number of SRCs left for use are 11. But, while initializing the 
           SRC registers to default values, all 12 SRCs are required. Hence, 
           this #ifdef. */
        for(uiSrcId = 0, ui32RegVal = 0;
            uiSrcId < BRAP_RM_P_MAX_SRC_PER_SRC_BLCK+1;
            uiSrcId++, ui32RegVal = 0)
#else
        for(uiSrcId = 0, ui32RegVal = 0;
            uiSrcId < BRAP_RM_P_MAX_SRC_PER_SRC_BLCK;
            uiSrcId++, ui32RegVal = 0)
#endif                
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
        BRAP_Write32(hFmm->hRegister, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP + uiBlkOffset, 0x200);

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
#endif
/* End of File */
