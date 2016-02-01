/***************************************************************************
*     Copyright (c) 2003-2011, Broadcom Corporation
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
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description: Header file for firmware command-response structures.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_CMDRESP_PRIV_H__
#define _BRAP_CMDRESP_PRIV_H__
#include "brap_af_priv.h"
 /* The following are the various ID's given to each command 
from host to any task on DSP */

#define BRAP_FWIF_P_ASYNC_EVENT_ID_OFFSET					((uint32_t)0x0)

#define BRAP_FWIF_P_INVALID_COMMAND                         ((uint32_t)-1)

#define BRAP_FWIF_P_COMMAND_ID_OFFSET                       ((uint32_t)0x100)

#define BRAP_FWIF_P_ACK_ID_OFFSET                           ((uint32_t)0x200)

#define BRAP_FWIF_P_RESPONSE_ID_OFFSET                      ((uint32_t)0x300)

#define BRAP_FWIF_P_DEFAULT_EVENTS_ENABLE_MASK				((uint32_t)0x3)

	/** This is the starting Task ID offset which can be 
		used by Host to create any	new tasks in future **/
#define BRAP_FWIF_P_TASK_ID_START_OFFSET					((uint32_t)15)	

	/** This is the maximum value of taskID which can be 
		used by host in future **/		
#define BRAP_FWIF_P_TASK_MAX_ID_AVAILABLE					((uint32_t)56)


/*** Following are the various Event Ids used  ***/

#define BRAP_FWIF_P_DUMMY_FRAME_GENERATION_EVENT_ID  \
        ((uint32_t)((BRAP_FWIF_P_ASYNC_EVENT_ID_OFFSET) )  )				/** 0x00 **/

#define BRAP_FWIF_P_FRAME_REPEAT_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_DUMMY_FRAME_GENERATION_EVENT_ID) + 1)   )	/** 0x01 **/

#define BRAP_FWIF_P_FRAME_DROP_FROM_CDB_EVENT_ID \
        ((uint32_t)((BRAP_FWIF_P_FRAME_REPEAT_EVENT_ID) + 1) )				/** 0x02 **/

#define BRAP_FWIF_P_TSM_FAIL_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_FRAME_DROP_FROM_CDB_EVENT_ID) + 1))		/** 0x03 **/

#define BRAP_FWIF_P_CDB_ITB_UNDERFLOW_EVENT_ID   \
        ((uint32_t)((BRAP_FWIF_P_TSM_FAIL_EVENT_ID) + 1) )					/** 0x04 **/

#define BRAP_FWIF_P_CDB_ITB_OVERFLOW_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_CDB_ITB_UNDERFLOW_EVENT_ID) + 1)    )		/** 0x05 **/

#define BRAP_FWIF_P_CRC_ERROR_EVENT_ID       \
        ((uint32_t)((BRAP_FWIF_P_CDB_ITB_OVERFLOW_EVENT_ID) + 1) )			/** 0x06 **/

#define BRAP_FWIF_P_AUDIO_MODE_CHANGE_EVENT_ID   \
        ((uint32_t)((BRAP_FWIF_P_CRC_ERROR_EVENT_ID) + 1)    )				/** 0x07 **/

#define BRAP_FWIF_P_BIT_RATE_CHANGE_EVENT_ID \
        ((uint32_t)((BRAP_FWIF_P_AUDIO_MODE_CHANGE_EVENT_ID) + 1)    )		/** 0x08 **/

#define BRAP_FWIF_P_SAMPLING_RATE_CHANGE_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_BIT_RATE_CHANGE_EVENT_ID) + 1)  )			/** 0x09 **/

#define BRAP_FWIF_P_FRAME_SYNC_LOCK_EVENT_ID \
        ((uint32_t)((BRAP_FWIF_P_SAMPLING_RATE_CHANGE_EVENT_ID) + 1))		/** 0x0A */

#define BRAP_FWIF_P_FRAME_SYNC_LOCK_LOST_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_FRAME_SYNC_LOCK_EVENT_ID) + 1))			/** 0x0B **/

#define BRAP_FWIF_P_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID    \
        ((uint32_t)((BRAP_FWIF_P_FRAME_SYNC_LOCK_LOST_EVENT_ID) + 1))		/** 0x0C */

#define BRAP_FWIF_P_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID) + 1))	/** 0x0D **/

#define BRAP_FWIF_P_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID) + 1))	/** 0x0E **/

#define BRAP_FWIF_P_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID \
    ((uint32_t)((BRAP_FWIF_P_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID) + 1))		/** 0x0F **/

#define BRAP_FWIF_P_PTS_ERR_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID) + 1))	/** 0x10 **/

#define BRAP_FWIF_P_TSM_LOCK_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_PTS_ERR_EVENT_ID) + 1))						/** 0x11 **/

#define BRAP_FWIF_P_START_PTS_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_TSM_LOCK_EVENT_ID) + 1))						/** 0x12 **/

#define BRAP_FWIF_P_STOP_PTS_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_START_PTS_EVENT_ID) + 1))						/** 0x13 **/

#define BRAP_FWIF_P_ASTMTSM_PASS_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_STOP_PTS_EVENT_ID) + 1))						/** 0x14 **/

#define BRAP_FWIF_P_RAMP_ENABLE_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_ASTMTSM_PASS_EVENT_ID) + 1))						/** 0x15 **/

#define BRAP_FWIF_P_CDB_DATA_AVAIL_EVENT_ID    \
    ((uint32_t)((BRAP_FWIF_P_RAMP_ENABLE_EVENT_ID) + 1))						/** 0x16 **/

#define BRAP_FWIF_P_STREAM_INFO_AVAIL_EVENT_ID \
    ((uint32_t)((BRAP_FWIF_P_CDB_DATA_AVAIL_EVENT_ID) + 1))						/** 0x17 **/

#define BRAP_FWIF_P_UNLICENSED_ALGO_EVENT_ID	\
	((uint32_t)((BRAP_FWIF_P_STREAM_INFO_AVAIL_EVENT_ID) + 1))					/** 0x18 **/

#define BRAP_FWGFX_FILL_CMD_COMPLETION_EVENT_ID \
    ((uint32_t)((BRAP_FWIF_P_UNLICENSED_ALGO_EVENT_ID) + 1))					/** 0x19 **/

#define BRAP_FWGFX_BLIT_CMD_COMPLETION_EVENT_ID \
    ((uint32_t)((BRAP_FWGFX_FILL_CMD_COMPLETION_EVENT_ID) + 1))					/** 0x1A **/

#define BRAP_FWGFX_GFX_CMD_COMPLETION_EVENT_ID	\
    ((uint32_t)((BRAP_FWGFX_BLIT_CMD_COMPLETION_EVENT_ID) + 1))					/** 0x1B **/


