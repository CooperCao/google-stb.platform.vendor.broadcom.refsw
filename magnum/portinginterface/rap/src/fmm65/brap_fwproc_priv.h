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
*	This file contains PI-FW Interface.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap_dspchn_priv.h"
#include "brap_fwif_priv.h"
#ifdef RAP_GFX_SUPPORT
#include "brap_gfx.h"
#endif

#define BRAP_MAX_DSP_SUPPORTED 1
#ifdef RAP_AUDIODESC_SUPPORT
#define BRAP_AUDDESC_TASK 1
#else
#define BRAP_AUDDESC_TASK 0
#endif

#if (BRAP_3548_FAMILY == 1)
#define BRAP_CAPTURE_TASK 1
#else
#define BRAP_CAPTURE_TASK 0
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
#define BRAP_MIXER_TASK 1
#else
#define BRAP_MIXER_TASK 0
#endif

#ifdef RAP_GFX_SUPPORT
#define BRAP_GFX_TASK 1
#else
#define BRAP_GFX_TASK 0
#endif

#ifdef RAP_SCM_SUPPORT
#define BRAP_SCM_TASK 1
#else
#define BRAP_SCM_TASK 0
#endif


#ifdef RAP_VIDEOONDSP_SUPPORT
#ifdef RAP_REALVIDEO_SUPPORT
#define BRAP_REALVIDEO_TASK 1
#endif
#else
#define BRAP_REALVIDEO_TASK 0
#endif

#ifdef RAP_SCM_SUPPORT
#define BRAP_MAX_FW_TSK_SUPPORTED (2 + BRAP_AUDDESC_TASK + BRAP_CAPTURE_TASK + BRAP_GFX_TASK + BRAP_REALVIDEO_TASK + BRAP_SCM_TASK )
#else
#define BRAP_MAX_FW_TSK_SUPPORTED (2 + BRAP_AUDDESC_TASK + BRAP_CAPTURE_TASK + \
                                BRAP_MIXER_TASK + BRAP_GFX_TASK + BRAP_REALVIDEO_TASK)
#endif

#define BRAP_MAX_FW_MAIN_EXEC_SUPPORTED BRAP_MAX_FW_TSK_SUPPORTED
#define BRAP_MAX_FW_TSM_EXEC_SUPPORTED BRAP_MAX_FW_TSK_SUPPORTED
#define BRAP_MAX_FW_FS_EXEC_SUPPORTED BRAP_MAX_FW_TSK_SUPPORTED

#define BRAP_MAX_MSGS_PER_QUEUE     5
#define BRAP_MAX_ASYNC_MSGS_PER_QUEUE     40
#define BRAP_FWIF_P_INVALID_TSK_ID		    ((unsigned int)(-1))
#define BRAP_FWIF_MSGQUE_ATTR_SIZE  16

#define BRAP_REALVIDEO_MAX_MSGS_PER_QUEUE   32
#define BRAP_REALVIDEO_TEMP_FRAME_BUF_SIZE 1000
#define BRAP_REALVIDEO_DECODE_LUMAFRAME_BUF_SIZE 368640
#define BRAP_REALVIDEO_DECODE_CHROMAFRAME_BUF_SIZE 184320  
#define BRAP_REALVIDEO_DECODE_REF_LUMAFRAME_BUF_SIZE 770048
#define BRAP_REALVIDEO_DECODE_REF_CHROMAFRAME_BUF_SIZE 385024  
#define BRAP_REALVIDEO_DISP_FRAME_BUF_SIZE  1000
#define BRAP_REALVIDEO_TEMP_UPB_BUF_SIZE    1000
#define BRAP_REALVIDEO_DECODE_UPB_BUF_SIZE  1000
#define BRAP_REALVIDEO_DISP_UPB_BUF_SIZE 1000
#define BRAP_REALVIDEO_DRAM_MB_BUF_SIZE 675000

#define BRAP_MAX_GFX_OP_IN_QUEUE   41
#ifdef RAP_GFX_SUPPORT
#define BRAP_ADDITIONAL_MSGS_FOR_GFX   ((BRAP_MAX_GFX_OP_SUPPORTED+1 -BRAP_MAX_MSGS_PER_QUEUE)*BRAP_GFX_TASK)
#else
#define BRAP_ADDITIONAL_MSGS_FOR_GFX   (0)
#endif



