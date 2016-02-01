/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
*   Module name: RBUF
*   This file contains the implementation of all PIs for the Ring
*   Buffer abstraction. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_rbuf);

static BERR_Code BRAP_RBUF_P_HWConfig (
    BRAP_RBUF_P_Handle  hRBuf  		/* [in] RBUF handle */
);

/* Default Settings and Parameters */
static const BRAP_RBUF_P_Settings defRbufSettings =
{
    {
       0,                                   /* sExtSettings.pBufferStart */
       BRAP_RBUF_P_DEFAULT_SIZE,            /* sExtSettings.uiSize */     
       BRAP_RBUF_P_DEFAULT_FREE_BYTE_MARK   /* sExtSettings.uiWaterMark */
    },
    false,                                   /* bProgRdWrRBufAddr */ 
    false                                   /* bRbufOfClonedPort*/
#if (BRAP_SECURE_HEAP==1)
	,false,						/* No secure memory by default */
#endif    
};
static const BRAP_RBUF_P_Params defRbufParams =
{
    BRAP_RBUF_P_DEFAULT_START_WR_POINT,   /* uiStartWRPoint */
    BRAP_RBUF_P_DEFAULT_SIZE
};



BERR_Code 
BRAP_RBUF_P_GetDefaultSettings ( 
    BRAP_RBUF_P_Settings   * pDefSettings  /* Pointer to memory where default
                                              settings should be written */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_RBUF_P_GetDefaultSettings);

    BDBG_ASSERT (pDefSettings);            
    
    *pDefSettings = defRbufSettings;
    BDBG_LEAVE (BRAP_RBUF_P_GetDefaultSettings);
    return ret;
}

