/***************************************************************************
*     Copyright (c) 2006-2012, Broadcom Corporation
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
*	This file contains low level functions to interact with firmware and DSP hardware.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"
#include "bkni.h"
#include "bdbg.h"
#include "berr.h"
#include "brap_fwif_priv.h"
#include "brap_af_priv.h"


BDBG_MODULE(rap_fwif);

#if (BRAP_3548_FAMILY == 1)
/* 
    The problem happens when the decoder is set to 2_0 + LFE mode
    We still get 2_0 which is essentially stereo but since we have
    LFE we have to be able to support more than 2_0 in such cases
    So we have made sure that any PP algorithm that can support 2_0 + LFE will
    get 5_1 mode.
*/
#define LFE_WORKAROUND 0
#else
#define LFE_WORKAROUND 0
#endif

extern const BRAP_AF_P_PpOutMode BRAP_sPpOutMode[BRAP_CIT_P_ProcessingType_eMax];


const BRAP_FWIF_P_MapProcessingEnum BRAP_sMapProcessingEnum[/*BRAP_ProcessingType_eMax*/]=
{
	{{BRAP_CIT_P_ProcessingType_eGenericPassThru},BRAP_CIT_P_AudioAlgoType_ePostProc},   /*Passthru*/
	{{BRAP_CIT_P_EncAudioType_eMpeg1Layer3},BRAP_CIT_P_AudioAlgoType_eEncode},	/*MP3 Encode*/
	{{BRAP_CIT_P_EncAudioType_eDTSBroadcast},BRAP_CIT_P_AudioAlgoType_eEncode},/*DTSEncode*/
	{{BRAP_CIT_P_EncAudioType_eAc3},BRAP_CIT_P_AudioAlgoType_eEncode},/*Ac3Encode*/
	{{BRAP_CIT_P_EncAudioType_eSbc},BRAP_CIT_P_AudioAlgoType_eEncode},/*SBC Encode*/			
	{{BRAP_CIT_P_ProcessingType_eDDConvert},BRAP_CIT_P_AudioAlgoType_ePostProc},/*DDConvert*/
	{{BRAP_CIT_P_ProcessingType_eWMAPassThrough},BRAP_CIT_P_AudioAlgoType_ePostProc},/*WmaProtoWma*/
	{{BRAP_CIT_P_ProcessingType_ePLll},BRAP_CIT_P_AudioAlgoType_ePostProc},/*PLII*/
	{{BRAP_CIT_P_ProcessingType_eSrsXt},BRAP_CIT_P_AudioAlgoType_ePostProc},/*SrsXt*/
	{{BRAP_CIT_P_ProcessingType_eAVL},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Avl*/
	{{BRAP_CIT_P_ProcessingType_eXen},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Xen*/
	{{BRAP_CIT_P_ProcessingType_eBbe},BRAP_CIT_P_AudioAlgoType_ePostProc},/*BBe*/
	{{BRAP_CIT_P_ProcessingType_eCustomSurround},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Custom surround*/
	{{BRAP_CIT_P_ProcessingType_eCustomBass},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Custom bass*/
	{{BRAP_CIT_P_ProcessingType_eCustomVoice},BRAP_CIT_P_AudioAlgoType_ePostProc},/*CostomVoice*/
	{{BRAP_CIT_P_ProcessingType_eAudioDescriptorPan},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Adpane*/
	{{BRAP_CIT_P_ProcessingType_eAudioDescriptorFade},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Adfade*/
	{{BRAP_CIT_P_ProcessingType_eSrc},BRAP_CIT_P_AudioAlgoType_ePostProc},	/*SRC*/
	{{BRAP_CIT_P_ProcessingType_ePCMRouter},BRAP_CIT_P_AudioAlgoType_ePostProc},	/*PCM Router*/		
	{{BRAP_CIT_P_ProcessingType_eDsola},BRAP_CIT_P_AudioAlgoType_ePostProc},	/*DSOLA*/		
	{{BRAP_CIT_P_ProcessingType_eSrsHd},BRAP_CIT_P_AudioAlgoType_ePostProc},/*SrsHd*/		
	{{BRAP_CIT_P_ProcessingType_eGenericPassThru},BRAP_CIT_P_AudioAlgoType_ePostProc},/*GenericPassthru*/			
	{{BRAP_CIT_P_ProcessingType_eSrsTruVolume},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Dolby Volume*/		
	{{BRAP_CIT_P_ProcessingType_eDolbyVolume},BRAP_CIT_P_AudioAlgoType_ePostProc},/*SrsTruVolume*/			
	{{BRAP_CIT_P_ProcessingType_eBrcm3DSurround},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Broadcom 3D surround*/				
    {{BRAP_CIT_P_ProcessingType_eMonoDownmix},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Mono Downmix*/
    {{BRAP_CIT_P_ProcessingType_eFWMixerPostProc},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Firmware Mixer*/
	{{BRAP_CIT_P_ProcessingType_eAudysseyVolume},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Audyssey Volume*/        
	{{BRAP_CIT_P_ProcessingType_eAudysseyABX},BRAP_CIT_P_AudioAlgoType_ePostProc},/*Audyssey ABX*/        	
	{{BRAP_CIT_P_ProcessingType_eSrsCsdTd},BRAP_CIT_P_AudioAlgoType_ePostProc},/* SRS CS Decoder and TruDialog */
	{{BRAP_CIT_P_ProcessingType_eSrsGeq},BRAP_CIT_P_AudioAlgoType_ePostProc},/* SRS Equalizer and Hard Limiter */
    {{BRAP_CIT_P_ProcessingType_eDv258},BRAP_CIT_P_AudioAlgoType_ePostProc},/*DolbyVolume258*/          	
    {{BRAP_CIT_P_ProcessingType_eDdre},BRAP_CIT_P_AudioAlgoType_ePostProc},/*DolbyVolume258*/          	        
    {{BRAP_CIT_P_ProcessingType_eBtsc},BRAP_CIT_P_AudioAlgoType_ePostProc},/*BTSC*/        
	{{BRAP_CIT_P_ProcessingType_eInvalid},BRAP_CIT_P_AudioAlgoType_eInvalid},	/*None*/		
	{{BRAP_CIT_P_ProcessingType_eInvalid},BRAP_CIT_P_AudioAlgoType_eInvalid},	/*Max*/	
};
/***************************************************************************
Summary:
    Insert internal stages for the Decode Channel.
    
Description:

    Insert internal stages for the Decode Channel.
    Currently not supported for Plaback/Capture Channel
    i.e. For Decode Channel bDecoderEnabled should be true.
    
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_InsertInternalStages(
	void	                *pHandle,           /* [in] Raptor Channel or Association handle */	
	bool                    bDecoderEnabled,    /* [in] If decoder is present */
	BRAP_DSPCHN_DecodeMode	eDecodeMode,         /* [in] Decode Mode */
	BRAP_DSPCHN_AudioType   eAudioType,
    bool                    bAssociationNw,	
	BRAP_ProcessingStageHandle **hAudioProcessingMatrix,
    BRAP_DestinationHandle      *hAudioNetworkDest,	
	unsigned int *puiStartBranchId,
	unsigned int *puiStartStageId);

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
BERR_Code BRAP_FWIF_P_CreateMsgQueue(BRAP_FWIF_MsgQueueParams *psMsgQueueParams ,/* [in]*/
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
    }
   else
    {
        hHandle = *hMsgQueue;
    }

	/* Conversion of address from virtual to physical*/	    
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
   if(false == bWdgRecovery)
    {        
	*hMsgQueue = hHandle;
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
BERR_Code BRAP_FWIF_P_DestroyMsgQueue(BRAP_FWIF_MsgQueueHandle   hMsgQueue, 
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
BERR_Code BRAP_FWIF_P_WriteMsg(BRAP_FWIF_MsgQueueHandle   hMsgQueue/*[in]*/,
									     void *pMsgBuf, /*[in]*/
									     unsigned int uiBufSize/*[in]*/)

{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BRAP_FWIF_P_WriteMsg);

	BKNI_EnterCriticalSection();
	rc = BRAP_FWIF_P_WriteMsg_isr(hMsgQueue, pMsgBuf,uiBufSize);
	BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_FWIF_P_WriteMsg);
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
BERR_Code BRAP_FWIF_P_WriteMsg_isr(BRAP_FWIF_MsgQueueHandle   hMsgQueue/*[in]*/,
									     void *pMsgBuf, /*[in]*/
									     unsigned int uiBufSize/*[in]*/)
{	
	BERR_Code err=BERR_SUCCESS;
	unsigned int i,uiFreeSpace=0;
	uint32_t ui32chunk1=0,ui32chunk2=0;
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0; 
	void *pvMsgQueueWriteAddr=NULL;
	
		
	BDBG_ENTER(BRAP_FWIF_P_WriteMsg_isr);

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
	
	/*This conversion is done to pass a virtual address as the second argument of BRAP_MemWrite32*/
	if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskWritePtr,(void **)&pvMsgQueueWriteAddr)))
		return BERR_TRACE(err);
	
	/*Writing data in two chunks taking wrap-around into consideration*/
    if((ui32maskWritePtr > ui32maskReadPtr)||(ui32maskWritePtr == ui32maskReadPtr))
    {
        if(uiBufSize > (hMsgQueue->uiEndAddr-ui32maskWritePtr))
        {
            ui32chunk1 = hMsgQueue->uiEndAddr-ui32maskWritePtr;
			ui32chunk2 = uiBufSize - ui32chunk1;
        }
        else
        {
            ui32chunk1 = uiBufSize;
			ui32chunk2 = 0;
        }
    }
    else
    {
        ui32chunk1 = uiBufSize;
		ui32chunk2 = 0;
    }
    
	/*Writing into chunk1*/
	for(i=0;i<(ui32chunk1/4);i++)
	{
	    BDBG_MSG(("*((uint32_t *)pMsgBuf+i) > %x", *((uint32_t *)pMsgBuf+i)));
		BRAP_MemWrite32_isr((uint32_t)pvMsgQueueWriteAddr+(i*4), *((uint32_t *)pMsgBuf+i));
		ui32dramWritePtr=ui32dramWritePtr+4;
	}

	/*Toggling the write pointer to wrap around*/
	if((ui32maskWritePtr + ui32chunk1) == hMsgQueue->uiEndAddr ) 
	{   
		ui32dramWritePtr = hMsgQueue->uiBaseAddr;
            ui32maskWritePtr = ui32dramWritePtr;
	}
	
	/*Writing into chunk 2*/
	if(ui32chunk2>0)
	{
		 if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskWritePtr,(void **)&pvMsgQueueWriteAddr)))
			return BERR_TRACE(err);
		for(i=0;i<(ui32chunk2/4);i++)
		{
            BDBG_MSG(("-->*((uint32_t *)pMsgBuf+i) > %x", *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i)));		
			BRAP_MemWrite32_isr((uint32_t)pvMsgQueueWriteAddr+(i*4), *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i));
			ui32dramWritePtr=ui32dramWritePtr+4;
		}
	}

	/*updating write ptr in the Queue Attribute Structure*/
	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset, ui32dramWritePtr); 
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
	
	/*updating write ptr in the handle*/
	hMsgQueue->uiWritePtr=ui32dramWritePtr;
	BDBG_LEAVE(BRAP_FWIF_P_WriteMsg_isr);
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

BERR_Code BRAP_FWIF_P_GetMsg(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 void *pMsgBuf,/*[in]*/
									 BRAP_P_MsgType eMgsType)
   
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BRAP_FWIF_P_GetMsg);

	BKNI_EnterCriticalSection();
	rc = BRAP_FWIF_P_GetMsg_isr(hMsgQueue, pMsgBuf,eMgsType);
	BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_FWIF_P_GetMsg);
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
BERR_Code BRAP_FWIF_P_GetMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 void *pMsgBuf,/*[in]*/
									 BRAP_P_MsgType eMgsType)
   
{
	BERR_Code err=BERR_SUCCESS;
	
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0;
       uint32_t ui32chunk1=0,ui32chunk2=0,i;
	int32_t i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
	void *pvMsgQueueReadAddr=NULL;
	
	BDBG_ENTER(BRAP_FWIF_P_GetMsg_isr);
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
    if(BRAP_P_MsgType_eSyn == eMgsType)
    {
        ui32ResponseSize = BRAP_FWIF_P_RESPONSE_SIZE_IN_BYTES;
    }
    else if(BRAP_P_MsgType_eAsyn == eMgsType)
    {
        ui32ResponseSize = BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

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
        i32BytesToBeRead=hMsgQueue->uiEndAddr-hMsgQueue->uiBaseAddr;
    }
	 if(i32BytesToBeRead < 0)
	 {
		BDBG_ERR(("The Message Queue is empty.No message is present."));
		BDBG_ASSERT(0);        
		return BERR_TRACE(BRAP_ERR_BUFFER_EMPTY);
	 }
	 
	
	/*Reading Message from the message queue into the message buffer*/
	if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(err);
	
	/*Reading data in two chunks taking wrap-around into consideration*/
	if((ui32maskReadPtr > ui32maskWritePtr)||(ui32maskReadPtr == ui32maskWritePtr))
	{
		if(ui32ResponseSize> (hMsgQueue->uiEndAddr-ui32maskReadPtr))
		{
			ui32chunk1=hMsgQueue->uiEndAddr-ui32maskReadPtr;
			ui32chunk2=i32BytesToBeRead-ui32chunk1;
		}
		else
		{
			ui32chunk1=ui32ResponseSize;
			ui32chunk2=0;	
		}
	}
	else
	{
		ui32chunk1=ui32ResponseSize;
		ui32chunk2=0;
	}
		

	/*Reading from chunk1 */
	for(i=0;i<(ui32chunk1/4);i++)
	{
		*((uint32_t *)pMsgBuf+i)=BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr+(i*4));
		ui32dramReadPtr=ui32dramReadPtr+4;
			
	}

    /*Toggling the read pointer */
	if(ui32maskReadPtr + ui32chunk1 == hMsgQueue->uiEndAddr)
	{
		ui32dramReadPtr=hMsgQueue->uiBaseAddr;
		ui32maskReadPtr=ui32dramReadPtr;
	}
	
		
	/*Reading from chunk2*/
	if(ui32chunk2>0)
	{
		if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
			return BERR_TRACE(err);
		
		for(i=0;i<(ui32chunk2/4);i++)
		{
			*((uint32_t *)pMsgBuf+i)=BRAP_MemRead32_isr((uint32_t) pvMsgQueueReadAddr+(i*4));
			ui32dramReadPtr=ui32dramReadPtr+4;
		}
	}
	

	/*updating read ptr in the Queue Attribute Structure*/
	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, ui32dramReadPtr); 
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
		
	/*updating read ptr in the handle*/
	hMsgQueue->uiReadPtr = ui32dramReadPtr;
	
	BDBG_LEAVE(BRAP_FWIF_P_GetMsg_isr);
	return err;
 
}

/***************************************************************************
Summary:
        Send command to firmware
        
Description:
        This function sends command to firmware.

Returns:
    BERR_SUCCESS else error

**************************************************************************/


/***************************************************************************
Summary:
        process command to firmware
        
Description:
        This function 
            - sends command to firmware.
            - Wait For Ack
            - Check the response

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_SendCommand( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
                        								    BRAP_FWIF_P_Command *psCommand/*[in]*/ ,
                        								        BRAP_Handle hRap ,
                                                                                BRAP_FWIF_P_FwTaskHandle     hTask   /* [in] Task handle */)
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BRAP_FWIF_P_SendCommand);

	BKNI_EnterCriticalSection();
	rc = BRAP_FWIF_P_SendCommand_isr(hMsgQueue, psCommand,hRap,hTask);
	BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_FWIF_P_SendCommand);
    return rc;
}

/***************************************************************************
Summary:
        Send command to firmware
        
Description:
        This function sends command to firmware. This is isr version of send 
        command        

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_SendCommand_isr( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
                        								    BRAP_FWIF_P_Command *psCommand/*[in]*/,
                        								        BRAP_Handle hRap,
                        								        BRAP_FWIF_P_FwTaskHandle     hTask   /* [in] Task handle */)
{
	BERR_Code err = BERR_SUCCESS;
       unsigned int uiCommandSize;
	BDBG_ENTER( BRAP_FWIF_P_SendCommand_isr );

	BDBG_ASSERT( hMsgQueue );
	BDBG_ASSERT( psCommand );
    	BDBG_ASSERT( hRap);

    if(hRap->bWatchdogTriggered == true)
    {
        BDBG_MSG(("Watchdog recovery is already trigerred, So No message will be written in DRAM. exit gracefully"));
        return BERR_SUCCESS;
    }
    if(psCommand->sCommandHeader.ui32CommandID != BRAP_FWIF_P_PING_COMMAND_ID)
    {
        BDBG_ASSERT(hTask);
        if((hTask->bStopped == true)/*When bStopped is true at that instance STOP/START commands can come and should be processed */
            &&(psCommand->sCommandHeader.ui32CommandID != BRAP_FWIF_P_START_TASK_COMMAND_ID))
        {
            BDBG_MSG(("Task is in stop state, Can't accept Command %#x",psCommand->sCommandHeader.ui32CommandID ));
            return BERR_SUCCESS;        
        }
    }
    else
    {
        BSTD_UNUSED(hTask);
    }

    psCommand->sCommandHeader.ui32CommandSizeInBytes = sizeof(BRAP_FWIF_P_Command);
    uiCommandSize = sizeof(BRAP_FWIF_P_Command);
    BDBG_MSG(("psCommand->sCommandHeader.ui32CommandSizeInBytes > %d", psCommand->sCommandHeader.ui32CommandSizeInBytes));
	err=BRAP_FWIF_P_WriteMsg_isr( hMsgQueue,(void *) psCommand,uiCommandSize);
	if(BERR_SUCCESS != err)
	{
		return BERR_TRACE(err);
	}

        if(psCommand->sCommandHeader.ui32CommandID == BRAP_FWIF_P_STOP_TASK_COMMAND_ID)
        {
                hTask->bStopped = true;
        }
	BDBG_LEAVE( BRAP_FWIF_P_SendCommand_isr );
	return err;
	
}

/***************************************************************************
Summary:
        Get response from firmware for a given command 
    
Description:
        This function receives response from firmware for an issued command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetResponse( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
        								BRAP_FWIF_P_Response    *psResponse,/*[in]*/ 
        								BRAP_P_MsgType          eMsgType)
{
	BERR_Code err = BERR_SUCCESS;
    
	BDBG_ENTER( BRAP_FWIF_P_GetResponse );

	BDBG_ASSERT( hMsgQueue );
	BDBG_ASSERT( psResponse );

    eMsgType = BRAP_P_MsgType_eSyn;
	err=BRAP_FWIF_P_GetMsg(hMsgQueue, (void *)psResponse, eMsgType);
	if(BERR_SUCCESS != err)
	{
		return BERR_TRACE(err);
	}

    /* Assume that calling function knows that a message in the queue is a 
    response message.Asserting if it is an acknowledgement*/ 
    if((psResponse->sCommonAckResponseHeader.ui32ResponseID == BRAP_FWIF_P_INVALID_COMMAND) || 
       (psResponse->sCommonAckResponseHeader.ui32ResponseID > BRAP_FWIF_P_RESPONSE_ID_OFFSET)) 
	{
        BDBG_ASSERT(0);	
		BDBG_ERR(("BRAP_FWIF_P_GetResponse: GetResponse() hasn't encountered an acknowledgement message!"));
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
	
 	BDBG_LEAVE( BRAP_FWIF_P_GetResponse );
 	return err;
}


/***************************************************************************
Summary:
        Get acknowledgement from firmware for a given command 
    
Description:
        This function receives acknowledgement from firmware for an issued command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAck( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
                       						  BRAP_FWIF_P_Response    *psAck /*[in]*/)
{
	BERR_Code err = BERR_SUCCESS;
    
	BDBG_ENTER( BRAP_FWIF_P_GetAck );
	BDBG_ASSERT( hMsgQueue );
	BDBG_ASSERT( psAck);

	err=BRAP_FWIF_P_GetMsg(hMsgQueue,(void *)psAck, BRAP_P_MsgType_eSyn);
	if(BERR_SUCCESS != err)
	{
		return BERR_TRACE(err);
	}
	
	 /* Assume that calling function knows that a message in the queue is an 
	 acknowledgement.Asserting if it is a response*/ 
	if(!((psAck->sCommonAckResponseHeader.ui32ResponseID >= (BRAP_FWIF_P_ACK_ID_OFFSET + 1)) && 
        (psAck->sCommonAckResponseHeader.ui32ResponseID <= (BRAP_FWIF_P_RESPONSE_ID_OFFSET + 1))))
	{
        BDBG_ASSERT(0);	
		BDBG_ERR(("BRAP_FWIF_P_GetAck: GetAck() hasn't encountered an ack message"));
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	BDBG_LEAVE( BRAP_FWIF_P_GetAck );
	return err;
}

/***************************************************************************
Summary:
        Get response from firmware for a given asynchronous command 
    
Description:
        This function receives response from firmware for an issued asynchronous 
        command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAsynResponse( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
                                        BRAP_FWIF_P_AsynEventMsg   *psAsynResponse /*[in]*/)
{
	BERR_Code err = BERR_SUCCESS;
    
	BDBG_ENTER( BRAP_FWIF_P_GetAsynResponse );
	BDBG_ASSERT( hMsgQueue );
	BDBG_ASSERT( psAsynResponse);


	err=BRAP_FWIF_P_GetMsg_isr(hMsgQueue,(void *)psAsynResponse, BRAP_P_MsgType_eAsyn);
	if(BERR_SUCCESS != err)
	{
		return BERR_TRACE(err);
	}

	 /* Assume that calling function knows that a message in the queue is an acknowledgement.
	    Asserting if it not having evetn id for asynchronous command */ 
	    BDBG_MSG(("psAsynResponse->sMsgHeader.ui32EventID  =%d",psAsynResponse->sMsgHeader.ui32EventID));
     	    BDBG_MSG(("psAsynResponse->sMsgHeader.ui32TaskID  =%d",psAsynResponse->sMsgHeader.ui32TaskID));
    if((psAsynResponse->sMsgHeader.ui32EventID <= BRAP_FWIF_P_ASYNC_EVENT_ID_OFFSET)|| 
       (psAsynResponse->sMsgHeader.ui32EventID > BRAP_FWIF_P_LAST_EVENT_ID)) 
	{
        BDBG_ASSERT(0);
		BDBG_ERR(("BRAP_FWIF_P_GetAsynResponse: hasn't encountered an async event msg"));
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);

	}
	BDBG_LEAVE( BRAP_FWIF_P_GetAsynResponse );
	return err;
}


/***************************************************************************
Summary:
        Flushes the queue
    
Description:
        This function resets all the pointers if a queue is created afresh
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_FlushMsgQueue( BRAP_FWIF_MsgQueueHandle  hMsgQueue)
{
	BERR_Code err = BERR_SUCCESS;
	BDBG_ENTER( BRAP_FWIF_P_FlushMsgQueue);
	BDBG_ASSERT(hMsgQueue);
	
	hMsgQueue->uiReadPtr=hMsgQueue->uiBaseAddr;
	hMsgQueue->uiWritePtr=hMsgQueue->uiBaseAddr;

	BRAP_Write32(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, hMsgQueue->uiReadPtr); 
	BRAP_Write32(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset, hMsgQueue->uiWritePtr); 
	
	BDBG_LEAVE( BRAP_FWIF_P_FlushMsgQueue);
	return err;
}

/***************************************************************************
Summary:
    Destroy all the internal stages for the Channel.
    
Description:

    Destroy all the internal stages for the Channel.
    
Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_FWIF_P_DestroyInternalStages(	BRAP_ChannelHandle      hRapCh/* [in] RAP Channel handle */)
{
    BERR_Code                       err = BERR_SUCCESS;
    unsigned int                             i=0, j=0,k =0;

    BDBG_ASSERT(hRapCh);
    BDBG_ENTER(BRAP_FWIF_P_DestroyInternalStages);
    
    for(i = 0 ; i < BRAP_P_MAX_DST_PER_RAPCH; i++)
    {
        for(j =0; j < BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED; j++)
        {
            if(BRAP_P_IsPointerValid((void *)hRapCh->sChAudProcessingStage[i][j].hAudioProcessing))
            {
                if(true == hRapCh->sChAudProcessingStage[i][j].bInternalProcessingStage)
                {
                    for(k =0; k< BRAP_P_MAX_DEST_PER_PROCESSING_STAGE; k++)
                    {
                        hRapCh->sChAudProcessingStage[i][j].hAudioProcessing->hDestHandle[k] =NULL;
                    }
                    if(true == hRapCh->sChAudProcessingStage[i][j].hAudioProcessing->bChSpecificStage)
                    {
                        hRapCh->sChAudProcessingStage[i][j].hAudioProcessing->uHandle.hRapCh = NULL;
                    }
                    else
                    {
                        hRapCh->sChAudProcessingStage[i][j].hAudioProcessing->uHandle.hAssociation = NULL;
                    }
                    err = BRAP_DestroyProcessingStage(hRapCh->sChAudProcessingStage[i][j].hAudioProcessing);
                    if(err != BERR_SUCCESS) 
                    {
                        BDBG_ERR(("BRAP_FWIF_P_DestroyInternalStages: Error in destroying Internal processing stage %d",
                            hRapCh->sChAudProcessingStage[i][j].hAudioProcessing->sProcessingStageSettings.eAudioProcessing));
                        err =  BERR_TRACE(err);
                        goto end;
                    }                
                    hRapCh->sChAudProcessingStage[i][j].hAudioProcessing = NULL;
                }
            }
        }
    }

end:
    BDBG_LEAVE(BRAP_FWIF_P_DestroyInternalStages);    
    return err;
}