/* This structure is used to store base pointer & size of buffers used by 
   firmware like the interframe buffers & configparams buffers */
typedef struct BRAP_Fwif_P_FwBufInfo
{
	uint32_t				ui32BaseAddr;
	uint32_t				ui32Size;
}BRAP_Fwif_P_FwBufInfo;

#ifdef RAP_GFX_SUPPORT                   
typedef struct BRAP_Fwif_P_GfxBufPool
{
    bool                                bFree[BRAP_MAX_GFX_OP_IN_QUEUE];
    unsigned int                                uiGfxOpId[BRAP_MAX_GFX_OP_IN_QUEUE];
    BRAP_Fwif_P_FwBufInfo   sGfxBuffer[BRAP_MAX_GFX_OP_IN_QUEUE];
}BRAP_Fwif_P_GfxBufPool;
#endif

/* This structure contains iframe & cfgparams buffer memory req per task. This
   memory pool size is calculated as the worst case. This can be per DSP. This 
   can be extended later to not to use the worst case size. */
typedef struct BRAP_Fwif_P_TskMemRequirement
{
    uint32_t    ui32IframeCommon; /* Memory Pool Size required for Iframe of common exec */
    uint32_t    ui32IFramePAlgo; /* Memory Pool Size required for Iframe of Processing Algo */
    uint32_t    ui32CfgBufCommon; /* Memory Pool Size required for ConfigParams of common exec */
    uint32_t    ui32CfgBufPAlgo; /* Memory Pool Size required for ConfigParams of Processing Algo */
    uint32_t    ui32StatusBufCommon; /* Memory Pool Size required for Status buffer of common exec */    
    uint32_t    ui32StatusBufPAlgo; /* Memory Pool Size required for Status buffer of Processing Algo */    
    uint32_t    ui32CBitBuf;    /* Buffer for the CBIT Information */
    uint32_t    ui32ExtraBuf ;  /* Extra buffer required for 
                                                - Stack Swap
                                                - Task Port Config
                                                - Task FMM Gate */
}BRAP_Fwif_P_TskMemRequirement;

/* contains Task Sync & Async Queue info */
typedef struct BRAP_Fwif_P_TskQueues
{
    BRAP_FWIF_MsgQueueParams    sTskSyncQueue;  /* Synchronous queue */
    BRAP_FWIF_MsgQueueParams    sTskAsyncQueue;  /* Asynchronous queue */
    uint32_t    ui32TaskId;     /* This determines whether this is in use or not */
    BRAP_Fwif_P_FwBufInfo   sAsyncMsgBufmem; /* Memory for consecutive 5 Async Msgs */
}BRAP_Fwif_P_TskQueues;

/* This structure contains actual addresses & sizes per task */
typedef struct BRAP_Fwif_P_TskMemInfo
{
    BRAP_Fwif_P_TskQueues    sTskQueue[BRAP_MAX_FW_TSK_SUPPORTED];
    BRAP_Fwif_P_FwBufInfo    sTskInfo[BRAP_MAX_FW_TSK_SUPPORTED]; /* Task memory info */
    BRAP_Fwif_P_FwBufInfo    sCitStruct[BRAP_MAX_FW_TSK_SUPPORTED]; /* Cit memory info */
    BRAP_Fwif_P_FwBufInfo    sTskIFrameCfgBufInfo[BRAP_MAX_FW_TSK_SUPPORTED];
}BRAP_Fwif_P_TskMemInfo;

/* Memory requirement for the Raptor Device */
typedef struct BRAP_Fwif_P_MemRequirement
{
    BRAP_DSP_DwnldMemInfo   sDwnldMemInfo; /* Download memory requirements */
    uint32_t    ui32OpenTimeDownloadSize; /* Open time download memory requirement */
    uint32_t    ui32TskMemory[BRAP_MAX_DSP_SUPPORTED]; /* Task memory requirements */
    uint32_t    ui32DspScratchMemReq[BRAP_MAX_DSP_SUPPORTED]; /* Scratch & interstage & interface buffers per dsp */
    BRAP_Fwif_P_TskMemRequirement sTskMemReq[BRAP_MAX_DSP_SUPPORTED]; /* Iframe & Cfgbuf memory requirement */
    uint32_t    ui32ConfigBufReq;   /* Extra buffer for on-the-fly programming of config params */
}BRAP_Fwif_P_MemRequirement;

