/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*	This file contains the implementations of the APIs for a decode channel, 
*   which are exposed to the upper layer by the Raptor Audio PI. This file 
*   is part of Audio Manager Module. Audio Manager module is the top level 
*   module of the RAP PI, which interfaces with the caller and manages the 
*   internal channel objects to get RAP PI produce the desired result.
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/
#include "brap.h"
#include "brap_priv.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(rap_vid);		/* Register software module with debug interface */

/***************************************************************************
Summary:
	Private API used to Initialize VIDEO Decode channel settings.

Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_P_InitVideoSettings( 
    BRAP_ChannelHandle              hRapCh,		    /* [in] Raptor Decode 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	)
{
    BERR_Code   ret=BERR_SUCCESS;

    BDBG_ENTER(BRAP_P_InitVideoSettings);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pChnSettings);

    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(pChnSettings);
    
    BDBG_LEAVE(BRAP_P_InitVideoSettings);
    return ret;
}


BERR_Code
BRAP_VID_GetPictureCount_isr(
         BRAP_ChannelHandle hRapCh,
         uint32_t *puiPictureCount
         )
{
    BRAP_FWIF_MsgQueueHandle hPicDelQueue = NULL;
   BDBG_ENTER( BRAP_VID_GetPictureCount_isr );

   BDBG_ASSERT(hRapCh);

   if (hRapCh->eState == BRAP_P_State_eStarted)
   {
        hPicDelQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hPDQueue;        
        BRAP_VID_P_GetPictureCount(hPicDelQueue,puiPictureCount);
   }
   else
   {
        BDBG_ERR(("Channel not started"));
        *puiPictureCount = 0;
   }

   BDBG_MSG(("GetPictureCount_isr:: count = %d\n", *puiPictureCount ));
   BDBG_LEAVE( BRAP_VID_GetPictureCount_isr );
   
   return BERR_SUCCESS;
}

BERR_Code
BRAP_VID_P_GetPictureCount(BRAP_FWIF_MsgQueueHandle hPicDelQueue,uint32_t *puiPictureCount)
{

	BERR_Code err=BERR_SUCCESS;
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0; 
   		
    BDBG_ENTER(BRAP_VID_P_GetPictureCount);

	BDBG_ASSERT(hPicDelQueue);
    
	ui32dramReadPtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hPicDelQueue->ui32DspOffset);
	
	ui32dramWritePtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hPicDelQueue->ui32DspOffset);

	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	
    ui32maskReadPtr=ui32dramReadPtr;
    ui32maskWritePtr=ui32dramWritePtr;
    
    /*Sanity check*/
	/*Checking boundness of read pointer- if((readptr>endaddr) OR (readptr<baseaddr)) read ptr not within bound*/
	if((ui32maskReadPtr>hPicDelQueue->uiEndAddr)||(ui32maskReadPtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Read pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
	/*Checking boundness of write pointer - if((writeptr>endaddr) OR (writeptr<baseaddr))  write ptr not within bound*/
	if((ui32maskWritePtr>hPicDelQueue->uiEndAddr)||(ui32maskWritePtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Write pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	/* checking write ptrs */
	if(hPicDelQueue->uiReadPtr!=ui32maskReadPtr)
	{
		BDBG_ERR(("Read pointer corrupted in the Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);	
	}
    /* End of Sanity Check */

	
	/*Calc Picture count in the message queue*/

	/* Case1:if(maskwriteptr>maskreadptr)*/
	if(ui32maskWritePtr > ui32maskReadPtr)
	{	
		*puiPictureCount = (ui32maskWritePtr - ui32maskReadPtr)>>2;/* Diving by 4*/
	}
	

	/* Case2:if(maskreadptr>maskwriteptr) */
	if(ui32maskReadPtr>ui32maskWritePtr)
	{
		*puiPictureCount = ((hPicDelQueue->uiEndAddr-ui32maskReadPtr)+
                            (ui32maskWritePtr-hPicDelQueue->uiBaseAddr))>>2;
	}

	
	/* Case3:if(maskreadptr==maskwriteptr) */
	if(ui32maskReadPtr==ui32maskWritePtr)
	{
		/* The buffer is empty */
		*puiPictureCount = 0x0;
	}
    
    BDBG_LEAVE(BRAP_VID_P_GetPictureCount);
    return err;

}

BERR_Code
BRAP_VID_PeekAtPicture_isr(
         BRAP_ChannelHandle hRapCh,
         uint32_t uiIndex,
         BXDM_Picture **pUnifiedPicture
         )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_FWIF_MsgQueueHandle hPicDelQueue = NULL;
    uint32_t uiUPBPhysicalAddr=0;
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0;
	int32_t i32BytesToBeRead=0;
	void *pvMsgQueueReadAddr=NULL;
    

   BDBG_ENTER( BRAP_VID_PeekAtPicture_isr );
    BDBG_ASSERT(hRapCh);
    /* TODO: add code to use the index */
    BSTD_UNUSED( uiIndex );

    hPicDelQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hPDQueue;
	
	ui32dramReadPtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hPicDelQueue->ui32DspOffset);

	ui32dramWritePtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hPicDelQueue->ui32DspOffset);
       
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	ui32maskReadPtr=ui32dramReadPtr;
	ui32maskWritePtr=ui32dramWritePtr;
	   
	 /*Sanity check*/
	/* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr is not within bound*/	
	if((ui32maskWritePtr>hPicDelQueue->uiEndAddr)||(ui32maskWritePtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Write pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	
	/* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound*/
	if((ui32maskReadPtr>hPicDelQueue->uiEndAddr)||(ui32maskReadPtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Read pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
	/*Updating write ptr */
	hPicDelQueue->uiWritePtr= ui32maskWritePtr;

	/* checking read ptrs to see if they are the same */
	if((hPicDelQueue->uiReadPtr)!=ui32maskReadPtr)
	{
		BDBG_ERR(("Read pointer corrupted in the Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE );		
	}

	/* End of Sanity Check */

	/*******************************************************************
   Different cases: 

    If maskreadptr>maskwriteptr
     ReadPtrMSB   WritePtrMSB 		
        0   				0                   This condn. is not possible
        0   				1   	     		(end-read)+(write-base)
        1   				0   	              (end-read)+(write-base)
        1   				1   	     	       this condn. is not possible

  If maskwriteptr>maskreadptr
   ReadptrMSB 	WritePtrMSB	 
    0   				0   		  	write-read
    0   				1   		       this condn. not possible
    1   				0   		       this condn. not possible
    1   				1   		       write-read

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,no message to read
  If the toggle bits are different all the messages have to be read
   

 ***********************************************************************/
	/*Condn. for reading messages from the message queue into the message buffer*/
	/* If no msg is to be read, generate a BRAP_ERR_BUFFER_EMPTY error(new error defined in brap.h)*/

	/* Checking if a msg is present */
	
	/* Case1:if(readptr>writeptr)*/
	if(ui32maskReadPtr > ui32maskWritePtr)
	{	
		i32BytesToBeRead=(hPicDelQueue->uiEndAddr-ui32maskReadPtr)+
                (ui32maskWritePtr-hPicDelQueue->uiBaseAddr);
	}

	 /* Case2:if(writeptr>readptr) */
	if(ui32maskWritePtr>ui32maskReadPtr)
	{
		i32BytesToBeRead=ui32maskWritePtr-ui32maskReadPtr;
	}

    /*Case 3:if readptr == writeptr */
    if(ui32maskWritePtr ==ui32maskReadPtr)
    {	
        /*All messages have to be read*/
        i32BytesToBeRead=hPicDelQueue->uiEndAddr-hPicDelQueue->uiBaseAddr;
    }
	 if(i32BytesToBeRead < 0)
	 {
		BDBG_ERR(("The Message Queue is empty.No message is present."));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BUFFER_EMPTY);
	 }
	 
	BDBG_MSG(("ui32maskReadPtr = 0x%x",ui32maskReadPtr));
	/*This conversion is done to pass a Phaysical address to BRAP_MemRead32_isr*/
	if(BERR_SUCCESS!=(ret=BRAP_ConvertOffsetToAddress_isr(hPicDelQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(ret);
	
	/*Reading Message from the message queue into the message buffer*/
	uiUPBPhysicalAddr=BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr);

    BDBG_MSG(("uiUPBPhysicalAddr = 0x%x",uiUPBPhysicalAddr));

	if(BERR_SUCCESS!=(ret=BRAP_ConvertOffsetToAddress_isr(hPicDelQueue->hHeap,uiUPBPhysicalAddr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(ret);

    *pUnifiedPicture = pvMsgQueueReadAddr;

    BDBG_MSG(("*pUnifiedPicture = 0x%x",*pUnifiedPicture));

   BDBG_LEAVE( BRAP_VID_PeekAtPicture_isr );
   return BERR_SUCCESS;
}

BERR_Code
BRAP_VID_GetNextPicture_isr(
         BRAP_ChannelHandle hRapCh,
         BXDM_Picture **pUnifiedPicture
         )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_FWIF_MsgQueueHandle hPicDelQueue = NULL;
    bool bReadUpdate = true; 
    uint32_t uiUPBPhysicalAddr=0;
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0;
	int32_t i32BytesToBeRead=0;
	void *pvMsgQueueReadAddr=NULL;    

    BDBG_ASSERT(hRapCh);
    
   BDBG_ENTER( BRAP_VID_GetNextPicture_isr );

    hPicDelQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hPDQueue;

	 
	ui32dramReadPtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hPicDelQueue->ui32DspOffset);

	ui32dramWritePtr=BRAP_Read32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hPicDelQueue->ui32DspOffset);
       
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	ui32maskReadPtr=ui32dramReadPtr;
	ui32maskWritePtr=ui32dramWritePtr;
	   
	 /*Sanity check*/
	/* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr is not within bound*/	
	if((ui32maskWritePtr>hPicDelQueue->uiEndAddr)||(ui32maskWritePtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Write pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	
	/* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound*/
	if((ui32maskReadPtr>hPicDelQueue->uiEndAddr)||(ui32maskReadPtr<hPicDelQueue->uiBaseAddr))
	{
		BDBG_ERR(("Read pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
	/*Updating write ptr */
	hPicDelQueue->uiWritePtr= ui32maskWritePtr;

	/* checking read ptrs to see if they are the same */
	if((hPicDelQueue->uiReadPtr)!=ui32maskReadPtr)
	{
		BDBG_ERR(("Read pointer corrupted in the Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE );		
	}

	/* End of Sanity Check */

	/*******************************************************************
   Different cases: 

    If maskreadptr>maskwriteptr
     ReadPtrMSB   WritePtrMSB 		
        0   				0                   This condn. is not possible
        0   				1   	     		(end-read)+(write-base)
        1   				0   	              (end-read)+(write-base)
        1   				1   	     	       this condn. is not possible

  If maskwriteptr>maskreadptr
   ReadptrMSB 	WritePtrMSB	 
    0   				0   		  	write-read
    0   				1   		       this condn. not possible
    1   				0   		       this condn. not possible
    1   				1   		       write-read

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,no message to read
  If the toggle bits are different all the messages have to be read
   

 ***********************************************************************/
	/*Condn. for reading messages from the message queue into the message buffer*/
	/* If no msg is to be read, generate a BRAP_ERR_BUFFER_EMPTY error(new error defined in brap.h)*/

	/* Checking if a msg is present */
	
	/* Case1:if(readptr>writeptr)*/
	if(ui32maskReadPtr > ui32maskWritePtr)
	{	
		i32BytesToBeRead=(hPicDelQueue->uiEndAddr-ui32maskReadPtr)+
                (ui32maskWritePtr-hPicDelQueue->uiBaseAddr);
	}

	 /* Case2:if(writeptr>readptr) */
	if(ui32maskWritePtr>ui32maskReadPtr)
	{
		i32BytesToBeRead=ui32maskWritePtr-ui32maskReadPtr;
	}

    /*Case 3:if readptr == writeptr */
    if(ui32maskWritePtr ==ui32maskReadPtr)
    {	
        /*All messages have to be read*/
        i32BytesToBeRead=hPicDelQueue->uiEndAddr-hPicDelQueue->uiBaseAddr;
    }
	 if(i32BytesToBeRead < 0)
	 {
		BDBG_ERR(("The Message Queue is empty.No message is present."));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BUFFER_EMPTY);
	 }
	 
	/*This conversion is done to pass a Phaysical address to BRAP_MemRead32_isr*/
	if(BERR_SUCCESS!=(ret=BRAP_ConvertOffsetToAddress_isr(hPicDelQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(ret);
	
	/*Reading Message from the message queue into the message buffer*/
	uiUPBPhysicalAddr=BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr);

    BDBG_MSG(("In BRAP_VID_GetNextPicture_isr uiUPBPhysicalAddr = 0x%x",uiUPBPhysicalAddr));

	if(BERR_SUCCESS!=(ret=BRAP_ConvertOffsetToAddress_isr(hPicDelQueue->hHeap,uiUPBPhysicalAddr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(ret);

    *pUnifiedPicture = pvMsgQueueReadAddr;

    BDBG_MSG(("In BRAP_VID_GetNextPicture_isr *pUnifiedPicture = 0x%x",*pUnifiedPicture));
    
    if (bReadUpdate == true)
    {
    	ui32dramReadPtr=ui32dramReadPtr+4;    
        if(ui32dramReadPtr==hPicDelQueue->uiEndAddr)
            ui32dramReadPtr=hPicDelQueue->uiBaseAddr;

    	/*updating read ptr in the Queue Attribute Structure*/
    	BRAP_Write32_isr(hPicDelQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hPicDelQueue->ui32FifoId + 
                        BRAP_FWIF_P_FIFO_READ_OFFSET + hPicDelQueue->ui32DspOffset, ui32dramReadPtr); 
    	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
    	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
    		
    	/*updating read ptr in the handle*/
    	hPicDelQueue->uiReadPtr = ui32dramReadPtr;
    }

   BDBG_LEAVE( BRAP_VID_GetNextPicture_isr );
    return ret;
}

BERR_Code
BRAP_VID_ReleasePicture_isr(
         BRAP_ChannelHandle hRapCh,
         BXDM_Picture *pUnifiedPicture
         )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int    uiBuffSize = 4;
    BRAP_FWIF_MsgQueueHandle hPicDisQueue = NULL;
    uint32_t    temp = (uint32_t)pUnifiedPicture;
    
   BDBG_ENTER( BRAP_VID_ReleasePicture_isr );
    BDBG_ASSERT(hRapCh);

    hPicDisQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hDSQueue;

    BDBG_MSG(("In BRAP_VID_ReleasePicture_isr pUnifiedPicture = 0x%x",pUnifiedPicture));
    ret = BRAP_P_WriteMsg(hPicDisQueue, &temp,uiBuffSize);

   BDBG_LEAVE( BRAP_VID_ReleasePicture_isr );
    return ret;
}

BERR_Code
BRAP_VID_GetPictureDropPendingCount_isr(
         BRAP_ChannelHandle hRapCh,
         uint32_t *puiPictureDropPendingCount
         )
{
   BDBG_ENTER( BRAP_VID_GetPictureDropPendingCount_isr );
   BSTD_UNUSED(hRapCh);
   BSTD_UNUSED(puiPictureDropPendingCount);

   BDBG_LEAVE( BRAP_VID_GetPictureDropPendingCount_isr );
   return BERR_SUCCESS;
}

BERR_Code
BRAP_VID_RequestPictureDrop_isr(
         BRAP_ChannelHandle hRapCh,
         uint32_t uiPictureDropRequestCount
         )
{
   BDBG_ENTER( BRAP_VID_RequestPictureDrop_isr );
   BSTD_UNUSED(hRapCh);   
   BSTD_UNUSED(uiPictureDropRequestCount);   

   BDBG_LEAVE( BRAP_VID_RequestPictureDrop_isr );
   return BERR_SUCCESS;
}

BERR_Code
BRAP_VID_DisplayInterruptEvent_isr(
         BRAP_ChannelHandle hRapCh
         )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_FWIF_MsgQueueHandle hPicDisQueue = NULL,hPicRelQueue = NULL;
    bool bRealUpdate = true;
    unsigned int    uiBuffSize = 4; 
    uint32_t puiPictureCount=0;
    uint32_t pGetUPB=0;
   BDBG_ENTER( BRAP_VID_DisplayInterruptEvent_isr );
    BDBG_ASSERT(hRapCh);

   if (hRapCh->eState == BRAP_P_State_eStarted)
   {
        hPicDisQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hDSQueue;        
        BRAP_VID_P_GetPictureCount(hPicDisQueue,&puiPictureCount);
   }
   else
   {
        BDBG_ERR(("Channel not started"));
        puiPictureCount = 0;
   }

    BDBG_MSG(("BRAP_VID_DisplayInterruptEvent pic count in disply queue = %d",puiPictureCount));

    if(puiPictureCount != 0)
    {
        uint32_t    physaddr=0;
        ret= BRAP_P_GetMsg(hPicDisQueue,&pGetUPB,bRealUpdate);
         
        BRAP_P_ConvertAddressToOffset(hPicDisQueue->hHeap,(void *)pGetUPB,&physaddr);
        BDBG_MSG(("pGetUPB = 0x%x physaddr = 0x%x\n",pGetUPB,physaddr));
        
        hPicRelQueue = hRapCh->hRap->hDsp[BRAP_RM_P_MAX_DSPS-1]->hPRQueue;
        ret = BRAP_P_WriteMsg(hPicRelQueue, &physaddr,uiBuffSize);
    }
   BDBG_LEAVE( BRAP_VID_DisplayInterruptEvent_isr );
   return BERR_SUCCESS;
}


BERR_Code
BRAP_VID_GetDMInterface(
         BRAP_ChannelHandle hRapCh,
         BXDM_Decoder_Interface *pstDecoderInterface,
         void **pPrivateContext
         )
{
   BDBG_ENTER( BRAP_VID_P_GetDMInterface );

   BDBG_ASSERT( pstDecoderInterface );
   BDBG_ASSERT( pPrivateContext );

   pstDecoderInterface->getPictureCount_isr = (BXDM_Decoder_GetPictureCount_isr) BRAP_VID_GetPictureCount_isr;
   pstDecoderInterface->peekAtPicture_isr = (BXDM_Decoder_PeekAtPicture_isr) BRAP_VID_PeekAtPicture_isr;
   pstDecoderInterface->getNextPicture_isr = (BXDM_Decoder_GetNextPicture_isr) BRAP_VID_GetNextPicture_isr;
   pstDecoderInterface->releasePicture_isr = (BXDM_Decoder_ReleasePicture_isr) BRAP_VID_ReleasePicture_isr;
   pstDecoderInterface->getPictureDropPendingCount_isr = (BXDM_Decoder_GetPictureDropPendingCount_isr) BRAP_VID_GetPictureDropPendingCount_isr;
   pstDecoderInterface->requestPictureDrop_isr = (BXDM_Decoder_RequestPictureDrop_isr) BRAP_VID_RequestPictureDrop_isr;
   pstDecoderInterface->displayInterruptEvent_isr = (BXDM_Decoder_DisplayInterruptEvent_isr) BRAP_VID_DisplayInterruptEvent_isr;

   *pPrivateContext = hRapCh;

   BDBG_LEAVE( BRAP_VID_P_GetDMInterface );
   return BERR_TRACE( BERR_SUCCESS );
}


/***************************************************************************
Summary:
	Private API used to open a VIDEO Decode channel.
Description:
	It is used to instantiate a VIDEO decode channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_VID_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              hRapCh,		    /* [in] Raptor Decode 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	)
{
    BERR_Code   ret=BERR_SUCCESS;
    unsigned int            uiChannelNo = 0;
    bool                    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRap);
#if 0    
    BXDM_PictureProvider_Settings stPictureProviderSettings;
#endif

    BDBG_ENTER(BRAP_VID_P_ChannelOpen);
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(hRapCh);

    if(false == bWdgRecovery)
    {
    /* pChnSettings is valid only when not in watchdog recovery */
        BDBG_ASSERT(pChnSettings);
        BDBG_MSG(("BRAP_VID_P_ChannelOpen():"
                            "hRap=0x%x," 
                            "\n\t pChnSettings->eChType = %d", 
                            hRap, pChnSettings->eChType));

        /* Validate if this channel can be added to hRap */
        for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_VIDEO_DEC_CHANNELS; uiChannelNo++)
        {
            if(BRAP_P_CheckIfPtrInvalid((void *)hRap->hRapVideoDecCh[uiChannelNo]) == true)
            {
                break;
            }
        }

        if(BRAP_RM_P_MAX_VIDEO_DEC_CHANNELS == uiChannelNo)
        {
            BDBG_ERR(("Max number of Video DEC channels(%d) already exist", uiChannelNo));
            ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end_open;
        }
        hRapCh->uiChannelNo = uiChannelNo;

#if 0
        BXDM_PictureProvider_GetDefaultSettings(&stPictureProviderSettings);

        ret = BXDM_PictureProvider_Create( &hRapCh->hPictureProvider, &stPictureProviderSettings );
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_VID_P_ChannelOpen : BXDM_PictureProvider_Create failed"));
            ret = BERR_TRACE(ret);
            goto end_open;        
        }

        {
            BXDM_Decoder_Interface sDecoderInterface;
            void *pPrivateContext;

            ret = BRAP_VID_P_GetDMInterface(
                   hRapCh,
                   &sDecoderInterface,
                   &pPrivateContext
                   );

            if (ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret);
                goto end_open;
            }

            BKNI_EnterCriticalSection();
            ret = BXDM_PictureProvider_SetDecoderInterface_isr(
                   hRapCh->hPictureProvider,
                   &sDecoderInterface,
                   pPrivateContext);
            BKNI_LeaveCriticalSection();

            if (ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret);
                goto end_open;
            }
        }
#endif

    }

end_open:
	
   /* only if channel has been successfully opened, save the handle */
	if((BERR_SUCCESS == ret) && (false == bWdgRecovery))
	{
        	hRapCh->eState = BRAP_P_State_eOpened; /* Opened successfully */
        	hRap->hRapVideoDecCh[uiChannelNo] = hRapCh ;    
	}    
    
    BDBG_LEAVE(BRAP_VID_P_ChannelOpen);
    return ret;
}

/***************************************************************************
Summary:
	API used to close a Video decode channel.
Description:
	It closes the instance of a decode channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_VID_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_VID_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	)
{
    BERR_Code	    ret = BERR_SUCCESS;
    unsigned int    uiChannelNo = 0;
    BRAP_Handle 	hRap = NULL;

	BDBG_ENTER(BRAP_VID_P_ChannelClose);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    hRap = hRapCh->hRap;

    if ( NULL != hRapCh->hPictureProvider )
    {
        ret = BXDM_PictureProvider_Destroy( hRapCh->hPictureProvider );
        if(ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
        }
    }


    /* Mark the place for the current channel handle to 'invalid' inside RAP 
    handle */
    for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_VIDEO_DEC_CHANNELS; uiChannelNo++)
    {
        if(hRapCh == hRap->hRapDecCh[uiChannelNo])
        {
            break;
        }
    }	
    hRapCh->hRap->hRapVideoDecCh[hRapCh->uiChannelNo] = NULL;

	/* Free the channel handle */
	/* BKNI_Free(hRapCh);*/

	BDBG_LEAVE(BRAP_VID_P_ChannelClose);
	return(ret);
}



BERR_Code BRAP_VID_P_ConfigureXDM ( 
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams	/* [in] Audio parameters for 
	                                               starting this channel */
	)
{
    BERR_Code ret = BERR_SUCCESS;
    BXDM_PictureProvider_PulldownMode ePulldownMode;
    BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings stXdmDihPictureProviderSettings;

    BDBG_ENTER(BRAP_VID_P_ConfigureXDM);

    ret = BRAP_VID_SetPictureDropMode(hRapCh,pAudioParams->sVideoParams.ePictureDropMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BRAP_VID_SetPtsStcDiffThreshold(hRapCh,pAudioParams->sVideoParams.uiVsyncDiffThreshDefault);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BRAP_VID_SetMonitorRefreshRate(hRapCh,pAudioParams->sVideoParams.eMonitorRefreshRate);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BRAP_VID_Set1080pScanMode(hRapCh,pAudioParams->sVideoParams.e1080pScanMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    BKNI_EnterCriticalSection();
    ret = BXDM_PictureProvider_SetProtocol_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.eVideoCmprStd);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret =  BXDM_PictureProvider_SetCRCMode_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.bCrcMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    switch ( pAudioParams->sVideoParams.ePulldownMode)
    {
        case BRAP_VID_PulldownMode_eBottomTop:
        ePulldownMode = BXDM_PictureProvider_PulldownMode_eBottomTop;
        break;

        case BRAP_VID_PulldownMode_eUseEncodedFormat:
        ePulldownMode = BXDM_PictureProvider_PulldownMode_eUseEncodedFormat;
        break;

        default:
        BDBG_ERR(("Unknown progressive override mode: %d",pAudioParams->sVideoParams.ePulldownMode));

        case BRAP_VID_PulldownMode_eTopBottom:
        ePulldownMode = BXDM_PictureProvider_PulldownMode_eTopBottom;
        break;
    }

    ret = BXDM_PictureProvider_Set480pPulldownMode_isr(hRapCh->hPictureProvider,ePulldownMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BXDM_PictureProvider_Set1080pPulldownMode_isr(hRapCh->hPictureProvider,ePulldownMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BXDM_PictureProvider_SetRemovalDelay_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.ulRemovalDelay);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    if ( pAudioParams->sVideoParams.uiPreRollRate > 1 )
    {
        BXDM_Picture_Rate stPreRollRate;
        BKNI_Memset(&stPreRollRate, 0, sizeof( BXDM_Picture_Rate ) );

        stPreRollRate.uiNumerator = 1;
        stPreRollRate.uiDenominator = pAudioParams->sVideoParams.uiPreRollRate;
        
        ret = BXDM_PictureProvider_SetPreRollRate_isr(hRapCh->hPictureProvider,
               &stPreRollRate);
        if (ret != BERR_SUCCESS)
        {
            return BERR_TRACE(ret);
        }
    }

    ret = BXDM_PictureProvider_SetPlaybackMode_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.bPlayback);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }

    ret = BXDM_PictureProvider_SetDefaultFrameRate_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.eDefaultFrameRate);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    
    ret = BXDM_PictureProvider_SetFrameRateDetectionMode_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.eFrameRateDetectionMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    
    ret = BXDM_PictureProvider_SetASTMMode_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.bAstmMode);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    
    ret = BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr(hRapCh->hPictureProvider,
            pAudioParams->sVideoParams.bVsyncModeOnPcrDiscontinuity);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    
    ret = BRAP_VID_SetErrorHandlingMode_isr(hRapCh,
            pAudioParams->sVideoParams.eErrorHandling);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    
    ret = BXDM_PictureProvider_StartDecode_isr(hRapCh->hPictureProvider);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
    BKNI_LeaveCriticalSection();


    stXdmDihPictureProviderSettings.uiVDCRectangleNumber = pAudioParams->sVideoParams.uiVDCRectangleNum;

    ret = BXDM_DisplayInterruptHandler_AddPictureProviderInterface(
        pAudioParams->sVideoParams.hXdmDih,
        BXDM_PictureProvider_GetPicture_isr,
        hRapCh->hPictureProvider,
        &stXdmDihPictureProviderSettings);
    if (ret != BERR_SUCCESS)
    {
        return BERR_TRACE(ret);
    }
   
    BDBG_LEAVE(BRAP_VID_P_ConfigureXDM);
    return ret;
}


BERR_Code BRAP_VID_P_InitInterframeBuffer(
				BRAP_FWIF_P_FwTaskHandle	hTask)
{
	BRAP_CIT_P_VideoCITOutput	*pCitOutput = NULL;
	BRAP_CIT_P_FwBufInfo *pOpNodeInfo = NULL;
	BERR_Code	rc = BERR_SUCCESS;
	unsigned int i,j,k;
	const BIMG_Interface *iface = NULL;	
	void *pImgContext = NULL;
	uint32_t	ui32ImgId = BRAP_MAX_IMG_ID,ui32physAddress=0;
	bool bDownloaded =false;
	BRAP_Handle	hRap = NULL;

	BDBG_ENTER(BRAP_VID_P_InitInterframeBuffer);
	BDBG_ASSERT(hTask);

    if(true == hTask->bChSpecificTask)
    {
        hRap = hTask->uHandle.hRapCh->hRap;
    }
    else
    {
        hRap = hTask->uHandle.hAssociation->hRap;
    }
	iface = hRap->sSettings.pImgInterface;	
	pImgContext = hRap->sSettings.pImgContext;
    pCitOutput = &(hTask->sVideoCitOutput);

	for(i = 0;i < pCitOutput->ui32NumBranches; i++)
	{
		for(j = 0; j < pCitOutput->sCitBranchInfo[i].ui32NumStages; j++)
		{
			for(k = 0; k < pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].ui32NumNodes; k++)
			{
				if(pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].sFwOpNodeInfo[k].eFwExecId != BRAP_AF_P_AlgoId_eMax)
				{
					pOpNodeInfo = &(pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].sFwOpNodeInfo[k]);

					/* check if enough memory is allocated for interframe buffer */
					if(pOpNodeInfo->ui32InterframeBufSize < hRap->sImgIdMappingArrays.ui32InterframeSize[pOpNodeInfo->eFwExecId])
					{
						BDBG_ERR(("Allocated memory (%d) for interframe buffer is less than required (%d) for ExecId =%#x",
							pOpNodeInfo->ui32InterframeBufSize ,
							hRap->sImgIdMappingArrays.ui32InterframeSize[pOpNodeInfo->eFwExecId],
							pOpNodeInfo->eFwExecId));
					}
					else
					{
                    	if(pOpNodeInfo->eFwExecId < BRAP_AF_P_AlgoId_eMax)
                    	{
                    		ui32ImgId = hRap->sImgIdMappingArrays.ui32InterframeImgId[pOpNodeInfo->eFwExecId];
                    	}      
                    	else
                    	{
                    		BDBG_ERR(("Invalid FW exec ID"));
                    		return BERR_INVALID_PARAMETER;
                    	}                      
                        
						BRAP_P_ConvertAddressToOffset(hRap->hHeap, (void *)(pOpNodeInfo->ui32InterframeBufAdr), &ui32physAddress);	
						BDBG_MSG(("pOpNodeInfo->ui32InterframeBufAdr %#x physical %#x",pOpNodeInfo->ui32InterframeBufAdr,ui32physAddress));

    					
    					if(BRAP_MAX_IMG_ID == ui32ImgId)
    					{
    						BDBG_MSG(("No FW Image file exists for Exec Id %#x",pOpNodeInfo->eFwExecId));
    					}
    					else if(false == bDownloaded)
    					{
    				        BDBG_MSG(("pOpNodeInfo->eFwExecId > %d, ui32ImgId > %d", pOpNodeInfo->eFwExecId, ui32ImgId));
    						BRAP_FWDWNLD_P_CopyFWImageToMem(iface,pImgContext,pOpNodeInfo->ui32InterframeBufAdr,ui32ImgId,hRap->hHeap);
    					}
				    }
				}
				else
					break;
				
			}
		}
	}

	
	BDBG_LEAVE(BRAP_VID_P_InitInterframeBuffer);
	return rc;

}


BERR_Code BRAP_P_StartVideoChannel ( 
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams	/* [in] Audio parameters for 
	                                               starting this channel */
	)
{
    BERR_Code ret = BERR_SUCCESS;
    bool bWdgRecovery = false;

    unsigned int  uiPth = 0, uiTemp=0;    
    BDBG_ENTER(BRAP_P_StartVideoChannel);

    /* Validate parameters */
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pAudioParams);

    /* Check if this is a watchdog recovery. */
    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);

#ifdef BCHP_PWR_RESOURCE_RAP_START
    if (!bWdgRecovery && hRapCh->eState != BRAP_P_State_eStarted) {
        BCHP_PWR_AcquireResource(hRapCh->hChip, BCHP_PWR_RESOURCE_RAP_START);
    }
#endif

    /*Start Sanity Checks*/
    if(false == BRAP_FWDWNLD_P_IsVidCodecSupported(pAudioParams->sVideoParams.eType))
    {
        BDBG_ERR(("Please export RAP_XXXX_SUPPORT=y at the compile time for Algo Id = %d, where XXXX is Algo Name e.g REALVIDEO etc"
            ,pAudioParams->sDspChParams.eType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    if(pAudioParams->eAudioSource != BRAP_AudioSource_eXptInterface)
    {
        BDBG_ERR(("Audio source must be - BRAP_AudioSource_eXptInterface"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    /*End Sanity Checks*/

    if(false == bWdgRecovery)
    {
/*        BRAP_FWIF_P_FormVideoProcessingNetworks(hRapCh,true,pAudioParams->sDspChParams.eDecodeMode,pAudioParams->sDspChParams.eType);        */
        hRapCh->sChAudProcessingStage[0][0].bDecoderStage = true;
    }
    
    if(hRapCh->bInternalCallFromRap == false)
    {
       /* ret = BRAP_VID_P_ConfigureXDM(hRapCh, pAudioParams);
        if(ret != BERR_SUCCESS) 
        {
            BDBG_ERR(("BRAP_StartChannel: BRAP_VID_P_ConfigureXDM returned %d",ret));
            return BERR_TRACE(ret);
        }
         Get all the channel paths required for this channel */
        /* This function should also populate the inlink, outlink, self link
           for all the paths required in this channel */
        hRapCh->eAudioSource = pAudioParams->eAudioSource;
        
        ret = BRAP_P_GetChannelPaths(hRapCh, pAudioParams,&uiTemp);
        if(ret != BERR_SUCCESS) 
        {
            BDBG_ERR(("BRAP_StartChannel: GetChannelPaths returned %d",ret));
            return BERR_TRACE(ret);
        }
        /* Open all the paths in this channel */


        
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if(BRAP_P_CheckIfPtrInvalid((void *)hRapCh->pPath[uiPth]) == true)
            {
                continue;
            }
            switch(hRapCh->pPath[uiPth]->eUsgPath)
            {
                case BRAP_P_UsgPath_eVideoDecode:                       
                    BDBG_MSG(("OpenVideoDecodePath"));
                    ret = BRAP_P_OpenVideoDecPath(hRapCh, pAudioParams, uiPth);
                    if(ret != BERR_SUCCESS) 
                    {
                        BDBG_ERR(("BRAP_StartChannel: OpenDecPath returned %d",ret));
                        return BERR_TRACE(ret);
                    }
                    break;
                default:
                    BDBG_MSG(("BRAP_StartChannel: Path Open# Unsupported pPath[%d].eUsgPath = %d",
                        uiPth, hRapCh->pPath[uiPth]->eUsgPath));
                    /* Do nothing */
                    break;
            }
        }/* uiPth */        
    }

    /* Start all the paths in this channel */
    for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
    {
        if(BRAP_P_CheckIfPtrInvalid((void *)hRapCh->pPath[uiPth]) == true)
        {
            continue;
        }
        switch(hRapCh->pPath[uiPth]->eUsgPath)
        {
            case BRAP_P_UsgPath_eVideoDecode:                                                           
                BDBG_MSG(("StartVideoDecPath"));
                ret = BRAP_P_StartVideoDecPath(hRapCh, pAudioParams, uiPth);
                if(ret != BERR_SUCCESS) 
                {
                    BDBG_ERR(("BRAP_StartChannel: StartDecPath returned %d",ret));
                    return BERR_TRACE(ret);
                }
                break;  
            default:
                BDBG_MSG(("BRAP_StartChannel: Path Start# Unsupported pPath[%d].eUsgPath = %d",
                uiPth, hRapCh->pPath[uiPth]->eUsgPath));
                /* Do nothing */
                break;
        }
    }/* uiPth */    
       
    /* Update the channel state */
    if((false == bWdgRecovery)
    && (BERR_SUCCESS == ret))
    {
        hRapCh->eState = BRAP_P_State_eStarted;
        hRapCh->sVideoParams = pAudioParams->sVideoParams;
    }  

#ifdef BCHP_PWR_RESOURCE_RAP_START
    if (ret != BERR_SUCCESS) {
        /* a failed Start releases the resource */
        BCHP_PWR_ReleaseResource(hRapCh->hChip, BCHP_PWR_RESOURCE_RAP_START);
    }
#endif

    BDBG_LEAVE(BRAP_P_StartVideoChannel);
    return ret;
}

BERR_Code BRAP_P_StopVideoChannel ( 
	BRAP_ChannelHandle 	        hRapCh		    /* [in] RAP channel handle */
	)
{
    BERR_Code ret = BERR_SUCCESS;
    bool bWdgRecovery = false;
    unsigned int uiPth = 0;
    BXDM_PictureProvider_Counters stCounters;
    BDBG_ENTER(BRAP_P_StopVideoChannel);

    /* Validate parameters */
    BDBG_ASSERT(hRapCh);

    /* Check if this is a watchdog recovery. */
    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);
    /* If channel is already stopped return success */

    ret = BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
        hRapCh->sVideoParams.hXdmDih,BXDM_PictureProvider_GetPicture_isr,
        hRapCh->hPictureProvider);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_P_StopVideoChannel: RemovePictureProviderInterface Failed"));
        return BERR_TRACE(ret);
    }
    BKNI_EnterCriticalSection();

    BERR_TRACE(BXDM_PictureProvider_GetCounters_isr(hRapCh->hPictureProvider,&stCounters));

    BDBG_MSG(("BRAP_P_StopVideoChannel() - Pictures Delayed: %d", stCounters.uiUnderflowCount));
    BDBG_MSG(("BRAP_P_StopVideoChannel() - Decoder error count: %d", stCounters.uiDecodeErrorCount));

    ret = BXDM_PictureProvider_StopDecode_isr(hRapCh->hPictureProvider);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_P_StopVideoChannel: BXDM_PictureProvider_StopDecode_isr Failed"));
        return BERR_TRACE(ret);
    }
    
    BKNI_LeaveCriticalSection();
    
    /* Stop all the paths in this channel */
    for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
    {
        if(BRAP_P_CheckIfPtrInvalid((void *)hRapCh->pPath[uiPth]) == true)
        {
            continue;
        }
        switch(hRapCh->pPath[uiPth]->eUsgPath)
        {
            case BRAP_P_UsgPath_eVideoDecode:                                    
                ret = BRAP_P_StopVideoDecPath(hRapCh, uiPth);
                if(ret != BERR_SUCCESS) 
                {
                    BDBG_ERR(("BRAP_P_StopVideoChannel: BRAP_P_StopVideoDecPath returned %d",ret));
                    return BERR_TRACE(ret);
                }
                break;  
            default:
                BDBG_MSG(("BRAP_P_StopVideoChannel: Path Stop# Unsupported pPath[%d].eUsgPath = %d",
                    uiPth, hRapCh->pPath[uiPth]->eUsgPath));
                /* Do nothing */
                break;
        }
    }/* uiPth */

    if(hRapCh->bInternalCallFromRap == false)
    {
        /* Close all the paths in this channel */
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {   
            if(BRAP_P_CheckIfPtrInvalid((void *)hRapCh->pPath[uiPth]) == true)
            {
                continue;
            }
            switch(hRapCh->pPath[uiPth]->eUsgPath)
            {
                case BRAP_P_UsgPath_eVideoDecode:                                       
                    BDBG_MSG(("Calling BRAP_P_CloseVideoDecPath for uiPth = %d eUsgPath = %d", 
                        uiPth, hRapCh->pPath[uiPth]->eUsgPath));
                    ret = BRAP_P_CloseVideoDecPath(hRapCh, uiPth);
                    if(ret != BERR_SUCCESS) 
                    {
                        BDBG_ERR(("BRAP_StopChannel: CloseDecPath returned %d",ret));
                        return BERR_TRACE(ret);
                    }
                    break;
                default:
                    BDBG_MSG(("BRAP_P_StopVideoChannel: Path Close# Unsupported pPath[%d].eUsgPath = %d",
                        uiPth, hRapCh->pPath[uiPth]->eUsgPath));
                    /* Do nothing */
                    break;
            } 
        }/* uiPth */
    }/* if bInternalCallFromRap == false */

    if(false == hRapCh->bInternalCallFromRap)
    {    
        ret = BRAP_P_UngetChannelPaths(hRapCh);
        if(ret != BERR_SUCCESS) 
        {
            BDBG_ERR(("BRAP_P_StopVideoChannel: UngetChannelPaths returned %d",ret));
            return BERR_TRACE(ret);
        }
    }


    if(false == bWdgRecovery)
    {
        hRapCh->sChAudProcessingStage[0][0].bDecoderStage = false;
    }        

    /* Update the channel state */
    if(BERR_SUCCESS == ret)
    {
        hRapCh->eState = BRAP_P_State_eOpened;
        hRapCh->bStopinvoked= true; 

#ifdef BCHP_PWR_RESOURCE_RAP_START
        /* a successful StopChannel releases the resource */
        BCHP_PWR_ReleaseResource(hRapCh->hChip, BCHP_PWR_RESOURCE_RAP_START);
#endif
    }

    BDBG_LEAVE(BRAP_P_StopVideoChannel);
    return ret;
}

BERR_Code 
BRAP_P_OpenVideoDecPath(
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_ChannelParams    *pAudioParams,  /* [in] Audio params passed 
                                                   during channel start time */
    const unsigned int          uiPth
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(uiPth);        
    BSTD_UNUSED(pAudioParams);            
    return ret;
}

BERR_Code 
  BRAP_P_StartVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_DSPCHN_P_AudioParams   *pDspChParams = NULL;
    bool                        bWdgRecovery = false;
    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);


    BDBG_ENTER (BRAP_P_StartVideoDecPath);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pAudioParams);   
    if(NULL == hRapCh->pPath[uiPth])
    {
        ret = BERR_TRACE(BERR_NOT_INITIALIZED);
        goto exit;
    }    
    /* Check if the path has already been started. If yes, just increment the 
       count and return success. */
    if((bWdgRecovery == false) 
        && (0 != hRapCh->pPath[uiPth]->uiPthStartCnt))
    {
        /* Increment the open count */
        hRapCh->pPath[uiPth]->uiPthStartCnt++;

        BDBG_MSG(("BRAP_P_StartVideoDecPath: Video Dec Path %d was already started. " 
                "New start count = %d", uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));

        /* Return success */
        BDBG_LEAVE (BRAP_P_StartVideoDecPath);
        ret = BERR_SUCCESS;
        goto exit;        
    }  

    /* Malloc large local structures */
    pDspChParams = ( BRAP_DSPCHN_P_AudioParams * ) BKNI_Malloc( sizeof( BRAP_DSPCHN_P_AudioParams ));
    if( NULL==pDspChParams )
    {
        ret = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto exit;        
    }
    BKNI_Memset(pDspChParams, 0, sizeof(BRAP_DSPCHN_P_AudioParams));
    
    pDspChParams->sExtAudioParams = pAudioParams->sDspChParams;
    pDspChParams->sExtVideoParams.eType = pAudioParams->sVideoParams.eType;
    pDspChParams->sExtVideoParams.ui32DispStripeHeight = pAudioParams->sVideoParams.ui32DispStripeHeight;
    pDspChParams->sExtVideoParams.ui32DispStripeWidth = pAudioParams->sVideoParams.ui32DispStripeWidth;
    pDspChParams->sExtVideoParams.ui32NumBuffAvl = pAudioParams->sVideoParams.ui32NumBuffAvl;
    
    /* Other misc DSP channel params */                        
    pDspChParams->eTimebase = pAudioParams->eTimebase;
    pDspChParams->sXptContextMap = pAudioParams->sXptContextMap;

    ret = BRAP_DSPCHN_P_StartVideo(hRapCh->pPath[uiPth]->hDspCh, pDspChParams);
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("BRAP_P_StartDecPath: BRAP_DSPCHN_P_Start failed %d", ret));
        ret = BERR_TRACE(ret); 
        goto exit;
    }    
    if((BERR_SUCCESS == ret) && (bWdgRecovery == false))
    {
        /* Increment the start count */
        hRapCh->pPath[uiPth]->uiPthStartCnt++;

        BDBG_MSG(("BRAP_P_StartDecPath: hRapCh->pPath[uiPth=%d]->uiPthStartCnt = %d",
        uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));

        /* Exit successfully */
        goto exit;    
    }
    
