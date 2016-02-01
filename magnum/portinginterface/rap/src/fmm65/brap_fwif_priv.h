/***************************************************************************
*     Copyright (c) 2006-2011, Broadcom Corporation
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
*	This file contains data structures and prototypes of low level functions that interact with 
*      firmware and DSP hardware.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_FWIF_H__
#define _BRAP_FWIF_H__

#include <brap_cmdresp_priv.h>
#include "brap_types_priv.h"
#include "brap_priv.h"

#define BRAP_FWIF_P_MASK_MSB 0x7fffffff  
/*#define MESSAGE_SIZE 20*/
#define MESSAGE_SIZE sizeof(BRAP_FWIF_P_Command)

#define BRAP_FWIF_P_MAX_FW_TASK_PER_DSPCHN      (1)

#define BRAP_FWIF_P_FIFO_BASE_OFFSET    0
#define BRAP_FWIF_P_FIFO_END_OFFSET     4
#define BRAP_FWIF_P_FIFO_READ_OFFSET    12 /*8*/
#define BRAP_FWIF_P_FIFO_WRITE_OFFSET   8  /*12*/


/*************************************************************************
Summary:
       Data structure details for the message queue in the system memory
    
Description:
	Parmeters passed in this structure:-
		Handle for the heap of memory  to be allocated
		Base and End Addresses of the message queue (local copy)
    		Read pointer and write pointer addresses (local copy)required for operations like writing a message and reading a message  
    		The message queue attribute address containing the base address of the message queue needed to be passed to the shared copy in DRAM 
		The MSB toggle bits for both the read and write pointers to determine wrap around conditions in the message queue
***************************************************************************/
typedef struct BRAP_FWIF_MsgQueue  
{ 
    BMEM_Handle hHeap; 
    BREG_Handle hRegister;
    unsigned int uiBaseAddr;  /* phys address */
    unsigned int uiEndAddr;  /* phys address */
    unsigned int uiReadPtr;  /* phys address */
    unsigned int uiWritePtr;  /* phys address */
    uint32_t ui32FifoId;  /* Fifo Id for this message queue */
    uint32_t ui32DspOffset; /* DSP Register Offset */
    
} BRAP_FWIF_MsgQueue; 
 
typedef struct BRAP_FWIF_MsgQueue *BRAP_FWIF_MsgQueueHandle;
 
/***************************************************************************
Summary:
	Data structure details of the message queue parameters in the system memory
    
Description:
	Parmeters passed:-
		Base address(virtual) of the message queue
		Size of the message queue
		Address of the attribute structure address of the message queue

***************************************************************************/
typedef struct BRAP_FWIF_MsgQueueParams
{
    unsigned int uiBaseAddr; /* Virtual Address */
    unsigned int uiMsgQueueSize;
    uint32_t ui32FifoId;  /* Fifo Id for this message queue */
} BRAP_FWIF_MsgQueueParams;

/*************************************************************************
Summary:
       Data structure for describing FW task
    
Description:
        This data structure describes firmware task object.
***************************************************************************/
typedef struct BRAP_FWIF_P_FwTask
{
    unsigned int    uiTaskId;

	BRAP_CIT_P_Output	sCitOutput;
#ifdef RAP_REALVIDEO_SUPPORT
    BRAP_CIT_P_VideoCITOutput   sVideoCitOutput;    
#endif
#ifdef RAP_GFX_SUPPORT    
    BRAP_CIT_P_GfxCITOutput	sGfxCitOutput;
#endif    
#ifdef RAP_SCM_SUPPORT
	BRAP_CIT_P_ScmCITOutput	sScmCitOutput;
#endif
    bool bChSpecificTask;   /* TRUE = Task is channel specific,
                                               FALSE = Task belongs to association */
    union
    {
        BRAP_ChannelHandle hRapCh; /* Valid if task is channel specific, bChSpecificTask = true */
        BRAP_AssociatedChannelHandle hAssociation; /* Valid if task is shared between channels, 
                                                                                   bChSpecificTask = false */
    } uHandle;
    BRAP_DSP_Handle hDsp; /* Handle of DSP in which the task is running */
    BRAP_FWIF_MsgQueueHandle hAsyncMsgQueue; /* Asynchronous message queue 
                                                belonging to this task */
    BRAP_FWIF_MsgQueueHandle hSyncMsgQueue; /* Synchronous message queue 
                                               belonging to this task */
    void *pAsyncMsgMemory;                  /* Memory for contiguous Async Msgs */
    bool bStopped;  /* TRUE : If the stop command for this task has been sent,
                                                But the Processing of stop is still under process. Keep it in true state untill the hDspCh is started*/              
    unsigned int uiLastEventType;                                                                                                        
    unsigned int uiCommandCounter;    
    bool                bMaster;                                                
    unsigned int    uiMasterTaskId;
                                                
} BRAP_FWIF_P_FwTask;