/***************************************************************************
Summary:
    It will tell if the Branch is the clone of already existing branch. i.e. complete branch is cloned
       
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_IsBranchCloned(
    BRAP_ChannelHandle hRapCh,
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    unsigned int iBranchId,
    unsigned int iLastStageId,
    bool *bCloned,
    bool *bIsFwBranch   /* If there is no, PP in the branch Fw doesn't treat it as a branch*/)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int                       ui32MasterBranchId =0,ui32MasterStageId =0;
    unsigned int k =0, l =0, m=0, uiCloneBranchId[BRAP_P_MAX_DST_PER_RAPCH];
    BRAP_OutputMode            eChannelOutputMode;

    BDBG_ENTER(BRAP_FWIF_P_IsBranchCloned);
    BDBG_ASSERT(psAudProcNetwork);

    if(hRapCh == NULL)
    {
        eChannelOutputMode = BRAP_OutputMode_e2_0;
    }
    else
    {
        eChannelOutputMode = hRapCh->eChannelOutputMode;
    }

    if(false == psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].bCloneStage)
    {
        *bCloned = false;
        *bIsFwBranch = true;
        return ret;
    }
    
    ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].ui32MasterBranchId;
    ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].ui32MasterStageId;

    if(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1))
        {
            for(k=0; k<BRAP_P_MAX_DST_PER_RAPCH; k++)
            {
                uiCloneBranchId[k] = BRAP_INVALID_VALUE;
            }

            k = ui32MasterBranchId;
            l = ui32MasterStageId;
            m = 0;

            /* Now check if the branches between Master branch and current branch is similar to current  branch.
            This is done by checking if the last stage is cloned, and its Master stage/branch id 
            is same as that of last stage id of current branch. Just to make clear, here 
            Current Branch/Stage Id is : i/uiLastStage
            Master Branch/Stage Id is : ui32MasterBranchId/ui32MasterStageId
            And we looping on K, which is between ui32MasterBranchId and i*/   
            
            for(; k<iBranchId; ++k)
            {   
                if(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1)))
                {
                    if((true == psAudProcNetwork->sAudProcessingStage[k][l].bCloneStage)
                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterBranchId == ui32MasterBranchId)
                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterStageId == ui32MasterStageId)
                        &&(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1))))
                    {
                        uiCloneBranchId[m]=k;
                        m++;
                    }
                } 
            }
            for(m=0; (m<BRAP_P_MAX_DST_PER_RAPCH)   ; ++m)
            {
                if(uiCloneBranchId[m] == BRAP_INVALID_VALUE)
                    break;
                
                k = uiCloneBranchId[m];
                /* For cloned branch */
                if((psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                    && (psAudProcNetwork->sAudProcessingStage[k][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort))
                {
                if((eChannelOutputMode == BRAP_OutputMode_e2_0)
                    ||(psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                    psAudProcNetwork->sAudProcessingStage[k][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort))
                {
                    *bCloned = true;
                            break;
                }
                else if (eChannelOutputMode > BRAP_OutputMode_e2_0)
                {
                    *bCloned = false;
                    }
                    else
                    {
                        *bCloned = true;                    
                    }
                }
                else
                {
                    *bCloned = true;                    
                }
            }            

            if(*bCloned == false)
            {
                if((iBranchId > 0))/* If it is not the first Branch and since its last stage is cloned , so its not FW branch */
                {
                    *bIsFwBranch = false;
                }
                else
                {
                    *bIsFwBranch = true;
                }
            }
            else
            {
                    *bIsFwBranch = false;
            }
        }
        else
        {

            for(k=0; k<BRAP_P_MAX_DST_PER_RAPCH; k++)
            {
                uiCloneBranchId[k] = BRAP_INVALID_VALUE;
            }

            k = ui32MasterBranchId;
            l = ui32MasterStageId;
            m = 0;
            uiCloneBranchId[m]=k;
            m++;
            k++;

            /* Now check if the branches between Master branch and current branch is similar to current  branch.
            This is done by checking if the last stage is cloned, and its Master stage/branch id 
            is same as that of last stage id of current branch. Just to make clear, here 
            Current Branch/Stage Id is : i/uiLastStage
            Master Branch/Stage Id is : ui32MasterBranchId/ui32MasterStageId
            And we looping on K, which is between ui32MasterBranchId and i*/   
            
            for(; k<iBranchId; ++k)
            {   
                if(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1)))
                {
                    if((true == psAudProcNetwork->sAudProcessingStage[k][l].bCloneStage)
                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterBranchId == ui32MasterBranchId)
                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterStageId == ui32MasterStageId)
                        &&(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1))))
                    {
                        uiCloneBranchId[m]=k;
                        m++;
                    }
                } 
            }
            for(m=0; (m<BRAP_P_MAX_DST_PER_RAPCH)   ; ++m)
            {
                if(uiCloneBranchId[m] == BRAP_INVALID_VALUE)
                    break;
                
                k = uiCloneBranchId[m];
                if((psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                    && (psAudProcNetwork->sAudProcessingStage[k][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort))
                {
                /* For cloned branch */
                if((eChannelOutputMode == BRAP_OutputMode_e2_0)
                    ||(psAudProcNetwork->sAudProcessingStage[iBranchId][iLastStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                    psAudProcNetwork->sAudProcessingStage[k][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort))
                {
                    *bCloned = true;
                            break;
                }
                else if (eChannelOutputMode > BRAP_OutputMode_e2_0)
                {
                    *bCloned = false;
                    }
                    else
                    {
                        *bCloned = true;                    
                    }                
                }
                else
                {
                    *bCloned = true;                    
                }                
            }            

            if(*bCloned == false)
            {
                if((iBranchId > 0)
                &&(iLastStageId == 0))/* If the Branch (except the first) don't have PP then its not a FW branch. Also Downmix branch with no PP is not a FW branch*/
                {
                    *bIsFwBranch = false;
                }
                else
                {
                    *bIsFwBranch = true;
                }
            }
            else
            {
                    *bIsFwBranch = false;
            }
        }
        BDBG_LEAVE(BRAP_FWIF_P_IsBranchCloned);
    return ret;
}

#ifdef RAP_SRC_SUPPORT                           

/***************************************************************************
Summary:
    It will tell if SRC Stage needs to be insterted internally or not.
    Currently it should be insertinted in following cases
    - If algo type is MPEG
    - If DTS Encode is present in network.

    SRC stage should not be insterted If Decoder stage is outputting Compressed data like 
    DDConvert and WMAPassthrough
       
Returns:
    BERR_SUCCESS else error

**************************************************************************/
static void BRAP_FWIF_P_IsSRCRequired(
	BRAP_ChannelHandle      hRapCh,		        /* [in] RAP Channel handle */
	BRAP_ProcessingStageHandle **hAudioProcessingMatrix,
	BRAP_DSPCHN_AudioType eAudioType,
	bool    *bSRCRequired
	)
{
    bool bCompressedBranch = false, bDtsEncodePresent = false, bAc3EncodePresent = false;
    unsigned int i =0 , j= 0;

    BDBG_ENTER(BRAP_FWIF_P_IsSRCRequired);
    BDBG_ASSERT(hRapCh);

    *bSRCRequired = false;
    for(i = 0 ; i < BRAP_P_MAX_DST_PER_RAPCH ; i++)
    {
        for(j =0 ; j < BRAP_MAX_PP_PER_BRANCH_SUPPORTED ; j++)
        {
            if(!(BRAP_P_IsPointerValid((void *)hAudioProcessingMatrix[i][j])))
                break;
            if(hAudioProcessingMatrix[i][j] ->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eEncodeDts)
            {
                bDtsEncodePresent = true;
            }
            else if(hAudioProcessingMatrix[i][j] ->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eEncodeAc3)
            {
                bAc3EncodePresent = true;
            }            
            else if((hAudioProcessingMatrix[i][j] ->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eConvertDdpToAc3)
                        ||(hAudioProcessingMatrix[i][j] ->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eWmaProPassThru)
                        )
            {
                bCompressedBranch = true;
            }
        }
    }

    if(bCompressedBranch == true)
    {
        *bSRCRequired = false;
    }
    else if( 
        (eAudioType == BRAP_DSPCHN_AudioType_eMpeg)
        /*SRC should not be added for Dolby pulse, even if it have DTS/Ac3 transcode.*/
#ifndef RAP_MULTISTREAM_DECODER_SUPPORT                     
             ||(bDtsEncodePresent == true)
             ||(bAc3EncodePresent == true)    
             ||(eAudioType == BRAP_DSPCHN_AudioType_eAac)
             ||(eAudioType == BRAP_DSPCHN_AudioType_eAacLoas)
             ||(eAudioType == BRAP_DSPCHN_AudioType_eAacSbr)
             ||(eAudioType == BRAP_DSPCHN_AudioType_eAacSbrAdts)
#endif             
             ||(eAudioType == BRAP_DSPCHN_AudioType_eWmaStd)
             ||(eAudioType == BRAP_DSPCHN_AudioType_eAmr)             
             ||(eAudioType == BRAP_DSPCHN_AudioType_eDra)             
             ||(eAudioType == BRAP_DSPCHN_AudioType_eRealAudioLbr)                          
             ||(eAudioType == BRAP_DSPCHN_AudioType_ePcmWav)             
            )
    {
        *bSRCRequired = true;                
    }
    else if((hRapCh->eAudioSource == BRAP_AudioSource_eExtCapPort) && 
            (hRapCh->eCapInputPort == BRAP_CapInputPort_eHdmi)
            && (eAudioType == BRAP_DSPCHN_AudioType_ePcm))
    {
        *bSRCRequired = true;    
    }

    BDBG_LEAVE(BRAP_FWIF_P_IsSRCRequired);
    return;
}
#endif

/***************************************************************************
Summary:
    Insert internal stages for the Decode Channel.
    
Description:

    Insert internal stages for the Decode Channel.
    Currently not supported for Plaback/Capture Channel
    i.e. For Decode Channel bDecoderEnabled should be true.
    
Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_FWIF_P_InsertInternalStages(
	void	                *pHandle,           /* [in] Raptor Channel or Association handle */	
	bool                    bDecoderEnabled,    /* [in] If decoder is present */
	BRAP_DSPCHN_DecodeMode	eDecodeMode,         /* [in] Decode Mode */
	BRAP_DSPCHN_AudioType   eAudioType,
    bool                    bAssociationNw,	
	BRAP_ProcessingStageHandle **hAudioProcessingMatrix,
    BRAP_DestinationHandle      *hAudioNetworkDest,
	unsigned int *puiStartBranchId,
	unsigned int *puiStartStageId)
{
    BERR_Code                       err = BERR_SUCCESS;
    unsigned int                    uiStageId =0,uiBranchId=0;
    unsigned int                             i=0, j=0,k=0;
    BRAP_ProcessingStageHandle	 hTempProcessingHandle;
    BRAP_ProcessingStageSettings	 *psProcessingStageSettings;  
    BRAP_OutputPort eOp = BRAP_OutputPort_eMax;
    bool bCompressedOutput = false;
    bool    bSRCRequired = false;

    BDBG_ENTER(BRAP_FWIF_P_InsertInternalStages);

    BDBG_ASSERT(pHandle);

    BSTD_UNUSED(hTempProcessingHandle);
    BSTD_UNUSED(psProcessingStageSettings);
#ifndef RAP_SRC_SUPPORT    
    BSTD_UNUSED(eAudioType);
    BSTD_UNUSED(bSRCRequired);
#endif

    psProcessingStageSettings = (BRAP_ProcessingStageSettings	 *)BKNI_Malloc(sizeof(BRAP_ProcessingStageSettings));
    if(psProcessingStageSettings == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(psProcessingStageSettings,0,sizeof(BRAP_ProcessingStageSettings));

    if(bAssociationNw == false)
    {
    	BRAP_ChannelHandle      hRapCh;
        
        hRapCh = *(BRAP_ChannelHandle *)pHandle;    
        BDBG_ASSERT(hRapCh);
        /* Find out any compressed outputs */
        bCompressedOutput = false;
        for(i=0; i < BRAP_P_MAX_DST_PER_RAPCH; i++)
        {
            if(!(BRAP_P_IsPointerValid((void *)hRapCh->pDstDetails[i])))
                continue;
            if(BRAP_AudioDst_eOutputPort == hRapCh->pDstDetails[i]->eAudioDst)
            {
                eOp = hRapCh->pDstDetails[i]->uDstDetails.sOpDetails.eOutput[0];
                if((BRAP_OutputPort_eSpdif == eOp)||
                    (BRAP_OutputPort_eMai == eOp))
                {
                    for(j=0;j<BRAP_RM_P_MAX_OUTPUTS;j++)
                    {
                        if(eOp == hRapCh->hRap->sOutputSettings[j].eOutputPort)
                        {
                            if(true == hRapCh->hRap->sOutputSettings[j].bCompressed)
                            {
                                bCompressedOutput = true;
                                break;
                            }
                        }
                    }
                }
            }
            if(true == bCompressedOutput)
                break;
        }
        
        if(bDecoderEnabled == true)
        {
            if (BRAP_DSPCHN_DecodeMode_eDecode == eDecodeMode)
            {
                hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].bDecoderStage = true;
                
                /* Find the branch that doesn't have GenericPassthru PP, since Auxilary data out
                required for this PP is not supported through SRC and DSOLA stages */    
                for(i = uiBranchId; i < BRAP_P_MAX_DST_PER_RAPCH; i++)
                {
                    if(hAudioNetworkDest[i] == NULL)
                    continue;
                    
                    for(k = 0; k <= BRAP_P_MAX_RAPCH_PER_DST; k++)
                    {
                        if((hAudioNetworkDest[i]->psProcessingSettings[k] != NULL) &&
                           (hAudioNetworkDest[i]->psProcessingSettings[k]->hRapCh == hRapCh))
                        break;
                    }
                    if(k > BRAP_P_MAX_RAPCH_PER_DST)
                        break;

                        if((hAudioNetworkDest[i]->psProcessingSettings[k]->sExternalSettings.hAudProcessing[0] != NULL) &&
                           (hAudioNetworkDest[i]->psProcessingSettings[k]->sExternalSettings.hAudProcessing[0]->
                        sProcessingStageSettings.eAudioProcessing != BRAP_ProcessingType_eGenericPassthru))
                    {
                        break;
                    }
                }
                if(i < BRAP_P_MAX_DST_PER_RAPCH)
                    uiBranchId = i; 

                    uiStageId++;
            }
            else if (BRAP_DSPCHN_DecodeMode_ePassThru == eDecodeMode)
            {
                hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].bDecoderStage = true;              
                uiBranchId++;
                uiStageId++;                
            }    

    /*ASSUMPTION : Internal processing stages can be added only to decode mode And only if its Decode Channel, i.e. bDecoderEnabled =true
    In the future, if internal post processing is applicable on Playback/Capture also, then need to modify this function accordingly*/
            if(BRAP_DSPCHN_DecodeMode_eDecode == eDecodeMode)
            {
#ifdef RAP_SRC_SUPPORT                           
                /* Add SRC PostProcessing */
                BRAP_FWIF_P_IsSRCRequired(hRapCh,hAudioProcessingMatrix,eAudioType,&bSRCRequired);

                if(true == bSRCRequired)
                {
                    /* Check if SRC has been supported during compile */
                    if(false == BRAP_FWDWNLD_P_IsAudProcSupported(BRAP_sMapProcessingEnum[BRAP_ProcessingType_eSRC].uEnumName.eProcessingType))
                    {
                        BDBG_ERR(("Please export RAP_SRC_SUPPORT=y at the compile time"));
                        err =  BERR_TRACE(BERR_NOT_SUPPORTED);
                        goto end;
                    }
                
                    err = BRAP_GetDefaultProcessingStageSettings( BRAP_ProcessingType_eSRC,psProcessingStageSettings );
                    if(err != BERR_SUCCESS)
                    {
                        BDBG_ERR(("GetDefaultProcessingStageSettings for eSRC failed"));
                        err = BERR_TRACE(err);
                        goto end;
                    }
                    err = BRAP_CreateProcessingStage(hRapCh->hRap ,psProcessingStageSettings,&hTempProcessingHandle);
                    if(err != BERR_SUCCESS)
                    {
                        BDBG_ERR(("Creation of the SRC stage failed"));
                        err = BERR_TRACE(err);
                        goto end;
                    }
                    hTempProcessingHandle->bChSpecificStage = true;                                    
                    hTempProcessingHandle->uHandle.hRapCh = hRapCh;
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].hAudioProcessing=hTempProcessingHandle;
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].bInternalProcessingStage = true;
                    
                    uiStageId++;
                }
#endif                  
              
#ifdef RAP_DSOLA_SUPPORT            
                if(BRAP_DSPCHN_PLAYBACKRATE_NORMAL != hRapCh->uiPBRate)
                {
                    if(true == bCompressedOutput)
                    {
                        BDBG_ERR(("DSOLA & Compressed Processing like Transcode , WMA PAssthru and/or DD Convert can't be enabled together!"));
                        hRapCh->uiPBRate = BRAP_DSPCHN_PLAYBACKRATE_NORMAL;
                    }
                    else
                    {
                        err = BRAP_GetDefaultProcessingStageSettings( BRAP_ProcessingType_eDSOLA,psProcessingStageSettings );
                        if(err != BERR_SUCCESS)
                        {
                            BDBG_ERR(("GetDefaultProcessingStageSettings for DSOLA failed"));
                            err = BERR_TRACE(err);
                            goto end;
                        }
                        err = BRAP_CreateProcessingStage(hRapCh->hRap ,psProcessingStageSettings,&hTempProcessingHandle);
                        if(err != BERR_SUCCESS)
                        {
                            BDBG_ERR(("Creation of the DSOLA stage failed"));
                            err = BERR_TRACE(err);
                            goto end;
                        }
                        hTempProcessingHandle->bChSpecificStage = true;                                    
                        hTempProcessingHandle->uHandle.hRapCh = hRapCh;
                        hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].hAudioProcessing=hTempProcessingHandle;
                        hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].bInternalProcessingStage = true;
                        
                        uiStageId++;
                    }
                }