BERR_Code 
BRAP_RBUF_P_GetDefaultParams ( 
    BRAP_RBUF_P_Params    *pDefParams   /* Pointer to memory where default
                                           settings should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_RBUF_P_GetDefaultParams);

    BDBG_ASSERT (pDefParams);            
    
    *pDefParams = defRbufParams;
    BDBG_LEAVE (BRAP_RBUF_P_GetDefaultParams);
    return ret;
}


BERR_Code BRAP_RBUF_P_Open (
    BRAP_FMM_P_Handle      		hFmm,       /* [in] FMM handle */
    BRAP_RBUF_P_Handle 	   		*phRBuf,    /* [out] Pointer to RBUF handle */
    unsigned int           		uiRbufIndex,/* [in] RBUF index */           
    const BRAP_RBUF_P_Settings 	*pSettings 	/* [in] Open time settings */
)
{

    BERR_Code ret = BERR_SUCCESS;
    BRAP_RBUF_P_Handle hRBuf;
    uint32_t ui32RegVal = 0;
    unsigned int i=0;
    uint32_t ui32DummyVar;
    uint32_t ui32PhyAddr =0;
	unsigned int uiSize = 0;
	unsigned int uiWaterMark = 0;
#if BCHP_7411_VER > BCHP_VER_C0
	BRAP_AudioMemAllocation eMemAllocation= BRAP_AudioMemAllocation_eStereo;
#endif
	
    BDBG_ENTER (BRAP_RBUF_P_Open);
    /* 1. Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of RBufs
     * - Pointer to Settings structure is NULL
     * - Pointer to RBuf handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phRBuf);
    
    if ( uiRbufIndex > (BRAP_RM_P_MAX_RBUFS -1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }      

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */   
        BDBG_ASSERT (pSettings);
    
        if (hFmm->hRBuf[uiRbufIndex] != NULL )
        {
            BDBG_ERR(("BRAP_RBUF_P_Open: RBUF %d is already open", uiRbufIndex));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
       
        BDBG_MSG(("BRAP_RBUF_P_Open: Given input parameters"
               "uiRbufIndex=%d, " 
               "pSettings->bProgRdWrRBufAddr=%d,"
               "pSettings->bRbufOfClonedPort=%d,"
               "pSettings->sExtSettings.pBufferStart=0x%x,"
               "pSettings->sExtSettings.uiSize=%d,"
               "pSettings->sExtSettings.uiWaterMark=%d ",
               uiRbufIndex,
               pSettings->bProgRdWrRBufAddr,
               pSettings->bRbufOfClonedPort,
               pSettings->sExtSettings.pBufferStart,
               pSettings->sExtSettings.uiSize,
               pSettings->sExtSettings.uiWaterMark)); 


        if (pSettings->sExtSettings.pBufferStart != NULL)
        { 
            /* Check Start address and size. Should be a multiple of 256 bytes */
            if ((((int)(pSettings->sExtSettings.pBufferStart)%BRAP_RBUF_P_ALIGNMENT) != 0) 
                || (((pSettings->sExtSettings.uiSize % BRAP_RBUF_P_ALIGNMENT)) != 0) )
            {
                BDBG_ERR (("BRAP_RBUF_P_Open: Ringbuffer start address or size is not properly aligned."));
                return BERR_TRACE (BERR_INVALID_PARAMETER);
            }  
        }

    
        /* 2. Allocate memory for the RBUF handle. Fill in parameters in the RBUF
           handle. */
    
        /* Allocate RBuf handle */
        hRBuf = (BRAP_RBUF_P_Handle) BKNI_Malloc (sizeof(BRAP_RBUF_P_Object));
        if (hRBuf == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset (hRBuf, 0, sizeof(BRAP_RBUF_P_Object));

        /* Initialise known elements in RBUF handle */
        hRBuf->hChip = hFmm->hChip;
        hRBuf->hRegister = hFmm->hRegister;
        hRBuf->hInt = hFmm->hInt;
        hRBuf->hFmm = hFmm;
        hRBuf->uiIndex = uiRbufIndex;
        hRBuf->ui32Offset = (BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR  
                       - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR) 
                       * uiRbufIndex; 
		hRBuf->hHeap = hFmm->hHeap;
#if ( BRAP_DVD_FAMILY == 1) || ( BCHP_CHIP == 3563) || ( BRAP_7405_FAMILY == 1)		
		hRBuf->bAllocatedInternally=false;
#endif
#if (BRAP_SECURE_HEAP==1)	
	/* If BRAP_SECURE_HEAP is defined, RBUFS should be allocated from Secure 
	Memory Region for Decode channels, and normal region for PB and 
	CAP channels. */
	if ((pSettings->bSecureMemory == true) && (hFmm->hRap->sSettings.hSecureMemory != NULL))	
	{
		hRBuf->hHeap =  hFmm->hRap->sSettings.hSecureMemory;
	}
#endif

		/* If caller has supplied size and watermark, use them, else use default */
	   	uiSize = (pSettings->sExtSettings.uiSize == 0) ?  
				BRAP_RBUF_P_DEFAULT_SIZE : pSettings->sExtSettings.uiSize;
		uiWaterMark = (pSettings->sExtSettings.uiWaterMark == 0) ?
				BRAP_RBUF_P_DEFAULT_WATER_MARK : pSettings->sExtSettings.uiWaterMark;

#if BCHP_7411_VER > BCHP_VER_C0
	eMemAllocation = BRAP_P_GetMemoryAllocationType(hFmm->hRap);
	if (eMemAllocation==BRAP_AudioMemAllocation_eMultiChannelSrc)
	{
		uiSize = BRAP_RBUF_P_7411D_MULTICHANNEL_SRC_SIZE;
	}
#endif

        /* Fill up the RBUF Settings Structure */
        if (pSettings->sExtSettings.pBufferStart == NULL)
        {
            /* Use internally allocated RBUF memory */
            hRBuf->sSettings.sExtSettings.pBufferStart 
		            = (void *)BRAP_P_AllocAligned (hFmm->hRap, uiSize, 8, 0
#if (BRAP_SECURE_HEAP==1)
					,false
#endif												
					);
            if (hRBuf->sSettings.sExtSettings.pBufferStart == (void *) BRAP_P_INVALID_DRAM_ADDRESS)
            {
                /* Free the RBUF Handle memory*/
                BKNI_Free (hRBuf); 
                return BERR_TRACE (BERR_OUT_OF_DEVICE_MEMORY);
            }

	        for (i=0; i<uiSize; i=i+4)
	        {
	            BRAP_P_DRAMWRITE ((BARC_Handle) hRBuf->hRegister, 
	                ((unsigned int)(hRBuf->sSettings.sExtSettings.pBufferStart) + i), 
	                0);
	        }			
  
      	    /* Have a DRAM read following DRAM writes, 
  	        in order to fix the Stream Arc memory coherency issue. */
       	    ui32DummyVar = BRAP_P_DRAMREAD((BARC_Handle) hRBuf->hRegister, 
                (unsigned int)(hRBuf->sSettings.sExtSettings.pBufferStart));
#if ( BRAP_DVD_FAMILY == 1)  || ( BCHP_CHIP == 3563) || ( BRAP_7405_FAMILY == 1)		
			hRBuf->bAllocatedInternally=true;
#endif
			
            BDBG_MSG (("BRAP_RBUF_P_Open: Device memory starting at 0x%x reserved RBUF[%d]", 
                     (hRBuf->sSettings.sExtSettings.pBufferStart), uiRbufIndex));
       	}
        else 
        {
            /* External memory has been allocated for this RBUF */
            hRBuf->sSettings.sExtSettings.pBufferStart 
                   =  pSettings->sExtSettings.pBufferStart;
#if (  BRAP_DVD_FAMILY == 1) || ( BCHP_CHIP == 3563) || ( BRAP_7405_FAMILY == 1)			
			hRBuf->bAllocatedInternally=false;
#endif
            BDBG_MSG (("BRAP_RBUF_P_Open: For RBUF[%d] External RBUF memory is used.",
                        uiRbufIndex));
#if ( BCHP_CHIP == 3563)
			for (i=0; i<uiSize; i=i+4)
			{
				BRAP_P_DRAMWRITE ((BARC_Handle) hRBuf->hRegister, 
					((unsigned int)(hRBuf->sSettings.sExtSettings.pBufferStart) + i), 
					0);
			}
#endif
        }
		
        hRBuf->sSettings.bRbufOfClonedPort = pSettings->bRbufOfClonedPort;
		hRBuf->sSettings.sExtSettings.uiSize = uiSize;
        hRBuf->sSettings.sExtSettings.uiWaterMark = uiWaterMark;
        hRBuf->sSettings.bProgRdWrRBufAddr = pSettings->bProgRdWrRBufAddr;
    }   /* End: If not in WatchDog recovery */
    else
    {
        hRBuf = *phRBuf;
    }
    ret = BRAP_P_ConvertAddressToOffset (hRBuf->hHeap, 
            hRBuf->sSettings.sExtSettings.pBufferStart, &ui32PhyAddr);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_RBUF_P_Open: Unable to convert RBUF %d start address to physical address", uiRbufIndex));
        /* Free the RBUF Handle memory*/
        BKNI_Free (hRBuf);         
        return BERR_TRACE(ret);
    }

    BDBG_MSG (("BRAP_RBUF_P_Open: For RBUF[%d], StartAddr(virtual)=0x%x, StartAddr(Physical)=0x%x, Size(in bytes)=0x%x, WaterMark(percentage)=%d", 
                    uiRbufIndex, 
                    hRBuf->sSettings.sExtSettings.pBufferStart,
                    ui32PhyAddr,
                    hRBuf->sSettings.sExtSettings.uiSize, 
                    hRBuf->sSettings.sExtSettings.uiWaterMark));
    /* program the base address */
    /* Note: JWORD address has to be programmed into the BASEADDR register. 
     * so dont use BCHP_FIELD_DATA */
    BRAP_Write32 (hRBuf->hRegister,  
                 (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset), 
                  ui32PhyAddr);    
    
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
    /* program the end address */
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        SOURCE_RINGBUF_ENDADDR, 
                        ui32PhyAddr + hRBuf->sSettings.sExtSettings.uiSize )); 

    BDBG_MSG(("BRAP_RBUF_P_Open: End addr (physical)=0x%x", ui32RegVal));
    BRAP_Write32 (hRBuf->hRegister,  
                 (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
                  ui32RegVal);
#else
    /* program the end address */
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        RINGBUF_ENDADDR, 
                        (ui32PhyAddr + hRBuf->sSettings.sExtSettings.uiSize - 1))); 
    BDBG_MSG(("BRAP_RBUF_P_Open: End addr (physical)=0x%x", ui32RegVal));
    BRAP_Write32 (hRBuf->hRegister,  
                 (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
                  ui32RegVal);
#endif

	/* Read - Write addresses are required in PCM after channel open. */
	/* Application may chose to write PCM data before calling a channel start */
	if (true == hRBuf->sSettings.bProgRdWrRBufAddr)
	{
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
		ui32RegVal = (BCHP_FIELD_DATA (
	                  AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
	                  SRING_RDADDR, 
	                  ui32PhyAddr));       
		BRAP_Write32 (hRBuf->hRegister,  
	                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset, 
	                  ui32RegVal);


		ui32RegVal = (BCHP_FIELD_DATA (
	                  AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
	                  SOURCE_RING_WRADDR, 
	                  ui32PhyAddr));       
		
	    BRAP_Write32 (hRBuf->hRegister,  
	                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
	                  ui32RegVal);
#else /* 7401 & 7411 D0 & 7400 & 7118*/
    	ui32RegVal = (BCHP_FIELD_DATA (
	                  AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
	                  RINGBUF_RDADDR, 
	                  ui32PhyAddr));       
		BRAP_Write32 (hRBuf->hRegister,  
	                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset, 
	                  ui32RegVal);


		ui32RegVal = (BCHP_FIELD_DATA (
	                  AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
	                  RINGBUF_WRADDR, 
	                  ui32PhyAddr));       
		
	    BRAP_Write32 (hRBuf->hRegister,  
	                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
	                  ui32RegVal);
#endif
	}
   
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  
        
        /*4. Store Ring buffer handle inside the FMM handle */
        hFmm->hRBuf[uiRbufIndex] = hRBuf;
    
        *phRBuf = hRBuf;
    }     
    
    BDBG_MSG (("BRAP_RBUF_P_Open: New RBUF Handle = 0x%x", hRBuf));
    
    BDBG_LEAVE (BRAP_RBUF_P_Open);
    return ret;
}


