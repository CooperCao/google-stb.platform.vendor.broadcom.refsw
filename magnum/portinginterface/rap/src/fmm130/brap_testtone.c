/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
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
*   Module name: Test Tone
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/


#include "brap.h"
#include "brap_priv.h"
#include "brap_types.h"

#include "brap_testtone_priv.h"

BDBG_MODULE(rap_tone);

#define BRAP_P_TESTTONE_RBUF_SIZE 0xC00

extern unsigned int rbuf0_data_lr_1k[];

BERR_Code BRAP_TestToneEnable(
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    void * pBufferStart;
    uint32_t ui32RegVal = 0;
    unsigned int count;
	static uint32_t ui32PhyAddr = 0;
	 
    BDBG_ENTER (BRAP_TestToneEnable);
    BDBG_ASSERT(hRap);
        
    BDBG_MSG(("Test tone enable:"));
    
        /* PLL 0 programming */
        BRAP_Write32(hRap->hRegister, BCHP_AUD_FMM_PLL0_MACRO, 0x2);

    /* allocate ring buffer memory */
	if(ui32PhyAddr == 0)
	{
    	pBufferStart = BRAP_P_AllocAligned (hRap, BRAP_P_TESTTONE_RBUF_SIZE, 8, 0
#if ( BCHP_CHIP == 7400) || ( BCHP_CHIP == 7401 ) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)												
												,false
#endif												
												);
   		if (pBufferStart == NULL )
    	{
       		return BERR_TRACE (BERR_OUT_OF_DEVICE_MEMORY);
    	}    
    	ret = BRAP_P_ConvertAddressToOffset (hRap->hHeap, pBufferStart, &ui32PhyAddr);
    	if (ret != BERR_SUCCESS)
    	{
        	BDBG_ERR(("BRAP_TestToneEnable: Unable to convert RBUF 0 start address to physical address"));
        	return BERR_TRACE(ret);
    	}
	}
    

    /* Save all registers that we're over writing */
    hRap->sToneRegs.ui32SpdifCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0);
    hRap->sToneRegs.ui32I2sCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE);
    hRap->sToneRegs.ui32OpEnableStatus = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_STATUS);    
    hRap->sToneRegs.ui32SerOutOe = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE);

    hRap->sToneRegs.ui32MsBypass = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);
    hRap->sToneRegs.ui32MsStreamCtrl0 = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);    
    hRap->sToneRegs.ui32MsStreamEna = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);

    hRap->sToneRegs.ui32Client0Mix1Cfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG);
    hRap->sToneRegs.ui32Client2Mix1Cfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT2_MIX1_CONFIG);
    hRap->sToneRegs.ui32Pb0ScaleSrcCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG);
    hRap->sToneRegs.ui32Pb2ScaleSrcCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK2_SCALE_SRC_CONFIG);
    hRap->sToneRegs.ui32DpStreamEna = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_STRM_ENA);

    hRap->sToneRegs.ui32Rbuf0BaseAddr = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR);
    hRap->sToneRegs.ui32Rbuf0EndAddr = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR);

    hRap->sToneRegs.ui32SrcCh0Cfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG);
    hRap->sToneRegs.ui32SrcCh0Ctrl = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL);
	
    /* Turn on SPDIF output in OP */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0, 0x2b);
    BDBG_MSG(("programming SPDIF "));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, 0x1);
    /* Turn on I2S output in OP */
    BDBG_MSG(("programming I2S"));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE, 0x421823);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, 0x4);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE, 0x7);
    /* Turn on Stream 0 in MS*/
    BDBG_MSG(("programming MS"));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, 0x0);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 0x11f);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0, 0x0);/* disable burst */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, 0x1);
    /* Channel status bits */
    BDBG_MSG(("programming CBITs"));

    for (count=BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_START; count<=BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END; count=count+4)
    {
        BRAP_Write32 (hRap->hRegister, 
                       ( BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + count), 0); 
    }
    
    BRAP_Write32 (hRap->hRegister, 
                       ( BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE 
                       + BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_START
                       + 0), 0x0004); /* CBIT0 */    
    BRAP_Write32 (hRap->hRegister, 
                       ( BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE 
                       + BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_START
                       + 4), 0x0200); /* CBIT1 */     
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, 0x50);
    /* Program the mixer */
    BDBG_MSG(("programming DP"));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 0x01000000);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT2_MIX1_CONFIG, 0x01000000);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 0);	
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK2_SCALE_SRC_CONFIG, 0);	
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_STRM_ENA, 0x5);
    /* Program the RBUF */
    BDBG_MSG(("programming RBUF"));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR, ui32PhyAddr);