exit:        
    BDBG_MSG(("BRAP_P_StartVideoDecPath Done!"));
    if(pDspChParams)
        BKNI_Free(pDspChParams);
    BDBG_LEAVE(BRAP_P_StartVideoDecPath);
    return ret;
}


BERR_Code 
BRAP_P_StopVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,     /* [in] RAP Channel handle */
    const unsigned int          uiPth       /* [in] Path idx corresponding 
                                               to pPath[] array element */
    )
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_P_StopVideoDecPath);
    BDBG_ASSERT(hRapCh);
    if(NULL == hRapCh->pPath[uiPth])
    {
        ret = BERR_TRACE(BERR_NOT_INITIALIZED);
        goto exit;
    }      

    /* Error state check */
    if (0 == hRapCh->pPath[uiPth]->uiPthStartCnt)
    { 
        /* This should never happen. If it does, it means the system
           has gone into an invalid state!!!!*/
        BDBG_ERR (("BRAP_P_StopDecPath: Dec Path start count is already 0"));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    /* Decrement the start count */
    hRapCh->pPath[uiPth]->uiPthStartCnt--;

    BDBG_MSG(("BRAP_P_StopDecPath: For uiPth=%d New uiPthStartCnt = %d", 
        uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));

    /* If the new start count is not 0, do not close the resources instead 
       return success */
    if(0 != hRapCh->pPath[uiPth]->uiPthStartCnt)
    {
        BDBG_MSG(("BRAP_P_StopDecPath: uiPthStartCnt = %d so not closing resources!"));
        BDBG_LEAVE(BRAP_P_StopDecPath);
        return BERR_SUCCESS;
    }
    else
    {
	 hRapCh->pPath[uiPth]->uiPthStartCnt++;
    }

    /* Stop the DSP channel, if any in this path */
    if(NULL != hRapCh->pPath[uiPth]->hDspCh)
    {  
        BDBG_MSG(("Calling BRAP_DSPCHN_P_StopVideo"));
        ret = BRAP_DSPCHN_P_StopVideo(hRapCh->pPath[uiPth]->hDspCh);
        if(BERR_SUCCESS != ret)
        {
        	BDBG_ERR(("BRAP_P_StopDecPath: BRAP_DSPCHN_P_StopVideo failed %d",ret));
        	return BERR_TRACE(ret); 
        }
    }
    else
        BDBG_MSG(("Did **NOT** Call BRAP_DSPCHN_P_StopVideo"));

    /* Decrement the start count */
    hRapCh->pPath[uiPth]->uiPthStartCnt--;
    BDBG_LEAVE(BRAP_P_StopVideoDecPath);

exit:        
    return ret;
}