#define BRAP_BSP_SCM_RESPONSE_EVENT_ID	\
    ((uint32_t)((BRAP_FWGFX_GFX_CMD_COMPLETION_EVENT_ID) + 1))					/** 0x1C **/

#define BRAP_FWIF_P_LAST_EVENT_ID \
    ((uint32_t)BRAP_BSP_SCM_RESPONSE_EVENT_ID)									/** 0x1D **/

#define BRAP_FWIF_P_MAX_ASYNC_EVENT   \
 	((uint32_t)(BRAP_FWIF_P_LAST_EVENT_ID - BRAP_FWIF_P_ASYNC_EVENT_ID_OFFSET))	/** 0x1E **/
    




/*** These are unique Command Ids used by Host ***/

#define BRAP_FWIF_P_START_TASK_COMMAND_ID   \
        ((uint32_t)((BRAP_FWIF_P_COMMAND_ID_OFFSET) + 1))				/** 0x101 **/

#define BRAP_FWIF_P_STOP_TASK_COMMAND_ID    \
        ((uint32_t)((BRAP_FWIF_P_START_TASK_COMMAND_ID) + 1))			/** 0x102 **/

#define BRAP_FWIF_P_ALGO_PARAMS_CFG_COMMAND_ID  \
        ((uint32_t)((BRAP_FWIF_P_STOP_TASK_COMMAND_ID) + 1))			/** 0x103 **/

#define BRAP_FWIF_P_PAUSE_COMMAND_ID        \
        ((uint32_t)((BRAP_FWIF_P_ALGO_PARAMS_CFG_COMMAND_ID) + 1))		/** 0x104 **/

#define BRAP_FWIF_P_FRAME_ADVANCE_COMMAND_ID        \
        ((uint32_t)((BRAP_FWIF_P_PAUSE_COMMAND_ID) + 1))				/** 0x105 **/

#define BRAP_FWIF_P_RESUME_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_FRAME_ADVANCE_COMMAND_ID) + 1))		/** 0x106 **/

#define BRAP_FWIF_P_EVENT_NOTIFICATION_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_RESUME_COMMAND_ID ) + 1))				/** 0x107 **/

#define BRAP_FWIF_P_FMMPORT_RECFG_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_EVENT_NOTIFICATION_COMMAND_ID ) + 1))	/** 0x108 **/

#define BRAP_FWIF_P_PING_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_FMMPORT_RECFG_COMMAND_ID ) + 1))		/** 0x109 **/

#define BRAP_FWGFX_P_FILL_COMMAND_ID  \
        ((uint32_t)((BRAP_FWIF_P_PING_COMMAND_ID ) + 1))		        /** 0x10A **/

#define BRAP_FWGFX_P_BLIT_COMMAND_ID  \
        ((uint32_t)((BRAP_FWGFX_P_FILL_COMMAND_ID ) + 1))		        /** 0x10B **/

#define BRAP_FWGFX_P_GFX_COMPLETION_CMD_ID	\
		((uint32_t)((BRAP_FWGFX_P_BLIT_COMMAND_ID) + 1))				/** 0x10C **/	

#define BRAP_FWIF_P_NUM_PIC_TO_DROP_COMMAND_ID       \
        ((uint32_t)((BRAP_FWGFX_P_GFX_COMPLETION_CMD_ID ) + 1))			/** 0x10D **/

#define BRAP_FWIF_P_CIT_RECONFIGURATION_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_NUM_PIC_TO_DROP_COMMAND_ID ) + 1))		/** 0x10E **/

#define BRAP_FWIF_P_BSP_SCM_COMMAND_ID       \
        ((uint32_t)((BRAP_FWIF_P_CIT_RECONFIGURATION_COMMAND_ID) + 1))		/** 0x10F **/
       



/*** Following are the Ack ids for different commands ***/

#define BRAP_FWIF_P_START_TASK_ACK_ID   \
        ((uint32_t)((BRAP_FWIF_P_ACK_ID_OFFSET) + 1))						/** 0x201 **/
	
#define BRAP_FWIF_P_STOP_TASK_ACK_ID        \
        ((uint32_t)((BRAP_FWIF_P_START_TASK_ACK_ID) + 1))					/** 0x202 **/

#define BRAP_FWIF_P_ALGO_PARAMS_CFG_COMMAND_ACK_ID            \
        ((uint32_t)((BRAP_FWIF_P_STOP_TASK_ACK_ID) + 1))					/** 0x203 **/

#define BRAP_FWIF_P_PAUSE_ACK_ID            \
        ((uint32_t)((BRAP_FWIF_P_ALGO_PARAMS_CFG_COMMAND_ACK_ID) + 1))		/** 0x204 **/

#define BRAP_FWIF_P_FRAME_ADVANCE_ACK_ID    \
        ((uint32_t)((BRAP_FWIF_P_PAUSE_ACK_ID ) + 1))						/** 0x205 **/

#define BRAP_FWIF_P_RESUME_ACK_ID           \
        ((uint32_t)((BRAP_FWIF_P_FRAME_ADVANCE_ACK_ID) + 1))				/** 0x206 **/

#define BRAP_FWIF_P_EVENT_NOTIFICATION_ACK_ID       \
        ((uint32_t)((BRAP_FWIF_P_RESUME_ACK_ID ) + 1))						/** 0x207 **/


#define BRAP_FWIF_P_FMMPORT_RECFG_ACK_ID       \
        ((uint32_t)((BRAP_FWIF_P_EVENT_NOTIFICATION_ACK_ID ) + 1))			/** 0x208 **/

#define BRAP_FWIF_P_PING_ACK_ID       \
        ((uint32_t)((BRAP_FWIF_P_FMMPORT_RECFG_ACK_ID ) + 1))				/** 0x209 **/


#define BRAP_FWGFX_P_FILL_ACK_ID       \
         ((uint32_t)((BRAP_FWIF_P_PING_ACK_ID ) + 1))                       /** 0x20A **/
 
#define BRAP_FWGFX_P_BLIT_ACK_ID       \
         ((uint32_t)((BRAP_FWGFX_P_FILL_ACK_ID ) + 1))                      /** 0x20B **/        

#define BRAP_FWGFX_P_CMD_COMPLETION_ACK_ID       \
         ((uint32_t)((BRAP_FWGFX_P_BLIT_ACK_ID) + 1))                       /** 0x20C **/        

#define BRAP_FWIF_P_NUM_PIC_TO_DROP_ACK_ID       \
        ((uint32_t)((BRAP_FWGFX_P_CMD_COMPLETION_ACK_ID ) + 1))				/** 0x20D **/

#define BRAP_FWIF_P_BSP_SCM_COMMAND_ACK_ID       \
		  ((uint32_t)((BRAP_FWIF_P_NUM_PIC_TO_DROP_ACK_ID ) + 1))		    /** 0x20E **/


        


 /*** The following are the various Response Ids used for different commands  ***/