BERR_Code BRAP_RBUF_P_Start (
    BRAP_RBUF_P_Handle   hRBuf,          /* [in] RBUF handle */
    const BRAP_RBUF_P_Params * pParams   /* [in] Start time settings */  
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_RBUF_P_Start);
    
    BDBG_ASSERT (hRBuf);
   
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hRBuf->hFmm) == false)
    {   /* If not in WatchDog recovery */  
        
        BDBG_ASSERT (pParams);
    
        BDBG_MSG (("BRAP_RBUF_P_Start:"
               "hRBuf=0x%x, RBuf index=%d," 
               "pParams.uiStartWRPoint=%d",
                hRBuf, hRBuf->uiIndex, pParams->uiStartWRPoint));
    
        /* Store the start parameters inside the handle */
        hRBuf->sParams = *pParams; 
    }
   
    /* Configure RBuf Hardware */
    ret = BRAP_RBUF_P_HWConfig (hRBuf);

    BDBG_LEAVE (BRAP_RBUF_P_Start);
    return ret;
}

BERR_Code BRAP_RBUF_P_Stop (
    BRAP_RBUF_P_Handle    hRBuf     /* [in] RBUF handle */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_RBUF_P_Stop);
    BDBG_ASSERT (hRBuf);
    BSTD_UNUSED (hRBuf);
    
    BDBG_MSG (("BRAP_RBUF_P_Stop:"
               "hRBuf=0x%x, RBuf index=%d ",
                hRBuf, hRBuf->uiIndex));

    BDBG_LEAVE (BRAP_RBUF_P_Stop);
    return ret;
}