BERR_Code 
BRAP_P_CloseVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(uiPth);    
    return ret;
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_SetPictureDropMode
(
   BRAP_ChannelHandle           hRapCh,             /* [in] The Rap Channel handle */
   BRAP_VID_PictureDropMode     ePictureDropMode    /* [in] the picture drop mode */
   )
{
   BERR_Code ret = BERR_SUCCESS;

   BDBG_ENTER(BRAP_VID_SetPictureDropMode);

   BKNI_EnterCriticalSection();
   ret = BRAP_VID_SetPictureDropMode_isr(
      hRapCh,
      ePictureDropMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetPictureDropMode);
   return ret;
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_SetPictureDropMode_isr
(
   BRAP_ChannelHandle           hRapCh, /* [in] The XVD Channel handle */
   BRAP_VID_PictureDropMode     ePictureDropMode /* [in] the picture drop mode */
   )
{
   BERR_Code ret = BERR_SUCCESS;
   BXDM_PictureProvider_PictureDropMode eXDMPictureDropMode;
   BDBG_ENTER(BRAP_VID_SetPictureDropMode_isr);

   BDBG_ASSERT(hRapCh);

   switch ( ePictureDropMode )
   {
      case BRAP_VID_PictureDropMode_eField:
         eXDMPictureDropMode = BXDM_PictureProvider_PictureDropMode_eField;
         break;

      case BRAP_VID_PictureDropMode_eFrame:
         eXDMPictureDropMode = BXDM_PictureProvider_PictureDropMode_eFrame;
         break;

      default:
         BDBG_ERR(("Invalid picture drop mode specified: %d", ePictureDropMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   ret = BXDM_PictureProvider_SetPictureDropMode_isr(
            hRapCh->hPictureProvider,
            eXDMPictureDropMode
            );

   BDBG_MSG(("BRAP_VID_SetPictureDropMode_isr(%d)", ePictureDropMode));

   BDBG_LEAVE(BRAP_VID_SetPictureDropMode_isr);

   return ret;
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_GetPictureDropMode
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BRAP_VID_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetPictureDropMode);

   rc = BRAP_VID_GetPictureDropMode_isr(
      hRapCh,
      pePictureDropMode
      );

   BDBG_LEAVE(BRAP_VID_GetPictureDropMode);
   return BERR_TRACE(rc);
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_GetPictureDropMode_isr(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BRAP_VID_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PictureDropMode eXDMPictureDropMode;

   BDBG_ENTER(BRAP_VID_GetPictureDropMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pePictureDropMode);

   rc = BXDM_PictureProvider_GetPictureDropMode_isr(
            hRapCh->hPictureProvider,
            &eXDMPictureDropMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMPictureDropMode )
      {
         case BXDM_PictureProvider_PictureDropMode_eField:
            *pePictureDropMode = BRAP_VID_PictureDropMode_eField;
            break;

         case BXDM_PictureProvider_PictureDropMode_eFrame:
            *pePictureDropMode = BRAP_VID_PictureDropMode_eFrame;
            break;

         default:
            BDBG_ERR( ("Unknown XDN picture drop mode: %d", eXDMPictureDropMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BRAP_VID_GetPictureDropMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_SetPtsStcDiffThreshold: Set the Pts/Stc difference threshold
****************************************************************************/
BERR_Code BRAP_VID_SetPtsStcDiffThreshold
(
   BRAP_ChannelHandle hRapCh,
   long uiPtsStcDiffThreshold
   )
{
   BERR_Code ret = BERR_SUCCESS;

   BDBG_ENTER(BRAP_VID_SetPtsStcDiffThreshold);

   BKNI_EnterCriticalSection();
   ret = BRAP_VID_SetPtsStcDiffThreshold_isr(
      hRapCh,
      uiPtsStcDiffThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetPtsStcDiffThreshold);
   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_SetPtsStcDiffThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   long uiPtsStcDiffThreshold
   )
{
    BERR_Code ret = BERR_SUCCESS;
    BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

    BDBG_ENTER(BRAP_VID_SetPtsStcDiffThreshold_isr);

    BDBG_ASSERT(hRapCh);

    ret = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
                                        hRapCh->hPictureProvider,
                                        &eCurrentTSMThresholdSettings
                                        );

    if ( BERR_SUCCESS == ret )
    {
        eCurrentTSMThresholdSettings.uiDeltaStcPtsDiffThreshold = uiPtsStcDiffThreshold;

        ret = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
           hRapCh->hPictureProvider,
           &eCurrentTSMThresholdSettings);

        BDBG_MSG(("BRAP_VID_SetPtsStcDiffThreshold(%d)", uiPtsStcDiffThreshold));
    }

    BDBG_LEAVE(BRAP_VID_SetPtsStcDiffThreshold_isr);
    return BERR_TRACE( ret );
}

/***************************************************************************
BRAP_VID_GetPtsStcDiffThreshold: Get the Pts/Stc difference threshold
****************************************************************************/
BERR_Code BRAP_VID_GetPtsStcDiffThreshold
(
   BRAP_ChannelHandle hRapCh,
   long *puiPtsStcDiffThreshold
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_GetPtsStcDiffThreshold);

   ret = BRAP_VID_GetPtsStcDiffThreshold_isr(
                                  hRapCh,
                                  puiPtsStcDiffThreshold);

   BDBG_LEAVE(BRAP_VID_GetPtsStcDiffThreshold);
   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_GetPtsStcDiffThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   long *puiPtsStcDiffThreshold
   )
{
   BERR_Code ret;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BRAP_VID_GetPtsStcDiffThreshold_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(puiPtsStcDiffThreshold);

   ret = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hRapCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   *puiPtsStcDiffThreshold = eCurrentTSMThresholdSettings.uiDeltaStcPtsDiffThreshold;

   BDBG_LEAVE(BRAP_VID_GetPtsStcDiffThreshold_isr);
   return BERR_TRACE( ret );
}


/***************************************************************************
    BRAP_VID_SetMonitorRefreshRate: Used to set the monitor refresh rate
****************************************************************************/
BERR_Code BRAP_VID_SetMonitorRefreshRate
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate eMonitorRefreshRate
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_SetMonitorRefreshRate);

   BKNI_EnterCriticalSection();
   ret = BRAP_VID_SetMonitorRefreshRate_isr(
      hRapCh,
      eMonitorRefreshRate
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetMonitorRefreshRate);
   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_SetMonitorRefreshRate_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate eMonitorRefreshRate
   )
{
   BERR_Code ret;
   BDBG_ENTER(BRAP_VID_SetMonitorRefreshRate_isr);
   BDBG_ASSERT(hRapCh);

   ret = BXDM_PictureProvider_SetMonitorRefreshRate_isr(
            hRapCh->hPictureProvider,
            (BXDM_PictureProvider_MonitorRefreshRate )eMonitorRefreshRate
            );

   BDBG_MSG(("BRAP_VID_SetMonitorRefreshRate(%d)", eMonitorRefreshRate));

   BDBG_LEAVE(BRAP_VID_SetMonitorRefreshRate_isr);
   return BERR_TRACE( ret );
}

/***************************************************************************
    BRAP_VID_GetMonitorRefreshRate: Used to get the current value of the monitor
  refresh rate
****************************************************************************/
BERR_Code BRAP_VID_GetMonitorRefreshRate
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate *peMonitorRefreshRate
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_GetMonitorRefreshRate);

   ret = BRAP_VID_GetMonitorRefreshRate_isr(
      hRapCh,
      peMonitorRefreshRate
      );

   BDBG_LEAVE(BRAP_VID_GetMonitorRefreshRate);
   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_GetMonitorRefreshRate_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate *peMonitorRefreshRate
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_GetMonitorRefreshRate_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peMonitorRefreshRate);

   ret = BXDM_PictureProvider_GetMonitorRefreshRate_isr(
            hRapCh->hPictureProvider,
            (BXDM_PictureProvider_MonitorRefreshRate *)peMonitorRefreshRate
            );

   BDBG_LEAVE(BRAP_VID_GetMonitorRefreshRate_isr);
   return BERR_TRACE( ret );
}