#define BRAP_FWIF_P_START_TASK_RESPONSE_ID   \
        ((uint32_t)((BRAP_FWIF_P_RESPONSE_ID_OFFSET) + 1))				/** 0x301 **/

#define BRAP_FWIF_P_STOP_TASK_RESPONSE_ID        \
        ((uint32_t)((BRAP_FWIF_P_START_TASK_RESPONSE_ID) + 1))          /** 0x302 **/

#define BRAP_FWIF_P_ALGO_PARAMS_CFG_RESPONSE_ID            \
        ((uint32_t)((BRAP_FWIF_P_STOP_TASK_RESPONSE_ID) + 1))           /** 0x303 **/

#define BRAP_FWIF_P_PAUSE_RESPONSE_ID            \
        ((uint32_t)((BRAP_FWIF_P_ALGO_PARAMS_CFG_RESPONSE_ID) + 1))		/** 0x304 **/

#define BRAP_FWIF_P_FRAME_ADVANCE_RESPONSE_ID    \
        ((uint32_t)((BRAP_FWIF_P_PAUSE_RESPONSE_ID ) + 1))              /** 0x305 **/

#define BRAP_FWIF_P_RESUME_RESPONSE_ID           \
        ((uint32_t)((BRAP_FWIF_P_FRAME_ADVANCE_RESPONSE_ID) + 1))       /** 0x306 **/

#define BRAP_FWIF_P_EVENT_NOTIFICATION_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWIF_P_RESUME_RESPONSE_ID ) + 1))				/** 0x307 **/

#define BRAP_FWIF_P_FMMPORT_RECFG_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWIF_P_EVENT_NOTIFICATION_RESPONSE_ID ) + 1))	/** 0x308 **/

#define BRAP_FWGFX_P_FILL_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWIF_P_FMMPORT_RECFG_RESPONSE_ID ) + 1))	    /** 0x309 **/

#define BRAP_FWGFX_P_BLIT_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWGFX_P_FILL_RESPONSE_ID ) + 1))	            /** 0x30A **/        

#define BRAP_FWGFX_P_CMD_COMPLETION_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWGFX_P_BLIT_RESPONSE_ID ) + 1))	            /** 0x30B **/        

#define BRAP_FWIF_P_CIT_RECONFIGURATION_RESPONSE_ID       \
        ((uint32_t)((BRAP_FWGFX_P_CMD_COMPLETION_RESPONSE_ID ) + 1))	/** 0x30C **/
               


#define BRAP_FWIF_P_BSP_SCM_COMMAND_RESPONSE_ID       \
		 ((uint32_t)((BRAP_FWIF_P_CIT_RECONFIGURATION_RESPONSE_ID ) + 1))/** 0x30E **/

        


/***************************************************************************
Summary: 
    Enum indicating Mask bit for Enabling/Disabling specific event for a task.

See Also:
****************************************************************************/

typedef enum BRAP_FWIF_P_EventIdMask
{
    BRAP_FWIF_P_EventIdMask_eNone                       = 0x00000000,
    BRAP_FWIF_P_EventIdMask_eDummyFrameGeneration		= 0x00000001, 
	BRAP_FWIF_P_EventIdMask_eFrameRepeat				= 0x00000002, 
	BRAP_FWIF_P_EventIdMask_eFrameDropFromCdb			= 0x00000004, 
	BRAP_FWIF_P_EventIdMask_eTsmFail					= 0x00000008, 
	BRAP_FWIF_P_EventIdMask_eCdbItbUnderflow			= 0x00000010, 
	BRAP_FWIF_P_EventIdMask_eCdbItbOverflow				= 0x00000020,
	BRAP_FWIF_P_EventIdMask_eCrcError					= 0x00000040, 
	BRAP_FWIF_P_EventIdMask_eAudioModeChange			= 0x00000080,
	BRAP_FWIF_P_EventIdMask_eBitRateChange				= 0x00000100, 
	BRAP_FWIF_P_EventIdMask_eSampleRateChange			= 0x00000200, 
	BRAP_FWIF_P_EventIdMask_eFrameSyncLock				= 0x00000400, 
	BRAP_FWIF_P_EventIdMask_eFrameSyncLockLost			= 0x00000800, 
	BRAP_FWIF_P_EventIdMask_eSTC_PTS_DiffLowerThreshold	= 0x00001000, 
	BRAP_FWIF_P_EventIdMask_eSTC_PTS_DiffUpperThreshold	= 0x00002000, 
	BRAP_FWIF_P_EventIdMask_eDesiredCDB_BufferLevel		= 0x00004000, 
	BRAP_FWIF_P_EventIdMask_eFirstPTS_Received			= 0x00008000, 
	BRAP_FWIF_P_EventIdMask_ePTS_error					= 0x00010000,
	BRAP_FWIF_P_EventIdMask_eTSM_Lock					= 0x00020000,
	BRAP_FWIF_P_EventIdMask_eStartOnPTS					= 0x00040000,
	BRAP_FWIF_P_EventIdMask_eStopOnPTS					= 0x00080000,
	BRAP_FWIF_P_EventIdMask_eAstmTsmPass				= 0x00100000,	
	BRAP_FWIF_P_EventIdMask_eRampEnable					= 0x00200000,		
	BRAP_FWIF_P_EventIdMask_eCDBDataAvail				= 0x00400000,
	BRAP_FWIF_P_EventIdMask_eStreamInfoAvail			= 0x00800000,
	BRAP_FWIF_P_EventIdMask_eUnlicensedAlgo				= 0x01000000,
	BRAP_FWIF_P_EventIdMask_eGFXFillCmdCompleted        = 0x02000000,
    BRAP_FWIF_P_EventIdMask_eGFXBlitCmdCompleted        = 0x04000000,
	BRAP_FWIF_P_EventIdMask_eGFXCompletionCmd			= 0x08000000,    

	BRAP_FWIF_P_EventIdMask_eBspScmResponse 			= 0x10000000,	 
    BRAP_FWIF_P_EventIdMask_eLast,
    BRAP_FWIF_P_EventIdMask_eAll                        = 0x1FFFFFFF,   /* Keep updating this All */
   
    BRAP_FWIF_P_EventIdMask_eInvalid					= 0x7FFFFFFF

} BRAP_FWIF_P_EventIdMask;


/***************************************************************************
Summary: 
    Enum indicating the task type.

See Also:
****************************************************************************/

typedef enum BRAP_FWIF_P_TaskType
{
    BRAP_FWIF_P_TaskType_eNonRealtime = 0, 
    BRAP_FWIF_P_TaskType_eSoftRealtime,
    BRAP_FWIF_P_TaskType_eRealtime,
	BRAP_FWIF_P_TaskType_eSlave,

    BRAP_FWIF_P_TaskType_eLast,
    BRAP_FWIF_P_TaskType_eInvalid = 0x7FFFFFFF

} BRAP_FWIF_P_TaskType;