#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   	
    ui32RegVal = (BCHP_FIELD_DATA ( AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        SOURCE_RINGBUF_ENDADDR, 
                        ui32PhyAddr + BRAP_P_TESTTONE_RBUF_SIZE )); 
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, ui32RegVal);    
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        SRING_RDADDR, 
                        ui32PhyAddr));      
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, ui32RegVal);
#else
	/* for 7401 & 7411 D0 */
    ui32RegVal = (BCHP_FIELD_DATA ( AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        RINGBUF_ENDADDR, 
                        ui32PhyAddr + BRAP_P_TESTTONE_RBUF_SIZE - 1)); 
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, ui32RegVal);    
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        RINGBUF_RDADDR, 
                        ui32PhyAddr));      
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, ui32RegVal);
#endif
    /* Program the Src Ch  */
    BDBG_MSG(("programming SrcCh"));
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG, 0x00000401);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, 0x00000001);
   
    /* Load the test tone into the rbuf */
    BDBG_MSG(("loading data into rbuf"));
    for (count = 0; count < BRAP_P_TESTTONE_RBUF_SIZE/4; count++) {
		BRAP_MemWrite32((BARC_Handle)hRap->hRegister, ui32PhyAddr + count * 4, rbuf0_data_lr_1k[count%(768)]); 
    } 
    
    BDBG_LEAVE (BRAP_TestToneEnable);
    return ret;

}

BERR_Code BRAP_TestToneDisable (
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
)
{
    unsigned int count;
    uint32_t ui32PhyAddr =0;
    BDBG_ENTER (BRAP_TestToneDisable);

    ui32PhyAddr = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR);

    /* Turn off  outputs */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, 0x4);
    /* Turn off Stream 0 in MS*/
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, 0x0);
    /* turn off the mixer */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_STRM_ENA, 0x0);
    /* turn off the src ch */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, 0x0);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG, 0x0);
    
    /* clear the rbuf */

    for (count = 0; count < BRAP_P_TESTTONE_RBUF_SIZE/4; count++) {
		BRAP_MemWrite32((BARC_Handle)hRap->hRegister, ui32PhyAddr + count * 4, 0); 
    }  
    
    /* Restore all the registers we'd overwritten */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0, hRap->sToneRegs.ui32SpdifCfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE, hRap->sToneRegs.ui32I2sCfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, hRap->sToneRegs.ui32OpEnableStatus);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE, hRap->sToneRegs.ui32SerOutOe);
    
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, hRap->sToneRegs.ui32MsBypass);
    /* Old status + Enable dither and insert dither signal even when stream is off */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, hRap->sToneRegs.ui32MsStreamCtrl0 | 0x102);
    /* Leave burst disabled, don't restore it */
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, hRap->sToneRegs.ui32MsStreamEna);
    
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, hRap->sToneRegs.ui32Client0Mix1Cfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT2_MIX1_CONFIG, hRap->sToneRegs.ui32Client2Mix1Cfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, hRap->sToneRegs.ui32Pb0ScaleSrcCfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK2_SCALE_SRC_CONFIG, hRap->sToneRegs.ui32Pb2ScaleSrcCfg);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_STRM_ENA, hRap->sToneRegs.ui32DpStreamEna);
   
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR, hRap->sToneRegs.ui32Rbuf0BaseAddr);
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, hRap->sToneRegs.ui32Rbuf0EndAddr);

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG, hRap->sToneRegs.ui32SrcCh0Cfg);	
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, hRap->sToneRegs.ui32SrcCh0Ctrl);	    
    
    BDBG_LEAVE (BRAP_TestToneDisable);

    return BERR_SUCCESS;

}

/* End of File */