/*************************************************************************
Summary:
       FW task handle
    
Description:
    This is an opaque handle for firmware task object. 
***************************************************************************/
typedef struct BRAP_FWIF_P_FwTask *BRAP_FWIF_P_FwTaskHandle;


typedef struct BRAP_FWIF_P_TaskInterface
{
	unsigned int	tbd;
} BRAP_FWIF_P_TaskInterface;

/***************************************************************************
Summary:
    This structure for providing info of DSP output ring buffers.
***************************************************************************/ 

typedef struct BRAP_P_DspOutputBufferConfig
{
    BRAP_CIT_P_IoBuf            sOutputBufConfig;
    BRAP_DestinationHandle      hDestHandle;
}BRAP_P_DspOutputBufferConfig;


/***************************************************************************
Summary:
    This structure holds all the Network info that are required for generating
    CIT input structure from channel Audio processing network.
***************************************************************************/ 
typedef struct BRAP_P_NetworkInfo
{
    BRAP_DSPCHN_AudioType           eAudioType;                                                      /*Audio type of decode stage */
    BRAP_DSPCHN_VideoType           eVideoType;                                                      /*Audio type of decode stage */
    BRAP_DSPCHN_DecodeMode        eDecodeMode;                                                      /*Decode Mode of decode stage */    
    BRAP_CIT_P_FwStgSrcDstType      eNetworkInputType;                                         /* Type of the input to the network */
    BRAP_CIT_P_IoBuf                sInputBufConfig;                                                      /* Input Buf Config*/ 
    BRAP_P_DspOutputBufferConfig    sDspOutConfig[BRAP_P_MAX_DST_PER_RAPCH]; /* Outputs Buf Config*/
    
}BRAP_P_NetworkInfo;

/***************************************************************************
Summary:
    This enum hold the type of message passed 
***************************************************************************/ 
typedef enum  BRAP_P_MsgType
{
    BRAP_P_MsgType_eSyn,
    BRAP_P_MsgType_eAsyn
}BRAP_P_MsgType;