/***************************************************************************
Summary: 
    Enum indicating the type of the deadline computation function to be used
    for the task.
    
See Also:
****************************************************************************/

typedef enum BRAP_FWIF_P_DeadLineComputeFuncType
{
    BRAP_FWIF_P_DeadLineComputeFuncType_eRealtimeDecode = 0,
    BRAP_FWIF_P_DeadLineComputeFuncType_eRealtimeEncode,
    BRAP_FWIF_P_DeadLineComputeFuncType_eNonRealtimeDecode,
    BRAP_FWIF_P_DeadLineComputeFuncType_eNonRealtimeEncode,
    BRAP_FWIF_P_DeadLineComputeFuncType_ePassthrough,
    BRAP_FWIF_P_DeadLineComputeFuncType_eGraphicsTask,
    BRAP_FWIF_P_DeadLineComputeFuncType_eScmTask,

    BRAP_FWIF_P_DeadLineComputeFuncType_eMax,
    BRAP_FWIF_P_DeadLineComputeFuncType_eInvalid = 0x7FFFFFFF

} BRAP_FWIF_P_DeadLineComputeFuncType;



/***************************************************************************
Summary: 
    Enum indicating the Algo type.

See Also:
****************************************************************************/

typedef enum BRAP_FWIF_P_AlgoType
{           
	BRAP_FWIF_P_AlgoType_eEncode = 10,
	BRAP_FWIF_P_AlgoType_eDecode,
	BRAP_FWIF_P_AlgoType_eTranscode,
	BRAP_FWIF_P_AlgoType_ePassThru,
	BRAP_FWIF_P_AlgoType_eGfxDecode,
	BRAP_FWIF_P_AlgoType_eVideoDecode,	
	BRAP_FWIF_P_AlgoType_eScm,

	BRAP_FWIF_P_AlgoType_eLast,
	BRAP_FWIF_P_AlgoType_eInvalid = 0x7FFFFFFF

} BRAP_FWIF_P_AlgoType;



/***************************************************************************
Summary: 
    Enum indicating whether Ack or Response required by Host

See Also:
****************************************************************************/

typedef enum BRAP_FWIF_P_ResponseType
{           
	BRAP_FWIF_P_ResponseType_eAckRequired = 0,
	BRAP_FWIF_P_ResponseType_eResponseRequired,
	BRAP_FWIF_P_ResponseType_eAckAndResponseBothRequired,
	BRAP_FWIF_P_ResponseType_eNone,
	BRAP_FWIF_P_ResponseType_eLast,
	BRAP_FWIF_P_ResponseType_eInvalid = 0x7FFFFFFF

} BRAP_FWIF_P_ResponseType;



/***************************************************************************
Summary: 
    Structure holding the common header for all commands between the host 
    and the DSP.
    
Description:
    This is the common header for all Commands from the host.
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_CommandHeader
{   
    uint32_t    ui32CommandID;              /*  Command ID */   
    uint32_t    ui32CommandSizeInBytes;     /*  Command Size in Bytes */    
    uint32_t    ui32CommandCounter;         /*  Command Counter */  
    uint32_t    ui32TaskID;                 /*  Task ID*/   
    BRAP_FWIF_P_ResponseType	eResponseType;	/*  Ack or response required */
} BRAP_FWIF_P_CommandHeader;



/***************************************************************************
Summary: 
    This structure holds all the information required by DSP for scheduling.

	 This structure is derived from the BRAP_FWIF_P_TaskParamInfo by ATM.
	 DSP uses this structure internally	
    
See Also:
****************************************************************************/

typedef union BRAP_FWIF_P_DspSchedulingBuffInfo
{
	uint32_t							ui32DramSchedulingBuffCfgAddr; /*	Handle for the Scheduling Buffer.
																			Used only for BRAP_AF_P_BufferType_eDRAM*/

	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		sRdbBasedSchedulingBuffer;	   /*	Used only for buffer of type FMM/CDB/RDB */

}BRAP_FWIF_P_DspSchedulingBuffInfo;

/***************************************************************************
Summary: 
    This structure holds all the information required by DSP for scheduling 
	the newly created task by Host
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_NonRTTaskParamInfo
{
    uint32_t    ui32MinTimeThreshold;
    uint32_t    ui32MaxTimeSlice;
}BRAP_FWIF_P_NonRTTaskParamInfo;

typedef struct BRAP_FWIF_P_TaskParamInfo
{
   uint32_t							ui32SamplingFrequency;
   uint32_t							ui32FrameSize;
   uint32_t							ui32SoftRealTimeThreshold;
   uint32_t							ui32Threshold;
   uint32_t							ui32BlockTime;
   uint32_t							ui32NonRealTimeRate;

   /*  Following Circ buffer details are used for task scheduling by FW CPT, 
	   also this is used by TSM for Buffer delay computation.

	   PI will populates the 'sRdbBasedSchedulingBuffer' for RDB based buffers.

	   Incase of DRAM intertask buffer as the scheduling buffer,
	     PI will take Buffer Handle of the intertask Dram buffer and hook to ui32DramSchedulingBuffCfgAddr
   */

	BRAP_AF_P_BufferType				eBufferType;

	BRAP_FWIF_P_DspSchedulingBuffInfo   sDspSchedulingBuffInfo;

   /* 
	  Feedback buffer from the Master Task to the Slave Tasks.

      Currently we support only one feedback buffer from the Master Task to all its slave tasks.

	  The concept of Feedback path exists only in the Master Slave configuration and the feed back path
	  becomes invalid in the single decode usage modes. PI has to make Valid/Invalid field according.

	  PI allocates the Feedback buffer when a master task is started and allocates a DRAM memory of 
	  type BRAP_AF_P_sSINGLE_CIRC_BUFFER to populate the circullar buffer details.  PI passes the base address 
	  of this DRAM memory to FW as Handle so that FW can take care of all kinds of buffers including 
	  the BRAP_AF_P_BufferType_eDRAM.	  

	  PI gives the same handle for all the slave tasks of a master task..	  

	  The 'ui32MasterTaskFeedbackBuffCfgAddr' is the converted address...
   
   */ 
   
   uint32_t							ui32MasterTaskFeedbackBuffValid;	/* Valid =1; Invalid 0 */
   uint32_t							ui32MasterTaskFeedbackBuffCfgAddr;


   uint32_t							ui32MaxIndepDelay ;
   uint32_t							ui32IndepDelay ;
   BRAP_FWIF_P_NonRTTaskParamInfo   sNonRealTimeTaskParams;

	
   BRAP_AF_P_FmmDstFsRate			eFmmDstFsRate; /* The sinking rate of Scheduling buffer.
													  The sinking buffer should be selected as the fastest sinking buffer in the task.
													  This will ensure the jitter in 4X rate FMM buffers getting accounted for scheduling */
}BRAP_FWIF_P_TaskParamInfo;