BERR_Code BRAP_RBUF_P_Close ( 
    BRAP_RBUF_P_Handle      hRBuf   /* [in] RBUF handle */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_RBUF_P_Close);
    BDBG_ASSERT (hRBuf);
    
    BDBG_MSG (("BRAP_RBUF_P_Close:"
               "hRBuf=0x%x, RBuf index=%d ",
                hRBuf, hRBuf->uiIndex));
    
    /* Remove referrence to this Rbuf from the parent FMM */ 
    hRBuf->hFmm->hRBuf[hRBuf->uiIndex] = NULL;

    /* For cloned port Rbuf does not have any memory allocated to it.*/
    if (hRBuf->sSettings.bRbufOfClonedPort == false)
    {
	    /* Free the ring buffer memory */
#if (BRAP_SECURE_HEAP==1)
		BRAP_P_Free(hRBuf->hFmm->hRap, (void *) hRBuf->sSettings.sExtSettings.pBufferStart,true);
#else
#if (  BRAP_DVD_FAMILY == 1) || ( BCHP_CHIP == 3563) || ( BRAP_7405_FAMILY == 1)		
		if(true == hRBuf->bAllocatedInternally) 
		{
			BDBG_MSG(("Freeing the Mem %#x",hRBuf->sSettings.sExtSettings.pBufferStart));
	    	BRAP_P_Free(hRBuf->hFmm->hRap, (void *) hRBuf->sSettings.sExtSettings.pBufferStart);
		}
		else
			BDBG_MSG(("Not Freeing the Mem.%#x allocated externally",hRBuf->sSettings.sExtSettings.pBufferStart));
#else	
	   	BRAP_P_Free(hRBuf->hFmm->hRap, (void *) hRBuf->sSettings.sExtSettings.pBufferStart);
#endif		
#endif

    }
	
    /* Free the RBUF Handle memory*/
    BKNI_Free (hRBuf); 
                 
    BDBG_LEAVE (BRAP_RBUF_P_Close);
    return ret;
}