/***************************************************************************
Summary:
    This structure is used to MAP the PI Processing Enum to CIT Encode/Processing Enum.
***************************************************************************/ 
typedef struct BRAP_FWIF_P_MapProcessingEnum
{
    union
    {
        BRAP_CIT_P_ProcessingType   eProcessingType;
        BRAP_CIT_P_EncAudioType     eEncodeType;
    } uEnumName;
    
    BRAP_CIT_P_AudioAlgoType    eEnumType; /* Either BRAP_CIT_P_AudioAlgoType_eEncode 
                                                                                or BRAP_CIT_P_AudioAlgoType_ePostProc */

}BRAP_FWIF_P_MapProcessingEnum;


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
BERR_Code BRAP_FWIF_P_CreateMsgQueue(BRAP_FWIF_MsgQueueParams *psMsgQueueParams ,
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
BERR_Code BRAP_FWIF_P_DestroyMsgQueue(BRAP_FWIF_MsgQueueHandle   hMsgQueue,
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
BERR_Code BRAP_FWIF_P_WriteMsg(BRAP_FWIF_MsgQueueHandle   hMsgQueue, 
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
BERR_Code BRAP_FWIF_P_WriteMsg_isr(BRAP_FWIF_MsgQueueHandle   hMsgQueue/*[in]*/,
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

BERR_Code BRAP_FWIF_P_GetMsg(BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
									  void *pMsgBuf, BRAP_P_MsgType eMgsType);

/***************************************************************************
Summary:
	Gets a message from the message queue and writes in into the message buffer    
    
Description:
    This is the isr version of BRAP_FWIF_P_GetMsg.

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
									  void *pMsgBuf, BRAP_P_MsgType eMgsType);

/***************************************************************************
Summary:
        Send command to firmware
        
Description:
        This function sends command to firmware.

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_SendCommand( BRAP_FWIF_MsgQueueHandle  hMsgQueue/*[in]*/, 
                        								    BRAP_FWIF_P_Command *psCommand/*[in]*/ ,
                        								        BRAP_Handle hRap ,
                                                                                BRAP_FWIF_P_FwTaskHandle     hTask   /* [in] Task handle */);

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
                        								        BRAP_FWIF_P_FwTaskHandle     hTask   /* [in] Task handle */);

/***************************************************************************
Summary:
        Get response from firmware for a given command 
    
Description:
        This function receives response from firmware for an issued command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetResponse( BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
                        BRAP_FWIF_P_Response    *psResponse,
                        BRAP_P_MsgType          eMsgType);
/***************************************************************************
Summary:
        Get acknowledgement from firmware for a given command 
    
Description:
        This function receives acknowledgement from firmware for an issued command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAck( BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
                       						  BRAP_FWIF_P_Response    *psAck );

/***************************************************************************
Summary:
        Get response from firmware for a given asynchronous command 
    
Description:
        This function receives response from firmware for an issued asynchronous 
        command

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAsynResponse( BRAP_FWIF_MsgQueueHandle  hMsgQueue, 
                                        BRAP_FWIF_P_AsynEventMsg   *psAsynResponse);

/***************************************************************************
Summary:
        Flushes the queue
    
Description:
        This function resets all the pointers if a queue is created afresh
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_FlushMsgQueue( BRAP_FWIF_MsgQueueHandle  hMsgQueue);

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
);

/***************************************************************************
Summary:
        Generate the CIT input Structure
    
Description:
        This function takes the audio processing network from channel handle and
        Generates the CIT input structure to be passed as Input to CIT module.
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
);

BERR_Code BRAP_FWIF_P_GenerateVideoCitInput(
    BRAP_Handle                         hRap,               /* [in] Rap Device Handle */
    BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    BRAP_P_NetworkInfo                     *psNetworkInfo,          /* [in] input network Info */
    BRAP_CIT_P_InputInfo                *pCitInputStruct,    /* [out] CIT input structure */
#ifdef RAP_REALVIDEO_SUPPORT  
    BRAP_VF_P_sVDecodeBuffCfg   *psVDecodeBuffCfg,
#endif    
    BRAP_FWIF_P_FwTaskHandle hFwTaskCreate     /*[in] Fw Task handle */
);
BERR_Code BRAP_FWIF_P_SetTsmStageConfigParams(
    BRAP_DSPCHN_Handle	hDspCh,	         /* [in]DSP  Channel Handle */
    BRAP_AF_P_AlgoId	eFwExecId,     /* [in] Node type */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize     /* [in] Config Buf Size */
);

BERR_Code BRAP_FWIF_P_SetFrameSyncStageConfigParams(
    BRAP_DSPCHN_Handle	hDspCh,	         /* [in]DSP  Channel Handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize     /* [in] Config Buf Size */
);

BERR_Code BRAP_FWIF_P_SetDecodeStageConfigParams(
    BRAP_ChannelHandle  hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType   eAudioType,     /* [in] Decode audio type */
    BRAP_DSPCHN_DecodeMode eDecodeMode,     /*[in] Decode Mode */    
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize,     /* [in] Config Buf Size */
    unsigned int   *uiActualConfigSize      /*[out] Actual Config Size */
);
BERR_Code BRAP_FWIF_P_SetProcessingStageConfigParams(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int    uiConfigBufSize,     /* [in] Config Buf Size */
    unsigned int   *uiActualSize    
);
BERR_Code BRAP_FWIF_P_InitSpdifChanStatusParams(
                        BRAP_OP_SpdifChanStatusParams   *psSpdifChStatusParams,
                        unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
                        unsigned int    uiConfigBufSize     /* [in] Config Buf Size */);

/***************************************************************************
Summary:
	Gets all posted Async messages from the Async queue     
    
Description:
    This is the isr version of BRAP_FWIF_P_GetMsg.

Returns:
    BERR_SUCCESS else error
    Also returns the number of messages received

**************************************************************************/
BERR_Code BRAP_FWIF_P_GetAsyncMsg_isr(BRAP_FWIF_MsgQueueHandle  hMsgQueue,/*[in]*/
									 void *pMsgBuf,/*[in]*/
                                     unsigned int *puiNumMsgs/*[out]*/);

/***************************************************************************
Summary:
    Destroy all the internal stages for the Channel.
    
Description:

    Destroy all the internal stages for the Channel.
    
Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BRAP_FWIF_P_DestroyInternalStages(	BRAP_ChannelHandle      hRapCh/* [in] RAP Channel handle */);


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
    bool *bIsFwBranch  /* If there is no, PP in the branch Fw doesn't treat it as a branch*/);



    
#endif /* _BRAP_FWIF_H__ */