/***************************************************************************
Summary: 
    Start task command structure.
    
Description:
    This is the command structure of the start task command to be issued by
    the host on start decode.
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_StartTaskCommand
{
    BRAP_FWIF_P_AlgoType				eTaskAlgoType; /*  Algorithm type enum */
    BRAP_FWIF_P_TaskType                eTaskType;          /*  Task type:Real Time, 
                                                                        Non real time */

	uint32_t							ui32MasterTaskId;

    BRAP_FWIF_P_DeadLineComputeFuncType eDeadlineComputationFuncType;
    uint32_t							ui32DramDeadlineConfigStructAddr; /* This is pointer to this
																			structure BRAP_FWIF_P_TaskParamInfo
																			in DRAM */
    uint32_t							ui32DramTaskConfigAddr;	/* Task config data structure address
																			from CIT module */
    uint32_t							ui32SyncQueueId;		/* Task Sync queue Id */
    uint32_t							ui32AsyncQueueId;		/* Task Async queue Id */
	BRAP_AF_P_sDRAM_BUFFER				sDramStackBuffer;		/* Stack DRAM address for stack swapping for each task */
	uint32_t							ui32EventEnableMask;

	BRAP_AF_P_EnableDisable				ePPMCorrEnable;  /* PPM correction enable or not */

	BRAP_AF_P_sDRAM_BUFFER				sDramTaskCodeBuffer;	/* Code pointer and size of Task code */

} BRAP_FWIF_P_StartTaskCommand;



typedef struct BRAP_FWIF_P_StopTaskCommand
{
	uint32_t	eTaskType;			/* Task Type*/
	uint32_t	ui32MasterTaskId;
}BRAP_FWIF_P_StopTaskCommand;




/***************************************************************************
Summary:
    Generic configuration change command structure.
    
Description:
    Any configuration change command will have a generic structure. The
    configuration change will only have three parameters. The DRAM address
    location where the host has changed the configuration. The DRAM location
    currently being used by the DSP and the size of the configuration 
    structure.
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_CfgChangeCommand
{
    uint32_t                    ui32HostConfigParamBuffAddr;
    uint32_t                    ui32DspConfigParamBuffAddr;
    uint32_t                    ui32SizeInBytes;

} BRAP_FWIF_P_CfgChangeCommand;


/***************************************************************************
Summary:
     Frame Advance command structure.
    
Description:
        
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_FrameAdvanceCommand
{
    TIME_45KHZ_TICKS		ui32DurationOfFrameAdv; /*  Duration for the frame 
                                                            advance */
} BRAP_FWIF_P_FrameAdvanceCommand;


/***************************************************************************
Summary:
     Event enable/disable command structure.
    
Description:
        
See Also:
****************************************************************************/

typedef struct  BRAP_FWIF_P_EventEnableDisable
{
    uint32_t ui32EnableEvent; /* Use  BRAP_FWIF_P_EventIdMask for enabling or
								 disabling specfic events by Host, We need to 
								 take care of starting 0x301 offset for event Ids */
} BRAP_FWIF_P_EventEnableDisable;


/***************************************************************************
Summary:
     Ping command structure.
    
Description:
        
See Also:
****************************************************************************/

typedef struct  BRAP_FWIF_P_PingCommand
{
    uint32_t    ui32AckQueueId; /* Ack queue Id for ping command */
} BRAP_FWIF_P_PingCommand;



/***************************************************************************
Summary:
    Fmm-Port Reconfig Command configuration change command structure.
    
Description:
		
	This structure contain two DRAM Addresses.
		a) ui32NewFMMPortBuffAdd :	Buffer that contains new FMM Buffer
									port cfg.
									This buffer is a temporary buffer 
									allocated in PI.

		b) ui32TaskFMMPortCfgBuffAddr : The task FMM port cfg addr that was
									allocated by CIT.

  
	Upon the FMM port Reconfiguration command, the FW is expected to copy
	the data in ui32NewFMMPortBuffAdd to ui32TaskFMMPortCfgBuffAddr and 
	perform the FMM port Reconfiguration.
	
	FW will send a response once the Command operations are done....
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_FmmPortReconfigCommand     
{	
    uint32_t                    ui32NewFMMPortBuffAdd;
    uint32_t                    ui32TaskFMMPortCfgBuffAddr;
    uint32_t                    ui32SizeInBytes;

} BRAP_FWIF_P_FmmPortReconfigCommand;


/***************************************************************************
Summary:
    Cit Reconfiguration Command    

Description:
	This structure contain two DRAM Addresses 
		a) ui32ModifiedCitAddr	 :Buffer that contains new CIT data structure
								  This buffer is a temporary buffer 
								  allocated in PI.

		b) ui32RunningTaskCitAddr: The CIT data structure of the running task

		All these addresses should be Converted Address.

		and

		c) CIT Datastructure Size
    
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_CitReconfigCommand
{
    uint32_t                    ui32ModifiedCitAddr;
    uint32_t                    ui32RunningTaskCitAddr;
    uint32_t                    ui32SizeInBytes;

} BRAP_FWIF_P_CitReconfigCommand;

/***************************************************************************
Summary:
     Common structure for all firmware commands
        
Description: Currently, it is assumed that all the commands posted by Host will be
			 of fixed size, which is sizeof(BRAP_FWIF_P_Command) data structure. 
			 It will be easier in future if we need to skip specific commands 
			 from the queue, which can be easily achieved by moving with a 
			 fixed size offset for each command in the queue.
See Also:
****************************************************************************/

typedef uint32_t BRAP_GFX_P_Pixel;

typedef enum BRAP_GFX_P_ePixelFormatSupported
{
    BRAP_GFX_P_PixelFormat_ePalette8,
    BRAP_GFX_P_PixelFormat_eA8_R8_G8_B8,
    BRAP_GFX_P_PixelFormat_eR8_G8_B8_A8,
    BRAP_GFX_P_PixelFormat_eR5_G6_B5,
    BRAP_GFX_P_PixelFormat_eA1_R5_G5_B5,
    BRAP_GFX_P_PixelFormat_eR5_G5_B5_A1,
    BRAP_GFX_P_PixelFormat_eA4_R4_G4_B4,
    BRAP_GFX_P_PixelFormat_eR4_G4_B4_A4,
    BRAP_GFX_P_PixelFormat_eA8_Y8_Cb8_Cr8,
    BRAP_GFX_P_PixelFormat_eInvalid
}BRAP_GFX_P_ePixelFormatSupported;