/***************************************************************************
Summary:
    Configures the HW registers for the Ring Buffer

Description:


Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
static BERR_Code BRAP_RBUF_P_HWConfig (
    BRAP_RBUF_P_Handle  hRBuf  		/* [in] RBUF handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;
    uint32_t  ui32BaseAddr; /* Physical address of RBUF base */
    BREG_Handle hRegister;
    uint32_t   ui32WaterMark;

#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
    BSTD_UNUSED(ui32WaterMark);
#endif

    BDBG_ENTER (BRAP_RBUF_P_HWConfig);
    BDBG_ASSERT (hRBuf);
    
    BDBG_MSG (("BRAP_RBUF_P_HWConfig:"
               "hRBuf=0x%x, RBuf index=%d ",
                hRBuf, hRBuf->uiIndex));
   
    hRegister = hRBuf->hRegister; 
    
    ui32BaseAddr = BRAP_Read32 (hRegister,
                            BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + 
                            hRBuf->ui32Offset);
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   

    /* Set RDADDR and WRADDR equal to BASEADDR Only for a decode channel */ 
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        SRING_RDADDR, 
                        ui32BaseAddr));       
   	BRAP_Write32 (hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

	    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        SOURCE_RING_WRADDR, 
                        ui32BaseAddr));       
    	BRAP_Write32 (hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
                  ui32RegVal);
	
    /* Workaround for PR15887: program the end address */
	 ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        SOURCE_RINGBUF_ENDADDR, 
                        ui32BaseAddr + hRBuf->sSettings.sExtSettings.uiSize )); 
    BDBG_MSG(("BRAP_RBUF_P_HWConfig: Base addr (physical)=0x%x End addr (physical)=0x%x", ui32BaseAddr, ui32RegVal));

    BRAP_Write32 (hRBuf->hRegister,  
                 (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
                  ui32RegVal);
    
    /* Configure water mark : note:uiWaterMark is %  */
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_FREEBYTE_MARK, 
                        SOURCE_RINGBUF_FREEBYTE_MARK, 
                        (hRBuf->sSettings.sExtSettings.uiWaterMark
                         * hRBuf->sSettings.sExtSettings.uiSize)/100));       
    BRAP_Write32 (hRegister,  
                 BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEBYTE_MARK + hRBuf->ui32Offset , 
                 ui32RegVal);

    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT, 
                        SOURCE_RINGBUF_START_WRPOINT, 
                        (uint32_t)(hRBuf->sParams.uiStartWRPoint)
                        + ui32BaseAddr));       
	
    BRAP_Write32 (hRegister,  
                   (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT 
                   + hRBuf->ui32Offset), 
                  ui32RegVal);