#ifdef RAP_REALVIDEO_SUPPORT
typedef struct BRAP_Fwif_P_RealVideoBufInfo
{
    BRAP_Fwif_P_FwBufInfo   sLumaFrameBuffParams[BRAP_FWMAX_VIDEO_BUFF_AVAIL];
    BRAP_Fwif_P_FwBufInfo   sChromaFrameBuffParams[BRAP_FWMAX_VIDEO_BUFF_AVAIL];  
    BRAP_Fwif_P_FwBufInfo   sLumaReferenceBuffParams[BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL];
    BRAP_Fwif_P_FwBufInfo   sChromaReferenceBuffParams[BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL];            
    BRAP_Fwif_P_FwBufInfo   sUPBs[BRAP_FWMAX_VIDEO_BUFF_AVAIL];
    BRAP_Fwif_P_FwBufInfo   sDRAMMBInfoStartAdr[BRAP_FWMAX_MB_INFO_AVAIL];
}BRAP_Fwif_P_RealVideoBufInfo;
#endif

/* Memory allocated information for whole Raptor device */
typedef struct BRAP_Fwif_MemInfo
{
    BRAP_DSP_DwnldMemInfo   sDwnldMemInfo; /* Download memory requirements */
    BRAP_Fwif_P_FwBufInfo   sOpenTimeMemInfo; /* Open time download memory info */
    BRAP_FWIF_MsgQueueParams    sCmdQueue[BRAP_MAX_DSP_SUPPORTED];  /* Command queue per DSP */
    BRAP_FWIF_MsgQueueParams    sGenRspQueue[BRAP_MAX_DSP_SUPPORTED];  /* Generic (non-task) response queue per DSP */
#ifdef RAP_VIDEOONDSP_SUPPORT    
    BRAP_FWIF_MsgQueueParams    sPDQueue[BRAP_MAX_DSP_SUPPORTED];  /* Picture Delivery queue per DSP */
    BRAP_FWIF_MsgQueueParams    sPRQueue[BRAP_MAX_DSP_SUPPORTED];  /* Picture Release queue per DSP */
    BRAP_FWIF_MsgQueueParams    sDSQueue[BRAP_MAX_DSP_SUPPORTED];  /* Picture Release queue per DSP */    
#endif
    BRAP_Fwif_P_FwBufInfo   sDspScratchInfo[BRAP_MAX_DSP_SUPPORTED]; /* Scratch & interstage & interface buffers per dsp */
    BRAP_Fwif_P_TskMemInfo  sTskMemInfo[BRAP_MAX_DSP_SUPPORTED]; /* Iframe & Cfgbuf memory */
    BRAP_Fwif_P_FwBufInfo   sConfigParamBufInfo;    /* Extra buffer for on-the-fly programming of config params */
    BRAP_Fwif_P_FwBufInfo   sTsmConfigParamBufInfo[BRAP_MAX_FW_TSK_SUPPORTED];    /* Extra buffer for on-the-fly programming of TSM config params for each task*/ 
                                                                                                                                        /*This memory is allocated per task for TSM configuration, because TSM configuration can be
                                                                                                                                        set asynchronously for different tasks, So using a single buffer was overwriting the memory*/
    BRAP_Fwif_P_FwBufInfo   sSpdifStatusBitBufInfo;    /* Extra buffer for on-the-fly programming of config params */    
#ifdef RAP_VIDEOONDSP_SUPPORT 
    BRAP_Fwif_P_RealVideoBufInfo   sRealVideoBufferInfo;    /* Real Video */                                  
#endif
#ifdef RAP_GFX_SUPPORT                                                                                                                                        
    BRAP_Fwif_P_GfxBufPool   sGfxfInfo;    /* Extra buffer for on-the-fly programming of config params */    
#endif                                                                                                                                        
}BRAP_Fwif_MemInfo;