#endif            
            }
        }

    /* if branch has only decode Stage (+ any internally added processing algorithm) then it should be linked to some destiantion handle*/
        if((uiStageId > 0) ||(uiBranchId > 0))
        {
            k = 0;
            for(i=0; i<BRAP_P_MAX_DST_PER_RAPCH ;i++)
            {
                    if((NULL != hAudioNetworkDest[i])
                    &&  (k <BRAP_P_MAX_DEST_PER_PROCESSING_STAGE)
                    && (NULL == hAudioProcessingMatrix[i][0])) /*Only those destination is added to 
                    deocder stage which is directly getting data from decoder stage, not through any other PP Statge */
                {
                    if((BRAP_ChannelSubType_ePrimary==hRapCh->eChannelSubType)
                        ||(BRAP_ChannelSubType_eNone==hRapCh->eChannelSubType))
                    {                
                        for(j = 0 ; j < BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP ; j++)
                        {   
#if (BRAP_3548_FAMILY == 1)
                            if (BRAP_ChannelType_ePcmCapture == hRapCh->eChannelType)
                            {   
                                if(hAudioNetworkDest[i]->hAssociation->hCapCh[j] == hRapCh)
                                {
                                    switch (eDecodeMode)
                                    {
                                        case BRAP_DSPCHN_DecodeMode_eDecode:
                                            if(NULL == hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing)
                                            {
                                                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                            }
                                            else
                                            {
                                                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing->hDestHandle[k] = hAudioNetworkDest[i];                                      
                                            }
                                            
                                            k++;
                                            break;
                                        case BRAP_DSPCHN_DecodeMode_ePassThru:
                                                    hRapCh->sChAudProcessingStage[uiBranchId-1][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                            k++;
                                            break;
                                        default:
                                            BDBG_ASSERT(0);
                                            err = BERR_TRACE(BERR_INVALID_PARAMETER);
                                            goto end;                                    
                                    }
                                    break;
                                }                        
                            }
                            else
#endif                            
                            {
                                if(hAudioNetworkDest[i]->hAssociation->hPriDecCh[j] == hRapCh)
                                {
                                    if (eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
                                    {
                                        if(NULL == hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing)
                                        {
                                                hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                        }
                                        else
                                        {
                                                hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing->hDestHandle[k] = hAudioNetworkDest[i];                                      
                                        }
                                        
                                        k++;
                                    }
                                    else if (eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
                                    {
                                        hRapCh->sChAudProcessingStage[uiBranchId-1][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                        k++;
                                    }                                
                                    break;
                                }
                            }
                        }	
                    }
                    else if (BRAP_ChannelSubType_eSecondary==hRapCh->eChannelSubType)
                    {
                        for(j = 0 ; j < BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP ; j++)
                        {
                            if(hAudioNetworkDest[i]->hAssociation->hSecDecCh[j] == hRapCh)
                            {
                                if (eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
                                {
                                    if(NULL == hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing)
                                    {
                                            hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                    }
                                    else
                                    {
                                            hRapCh->sChAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing->hDestHandle[k] = hAudioNetworkDest[i];                                      
                                    }
                                    k++;
                                }
                                else if (eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
                                {                            
                                    hRapCh->sChAudProcessingStage[uiBranchId-1][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                                    k++;                             
                                }
                                break;						
                            }
                        }
                    }
                }
            }
        }
        
        for(i = 0; i< BRAP_P_MAX_DST_PER_RAPCH; i++)
        {
            if(NULL == hAudioNetworkDest[i])
            {
                break;
            }

            for(j = 0; j< uiStageId; j++)
            {
                if(i >0)
                {
                    /* Avoid cloning of SRC and DSOLA stages to the branch that has GenericPassthru PP */
                    if(j > 0)
                    {
                        for(k=0; k<=BRAP_P_MAX_RAPCH_PER_DST; k++)
                        {
                            if((hAudioNetworkDest[i]->psProcessingSettings[k] != NULL) &&
                               (hAudioNetworkDest[i]->psProcessingSettings[k]->hRapCh == hRapCh))
                            break;
                        }
                        if((k <= BRAP_P_MAX_RAPCH_PER_DST) && 
                               (hAudioNetworkDest[i]->psProcessingSettings[k]->sExternalSettings.hAudProcessing[0] != NULL) &&
                               (hAudioNetworkDest[i]->psProcessingSettings[k]->sExternalSettings.hAudProcessing[0]->
                            sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eGenericPassthru))
                        {                   
                            break;
                        }

                    }        

                    /* For uiBranchId clone only Decoder stage */
                    if(((i == uiBranchId) && (j == 0)) ||
                       (i != uiBranchId))
                    {
                        BDBG_MSG(("bCloneStage = true for i = %d and j = %d",i,j));
                        hRapCh->sChAudProcessingStage[i][j].bCloneStage = true;

                        if(j == 0)
                            hRapCh->sChAudProcessingStage[i][j].ui32MasterBranchId = 0;
                        else
                            hRapCh->sChAudProcessingStage[i][j].ui32MasterBranchId = uiBranchId;

                        /* Stage positions of Master branch and this branch(i) will be same */
                        hRapCh->sChAudProcessingStage[i][j].ui32MasterStageId = j;             
                    }           
                }    
                hRapCh->sChAudProcessingStage[i][j].hValidDestHandle = hAudioNetworkDest[i];                    
            }
        }
    } /* if(bAssociationNw == false) */
    else
    {
        BRAP_AssociatedChannelHandle    hAssociatedCh;    

        hAssociatedCh = *(BRAP_AssociatedChannelHandle *)pHandle;    
        BDBG_ASSERT(hAssociatedCh);

        if(true == bDecoderEnabled)
        {
            if (BRAP_DSPCHN_DecodeMode_eDecode == eDecodeMode)
            {
                hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].bDecoderStage = true;
                uiStageId++;
            }
            else if (BRAP_DSPCHN_DecodeMode_ePassThru == eDecodeMode)
            {
                hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].bDecoderStage = true;              
                uiBranchId++;
                uiStageId++;                
            }   
        }
        
    /* if branch has only decode Stage (+ any internally added processing algorithm) then it should be linked to some destiantion handle*/
        if((uiStageId > 0) ||(uiBranchId > 0))
        {
            k = 0;
            for(i=0; i<BRAP_P_MAX_DST_PER_RAPCH ;i++)
            {
                if((NULL != hAudioNetworkDest[i])
                    &&  (k <BRAP_P_MAX_DEST_PER_PROCESSING_STAGE)
                    && (NULL == hAudioProcessingMatrix[i][0])) /*Only those destination is added to 
                    decoder stage which is directly getting data from decoder stage, not through any other PP Statge */
                {        
                    switch (eDecodeMode)
                    {
                        case BRAP_DSPCHN_DecodeMode_eDecode:
                            if(NULL == hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing)
                            {
                                hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                            }
                            else
                            {
                                hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId-1].hAudioProcessing->hDestHandle[k] = hAudioNetworkDest[i];                                      
                            }
                            
                            k++;
                            break;
                        case BRAP_DSPCHN_DecodeMode_ePassThru:
                            hAssociatedCh->sAudProcessingStage[uiBranchId-1][uiStageId-1].hDestHandle[k] = hAudioNetworkDest[i];
                            k++;
                            break;
                        default:
                            BDBG_ASSERT(0);
                            err = BERR_TRACE(BERR_INVALID_PARAMETER);
                            goto end;                                    
                    }
                    break;
                }
            }
        }

        for(i = 0; i< BRAP_P_MAX_DST_PER_RAPCH; i++)
        {
            if(NULL == hAudioNetworkDest[i])
            {
                break;
            }

            for(j = 0; j < uiStageId; j++)
            {
                if(i >0)
                {
                    hAssociatedCh->sAudProcessingStage[i][j].bCloneStage = true;
                    hAssociatedCh->sAudProcessingStage[i][j].ui32MasterBranchId = 0;
                    hAssociatedCh->sAudProcessingStage[i][j].ui32MasterStageId = j;             
                }           
                hAssociatedCh->sAudProcessingStage[i][j].hValidDestHandle = hAudioNetworkDest[i];
            }
        }
    }
    *puiStartBranchId = uiBranchId;
    *puiStartStageId = uiStageId;

end:
    if(psProcessingStageSettings)
        BKNI_Free(psProcessingStageSettings);
    BDBG_LEAVE(BRAP_FWIF_P_InsertInternalStages);
    return err;

}

/***************************************************************************
Summary:
        Forms the audio processing network.
    
Description:
        This function forms the audio processing network and stores it in the 
        RAP Channel Handle.
Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_FWIF_P_FormProcessingNetworks(
	BRAP_ChannelHandle      hRapCh,		        /* [in] RAP Channel handle */
	bool                    bDecoderEnabled,    /* [in] If decoder is present */
	BRAP_DSPCHN_DecodeMode	eDecodeMode,         /* [in] Decode Mode */
	BRAP_DSPCHN_AudioType eAudioType
)
{
    BERR_Code                       err = BERR_SUCCESS;
    BRAP_DestinationHandle          hDestHandleFound[BRAP_P_MAX_DST_PER_RAPCH];
    BRAP_DestinationHandle          hChNetworkDest[BRAP_P_MAX_DST_PER_RAPCH];
    unsigned int                    uiAssociationId =0,uiId=0;
    unsigned int                    uiChDstCount =0;
    unsigned int                    uiDstId=0, uiValidDst=0, uiStageId =0,uiBranchId=0;
    unsigned int                    uiCommonDstCount[BRAP_MAX_ASSOCIATED_GROUPS];
    unsigned int                             i=0, j=0,k=0,l=0,uiMisMatchPt =0;
    BRAP_AssociatedChannelHandle    hAssociatedCh = NULL;

    bool                            bAudProcAdded = false, bDstnWithAssocPPFound = false;
    BRAP_P_ChannelAudioProcessingStage sTempStage;
    unsigned int uiStartBranchId =0, uiStartStageId =0;

    BRAP_ProcessingStageHandle      **hAudioProcessingMatrix=NULL;/*[BRAP_P_MAX_DST_PER_RAPCH][BRAP_MAX_PP_PER_BRANCH_SUPPORTED];    */
    BRAP_DestinationHandle          *hCommonNetworkDest=NULL;/*[BRAP_P_MAX_DST_PER_RAPCH*BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP];*/
    BRAP_DestinationHandle          **hAssociationNetworkDest=NULL;/*[BRAP_MAX_ASSOCIATED_GROUPS][ BRAP_P_MAX_DST_PER_RAPCH];*/    
    BRAP_ProcessingStageHandle      ***hCommonProcessingMatrix=NULL;/* [BRAP_MAX_ASSOCIATED_GROUPS][BRAP_P_MAX_DST_PER_RAPCH][BRAP_MAX_PP_PER_BRANCH_SUPPORTED];*/

#ifdef RAP_PCMROUTER_SUPPORT       
    bool    bFadePresent = false;
    unsigned int uiFadePosition = BRAP_INVALID_VALUE;
    unsigned int uiDestinationNo = BRAP_INVALID_VALUE;
    BRAP_ProcessingStageSettings	 *psProcessingStageSettings;  
    BRAP_ProcessingStageHandle	 hTempProcessingHandle =NULL;    
    unsigned int count =0;
    psProcessingStageSettings = (BRAP_ProcessingStageSettings	 *)BKNI_Malloc(sizeof(BRAP_ProcessingStageSettings));
    if(psProcessingStageSettings == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }    
    BKNI_Memset(psProcessingStageSettings,0,sizeof(BRAP_ProcessingStageSettings));    
    
#endif
    
    BDBG_ENTER( BRAP_FWIF_P_FormProcessingNetworks );

    BDBG_ASSERT( hRapCh );

    hAudioProcessingMatrix= (BRAP_ProcessingStageHandle **)BKNI_Malloc(4*BRAP_P_MAX_DST_PER_RAPCH);
    if(hAudioProcessingMatrix == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }      
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        hAudioProcessingMatrix[i] = (BRAP_ProcessingStageHandle *)BKNI_Malloc(4*BRAP_MAX_PP_PER_BRANCH_SUPPORTED);
        if(hAudioProcessingMatrix[i] == NULL)
        {
            err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto end;
        }              
    }

    hCommonNetworkDest= (BRAP_DestinationHandle *)BKNI_Malloc(4*BRAP_P_MAX_DST_PER_RAPCH*BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP);
    if(hCommonNetworkDest == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }   

    hAssociationNetworkDest= (BRAP_DestinationHandle **)BKNI_Malloc(4*BRAP_MAX_ASSOCIATED_GROUPS);
    if(hAssociationNetworkDest == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }      
    for(i=0;i<BRAP_MAX_ASSOCIATED_GROUPS;i++)
    {
        hAssociationNetworkDest[i] = (BRAP_DestinationHandle *)BKNI_Malloc(4*BRAP_P_MAX_DST_PER_RAPCH);
        if(hAssociationNetworkDest[i] == NULL)
        {
            err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto end;
        }              
    }    


    hCommonProcessingMatrix= (BRAP_ProcessingStageHandle ***)BKNI_Malloc(4*BRAP_MAX_ASSOCIATED_GROUPS);
    if(hCommonProcessingMatrix == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }      
    for(i=0;i<BRAP_MAX_ASSOCIATED_GROUPS;i++)
    {
        hCommonProcessingMatrix[i] = (BRAP_ProcessingStageHandle **)BKNI_Malloc(4*BRAP_P_MAX_DST_PER_RAPCH);
        if(hCommonProcessingMatrix[i] == NULL)
        {
            err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto end;
        }              
        for(j=0;j<BRAP_P_MAX_DST_PER_RAPCH;j++)
        {
            hCommonProcessingMatrix[i][j] = (BRAP_ProcessingStageHandle *)BKNI_Malloc(4*BRAP_MAX_PP_PER_BRANCH_SUPPORTED);
            if(hCommonProcessingMatrix[i][j] == NULL)
            {
                err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                goto end;
            }            
        }            
    }   

    /* Invalid Init */
    for(uiDstId=0;uiDstId<BRAP_P_MAX_DST_PER_RAPCH;uiDstId++)
    {
        hDestHandleFound[uiDstId]=NULL;
		hChNetworkDest[uiDstId]=NULL;
        hCommonNetworkDest[uiDstId]=NULL;
    }
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        for(j=0;j<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;j++)
        {
            hAudioProcessingMatrix[i][j]=NULL;
        }
    }
	
    for(i=0;i<BRAP_MAX_ASSOCIATED_GROUPS;i++)
    {
        uiCommonDstCount[i] = 0;    
        for(j=0;j<BRAP_P_MAX_DST_PER_RAPCH;j++)
        {
            hAssociationNetworkDest[i][j]=NULL;
        }
    }
    
    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {
        for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
        {
            for(j=0;j<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;j++)
            {
                hCommonProcessingMatrix[uiId][i][j]=NULL;
            }
        }
    }
    
    sTempStage.bCloneStage = false;
    sTempStage.bDecoderStage = false;
    sTempStage.bInternalProcessingStage = false;    
    sTempStage.hAudioProcessing = NULL;
    sTempStage.ui32CitBranchId = BRAP_INVALID_VALUE;
    sTempStage.ui32CitStageId = BRAP_INVALID_VALUE;
    for(i=0;i<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;i++)    
    {
        sTempStage.hDestHandle[i] = NULL;
    }
    sTempStage.ui32MasterBranchId = BRAP_INVALID_VALUE;
    sTempStage.ui32MasterStageId = BRAP_INVALID_VALUE;
    sTempStage.hValidDestHandle = NULL;

    for (i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        for(j=0;j<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;j++)
        {
            hRapCh->sChAudProcessingStage[i][j] = sTempStage;
        }
    }
    
    /* Make the Audio Processing network for all the associations */
    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[hRapCh->uiAssociationId[uiId]]);

        if(hAssociatedCh != NULL)
        {
            for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
            {
                for(j=0;j<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;j++)
                {
                    hAssociatedCh->sAudProcessingStage[i][j] = sTempStage;
                }
            }
            /* Populate the hDestHandleFound with Valid Dest Handles */
            for(uiDstId=0;uiDstId<BRAP_P_MAX_DST_PER_RAPCH;uiDstId++)
            {
                if(BRAP_AudioDst_eMax != hAssociatedCh->sDstDetails[uiDstId].sExtDstDetails.eAudioDst)
                {
                    hDestHandleFound[uiValidDst] = &(hAssociatedCh->sDstDetails[uiDstId]);
                    BDBG_MSG(("----> Valid Destination found , Destination: %d",hDestHandleFound[uiValidDst]->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));
                    uiValidDst++;
                }
            }
        } /* if (hAssociatedCh != NULL) */
    } /* For uiId */
    
    /* Find Common(Association) Network Dst */
    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {    
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        uiAssociationId = hRapCh->uiAssociationId[uiId];        
        hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[uiAssociationId]);

        if(hAssociatedCh != NULL)
        {    
            for(i =0; i<uiValidDst; i++)
            {
                if ((NULL != hDestHandleFound[i])
                	&&(BRAP_P_IsAssociationNWValid(hDestHandleFound[i]))
                	&&(hDestHandleFound[i]->hAssociation == hAssociatedCh)) 
                {
                    /* Check if any post processing stages added in this entry */
                    for(k=0; k< BRAP_MAX_PP_PER_BRANCH_SUPPORTED;k++)
                    {
                        if(hDestHandleFound[i]->psProcessingSettings[BRAP_P_MAX_RAPCH_PER_DST]->
                                      sExternalSettings.hAudProcessing[k]!=NULL)
                        {
                            hCommonNetworkDest[uiCommonDstCount[uiAssociationId]++] = hDestHandleFound[i];
                            break;
                        }
                    }
                }
            }            
        }
    }

    /* Find Channel Network Dst */
    for(i =0 ;i<uiValidDst;i++)
    {
        if(NULL == hDestHandleFound[i])
            continue;
        
        if(BRAP_P_IsAssociationNWValid(hDestHandleFound[i]))
        {
            /* Represent one Channel NW destination(with NULL handle) for loopback path */
            if(bDstnWithAssocPPFound == false)
            {
                bDstnWithAssocPPFound = true;
                hChNetworkDest[uiChDstCount++]=NULL;
            }
        }
        else
        {
            /* Destination on Channel network */
            hChNetworkDest[uiChDstCount++]=hDestHandleFound[i];
            BDBG_MSG(("----> Valid Channel specific Destination found , Destination: 0x%x",hChNetworkDest[uiChDstCount -1]));
        }
    }

    for(uiId=0; (uiId<BRAP_MAX_ASSOCIATED_GROUPS);uiId++)
    {
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        /* Segregate further the destinations for common networks */
            /*TODO: Take association Id from the array when support for multiple 
                association per RAPCH is added */
                uiAssociationId = hRapCh->uiAssociationId[uiId];
            hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[uiAssociationId]);
            i = 0;
            if(NULL != hAssociatedCh)
            {
            for(k=0; k < uiCommonDstCount[uiAssociationId]; k++)
                {
                    if(hCommonNetworkDest[k]->hAssociation==hAssociatedCh)
                    {
                    	hAssociationNetworkDest[uiAssociationId][i++] = hCommonNetworkDest[k];
                }
            }
        }
    }
#ifdef RAP_PCMROUTER_SUPPORT       
        /*Check if the Network has Fade Stage */
    /* Form Audio Processing Matrix for Channel specific audio Processing */
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        if(NULL != hChNetworkDest[i])
        {
            for(j=0;j<BRAP_P_MAX_RAPCH_PER_DST+1 ;j++)
            {
                if((NULL != hChNetworkDest[i]->psProcessingSettings[j])
                    && (hChNetworkDest[i]->psProcessingSettings[j]->hRapCh == hRapCh))
                {
                    for(k=0;k<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;k++)
                    {
                        if((hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k] != NULL)
                            &&(hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k]->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eAdFade))
                        {
                            bFadePresent = true;
                            uiFadePosition = k;
                            uiDestinationNo = i;
                        }
                    }
                }
            }
        }
    }
    if(true == bFadePresent)
    {
        err = BRAP_GetDefaultProcessingStageSettings( BRAP_ProcessingType_ePCMRouter,psProcessingStageSettings );
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("GetDefaultProcessingStageSettings for ePCMRouter failed"));
            err = BERR_TRACE(err);
            goto end;
        }
        err = BRAP_CreateProcessingStage(hRapCh->hRap ,psProcessingStageSettings,&hTempProcessingHandle);
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("Creation of the PCM Router stage failed"));
            err = BERR_TRACE(err);
            goto end;
        }
        hTempProcessingHandle->bChSpecificStage = true;                                    
        hTempProcessingHandle->uHandle.hRapCh = hRapCh;
        hTempProcessingHandle->uiStagePosition = uiFadePosition;    
        BDBG_MSG(("bFadePresent =%d , uiFadePosition =%d, uiDestinationNo =%d",bFadePresent,uiFadePosition,uiDestinationNo));

        for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
        {
            if(NULL != hChNetworkDest[i])
            {
                    if((true == bFadePresent)
                        &&(uiDestinationNo != i ))
                    { 
                        BDBG_MSG(("Adding Fade at branch %d , stage %d destination %d",i,uiFadePosition,hChNetworkDest[i]->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));            
                        hAudioProcessingMatrix[i][uiFadePosition] =  hTempProcessingHandle;
                        hAudioProcessingMatrix[i][uiFadePosition]->hDestHandle[count++] = hChNetworkDest[i];                
                    }
            }
        }        
    }
    
#endif    
    /* Form Audio Processing Matrix for Channel specific audio Processing */
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        if(NULL != hChNetworkDest[i])
        {
            for(j=0;j<BRAP_P_MAX_RAPCH_PER_DST+1 ;j++)
            {
                if((NULL != hChNetworkDest[i]->psProcessingSettings[j])
                    && (hChNetworkDest[i]->psProcessingSettings[j]->hRapCh == hRapCh))
                {
                    for(k=0;k<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;k++)
                    {
#ifdef RAP_PCMROUTER_SUPPORT 
                        BDBG_MSG(("bFadePresent = %d uiDestinationNo = %d , i =%d ,k =%d ",bFadePresent,uiDestinationNo,i,k));
                        if((true == bFadePresent)
                            &&(uiDestinationNo != i ))
                        {                 
                            BDBG_MSG(("uiDestinationNo = %d , i =%d ,k =%d ",uiDestinationNo,i,k));
                            if( (k >= uiFadePosition) && (NULL != hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k]))
                            {
                                hAudioProcessingMatrix[i][k+1] = hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k];
                                hAudioProcessingMatrix[i][k+1]->uiStagePosition++;
                            }                                
                            else
                            {
                                hAudioProcessingMatrix[i][k] = hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k];
                            }                                
                        }
                        else
                        {
                            hAudioProcessingMatrix[i][k] = hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k];
                            if(hAudioProcessingMatrix[i][k]!=NULL)
                                BDBG_MSG(("i =%d , k = %d , hAudProcessing =%d",i,k,hAudioProcessingMatrix[i][k]->sProcessingStageSettings.eAudioProcessing));
                        }
#else
                        hAudioProcessingMatrix[i][k]=
                            hChNetworkDest[i]->psProcessingSettings[j]->sExternalSettings.hAudProcessing[k];
#endif                    
                        if (NULL != hAudioProcessingMatrix[i][k])
                        {
                            BDBG_MSG(("Found processing stage =%x for destniation %d",hAudioProcessingMatrix[i][k]->sProcessingStageSettings.eAudioProcessing,hChNetworkDest[i]->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));
                            BDBG_MSG(("i > %d, k > %d", i, k));
#if (BRAP_7405_FAMILY == 1)
/* This implementaion is to fix a case where Dts XCD to PCm transition on 
HDMI soesn't happen on Onkyo reciever when we don't diable MAI_CFG at MAI_STOP*/
                            if((hAudioProcessingMatrix[i][k]->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eEncodeDts)
                                &&(hChNetworkDest[i]->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] == BRAP_OutputPort_eMai))
                            {
                                hRapCh->hRap->bLastRunHadDtsTranscodeOnHdmi = true;
                            }             

                            if((hAudioProcessingMatrix[i][k]->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eEncodeDts)
                                &&(hChNetworkDest[i]->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] == BRAP_OutputPort_eSpdif))
                            {
                                hRapCh->hRap->bLastRunHadDtsTranscodeOnSpdif = true;
                            }                              
#endif
                        }
                    }
                }
            }
        }
    }
/* Check consistency of stage position. In a stage network, a stage can appear in multiple rows but it can't appear in multiple columns. */    
    for (i = 0; i < BRAP_P_MAX_DST_PER_RAPCH; i++)
    {
    	for (j = 0; j < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; j++)
    	{
    	    if(hAudioProcessingMatrix[i][j] == NULL)
            {
                break;
            }
            if(hAudioProcessingMatrix[i][j]->uiStagePosition!=j)
            {
                BDBG_ERR(("Error !!! In a stage network, a stage can appear in multiple rows but it can't appear in multiple columns"));
                err = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto end;
            }
    	}
    }

    /* Check consistency of order of stages */
    for(i = 0; i <( BRAP_P_MAX_DST_PER_RAPCH-1); i++)
    {
        for(j = ( i+1); j < BRAP_P_MAX_DST_PER_RAPCH; j++)
        {
            bool bFoundMismatch = false;

            /* Find out point of mis-match */
            for(k = 0; k < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; k++)
            {
                if((hAudioProcessingMatrix[i][k] == NULL)
                    || (hAudioProcessingMatrix[j][k] == NULL))
                {
                    break;
                }            
                if(hAudioProcessingMatrix[i][k]!=hAudioProcessingMatrix[j][k])
                {
                    uiMisMatchPt = k;
                    bFoundMismatch = true;
                    break;
                }
            }
            if ( bFoundMismatch )
            {
                /* After point of mis-match, none of the stages in same position (column) on two branches (rows) should match */
                for(k = uiMisMatchPt; k < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; k++)
                {
                    if((hAudioProcessingMatrix[i][k] == NULL)
                    || (hAudioProcessingMatrix[j][k] == NULL))
                    {
                        break;
                    }            

                    if(hAudioProcessingMatrix[i][k] ==hAudioProcessingMatrix[j][k])
                    {
                        BDBG_ERR(("Error !!! order of stages in the network is not consistent"));                
                        err = BERR_TRACE(BERR_INVALID_PARAMETER);
                        goto end;
                    }
                }
            }
        }
    }

    
    /* Populate the AudioProcessing info in hRapCh */
    err = BRAP_FWIF_P_InsertInternalStages(&hRapCh,bDecoderEnabled,eDecodeMode,eAudioType,
            false, hAudioProcessingMatrix,hChNetworkDest,&uiStartBranchId,&uiStartStageId);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_FWIF_P_InsertInternalStages failed"));
        err = BERR_TRACE(err);
        goto end;
    }
    /* Attach different PP stages to it's corresponding branches, start from branch 0 */
    uiBranchId = 0;
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        if(NULL == hChNetworkDest[i]) /* i.e.If there is valid destination, then there will be a valid brach corresponding to it*/
        {
            break;
        }
        for(j=0; j<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED ;j++)
        {        
            if(!(BRAP_P_STAGE_VALID(hRapCh->sChAudProcessingStage,uiBranchId,j)))
                break;
        }        
        uiStageId = j;

        for(j=0;j<BRAP_MAX_PP_PER_BRANCH_SUPPORTED ;j++)
        {
            if(NULL != hAudioProcessingMatrix[i][j])
            {
                bAudProcAdded = false;
                /* Find out if this stage has already been added to Network */
                for(k=0;k<uiBranchId;k++)
                {
/*                    for(l=0;l<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;l++)*/
                    for(l=0;l<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;l++)
                    {
                        if(NULL == &(hRapCh->sChAudProcessingStage[k][l])) /* No more valid stage after it */
                            break;
                        else if(hAudioProcessingMatrix[i][j] ==hRapCh->sChAudProcessingStage[k][l].hAudioProcessing)
                        {
                            bAudProcAdded = true;
                            break;
                        }
                    }
                    if(bAudProcAdded == true)
                        break;
                }

                if(bAudProcAdded == true)
                {
                    /* This Stage is already in network */
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].bCloneStage = true;
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].ui32MasterBranchId = k;
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].ui32MasterStageId = l;
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].hValidDestHandle = hChNetworkDest[i];
                }
                else
                {
                    /* This stage is not a part of network. Add it now */
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].hAudioProcessing = hAudioProcessingMatrix[i][j];
                    hRapCh->sChAudProcessingStage[uiBranchId][uiStageId].hValidDestHandle = hChNetworkDest[i];
                }
                /* Stage added Increment the count */
                uiStageId++;
            }
            else
                break;
        }
        /* Branch Populated. Increment the Count */
        uiBranchId++;
    }
    
    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        /* Form Audio Processing Matrix for Common audio Processing */
        uiAssociationId = hRapCh->uiAssociationId[uiId];
        hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[uiAssociationId]);
        for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
        {
            if(NULL != hAssociationNetworkDest[uiAssociationId][i])
            {
                if(BRAP_P_IsAssociationNWValid(hAssociationNetworkDest[uiAssociationId][i]))
                {
                    for(k=0;k<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;k++)
                    {
                        if(NULL != hAssociationNetworkDest[uiAssociationId][i]->psProcessingSettings[BRAP_P_MAX_RAPCH_PER_DST]->sExternalSettings.hAudProcessing[k])
                        {
                            hCommonProcessingMatrix[uiAssociationId][i][k]=
                            hAssociationNetworkDest[uiAssociationId][i]->psProcessingSettings[BRAP_P_MAX_RAPCH_PER_DST]->sExternalSettings.hAudProcessing[k];                            
                        }
                    }
                }
            }
        }
    }

    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        
        uiAssociationId = hRapCh->uiAssociationId[uiId];
        hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[uiAssociationId]);
        
        /* Check consistency of stage position. In a stage network, a stage can 
           appear in multiple rows but it can't appear in multiple columns. */    
        for (i = 0; i < BRAP_P_MAX_DST_PER_RAPCH; i++)
        {
        	for (j = 0; j < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; j++)
        	{
                if(hCommonProcessingMatrix[uiAssociationId][i][j] == NULL)
                {
                    break;
                }            

                if(hCommonProcessingMatrix[uiAssociationId][i][j]->uiStagePosition != j)
                {
                    BDBG_ERR(("Error !!! In a stage network, a stage can appear"
                        " in multiple rows but it can't appear in multiple columns"));
                    err = BERR_TRACE(BERR_INVALID_PARAMETER);
                    goto end;
                }
        	}
        }

        /* Check consistency of order of stages. In a network a stage 
           cannot exist in same column for multiple rows */
        for(i = 0; i <( BRAP_P_MAX_DST_PER_RAPCH-1); i++)
        {
            for(j = ( i+1); j < BRAP_P_MAX_DST_PER_RAPCH; j++)
            {
                bool bFoundMismatch = false;

                /* Find out point of mis-match */
                for(k = 0; k < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; k++)
                {
                    if((hCommonProcessingMatrix[uiAssociationId][i][k] == NULL)
                        || (hCommonProcessingMatrix[uiAssociationId][j][k] == NULL))
                    {
                        break;
                    }            
                
                    if(hCommonProcessingMatrix[uiAssociationId][i][k] !=
                       hCommonProcessingMatrix[uiAssociationId][j][k])
                    {
                        uiMisMatchPt = k;
                        bFoundMismatch = true;
                        break;
                    }
                }
                if ( bFoundMismatch )
                {
                    /* After point of mis-match, none of the stages in same position (column) on two branches (rows) should match */
                    for(k = uiMisMatchPt; k < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; k++)
                    {
                        if((hCommonProcessingMatrix[uiAssociationId][i][k] == NULL)
                            || (hCommonProcessingMatrix[uiAssociationId][j][k] == NULL))
                        {
                            break;
                        }            
                    
                        if(hCommonProcessingMatrix[uiAssociationId][i][k] ==
                           hCommonProcessingMatrix[uiAssociationId][j][k])
                        {
                            BDBG_ERR(("Error !!! order of stages in the network is not consistent"));                
                            err =  BERR_TRACE(BERR_INVALID_PARAMETER);
                            goto end;
                        }
                    }
                }
            }
        }
    }

    for(uiId=0;uiId<BRAP_MAX_ASSOCIATED_GROUPS;uiId++)
    {
        if(hRapCh->uiAssociationId[uiId] == BRAP_INVALID_VALUE)
            continue;
        
        uiAssociationId = hRapCh->uiAssociationId[uiId];
        hAssociatedCh = &(hRapCh->hRap->sAssociatedCh[uiAssociationId]);

        if(NULL != hAssociatedCh)
        {
            for(k = 0; k < BRAP_P_MAX_DST_PER_RAPCH; k++)
            {        
                if((NULL != hAssociationNetworkDest[uiAssociationId][k]) &&
                   (hAssociationNetworkDest[uiAssociationId][k]->hAssociation == hAssociatedCh)) 
                {
                    break;
                }
            }
            if(k == BRAP_P_MAX_DST_PER_RAPCH)
                continue;
            
            uiBranchId=0;

            if(NULL != hAssociationNetworkDest[uiAssociationId][0])
            {
                /* Currently no decode stage required in Assoc N/W */
                bDecoderEnabled = false;

                /* Populate the AudioProcessing info in hRapCh */
                err = BRAP_FWIF_P_InsertInternalStages(&hAssociatedCh,bDecoderEnabled,eDecodeMode,
                        eAudioType,true,hCommonProcessingMatrix[uiAssociationId],
                        hAssociationNetworkDest[uiAssociationId],&uiStartBranchId,&uiStartStageId);
                if(err != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_FWIF_P_InsertInternalStages failed"));
                    err = BERR_TRACE(err);
                    goto end;
                }            
            }
                
            for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
            {
                if(NULL == hAssociationNetworkDest[uiAssociationId][i])
                {
                    break;      
                }
        
                uiStageId=uiStartStageId;
                for(j=0;j<BRAP_MAX_PP_PER_BRANCH_SUPPORTED ;j++)
                {
                    if(NULL != hCommonProcessingMatrix[uiAssociationId][i][j])
                {
                    bAudProcAdded = false;                
                    /* Find out if this stage has already been added to Network */
                    for(k=0;k<uiBranchId;k++)
                    {
                        for(l=0;l<BRAP_MAX_PP_PER_BRANCH_SUPPORTED;l++)
                        {
                            if(NULL == &(hAssociatedCh->sAudProcessingStage[k][l])) /* No more valid stage after it */
                                break;
                                else if(hCommonProcessingMatrix[uiAssociationId][i][j] == hAssociatedCh->sAudProcessingStage[k][l].hAudioProcessing)
                            {
                                bAudProcAdded = true;
                                break;
                            }
                        }
                        if(bAudProcAdded == true)
                                break;
                    }

                    if(bAudProcAdded == true)
                    {
                        /* This Stage is already in network */
                        hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].bCloneStage = true;
                        hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].ui32MasterBranchId = k;
                        hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].ui32MasterStageId = l;
                                hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].
                                    hValidDestHandle = hAssociationNetworkDest[uiAssociationId][i];
                    }
                    else
                    {
                        /* This stage is not a part of network. Add it now */
                            hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].
                                hAudioProcessing = hCommonProcessingMatrix[uiAssociationId][i][j];
                            hAssociatedCh->sAudProcessingStage[uiBranchId][uiStageId].
                                hValidDestHandle = hAssociationNetworkDest[uiAssociationId][i];
                    }
                    /* Stage added Increment the count */
                    uiStageId++;
                }
                else
                    break;
            }
            /* Branch Populated. Increment the Count */
            uiBranchId++;
        }
        }
    }
    