BERR_Code BRAP_VID_Set1080pScanMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode e1080pScanMode
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_Set1080pScanMode);

   BKNI_EnterCriticalSection();
   ret = BRAP_VID_Set1080pScanMode_isr(
      hRapCh,
      e1080pScanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_Set1080pScanMode);
   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_Set1080pScanMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode e1080pScanMode
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_Set1080pScanMode_isr);

   BDBG_ASSERT(hRapCh);

   ret = BXDM_PictureProvider_Set1080pScanMode_isr(
            hRapCh->hPictureProvider,
            e1080pScanMode
            );

   BDBG_MSG(("BRAP_VID_Set1080pScanMode(%d)", e1080pScanMode));

   BDBG_LEAVE(BRAP_VID_Set1080pScanMode_isr);
   return ret;
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_Get1080pScanMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode *pe1080pScanMode
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_Get1080pScanMode);

   BKNI_EnterCriticalSection();
   ret = BRAP_VID_Get1080pScanMode_isr(
      hRapCh,
      pe1080pScanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_Get1080pScanMode);

   return BERR_TRACE(ret);
}

BERR_Code BRAP_VID_Get1080pScanMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode *pe1080pScanMode
   )
{
   BERR_Code ret;

   BDBG_ENTER(BRAP_VID_Get1080pScanMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pe1080pScanMode);

   ret = BXDM_PictureProvider_Get1080pScanMode_isr(
            hRapCh->hPictureProvider,
            pe1080pScanMode
            );

   BDBG_LEAVE(BRAP_VID_Get1080pScanMode_isr);
   return BERR_TRACE(ret);
}

/***************************************************************************
    BRAP_VID_SetErrorHandlingMode: Used to set the error handling mode.
****************************************************************************/
BERR_Code BRAP_VID_SetErrorHandlingMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling eErrorMode
)
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetErrorHandlingMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetErrorHandlingMode_isr(
      hRapCh,
      eErrorMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetErrorHandlingMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetErrorHandlingMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling eErrorMode
)
{
   BERR_Code ret;
   BXDM_PictureProvider_ErrorHandlingMode eXDMErrorHandlingMode;

   BDBG_ENTER(BRAP_VID_SetErrorHandlingMode_isr);
   BDBG_ASSERT(hRapCh);

   switch ( eErrorMode )
   {
      case BRAP_VID_ErrorHandling_eOff:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_eOff;
         break;

      case BRAP_VID_ErrorHandling_ePicture:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_ePicture;
         break;

      case BRAP_VID_ErrorHandling_ePrognostic:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_ePrognostic;
         break;

      default:
         BDBG_ERR(("Invalid Error Handling Mode: 0x%x (%d)",
                  eErrorMode, eErrorMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   ret = BXDM_PictureProvider_SetErrorHandlingMode_isr(hRapCh->hPictureProvider,
            eXDMErrorHandlingMode);

   BDBG_MSG(("BRAP_VID_SetErrorHandlingMode(%d)", eErrorMode));

   BDBG_LEAVE(BRAP_VID_SetErrorHandlingMode_isr);
   return BERR_TRACE(ret);
}

/***************************************************************************
    BRAP_VID_GetErrorHandlingMode: Used to get the error handling mode.
****************************************************************************/
BERR_Code BRAP_VID_GetErrorHandlingMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling * peErrorMode
)
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetErrorHandlingMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetErrorHandlingMode_isr(
      hRapCh,
      peErrorMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetErrorHandlingMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetErrorHandlingMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling * peErrorMode
)
{
   BERR_Code rc;
   BXDM_PictureProvider_ErrorHandlingMode eXDMErrorHandlingMode;

   BDBG_ENTER(BRAP_VID_GetErrorHandlingMode_isr);
   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peErrorMode);

   rc = BXDM_PictureProvider_GetErrorHandlingMode_isr(
            hRapCh->hPictureProvider,
            &eXDMErrorHandlingMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMErrorHandlingMode )
      {
         case BXDM_PictureProvider_ErrorHandlingMode_eOff:
            *peErrorMode = BRAP_VID_ErrorHandling_eOff;
            break;

         case BXDM_PictureProvider_ErrorHandlingMode_ePicture:
            *peErrorMode = BRAP_VID_ErrorHandling_ePicture;
            break;

         case BXDM_PictureProvider_ErrorHandlingMode_ePrognostic:
            *peErrorMode = BRAP_VID_ErrorHandling_ePrognostic;
            break;

         default:
            BDBG_ERR(("Unknown XDM Error Handling Mode: 0x%x (%d)",
                     eXDMErrorHandlingMode, eXDMErrorHandlingMode));

            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_MSG(("BRAP_VID_GetErrorHandlingMode(%d)", *peErrorMode ));

   BDBG_LEAVE(BRAP_VID_GetErrorHandlingMode_isr);
   return BERR_TRACE(rc);
}



/***************************************************************************
BRAP_VID_GetPTS: API used to get the running PTS.
****************************************************************************/
BERR_Code BRAP_VID_GetPTS
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo   *pPTSInfo
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetPTS);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetPTS_isr(
      hRapCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetPTS);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetPTS_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;
   BDBG_ENTER(BRAP_VID_GetPTS_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pPTSInfo);

   /*
    * Copy the "stCurrentPTSInfo" structure.
    */

   rc = BXDM_PictureProvider_GetCurrentPTSInfo_isr(
            hRapCh->hPictureProvider,
            &stPTSInfo
            );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BRAP_VID_GetPTS_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_GetLastCodedPTS: API used to get the last coded PTS.
****************************************************************************/
BERR_Code BRAP_VID_GetLastCodedPTS
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetLastCodedPTS);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetLastCodedPTS_isr(
      hRapCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetLastCodedPTS);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetLastCodedPTS_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;

   BDBG_ENTER(BRAP_VID_GetLastCodedPTS_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pPTSInfo);


   rc = BXDM_PictureProvider_GetLastCodedPTSInfo_isr(
            hRapCh->hPictureProvider,
            &stPTSInfo
            );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BRAP_VID_GetLastCodedPTS_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_GetNextPTS: API used to get the next PTS.
****************************************************************************/
BERR_Code BRAP_VID_GetNextPTS
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetNextPTS);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetNextPTS_isr(
      hRapCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetNextPTS);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetNextPTS_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;

   BDBG_ENTER(BRAP_VID_GetNextPTS_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pPTSInfo);

   rc = BXDM_PictureProvider_GetNextPTSInfo_isr(
               hRapCh->hPictureProvider,
               &stPTSInfo
               );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BRAP_VID_GetNextPTS_isr);

   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_GetIPictureFoundState: Retrieve I picture found status
****************************************************************************/
BERR_Code BRAP_VID_GetIPictureFoundStatus
(
   BRAP_ChannelHandle hRapCh,
   bool *pbIPictureFound
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetIPictureFoundStatus);

   rc = BRAP_VID_GetIPictureFoundStatus_isr(
      hRapCh,
      pbIPictureFound
      );

   BDBG_LEAVE(BRAP_VID_GetIPictureFoundStatus);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetIPictureFoundStatus_isr