#else
    /* for 7401,7411 D0 etc*/
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        RINGBUF_RDADDR, 
                        ui32BaseAddr));       
   	BRAP_Write32 (hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

	    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        RINGBUF_WRADDR, 
                        ui32BaseAddr));       
	    BRAP_Write32 (hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

    /* Workaround for PR15887: program the end address */
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
                        RINGBUF_ENDADDR, 
                        ui32BaseAddr + hRBuf->sSettings.sExtSettings.uiSize - 1)); 
    BDBG_MSG(("BRAP_RBUF_P_HWConfig: Base addr (physical)=0x%x End addr (physical)=0x%x", ui32BaseAddr, ui32RegVal));

    /* Calculate the Water Mark Level depending on the type of the System 
       If Edge Triggered system, the application provided Water Mark is Ignored 
       and internallyforced to be (100-FrameSize)%, where FrameSize is also in 
       percentage. Currently the FrameSize used is 25%.
       If Level Triggered System, the user provided Water Mark is Used.*/
#if 0       
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    BRAP_RBUF_P_GetFrameSize(hRBuf->sSettings.sExtSettings.uiSize, &uiFrameSize);
    ui32WaterMark = hRBuf->sSettings.sExtSettings.uiSize - uiFrameSize + 1;
#else
#endif
#endif
    ui32WaterMark = (hRBuf->sSettings.sExtSettings.uiWaterMark
                         * hRBuf->sSettings.sExtSettings.uiSize)/100;


    BRAP_Write32 (hRBuf->hRegister,  
                 (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
                  ui32RegVal);
  
    /* Configure water mark : note:uiWaterMark is %  */
    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK, 
                        RINGBUF_FREEFULL_MARK,
                        ui32WaterMark));
                            
    BRAP_Write32 (hRegister,  
                 BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + hRBuf->ui32Offset , 
                  ui32RegVal);

    ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT, 
                        RINGBUF_START_WRPOINT, 
                        (uint32_t)(hRBuf->sParams.uiStartWRPoint)
                          + ui32BaseAddr));       

    BRAP_Write32 (hRegister,  
                   (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT 
                   + hRBuf->ui32Offset), 
                  ui32RegVal);    
      