end:        
#ifdef RAP_PCMROUTER_SUPPORT           
    if(psProcessingStageSettings)
        BKNI_Free(psProcessingStageSettings);    
#endif    
    if(hAudioProcessingMatrix !=NULL)
    {
        for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
        {     
            if(hAudioProcessingMatrix[i] != NULL)
                BKNI_Free(hAudioProcessingMatrix[i]);            
        }
        BKNI_Free(hAudioProcessingMatrix);
    }

    if(hCommonNetworkDest !=NULL)
        BKNI_Free(hCommonNetworkDest);

    if(hAssociationNetworkDest !=NULL)
    {
        for(i=0;i<BRAP_MAX_ASSOCIATED_GROUPS;i++)
        {     
            if(hAssociationNetworkDest[i] != NULL)
                BKNI_Free(hAssociationNetworkDest[i]);            
        }
        BKNI_Free(hAssociationNetworkDest);   
    }

    if(hCommonProcessingMatrix !=NULL)
    {
        for(i=0;i<BRAP_MAX_ASSOCIATED_GROUPS;i++)
        {    
            if(hCommonProcessingMatrix[i] != NULL)
            {     
                for(j=0;j<BRAP_P_MAX_DST_PER_RAPCH;j++)
                {     
                    if(hCommonProcessingMatrix[i][j] != NULL)
                        BKNI_Free(hCommonProcessingMatrix[i][j]);            
                }
                BKNI_Free(hCommonProcessingMatrix[i]);                    
            }
        }
        BKNI_Free(hCommonProcessingMatrix);  
    }


    BDBG_LEAVE( BRAP_FWIF_P_FormProcessingNetworks );
    return err;    
}


bool    BRAP_FWIF_P_IsDDREPPAdded(    
    BRAP_P_AudioProcNetwork             *psAudProcNetwork  /* [in] Audio Processing Network */
    )
{
    unsigned int i=0,j=0;
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        for(j=0;j<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;j++)
        {
            if((psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing!=NULL)
                &&(psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eDDRE))
            {
                return true;
            }
        }
    }
    return false;
}

bool    BRAP_FWIF_P_CheckDDREUsageModeValidity(   
    BRAP_Handle                         hRap,               /* [in] Rap Device Handle */    
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    unsigned int    *uiDDREStageId
    )
{
    unsigned int i=0,j=0,uiStartBranch=0;
    bool    bValid=true;

    *uiDDREStageId = BRAP_INVALID_VALUE;
#if 0
    if((psAudProcNetwork->sAudProcessingStage[0][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
        &&(psAudProcNetwork->sAudProcessingStage[0][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] != BRAP_OutputPort_eMai)        
        &&(psAudProcNetwork->sAudProcessingStage[0][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]
        == hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector))
    {
        uiStartBranch= 1;
    }
#endif    
    for(j=0;j<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;j++)
    { 
        BDBG_MSG(("psAudProcNetwork->sAudProcessingStage[%d][%d].hAudioProcessing= %x psAudProcNetwork->sAudProcessingStage[0][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] =%d"
            ,uiStartBranch,j,psAudProcNetwork->sAudProcessingStage[uiStartBranch][j].hAudioProcessing
            ,psAudProcNetwork->sAudProcessingStage[uiStartBranch][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));
        if((psAudProcNetwork->sAudProcessingStage[uiStartBranch][j].hAudioProcessing!=NULL)
            &&(psAudProcNetwork->sAudProcessingStage[uiStartBranch][j].hAudioProcessing->sProcessingStageSettings.eAudioProcessing == BRAP_ProcessingType_eDDRE))
        {
            *uiDDREStageId = j;
            break;
        } 
    }


    if(*uiDDREStageId != BRAP_INVALID_VALUE)
    {
        for(i=uiStartBranch + 1;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
        {        
            if(!(BRAP_P_BRANCH_VALID(psAudProcNetwork->sAudProcessingStage,i)))
            {
                break;
            }
            if((psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                &&(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] != BRAP_OutputPort_eMai)                        
                &&(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]
                == hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector))
            {
                continue;
            }
            for(j=0;j<*uiDDREStageId;j++)
            {                       
                if(psAudProcNetwork->sAudProcessingStage[i][j].bDecoderStage==true)
                {                                  
                    if(psAudProcNetwork->sAudProcessingStage[0][j].bDecoderStage==false)
                    {                                                         
                        bValid = false;
                        break;
                    }
                }
                else if (psAudProcNetwork->sAudProcessingStage[i][j].bCloneStage==true)
                {                                                    
                    if((psAudProcNetwork->sAudProcessingStage[i][j].ui32MasterBranchId!=0)
                        ||(psAudProcNetwork->sAudProcessingStage[i][j].ui32MasterStageId!=j))
                    {                                                          
                        bValid = false;
                        break;
                    }            
                }
                else if (psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing !=NULL)
                {                                                        
                        bValid = false;
                        break;
                } 
                else
                {
                    /*Error Case*/
                        bValid = false;
                        break;                    
                }
            }
        }
    }
    BDBG_MSG(("uiDDREStageId = %d, bValid = %d",*uiDDREStageId,bValid));
    if(*uiDDREStageId == BRAP_INVALID_VALUE)
    {
        bValid = false;
    }
    if(bValid== false)
    {
        *uiDDREStageId =BRAP_INVALID_VALUE;
    }
    return bValid;
}



/***************************************************************************
Summary:
        Generate the CIT input Structure
    
Description:
        This function takes the audio processing network from channel handle and
        Generates the CIT input structure to be passed as Input to CIT module.
        Preprocessing is not supported.
Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_FWIF_P_GenerateCitInput(
    BRAP_Handle                         hRap,               /* [in] Rap Device Handle */
    BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    BRAP_P_NetworkInfo                     *psNetworkInfo,          /* [in] input network Info */
    BRAP_CIT_P_InputInfo                *pCitInputStruct,    /* [out] CIT input structure */
    BRAP_FWIF_P_FwTaskHandle hFwTaskCreate     /*[in] Fw Task handle */
)
{
    BERR_Code                       ret=BERR_SUCCESS;
    unsigned int                             i=0, j=0,k=0,l=0;
    unsigned int                        ui32BranchId =0,ui32StageId =0;
    unsigned int                        ui32SrcBranchId =0,ui32SrcStageId =0;
    unsigned int                        ui32MasterBranchId =0,ui32MasterStageId =0;
    unsigned int                        uiDecoderStageId=BRAP_INVALID_VALUE,uiDecoderBranchId=BRAP_INVALID_VALUE;
    unsigned int                    uiNumDst = 0/*,uiNumSrc=0*/;
    BRAP_ProcessingStageHandle      hTempHandle = NULL;
    bool bBreakLoop =false, bStageFound =false;
    bool bCompleteBranchCloned = false;
    bool bFlag =false;
    unsigned int uiDistinctOut = 0;
    BRAP_AF_P_DistinctOpType eDistinctOutType = BRAP_AF_P_DistinctOpType_eInvalid;
    bool bIsFwBranch =true;
    unsigned int m=0, uiCloneBranchId[BRAP_P_MAX_DST_PER_RAPCH];     
    bool bCloned =false;
    BRAP_ChannelHandle          hRapCh = NULL;
    BRAP_OutputMode            eChannelOutputMode = BRAP_OutputMode_eLast;
    BRAP_AF_P_OutModeType      eOutModeType = BRAP_AF_P_OutModeType_eBypass;
    unsigned uiTempBranchId = BRAP_INVALID_VALUE,uiTempStageId = BRAP_INVALID_VALUE;    
    bool    bDac0Present =false, bDac1Present = false, bDac2Present = false;
    bool    bOtherOpPresent = false, bRbufDstnPresent = false;
    unsigned int    uiDac0OpIndex = BRAP_INVALID_VALUE, uiDac1OpIndex = BRAP_INVALID_VALUE;
    unsigned int    uiDac2OpIndex = BRAP_INVALID_VALUE, ui32DistinctOpNum = BRAP_INVALID_VALUE;
    unsigned int    uiRbufDstnIndex = BRAP_INVALID_VALUE, uiTempPllIndex = BRAP_INVALID_VALUE;
    bool    bEnableFixedSampleRateOutput=false;
    unsigned int uiDDREStageId=BRAP_INVALID_VALUE;
    unsigned int uiStartBranchIndex=0,uiStartStageIndex=0;

    BDBG_ENTER(BRAP_FWIF_P_GenerateCitInput);

    BDBG_ASSERT(hRap);
    BDBG_ASSERT(pCitInputStruct);
    BDBG_ASSERT(psAudProcNetwork);
    BDBG_ASSERT(psNetworkInfo);

    BKNI_Memset(pCitInputStruct,0,sizeof(BRAP_CIT_P_InputInfo));
    for(i=0; i< BRAP_P_MAX_FW_BRANCH_PER_FW_TASK; i++)
    {
        pCitInputStruct->pBranchInfo[i] = hRap->sOpenTimeMallocs.psBranchInfo[i];
        if (NULL==pCitInputStruct->pBranchInfo[i])
        {
            BDBG_ERR(("Out of System memory"));
            ret = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        else
        {
            BKNI_Memset(pCitInputStruct->pBranchInfo[i],0,sizeof(BRAP_CIT_P_FwBranchInfo));
        }
    }

    if(true == hDspCh->bChSpecificDspCh)
    {
        hRapCh = hDspCh->uHandle.hRapCh;        
        eChannelOutputMode = hRapCh->eChannelOutputMode;
        bEnableFixedSampleRateOutput = hRapCh->bEnableFixedSampleRateOutput;
    
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)    
        if(true == hRapCh->bIndepDelayEnabled)
        {
            if(BRAP_P_IsPointerValid((void *)psNetworkInfo->sDspOutConfig[0].hDestHandle))
            {
                pCitInputStruct->eIndepDelayEnableFlag = BRAP_AF_P_eEnable;
            }
            else
            {
                /* Disable Independent Delay for Channel N/W output feeding to Association network */
                pCitInputStruct->eIndepDelayEnableFlag = BRAP_AF_P_eDisable;
            }                
        }
        else
#endif            
        {
            pCitInputStruct->eIndepDelayEnableFlag = BRAP_AF_P_eDisable;
        }
    }
    else
    {
        hRapCh = BRAP_P_GetRapChHandle(hDspCh->uHandle.hAssociation);
        if(!(BRAP_P_IsPointerValid((void *)hRapCh)))
        {
            BDBG_ERR(("No Raptor channel found in Association handle"));
            ret = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
        }

        eChannelOutputMode = BRAP_OutputMode_e2_0;        
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)    
        if(true == hRapCh->bIndepDelayEnabled)
        {
            pCitInputStruct->eIndepDelayEnableFlag = BRAP_AF_P_eEnable;
        }
        else
#endif            
        {
            pCitInputStruct->eIndepDelayEnableFlag = BRAP_AF_P_eDisable;
        }        
    }

    if(bEnableFixedSampleRateOutput ==false)
    {
        bool    bDspToProgramClock = true;
    for(i=0;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        if(NULL == psNetworkInfo->sDspOutConfig[i].hDestHandle)
            continue;

        /* Check per Output if Port config structure needs to be filled for clock programming by DSP */
        bDspToProgramClock = BRAP_P_DspToConfigureOutputClock(hRapCh, 
                                    &psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails);
        if(false == bDspToProgramClock)
            continue;
            
        if(BRAP_AudioDst_eOutputPort == psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails.eAudioDst)
        {            
            pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].eFmmDstFsRate= psNetworkInfo->sDspOutConfig[i].sOutputBufConfig.eFmmDstFsRate;        
            pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.eFmmDstSelect = BRAP_CIT_P_FmmDstSelect_eOpPort;

            pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.uFmmDstPortType.eOpType = 
                psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0];
            
#if (BRAP_7405_FAMILY == 1) ||  (BRAP_3548_FAMILY == 1)          
            pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect =  BRAP_AF_P_PllSelect;
#endif

            switch(pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.uFmmDstPortType.eOpType)
            {
                case BRAP_OutputPort_eI2s0:
					pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eI2s0].uOutputPortSettings.sI2sSettings.ePll; 
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;
                    break;
                case BRAP_OutputPort_eI2s1:
					pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eI2s1].uOutputPortSettings.sI2sSettings.ePll; 
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eI2s2:
					pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eI2s2].uOutputPortSettings.sI2sSettings.ePll; 
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eI2s3:
					pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eI2s3].uOutputPortSettings.sI2sSettings.ePll; 
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eI2s4:
					pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eI2s4].uOutputPortSettings.sI2sSettings.ePll; 
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
#if (BRAP_7550_FAMILY == 1)                    
                case BRAP_OutputPort_eMaiMulti0:
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = BRAP_INVALID_VALUE;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;
                    break;
                case BRAP_OutputPort_eMaiMulti1:
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = BRAP_INVALID_VALUE;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eMaiMulti2:
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = BRAP_INVALID_VALUE;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break; 
#endif                    
                case BRAP_OutputPort_eMai:
#if (BRAP_7550_FAMILY == 1)
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = BRAP_INVALID_VALUE;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
#else                    
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.ePll;                       
#endif
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sMaiCfg.sMaiContent.bHbr = 
                        hRap->sOutputSettings[BRAP_OutputPort_eMai].bHbrEnable;                 
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sMaiCfg.sMaiContent.bPcmOnSpdif = 
                        hRap->sOutputSettings[BRAP_OutputPort_eMai].bCompressed;
                    if((hDspCh->sDspAudioParams.sExtAudioParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus)
                        &&(hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru))
                    {
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sMaiCfg.sMaiContent.bHwCbits = true;
                    }
                    else
                    {
                        pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sMaiCfg.sMaiContent.bHwCbits = false;                        
                    }
                    switch(hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector)
                    {
                        case BRAP_OutputPort_eSpdif:                    
                        case BRAP_OutputPort_eMai:                            
                        case BRAP_OutputPort_eI2s0:
                        case BRAP_OutputPort_eI2s1:
                        case BRAP_OutputPort_eI2s2:
                        case BRAP_OutputPort_eI2s3:
                        case BRAP_OutputPort_eI2s5:
                        case BRAP_OutputPort_eI2s6:
                        case BRAP_OutputPort_eI2s7:
                        case BRAP_OutputPort_eI2s8:                                                        
                        case BRAP_OutputPort_eMaiMulti0:                                      
                        case BRAP_OutputPort_eMaiMulti1:                            
                        case BRAP_OutputPort_eMaiMulti2:                            
                        case BRAP_OutputPort_eMaiMulti3:                                                        
                            pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sMaiCfg.eMaiMuxSelector = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
                            break;
                        default:
                            BDBG_ERR(("Invalid  eMaiMuxSelector =%d",hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector));
                            break;                            
                    }
                    

                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eSpdif:
#if (BRAP_7550_FAMILY == 1)					
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = BRAP_INVALID_VALUE;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
#else                    
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;
#endif
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sSpdifCfg.bPcmOnSpdif = 
                        hRap->sOutputSettings[pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.uFmmDstPortType.eOpType].bCompressed;
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex; 
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sSpdifCfg.bHwCbits = false;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sSpdifCfg.bHbr = 
                        hRap->sOutputSettings[BRAP_OutputPort_eSpdif].bHbrEnable;
                    break;
                case BRAP_OutputPort_eSpdif1:
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 
                        hRap->sOutputSettings[BRAP_OutputPort_eSpdif1].uOutputPortSettings.sSpdifSettings.ePll;
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uOpCfg.sSpdifCfg.bPcmOnSpdif = 
                        hRap->sOutputSettings[pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.uFmmDstPortType.eOpType].bCompressed;
                    bOtherOpPresent = true;
                    uiTempPllIndex = pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex;                    
                    break;
                case BRAP_OutputPort_eDac0:
                    bDac0Present = true;
                    uiDac0OpIndex = pCitInputStruct->ui32NumOutputs;
#if (BRAP_7550_FAMILY == 1)					                    
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ePllNcoSelect = BRAP_AF_P_NcoSelect;
#endif                    
                    break;                    
                case BRAP_OutputPort_eDac1:
                    bDac1Present = true;
                    uiDac1OpIndex = pCitInputStruct->ui32NumOutputs;                    
                    break;                    
                case BRAP_OutputPort_eDac2:
                    bDac2Present = true;
                    uiDac2OpIndex = pCitInputStruct->ui32NumOutputs;                    
                    break;                                        
                default:
                    pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].ui32PllIndex = 0;
                    break;
            }
            pCitInputStruct->ui32NumOutputs++;
        }
#if (BRAP_7550_FAMILY != 1)        
        else if(BRAP_AudioDst_eRingBuffer == psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails.eAudioDst)
        {
#if(BRAP_48KHZ_RINGBUFFER_DESTINATION != 1)
            /* If this macro==1 clk programming is done by PI to support fixed 48khz capture */
            bRbufDstnPresent = true;
            uiRbufDstnIndex = pCitInputStruct->ui32NumOutputs;
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].eFmmDstFsRate= psNetworkInfo->sDspOutConfig[i].sOutputBufConfig.eFmmDstFsRate;            
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].ePllNcoSelect =  BRAP_AF_P_PllSelect;            
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].sFmmDstDetails.eFmmDstSelect = BRAP_CIT_P_FmmDstSelect_eCapturePort;
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].sFmmDstDetails.uFmmDstPortType.eCapPortType = 
                psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eCapPort[0];

            BDBG_MSG(("pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].uFmmDstPortType.eCapPortType = %d",
                pCitInputStruct->sPortCfg[pCitInputStruct->ui32NumOutputs].sFmmDstDetails.uFmmDstPortType.eCapPortType));

            pCitInputStruct->ui32NumOutputs++;
#endif            
        }
#endif        
        else
        {
            BDBG_ERR(("Invalid Destiantion type %d", psNetworkInfo->sDspOutConfig[i].hDestHandle->sExtDstDetails.eAudioDst));
            ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
        }  
    }

    if(true == bDac0Present)
    {
        if(true == bOtherOpPresent)
        {
            pCitInputStruct->sPortCfg[uiDac0OpIndex].ui32PllIndex = uiTempPllIndex;
        }
        else
        {
            pCitInputStruct->sPortCfg[uiDac0OpIndex].ui32PllIndex = 0;
#if (BRAP_7405_FAMILY == 1) ||  (BRAP_3548_FAMILY == 1)
            /* For Rate Manager select None */
            pCitInputStruct->sPortCfg[uiDac0OpIndex].ePllNcoSelect =  BRAP_AF_P_NoneSelect;
#endif              
        }
    }
    
    if(true == bDac1Present)
    {
        if(true == bOtherOpPresent)
        {
            pCitInputStruct->sPortCfg[uiDac1OpIndex].ui32PllIndex = uiTempPllIndex;
        }
        else
        {
            pCitInputStruct->sPortCfg[uiDac1OpIndex].ui32PllIndex = 0;
#if (BRAP_7405_FAMILY == 1) ||  (BRAP_3548_FAMILY == 1)          
            pCitInputStruct->sPortCfg[uiDac1OpIndex].ePllNcoSelect =  BRAP_AF_P_NoneSelect;
#endif              
        }        
    }

    if(true == bDac2Present)
    {
        if(true == bOtherOpPresent)
        {
            pCitInputStruct->sPortCfg[uiDac2OpIndex].ui32PllIndex = uiTempPllIndex;
        }
        else
        {
            pCitInputStruct->sPortCfg[uiDac2OpIndex].ui32PllIndex = 0;
#if (BRAP_7405_FAMILY == 1) ||  (BRAP_3548_FAMILY == 1)          
            pCitInputStruct->sPortCfg[uiDac2OpIndex].ePllNcoSelect =  BRAP_AF_P_NoneSelect;
#endif              
        }        
    }

    if(true == bRbufDstnPresent)
    {
        if(true == bOtherOpPresent)
        {
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].ui32PllIndex = uiTempPllIndex;
        }
        else
        {
            pCitInputStruct->sPortCfg[uiRbufDstnIndex].ui32PllIndex = BRAP_OP_Pll_ePll0;
        }    
    }
    }