(
   BRAP_ChannelHandle hRapCh,
   bool *pbIPictureFound
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetIPictureFoundStatus_isr);

   rc = BXDM_PictureProvider_GetIPictureFoundStatus_isr(
                     hRapCh->hPictureProvider,
                     pbIPictureFound
                     );

   BDBG_LEAVE(BRAP_VID_GetIPictureFoundStatus_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_GetPictureTag: Get the current picture tag from DM.
***************************************************************************/
BERR_Code BRAP_VID_GetPictureTag
(
   BRAP_ChannelHandle    hRapCh,
   unsigned long        *pulPictureTag
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetPictureTag);

   rc = BRAP_VID_GetPictureTag_isr(
      hRapCh,
      pulPictureTag
      );

   BDBG_LEAVE(BRAP_VID_GetPictureTag);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetPictureTag_isr
(
   BRAP_ChannelHandle    hRapCh,
   unsigned long        *pulPictureTag
   )
{
   BERR_Code rc;
   uint32_t uiPictureTag;

   BDBG_ENTER(BRAP_VID_GetPictureTag_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pulPictureTag);

   /* Return current picture tag */
   rc = BXDM_PictureProvider_GetPictureTag_isr(
                     hRapCh->hPictureProvider,
                     &uiPictureTag
                     );

   *pulPictureTag = uiPictureTag;

   BDBG_LEAVE(BRAP_VID_GetPictureTag_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_GetGopTimeCode: Get the GOP timecode from DM and decode into HH:MM:SS:FF
***************************************************************************/
BERR_Code BRAP_VID_GetGopTimeCode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_GopTimeCode *pGopTimeCode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetGopTimeCode);

   rc = BRAP_VID_GetGopTimeCode_isr(
      hRapCh,
      pGopTimeCode
      );

   BDBG_LEAVE(BRAP_VID_GetGopTimeCode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetGopTimeCode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_GopTimeCode *pGopTimeCode
   )
{
   BERR_Code rc;
   BXDM_Picture_GopTimeCode stTimeCode;
   BDBG_ENTER(BRAP_VID_GetGopTimeCode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pGopTimeCode);

/*warning [XVD Decoder] Move GOP Time code parsing logic to PPB to Unified parser*/
#if 0
   /* If DM doesn't give us a time code, zero out the structure otherwise
      unpack the GOP time code into the structure according to the following
      layout:
      ulGopTimeCode = [23:19]=hour, [18:13]=minute, [11:6]=sec, [5:0]=frame
   */
   if (hRapCh->stDMStatus.uiGopTimeCode == BRAP_VID_P_INVALID_TIMECODE)
   {
      pGopTimeCode->ulTimeCodeHours    = 0;
      pGopTimeCode->ulTimeCodeMinutes  = 0;
      pGopTimeCode->ulTimeCodeSeconds  = 0;
      pGopTimeCode->ulTimeCodePictures = 0;
      pGopTimeCode->bTimeCodeValid = false;
   }
   else
   {
      pGopTimeCode->ulTimeCodeHours =
         (hRapCh->stDMStatus.uiGopTimeCode&BRAP_VID_P_PPB_MPEG_GOP_HOUR_MASK) >>
         BRAP_VID_P_PPB_MPEG_GOP_HOUR_SHIFT;
      pGopTimeCode->ulTimeCodeMinutes =
         (hRapCh->stDMStatus.uiGopTimeCode&BRAP_VID_P_PPB_MPEG_GOP_MINUTE_MASK) >>
         BRAP_VID_P_PPB_MPEG_GOP_MINUTE_SHIFT;
      pGopTimeCode->ulTimeCodeSeconds =
         (hRapCh->stDMStatus.uiGopTimeCode&BRAP_VID_P_PPB_MPEG_GOP_SECOND_MASK) >>
         BRAP_VID_P_PPB_MPEG_GOP_SECOND_SHIFT;
      pGopTimeCode->ulTimeCodePictures =
         hRapCh->stDMStatus.uiGopTimeCode&BRAP_VID_P_PPB_MPEG_GOP_PICTURE_MASK;
      pGopTimeCode->bTimeCodeValid = true;
   }
#endif
   rc = BXDM_PictureProvider_GetCurrentTimeCode_isr(
            hRapCh->hPictureProvider,
            &stTimeCode
            );

   pGopTimeCode->ulTimeCodeHours = stTimeCode.uiHours;
   pGopTimeCode->ulTimeCodeMinutes = stTimeCode.uiMinutes;
   pGopTimeCode->ulTimeCodeSeconds = stTimeCode.uiSeconds;
   pGopTimeCode->ulTimeCodePictures = stTimeCode.uiPictures;
   pGopTimeCode->bTimeCodeValid = stTimeCode.bValid;

   BDBG_LEAVE(BRAP_VID_GetGopTimeCode_isr);
   return BERR_TRACE(rc);
}

/**********************************/
/* Display Manager APIs - Display */
/**********************************/

/***************************************************************************
BRAP_VID_EnableMute: The Application can override the decoder mute with this API.
***************************************************************************/
BERR_Code BRAP_VID_EnableMute
(
   BRAP_ChannelHandle hRapCh,
   bool               bEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_EnableMute);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_EnableMute_isr(
      hRapCh,
      bEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_EnableMute);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_EnableMute_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               bEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_EnableMute_isr);
   BDBG_ASSERT(hRapCh);

   /* Save user request */
   rc = BXDM_PictureProvider_SetMuteMode_isr(
            hRapCh->hPictureProvider,
            bEnable
            );

   BDBG_MSG( ("BRAP_VID_EnableMute(%d)", bEnable));

   BDBG_LEAVE(BRAP_VID_EnableMute_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_SetDisplayFieldMode: API used to set the display field mode.
****************************************************************************/
BERR_Code BRAP_VID_SetDisplayFieldMode
(
   BRAP_ChannelHandle    hRapCh,
   BXDM_PictureProvider_DisplayFieldMode eDisplayFieldType
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetDisplayFieldMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetDisplayFieldMode_isr(
      hRapCh,
      eDisplayFieldType
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetDisplayFieldMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetDisplayFieldMode_isr
(
   BRAP_ChannelHandle    hRapCh,
   BXDM_PictureProvider_DisplayFieldMode eDisplayFieldType
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetDisplayFieldMode_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_SetDisplayFieldMode_isr(
            hRapCh->hPictureProvider,
            eDisplayFieldType);

   BDBG_MSG( ("BRAP_VID_SetDisplayFieldMode(%d)", eDisplayFieldType));

   BDBG_LEAVE(BRAP_VID_SetDisplayFieldMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
BRAP_VID_GetDisplayFieldMode: Gets the video display mode status.
****************************************************************************/
BERR_Code BRAP_VID_GetDisplayFieldMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayFieldMode *peDisplayFieldType
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetDisplayFieldMode);

   rc = BRAP_VID_GetDisplayFieldMode_isr(
      hRapCh,
      peDisplayFieldType
      );

   BDBG_LEAVE(BRAP_VID_GetDisplayFieldMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetDisplayFieldMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayFieldMode *peDisplayFieldType
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetDisplayFieldMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peDisplayFieldType);


   rc = BXDM_PictureProvider_GetDisplayFieldMode_isr(
            hRapCh->hPictureProvider,
            peDisplayFieldType);


   BDBG_LEAVE(BRAP_VID_GetDisplayFieldMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_SetChannelChangeMode: Tell DM how to handle channel changes.
****************************************************************************/
BERR_Code BRAP_VID_SetChannelChangeMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode eChChangeMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetChannelChangeMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetChannelChangeMode_isr(
      hRapCh,
      eChChangeMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetChannelChangeMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetChannelChangeMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode eChChangeMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ChannelChangeSettings eNewChannelChangeSettings;

   BDBG_ENTER(BRAP_VID_SetChannelChangeMode_isr);
   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_GetChannelChangeSettings_isr(
            hRapCh->hPictureProvider,
            &eNewChannelChangeSettings);

   if ( BERR_SUCCESS == rc )
   {
      switch ( eChChangeMode )
      {
         case BRAP_VID_ChannelChangeMode_eMute:
            eNewChannelChangeSettings.bHoldLastPicture = false;
            eNewChannelChangeSettings.bFirstPicturePreview = false;
            break;

         case BRAP_VID_ChannelChangeMode_eLastFramePreviousChannel:
            eNewChannelChangeSettings.bHoldLastPicture = true;
            eNewChannelChangeSettings.bFirstPicturePreview = false;
            break;

         case BRAP_VID_ChannelChangeMode_eMuteWithFirstPicturePreview:
            eNewChannelChangeSettings.bHoldLastPicture = false;
            eNewChannelChangeSettings.bFirstPicturePreview = true;
            break;

         case BRAP_VID_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview:
            eNewChannelChangeSettings.bHoldLastPicture = true;
            eNewChannelChangeSettings.bFirstPicturePreview = true;
            break;

         default:
            BDBG_ERR( ("Invalid channel change parameter (%d)",
                     eChChangeMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      rc = BXDM_PictureProvider_SetChannelChangeSettings_isr(
               hRapCh->hPictureProvider,
               &eNewChannelChangeSettings);

   BDBG_MSG( ("BRAP_VID_SetChannelChangeMode(%d)", eChChangeMode));
   }

   BDBG_LEAVE(BRAP_VID_SetChannelChangeMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_GetChannelChangeMode: Retrieve current channel change mode
****************************************************************************/
BERR_Code BRAP_VID_GetChannelChangeMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode *peChChangeMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetChannelChangeMode);

   rc = BRAP_VID_GetChannelChangeMode_isr(
      hRapCh,
      peChChangeMode
      );

   BDBG_LEAVE(BRAP_VID_GetChannelChangeMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetChannelChangeMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode *peChChangeMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ChannelChangeSettings eCurrentChannelChangeSettings;

   BDBG_ENTER(BRAP_VID_GetChannelChangeMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peChChangeMode);

   rc = BXDM_PictureProvider_GetChannelChangeSettings_isr(
            hRapCh->hPictureProvider,
            &eCurrentChannelChangeSettings);

   if ( BERR_SUCCESS == rc )
   {
      if ( ( false == eCurrentChannelChangeSettings.bHoldLastPicture )
        && ( false == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BRAP_VID_ChannelChangeMode_eMute;
      } else if ( ( true == eCurrentChannelChangeSettings.bHoldLastPicture )
               && ( false == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BRAP_VID_ChannelChangeMode_eLastFramePreviousChannel;
      } else if ( ( false == eCurrentChannelChangeSettings.bHoldLastPicture )
               && ( true == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BRAP_VID_ChannelChangeMode_eMuteWithFirstPicturePreview;
      } else
      {
         *peChChangeMode = BRAP_VID_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview;
      }
   }

   BDBG_LEAVE(BRAP_VID_GetChannelChangeMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_SetInterpolationModeForStillContent: This API sets the SPIM for the
  video decoder.
****************************************************************************/
BERR_Code BRAP_VID_SetInterpolationModeForStillContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetInterpolationModeForStillContent);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetInterpolationModeForStillContent_isr(
      hRapCh,
      eNewInterpolation
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetInterpolationModeForStillContent);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetInterpolationModeForStillContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_SourceFormatOverride eSourceFormatOverride;
   BDBG_ENTER(BRAP_VID_SetInterpolationModeForStillContent_isr);

   BDBG_ASSERT(hRapCh);

   switch ( eNewInterpolation )
   {
      case BRAP_VID_StillContentInterpolationMode_eSingleField:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eInterlaced;
         break;

      case BRAP_VID_StillContentInterpolationMode_eBothField:
      case BRAP_VID_StillContentInterpolationMode_eFrame:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eProgressive;
       break;

      case BRAP_VID_StillContentInterpolationMode_eDefault:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eDefault;
         break;

      default:
         BDBG_ERR( ("Invalid SPIM Mode (%d)",
                  eNewInterpolation));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetSourceFormatOverride_isr(
            hRapCh->hPictureProvider,
            eSourceFormatOverride
            );

   BDBG_MSG( ("BRAP_VID_SetInterpolationModeForStillContent(%d)", eNewInterpolation));

   BDBG_LEAVE(BRAP_VID_SetInterpolationModeForStillContent_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_GetInterpolationModeForStillContent: This function gets the still
  content interpolation mode(SPIM).
****************************************************************************/
BERR_Code BRAP_VID_GetInterpolationModeForStillContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode *peStillContentIntrpMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetInterpolationModeForStillContent);

   rc = BRAP_VID_GetInterpolationModeForStillContent_isr(
      hRapCh,
      peStillContentIntrpMode
      );

   BDBG_LEAVE(BRAP_VID_GetInterpolationModeForStillContent);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetInterpolationModeForStillContent_isr
(
   BRAP_ChannelHandle   hRapCh,
   BRAP_VID_StillContentInterpolationMode *peStillContentIntrpMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_SourceFormatOverride eSourceFormatOverride;

   BDBG_ENTER(BRAP_VID_GetInterpolationModeForStillContent_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peStillContentIntrpMode);

   rc = BXDM_PictureProvider_GetSourceFormatOverride_isr(
            hRapCh->hPictureProvider,
            &eSourceFormatOverride
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eSourceFormatOverride )
      {
         case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:
            *peStillContentIntrpMode = BRAP_VID_StillContentInterpolationMode_eSingleField;
            break;

         case BXDM_PictureProvider_SourceFormatOverride_eProgressive:
            *peStillContentIntrpMode = BRAP_VID_StillContentInterpolationMode_eBothField;
            break;

         case BXDM_PictureProvider_SourceFormatOverride_eDefault:
            *peStillContentIntrpMode = BRAP_VID_StillContentInterpolationMode_eDefault;
            break;

         default:
            BDBG_ERR( ("Unknown XDM SPIM Mode (%d)",
                     eSourceFormatOverride));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BRAP_VID_GetInterpolationModeForStillContent_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_SetInterpolationModeForMovingContent: This API sets the MPIM for
  the video decoder.
****************************************************************************/
BERR_Code BRAP_VID_SetInterpolationModeForMovingContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetInterpolationModeForMovingContent);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetInterpolationModeForMovingContent_isr(
      hRapCh,
      eNewInterpolation
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetInterpolationModeForMovingContent);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetInterpolationModeForMovingContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ScanModeOverride eXDMScanModeOverride;
   BDBG_ENTER(BRAP_VID_SetInterpolationModeForMovingContent_isr);
   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(eNewInterpolation <= BRAP_VID_MovingContentInterpolationMode_eProgressiveScanout);

   switch ( eNewInterpolation )
   {
      case BRAP_VID_MovingContentInterpolationMode_eInterlacedScanout:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eInterlaced;
         break;

      case BRAP_VID_MovingContentInterpolationMode_eProgressiveScanout:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eProgressive;
         break;

      case BRAP_VID_MovingContentInterpolationMode_eDefault:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eDefault;
         break;

      default:
         BDBG_ERR( ("Invalid MPIM Mode (%d)",
                  eNewInterpolation));
         return BERR_TRACE(BERR_INVALID_PARAMETER);

   }

   rc = BXDM_PictureProvider_SetScanModeOverride_isr(
            hRapCh->hPictureProvider,
            eXDMScanModeOverride
            );

   BDBG_MSG( ("BRAP_VID_SetInterpolationModeForMovingContent(%d)", eNewInterpolation));

   BDBG_LEAVE(BRAP_VID_SetInterpolationModeForMovingContent_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_GetInterpolationModeForMovingContent: This function gets interpolation
  mode for moving content.
****************************************************************************/
BERR_Code BRAP_VID_GetInterpolationModeForMovingContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode *peNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetInterpolationModeForMovingContent);

   rc = BRAP_VID_GetInterpolationModeForMovingContent_isr(
      hRapCh,
      peNewInterpolation
      );

   BDBG_LEAVE(BRAP_VID_GetInterpolationModeForMovingContent);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetInterpolationModeForMovingContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode *peNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ScanModeOverride eScanModeOverride;

   BDBG_ENTER(BRAP_VID_GetInterpolationModeForMovingContent_isr);
   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peNewInterpolation);

   rc = BXDM_PictureProvider_GetScanModeOverride_isr(
            hRapCh->hPictureProvider,
            &eScanModeOverride
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eScanModeOverride )
      {

         case BXDM_PictureProvider_ScanModeOverride_eInterlaced:
            *peNewInterpolation = BRAP_VID_MovingContentInterpolationMode_eInterlacedScanout;
            break;

         case BXDM_PictureProvider_ScanModeOverride_eProgressive:
            *peNewInterpolation = BRAP_VID_MovingContentInterpolationMode_eProgressiveScanout;
            break;

         case BXDM_PictureProvider_ScanModeOverride_eDefault:
            *peNewInterpolation = BRAP_VID_MovingContentInterpolationMode_eDefault;
            break;

         default:
            BDBG_ERR( ("Unknown XDM MPIM Mode (%d)",
                     eScanModeOverride));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BRAP_VID_GetInterpolationModeForMovingContent_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BRAP_VID_EnableVideoFreeze: Used to freeze video while the decoder continues
  to run
****************************************************************************/
BERR_Code BRAP_VID_SetVideoFreezeState
(
   BRAP_ChannelHandle hRapCh,
   bool               bVFState
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetVideoFreezeState);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetVideoFreezeState_isr(
      hRapCh, bVFState
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetVideoFreezeState);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetVideoFreezeState_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               bVFState
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetVideoFreezeState_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_SetFreeze_isr(
            hRapCh->hPictureProvider,
            bVFState);

   BDBG_MSG( ("BRAP_VID_SetVideoFreezeState_isr()"));

   BDBG_LEAVE(BRAP_VID_SetVideoFreezeState_isr);
   return BERR_TRACE( rc );
}


/***************************************************************************
  BRAP_VID_GetVideoFreezeState: Used to get the video freeze state
****************************************************************************/
BERR_Code BRAP_VID_GetVideoFreezeState
(
   BRAP_ChannelHandle hRapCh,
   bool               *bVFState
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetVideoFreezeState);

   rc = BRAP_VID_GetVideoFreezeState_isr(
      hRapCh,
      bVFState
      );

   BDBG_LEAVE(BRAP_VID_GetVideoFreezeState);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetVideoFreezeState_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               *bVFState
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetVideoFreezeState_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(bVFState);

   rc = BXDM_PictureProvider_GetFreeze_isr(
            hRapCh->hPictureProvider,
            bVFState);

   BDBG_LEAVE(BRAP_VID_GetVideoFreezeState_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_Set240iScanMode
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_Set240iScanMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_Set240iScanMode_isr(
      hRapCh,
      e240iScanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_Set240iScanMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_Set240iScanMode_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_Set240iScanMode_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_Set240iScanMode_isr(
            hRapCh->hPictureProvider,
            e240iScanMode
            );

   BDBG_MSG( ("BRAP_VID_Set240iScanMode(%d)", e240iScanMode));

   BDBG_LEAVE(BRAP_VID_Set240iScanMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_Get240iScanMode
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_Get240iScanMode);

   rc = BRAP_VID_Get240iScanMode_isr(
      hRapCh,
      pe240iScanMode
      );

   BDBG_LEAVE(BRAP_VID_Get240iScanMode);

   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_Get240iScanMode_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_Get240iScanMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pe240iScanMode);

   rc = BXDM_PictureProvider_Get240iScanMode_isr(
            hRapCh->hPictureProvider,
            pe240iScanMode
            );

   BDBG_LEAVE(BRAP_VID_Get240iScanMode_isr);
   return BERR_TRACE( rc );
}


/******************************/
/* Display Manager APIs - TSM */
/******************************/

/***************************************************************************
BRAP_VID_SetDisplayOffset: API used to set the display offset.
****************************************************************************/
BERR_Code BRAP_VID_SetDisplayOffset
(
   BRAP_ChannelHandle hRapCh,
   long               lDisplayOffsetValue
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetDisplayOffset);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetDisplayOffset_isr(
      hRapCh,
      lDisplayOffsetValue
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetDisplayOffset);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetDisplayOffset_isr
(
   BRAP_ChannelHandle hRapCh,
   long               lDisplayOffsetValue
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetDisplayOffset_isr);
   BDBG_ASSERT(hRapCh);

   /* Set the display offset value */
   rc = BXDM_PictureProvider_SetPTSOffset_isr(
            hRapCh->hPictureProvider,
            (uint32_t) lDisplayOffsetValue
            );

   BDBG_MSG( ("BRAP_VID_SetDisplayOffset(%d)", lDisplayOffsetValue));

   BDBG_LEAVE(BRAP_VID_SetDisplayOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_GetDisplayOffset: API used to get the display offset.
****************************************************************************/
BERR_Code BRAP_VID_GetDisplayOffset
(
   BRAP_ChannelHandle hRapCh,
   long               *plDisplayOffsetValue
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetDisplayOffset);

   rc = BRAP_VID_GetDisplayOffset_isr(
      hRapCh,
      plDisplayOffsetValue);

   BDBG_LEAVE(BRAP_VID_GetDisplayOffset);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetDisplayOffset_isr
(
   BRAP_ChannelHandle hRapCh,
   long               *plDisplayOffsetValue
   )
{
   BERR_Code rc;
   uint32_t uiPTSOffset;

   BDBG_ENTER(BRAP_VID_GetDisplayOffset_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(plDisplayOffsetValue);

   /* get the display offset value */
   rc = BXDM_PictureProvider_GetPTSOffset_isr(
            hRapCh->hPictureProvider,
            &uiPTSOffset
            );

   *plDisplayOffsetValue = uiPTSOffset;

   BDBG_LEAVE(BRAP_VID_GetDisplayOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_SetVideoDisplayMode: Used to set the display mode.
****************************************************************************/
BERR_Code BRAP_VID_SetVideoDisplayMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   eDisplayMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetVideoDisplayMode);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetVideoDisplayMode_isr(
      hRapCh,
      eDisplayMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetVideoDisplayMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetVideoDisplayMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   eDisplayMode
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetVideoDisplayMode_isr);

   BDBG_ASSERT(hRapCh);

   /* Set the video display mode */
   rc = BXDM_PictureProvider_SetDisplayMode_isr(
            hRapCh->hPictureProvider,
            eDisplayMode
            );

   BDBG_MSG( ("BRAP_VID_SetVideoDisplayMode(%d)", eDisplayMode));

   BDBG_LEAVE(BRAP_VID_SetVideoDisplayMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_GetVideoDisplayMode: Used to get the display mode.
****************************************************************************/
BERR_Code BRAP_VID_GetVideoDisplayMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   *peDisplayMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetVideoDisplayMode);

   rc = BRAP_VID_GetVideoDisplayMode_isr(
      hRapCh,
      peDisplayMode
      );

   BDBG_LEAVE(BRAP_VID_GetVideoDisplayMode);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetVideoDisplayMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   *peDisplayMode
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetVideoDisplayMode_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peDisplayMode);

   /* Get the video display mode */
   rc = BXDM_PictureProvider_GetDisplayMode_isr(
            hRapCh->hPictureProvider,
            peDisplayMode
            );

   BDBG_LEAVE(BRAP_VID_GetVideoDisplayMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_GetDisplayThresholds: API used to get display threshold values
****************************************************************************/
BERR_Code BRAP_VID_GetDisplayThresholds
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_DisplayThresholds *pDispThresholds
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetDisplayThresholds);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetDisplayThresholds_isr(
      hRapCh,
      pDispThresholds
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetDisplayThresholds);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetDisplayThresholds_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_DisplayThresholds *pDispThresholds
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BRAP_VID_GetDisplayThresholds_isr);

   /* Check for validity of input ptrs */
   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pDispThresholds);

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hRapCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   pDispThresholds->ui32DiscardThreshold =
            eCurrentTSMThresholdSettings.uiTooEarlyThreshold;

   pDispThresholds->ui32VeryLateThreshold =
            eCurrentTSMThresholdSettings.uiTooLateThreshold;

   BDBG_LEAVE(BRAP_VID_GetDisplayThresholds_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_SetDiscardThreshold: API used to supply the decoder with display
  discard threshold.
****************************************************************************/
BERR_Code BRAP_VID_SetDiscardThreshold
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32DiscardThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetDiscardThreshold);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetDiscardThreshold_isr(
      hRapCh,
      ui32DiscardThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetDiscardThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetDiscardThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32DiscardThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BRAP_VID_SetDiscardThreshold_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hRapCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   if ( BERR_SUCCESS == rc )
   {
      eCurrentTSMThresholdSettings.uiTooEarlyThreshold = ui32DiscardThreshold;

      rc = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
               hRapCh->hPictureProvider,
               &eCurrentTSMThresholdSettings
               );

   BDBG_MSG( ("BRAP_VID_SetDiscardThreshold(%d)", ui32DiscardThreshold));
   }

   BDBG_LEAVE(BRAP_VID_SetDiscardThreshold_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BRAP_VID_SetVeryLateThreshold:    API used to supply the decoder with display
  very late threshold.
****************************************************************************/
BERR_Code BRAP_VID_SetVeryLateThreshold
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32VeryLateThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetVeryLateThreshold);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetVeryLateThreshold_isr(
      hRapCh,
      ui32VeryLateThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetVeryLateThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetVeryLateThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32VeryLateThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;
   BDBG_ENTER(BRAP_VID_SetVeryLateThreshold_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hRapCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   if ( BERR_SUCCESS == rc )
   {
      eCurrentTSMThresholdSettings.uiTooLateThreshold = ui32VeryLateThreshold;

      rc = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
               hRapCh->hPictureProvider,
               &eCurrentTSMThresholdSettings
               );

   BDBG_MSG( ("BRAP_VID_SetVeryLateThreshold(%d)", ui32VeryLateThreshold));
   }

   BDBG_LEAVE(BRAP_VID_SetVeryLateThreshold_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_SetSTCInvalidFlag: Used to set the state of the STC invalid flag
****************************************************************************/
BERR_Code BRAP_VID_SetSTCInvalidFlag
(
   BRAP_ChannelHandle hRapCh,
   bool bStcInvalidFlag
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetSTCInvalidFlag);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetSTCInvalidFlag_isr(
      hRapCh,
      bStcInvalidFlag
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetSTCInvalidFlag);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetSTCInvalidFlag_isr
(
   BRAP_ChannelHandle hRapCh,
   bool bStcInvalidFlag
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetSTCInvalidFlag_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_SetSTCValid_isr(
            hRapCh->hPictureProvider,
            !bStcInvalidFlag);

   BDBG_MSG( ("BRAP_VID_SetSTCInvalidFlag(%d)", bStcInvalidFlag));

   BDBG_LEAVE(BRAP_VID_SetSTCInvalidFlag_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BRAP_VID_GetSTCInvalidFlag: Used to get the current state of the STC invalid flag
****************************************************************************/
BERR_Code BRAP_VID_GetSTCInvalidFlag
(
   BRAP_ChannelHandle hRapCh,
   bool *pbStcInvalidFlag
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetStcInvalidFlag);

   rc = BRAP_VID_GetSTCInvalidFlag_isr(
      hRapCh,
      pbStcInvalidFlag
      );

   BDBG_LEAVE(BRAP_VID_GetStcInvalidFlag);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetSTCInvalidFlag_isr
(
   BRAP_ChannelHandle hRapCh,
   bool *pbStcInvalidFlag
   )
{
   BERR_Code rc;
   bool bCurrentSTCValid;
   BDBG_ENTER(BRAP_VID_GetStcInvalidFlag_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pbStcInvalidFlag);

   rc = BXDM_PictureProvider_GetSTCValid_isr(
            hRapCh->hPictureProvider,
            &bCurrentSTCValid);

   *pbStcInvalidFlag = !bCurrentSTCValid;

   BDBG_LEAVE(BRAP_VID_GetStcInvalidFlag_isr);
   return BERR_TRACE(rc);
}
#if 0
/***************************************************************************
   BRAP_VID_SetSTCSource: Set the STC source
****************************************************************************/
BERR_Code BRAP_VID_SetSTCSource
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BRAP_VID_STC eSTC               /* [in] STC Time base */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetSTCSource);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetSTCSource_isr(
      hRapCh,
      eSTC
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetSTCSource);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetSTCSource_isr(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BRAP_VID_STC eSTC               /* [in] STC Time base */
   )
{
   BERR_Code rc;
   uint32_t uiXDMSTCIndex;

   BDBG_ENTER(BRAP_VID_SetSTCSource_isr);

   BDBG_ASSERT(hRapCh);

   switch ( eSTC )
   {
      case BRAP_VID_STC_eOne:
         uiXDMSTCIndex = 1;
         break;

      case BRAP_VID_STC_eZero:
         uiXDMSTCIndex = 0;
         break;

      default:
         BDBG_ERR( ("Invalid STC: 0x%x (%d)",
                                    eSTC, eSTC));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetSTCIndex_isr(
            hRapCh->hPictureProvider,
            uiXDMSTCIndex
            );

   hRapCh->sDecodeSettings.eSTC = eSTC;
   BDBG_MSG( ("BRAP_VID_SetSTCSource(%d)", eSTC));

   BDBG_LEAVE(BRAP_VID_SetSTCSource_isr);
   return BERR_TRACE( rc );
}


/***************************************************************************
   BRAP_VID_GetSTCSource: Get the STC source
****************************************************************************/
BERR_Code BRAP_VID_GetSTCSource
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BRAP_VID_STC *peSTC             /* [out] STC Time base */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetSTCSource);

   rc = BRAP_VID_GetSTCSource_isr(
      hRapCh,
      peSTC
      );

   BDBG_LEAVE(BRAP_VID_GetSTCSource);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetSTCSource_isr
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BRAP_VID_STC *peSTC             /* [out] STC Time base */
   )
{
   BERR_Code rc;
   uint32_t uiXDMSTCIndex;

   BDBG_ENTER(BRAP_VID_GetSTCSource_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(peSTC);

   rc = BXDM_PictureProvider_GetSTCIndex_isr(
            hRapCh->hPictureProvider,
            &uiXDMSTCIndex
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( uiXDMSTCIndex )
      {
         case 0:
            *peSTC = BRAP_VID_STC_eZero;
            break;

         case 1:
            *peSTC = BRAP_VID_STC_eOne;
            break;

         default:
            BDBG_ERR( ("Unknown XDM STC Source: 0x%x (%d)",
                     uiXDMSTCIndex, uiXDMSTCIndex));

            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BRAP_VID_GetSTCSource_isr);
   return BERR_TRACE( rc );
}
/***************************************************************************
   BRAP_VID_GetSTCValue: Get's the STC snapshot
****************************************************************************/
BERR_Code BRAP_VID_GetSTCValue
(
   BRAP_VID_Handle hXvd,                         /* [in] The XVD handle */
   BRAP_VID_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BRAP_VID_STC eSTC,                            /* [in] STC Time base */
   BRAP_VID_STCInfo *pSTCInfo                    /* [out] STC info */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetSTCValue);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetSTCValue_isr(
      hXvd,
      eDisplayInterrupt,
      eSTC,
      pSTCInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetSTCValue);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetSTCValue_isr
(
   BRAP_VID_Handle hXvd,                         /* [in] The XVD handle */
   BRAP_VID_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BRAP_VID_STC eSTC,                            /* [in] STC Time base */
   BRAP_VID_STCInfo *pSTCInfo                    /* [out] STC info */
   )
{
   BERR_Code rc;
   BXDM_DisplayInterruptInfo stDisplayInterruptInfo;

   BDBG_ENTER(BRAP_VID_GetSTCValue_isr);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(pSTCInfo);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BRAP_VID_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BRAP_VID_ERR_INVALID_HANDLE);
   }

   if ( eDisplayInterrupt >= BRAP_VID_DisplayInterrupt_eMax )
   {
         BRAP_VID_DBG_ERR(hXvd, ("Invalid Display Interrupt requested: 0x%x (%d)",
                                  eDisplayInterrupt));
      return BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   rc = BRAP_VID_DisplayInterruptProvider_GetDisplayInterruptInfo_isr(
            hXvd->hXvdDipCh[eDisplayInterrupt],
                                                              &stDisplayInterruptInfo );

   if ( BERR_SUCCESS == rc )
   {
   pSTCInfo->eInterruptPolarity = stDisplayInterruptInfo.eInterruptPolarity;

   if ( ( eSTC >= stDisplayInterruptInfo.uiSTCCount )
        || ( false == stDisplayInterruptInfo.astSTC[eSTC].bValid ) )
   {
         BRAP_VID_DBG_ERR(hXvd, ("Invalid STC requested: 0x%x (%d)",
                                  eSTC));
         return BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   pSTCInfo->ui32STCValue = stDisplayInterruptInfo.astSTC[eSTC].uiValue;
   }

   BDBG_LEAVE(BRAP_VID_GetSTCValue_isr);
   return BERR_TRACE( rc );
}
#endif

/***************************************************************************
 ***************************************************************************/
BERR_Code BRAP_VID_SetClipTime
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BXDM_PictureProvider_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetClipTime);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetClipTime_isr(
      hRapCh,
      eClipTimeType,
      ui32StartTime,
      ui32StopTime
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetClipTime);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetClipTime_isr
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BXDM_PictureProvider_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ClipTimeSettings stNewClipTimeSettings;

   BDBG_ENTER(BRAP_VID_SetClipTime_isr);

   BDBG_ASSERT(hRapCh);

   BKNI_Memset( &stNewClipTimeSettings, 0, sizeof( BXDM_PictureProvider_ClipTimeSettings) );

   stNewClipTimeSettings.uiStart = ui32StartTime;
   stNewClipTimeSettings.uiStop = ui32StopTime;
   stNewClipTimeSettings.bValid = true;
   stNewClipTimeSettings.eType = eClipTimeType;

   rc = BXDM_PictureProvider_SetClipTimeSettings_isr(
            hRapCh->hPictureProvider,
            &stNewClipTimeSettings
            );

   BDBG_MSG( ("BRAP_VID_SetClipTime(%d,%u,%u)", eClipTimeType, ui32StartTime, ui32StopTime));

   BDBG_LEAVE(BRAP_VID_SetClipTime_isr);
   return BERR_TRACE( rc );
}
# if 0
/***************************************************************************
****************************************************************************/
BERR_Code BRAP_VID_SetTSMWaitForValidSTC
(
   BRAP_ChannelHandle hRapCh /* [in] The XVD Channel handle */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetTSMWaitForValidSTC);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetTSMWaitForValidSTC_isr(
      hRapCh
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetTSMWaitForValidSTC);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetTSMWaitForValidSTC_isr
(
   BRAP_ChannelHandle hRapCh /* [in] The XVD Channel handle */
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BRAP_VID_SetTSMWaitForValidSTC_isr);

   /* Check to make sure the RequestSTC callback is installed */
   if (hRapCh->stInterruptCallbackInfo[BRAP_VID_Interrupt_eRequestSTC].BRAP_VID_P_pAppIntCallbackPtr == NULL)
   {
      BDBG_ERR( ("BRAP_VID_Interrupt_eRequestSTC callback must be installed"));
      return BERR_TRACE(BERR_NOT_INITIALIZED);
   }

   /* Enable the RequestSTC callback */
   rc = BRAP_VID_EnableInterrupt_isr(hRapCh,
                                 BRAP_VID_Interrupt_eRequestSTC,
                                 BRAP_VID_InterruptEnable_eEnable);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Invalidate STC */
   rc = BRAP_VID_SetSTCInvalidFlag_isr(hRapCh,
                                   true);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BDBG_MSG( ("BRAP_VID_SetTSMWaitForValidSTC()"));

   BDBG_LEAVE(BRAP_VID_SetTSMWaitForValidSTC_isr);
   return BERR_TRACE(rc);
}
#endif

/***************************************************************************
   BRAP_VID_SetSwPcrOffset
****************************************************************************/
BERR_Code BRAP_VID_SetSwPcrOffset
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_SetSwPcrOffset);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetSwPcrOffset_isr(
      hRapCh,
      uiSwPcrOffset
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetSwPcrOffset);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetSwPcrOffset_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetSwPcrOffset_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_SetSoftwarePCROffset_isr(
            hRapCh->hPictureProvider,
            uiSwPcrOffset
            );

   BDBG_MSG( ("BRAP_VID_SetSwPcrOffset_isr(%d)", uiSwPcrOffset));

   BDBG_LEAVE(BRAP_VID_SetSwPcrOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BRAP_VID_GetSwPcrOffset
****************************************************************************/
BERR_Code BRAP_VID_GetSwPcrOffset
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BRAP_VID_GetSwPcrOffset);

   rc = BRAP_VID_GetSwPcrOffset_isr(
      hRapCh,
      puiSwPcrOffset
      );

   BDBG_LEAVE(BRAP_VID_GetSwPcrOffset);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetSwPcrOffset_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetSwPcrOffset_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(puiSwPcrOffset);

   rc = BXDM_PictureProvider_GetSoftwarePCROffset_isr(
            hRapCh->hPictureProvider,
            puiSwPcrOffset
            );

   BDBG_LEAVE(BRAP_VID_GetSwPcrOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BRAP_VID_SetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BRAP_VID_SetHwPcrOffsetEnable
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetHwPcrOffsetEnable);

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetHwPcrOffsetEnable_isr(
      hRapCh,
      bHwPcrOffsetEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetHwPcrOffsetEnable);
   return BERR_TRACE(rc);
}


BERR_Code BRAP_VID_SetHwPcrOffsetEnable_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetHwPcrOffsetEnable_isr);

   BDBG_ASSERT(hRapCh);

   rc = BXDM_PictureProvider_SetHardwarePCROffsetMode_isr(
            hRapCh->hPictureProvider,
            bHwPcrOffsetEnable
            );

   BDBG_MSG( ("BRAP_VID_SetHwPcrOffsetEnable_isr(%d)", bHwPcrOffsetEnable));

   BDBG_LEAVE(BRAP_VID_SetHwPcrOffsetEnable_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BRAP_VID_GetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BRAP_VID_GetHwPcrOffsetEnable
(
   BRAP_ChannelHandle hRapCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetHwPcrOffsetEnable);

   rc = BRAP_VID_GetHwPcrOffsetEnable_isr(
      hRapCh,
      pbHwPcrOffsetEnable
      );

   BDBG_LEAVE(BRAP_VID_GetHwPcrOffsetEnable);
   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetHwPcrOffsetEnable_isr
(
   BRAP_ChannelHandle hRapCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetHwPcrOffsetEnable_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(pbHwPcrOffsetEnable);

   rc = BXDM_PictureProvider_GetHardwarePCROffsetMode_isr(
            hRapCh->hPictureProvider,
            pbHwPcrOffsetEnable
            );

   BDBG_LEAVE(BRAP_VID_GetHwPcrOffsetEnable_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BRAP_VID_SetVsyncPlaybackRate
****************************************************************************/

BERR_Code BRAP_VID_SetVsyncPlaybackRate(
   BRAP_ChannelHandle hRapCh,
   uint32_t uiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_SetVsyncPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BRAP_VID_SetVsyncPlaybackRate_isr( hRapCh, uiVsyncPlaybackRate );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_SetVsyncPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_SetVsyncPlaybackRate_isr(
   BRAP_ChannelHandle hRapCh,
   uint32_t uiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BXDM_Picture_Rate stPlaybackRate;
   BDBG_ENTER(BRAP_VID_SetVsyncPlaybackRate_isr);

   BDBG_ASSERT(hRapCh);

   /* "uiVsyncPlaybackRate" is a percentage . */
   BKNI_Memset(&stPlaybackRate, 0, sizeof( BXDM_Picture_Rate ) );

   stPlaybackRate.uiNumerator = uiVsyncPlaybackRate;
   stPlaybackRate.uiDenominator = 100;

   rc = BXDM_PictureProvider_SetPlaybackRate_isr(
            hRapCh->hPictureProvider,
            &stPlaybackRate
            );

   BDBG_LEAVE(BRAP_VID_SetVsyncPlaybackRate_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
   BRAP_VID_GetVsyncPlaybackRate
****************************************************************************/

BERR_Code BRAP_VID_GetVsyncPlaybackRate(
   BRAP_ChannelHandle hRapCh,
   uint32_t * puiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BDBG_ENTER(BRAP_VID_GetVsyncPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BRAP_VID_GetVsyncPlaybackRate_isr( hRapCh, puiVsyncPlaybackRate );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BRAP_VID_GetVsyncPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BRAP_VID_GetVsyncPlaybackRate_isr(
   BRAP_ChannelHandle hRapCh,
   uint32_t * puiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BXDM_Picture_Rate stCurrentPlaybackRate;
   BDBG_ENTER(BRAP_VID_GetVsyncPlaybackRate_isr);

   BDBG_ASSERT(hRapCh);
   BDBG_ASSERT(puiVsyncPlaybackRate);

   /* "*puiVsyncPlaybackRate" is a percentage, divide by the number of
    * extra decimal positions used for the DM internal value.
    */
   rc = BXDM_PictureProvider_GetPlaybackRate_isr(
            hRapCh->hPictureProvider,
            &stCurrentPlaybackRate
            );

   *puiVsyncPlaybackRate = (stCurrentPlaybackRate.uiNumerator * 100) / stCurrentPlaybackRate.uiDenominator;

   BDBG_LEAVE(BRAP_VID_GetVsyncPlaybackRate_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
    BRAP_VID_[Set/Get]JitterToleranceImprovementEnable: Used to enable/disable
    Jitter Tolerance Improvement logic in XVD DM
****************************************************************************/

BERR_Code BRAP_VID_SetJitterToleranceImprovementEnable(
   BRAP_ChannelHandle hRapCh,
   bool bEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER( BRAP_VID_SetJitterToleranceImprovementEnable );

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_SetJitterToleranceImprovementEnable_isr(
      hRapCh,
      bEnable
      );
   BKNI_LeaveCriticalSection();
   
   BDBG_LEAVE( BRAP_VID_SetJitterToleranceImprovementEnable );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_SetJitterToleranceImprovementEnable_isr(
   BRAP_ChannelHandle hRapCh,
   bool bEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER( BRAP_VID_SetJitterToleranceImprovementEnable_isr );
   
   BDBG_ASSERT( hRapCh );

   rc = BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr(
            hRapCh->hPictureProvider,
            bEnable
            );

   BDBG_MSG( ("BRAP_VID_SetJitterToleranceImprovementEnable(%d)", bEnable));

   BDBG_LEAVE( BRAP_VID_SetJitterToleranceImprovementEnable_isr );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_GetJitterToleranceImprovementEnable(
   BRAP_ChannelHandle hRapCh,
   bool *pbEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER( BRAP_VID_GetJitterToleranceImprovementEnable );

   BKNI_EnterCriticalSection();
   rc = BRAP_VID_GetJitterToleranceImprovementEnable_isr(
      hRapCh,
      pbEnable
      );
   BKNI_LeaveCriticalSection();
   
   BDBG_LEAVE( BRAP_VID_GetJitterToleranceImprovementEnable );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_GetJitterToleranceImprovementEnable_isr(
   BRAP_ChannelHandle hRapCh,
   bool *pbEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER( BRAP_VID_GetJitterToleranceImprovementEnable_isr );
   
   BDBG_ASSERT( hRapCh );
   BDBG_ASSERT( pbEnable );

   rc = BXDM_PictureProvider_GetJitterToleranceImprovementMode_isr(
            hRapCh->hPictureProvider,
            pbEnable
            );

   BDBG_LEAVE( BRAP_VID_GetJitterToleranceImprovementEnable_isr );

   return BERR_TRACE( rc );
}

#if 0
/***************************************************************************
    BRAP_VID_[Set/Get]MPEGPulldownOverride:
    Set the pulldown override mode for MPEG content.
****************************************************************************/

BERR_Code BRAP_VID_SetMPEGPulldownOverride(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MPEGPulldownOverride ePulldownOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BRAP_VID_SetMPEGPulldownOverride );

   BKNI_EnterCriticalSection();

   rc = BRAP_VID_SetMPEGPulldownOverride_isr( hRapCh, ePulldownOverride );

   BKNI_LeaveCriticalSection();
   
   BDBG_LEAVE( BRAP_VID_SetMPEGPulldownOverride );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_SetMPEGPulldownOverride_isr(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MPEGPulldownOverride ePulldownOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BRAP_VID_SetMPEGPulldownOverride_isr );
   
   BDBG_ASSERT( hRapCh );

#warning [XVD Decoder] Implement MPEG Pulldown Override
   hRapCh->stDecoderConfig.ePulldownOverride = ePulldownOverride;

   BDBG_MSG( ("BRAP_VID_SetMPEGPulldownOverride(%d)", ePulldownOverride));

   BDBG_LEAVE( BRAP_VID_SetMPEGPulldownOverride_isr );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_GetMPEGPulldownOverride(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MPEGPulldownOverride * pePulldownOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BRAP_VID_GetMPEGPulldownOverride );

   BKNI_EnterCriticalSection();

   rc = BRAP_VID_GetMPEGPulldownOverride_isr( hRapCh, pePulldownOverride );

   BKNI_LeaveCriticalSection();
   
   BDBG_LEAVE( BRAP_VID_GetMPEGPulldownOverride );

   return BERR_TRACE( rc );
}

BERR_Code BRAP_VID_GetMPEGPulldownOverride_isr(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MPEGPulldownOverride * pePulldownOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BRAP_VID_GetMPEGPulldownOverride_isr );
   
   BDBG_ASSERT( hRapCh );
   BDBG_ASSERT( pePulldownOverride );

   *pePulldownOverride = hRapCh->stDecoderConfig.ePulldownOverride;

   BDBG_LEAVE( BRAP_VID_GetMPEGPulldownOverride_isr );

   return BERR_TRACE( rc );
}
#endif


/***************************************************************************
Summary:
    Creates the Message Queue
    
Description:

	Allocate memory for the message queue in system memory
	Initializes attributes in the DRAM(shared copy)
	Initializes the attributes in the local copy in the sytem memory

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_CreateMsgQueue(BRAP_FWIF_MsgQueueParams *psMsgQueueParams ,/* [in]*/
                            		      BMEM_Handle    hHeap,/* [in] */
                            		      BREG_Handle    hRegister,/* [in] */
                            		      uint32_t       ui32DspOffset, /* [in] */
                            		      BRAP_FWIF_MsgQueueHandle  *hMsgQueue, /*[out]*/
                            		      bool bWdgRecovery /*bWatchdogrecovery*/)
 {

    BERR_Code err=BERR_SUCCESS;

    uint32_t    ui32BaseAddrOffset=0;
    uint32_t    ui32EndAddrOffset=0;
    BRAP_FWIF_MsgQueueHandle  hHandle = NULL;
    uint32_t    ui32RegOffset = 0;

    BDBG_ENTER(BRAP_FWIF_P_CreateMsgQueue);
    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(hHeap);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hMsgQueue);
    BDBG_MSG(("psMsgQueueParams->ui32FifoId > %d", psMsgQueueParams->ui32FifoId));
    BDBG_ASSERT(psMsgQueueParams->ui32FifoId != BRAP_DSP_P_FIFO_INVALID);
	
    if(false == bWdgRecovery)
    {
        /* Allocate memory for the Message Queue */
        hHandle =(BRAP_FWIF_MsgQueueHandle)BKNI_Malloc(sizeof(struct BRAP_FWIF_MsgQueue));
        if(hHandle == NULL)
        {
            err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto exit;
        }
        BKNI_Memset (hHandle, 0, sizeof(struct BRAP_FWIF_MsgQueue));              

        /* Conversion of address from physical to virtual*/	    
        if(BERR_SUCCESS !=(err=BRAP_P_ConvertAddressToOffset(hHeap,(void *)psMsgQueueParams->uiBaseAddr,&ui32BaseAddrOffset)))
        {
            BDBG_ERR(("Conversion Failed"));
            if(hHandle)
            BKNI_Free(hHandle);
            goto exit;
        }

        BKNI_Memset((void *)psMsgQueueParams->uiBaseAddr,0,psMsgQueueParams->uiMsgQueueSize);

        /*Initializing attributes of message queue in DRAM (device memory)*/
        ui32RegOffset = BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_1 - BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0;
        BRAP_Write32(hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * psMsgQueueParams->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_BASE_OFFSET + ui32DspOffset, ui32BaseAddrOffset); /* base */

        ui32EndAddrOffset=ui32BaseAddrOffset+(psMsgQueueParams->uiMsgQueueSize);	
        BRAP_Write32(hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * psMsgQueueParams->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_END_OFFSET + ui32DspOffset, ui32EndAddrOffset); /* end */

        BRAP_Write32(hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * psMsgQueueParams->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + ui32DspOffset, ui32BaseAddrOffset); /* read */

        BRAP_Write32(hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * psMsgQueueParams->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + ui32DspOffset, ui32BaseAddrOffset); /* write */

        /* Initializes attributes in the local copy(handle) in system memory*/

        hHandle->hHeap=hHeap;
        hHandle->hRegister = hRegister;
        hHandle->uiBaseAddr=ui32BaseAddrOffset ;
        hHandle->uiEndAddr=ui32EndAddrOffset;
        hHandle->uiReadPtr=ui32BaseAddrOffset;
        hHandle->uiWritePtr=ui32BaseAddrOffset;
        hHandle->ui32FifoId = psMsgQueueParams->ui32FifoId;
        hHandle->ui32DspOffset = ui32DspOffset;

        *hMsgQueue = hHandle;
    }
    else
    {
        hHandle = *hMsgQueue;
    }

exit:
	
   BDBG_LEAVE(BRAP_FWIF_P_CreateMsgQueue);
   return err;

}

/***************************************************************************
Summary:
    Destroys the Message Queue
    
Description:
    Free the memory that was allocated for the Message Queue

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_DestroyMsgQueue(BRAP_FWIF_MsgQueueHandle   hMsgQueue, 
                                        BRAP_DSP_Handle          hDsp)
 
{
	BERR_Code err=BERR_SUCCESS;
    uint32_t    ui32RegOffset = 0;
	BDBG_ENTER(BRAP_FWIF_P_DestroyMsgQueue);
	BDBG_ASSERT(hMsgQueue);

    /*Reseting the FIFO buffers to invalid dram address*/
    ui32RegOffset = BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_1 - BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0;
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_BASE_OFFSET + hDsp->ui32Offset, BRAP_P_INVALID_DRAM_ADDRESS); 
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_END_OFFSET + hDsp->ui32Offset, BRAP_P_INVALID_DRAM_ADDRESS); 
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hDsp->ui32Offset, BRAP_P_INVALID_DRAM_ADDRESS); 
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + ui32RegOffset * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hDsp->ui32Offset, BRAP_P_INVALID_DRAM_ADDRESS); 
    
    BKNI_Free(hMsgQueue);
	BDBG_LEAVE	(BRAP_FWIF_P_DestroyMsgQueue);
	return err;

}

/***************************************************************************
Summary:
    Writes a message into the message queue reading from the message buffer
    
Description:
	Sanity check is done to check if the read and write pointers haven't been corrupted
	Checks for free space in the message queue.BUFFER FULL error is generated if there no free space in the message queue.
	Buffer_Size/4 number of bytes are copied from the Message buffer into the message queue 
	taking wrap around also into consideration
	Write Pointers are updated in both the QueueAttr Structure and the handle.
	
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_WriteMsg(BRAP_FWIF_MsgQueueHandle   hMsgQueue/*[in]*/,
									     void *pMsgBuf, /*[in]*/
									     unsigned int uiBufSize/*[in]*/)

{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BRAP_P_WriteMsg);

	rc = BRAP_P_WriteMsg_isr(hMsgQueue, pMsgBuf,uiBufSize);

    BDBG_LEAVE(BRAP_P_WriteMsg);
    return rc;
}

/***************************************************************************
Summary:
    Isr function of BRAP_FWIF_P_WriteMsg
    Writes a message into the message queue reading from the message buffer
    
Description:
	Sanity check is done to check if the read and write pointers haven't been corrupted
	Checks for free space in the message queue.BUFFER FULL error is generated if there no free space in the message queue.
	Buffer_Size/4 number of bytes are copied from the Message buffer into the message queue 
	taking wrap around also into consideration
	Write Pointers are updated in both the QueueAttr Structure and the handle.
	
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_WriteMsg_isr(BRAP_FWIF_MsgQueueHandle   hMsgQueue/*[in]*/,
									     void *pMsgBuf, /*[in]*/
									     unsigned int uiBufSize/*[in]*/)
{	
	BERR_Code err=BERR_SUCCESS;
	unsigned int uiFreeSpace=0;
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0; 
	void *pvMsgQueueWriteAddr=NULL;
	
		
	BDBG_ENTER(BRAP_P_WriteMsg_isr);

	BDBG_ASSERT(hMsgQueue);
	BDBG_ASSERT(pMsgBuf);
	
	ui32dramReadPtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset);
	
	ui32dramWritePtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset);
	
        ui32maskReadPtr=ui32dramReadPtr;
        ui32maskWritePtr=ui32dramWritePtr;

        /*Sanity check*/
	/*Checking boundness of read pointer- if((readptr>endaddr) OR (readptr<baseaddr)) read ptr not within bound*/
	if((ui32maskReadPtr>hMsgQueue->uiEndAddr)||(ui32maskReadPtr<hMsgQueue->uiBaseAddr))
	{
		BDBG_ERR(("Read pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
	/*Checking boundness of write pointer - if((writeptr>endaddr) OR (writeptr<baseaddr))  write ptr not within bound*/
	if((ui32maskWritePtr>hMsgQueue->uiEndAddr)||(ui32maskWritePtr<hMsgQueue->uiBaseAddr))
	{
		BDBG_ERR(("Write pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	/*update read ptr */
	hMsgQueue->uiReadPtr= ui32maskReadPtr;

	/* checking write ptrs */
	if((hMsgQueue->uiWritePtr)!=ui32maskWritePtr)
	{
		BDBG_ERR(("Write pointer corrupted in the Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);	
	}
    
	
/* End of Sanity Check */
/*******************************************************************
   Different cases: 

    If maskwriteptr> maskreadptr
     ReadPtrMSB   WritePtrMSB 	freespace
        0   				0          freespace=((endaddr-writeptr)+(readptr-baseaddr));           		
        0   				1   	     this condn not possible	
        1   				0   	     this condn. not possible
        1   				1   	     freespace=((endaddr-writeptr)+(readptr-baseaddr));	

  If maskreadptr>maskwriteptr
   ReadptrMSB 	WritePtrMSB	 freespace 
    0   				0   		  this condn. not possible	
    0   				1   		  read-write
    1   				0   		  read-write
    1   				1   		  this condn. not possible

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,then the buffer is empty
  If the toggle bits are different,the buffer is full
 ***********************************************************************/

	
	/*Calc free space in the message queue*/

	/* Case1:if(maskwriteptr>maskreadptr)*/
	if(ui32maskWritePtr > ui32maskReadPtr)
	{	
		uiFreeSpace=((hMsgQueue->uiEndAddr-ui32maskWritePtr)+
                     (ui32maskReadPtr-hMsgQueue->uiBaseAddr))-uiBufSize;
	}
	

	/* Case2:if(maskreadptr>maskwriteptr) */
	if(ui32maskReadPtr>ui32maskWritePtr)
	{
		uiFreeSpace=(ui32maskReadPtr-ui32maskWritePtr)-uiBufSize;
	}

	
	/* Case3:if(maskreadptr==maskwriteptr) */
	if(ui32maskReadPtr==ui32maskWritePtr)
	{
		/* The buffer is empty */
		uiFreeSpace=(hMsgQueue->uiEndAddr-hMsgQueue->uiBaseAddr)-uiBufSize;
	}

	/*Generate BUFFER_FULL error  when there is no space for the message to be written into the message queue*/
	/* BRAP_ERR_BUFFER_FULL is a new error that has been defined in brap.h */
		
	if(uiFreeSpace <= 0)/*Considering the buffer has only one message (i.e) uiBufSize=MESSAGE_SIZE */
	{
		BDBG_ERR(("No Free space in the buffer.No new messages can be written"));
		return BERR_TRACE(BRAP_ERR_BUFFER_FULL);
	}
	
	/*writing msgs into the message queue*/
	/*Assume:the Message buffer always has only one message*/
	
	BDBG_MSG(("Buffer size should be a multiple of 4"));
	BDBG_ASSERT(!(uiBufSize%4));
    BDBG_MSG(("uiBufSize > %d", uiBufSize));
	
	/*This conversion is done to pass a Phaysical address as the First argument of BRAP_MemWrite32*/
	if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskWritePtr,(void **)&pvMsgQueueWriteAddr)))
		return BERR_TRACE(err);
	   
	/*Writing into Message queue*/
    BDBG_MSG(("In BRAP_P_WriteMsg_isr *(uint32_t *)pMsgBuf > 0x%x\n", *((uint32_t *)pMsgBuf)));
	BRAP_MemWrite32_isr((uint32_t)pvMsgQueueWriteAddr, *((uint32_t *)pMsgBuf));
	ui32dramWritePtr=ui32dramWritePtr+4;

    /* Taking wrap-around into consideration*/
    if(ui32dramWritePtr==hMsgQueue->uiEndAddr)
        ui32dramWritePtr=hMsgQueue->uiBaseAddr;
	
	/*updating write ptr in the Queue Attribute Structure*/
	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset, ui32dramWritePtr); 
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	
	/*updating write ptr in the handle*/
	hMsgQueue->uiWritePtr=ui32dramWritePtr;
	BDBG_LEAVE(BRAP_P_WriteMsg_isr);
	return err;
		
  }
/***************************************************************************
Summary:
	Gets a message from the message queue and writes in into the message buffer    
    
Description:
	Sanity check is done to check if the read and write pointers haven't been corrupted in the shared copy.
	Checks if a message is present. If no message is there in the message queue BUFFER_EMPTY error is returned
	MESSAGE_SIZE/4 number of words are read from the msg buffer into the message queue
	Read Pointers are updated both in the Queue Attribute Structure and the handle   

Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_P_GetMsg(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 uint32_t *pMsgBuf,bool bReadUpdate)
   
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BRAP_P_GetMsg);

	rc = BRAP_P_GetMsg_isr(hMsgQueue,pMsgBuf,bReadUpdate);

    BDBG_LEAVE(BRAP_P_GetMsg);
    return rc;
}

/***************************************************************************
Summary:
	Gets a message from the message queue and writes in into the message buffer    
    
Description:
    This is the isr version of BRAP_FWIF_P_GetMsg.

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_GetMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 uint32_t *pMsgBuf,bool bReadUpdate)
   
{
	BERR_Code err=BERR_SUCCESS;
	
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0;
	int32_t i32BytesToBeRead=0;
	void *pvMsgQueueReadAddr=NULL;
	
	BDBG_ENTER(BRAP_P_GetMsg_isr);
	BDBG_ASSERT(hMsgQueue);
	BDBG_ASSERT(pMsgBuf);

	 
	ui32dramReadPtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset);

	ui32dramWritePtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset);
       
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	ui32maskReadPtr=ui32dramReadPtr;
	ui32maskWritePtr=ui32dramWritePtr;
	   
	 /*Sanity check*/
	/* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr is not within bound*/	
	if((ui32maskWritePtr>hMsgQueue->uiEndAddr)||(ui32maskWritePtr<hMsgQueue->uiBaseAddr))
	{
		BDBG_ERR(("Write pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	
	/* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound*/
	if((ui32maskReadPtr>hMsgQueue->uiEndAddr)||(ui32maskReadPtr<hMsgQueue->uiBaseAddr))
	{
		BDBG_ERR(("Read pointer not within bounds in Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
	/*Updating write ptr */
	hMsgQueue->uiWritePtr= ui32maskWritePtr;

	/* checking read ptrs to see if they are the same */
	if((hMsgQueue->uiReadPtr)!=ui32maskReadPtr)
	{
		BDBG_ERR(("Read pointer corrupted in the Message Queue"));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE );		
	}

	/* End of Sanity Check */

	/*******************************************************************
   Different cases: 

    If maskreadptr>maskwriteptr
     ReadPtrMSB   WritePtrMSB 		
        0   				0                   This condn. is not possible
        0   				1   	     		(end-read)+(write-base)
        1   				0   	              (end-read)+(write-base)
        1   				1   	     	       this condn. is not possible

  If maskwriteptr>maskreadptr
   ReadptrMSB 	WritePtrMSB	 
    0   				0   		  	write-read
    0   				1   		       this condn. not possible
    1   				0   		       this condn. not possible
    1   				1   		       write-read

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,no message to read
  If the toggle bits are different all the messages have to be read
   

 ***********************************************************************/
	/*Condn. for reading messages from the message queue into the message buffer*/
	/* If no msg is to be read, generate a BRAP_ERR_BUFFER_EMPTY error(new error defined in brap.h)*/

	/* Checking if a msg is present */
	
	/* Case1:if(readptr>writeptr)*/
	if(ui32maskReadPtr > ui32maskWritePtr)
	{	
		i32BytesToBeRead=(hMsgQueue->uiEndAddr-ui32maskReadPtr)+
                (ui32maskWritePtr-hMsgQueue->uiBaseAddr);
	}

	 /* Case2:if(writeptr>readptr) */
	if(ui32maskWritePtr>ui32maskReadPtr)
	{
		i32BytesToBeRead=ui32maskWritePtr-ui32maskReadPtr;
	}

    /*Case 3:if readptr == writeptr */
    if(ui32maskWritePtr ==ui32maskReadPtr)
    {	
        /*All messages have to be read*/
        i32BytesToBeRead=0;
    }
	 if(i32BytesToBeRead == 0)
	 {
		BDBG_ERR(("The Message Queue is empty.No message is present."));
    	return err;
	 }
	 
	
	/*This conversion is done to pass a Phaysical address to BRAP_MemRead32_isr*/
	if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(err);
	
	
	/*Reading Message from the message queue into the message buffer*/
	*pMsgBuf=BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr);

    BDBG_MSG(("In BRAP_P_GetMsg_isr *pMsgBuf = 0x%x\n",*pMsgBuf));
    
    if ((bReadUpdate == true)&&(i32BytesToBeRead!=0))
    {
    	ui32dramReadPtr=ui32dramReadPtr+4;
        if(ui32dramReadPtr==hMsgQueue->uiEndAddr)
            ui32dramReadPtr=hMsgQueue->uiBaseAddr;        

    	/*updating read ptr in the Queue Attribute Structure*/
    	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                        BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, ui32dramReadPtr); 
    	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
    	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
    		
    	/*updating read ptr in the handle*/
    	hMsgQueue->uiReadPtr = ui32dramReadPtr;
    }
	
	BDBG_LEAVE(BRAP_P_GetMsg_isr);
	return err;
 
}