typedef enum BRAP_GFX_P_eOperationsFill
{
    BRAP_GFX_P_eIgnore,   /* Do not change the values for the channel (either color or alpha) */
    BRAP_GFX_P_eCopy,     /* Copy the value of the channel (color or alpha) from the constant color to the surface */
    BRAP_GFX_P_eBlend,    /* For color channel, blend surface color with constant color using constant alpha.
                                     This operation is not support for the alpha channel. */
    BRAP_GFX_P_eInvalid,
    BRAP_GFX_P_eMax = 0x7fffffff
}BRAP_GFX_P_eOperationsFill;

typedef enum BRAP_GFX_P_eColorOperationsBlit
{
    BRAP_GFX_P_eColorCopySource,      /* Copy the source color with no blending. Valid only with B_SoftGfx_BlitAlphaOp_eCopySource. This means Blitting. */
    BRAP_GFX_P_eColorUseConstantAlpha,    /* Blend the source and dest colors using the alpha from the constAlphaColorOp param */
    BRAP_GFX_P_eColorUseSourceAlpha,      /* Blend the source and dest colors using the alpha from the source pixels */
    BRAP_GFX_P_eColorUseDestinationAlpha, /* Blend the source and dest colors using the alpha from the dest pixels */
    BRAP_GFX_P_eColorUseBlendEquation,    /* Blend the source and dest colors using constant alpha for color & blend operations */
    BRAP_GFX_P_eColorIgnore,
    BRAP_GFX_P_eColorInvalid,
    BRAP_GFX_P_eColorMax = 0x7fffffff
}BRAP_GFX_P_eColorOperationsBlit;


typedef enum BRAP_GFX_P_eAlphaOperationsBlit
{
    BRAP_GFX_P_eAlphaCopySource,          /* Oa = Sa. Copy the source alpha to the output alpha. */
    BRAP_GFX_P_eAlphaCopyDestination,     /* Oa = Da. Copy the dest alpha to the output alpha */
    BRAP_GFX_P_eAlphaCopyConstant,        /* Oa = constAlphaAlphaOp. Copy the constant alpha for alphaOp parameter as the output alpha */    
    BRAP_GFX_P_eAlphaUseBlendEquation,    /* Alpha = 1 - (1 - Alpha1) * (1 - Alpha2) 
                                                     where, Alpha1 and Alpha2 are defined as per the BlitColorOp given 
                                                     in the description of B_SoftGfx_BlitColorOp above */
	BRAP_GFX_P_eAlphaCombine,             /* Oa = Sa*Sa + Da*(1-Sa). Combine source and dest alpha. */
    BRAP_GFX_P_eAlphaIgnore,
    BRAP_GFX_P_eAlphaInvalid,
    BRAP_GFX_P_eAlphaMax  = 0x7fffffff
}BRAP_GFX_P_eAlphaOperationsBlit;

typedef struct BRAP_GFX_P_BlkSize
{
    uint32_t height;
    uint32_t width;
}BRAP_GFX_P_BlkSize;



typedef struct BRAP_GFX_P_Dimension
{
    BRAP_GFX_P_BlkSize    blockSize;
    uint32_t pitch;
}BRAP_GFX_P_Dimension;

typedef struct BRAP_GFX_P_Coordinate
{
    uint32_t    x;
    uint32_t    y;
}BRAP_GFX_P_Coordinate;

typedef struct BRAP_GFX_P_Rect
{
    BRAP_GFX_P_Coordinate sStartXY;
    BRAP_GFX_P_BlkSize    sBlkSize;
}BRAP_GFX_P_Rect;

typedef struct BRAP_GFX_P_Palette
{
    BRAP_AF_P_sDRAM_BUFFER          sDramPaletteBuffer;
    uint32_t                        ui32NumEntries;
    BRAP_GFX_P_ePixelFormatSupported  ePalettePixelFormat;
}BRAP_GFX_P_Palette;


typedef struct BRAP_GFX_P_SurfaceStruct
{

    BRAP_AF_P_sDRAM_BUFFER          sDramSurfaceBuffer;
    BRAP_GFX_P_ePixelFormatSupported  ePixelFormat;
    BRAP_GFX_P_Palette                sPalette;
    BRAP_GFX_P_Dimension              sSurfaceDimension;
    uint32_t                        ui32BitsPerPixel;
 
}BRAP_GFX_P_SurfaceStruct;


typedef struct BRAP_GFX_P_Fill
{
    BRAP_GFX_P_SurfaceStruct  sOutSurface;
    BRAP_GFX_P_Rect           sFillRect;
    BRAP_GFX_P_Pixel          ui32FillColor;

    BRAP_GFX_P_eOperationsFill   eColorOperation;
    BRAP_GFX_P_eOperationsFill   eAplhaOperation;
}BRAP_GFX_P_Fill;

#define BRAP_COLOR_P_MATRIX_COEFF_COUNT   20


typedef struct BRAP_GFX_P_CSCMatrix
{
    uint32_t ui32Shift;
    int32_t  coeffMatrix[BRAP_COLOR_P_MATRIX_COEFF_COUNT];
} BRAP_GFX_P_CSCMatrix;


typedef enum BRAP_GFX_P_eScalingFilter
{
    BRAP_GFX_P_eScalingFilter_eLinear,
    BRAP_GFX_P_eScalingFilter_eGaussian,
    BRAP_GFX_P_eScalingFilter_eInvalid,
	BRAP_GFX_P_eScalingFilter_eMax = 0x7fffffff
}BRAP_GFX_P_eScalingFilter;

typedef  struct BRAP_GFX_P_ColorKeyParams    
{
    struct {
        uint32_t ui32ColorKeyEnabled;     /* Enable colorkey. If disabled, the following values are ignored. */
        uint32_t ui32ColorKeyLower;   /* Lower colorkey value. */
        uint32_t ui32ColorKeyUpper;   /* Upper colorkey value.*/
        uint32_t ui32ColorKeyMask;    /* Mask to use when comparing color key to source color */
        uint32_t ui32ColorKeyReplace; /* Replacement color */
        uint32_t ui32ColorKeyRplMask; /*  Mask for replacement color */
    } sSource,sDest;
} BRAP_GFX_P_ColorKeyParams;


typedef struct BRAP_GFX_P_Blit
{
    BRAP_GFX_P_SurfaceStruct  sSrcSurface;
    BRAP_GFX_P_SurfaceStruct  sDstSurface;
    BRAP_GFX_P_SurfaceStruct  sOutSurface;

    BRAP_GFX_P_Rect           sSrcRect;
    BRAP_GFX_P_Rect           sDstRect;
    BRAP_GFX_P_Rect           sOutRect;


    BRAP_GFX_P_CSCMatrix      sCSCMatrix;

    BRAP_GFX_P_eColorOperationsBlit   eColorOperation;
    BRAP_GFX_P_eAlphaOperationsBlit   eAlphaOperation;

    uint32_t                constAlphaColorOp;
    uint32_t                constAlphaAlphaOp;

    BRAP_AF_P_EnableDisable          eApplyAFF;
    BRAP_AF_P_EnableDisable          eApplyCSC;
 	BRAP_GFX_P_eScalingFilter        eScalingFilter;

    BRAP_GFX_P_ColorKeyParams		 sGfxColorKeyParams;

}BRAP_GFX_P_Blit;