#ifdef RAP_MS11_SUPPORT    
    if(BRAP_FWIF_P_IsDDREPPAdded(psAudProcNetwork))
    {
        if(BRAP_FWIF_P_CheckDDREUsageModeValidity(hRap,psAudProcNetwork,&uiDDREStageId))
        {
                    i=0;
            ui32BranchId=0;
            ui32StageId = 0;            
            for(j=0;j<=uiDDREStageId;j++)
            {
                if(true == psAudProcNetwork->sAudProcessingStage[i][j].bDecoderStage)
                {
                    BDBG_MSG(("This is a decoder stage"));
                    bStageFound =true;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages++;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                                    BRAP_CIT_P_AudioAlgoType_eDecode;

                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo = 
                                                    psNetworkInfo->eAudioType;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eDecAudioMode = 
                                                    psNetworkInfo->eDecodeMode;
                    /* Stage Connectivity */
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

                    switch (psNetworkInfo->eNetworkInputType)
                    {
                        case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
                        case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                            break;
                        case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
                        default:
                            /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                            BDBG_ASSERT(0);
                    }

                    uiDecoderStageId = ui32StageId;
                    uiDecoderBranchId = ui32BranchId;         
                    
                }
                else if(NULL != psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing)                
                {
                    BDBG_MSG(("This is a processing stage"));
                    hTempHandle = psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing;

                    bStageFound =true;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages++;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

                    if(BRAP_CIT_P_AudioAlgoType_eEncode == BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                    {
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                            BRAP_CIT_P_AudioAlgoType_eEncode;
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo = 
                                            BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eEncodeType;

                        /*TODO: Remove hardcoding of encoder audio mode. Should come through proper input parameter to this function */
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eEncAudioMode = 
                                            BRAP_DSPCHN_EncodeMode_eRealtime;
                    }
                    else if(BRAP_CIT_P_AudioAlgoType_ePostProc== BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                    {
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                            BRAP_CIT_P_AudioAlgoType_ePostProc;
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo = 
                                            BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eProcessingType;
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eProcessingAudioMode = 
                                            BRAP_DSPCHN_ProcessingMode_ePP;
                    }

                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
                    
                    /* If there is no decoder stage and PP stage is first Stage in Network */
                    if((0 == ui32BranchId) && (0 == ui32StageId))
                    {
                        /* Stage Connectivity */
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

                        switch (psNetworkInfo->eNetworkInputType)
                        {
                            case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
                            case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                            case BRAP_CIT_P_FwStgSrcDstType_eDRAMBuf:    
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                                break;
                            case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
                            default:
                                /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                                BDBG_ASSERT(0);
                        }
                    }
                    else
                    {
                        /* The Input to this stage is a stage. Update source of this
                           stage and destination of previous stage */
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;

                        BDBG_MSG(("LINE > %d, ui32BranchId > %d, ui32StageId > %d",__LINE__,ui32BranchId,ui32StageId));
                        if((psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitBranchId != BRAP_INVALID_VALUE)
                            &&(psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitStageId != BRAP_INVALID_VALUE))                        
                        {
                            /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                            while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/
                            ui32MasterBranchId = i;
                            ui32MasterStageId = j - 1;                        
                            ui32SrcBranchId = ui32BranchId;
                            ui32SrcStageId = (ui32StageId -1);
                        }
                        else
                        {
                            /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                            while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/                    
                            ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterBranchId;
                            ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterStageId;
                            
                            ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                            ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;
                        }
                        BDBG_MSG(("LINE > %d, ui32SrcBranchId > %d, ui32SrcStageId > %d",__LINE__,ui32SrcBranchId,ui32SrcStageId));
                        
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId = ui32SrcBranchId;
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId = ui32SrcStageId;                          


                        uiDistinctOut =0;
                        uiNumDst =0;
                        eDistinctOutType=BRAP_AF_P_DistinctOpType_e5_1_PCM;

                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp = 1;                        
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]= 1;    
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                             
                        
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32OpNum = uiNumDst;

                        /* Populate previous stage dst info */                    


                                                                                          
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32BranchId = ui32BranchId;
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32StageId = ui32StageId;
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32OpNum = uiNumDst;                    
                        BDBG_MSG(("<=====>tDetails[%d][%d].eType =%d",uiDistinctOut,uiNumDst,pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType));                    
                    }                    
                }
                else
                {
                    BDBG_ERR(("Error!!! Invalid Usage Mode"));
                    BDBG_ASSERT(0);
                }
                if(true == bStageFound)
                {            
                    psAudProcNetwork->sAudProcessingStage[i][j].ui32CitBranchId = ui32BranchId;
                    psAudProcNetwork->sAudProcessingStage[i][j].ui32CitStageId = ui32StageId;                                            
                    ui32StageId++;
                }
            }
#if 0            
            ui32BranchId++;
            pCitInputStruct->ui32NumBranches++;            
#endif            
        }
        else
        {
            ret = BERR_TRACE(BERR_INVALID_PARAMETER);
            BDBG_ERR(("INVALID MS11 Usage Case. All the destination should have foollowing PP Config"));
            BDBG_ERR(("Decoder--->PP1--->PP2--->PP3---->DDRE----> where PP1,PP2 and PP2 is optional"));
            goto error;
        }
        uiStartBranchIndex =0;
        uiStartStageIndex =uiDDREStageId+1;               
    }
    else
#endif        
    {
        uiStartBranchIndex =0;
        uiStartStageIndex =0;        
    }
    {
        i = uiStartBranchIndex;
        for(;i<BRAP_P_MAX_DST_PER_RAPCH;i++)
    {
        bCompleteBranchCloned = false;
        bIsFwBranch = true;

        if(BRAP_P_BRANCH_VALID(psAudProcNetwork->sAudProcessingStage,i))
        {
            if((psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle != NULL)
               &&(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0] == BRAP_OutputPort_eMai)
               &&(hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector != BRAP_OutputPort_eMai))
            {
                continue;
            }
            pCitInputStruct->ui32NumBranches++;
        }
	    else
		    break;
            if((ui32BranchId ==0) &&(uiDDREStageId!=BRAP_INVALID_VALUE))
            {
                ui32StageId =uiDDREStageId + 1;
            }
            else
            {
                ui32StageId = 0;
            }
            j = uiStartStageIndex;
            for(;j<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;j++)
        {
            bStageFound = false;
            bFlag  = false;
            uiDistinctOut = 0;   
            BDBG_MSG(("\n\n\n__LINE__ =%d , i =%d , j =%d",__LINE__,i,j));
            if(true == psAudProcNetwork->sAudProcessingStage[i][j].bDecoderStage)
            {
                BDBG_MSG(("This is a decoder stage"));
                bStageFound =true;
                pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages++;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                                BRAP_CIT_P_AudioAlgoType_eDecode;

                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo = 
                                                psNetworkInfo->eAudioType;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eDecAudioMode = 
                                                psNetworkInfo->eDecodeMode;
                /* Stage Connectivity */
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

                switch (psNetworkInfo->eNetworkInputType)
                {
                    case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
                    case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                    case BRAP_CIT_P_FwStgSrcDstType_eDRAMBuf:    
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                        break;
                    case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
                    default:
                        /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                        BDBG_ASSERT(0);
                }
                
                    uiDecoderStageId = ui32StageId;
                    uiDecoderBranchId = ui32BranchId;                
                    
            }
            else if(NULL != psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing)
            {
                BDBG_MSG(("This is a processing stage"));
                hTempHandle = psAudProcNetwork->sAudProcessingStage[i][j].hAudioProcessing;

                bStageFound =true;
                pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages++;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

                if(BRAP_CIT_P_AudioAlgoType_eEncode == BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                {
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                        BRAP_CIT_P_AudioAlgoType_eEncode;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo = 
                                        BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eEncodeType;

                    /*TODO: Remove hardcoding of encoder audio mode. Should come through proper input parameter to this function */
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eEncAudioMode = 
                                        BRAP_DSPCHN_EncodeMode_eRealtime;
                }
                else if(BRAP_CIT_P_AudioAlgoType_ePostProc== BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                {
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                        BRAP_CIT_P_AudioAlgoType_ePostProc;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo = 
                                        BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eProcessingType;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eProcessingAudioMode = 
                                        BRAP_DSPCHN_ProcessingMode_ePP;
                }

                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
                
                /* If there is no decoder stage and PP stage is first Stage in Network */
                if((0 == ui32BranchId) && (0 == ui32StageId))
                {
                    /* Stage Connectivity */
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

                    switch (psNetworkInfo->eNetworkInputType)
                    {
                        case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
                        case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                        case BRAP_CIT_P_FwStgSrcDstType_eDRAMBuf:                            
                            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                            break;
                        case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
                        default:
                            /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                            BDBG_ASSERT(0);
                    }
                }
                else
                {
                    /* The Input to this stage is a stage. Update source of this
                       stage and destination of previous stage */
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;

                    BDBG_MSG(("LINE > %d, ui32BranchId > %d, ui32StageId > %d",__LINE__,ui32BranchId,ui32StageId));
                    if((psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitBranchId != BRAP_INVALID_VALUE)
                        &&(psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitStageId != BRAP_INVALID_VALUE))                        
                    {
                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/
                        ui32MasterBranchId = i;
                        ui32MasterStageId = j - 1;                        
                        ui32SrcBranchId = ui32BranchId;
                        ui32SrcStageId = (ui32StageId -1);
                    }
                    else
                    {
                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/                    
                        ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterBranchId;
                        ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterStageId;
                        
                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;
                    }
                    BDBG_MSG(("LINE > %d, ui32SrcBranchId > %d, ui32SrcStageId > %d",__LINE__,ui32SrcBranchId,ui32SrcStageId));
                    
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId = ui32SrcBranchId;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId = ui32SrcStageId;

                    /* Update the DST info for SRC Branch ID and SRC Stage Id */
                    /*TODO : Populate this param based on Concurrent Downmix Enable mode similarly as DDConvert is implemented here*/
                    if((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                        &&((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                            ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                            ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                    {                           
                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                        {
                            if(BRAP_AF_P_DistinctOpType_eAuxilaryDataOut== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                            {
                                bFlag =true;
                                break;
                            }
                        }
                        if(bFlag == true)
                        {
                            uiDistinctOut = k;
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                            BDBG_MSG(("---> 0.5 Incrmenting ui32NumDistinctOp(%d) for [%d][%d] eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,BRAP_AF_P_DistinctOpType_eAuxilaryDataOut));                            
                        }
                        else
                        {
                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                            BDBG_MSG(("---> 1 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d ",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId, BRAP_AF_P_DistinctOpType_eAuxilaryDataOut));
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eAuxilaryDataOut;                                                           
                        }
                    }
                        else if((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                            &&(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo ==BRAP_CIT_P_EncAudioType_eAc3)
                             &&(
                             /*If Previous stage is dolby pulse*/
                             ((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType==BRAP_CIT_P_AudioAlgoType_eDecode)
                             &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacSbrAdts)
                                ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacSbrLoas)
                                ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacAdts)
                                ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacLoas)))
                             ||
                             /*If prev stage is DDRE*/
                             ((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType==BRAP_CIT_P_AudioAlgoType_ePostProc)
                             &&(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDdre))))
                        {                           
                            for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                            {
                                if(BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                {
                                    bFlag =true;
                                    break;
                                }
                            }
                            if(bFlag == true)
                            {
                                uiDistinctOut = k;
                                uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                BDBG_MSG(("---> 0.5.1 Incrmenting ui32NumDistinctOp(%d) for [%d][%d] eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut));                            
                            }
                            else
                            {
                                uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                BDBG_MSG(("---> 1.1 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d ",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId, BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut));
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                                uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut;                                                           
                            }
                        }                    
                    else
                    {
                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            if((psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle != NULL) &&
                               (psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort))
                            {
                                if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }
                                else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                }
                                else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                }
#if (1 == LFE_WORKAROUND)
                                else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                }
#endif							
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }
                            }
                            else
                            {
                                if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                }
                                else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                }
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }
                            }
                        }
                        else
                        {
                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                        }                        
    
                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                        {
                            if(eDistinctOutType== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                            {
                                bFlag =true;
                                break;
                            }
                        }
                        if(bFlag == true)
                        {
                            uiDistinctOut = k;
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                            BDBG_MSG(("---> 1.5 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d",k,ui32SrcBranchId,ui32SrcStageId,eDistinctOutType));                                                        
                        }
                        else
                        {
                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                            BDBG_MSG(("---> 2 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,eDistinctOutType));                            
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;                                                                                                 
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                                                       
                        }
                    }  

                    /* Populate current stage src info */
                    /* Distinct OP no. and Destination no of that distinct output (its not their index , their index will be 1 less than each value)*/
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32OpNum = uiNumDst;


                    /* Populate previous stage dst info */                    
          
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32BranchId = ui32BranchId;
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32StageId = ui32StageId;
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32OpNum = uiNumDst;                    
                    BDBG_MSG(("<=====>tDetails[%d][%d].eType =%d",uiDistinctOut,uiNumDst,pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType));                    
                }
                
            }
            else if(true == psAudProcNetwork->sAudProcessingStage[i][j].bCloneStage) /* ie If its  Clone  stage. */ 
            {
                
                /* [TODO] If its clone stage in the branch which is feeding to Downmix stereo o/p. then 
                corresponding stage should be added in fw stage because this will be different from Main path*/
                ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j].ui32MasterBranchId;
                ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j].ui32MasterStageId;
                
                BDBG_MSG(("This is a clone stage ui32MasterBranchId =%d ui32MasterStageId =%d",ui32MasterBranchId,ui32MasterStageId));            
                
                ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;
                if((psAudProcNetwork->sAudProcessingStage[i][j].hValidDestHandle != NULL) &&
                   (psAudProcNetwork->sAudProcessingStage[i][j].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort))
                {
                    BDBG_MSG(("===> eChannelOutputMode  =%d, bDownmixedOpPort1 =%d bDownmixedOpPort-Master =%d , i =%d , j = %d",
                        eChannelOutputMode
                        ,psAudProcNetwork->sAudProcessingStage[i][j].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                        , psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32SrcBranchId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                        ,i,j));
                if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].bDecoderStage != true)/* If its clone of decoder stage, Do nothing*/
                {
                        bCloned = false;
                        k = ui32MasterBranchId;
                        for(; k<i; ++k)
                        {                     
                            if((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                || (psAudProcNetwork->sAudProcessingStage[i][j].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort ==
                                psAudProcNetwork->sAudProcessingStage[k][ui32MasterStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort )
                            )
                                {
                                    bCloned =true;
                                    break;
                                }
                        }
                         if(bCloned == false)
                    {
                        bCloned = false;
                        /*Add this Clone stage as well. First check if similar stage had been added in previous branches.*/
                        for(k=0; k<BRAP_P_MAX_DST_PER_RAPCH; k++)
                        {
                            uiCloneBranchId[k] = BRAP_INVALID_VALUE;
                        }

                        k = ui32MasterBranchId;
                        l = ui32MasterStageId;
                        m = 0;
                        if(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1)))
                        {
                        uiCloneBranchId[m]=k;
                        m++;
                        k++;
                        }

                        /* Now check if the branches between Master branch and current branch is similar to current  branch.
                        This is done by checking if the last stage is cloned, and its Master stage/branch id 
                        is same as that of last stage id of current branch. Just to make clear, here 
                        Current Branch/Stage Id is : i/uiLastStage
                        Master Branch/Stage Id is : ui32MasterBranchId/ui32MasterStageId
                        And we looping on K, which is between ui32MasterBranchId and i*/   
                        
                        for(; k<i; ++k)
                        {   
                            if((true == psAudProcNetwork->sAudProcessingStage[k][l].bCloneStage)
                                &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterBranchId == ui32MasterBranchId)
                                &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterStageId == ui32MasterStageId)
                                &&(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1))))
                            {
                                uiCloneBranchId[m]=k;
                                m++;
                            }
                        }
                        for(m=0; (m<BRAP_P_MAX_DST_PER_RAPCH)   ; ++m)
                        {
                            if(uiCloneBranchId[m] == BRAP_INVALID_VALUE)
                                break;
                            
                            k = uiCloneBranchId[m];
                            BDBG_MSG(("####eChannelOutputMode, =%d k = %d , l =%d, bDownmixedOpPort",eChannelOutputMode,k,l,
                                psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort));
                            /* For cloned branch */
                                if((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                    ||(psAudProcNetwork->sAudProcessingStage[i][j].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                                psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort))
                            {
                                bCloned = true;
                                        break;
                            }
                            else if (eChannelOutputMode > BRAP_OutputMode_e2_0)
                            {
                                bCloned = false;
                            }
                            else
                            {
                                bCloned = true;
                            }
                        }      

                        BDBG_MSG(("===> bCloned = %d",bCloned));
                        if(bCloned == false) /*Add this stage now */
                        {
                            {
                                hTempHandle = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing;
                                BDBG_MSG(("ui32MasterBranchId =%d, ui32MasterStageId = %d , hTempHandle =%#x",ui32MasterBranchId,ui32MasterStageId,hTempHandle));
                                bStageFound =true;
                                pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages++;
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

                                if(BRAP_CIT_P_AudioAlgoType_eEncode == BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                                {
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                                        BRAP_CIT_P_AudioAlgoType_eEncode;
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo = 
                                                        BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eEncodeType;

                                    /*TODO: Remove hardcoding of encoder audio mode. Should come through proper input parameter to this function */
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eEncAudioMode = 
                                                        BRAP_DSPCHN_EncodeMode_eRealtime;
                                }
                                else if(BRAP_CIT_P_AudioAlgoType_ePostProc== BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].eEnumType)
                                {
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                                        BRAP_CIT_P_AudioAlgoType_ePostProc;
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo = 
                                                        BRAP_sMapProcessingEnum[hTempHandle->sProcessingStageSettings.eAudioProcessing].uEnumName.eProcessingType;
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAudioMode.eProcessingAudioMode = 
                                                        BRAP_DSPCHN_ProcessingMode_ePP;
                                }

                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
                                
                                /* If there is no decoder stage and PP stage is first Stage in Network */
                                if((0 == ui32BranchId) && (0 == ui32StageId))
                                {
                                    /* Stage Connectivity */
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

                                    switch (psNetworkInfo->eNetworkInputType)
                                    {
                                        case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
                                        case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                                            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                                            break;
                                        case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
                                        default:
                                            /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                                            BDBG_ASSERT(0);
                                    }
                                }
                                else
                                {
                                    /* The Input to this stage is a stage. Update source of this
                                       stage and destination of previous stage */
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;

                                    BDBG_MSG(("LINE > %d, ui32BranchId > %d, ui32StageId > %d",__LINE__,ui32BranchId,ui32StageId));
                                    if((psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitBranchId != BRAP_INVALID_VALUE)
                                        &&(psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitStageId != BRAP_INVALID_VALUE))
                                    {
                                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/
                                        ui32MasterBranchId = i;
                                        ui32MasterStageId = j - 1;                        
                                        ui32SrcBranchId = ui32BranchId;
                                        ui32SrcStageId = (ui32StageId -1);
                                    }
                                    else
                                    {
                                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/                    
                                        ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterBranchId;
                                        ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterStageId;
                                        
                                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;
                                    }
                                    BDBG_MSG(("LINE > %d, ui32SrcBranchId > %d, ui32SrcStageId > %d",__LINE__,ui32SrcBranchId,ui32SrcStageId));
                                    
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId = ui32SrcBranchId;
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId = ui32SrcStageId;

                                    /* Update the DST info for SRC Branch ID and SRC Stage Id */
                                    /*TODO : Populate this param based on Concurrent Downmix Enable mode similarly as DDConvert is implemented here*/
                                    if((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                                         &&((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                                            ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                                            ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                                    {                           
                                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                                        {
                                            if(BRAP_AF_P_DistinctOpType_eAuxilaryDataOut== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                            {
                                                bFlag =true;
                                                break;
                                            }
                                        }
                                        if(bFlag == true)
                                        {
                                            uiDistinctOut = k;
                                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                        }
                                        else
                                        {
                                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                            BDBG_MSG(("---> 3 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,BRAP_AF_P_DistinctOpType_eAuxilaryDataOut));                                            
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eAuxilaryDataOut;                                                           
                                        }
                                    }
                                        else if((pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                                            &&(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo ==BRAP_CIT_P_EncAudioType_eAc3)
                                             &&(
                                                 /*If Previous stage is dolby pulse*/
                                                 ((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType==BRAP_CIT_P_AudioAlgoType_eDecode)
                                                 &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacSbrAdts)
                                                    ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacSbrLoas)
                                                    ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacAdts)
                                                    ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eDecAudioAlgo==BRAP_DSPCHN_AudioType_eAacLoas)))
                                                 ||
                                                 /*If prev stage is DDRE*/
                                                 ((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType==BRAP_CIT_P_AudioAlgoType_ePostProc)
                                                 &&(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDdre))))
                                        {                           
                                            for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                                            {
                                                if(BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                                {
                                                    bFlag =true;
                                                    break;
                                                }
                                            }
                                            if(bFlag == true)
                                            {
                                                uiDistinctOut = k;
                                                uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                                BDBG_MSG(("---> 3.1 Incrmenting ui32NumDistinctOp(%d) for [%d][%d] eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut));                            
                                            }
                                            else
                                            {
                                                uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                                BDBG_MSG(("---> 3 Incrmenting ui32NumDistinctOp(%d) for [%d][%d], eDistinctOutType =%d ",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId, BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut));
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                                                uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut;                                                           
                                            }
                                        }                                             
                                    else
                                    {
                                        if(hDspCh->bChSpecificDspCh == true)
                                        {
                                            if((psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle != NULL) &&
                                               (psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true))
                                            {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                            }
                                            else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                            {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                            }
                                            else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                            {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                            }
#if (1 == LFE_WORKAROUND)											
                                            else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                            {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                            }
#endif											
                                            else
                                            {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                            }
                                        }
                                        else
                                        {
                                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                        }                        
                    
                                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                                        {
                                            if(eDistinctOutType== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                            {
                                                bFlag =true;
                                                break;
                                            }
                                        }
                                        if(bFlag == true)
                                        {
                                            uiDistinctOut = k;
                                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                        }
                                        else
                                        {
                                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                            BDBG_MSG(("---> 4 Incrmenting ui32NumDistinctOp(%d) for [%d][%d] eDistinctOutType =%d",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId,eDistinctOutType));                                            
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                              
                                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;                                                                                                 
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                                                       
                                        }
                                    }  

                                    /* Populate current stage src info */
                                    /* Distinct OP no. and Destination no of that distinct output (its not their index , their index will be 1 less than each value)*/
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32OpNum = uiNumDst;


                                    /* Populate previous stage dst info */                    
                          
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFwStg;
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32BranchId = ui32BranchId;
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32StageId = ui32StageId;
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32DistinctOpNum = uiDistinctOut;
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sFwStage.ui32OpNum = uiNumDst;                    
                                    BDBG_MSG(("<=====>tDetails[%d][%d].eType =%d",uiDistinctOut,uiNumDst,pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType));                    
                                }                                
                            }
                        }             
                    }
                }
            }
            }
            else if(false == psAudProcNetwork->sAudProcessingStage[i][j].bCloneStage) /* ie If its not decoder/Processing/Clone  stage.
                                                                                                                                        It means no more Processing stage in the branch */ 
            {
                BDBG_MSG(("no more Processing stage in the branch"));             
                /* No more processing stages in this branch.Update DSt details 
                   of last stage and Go to next branch */
                if(j > 1) /*If its not only the stage in the branch */
                {
                    BDBG_MSG(("LINE > %d, ui32BranchId > %d, ui32StageId > %d",__LINE__,ui32BranchId,ui32StageId));
                    if((psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitBranchId != BRAP_INVALID_VALUE)  /* Check if the previous stage is a true stage. i.e the stage also exist in Fw network*/
                        &&(psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitStageId != BRAP_INVALID_VALUE))                    
/*                    if(false == psAudProcNetwork->sAudProcessingStage[i][j - 1].bCloneStage)*/
                    {
                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/
                        ui32MasterBranchId = i;
                        ui32MasterStageId = j - 1;                        
                        ui32SrcBranchId = ui32BranchId;
                        ui32SrcStageId = (ui32StageId -1);
                    }
                    else
                    {
                        /* Master BranchId/StageId represent the position of master stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position of master in sBranchInfo[].sFwStgInfo[]*/                    
                        ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterBranchId;
                        ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterStageId;
                        
                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;

                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            BRAP_FWIF_P_IsBranchCloned(hRapCh,psAudProcNetwork,i,j-1,&bCompleteBranchCloned,&bIsFwBranch);
                        }
                        else
                        {
                        }
                    }
                    BDBG_MSG(("LINE > %d, ui32SrcBranchId > %d, ui32SrcStageId > %d bCompleteBranchCloned = %d , i =%d , j =%d ,bIsFwBranch =%d",
                        __LINE__,ui32SrcBranchId,ui32SrcStageId,bCompleteBranchCloned,i,j,bIsFwBranch));
                    
                    if((false == bCompleteBranchCloned))
                    { 
                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle != NULL)
                            {
                                if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                    if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#if (1 == LFE_WORKAROUND)							
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#endif							
                                    else
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }
                                }
                                else
                                {
                                        if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode == BRAP_OutputMode_e3_2)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                        }
                                        else
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                        }                                
                                }
                            }
                            else
                            {
                                if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                }
                                else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                }
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }                            
                            }
                        }
                        else
                        {
                            eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                        }

                    /* Update the DST info for SRC Branch ID and SRC Stage Id */
                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                        {
                            /* Only either  PCM/Compressed O/P type can feed to FMM Buf */
                            if((eDistinctOutType == pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                ||(BRAP_AF_P_DistinctOpType_eCompressed== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k]))
                            {
                                bFlag =true;
                                break;
                            }
                        }
                        if(bFlag == true)
                        {
                            uiDistinctOut = k;
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                        }
                        else
                        {
                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                            BDBG_MSG(("---> 5 Incrmenting ui32NumDistinctOp(%d) for [%d][%d]",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId));                            
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                                                          
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;      
                            
                            if(((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                                  &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                                        ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                            ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                            ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAudioMode.eDecAudioMode == BRAP_DSPCHN_DecodeMode_ePassThru))
                            {
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eCompressed;                                                           
                                BDBG_MSG(("-->5 eDistinctOutType = %d",BRAP_AF_P_DistinctOpType_eCompressed));
                            }
                            else
                            {
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                           
                                BDBG_MSG(("-->5 eDistinctOutType = %d",eDistinctOutType));                                
                            }                                                                                           
                        }

                    /*DSP output would go to FMM RBUF */
                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFMMBuf;                 

                        bBreakLoop = false;
                        /* Find out the FMM path for this branch */

                        if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle == NULL)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }
                        else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle == NULL)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }                        
                        else if((psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                            &&(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                        != psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)
                        && (eChannelOutputMode > BRAP_OutputMode_e2_0))
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                    if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                    {
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                        bBreakLoop = true;
                                    }
                                    if(true == bBreakLoop)
                                        break;
                            }

                        }
                        else if(psAudProcNetwork->sAudProcessingStage[i][j-1].bCloneStage == true)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }                        
                        else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing != NULL)
                        {
                            hTempHandle = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing;                    
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                {
                             	    BDBG_MSG(("hTempHandle->hDestHandle[%d] =%#x",l,hTempHandle->hDestHandle[l]));
                                    if(psNetworkInfo->sDspOutConfig[k].hDestHandle == hTempHandle->hDestHandle[l])
                                    {
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                        bBreakLoop = true;
                                    }
                                    if(true == bBreakLoop)
                                        break;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }
                        else
                        {
                            BDBG_ERR(("This stage (%d) can not be decode stage",j));
                            BDBG_ASSERT(0);
                        }
                    }
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
                    else
                    {
                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            for(k=0; k<BRAP_P_MAX_DST_PER_RAPCH; k++)
                            {
                                uiCloneBranchId[k] = BRAP_INVALID_VALUE;
                            }

                            k = ui32MasterBranchId;
                            l = ui32MasterStageId;
                            m = 0;
                           if((!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1)))
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort) 
                            && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                &&((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                || (psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                                psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)))
                            {
                                uiCloneBranchId[m]=k;
                                m++;
                                k++;
                            }                            

                            /* Now check if the branches between Master branch and current branch is similar to current  branch.
                            This is done by checking if the last stage is cloned, and its Master stage/branch id 
                            is same as that of last stage id of current branch. Just to make clear, here 
                            Current Branch/Stage Id is : i/uiLastStage
                            Master Branch/Stage Id is : ui32MasterBranchId/ui32MasterStageId
                            And we looping on K, which is between ui32MasterBranchId and i*/   
                            
                            for(; k<i; ++k)
                            {   
                                BDBG_MSG(("k = %d, bDownmixedOpPort = %d , bDownmixedOpPort-Master = %d",
                                    k,psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort,
                                    psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort));
                                if((true == psAudProcNetwork->sAudProcessingStage[k][l].bCloneStage)
                                    &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterBranchId == ui32MasterBranchId)
                                    &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterStageId == ui32MasterStageId)
                                    &&(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1)))
                                    &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort) 
                                    && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        && ((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                                ||(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                                        psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)))
                                {
                                    uiCloneBranchId[m]=k;
                                    m++;
                                }
                            }
                            for(m=0; (m<BRAP_P_MAX_DST_PER_RAPCH)   ; ++m)
                            {
                                if(uiCloneBranchId[m] == BRAP_INVALID_VALUE)
                                    continue;
                                else
                                {
                                    k = uiCloneBranchId[m];  
                                    /* This independent delay path will take data from last stage of its cloned branch*/
                                    if((psAudProcNetwork->sAudProcessingStage[k][l].ui32CitBranchId != BRAP_INVALID_VALUE)
                                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32CitStageId != BRAP_INVALID_VALUE))
                                    {
                                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[k][l].ui32CitBranchId;
                                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[k][l].ui32CitStageId;
                                        break;
                                    }
                                }
                                
                            }   
                            
                            if(hRapCh->bIndepDelayEnabled == true)
                            {
#if (BRAP_DYNAMIC_PORT_SWITCHING != 1)
                                bCloned = false;
#else
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1) 
                                BRAP_OutputPort         eOutput = BRAP_OutputPort_eMax; 
#endif
                                /*If the Destination has Indepdelay Enabled then create similar path to its cloned.*/
                                eOutput = hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0];
                                /* Dummy ports act as slave to Master port. */
                                if(hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                switch(eOutput)
                                {
                                    case BRAP_OutputPort_eDac1:
                                    case BRAP_OutputPort_eI2s1:
                                        bCloned = true;
                                        break;
                                    default:
                                        bCloned = false;
                                        break;
                                }
                                }
                                else if(hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.eAudioDst == BRAP_AudioDst_eRingBuffer)
                                {
                                    bCloned = true;
                                }
