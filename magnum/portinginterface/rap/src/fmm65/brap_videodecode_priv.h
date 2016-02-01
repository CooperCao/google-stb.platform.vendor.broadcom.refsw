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
#ifndef _BRAP_VIDEODECODE_PRIV_H_
#define _BRAP_VIDEODECODE_PRIV_H_

#include "brap.h"
#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

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
	);

BERR_Code BRAP_VID_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);

BERR_Code BRAP_P_StartVideoChannel ( 
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams	/* [in] Audio parameters for 
	                                               starting this channel */
	);

BERR_Code BRAP_P_StopVideoChannel ( 
	BRAP_ChannelHandle 	        hRapCh		    /* [in] RAP channel handle */
	);

BERR_Code 
BRAP_P_OpenVideoDecPath(
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_ChannelParams    *pAudioParams,  /* [in] Audio params passed 
                                                   during channel start time */
    const unsigned int          uiPth
    );

BERR_Code 
  BRAP_P_StartVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

BERR_Code 
BRAP_P_StopVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,     /* [in] RAP Channel handle */
    const unsigned int          uiPth       /* [in] Path idx corresponding 
                                               to pPath[] array element */
    );

BERR_Code 
BRAP_P_CloseVideoDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

BERR_Code 
BRAP_VID_P_InitInterframeBuffer(
    BRAP_FWIF_P_FwTaskHandle	hTask
    );

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
BERR_Code BRAP_P_CreateMsgQueue(BRAP_FWIF_MsgQueueParams *psMsgQueueParams ,
                                       	 BMEM_Handle    hHeap, 
                            		     BREG_Handle    hRegister,
                            		     uint32_t       ui32DspOffset, /* Dsp Register offset for DSP */
                                       	 BRAP_FWIF_MsgQueueHandle  *hMsgQueue,
                                       	  bool bWdgRecovery /*bWatchdogrecovery*/);



/***************************************************************************
Summary:
    Destroys the Message Queue
    
Description:
    Free the memory that was allocated for the Message Queue

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_DestroyMsgQueue(BRAP_FWIF_MsgQueueHandle   hMsgQueue,
                                        BRAP_DSP_Handle          hDsp);




/***************************************************************************
Summary:
    Writes a message into the message queue reading from the message buffer
    
Description:
	Sanity check is done to check if the read and write pointers haven't been corrupted
	Checks for free space in the message queue.BUFFER FULL error is generated if there no free space in the message queue.
	Buffer_Size/4 number of bytes are copied from the Message buffer into the message queue
	Write Pointers are updated in both the shared and the local copy.
	
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_WriteMsg(BRAP_FWIF_MsgQueueHandle   hMsgQueue, 
									     void *pMsgBuf,
									     unsigned int uiBufSize);


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
									     unsigned int uiBufSize/*[in]*/);


/***************************************************************************
Summary:
	Gets a message from the message queue and writes in into the message buffer    
    
Description:
	Sanity check is done to check if the read and write pointers haven't been corrupted in the shared copy.
	Checks if a message is present. If no message is there in the message queue BUFFER_EMPTY error is returned
	MESSAGE_SIZE/4 number of words are read from the msg buffer into the message queue
	Read Pointers are updated both in the shared and the local copy    

Returns:
    BERR_SUCCESS else error

**************************************************************************/

BERR_Code BRAP_P_GetMsg(BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
									  uint32_t *pMsgBuf,bool bReadUpdate);

/***************************************************************************
Summary:
	Gets a message from the message queue and writes in into the message buffer    
    
Description:
    This is the isr version of BRAP_FWIF_P_GetMsg.

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_P_GetMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
									  uint32_t *pMsgBuf,bool bReadUpdate);

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
	);

BERR_Code BRAP_VID_P_GetPictureCount(
    BRAP_FWIF_MsgQueueHandle hPicDelQueue,
    uint32_t *puiPictureCount);

#ifdef __cplusplus
}
#endif


#endif /* _BRAP_VIDEODECODE_PRIV_H_ */