typedef struct BRAP_GFX_P_TaskParamInfo
{
    uint32_t    ui32MinTimeThreshold;
    uint32_t    ui32MaxTimeSlice;
}BRAP_GFX_P_TaskParamInfo;



typedef BRAP_FWIF_P_StopTaskCommand BRAP_FWIF_GFX_P_StopTaskCommand;

typedef struct BRAP_FWIF_GFX_P_CmdOperation
{
    BRAP_AF_P_sDRAM_BUFFER  sCmdBufferInfo;
}BRAP_FWIF_GFX_P_CmdOperation;

typedef struct BRAP_FWIF_GFX_P_ResponseCmd
{
	uint32_t				ui32CmdCnt;
}BRAP_FWIF_GFX_P_ResponseCmd;


/***************************************************************************
Summary: 
    Number of picture to drop at decoder command structure.   

Description:

    This is the command structure of the NUM_PIC_TO_DROP command to be 
    issued by the host to video decoder to update the decoder whenever DM 
    increments it's picture drop counter.   

See Also:
****************************************************************************/
typedef struct BRAP_FWIF_P_NumPicToDropCommand
{
    uint32_t      ui32NumPicToDrop;      

} BRAP_FWIF_P_NumPicToDropCommand;

/***************************************************************************
							BSP Commands 
***************************************************************************/


/***************************************************************************
Summary: 
    BSP Command: Send SCM Commands to DSP   

Description:

    Host sends this command structure to DSP for SCM operations.  

****************************************************************************/

typedef struct BRAP_FWIF_SCM_P_CmdPayload
{
	uint32_t ui32DramCmdBufAddress;
	uint32_t ui32DramCmdBufSize;
	uint32_t ui32DramRespBufAddress;
	uint32_t ui32DramRespBufSize;
	
}BRAP_FWIF_SCM_P_CmdPayload;


typedef struct BRAP_FWIF_SCM_P_CmdOperation
{
    BRAP_FWIF_SCM_P_CmdPayload  sCmdBufferInfo;
}BRAP_FWIF_SCM_P_CmdOperation;


/***************************************************************************
Summary:
     Common structure for all firmware commands
        
Description: Currently, it is assumed that all the commands posted by Host will be
			 of fixed size, which is sizeof(BRAP_FWIF_P_Command) data structure. 
			 It will be easier in future if we need to skip specific commands 
			 from the queue, which can be easily achieved by moving with a 
			 fixed size offset for each command in the queue.
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_Command
{
    BRAP_FWIF_P_CommandHeader   sCommandHeader;
    union
    {
        BRAP_FWIF_P_StartTaskCommand					sStartTask;
		BRAP_FWIF_P_StopTaskCommand						sStopTask;
        BRAP_FWIF_P_CfgChangeCommand					sCfgChange;
        BRAP_FWIF_P_FrameAdvanceCommand					sFrameAdvance;
        BRAP_FWIF_P_EventEnableDisable					sEnableDisableEvent;
		BRAP_FWIF_P_FmmPortReconfigCommand				sFmmPortReconfig;
        BRAP_FWIF_P_PingCommand							sPing;
        BRAP_FWIF_GFX_P_CmdOperation        			sGfxFill;
        BRAP_FWIF_GFX_P_CmdOperation        			sGfxBlit;
		BRAP_FWIF_GFX_P_ResponseCmd						sGfxCmdCnt;
		BRAP_FWIF_P_NumPicToDropCommand					sNumPicToDropCommand;
		BRAP_FWIF_P_CitReconfigCommand					sCitReconfigCommand;
		BRAP_FWIF_SCM_P_CmdOperation		sScmCmd;

    } uCommand;
		
} BRAP_FWIF_P_Command;

/***************************************************************************
Summary: 
    This structure holds common header for both Ack and responses sent by 
	the DSP to the host in synchronous queue. 
	Only header is sufficient for sending any specifc Acks to the Host.
    
Description:
    This is the common header for Ack and Response messages sent by DSP;
    
See Also:
****************************************************************************/
typedef struct BRAP_FWIF_P_CommonAckResponseHeader
{
    uint32_t                    ui32ResponseID;             /*  Response ID */  
    uint32_t                    ui32ResponseSizeInBytes;    /*  Response Size in Bytes */   
    uint32_t                    ui32ResponseCounter;        /*  Response Counter */ 
    uint32_t                    ui32TaskID;                 /*  Task ID */  
    uint32_t                    eStatus;                    /*  Status of completion */ 
} BRAP_FWIF_P_CommonAckResponseHeader;

/***************************************************************************
Summary:
     Frame Advance Response structure.
    
Description:
        
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_FrameAdvanceResponse
{
    TIME_45KHZ_TICKS	ui32DurationOfFrmAdvCompleted;  /*  Duration for the frame 
                                                                    advance completed */
} BRAP_FWIF_P_FrameAdvanceResponse;



/***************************************************************************
Summary:
     Structure for Ack/Response message in sync queue
    
Description: Currently, it is assumed that all the sync responses posted by DSP will 
			 be of fixed size, which is sizeof(BRAP_FWIF_P_Response) data structure. 
			 It will be easier in future if Host needs to skip specific response 
			 message	in the sync queue for processing, which can be easily achieved 
			 by moving with a fixed size offset for each response in the sync queue.
        
			Only, header needs to be posted by DSP for any Ack to the Host.
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_Response
{
    BRAP_FWIF_P_CommonAckResponseHeader     sCommonAckResponseHeader;
    union
    {
        BRAP_FWIF_P_FrameAdvanceResponse				sFrameAdvance;	

    } uResponse;
} BRAP_FWIF_P_Response;


/***************************************************************************
Summary:
     Structure for async message header for any event
    
Description:
        
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_AsynMSGHeader
{   
    uint32_t    ui32EventID;            	/* Event ID */   
    uint32_t    ui32MsgSize;           		/* Size of Message in bytes */   
    uint32_t    ui32TaskID;             	/* Task ID for which the message occurs */   
}BRAP_FWIF_P_AsynMSGHeader;

/***************************************************************************
Summary:
     Structure for getting the TSM buffer ID info from FW
Description:
        
See Also:
****************************************************************************/
typedef struct BRAP_DSPCN_P_TsmFwInfo
{
	uint32_t	uiBufId;	
}BRAP_DSPCN_P_TsmFwInfo;