#endif                                
                            }                     
                            else
                            {
                                bCloned = true;                                            
                            }                               
                            if(false == bCloned)
                            {
                                if(hDspCh->bChSpecificDspCh == true)
                                {                            
                                if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                    if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#if (1 == LFE_WORKAROUND)								
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#endif								
                                    else
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }                        
                                }
                                else
                                {
                                    if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode == BRAP_OutputMode_e3_2)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
                                    else
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }                                
                                    }
                                }
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;                                
                                }                        

                                if(hRapCh->bIndepDelayEnabled == true)
                                {  

                                /* Update the DST info for SRC Branch ID and SRC Stage Id */
                                    for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                                    {
                                        /* Only either  PCM/Compressed O/P type can feed to FMM Buf */
                                        if((eDistinctOutType == pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                            ||(BRAP_AF_P_DistinctOpType_eCompressed== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k]))
                                        {
                                            bFlag =true;
                                            break;
                                        }
                                    }
                                    if(bFlag == true)
                                    {
                                        uiDistinctOut = k;
                                        uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                    }
                                    else
                                    {
                                        uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                        BDBG_MSG(("---> 6 Incrmenting ui32NumDistinctOp(%d) for [%d][%d]",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId));                                        
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                                                          
                                        uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;      
                                        
                                        if(((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                                              &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                                                    ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                                                    ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAudioMode.eDecAudioMode == BRAP_DSPCHN_DecodeMode_ePassThru))
                                        {
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eCompressed;                                                           
                                            BDBG_MSG(("-->6 eDistinctOutType = %d",BRAP_AF_P_DistinctOpType_eCompressed));                                            
                                            
                                        }
                                        else
                                        {
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                           
                                            BDBG_MSG(("-->6 eDistinctOutType = %d",eDistinctOutType));                                            
                                        }                                                                                           
                                    }

                                /*DSP output would go to FMM RBUF */
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFMMBuf;                 

                                    bBreakLoop = false;
                                    /* Find out the FMM path for this branch */
                                    if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle == NULL)
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                            {
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                bBreakLoop = true;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }
                                    else if((psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        &&(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                                    != psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)
                                    && (eChannelOutputMode > BRAP_OutputMode_e2_0) )
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                                {
                                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                    bBreakLoop = true;
                                                }
                                                if(true == bBreakLoop)
                                                    break;
                                        }

                                    }
                                    else if(psAudProcNetwork->sAudProcessingStage[i][j-1].bCloneStage == true)
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle)
                                            {
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                bBreakLoop = true;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }                        
                                    else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing != NULL)
                                    {
                                        hTempHandle = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing;                    
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                            {
                                         	    BDBG_MSG(("hTempHandle->hDestHandle[%d] =%#x",l,hTempHandle->hDestHandle[l]));
                                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == hTempHandle->hDestHandle[l])
                                                {
                                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                    bBreakLoop = true;
                                                }
                                                if(true == bBreakLoop)
                                                    break;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }
                                    else
                                    {
                                        BDBG_ERR(("This stage (%d) can not be decode stage",j));
                                        BDBG_ASSERT(0);
                                    }
                                }
                            }
                        }
                    }
#endif			
                }
                else if (1 == j) /* branch has only decode stage */
                {
                    BDBG_MSG (("LINE > %d, ui32BranchId > %d, ui32StageId > %d",__LINE__,ui32BranchId,ui32StageId));

                    if((psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitBranchId != BRAP_INVALID_VALUE)/* Check if the previous stage is a true stage. i.e the sage also exist in Fw network*/
                        &&(psAudProcNetwork->sAudProcessingStage[i][j - 1].ui32CitStageId != BRAP_INVALID_VALUE))                    
/*                    if(false == psAudProcNetwork->sAudProcessingStage[i][j - 1].bCloneStage)*/
                    {
                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/
                        ui32MasterBranchId = i;
                        ui32MasterStageId = j - 1;                        
                        ui32SrcBranchId = ui32BranchId;
                        ui32SrcStageId = (ui32StageId -1);
                    }
                    else
                    {
                        /* Master BranchId/StageId represent the position of current stage in the sAudProcessingStage[][]. 
                        while Source BranchId/StageId points the position in sBranchInfo[].sFwStgInfo[]*/                    
                        ui32MasterBranchId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterBranchId;
                        ui32MasterStageId = psAudProcNetwork->sAudProcessingStage[i][j -1].ui32MasterStageId;
                        
                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitBranchId;
                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].ui32CitStageId;

                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            BRAP_FWIF_P_IsBranchCloned(hRapCh,psAudProcNetwork,i,j-1,&bCompleteBranchCloned,&bIsFwBranch);
                        }
                    }
                    BDBG_MSG (("LINE > %d, ui32SrcBranchId > %d, ui32SrcStageId > %d ,bCompleteBranchCloned =%d",__LINE__,ui32SrcBranchId,ui32SrcStageId,bCompleteBranchCloned));

                    if((false == bCompleteBranchCloned))
                    {

                        if(hDspCh->bChSpecificDspCh == true)
                        {
                            if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle != NULL)
                            {
                                if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                    if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                    }
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#if (1 == LFE_WORKAROUND)							
                                    else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
#endif							
                                    else
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }
                                }
                                else
                                {
                                    if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode == BRAP_OutputMode_e3_2)
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                    }
                                    else
                                    {
                                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                    }                                
                                }
                            }
                            else
                            {
                                if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                }
                                else if((hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2) ||
                                        (BRAP_P_IsMs11UsageMode(hRapCh)))
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                }
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }                            
                            }
                        }
                        else
                        {
                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                        }

                    /* Update the DST info for SRC Branch ID and SRC Stage Id */
                        for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                        {
                            /* Only either  PCM/Compressed O/P type can feed to FMM Buf */
                            if((eDistinctOutType == pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                ||(BRAP_AF_P_DistinctOpType_eCompressed== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k]))                         
                            {
                                bFlag =true;
                                break;
                            }
                        }
                        if(bFlag == true)
                        {
                            uiDistinctOut = k;
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                        }
                        else
                        {
                            uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                            BDBG_MSG(("---> 7 Incrmenting ui32NumDistinctOp(%d) for [%d][%d]",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId));                            
                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                                                          
                            uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                            
                            if(((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                                  &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                                        ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                            ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                            ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAudioMode.eDecAudioMode == BRAP_DSPCHN_DecodeMode_ePassThru))
                            {
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eCompressed;                                                           
                                BDBG_MSG(("-->7 eDistinctOutType = %d",BRAP_AF_P_DistinctOpType_eCompressed));                                            
                            }
                            else
                            {
                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                           
                                BDBG_MSG(("-->7 eDistinctOutType = %d",eDistinctOutType));                                         
                            }                                                                                      
                        }
                        
                    /*DSP output would go to FMM RBUF */
                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFMMBuf;
                    
                        bBreakLoop = false;
                        /* Find out the FMM path for this branch */
                        if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle == NULL)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }
                        if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle == NULL)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }                        
                        else if((psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                            &&(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                        != psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)
                        && (eChannelOutputMode > BRAP_OutputMode_e2_0))
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                    if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                    {
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                        bBreakLoop = true;
                                    }
                                    if(true == bBreakLoop)
                                        break;
                            }

                        }
                        else if(psAudProcNetwork->sAudProcessingStage[i][j-1].bCloneStage == true)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle)
                                {
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                    bBreakLoop = true;
                                }
                                if(true == bBreakLoop)
                                    break;
                            }
                        }                        
                        else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].bDecoderStage == true)
                        {
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                {
                             	    BDBG_MSG(("hTempHandle->hDestHandle =%#x",psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hDestHandle[l]));
                                    if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hDestHandle[l])
                                    {
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                        bBreakLoop = true;
                                    }
                                    if(true == bBreakLoop)
                                        break;
                                }
                                if(true == bBreakLoop)
                                    break;                        
                            }
                        }
                        else if (psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing != NULL)
                        {
                            hTempHandle = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing;                        
                            for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                            {
                                for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                {
                                    BDBG_MSG(("hTempHandle->hDestHandle =%#x",hTempHandle->hDestHandle[l]));
                                    if(psNetworkInfo->sDspOutConfig[k].hDestHandle == hTempHandle->hDestHandle[l])
                                    {
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                        bBreakLoop = true;
                                    }
                                    if(true == bBreakLoop)
                                        break;
                                }
                                if(true == bBreakLoop)
                                    break;                        
                            }
                        }
                        else
                        {
                            BDBG_ERR(("Invalid stage (%d) ",j));
                            BDBG_ASSERT(0);
                    }                                            
                    }                                            
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
                    else
                    {
                        if(hDspCh->bChSpecificDspCh == true)
                        {                    
                            for(k=0; k<BRAP_P_MAX_DST_PER_RAPCH; k++)
                            {
                                uiCloneBranchId[k] = BRAP_INVALID_VALUE;
                            }

                            k = ui32MasterBranchId;
                            l = ui32MasterStageId;
                            m = 0;
                           if(((!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1)))
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort) 
                            && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                && ((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                        ||(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                                psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)))
                            ||
                            ((!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1)))
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eRingBuffer) 
                            && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                            && (psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode== 
                            psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eAudioMode))
                            ||
                            ((!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,ui32MasterBranchId,ui32MasterStageId +1)))
                            &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort) 
                            && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eRingBuffer)
                            && (psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eAudioMode== 
                            psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode)))
                            {
                                uiCloneBranchId[m]=k;
                                m++;
                                k++;
                            }                            

                            /* Now check if the branches between Master branch and current branch is similar to current  branch.
                            This is done by checking if the last stage is cloned, and its Master stage/branch id 
                            is same as that of last stage id of current branch. Just to make clear, here 
                            Current Branch/Stage Id is : i/uiLastStage
                            Master Branch/Stage Id is : ui32MasterBranchId/ui32MasterStageId
                            And we looping on K, which is between ui32MasterBranchId and i*/   
                            
                            for(; k<i; ++k)
                            {   
                                if((true == psAudProcNetwork->sAudProcessingStage[k][l].bCloneStage)
                                    &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterBranchId == ui32MasterBranchId)
                                    &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32MasterStageId == ui32MasterStageId)
                                    &&(!(BRAP_P_STAGE_VALID(psAudProcNetwork->sAudProcessingStage,k,l+1)))
                                    &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort) 
                                    && (psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        && ((eChannelOutputMode == BRAP_OutputMode_e2_0)
                                        ||(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == 
                                        psAudProcNetwork->sAudProcessingStage[k][l].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)))
                                {
                                    uiCloneBranchId[m]=k;
                                    m++;
                                }
                            }
                            for(m=0; (m<BRAP_P_MAX_DST_PER_RAPCH)   ; ++m)
                            {
                                if(uiCloneBranchId[m] == BRAP_INVALID_VALUE)
                                    continue;
                                else
                                {
                                    k = uiCloneBranchId[m];  
                                    /* This independent delay path will take data from last stage of its cloned branch*/
                                    if((psAudProcNetwork->sAudProcessingStage[k][l].ui32CitBranchId != BRAP_INVALID_VALUE)
                                        &&(psAudProcNetwork->sAudProcessingStage[k][l].ui32CitStageId != BRAP_INVALID_VALUE))
                                    {
                                        ui32SrcBranchId = psAudProcNetwork->sAudProcessingStage[k][l].ui32CitBranchId;
                                        ui32SrcStageId = psAudProcNetwork->sAudProcessingStage[k][l].ui32CitStageId;
                                        break;
                                    }
                                }
                                
                            }      

                            if(hRapCh->bIndepDelayEnabled == true)
                            {
#if (BRAP_DYNAMIC_PORT_SWITCHING != 1)
                                bCloned = false;
#else
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1) 
                                BRAP_OutputPort         eOutput = BRAP_OutputPort_eMax; 
#endif
                                /*If the Destination has Indepdelay Enabled then create similar path to its cloned.*/
                                eOutput = hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0];
                                /* Dummy ports act as slave to Master port. */
                                if(hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                switch(eOutput)
                                {
                                    case BRAP_OutputPort_eDac1:
                                    case BRAP_OutputPort_eI2s1:
                                        bCloned = true;
                                        break;
                                    default:
                                        bCloned = false;
                                        break;
                                    }                                
                                }
                                else if(hRapCh->sChAudProcessingStage[i][j-1].hValidDestHandle ->sExtDstDetails.eAudioDst == BRAP_AudioDst_eRingBuffer)
                                {
                                    bCloned = true;
                                }
#endif
                            }                     
                            else
                            {
                                bCloned = true;                                            
                            }                               
                            if(false == bCloned)
                            {
                                if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                {
                                    if(hDspCh->bChSpecificDspCh == true)
                                    {                                
                                        if(psAudProcNetwork->sAudProcessingStage[i][0].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort == true)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                        }
                                        else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_4)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_e7_1_PCM;
                                        }
                                        else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e3_2)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                        }
#if (1 == LFE_WORKAROUND)								
                                        else if(hRapCh->eChannelOutputMode == BRAP_OutputMode_e2_0)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                        }
#endif								
                                        else
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                        }
                                    }
                                    else
                                    {
                                        if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sRBufDetails.eAudioMode == BRAP_OutputMode_e3_2)
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                                        }
                                        else
                                        {
                                            eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                        }                                
                                    }
                                }
                                else
                                {
                                    eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                                }
                                
                                if(hRapCh->bIndepDelayEnabled == true)
                                {                            
                                /* Update the DST info for SRC Branch ID and SRC Stage Id */
                                    for(k = 0 ;k <pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp; k++)
                                    {
                                        /* Only either  PCM/Compressed O/P type can feed to FMM Buf */
                                        if((eDistinctOutType == pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k])
                                            ||(BRAP_AF_P_DistinctOpType_eCompressed== pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[k]))                         
                                        {
                                            bFlag =true;
                                            break;
                                        }
                                    }
                                    if(bFlag == true)
                                    {
                                        uiDistinctOut = k;
                                        uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[k]++;         
                                    }
                                    else
                                    {
                                        uiDistinctOut = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDistinctOp++;         
                                        BDBG_MSG(("---> 8 Incrmenting ui32NumDistinctOp(%d) for [%d][%d]",uiDistinctOut,ui32SrcBranchId,ui32SrcStageId));                                        
                                        pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut] = 0;                                                          
                                        uiNumDst = pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.ui32NumDst[uiDistinctOut]++;           
                                        
                                        if(((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
                                              &&((pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eDDConvert)
                                                    ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eWMAPassThrough)
                                                    ||(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo ==BRAP_CIT_P_ProcessingType_eGenericPassThru)))
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
                                        ||(pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].uAudioMode.eDecAudioMode == BRAP_DSPCHN_DecodeMode_ePassThru))
                                        {
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = BRAP_AF_P_DistinctOpType_eCompressed;                                                           
                                            BDBG_MSG(("-->8 eDistinctOutType = %d",BRAP_AF_P_DistinctOpType_eCompressed));                                                           
                                        }
                                        else
                                        {
                                            pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut] = eDistinctOutType;                                                           
                                            BDBG_MSG(("-->8 eDistinctOutType = %d",eDistinctOutType));                                                           
                                        }                                                                                      
                                    }
                                    
                                /*DSP output would go to FMM RBUF */
                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].eType = BRAP_CIT_P_FwStgSrcDstType_eFMMBuf;
                                
                                    bBreakLoop = false;
                                    /* Find out the FMM path for this branch */
                                    if(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle == NULL)
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                            {
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                bBreakLoop = true;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }  
                                    else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle == NULL)
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle)
                                            {
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                bBreakLoop = true;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }                                     
                                    else if((psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        &&(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.eAudioDst == BRAP_AudioDst_eOutputPort)
                                        &&(psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort
                                    != psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort)
                                    && (eChannelOutputMode > BRAP_OutputMode_e2_0))
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle)
                                                {
                                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                    bBreakLoop = true;
                                                }
                                                if(true == bBreakLoop)
                                                    break;
                                        }

                                    }
                                    else if(psAudProcNetwork->sAudProcessingStage[i][j-1].bCloneStage == true)
                                    {
                                        BDBG_MSG(("psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle  =%#x",psAudProcNetwork->sAudProcessingStage[i][j -1].hValidDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));                                                    
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[i][j-1].hValidDestHandle)
                                            {
                                            BDBG_MSG(("psNetworkInfo->sDspOutConfig[k].hDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]  =%#x",psNetworkInfo->sDspOutConfig[k].hDestHandle->sExtDstDetails.uDstDetails.sOpDetails.eOutput[0]));                            
                                                pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                bBreakLoop = true;
                                            }
                                            if(true == bBreakLoop)
                                                break;
                                        }
                                    }                        
                                    else if(psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].bDecoderStage == true)
                                    {
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                            {
                                         	    BDBG_MSG(("hTempHandle->hDestHandle =%#x",psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hDestHandle[l]));
                                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hDestHandle[l])
                                                {
                                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                    bBreakLoop = true;
                                                }
                                                if(true == bBreakLoop)
                                                    break;
                                            }
                                            if(true == bBreakLoop)
                                                break;                        
                                        }
                                    }
                                    else if (psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing != NULL)
                                    {
                                        hTempHandle = psAudProcNetwork->sAudProcessingStage[ui32MasterBranchId][ui32MasterStageId].hAudioProcessing;                        
                                        for(k=0;k<BRAP_P_MAX_DST_PER_RAPCH;k++)
                                        {
                                            for(l=0;l<BRAP_P_MAX_DEST_PER_PROCESSING_STAGE;l++)
                                            {
                                                BDBG_MSG(("hTempHandle->hDestHandle =%#x",hTempHandle->hDestHandle[l]));
                                                if(psNetworkInfo->sDspOutConfig[k].hDestHandle == hTempHandle->hDestHandle[l])
                                                {
                                                    pCitInputStruct->pBranchInfo[ui32SrcBranchId]->sFwStgInfo[ui32SrcStageId].sStgConnectivity.sDstDetails[uiDistinctOut][uiNumDst].uDetails.sIoBuf = psNetworkInfo->sDspOutConfig[k].sOutputBufConfig;
                                                    bBreakLoop = true;
                                                }
                                                if(true == bBreakLoop)
                                                    break;
                                            }
                                            if(true == bBreakLoop)
                                                break;                        
                                        }
                                    }
                                    else
                                    {
                                        BDBG_ERR(("Invalid stage (%d) ",j));
                                        BDBG_ASSERT(0);
                                    }   
                                }
                            }
                        }
                    }
#endif                    
                }
                break;
            }

            if(true == bStageFound)
            {            
                psAudProcNetwork->sAudProcessingStage[i][j].ui32CitBranchId = ui32BranchId;
                psAudProcNetwork->sAudProcessingStage[i][j].ui32CitStageId = ui32StageId;                                            
                ui32StageId++;
            }
        }
        if(true == bIsFwBranch)
        {
            ui32BranchId++;
        }
        else/* If complete branch is cloned then decrease the no of Branch stored in pCitInputStruct  (which was earlier incremented)*/
        {
            pCitInputStruct->ui32NumBranches--;
        }
    }
    }


#if (1 == LFE_WORKAROUND)
	/* Re-Assigning eDistinctOpType of decoder stage on each branch based on the PP next to it */
    for(ui32BranchId =0 ; ui32BranchId < pCitInputStruct->ui32NumBranches; ui32BranchId++)
    {
        for(ui32StageId = 0; ui32StageId < pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages; ui32StageId++)
        {
            if(BRAP_CIT_P_AudioAlgoType_ePostProc == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType)
        	{
				if(BRAP_CIT_P_FwStgSrcDstType_eFwStg == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType)
				{
					uiTempBranchId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId;
					uiTempStageId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId;
					ui32DistinctOpNum = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum;

                    if ( (BRAP_DSPCHN_AudioType_ePcm == pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].uAlgorithm.eDecAudioAlgo) ||
					     ( (BRAP_CIT_P_ProcessingType_eCustomVoice != pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo) &&
				           (BRAP_CIT_P_ProcessingType_ePLll != pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo) &&
				           (BRAP_CIT_P_ProcessingType_eSrsXt != pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo)
				         )
			           )
					{
					    if (pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].sStgConnectivity.eDistinctOpType[ui32DistinctOpNum] == BRAP_AF_P_DistinctOpType_e5_1_PCM)
					    {
    						pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].sStgConnectivity.eDistinctOpType[ui32DistinctOpNum] = BRAP_AF_P_DistinctOpType_eStereo_PCM;
					    }
					}
					
				}
				break;
			
        	}
			else if (BRAP_CIT_P_AudioAlgoType_eEncode == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType)
			{
			    /* 
			        The encode algorithms supported so far are capable of taking 5.1 so only check the algo type
                    This might have to be changed later
                */    
				if(BRAP_CIT_P_FwStgSrcDstType_eFwStg == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType)
				{
					uiTempBranchId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId;
					uiTempStageId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId;
					ui32DistinctOpNum = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum;
                

                    if (BRAP_DSPCHN_AudioType_ePcm == pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].uAlgorithm.eDecAudioAlgo)
                    {
                        pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].sStgConnectivity.eDistinctOpType[ui32DistinctOpNum] = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                    }

                    if (BRAP_CIT_P_EncAudioType_eDDTranscode == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eEncAudioAlgo)
                    {
                        pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].sStgConnectivity.eDistinctOpType[ui32DistinctOpNum] = BRAP_AF_P_DistinctOpType_e5_1_PCM;
                    }
			    }
                break;
			}
            else if (BRAP_CIT_P_AudioAlgoType_eDecode == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType)
            {
                if ( (BRAP_DSPCHN_AudioType_ePcm == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo) ||
                    (BRAP_DSPCHN_AudioType_eWmaStd == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo) )
                {
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[0] = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                }                
            }
    	}
	}
#endif

    /*Re-Assigning eDistinctOpType for each stage*/
    for(ui32BranchId =0 ; ui32BranchId < pCitInputStruct->ui32NumBranches; ui32BranchId++)
    {
        for(ui32StageId = 0; ui32StageId < pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages; ui32StageId++)
        {
            eOutModeType = BRAP_AF_P_OutModeType_eBypass;
            ui32DistinctOpNum = BRAP_INVALID_VALUE;
            uiTempBranchId = BRAP_INVALID_VALUE;
            uiTempStageId = BRAP_INVALID_VALUE;  
            
            if( pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_eEncode)
            {
                for(i = 0 ; i < pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDistinctOp; i++)
                {
                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i] = BRAP_AF_P_DistinctOpType_eCompressed;
                }
            }
            else if( pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc)
            {
                eOutModeType = BRAP_sPpOutMode[pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo].eOutModeType;
                if(eOutModeType == BRAP_AF_P_OutModeType_eFixed)
                {            
                    eDistinctOutType = BRAP_sPpOutMode[pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo].eOpType;                

                    if (BRAP_AF_P_DistinctOpType_eInvalid == eDistinctOutType)
                    {
                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;  
                    }
                }    
                else if(eOutModeType == BRAP_AF_P_OutModeType_eBypass)
                {
                    if (BRAP_CIT_P_ProcessingType_eGenericPassThru == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo)
                    {
                        BRAP_DSPCHN_Handle hDspCh = NULL;
                        hDspCh = BRAP_DSPCHN_P_GetDspChHandle(&hRapCh, true);

                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;

                        if (NULL != hDspCh)
                        {
                            if (BRAP_DSPCHN_AudioType_ePcm != hDspCh->sDspAudioParams.sExtAudioParams.eType)
                            {
                                eDistinctOutType = BRAP_AF_P_DistinctOpType_eCompressed;
                            }
                        }
                    }
                    else if (BRAP_CIT_P_ProcessingType_eDdre == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo)
                    {
                        eDistinctOutType =BRAP_AF_P_DistinctOpType_eInvalid;
                    }                    
                    else if(BRAP_CIT_P_FwStgSrcDstType_eFwStg == pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType)
                    {
                        uiTempBranchId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32BranchId;
                        uiTempStageId = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32StageId;   
                        ui32DistinctOpNum = pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sFwStage.ui32DistinctOpNum;                        
                        
                        if((uiTempBranchId != BRAP_INVALID_VALUE)       
                            &&(uiTempStageId != BRAP_INVALID_VALUE))
                        {
                            if (BRAP_CIT_P_ProcessingType_eFWMixerPostProc != pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].uAlgorithm.eProcAudioAlgo)
                            {
                                eDistinctOutType = pCitInputStruct->pBranchInfo[uiTempBranchId]->sFwStgInfo[uiTempStageId].sStgConnectivity.eDistinctOpType[ui32DistinctOpNum];
                            }
                        }
                        else
                        {
                            BDBG_ERR(("Error!!! BranchId/StageId of the Source is not programmed correctly. "));
                            ret = BERR_TRACE(BERR_INVALID_PARAMETER);
                            goto error;
                        }                        
                    }
                    else /* If the Input is RDB/FMM then it is assumed to be Stereo PCM*/
                    {
                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;
                    }
                }
                else if(eOutModeType == BRAP_AF_P_OutModeType_eConfigurable)
                {
                    /*[TODO] Configure eDistinctOutType as coming in user config of PP stage. For the time being assign it toStero PCM.*/
                        eDistinctOutType = BRAP_AF_P_DistinctOpType_eStereo_PCM;                      
                }

                /* Assign the eDistinctOutType determined above */
                for(i = 0 ; i < pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDistinctOp; i++)
                {
                    if (BRAP_CIT_P_ProcessingType_eFWMixerPostProc== pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eProcAudioAlgo)
                    {
                        switch(pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i])
                        {
                            case BRAP_AF_P_DistinctOpType_eStereo_PCM:
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i] = BRAP_AF_P_DistinctOpType_eStereo_MixPcm;
                                break;
                            case BRAP_AF_P_DistinctOpType_e5_1_PCM:
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i] = BRAP_AF_P_DistinctOpType_e5_1_MixPcm;
                                break;
                            case BRAP_AF_P_DistinctOpType_e7_1_PCM:
                                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i] = BRAP_AF_P_DistinctOpType_e7_1_MixPcm;
                                break;
                            default:                                
                                BDBG_ERR(("Invalid Distinct O/P =%d for FwMixer .",
                                    pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i]));
                                BDBG_ASSERT(0);
                        }
                    }    
                    else if(eDistinctOutType != BRAP_AF_P_DistinctOpType_eInvalid)
                    {
                        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[i] = eDistinctOutType;
                    }                 
                }
            }
        }
    }
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT                
    if(true == hDspCh->bChSpecificDspCh)
    {
        /* Program Common inter task DRAM address for Slave(Decode) task */
        if(BRAP_P_IsFwMixingPostLoopbackEnabled(hRapCh))
        {
            if((uiDecoderBranchId != BRAP_INVALID_VALUE)
                &&(uiDecoderStageId != BRAP_INVALID_VALUE))
            {
                uiDistinctOut = pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.ui32NumDistinctOp;
                eDistinctOutType = pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.eDistinctOpType[uiDistinctOut -1];
                uiNumDst = pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.ui32NumDst[uiDistinctOut -1];

                pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.sDstDetails[uiDistinctOut -1][uiNumDst -1].eType = BRAP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf;

                k = hDspCh->uHandle.hRapCh->uiFWMixerIpIndex;
                pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.sDstDetails[uiDistinctOut -1][uiNumDst -1].uDetails.sInterTaskDramBuffCfg.ui32IoBuffCfgAddr
                    = hDspCh->uHandle.hRapCh->hMultiStreamDecoder->ui32InterTaskIoBufferHandle[k];

                pCitInputStruct->pBranchInfo[uiDecoderBranchId]->sFwStgInfo[uiDecoderStageId].sStgConnectivity.sDstDetails[uiDistinctOut -1][uiNumDst -1].uDetails.sInterTaskDramBuffCfg.ui32IoGenericBuffCfgAddr
                    = hDspCh->uHandle.hRapCh->hMultiStreamDecoder->ui32InterTaskIoGenericBufferHandle[k];
            }
        }
    }