#endif 
    BDBG_LEAVE (BRAP_RBUF_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
	Gets the base and end pointers of the ring buffer.

Description:
	Gets the base and end pointers of the ring buffer associated with an 
	output audio channel for a RAP Channel. This is a debug API.

Returns:
	BERR_SUCCESS 

See Also:
	

****************************************************************************/
BERR_Code 
BRAP_RBUF_GetBaseAndEndAddress( 
            BRAP_ChannelHandle hRapCh,        /* [in] Audio Device handle */
            BRAP_OutputChannel eOpCh,    /* [in] Output channel type */
            uint32_t         *pBaseAddr,      /* [Out] Ring buffer base address */   
            uint32_t         *pEndAddr        /* [Out] Ring buffer end address */ 
            )
{

#if (BCHP_7411_VER != BCHP_VER_C0) 
	BSTD_UNUSED(hRapCh);
	BSTD_UNUSED(eOpCh);
	BSTD_UNUSED(pBaseAddr);
	BSTD_UNUSED(pEndAddr);
	return BERR_TRACE (BERR_NOT_SUPPORTED);
#else

    BRAP_RBUF_P_Handle    hRBuf;

	BDBG_ENTER (BRAP_RBUF_GetReadPointer);
    BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (pBaseAddr);
	BDBG_ASSERT (pEndAddr);

    /* Get the appropriate ring buffer handle */
    hRBuf = hRapCh->sModuleHandles.hRBuf[eOpCh];

    /* Assert on hRBuf */
	BDBG_ASSERT (hRBuf);

    /* Read the base and end addresses from the registers */
    *pBaseAddr = BRAP_Read32(hRBuf->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + 
                            hRBuf->ui32Offset);
	*pEndAddr = BRAP_Read32(hRBuf->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + 
                            hRBuf->ui32Offset);
	
    BDBG_LEAVE(BRAP_RBUF_GetBaseAndEndAddress);

    return BERR_SUCCESS;    
#endif /* 7401 etc*/
}

BERR_Code BRAP_RBUF_P_GetFrameSize(
    unsigned int uiRBufSize, 
    unsigned int *pFrameSize
    )
{
    BDBG_ASSERT(pFrameSize);

    /* Each Ring buffer is logically divided into 4 frames. 
       This is also the max free contiguous ring buffer size 
       in bytes that is informed to the user */       
    *pFrameSize = uiRBufSize >> 2;
    
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Configures the End address of the the Ring Buffer

Description:


Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
BERR_Code BRAP_RBUF_P_ProgramEndAddress (
    BRAP_RBUF_P_Handle  hRBuf  		/* [in] RBUF handle */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;
	uint32_t  ui32BaseAddr; /* Physical address of RBUF base */	
	BREG_Handle hRegister;
	uint32_t   ui32WaterMark;

	BDBG_ENTER (BRAP_RBUF_P_ProgramEndAddress);
	BDBG_ASSERT (hRBuf);

	hRegister = hRBuf->hRegister; 

	ui32BaseAddr = BRAP_Read32 (hRegister,
					BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + 
					hRBuf->ui32Offset);

#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
	ui32RegVal = (BCHP_FIELD_DATA (
					AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
					SOURCE_RINGBUF_ENDADDR, 
					ui32BaseAddr + hRBuf->sParams.uiStartSize)); 

	BDBG_MSG(("BRAP_RBUF_P_HWConfig: Base addr (physical)=0x%x End addr (physical)=0x%x", ui32BaseAddr, ui32RegVal));

	BRAP_Write32 (hRBuf->hRegister,  
		(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
		ui32RegVal);
    
	/* Configure water mark : note:uiWaterMark is %  */
	ui32RegVal = (BCHP_FIELD_DATA (
					AUD_FMM_BF_CTRL_RINGBUF_0_FREEBYTE_MARK, 
					SOURCE_RINGBUF_FREEBYTE_MARK, 
					(hRBuf->sSettings.sExtSettings.uiWaterMark
					* hRBuf->sParams.uiStartSize)/100));       

	BRAP_Write32 (hRegister,  
		BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEBYTE_MARK + hRBuf->ui32Offset , 
		ui32RegVal);
#else    /* for 7401,7411 D0 etc*/
	ui32RegVal = (BCHP_FIELD_DATA (
	                    AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
	                    RINGBUF_ENDADDR, 
	                    ui32BaseAddr +  hRBuf->sParams.uiStartSize - 1)); 
	BDBG_MSG(("BRAP_RBUF_P_HWConfig: Base addr (physical)=0x%x End addr (physical)=0x%x", ui32BaseAddr, ui32RegVal));

	ui32WaterMark = (hRBuf->sSettings.sExtSettings.uiWaterMark
	                     * hRBuf->sParams.uiStartSize)/100;


	BRAP_Write32 (hRBuf->hRegister,  
	 			(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset), 
	 			ui32RegVal);
  
	/* Configure water mark : note:uiWaterMark is %  */
	ui32RegVal = (BCHP_FIELD_DATA (
	                    AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK, 
	                    RINGBUF_FREEFULL_MARK,
	                    ui32WaterMark));
                            
	BRAP_Write32 (hRegister,  
	             BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + hRBuf->ui32Offset , 
	              ui32RegVal);
#endif

	BDBG_LEAVE (BRAP_RBUF_P_ProgramEndAddress);
	return ret;

}

/* End of File */