typedef struct BRAP_DSPCHN_P_FsToHost
{
       uint32_t      ui32ValidFlag;       /* 1: valid and 0: Invalid */
       uint32_t      ui32SamplingRate;    

} BRAP_DSPCHN_P_FsToHost;


/***************************************************************************
Summary:
     Structure for getting the Sample rate info from FW
Description:
        
See Also:
****************************************************************************/
typedef struct BRAP_DSPCHN_P_FwSampleinfo
{
        BRAP_DSPCHN_P_FsToHost        sBaseSamplingRate;
        BRAP_DSPCHN_P_FsToHost        sHDMISamplingRate;
        BRAP_DSPCHN_P_FsToHost        sHBRSamplingRate;
    
}BRAP_DSPCHN_P_FwSampleinfo;

/***************************************************************************
Summary:
     Structure for getting the Bit rate change info from FW
Description:
        
See Also:
****************************************************************************/
typedef struct BRAP_DSPCHN_P_FwBitRateChangeInfo
{
    uint32_t	ui32BitRate;    /* New Bit Rate value*/
    uint32_t 	ui32BitRateIndex;    /* This has the Bit rate index
    				as given in the standard. This value
    				is zero for audio type AAC-HE*/
}BRAP_DSPCHN_P_FwBitRateChangeInfo;

/***************************************************************************
Summary:
     Structure for getting the mode change info from FW
Description:

See Also:
****************************************************************************/
typedef struct BRAP_DSPCHN_P_FwModeChnageInfo
{
	uint32_t	ui32ModeValue;	
} BRAP_DSPCHN_P_FwModeChangeInfo;


/***************************************************************************
Summary:
     Structure for ramp enable event

Description: 
See Also:
****************************************************************************/
typedef struct BRAP_DSPCHN_RampEnableInfo
{
 uint32_t    ui32TimeDelay;   /* PI should enable the ramp after ui32TimeDelay(ms) */
 
} BRAP_DSPCHN_RampEnableInfo;


/***************************************************************************
Summary:
     Structure for unlicensed Algo event

Description: 
See Also:
****************************************************************************/
typedef struct BRAP_FWIF_P_UnsupportedAlgoInfo
{	
	uint32_t		ui32AudioAlgorithm; /* Algorithm Name */

} BRAP_FWIF_P_UnsupportedAlgoInfo;

/***************************************************************************
Summary:
     Structure for Graphics event

Description: 
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_GfxCommandCompletionInfo
{	
	uint32_t		ui32CommandId; 

} BRAP_FWIF_P_GfxCommandCompletionInfo;

typedef struct BRAP_FWIF_P_GfxResponseCmdCompletionInfo
{
	uint32_t		ui32CommandId;
	uint32_t		ui32CommandCntr;
}BRAP_FWIF_P_GfxResponseCmdCompletionInfo;


typedef struct BRAP_FWIF_GFX_P_FillCmdCompletion
{
    uint32_t        ui32CompletionStatus;
}BRAP_FWIF_GFX_P_FillCmdCompletion;

typedef struct BRAP_FWIF_GFX_P_BlitCmdCompletion
{
    uint32_t        ui32CompletionStatus;
}BRAP_FWIF_GFX_P_BlitCmdCompletion;


/***************************************************************************
Summary:
     This structure is used for Asynchronous Response from DSP for Code download
     Authentication operation

 
****************************************************************************/

typedef enum BRAP_FWIF_P_BspScmCodeAuthStatus
{           
	BRAP_FWIF_P_BspScmCodeAuthStatus_eFailure = 0x1,
	BRAP_FWIF_P_BspScmCodeAuthStatus_eLast	  = 0x7FFFFFFF
	
} BRAP_FWIF_P_BspScmCodeAuthStatus;

/***************************************************************************
Summary:
	This structure is used for Asynchronous Response from DSP for Code download
	Authentication operation
     
Description: 

See Also: BRAP_BspScmResponseType

****************************************************************************/

typedef struct BRAP_FWIF_P_BspScmCodeAuthInfo
{
    BRAP_FWIF_P_BspScmCodeAuthStatus     eBspScmCodeAuthStatus;

}BRAP_FWIF_P_BspScmCodeAuthInfo;


/***************************************************************************
Summary:
     Structure for Async message events

Description: Currently, it is assumed that all the async message events posted by 
			 DSP will be of fixed size, which is sizeof(BRAP_FWIF_P_AsynEventMsg) data 
			 structure. It will be easier in future if Host needs to skip specific event
			 message from async queue for processing, which can be easily achieved by 
			 moving with a fixed size offset for each async event message in the queue.
See Also:
****************************************************************************/

typedef struct BRAP_FWIF_P_AsynEventMsg
{
    BRAP_FWIF_P_AsynMSGHeader  sMsgHeader;
    union 
    {
        BRAP_DSPCHN_PtsInfo  				sPtsInfo;
        BRAP_DSPCN_P_TsmFwInfo 			    sTsmInfo;
        BRAP_DSPCHN_P_FwSampleinfo 		    sFwSampleInfo;
        BRAP_DSPCHN_P_FwModeChangeInfo 	    sFwModeChange;
        BRAP_DSPCHN_P_FwBitRateChangeInfo	sFwBitRateChange;
        BRAP_DSPCHN_RampEnableInfo  		sRampEnableInfo;   
		BRAP_FWIF_P_UnsupportedAlgoInfo					sUnsupportedAlgoInfo;
        BRAP_FWIF_P_GfxCommandCompletionInfo			sFillCommandCompletionInfo;
        BRAP_FWIF_P_GfxCommandCompletionInfo			sBlitCommandCompletionInfo;        
		BRAP_FWIF_P_GfxResponseCmdCompletionInfo		sGfxCommandCompletionInfo;
		BRAP_FWIF_P_BspScmCodeAuthInfo				sBspScmResponseInfo;
		
    }uFWMessageInfo;
}BRAP_FWIF_P_AsynEventMsg;


/* This define return the fixed size value for any command 
   This SIZEOF() macro return the value in bytes */
#define BRAP_FWIF_P_COMMAND_SIZE_IN_BYTES					SIZEOF(BRAP_FWIF_P_Command)

/* This define return the fixed size value for any Ack */
#define BRAP_FWIF_P_ACK_SIZE_IN_BYTES						SIZEOF(BRAP_FWIF_P_Response)

/* This define return the fixed size value for any Response */
#define BRAP_FWIF_P_RESPONSE_SIZE_IN_BYTES					SIZEOF(BRAP_FWIF_P_Response)

/* This define return the fixed size value for any Async Event */
#define BRAP_FWIF_P_ASYNC_RESPONSE_SIZE_IN_BYTES			SIZEOF(BRAP_FWIF_P_AsynEventMsg)

#endif