#endif        
    if(pCitInputStruct->pBranchInfo[0]->sFwStgInfo[0].eAudioAlgoType ==BRAP_CIT_P_AudioAlgoType_eDecode)
    {
        unsigned   uiNumOutput=0;
        for(i = 0 ; i < pCitInputStruct->pBranchInfo[0]->sFwStgInfo[0].sStgConnectivity.ui32NumDistinctOp; i++)
        {
            if(pCitInputStruct->pBranchInfo[0]->sFwStgInfo[0].sStgConnectivity.eDistinctOpType[i] == BRAP_AF_P_DistinctOpType_eStereo_PCM)
                    uiNumOutput++;
            else if((pCitInputStruct->pBranchInfo[0]->sFwStgInfo[0].sStgConnectivity.eDistinctOpType[i] == BRAP_AF_P_DistinctOpType_e5_1_PCM)
                ||(pCitInputStruct->pBranchInfo[0]->sFwStgInfo[0].sStgConnectivity.eDistinctOpType[i] == BRAP_AF_P_DistinctOpType_e7_1_PCM))
                    uiNumOutput++;                
            }                    
            hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.i32NumOutPorts = uiNumOutput;
            hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.i32NumOutPorts = uiNumOutput;        
            hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.ui32NumOutPorts = uiNumOutput;        
    }
    else
    {
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.i32NumOutPorts = 1;
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.i32NumOutPorts = 1;        
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.ui32NumOutPorts =1;        
    }


    /* Assigning Mit Structure */
    pCitInputStruct->psMit= &(hRap->sMit);

    /* Assigning CIT memInfo */
    pCitInputStruct->sMemInfo.ui32SharedMemPhysAdr = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32BaseAddr;		
    pCitInputStruct->sMemInfo.ui32SharedMemSize = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32Size;
    pCitInputStruct->sMemInfo.ui32TaskMemPhysAdr = hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr;
    pCitInputStruct->sMemInfo.ui32TaskMemSize =	hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId )].ui32Size;

    pCitInputStruct->sRdbCfg.ui32RdbBaseOffset = 0;
    pCitInputStruct->sRdbCfg.ui32RdbDspOffset = BCHP_PHYSICAL_OFFSET;

	if (hDspCh->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssEs ||
		hDspCh->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssPes) 
    {
        pCitInputStruct->eTimeBaseType = BRAP_AF_P_TimeBaseType_e27Mhz;
    }
    else
    {
        pCitInputStruct->eTimeBaseType = BRAP_AF_P_TimeBaseType_e45Khz;
    }    

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
    if(true == hDspCh->bChSpecificDspCh)
    {
            if(BRAP_P_IsMs11UsageMode(hRapCh))
            {
                pCitInputStruct->eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode;
            }
            else if(BRAP_P_IsMs10UsageMode(hRapCh))
            {
            if(hRapCh->sDecSettings.sUserConfigAppFormat.bMsUsageMode == true)
            {                
                pCitInputStruct->eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eMS10DecodeMode;
            }
            else
            {
                pCitInputStruct->eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
            }
        }
        else
        {        
            pCitInputStruct->eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
        }        
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.eDolbyMsUsageMode = pCitInputStruct->eDolbyMsUsageMode;            
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.eDolbyMsUsageMode = pCitInputStruct->eDolbyMsUsageMode;                    
        hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.eDolbyPulseUsageMode = pCitInputStruct->eDolbyMsUsageMode;                    
        

        if(pCitInputStruct->eDolbyMsUsageMode == BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode)
        {
            hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sPcmWavConfigParam.eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eMS11SoundEffectMixing;
        }
        else
        {
           hDspCh->uHandle.hRapCh->sDecSettings.sUserConfigStruct.sPcmWavConfigParam.eDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eINVALID;
        }            
        BDBG_MSG(("pCitInputStruct->eDolbyMsUsageMode =%d",pCitInputStruct->eDolbyMsUsageMode));
    }
#endif
        
error:    
    if(BERR_SUCCESS != ret)
    {
        for(i=0; i< BRAP_P_MAX_FW_BRANCH_PER_FW_TASK; i++)
        {
            if (NULL!=pCitInputStruct->pBranchInfo[i])
            {
                pCitInputStruct->pBranchInfo[i] = NULL;
            }
        }
    }
    BDBG_LEAVE(BRAP_FWIF_P_GenerateCitInput);
    return ret;
}

BERR_Code BRAP_FWIF_P_SetDecodeStageConfigParams(
    BRAP_ChannelHandle  hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType   eAudioType,     /* [in] Decode audio type */
    BRAP_DSPCHN_DecodeMode eDecodeMode,     /*[in] Decode Mode */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize,     /* [in] Config Buf Size */
    unsigned int   *uiActualConfigSize
)
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ENTER( BRAP_FWIF_P_SetDecodeStageConfigParams );

    BDBG_ASSERT( hRapCh );

    /* Update Start Time internally deducible FW exposed parameters */
    err = BRAP_P_UpdateStartTimeDeducibleFWUserConfig(hRapCh, eAudioType);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BRAP_P_SetStartTimeDeducibleFWUserConfig returned error"));
        return BERR_TRACE(err);
    }

    if(eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
    {
        BRAP_FWIF_P_PassthruConfigParams *psPassthruConfigParams;
        psPassthruConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sPassthruConfigParam);
        *uiActualConfigSize = sizeof(BRAP_FWIF_P_PassthruConfigParams);        
        if (*uiActualConfigSize > uiConfigBufSize)
        {
            BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
            BDBG_ASSERT(0);
            return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
        }
        BKNI_Memcpy((void *)(volatile void*)uiConfigBufAddr,(void *)psPassthruConfigParams,*uiActualConfigSize);
    }
    else  if(eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
    {
        switch( eAudioType )
        {
            case BRAP_DSPCHN_AudioType_eAc3:
#if 1
            {
                BRAP_FWIF_P_DDPMultiStreamConfigParams *psAc3ConfigParams;
                psAc3ConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_DDPMultiStreamConfigParams);               
                if (*uiActualConfigSize > uiConfigBufSize)
                {
                    BDBG_ERR(("Buffer size allocated(%d) for MS11 DDP is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
                    BDBG_ASSERT(0);
                    return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                }
                BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psAc3ConfigParams,*uiActualConfigSize);         
                    break;
            }

#else
    	    {
    		BRAP_FWIF_P_Ac3ConfigParams *psAc3ConfigParams;
    		psAc3ConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sAc3ConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_Ac3ConfigParams);               
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psAc3ConfigParams,*uiActualConfigSize);         
                break;
        	}
#endif
            case BRAP_DSPCHN_AudioType_eAc3Plus:
#if 1
            {
        		BRAP_FWIF_P_DDPMultiStreamConfigParams    *psAc3PlusConfigParams;
        		psAc3PlusConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam);                    
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_DDPMultiStreamConfigParams);                       
        		if (*uiActualConfigSize > uiConfigBufSize)
        		{
        			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
        			BDBG_ASSERT(0);
        			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
        		}
        		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psAc3PlusConfigParams,*uiActualConfigSize);         
                
                    break;
            	}    
#else
    	    {
    		BRAP_FWIF_P_Ac3ConfigParams *psAc3PlusConfigParams;
    		psAc3PlusConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sAc3PlusConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_Ac3ConfigParams);                       
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psAc3PlusConfigParams,*uiActualConfigSize);         
                break;
        	}
#endif
            case BRAP_DSPCHN_AudioType_eMpeg:
    	    {
    		BRAP_FWIF_P_MpegConfigParams *psMpegConfigParams;
    		psMpegConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sMpegConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_MpegConfigParams);                     
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psMpegConfigParams,*uiActualConfigSize);            
                break;			
        	}
            case BRAP_DSPCHN_AudioType_eAac:
            case BRAP_DSPCHN_AudioType_eAacLoas:                        
            case BRAP_DSPCHN_AudioType_eAacSbr:
            case BRAP_DSPCHN_AudioType_eAacSbrAdts:
    	    {
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
        		BRAP_FWIF_P_DolbyPulseUserConfig *psDolyPulseConfigParams;
        		psDolyPulseConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_DolbyPulseUserConfig);                    
        		if (*uiActualConfigSize > uiConfigBufSize)
        		{
        			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
        			BDBG_ASSERT(0);
        			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
        		}
        		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psDolyPulseConfigParams,*uiActualConfigSize);    
                break;
#else
    		BRAP_FWIF_P_AacheConfigParams *psAacheConfigParams;
    		psAacheConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sAacheConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_AacheConfigParams);                    
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psAacheConfigParams,*uiActualConfigSize);    
                break;
#endif                
        	}
            case BRAP_DSPCHN_AudioType_eWmaStd:
    	    {
    		BRAP_FWIF_P_WmaConfigParams *psWmaConfigParams;
    		psWmaConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sWmaStdConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_WmaConfigParams);                     
    		if (*uiActualConfigSize> uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psWmaConfigParams,*uiActualConfigSize);            
                break;
        	}
             case BRAP_DSPCHN_AudioType_eWmaPro:
    	    {
    		BRAP_FWIF_P_WmaProConfigParams *psWmaProConfigParams;
    		psWmaProConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sWmaProConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_WmaProConfigParams);                
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psWmaProConfigParams,*uiActualConfigSize);                 
                break;
        	}
             case BRAP_DSPCHN_AudioType_ePcm:
            {
                BRAP_FWIF_P_PassthruConfigParams *psPassthruConfigParams;
                hRapCh->sDecSettings.sUserConfigStruct.sPassthruConfigParam.ui32PassthruType = BRAP_FWIF_ePassthruType_PCM;
                psPassthruConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sPassthruConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_PassthruConfigParams);        
                
                if (*uiActualConfigSize > uiConfigBufSize)
                {
                    BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure.(%d)",uiConfigBufSize,*uiActualConfigSize));
                    BDBG_ASSERT(0);
                    return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                }
                BKNI_Memcpy((void *)(volatile void*)uiConfigBufAddr,(void *)psPassthruConfigParams,*uiActualConfigSize);                
                break;     
            }
            case BRAP_DSPCHN_AudioType_eLpcmDvd:
    	    {
    		BRAP_FWIF_P_LpcmUserConfig *psLpcmDvdConfigParams;
    		psLpcmDvdConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sLpcmDvdConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_LpcmUserConfig);                       
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for LPCM DVD.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psLpcmDvdConfigParams,*uiActualConfigSize);         
                break;
        	}             
             case BRAP_DSPCHN_AudioType_eDtsBroadcast:
    	    {
    		BRAP_FWIF_P_DtsHdConfigParams *psDtsBroadcastConfigParams;
    		psDtsBroadcastConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sDtsBroadcastConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_DtsHdConfigParams);                
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for DTS broadcast.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psDtsBroadcastConfigParams,*uiActualConfigSize);                 
                break;
        	}            
            case BRAP_DSPCHN_AudioType_eDtshd:
            {
        		BRAP_FWIF_P_DtsHdConfigParams *psDtsHdConfigParams;
        		psDtsHdConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sDtsHdConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_DtsHdConfigParams);                
        		if (*uiActualConfigSize > uiConfigBufSize)
        		{
        			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for DTS HD.(%d)",uiConfigBufSize,*uiActualConfigSize));
        			BDBG_ASSERT(0);
        			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
        		}

                BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psDtsHdConfigParams,*uiActualConfigSize);                 
                break;
            }
             case BRAP_DSPCHN_AudioType_ePcmWav:
    	    {
    		BRAP_FWIF_P_PcmWavConfigParams  *psPcmWavConfigParams;
    		psPcmWavConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sPcmWavConfigParam);
            *uiActualConfigSize = sizeof(BRAP_FWIF_P_PcmWavConfigParams);                
    		if (*uiActualConfigSize > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for PCM On AVI.(%d)",uiConfigBufSize,*uiActualConfigSize));
    			BDBG_ASSERT(0);
    			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psPcmWavConfigParams,*uiActualConfigSize);                 
                break;
        	}             
            case BRAP_DSPCHN_AudioType_eAmr:
        	    {
            		BRAP_FWIF_P_AmrConfigParams  *psAmrConfigParams;
            		psAmrConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sAmrConfigParam);
                    *uiActualConfigSize = sizeof(BRAP_FWIF_P_AmrConfigParams);                
            		if (*uiActualConfigSize > uiConfigBufSize)
            		{
            			BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for AMR.(%d)",uiConfigBufSize,*uiActualConfigSize));
            			BDBG_ASSERT(0);
            			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
            		}
            		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psAmrConfigParams,*uiActualConfigSize);
                    break;
            	}
            case BRAP_DSPCHN_AudioType_eDra:
            {
                BRAP_FWIF_P_DraConfigParams  *psDraConfigParams;
                psDraConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sDraConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_DraConfigParams);                
                if (*uiActualConfigSize > uiConfigBufSize)
                {
                    BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for DRA.(%d)",uiConfigBufSize,*uiActualConfigSize));
                    BDBG_ASSERT(0);
                    return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                }
                BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psDraConfigParams,*uiActualConfigSize);
                break;
            }            
            case BRAP_DSPCHN_AudioType_eRealAudioLbr:
            {
                BRAP_FWIF_P_RalbrConfigParams   *psRealAudioLbrConfigParams;
                psRealAudioLbrConfigParams  = &(hRapCh->sDecSettings.sUserConfigStruct.sRealAudioLbrConfigParam);
                *uiActualConfigSize = sizeof(BRAP_FWIF_P_RalbrConfigParams);                
                if (*uiActualConfigSize > uiConfigBufSize)
                {
                    BDBG_ERR(("Buffer size allocated(%d) is less than size of configuration parameters structure for Real Audio LBR.(%d)",uiConfigBufSize,*uiActualConfigSize));
                    BDBG_ASSERT(0);
                    return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                }
                BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psRealAudioLbrConfigParams,*uiActualConfigSize);
                break;
            }                         
            default:
                break;
        }
    }
    else
    {
            BDBG_ERR(("Invalid Decode Mode %d",eDecodeMode));
            BDBG_ASSERT(0);
    }

    BDBG_LEAVE( BRAP_FWIF_P_SetDecodeStageConfigParams );
    return err;
}


BERR_Code BRAP_FWIF_P_SetFrameSyncStageConfigParams(
    BRAP_DSPCHN_Handle	hDspCh,	         /* [in]DSP  Channel Handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize     /* [in] Config Buf Size */
)
{
    BERR_Code err = BERR_SUCCESS;
    BRAP_ChannelHandle  hRapCh;    
    BRAP_FWIF_P_FrameSyncConfigParams      *psFrameSyncConfigParams = NULL;
    
    BDBG_ENTER( BRAP_FWIF_P_SetFrameSyncStageConfigParams );

    BDBG_ASSERT( hDspCh );

    if(hDspCh->bChSpecificDspCh == true)
    {
        hRapCh = hDspCh->uHandle.hRapCh;
        psFrameSyncConfigParams = &(hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams);
    }
    else
    {
        BRAP_AssociatedChannelHandle    hAssociatedCh;    

        hAssociatedCh = hDspCh->uHandle.hAssociation;
        hRapCh = BRAP_P_GetRapChHandle(hAssociatedCh);
        if(!(BRAP_P_IsPointerValid((void *)hRapCh)))
        {
            BDBG_ERR(("No Raptor channel found in Association handle"));
            err = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;
        }
        
        psFrameSyncConfigParams = (BRAP_FWIF_P_FrameSyncConfigParams *) BKNI_Malloc(sizeof(BRAP_FWIF_P_FrameSyncConfigParams));
        if(psFrameSyncConfigParams == NULL)
        {
            err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			goto end;
            
        }
        BKNI_Memset(psFrameSyncConfigParams,0,sizeof(BRAP_FWIF_P_FrameSyncConfigParams));        
    }


    /* 
        Initialize the PES based framesync to 0 for all algorithms
        This will be made below to 1 for 3548 in case of a PES requirement
    */
    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eDisable;

    if(hDspCh->bChSpecificDspCh == false)
    {       
        psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eRingbuffer;
        err = BRAP_P_ConvertSR (hRapCh->eSamplingRate, &psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency);
        if (err != BERR_SUCCESS)
        {
            psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency = 48000;
            err = BERR_SUCCESS;
        }
    }
    else 
    {
        if(hRapCh->eAudioSource == BRAP_AudioSource_eRingBuffer)
        {
            psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
            psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eRingbuffer;
            psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency = hRapCh->uiInputSamplingRate;
        }
#if (BRAP_3548_FAMILY == 1)    
        else if(hRapCh->eAudioSource == BRAP_AudioSource_eExtCapPort)
        {
            switch (hRapCh->eCapInputPort)
            {   
                case BRAP_CapInputPort_eExtI2s0:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
                        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eExtI2s0;
                    err = BRAP_P_ConvertSR (hRapCh->hRap->sInputSettings[BRAP_FWIF_P_CapInputPort_eExtI2s0].eSampleRate, &psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency);
                    if (err != BERR_SUCCESS)
                    {
                        psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency = 48000;
                        err = BERR_SUCCESS;
                    }
                    break;

                case BRAP_CapInputPort_eRfAudio:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
                        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eCapPortRfAudio;            
                    psFrameSyncConfigParams->uAudioIpSourceDetail.ui32RfAudioCtrlStatusRegAddr = BCHP_PHYSICAL_OFFSET + BCHP_BTSC_CTRL_MAI_FORMAT_WORD;
                    break;

                case BRAP_CapInputPort_eSpdif:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
                        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eCapPortSpdif;
                    psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SpdifCtrlStatusRegAddr = BCHP_PHYSICAL_OFFSET + BCHP_SPDIF_RCVR_CTRL_STATUS;
                    break;

                case BRAP_CapInputPort_eHdmi:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
                        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eCapPortHdmi;
                    psFrameSyncConfigParams->uAudioIpSourceDetail.ui32MaiCtrlStatusRegAddr = BCHP_PHYSICAL_OFFSET + BCHP_SPDIF_RCVR_CTRL_MAI_FORMAT;
                    break;

                case BRAP_CapInputPort_eAdc:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eEnable;
                        psFrameSyncConfigParams->eAudioIpSourceType = BRAP_FWIF_P_AudioInputSource_eCapPortAdc;
                    err = BRAP_P_ConvertSR (hRapCh->hRap->sInputSettings[BRAP_CapInputPort_eAdc].eSampleRate, &psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency);
                    if (err != BERR_SUCCESS)
                    {
                        psFrameSyncConfigParams->uAudioIpSourceDetail.ui32SamplingFrequency = 48000;
                        err = BERR_SUCCESS;
                    }
                    break;

                default:
                    psFrameSyncConfigParams->eEnablePESBasedFrameSync = BRAP_AF_P_eDisable;  
                    break;
            }    
        }
#endif    
    }

    psFrameSyncConfigParams->eEnableTargetSync = hDspCh->sDspAudioParams.sExtAudioParams.bEnableTargetSync;

    if (sizeof(BRAP_FWIF_P_FrameSyncConfigParams) > uiConfigBufSize)
    {
        BDBG_ERR(("Buffer size allocated is less than size of configuration parameters structure."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    }
	BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psFrameSyncConfigParams,sizeof(BRAP_FWIF_P_FrameSyncConfigParams));

end:
    
    if(hDspCh->bChSpecificDspCh == false)
    {
        if(psFrameSyncConfigParams)
            BKNI_Free(psFrameSyncConfigParams);
    }
    BDBG_LEAVE( BRAP_FWIF_P_SetFrameSyncStageConfigParams );
    return err;    
}


BERR_Code BRAP_FWIF_P_SetTsmStageConfigParams(
    BRAP_DSPCHN_Handle	hDspCh,	         /* [in]DSP  Channel Handle */
    BRAP_AF_P_AlgoId	eFwExecId,     /* [in] Node type */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize     /* [in] Config Buf Size */
)
{
    BERR_Code err = BERR_SUCCESS;
    BRAP_ChannelHandle  hRapCh;    
    BRAP_DSPCHN_DecodeMode  eDecodeMode;    
    BRAP_FWIF_P_TsmConfigParams      *psTsmConfigParam = NULL;
    BDBG_ENTER( BRAP_FWIF_P_SetTsmStageConfigParams );

    BDBG_ASSERT( hDspCh );

    eDecodeMode =hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode;
    switch( eFwExecId )
    {
        case BRAP_AF_P_AlgoId_eDecodeTsm:
	    {
            BRAP_FWIF_P_TsmConfigParams      *psDecodeTsmConfigParam = NULL;
            if(hDspCh->bChSpecificDspCh == true)
            {
                hRapCh = hDspCh->uHandle.hRapCh;     
                
                if(eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
                {
                    psDecodeTsmConfigParam  = &(hRapCh->sDecSettings.sUserConfigStruct.sDecodeTsmConfigParam);
                }
                else if(eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
                {
                    psDecodeTsmConfigParam  = &(hRapCh->sDecSettings.sUserConfigStruct.sPassthruTsmConfigParam);
                }
                else
                {
                    BDBG_ERR(("Invalid decode mode %d",eDecodeMode));
                    err =BERR_TRACE(BERR_INVALID_PARAMETER);
                    goto end;
                }
            }
            else
            {
                BSTD_UNUSED(hRapCh);            
                psTsmConfigParam = (BRAP_FWIF_P_TsmConfigParams *) BKNI_Malloc(sizeof(BRAP_FWIF_P_TsmConfigParams));
                if(psTsmConfigParam == NULL)
                {
                    err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                    goto end;
                }
                BKNI_Memset(psTsmConfigParam,0,sizeof(BRAP_FWIF_P_TsmConfigParams));                
                psDecodeTsmConfigParam = psTsmConfigParam;
            }  


            if(true == hDspCh->sDspAudioParams.bPlayback)
            {
                psDecodeTsmConfigParam->ePlayBackOn = BRAP_FWIF_eTsmBool_True;
            }
            else
            {
                psDecodeTsmConfigParam->ePlayBackOn = BRAP_FWIF_eTsmBool_False;
            }
            if(hDspCh->bChSpecificDspCh == true)
            {                            
                if (hDspCh->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eEs)
                {
                    psDecodeTsmConfigParam->eTsmEnable = false;                
                    psDecodeTsmConfigParam->eAstmEnable= false;                                
                }            
            }
            else
            {
                psDecodeTsmConfigParam->eTsmEnable = false;                
                psDecodeTsmConfigParam->eAstmEnable= false;              
            }
            
            psDecodeTsmConfigParam->ui32AVOffset = hDspCh->sDspAudioParams.sExtAudioParams.i32AVOffset;
            psDecodeTsmConfigParam->ui32STCAddr = BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + (hDspCh->sDspAudioParams.eTimebase) * 4 + BCHP_PHYSICAL_OFFSET;                           
            psDecodeTsmConfigParam->eSTCValid  = BRAP_FWIF_eTsmBool_False;

            if( (hDspCh->sDspAudioParams.sExtAudioParams.eType == BRAP_DSPCHN_AudioType_ePcm)&&
                (hDspCh->sDspAudioParams.sExtAudioParams.eLowDelayEnableMode == BRAP_DSPCHN_LowDelayMode_eEnabled) )
            {
                psDecodeTsmConfigParam->i32TSMGrossThreshold = 3*45;
            }

            if (sizeof(BRAP_FWIF_P_TsmConfigParams) > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of configuration parameters structure."));
                BDBG_ASSERT(0);
                return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
            }
            BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psDecodeTsmConfigParam,sizeof(BRAP_FWIF_P_TsmConfigParams));
			goto end;
    	}
        case BRAP_AF_P_AlgoId_eEncodeTsm:
	    {
            BRAP_FWIF_P_TsmConfigParams      *psEncodeTsmConfigParam;
            if(hDspCh->bChSpecificDspCh == true)
            {            
                hRapCh = hDspCh->uHandle.hRapCh;                 
                psEncodeTsmConfigParam  = &(hRapCh->sDecSettings.sUserConfigStruct.sEncodeTsmConfigParam);
            }
            else
            {
                BSTD_UNUSED(hRapCh);            
                psTsmConfigParam = (BRAP_FWIF_P_TsmConfigParams *) BKNI_Malloc(sizeof(BRAP_FWIF_P_TsmConfigParams));
                if(psTsmConfigParam == NULL)
                {
                    err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                    goto end;
                }
                BKNI_Memset(psTsmConfigParam,0,sizeof(BRAP_FWIF_P_TsmConfigParams));
                psEncodeTsmConfigParam = psTsmConfigParam;
            }
            
            if(true == hDspCh->sDspAudioParams.bPlayback)
            {
                psEncodeTsmConfigParam->ePlayBackOn = BRAP_FWIF_eTsmBool_True;
            }
            else
            {
                psEncodeTsmConfigParam->ePlayBackOn = BRAP_FWIF_eTsmBool_False;
            }
            psEncodeTsmConfigParam->ui32AVOffset = hDspCh->sDspAudioParams.sExtAudioParams.i32AVOffset;
            psEncodeTsmConfigParam->ui32STCAddr = BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + (hDspCh->sDspAudioParams.eTimebase) * 4 + BCHP_PHYSICAL_OFFSET;                              
        
    		if (sizeof(BRAP_FWIF_P_TsmConfigParams) > uiConfigBufSize)
    		{
    			BDBG_ERR(("Buffer size allocated is less than size of configuration parameters structure."));
    			BDBG_ASSERT(0);
    			err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                         goto end;
    		}
    		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psEncodeTsmConfigParam,sizeof(BRAP_FWIF_P_TsmConfigParams));
    		goto end;
    	}
#if 0  /*Enable when passthru TSM will be supported.*/

        case BRAP_AF_P_AlgoId_ePassthroughTsm:
	{
        	BRAP_FWIF_P_TsmConfigParams      *psPassthruTsmConfigParam;
		psPassthruTsmConfigParam  = &(hRapCh->sDecSettings.sUserConfigStruct.sPassthruTsmConfigParam);
		if (sizeof(BRAP_FWIF_P_TsmConfigParams) > uiConfigBufSize)
		{
			BDBG_ERR(("Buffer size allocated is less than size of configuration parameters structure."));
			BDBG_ASSERT(0);
			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
		}
		BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psPassthruTsmConfigParam,sizeof(BRAP_FWIF_P_TsmConfigParams));
		break;
    	}    
#endif        
        default:
            BDBG_ERR(("Not a TSM Node"));
            BDBG_ASSERT(0);
            return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);            
    }

end:
    
    if(hDspCh->bChSpecificDspCh == false)
    {
        if(psTsmConfigParam)
            BKNI_Free(psTsmConfigParam);
    }    
    BDBG_LEAVE( BRAP_FWIF_P_SetTsmStageConfigParams );
    return err;
}


BERR_Code BRAP_FWIF_P_SetProcessingStageConfigParams(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize,     /* [in] Config Buf Size */
    unsigned int   *uiActualSize            /*[out] Actual size of the user config param structure of processing algo */
)
{
    BERR_Code err = BERR_SUCCESS;
    
    BDBG_ENTER( BRAP_FWIF_P_SetProcessingStageConfigParams );

    BDBG_ASSERT( hStage );
    switch(hStage->sProcessingStageSettings.eAudioProcessing)
    {
        case BRAP_ProcessingType_eAdPan:
            *uiActualSize = sizeof(BRAP_FWIF_P_AudioDescPanConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
        		BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
        		BDBG_ASSERT(0);
        		err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            BRAP_P_MapAdPanUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            break;

        case BRAP_ProcessingType_eAdFade:
            *uiActualSize = sizeof(BRAP_FWIF_P_AudioDescFadeConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
        		BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
        		BDBG_ASSERT(0);
        		err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            BRAP_P_MapAdFadeUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            break;            
            
        case BRAP_ProcessingType_eSRC:        
            *uiActualSize = sizeof(BRAP_FWIF_P_SRCUserConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            /* write a function which will dump Processing user config at the address uiConfigBufAddr in Fw Format */
            err = BRAP_P_MapSrcUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for SRC post processing"));
                err = BERR_TRACE(err);
                goto end;
            }            
            break;
            
        case BRAP_ProcessingType_eDSOLA:        
            *uiActualSize = sizeof(BRAP_FWIF_P_DsolaConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            /* write a function which will dump Processing user config at the address uiConfigBufAddr in Fw Format */
            err = BRAP_P_MapDsolaUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for DSOLA post processing"));
                err = BERR_TRACE(err);
                goto end;
            }            
            break;
            
        case BRAP_ProcessingType_eCustomVoice:
            *uiActualSize = sizeof(BRAP_FWIF_P_CustomVoiceConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapCustomVoiceUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Custom Voice post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eCustomSurround:
            *uiActualSize = sizeof(BRAP_FWIF_P_CustomSurroundUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapCustomSurroundUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Custom Surround post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eCustomBass:
            *uiActualSize = sizeof(BRAP_FWIF_P_CustomBassUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapCustomBassUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Custom Bass post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;
            
            
            
        case BRAP_ProcessingType_eSrsXt:            
            *uiActualSize = sizeof(BRAP_FWIF_P_TruSurrndXTConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSrsxtUserConfigApptoFw (hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for SRS-XT post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eSrsHd:            
            *uiActualSize = sizeof(BRAP_FWIF_P_TruSurrndHDConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSrshdUserConfigApptoFw (hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for SRS-HD post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eSrsTruVolume:
            *uiActualSize = sizeof(BRAP_FWIF_P_TruVolumeUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSrsTruVolumeUserConfigApptoFw (hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for SRS-Tru Volume post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eGenericPassthru:            
            *uiActualSize = sizeof(BRAP_FWIF_P_PassthruConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapGenericPassthruUserConfigApptoFw (hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Generic Passthru post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
            
        case BRAP_ProcessingType_eAvl:
            *uiActualSize = sizeof(BRAP_FWIF_P_AVLConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapAvlUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for AVL post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_ePl2:
            *uiActualSize = sizeof(BRAP_FWIF_P_PL2ConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapPl2UserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for PL2 post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eEncodeDts:
            *uiActualSize = sizeof(BRAP_FWIF_P_DtsBroadcastEncConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapDTSENCUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for DTSENC post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;

        case BRAP_ProcessingType_eEncodeAc3:
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
            *uiActualSize = sizeof(BRAP_FWIF_P_DDTranscodeConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of DD Transcode configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapDolbyTranscodeUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Dolby Transcode "));
                err = BERR_TRACE(err);
                goto end;
            }      
#else            
            *uiActualSize = sizeof(BRAP_FWIF_P_Ac3EncConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapAc3ENCUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Ac3ENC post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
#endif            
            break;
        case BRAP_ProcessingType_eDolbyVolume:
            *uiActualSize = sizeof(BRAP_FWIF_P_DolbyVolumeUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Dolby volume processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapDolbyVolUserConfigApptoFw (hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Dolby volume post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
        case BRAP_ProcessingType_eEncodeMp3:
            *uiActualSize = sizeof(BRAP_FWIF_P_Mpeg1L3EncConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Mp3 Enc configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapMp3EncUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Mp3ENC post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
        case BRAP_ProcessingType_eEncodeSbc:
            *uiActualSize = sizeof(BRAP_FWIF_P_SbcEncoderUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of SBC Enc configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSbcEncUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for SBC Enc "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
        case BRAP_ProcessingType_eBrcm3DSurround:
            *uiActualSize = sizeof(BRAP_FWIF_P_Brcm3DSurroundConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of BRCM 3D Surround configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapBrcm3DSurroundUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for BRCM 3D Surround "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
            
        case BRAP_ProcessingType_eMonoDownmix:
            *uiActualSize = sizeof(BRAP_FWIF_P_MonoDownMixConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Mono Downmix configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapMonoDownmixUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Mono Downmix"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT            
        case BRAP_ProcessingType_eFwMixer:
            *uiActualSize = sizeof(BRAP_FWIF_P_MixerConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Fw Mixer PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapFwMixerUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  Fw Mixer PP "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;
#endif            
        case BRAP_ProcessingType_eAudysseyVolume:
            *uiActualSize = sizeof(BRAP_FWIF_P_AudysseyVolUserConfig);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Audyssey volume PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapAudysseyVolUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  Audyssey volume PP "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;             
        case BRAP_ProcessingType_eAudysseyABX:
            *uiActualSize = sizeof(BRAP_FWIF_P_ABXConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Audyssey ABX PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapAudysseyABXUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  Audyssey ABX PP "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;     
        case BRAP_ProcessingType_eSrsCsTd:
            *uiActualSize = sizeof(BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Srs CsTd "
                    "post processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSrsCsTdUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Srs CsTd post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
        case BRAP_ProcessingType_eSrsEqHl:
            *uiActualSize = sizeof(BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of Srs EqHl "
                    "post processing configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapSrsEqHlUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for Srs EqHl post processing"));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;            
        case BRAP_ProcessingType_eDV258:
            *uiActualSize = sizeof(BRAP_FWIF_P_DV258ConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of DV258 PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapDV258UserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  DV258 "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;    
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT            
        case BRAP_ProcessingType_eDDRE:
            *uiActualSize = sizeof(BRAP_FWIF_P_DDReencodeConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of DDRE PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapDDREUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  DDRE "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;    
#endif            
        case BRAP_ProcessingType_eBtsc:
            *uiActualSize = sizeof(BRAP_FWIF_P_BTSCEncodeConfigParams);            
            if(*uiActualSize > uiConfigBufSize)
            {
                BDBG_ERR(("Buffer size allocated is less than size of BTSC PP configuration parameters structure."));
                BDBG_ASSERT(0);
                err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                goto end;
            }                
            err = BRAP_P_MapBtscUserConfigApptoFw(hStage,uiConfigBufAddr,*uiActualSize);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("Error in setting user config for  BTSC "));
                err = BERR_TRACE(err);
                goto end;
            }               
            break;              
            
        case BRAP_ProcessingType_eXen:
        case BRAP_ProcessingType_eBbe:
        default:
            break;
    }
    BDBG_LEAVE( BRAP_FWIF_P_SetProcessingStageConfigParams );
end:
    return err;
}

BERR_Code BRAP_FWIF_P_InitSpdifChanStatusParams(
                        BRAP_OP_SpdifChanStatusParams   *psSpdifChStatusParams,
                        unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
                        unsigned int    uiConfigBufSize     /* [in] Config Buf Size */)
{
    BERR_Code err = BERR_SUCCESS;
    BRAP_AF_P_sSPDIF_USER_CFG   *psCbitParams;
    
    BDBG_ENTER( BRAP_FWIF_P_InitSpdifChanStatusParams );
       
    if (sizeof(BRAP_AF_P_sSPDIF_USER_CFG) > uiConfigBufSize)
    {
    	BDBG_ERR(("Buffer size allocated is less than size of CBit structure."));
    	BDBG_ASSERT(0);
    	return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    }
    
    psCbitParams = (BRAP_AF_P_sSPDIF_USER_CFG   *) BKNI_Malloc(sizeof( BRAP_AF_P_sSPDIF_USER_CFG));
    if(NULL == psCbitParams)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset (psCbitParams, 0, sizeof( BRAP_AF_P_sSPDIF_USER_CFG));         
    
    psCbitParams->i32ProfessionalModeFlag = psSpdifChStatusParams->bProfessionalMode;
    psCbitParams->i32SoftwareCopyrightAsserted =psSpdifChStatusParams->bSwCopyRight;
    psCbitParams->i32CategoryCode =psSpdifChStatusParams->ui16CategoryCode;
    psCbitParams->i32ClockAccuracy =psSpdifChStatusParams->ui16ClockAccuracy;
    psCbitParams->i32bSeparateLRChanNum =psSpdifChStatusParams->bSeparateLRChanNum;       
    psCbitParams->i32CgmsA = psSpdifChStatusParams->uiCGMS_A;
    psCbitParams->i32SpdifPcmWordWidth = psSpdifChStatusParams->ui16SpdifPcmWordWidth;    

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,psCbitParams,sizeof(BRAP_AF_P_sSPDIF_USER_CFG));
    
    BKNI_Free(psCbitParams);
    BDBG_LEAVE( BRAP_FWIF_P_InitSpdifChanStatusParams );    
    return err;
}

/***************************************************************************
Summary:
	Gets all posted Async messages from the Async queue     
    
Description:
    This is the isr version of BRAP_FWIF_P_GetAsyncMsg.

Returns:
    BERR_SUCCESS else error
    Also returns the number of messages received

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAsyncMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 void *pMsgBuf,/*[in]*/
									 unsigned int *puiNumMsgs/*[out]*/)
   
{
	BERR_Code err=BERR_SUCCESS;
	
	uint32_t ui32dramReadPtr=0;
	uint32_t ui32dramWritePtr=0;
	uint32_t ui32maskReadPtr=0;
	uint32_t ui32maskWritePtr=0;
    uint32_t ui32chunk1=0,ui32chunk2=0,i = 0;
	int32_t i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
	void *pvMsgQueueReadAddr=NULL;
    unsigned int uiMsgIndex = 0, uiContMsgs = 0, uiMoreMsgs = 0;
	
	BDBG_ENTER(BRAP_FWIF_P_GetMsg);
	BDBG_ASSERT(hMsgQueue);
	BDBG_ASSERT(pMsgBuf);
    BDBG_ASSERT(puiNumMsgs);
    BSTD_UNUSED(ui32chunk1);
    BSTD_UNUSED(ui32chunk2);

    *puiNumMsgs = 0;
	ui32dramReadPtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset);

	ui32dramWritePtr=BRAP_Read32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset);
       
#if 0
	BDBG_MSG(("ui32dramReadPtr > %x", ui32dramReadPtr));
	BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));
#endif    
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
    ui32ResponseSize = BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;

	/* Checking if a msg is present */
	
	/* Case1:if(readptr>writeptr)*/
	if(ui32maskReadPtr > ui32maskWritePtr)
	{	

		i32BytesToBeRead=(hMsgQueue->uiEndAddr-ui32maskReadPtr)+
                (ui32maskWritePtr-hMsgQueue->uiBaseAddr);
        uiContMsgs = (hMsgQueue->uiEndAddr-ui32maskReadPtr)/
                        BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
        uiMoreMsgs = (ui32maskWritePtr-hMsgQueue->uiBaseAddr)/
                        BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
	}

	 /* Case2:if(writeptr>readptr) */
	if(ui32maskWritePtr>ui32maskReadPtr)
	{
		i32BytesToBeRead=ui32maskWritePtr-ui32maskReadPtr;
        uiContMsgs = i32BytesToBeRead / BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
        uiMoreMsgs = 0;
	}

/* The following two cases shouldn't happen as one buffer gap is maintained 
    between the read & write ptr */
#if 0    
    /*Case 3:if readptr == writeptr & wrap bits not matching */
    if((ui32maskWritePtr ==ui32maskReadPtr)&&
        ((ui32dramReadPtr & ~(BRAP_FWIF_P_MASK_MSB)) !=
         (ui32dramWritePtr & ~(BRAP_FWIF_P_MASK_MSB))))
    {	
        /*All messages have to be read*/
        i32BytesToBeRead=hMsgQueue->uiEndAddr-hMsgQueue->uiBaseAddr;
        uiContMsgs = (hMsgQueue->uiEndAddr-ui32maskReadPtr)/
                        BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
        uiMoreMsgs = (ui32maskWritePtr-hMsgQueue->uiBaseAddr)/
                        BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

    /*Case 4:if readptr == writeptr & wrap bits matching  */
    if((ui32maskWritePtr ==ui32maskReadPtr)&&
        ((ui32dramReadPtr & ~(BRAP_FWIF_P_MASK_MSB)) ==
         (ui32dramWritePtr & ~(BRAP_FWIF_P_MASK_MSB))))
    {	
        /*No messages have to be read*/
        i32BytesToBeRead=0;
        uiContMsgs = 0;
        uiMoreMsgs = 0;
    }
#endif    
    
	 if(i32BytesToBeRead <= 0)
	 {
		BDBG_MSG(("The Message Queue is empty.No message is present."));
		/*BDBG_ASSERT(0); *//* Removing the assert to take care of int timings from fw */
		/*return BERR_TRACE(BRAP_ERR_BUFFER_EMPTY);*/
		return BERR_TRACE(err);
	 }
    /* Revisit this if we make buffers a non-integral multiple of message size */
    *puiNumMsgs = i32BytesToBeRead/BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
	
	/*Reading Message from the message queue into the message buffer*/

    if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,ui32maskReadPtr,(void **)&pvMsgQueueReadAddr)))
    	return BERR_TRACE(err);
    for(uiMsgIndex = 0; uiMsgIndex < uiContMsgs; uiMsgIndex++)
    {
    	for(i=0;i<(ui32ResponseSize/4);i++)
    	{
    		*((uint32_t *)pMsgBuf+(uiMsgIndex * BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i)
                =BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr+(i*4)+(uiMsgIndex * BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES));
    	}
        
       ui32dramReadPtr +=  BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;
	/*updating read ptr in the Queue Attribute Structure*/
	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, ui32dramReadPtr);        
    }
	if(BERR_SUCCESS!=(err=BRAP_ConvertOffsetToAddress_isr(hMsgQueue->hHeap,hMsgQueue->uiBaseAddr,(void **)&pvMsgQueueReadAddr)))
		return BERR_TRACE(err);
    
    for(uiMsgIndex = 0; uiMsgIndex < uiMoreMsgs; uiMsgIndex++)
    {
    	for(i=0;i<(ui32ResponseSize/4);i++)
    	{
    		*((uint32_t *)pMsgBuf+((uiMsgIndex+uiContMsgs) * BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i)
                =BRAP_MemRead32_isr((uint32_t )pvMsgQueueReadAddr+(i*4)+(uiMsgIndex * BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES));
    	}
        ui32dramReadPtr = hMsgQueue->uiBaseAddr +(uiMsgIndex+1)* BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES;  
	/*updating read ptr in the Queue Attribute Structure*/
	BRAP_Write32_isr(hMsgQueue->hRegister,BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0 + 4 * 4 * hMsgQueue->ui32FifoId + 
                    BRAP_FWIF_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, ui32dramReadPtr);         
    }
	/*updating read ptr in the handle*/
	hMsgQueue->uiReadPtr = ui32dramReadPtr;
	
	BDBG_LEAVE(BRAP_FWIF_P_GetMsg);
	return err;
 
}

#ifdef RAP_VIDEOONDSP_SUPPORT
/***************************************************************************
Summary:
        Generate the CIT input Structure
    
Description:
        This function takes the audio processing network from channel handle and
        Generates the CIT input structure to be passed as Input to CIT module.
        Preprocessing is not supported.
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GenerateVideoCitInput(
    BRAP_Handle                         hRap,               /* [in] Rap Device Handle */
    BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    BRAP_P_NetworkInfo                     *psNetworkInfo,          /* [in] input network Info */
    BRAP_CIT_P_InputInfo                *pCitInputStruct,    /* [out] CIT input structure */
    BRAP_VF_P_sVDecodeBuffCfg   *psVDecodeBuffCfg,
    BRAP_FWIF_P_FwTaskHandle hFwTaskCreate     /*[in] Fw Task handle */
)

{
    BERR_Code                       ret=BERR_SUCCESS;
    unsigned int                             i=0;
    unsigned int                        ui32BranchId =0,ui32StageId =0;
    BRAP_ChannelHandle          hRapCh = NULL;

    BDBG_ENTER(BRAP_FWIF_P_GenerateCitInput);

    BDBG_ASSERT(hRap);
    BDBG_ASSERT(pCitInputStruct);
    BDBG_ASSERT(psAudProcNetwork);
    BDBG_ASSERT(psNetworkInfo);

    BKNI_Memset(pCitInputStruct,0,sizeof(BRAP_CIT_P_InputInfo));
    for(i=0; i< BRAP_P_MAX_FW_BRANCH_PER_FW_TASK; i++)
    {
        pCitInputStruct->pBranchInfo[i] = (BRAP_CIT_P_FwBranchInfo *)BKNI_Malloc(sizeof(BRAP_CIT_P_FwBranchInfo));
        if (NULL==pCitInputStruct->pBranchInfo[i])
        {
            BDBG_ERR(("Out of System memory"));
            ret = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        else
        {
            BKNI_Memset(pCitInputStruct->pBranchInfo[i],0,sizeof(BRAP_CIT_P_FwBranchInfo));
        }
    }

    hRapCh = hDspCh->uHandle.hRapCh;        
    ui32StageId = 0;
    ui32BranchId=0;
    if(true == psAudProcNetwork->sAudProcessingStage[0][0].bDecoderStage)
    {
        BDBG_MSG(("This is a decoder stage"));
        pCitInputStruct->ui32NumBranches=1;        
        pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages=1;
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;

        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                        BRAP_CIT_P_AudioAlgoType_eDecode;

        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eVideoAlgo = 
                                        psNetworkInfo->eVideoType;
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo = 
                                        psNetworkInfo->eVideoType;       

        BDBG_ERR(("pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo %d"
            ,pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eDecAudioAlgo));
        BDBG_ERR(("pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eVideoAlgo %d"
            ,pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eVideoAlgo));        
        /* Stage Connectivity */
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc++;
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType = psNetworkInfo->eNetworkInputType;

        switch (psNetworkInfo->eNetworkInputType)
        {
            case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
            case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
                pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].uDetails.sIoBuf = psNetworkInfo->sInputBufConfig;
                break;
            case BRAP_CIT_P_FwStgSrcDstType_eFwStg:
            default:
                /*FW stage can not be input to Decode stage untill Pre-Processing support is added */
                BDBG_ASSERT(0);
        }
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDistinctOp = 0;
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDst[0] = 0;        
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[0] = BRAP_AF_P_DistinctOpType_eInvalid;         
/*Need to check if it needs to be populated*/
#if 0        
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sDstDetails[0][0].eType=BRAP_CIT_P_FwStgSrcDstType_eDRAMBuf ;  
        
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sDstDetails[0][0].uDetails.sIoBuf.uIoBuffer.sIoBuffer.eBufferType=BRAP_AF_P_BufferType_eDRAM ;               
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sDstDetails[0][0].uDetails.sIoBuf.uIoBuffer.sIoBuffer.ui32NumBuffers = 6 ;               
        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sDstDetails[0][0].uDetails.sIoBuf.uIoBuffer.sIoBuffer.sCircBuffer[]= ;                       

        pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sDstDetails[0][0].uDetails.sIoBuf.uIoBuffer.sIoBuffer.sCircBuffer[]= ;                       

#endif        
        psAudProcNetwork->sAudProcessingStage[0][0].ui32CitBranchId = ui32BranchId;
        psAudProcNetwork->sAudProcessingStage[0][0].ui32CitStageId = ui32StageId;                   
    }       

    

        /* Assigning Mit Structure */
        pCitInputStruct->psMit= &(hRap->sMit);

        /* Assigning CIT memInfo */
        pCitInputStruct->sMemInfo.ui32SharedMemPhysAdr = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32BaseAddr;		
        pCitInputStruct->sMemInfo.ui32SharedMemSize = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32Size;
        pCitInputStruct->sMemInfo.ui32TaskMemPhysAdr = hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr;
        pCitInputStruct->sMemInfo.ui32TaskMemSize =	hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId )].ui32Size;
        pCitInputStruct->sRdbCfg.ui32RdbBaseOffset = 0;
        pCitInputStruct->sRdbCfg.ui32RdbDspOffset = BCHP_PHYSICAL_OFFSET;
    
    	if (hDspCh->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssEs ||
    		hDspCh->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssPes) 
        {
            pCitInputStruct->eTimeBaseType = BRAP_AF_P_TimeBaseType_e27Mhz;
        }
        else
        {
            pCitInputStruct->eTimeBaseType = BRAP_AF_P_TimeBaseType_e45Khz;
        }    

        for(i = 0; i < BRAP_RM_P_MAX_DSPS; i++)
        {
            psVDecodeBuffCfg->ui32PDQCircBuffAddr = BRAP_REALVIDEO_DSP_P_FIFO_PDQ;
            psVDecodeBuffCfg->ui32PRQCircBuffAddr = BRAP_REALVIDEO_DSP_P_FIFO_PRQ;
        }
        
        for(i=0;i<BRAP_FWMAX_VIDEO_BUFF_AVAIL;i++)
        {
            psVDecodeBuffCfg->sFrameBuffParams[i].sFrameBuffLuma.ui32DramBufferAddress= hRap->sMemInfo.sRealVideoBufferInfo.sLumaFrameBuffParams[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sFrameBuffParams[i].sFrameBuffLuma.ui32BufferSizeInBytes= hRap->sMemInfo.sRealVideoBufferInfo.sLumaFrameBuffParams[i].ui32Size;                
            psVDecodeBuffCfg->sFrameBuffParams[i].sFrameBuffChroma.ui32DramBufferAddress= hRap->sMemInfo.sRealVideoBufferInfo.sChromaFrameBuffParams[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sFrameBuffParams[i].sFrameBuffChroma.ui32BufferSizeInBytes= hRap->sMemInfo.sRealVideoBufferInfo.sChromaFrameBuffParams[i].ui32Size;                            
        }
        
        for(i=0;i<BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL;i++)
        {
            psVDecodeBuffCfg->sReferenceBuffParams[i].sFrameBuffLuma.ui32DramBufferAddress = hRap->sMemInfo.sRealVideoBufferInfo.sLumaReferenceBuffParams[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sReferenceBuffParams[i].sFrameBuffLuma.ui32BufferSizeInBytes = hRap->sMemInfo.sRealVideoBufferInfo.sLumaReferenceBuffParams[i].ui32Size;                
            psVDecodeBuffCfg->sReferenceBuffParams[i].sFrameBuffChroma.ui32DramBufferAddress= hRap->sMemInfo.sRealVideoBufferInfo.sChromaReferenceBuffParams[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sReferenceBuffParams[i].sFrameBuffChroma.ui32BufferSizeInBytes= hRap->sMemInfo.sRealVideoBufferInfo.sChromaReferenceBuffParams[i].ui32Size;                            
        }
        
     
        for(i=0;i<BRAP_FWMAX_VIDEO_BUFF_AVAIL;i++)
        {
            psVDecodeBuffCfg->sUPBs[i].ui32DramBufferAddress= hRap->sMemInfo.sRealVideoBufferInfo.sUPBs[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sUPBs[i].ui32BufferSizeInBytes= hRap->sMemInfo.sRealVideoBufferInfo.sUPBs[i].ui32Size;                
        }    

        for(i=0;i<BRAP_FWMAX_MB_INFO_AVAIL;i++)
        {
            psVDecodeBuffCfg->sDRAMMBInfoStartAdr[i].ui32DramBufferAddress= hRap->sMemInfo.sRealVideoBufferInfo.sDRAMMBInfoStartAdr[i].ui32BaseAddr;        
            psVDecodeBuffCfg->sDRAMMBInfoStartAdr[i].ui32BufferSizeInBytes= hRap->sMemInfo.sRealVideoBufferInfo.sDRAMMBInfoStartAdr[i].ui32Size;                
        }    

        psVDecodeBuffCfg->ui32NumBuffAvl = hDspCh->sDspAudioParams.sExtVideoParams.ui32NumBuffAvl;        

        psVDecodeBuffCfg->ui32DispStripeWidth =hDspCh->sDspAudioParams.sExtVideoParams.ui32DispStripeWidth;        
        psVDecodeBuffCfg->ui32DispStripeHeight = hDspCh->sDspAudioParams.sExtVideoParams.ui32DispStripeHeight;         

        
error:    
    if(BERR_SUCCESS != ret)
    {
        for(i=0; i< BRAP_P_MAX_FW_BRANCH_PER_FW_TASK; i++)
        {
            if (NULL!=pCitInputStruct->pBranchInfo[i])
            {
                BKNI_Free(pCitInputStruct->pBranchInfo[i]);
                pCitInputStruct->pBranchInfo[i] = NULL;
            }
        }
    }
    BDBG_LEAVE(BRAP_FWIF_P_GenerateCitInput);
    return ret;
}
#endif

