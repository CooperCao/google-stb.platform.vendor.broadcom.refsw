/***************************************************************************
*     Copyright (c) 2003-2009, Broadcom Corporation
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
*
* Revision History:
*
* $brcm_Log: $
* 
 Hydra_Software_Devel/1   4/9/03 5:32p vobadm
* merged from aramPrivateBranch
* 
* aramPrivateBranch/3   4/9/03 8:56a aram
* more meat was added
* 
* aramPrivateBranch/2   4/1/03 9:27a aram
* more files was added
* 
* aramPrivateBranch/1   3/24/03 12:29p aram
* genesis of files
***************************************************************************/
 
#ifndef AUDIO_DSP_HOST_INTERFACE_H__
#define AUDIO_DSP_HOST_INTERFACE_H__

#include "brap_dsphostcommon.h"

#if((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325) || (BCHP_CHIP == 7335))
#define BRAP_7405_FAMILY    1
#endif

#if((BCHP_CHIP == 7440) || (BCHP_CHIP == 7601))
#define BRAP_DVD_FAMILY         1
#endif

#if (BCHP_CHIP == 3563)
#define BRAP_DSP_P_3563_NEWMIT  1
#define BRAP_DSP_P_NEWMIT       0
#define BRAP_DSP_P_NEWMIT_RDB   0
#elif ((BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
#define BRAP_DSP_P_3563_NEWMIT  0     
#define BRAP_DSP_P_NEWMIT       1
#define BRAP_DSP_P_NEWMIT_RDB   0
#else
#define BRAP_DSP_P_3563_NEWMIT  0     
#define BRAP_DSP_P_NEWMIT       0
#define BRAP_DSP_P_NEWMIT_RDB   0
#endif






typedef  uint32_t  BAF_HostDramAddress;

#define BAF_HOST_MAX_NAME_SIZE   4
#define BAF_MAX_DRAM_POINTERS   10 /* max number of tables used by an algorithm, decode or post processing */
#if (BCHP_CHIP == 7401)||(BCHP_CHIP == 7403)||(BCHP_CHIP == 7118) || (BRAP_7405_FAMILY == 1) || ( BCHP_CHIP == 7400 ) || (BCHP_CHIP == 3563)
    #if (BCHP_CHIP == 7401)||(BCHP_CHIP == 7403)||(BCHP_CHIP == 7118) || ( BCHP_CHIP == 7400 )
    #define BAF_HOST_MAX_DL_MODULE  40 /* Max number of downloadable modules for frame sync, decode or post processing */
    #else
    #define BAF_HOST_MAX_DL_MODULE  30 /* Max number of downloadable modules for frame sync, decode or post processing */
    #endif
#elif (BRAP_DVD_FAMILY == 1)
    #define BAF_HOST_MAX_DL_MODULE  40 /* Max number of downloadable modules for frame sync, decode or post processing */
#else
    #define BAF_HOST_MAX_DL_MODULE  20 /* Max number of downloadable modules for frame sync, decode or post processing */
#endif
#define BAF_MAX_SCRATCH_BUF     16 /* max number of scratch buffers available to an algorithm ( NUM_STAGE_BFR * 2) */
#define NUM_CHANNELS			3

#if ((BRAP_DSP_P_3563_NEWMIT == 0) && (BRAP_DSP_P_NEWMIT == 0) && (BRAP_DSP_P_7401_NEWMIT == 0) )
typedef struct BAF_HostBufInfo
{
    uint32_t                 id;     /* One of AUDIO_DSP_DECODER_xxx or AUDIO_DSP_PP_xxx */
    uint32_t                 size;   /* size */    
    BAF_HostDramAddress      data;   /* data pointer to DRAM address */
}BAF_HostBufInfo;
#define SIZE_OF_BAF_HOST_BUF_INFO (4 + 4 + 4)

typedef struct BAF_Download
{
    BAF_HostBufInfo          sFirmware;                         /* Firmware */
    BAF_HostBufInfo          sTables;  /* Data tables for given algorithm only one pointer to all the tables */    
    BAF_HostBufInfo          sInterStageBuf;  /* Pointer to the interstage buffer that would be used by decode */    
    BAF_HostBufInfo          sInterStageInterfaceBuf;  /* Pointer to Interstage interface that is sued by decoders to pass on config info */
    BAF_HostBufInfo          sDecodeScratchBuf;  /* Pointer to Decoder scratch buffer */    
    
}BAF_Download;
#define SIZE_OF_BAF_DOWNLOAD (   SIZE_OF_BAF_HOST_BUF_INFO * 5 )

typedef struct BAF_HostInfo
{
    uint32_t          id;                /* BAF_HOST_AL_TABLE_ID */
    uint32_t          version;           /* BAF_HOST_AL_VERSION */
    BAF_HostBufInfo   apmScratchBuf;     /* scratch buffer for APM */
    BAF_HostBufInfo	 InterFrameBuf[ NUM_CHANNELS ]; /* Inter frame buffers ( context dependant ) */
    BAF_HostBufInfo   *pFrameSync   [ BAF_HOST_MAX_DL_MODULE ]; /* frame sync module i/f */
    BAF_Download      *pDecode      [ BAF_HOST_MAX_DL_MODULE ]; /* decode module i/f */
    BAF_Download      *pPostProcess [ BAF_HOST_MAX_DL_MODULE ]; /* post process module i/f */
	BAF_Download      *pPassThru    [ BAF_HOST_MAX_DL_MODULE ]; /* Pass Thru i/f */
}BAF_HostInfo;

#define SIZE_OF_BAF_HOST_INFO ( (4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO * 1 ) + ( SIZE_OF_BAF_HOST_BUF_INFO * NUM_CHANNELS ) + \
(4 * (BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE) ) )

#else
/* for new mit for 7401, 7440 and 3563 */


#if ((BRAP_DSP_P_7401_NEWMIT==1)||(BRAP_7405_FAMILY == 1))
#define BAF_MAX_BRANCHES    1   
#define BAF_MAX_POSTPROCESS_STAGES  3 
#define BAF_MAX_CONTEXTS    3 
#else
#define BAF_MAX_CONTEXTS    3 /*4*/ /* Changed to 3 as ZSP doesn't have enough memory */
#define BAF_MAX_BRANCHES    3
#define BAF_MAX_POSTPROCESS_STAGES  7
#endif

typedef struct BAF_HostBufInfo
{
    uint32_t                 id;     /* One of AUDIO_DSP_DECODER_xxx or AUDIO_DSP_PP_xxx */
    uint32_t                 size;   /* size */    
    BAF_HostDramAddress      data;   /* data pointer to DRAM address */
}BAF_HostBufInfo;
#define SIZE_OF_BAF_HOST_BUF_INFO (4 + 4 + 4)

typedef struct BAF_Download
{
    BAF_HostBufInfo          sFirmware;                         /* Firmware */
    BAF_HostBufInfo          sTables;  /* Data tables for given algorithm only one pointer to all the tables */    
    BAF_HostBufInfo          sDecodeScratchBuf;  /* Pointer to Decoder scratch buffer */    
}BAF_Download;
#define SIZE_OF_BAF_DOWNLOAD (   SIZE_OF_BAF_HOST_BUF_INFO * 3 )

typedef struct BAF_HostInfo
{
    uint32_t          id;                /* BAF_HOST_AL_TABLE_ID */
    uint32_t          version;           /* BAF_HOST_AL_VERSION */
#if (BRAP_DVD_FAMILY == 1)||(BRAP_DSP_P_7401_NEWMIT==1)
    BAF_HostDramAddress DnldSchedular;   /* Downaloadable Scheduler Address */
    uint32_t            DnldSchedularSize; /* Downaloadable Scheduler Size */
#endif
    BAF_HostBufInfo   apmScratchBuf;     /* scratch buffer for APM */
    BAF_HostBufInfo	  InterFrameBuf[BAF_MAX_CONTEXTS][BAF_MAX_BRANCHES]
                                    [BAF_MAX_POSTPROCESS_STAGES + 1]; 
    /* Inter frame buffers ( context, branch & stage dependant ) */
    BAF_HostBufInfo	  InterStageInputBuf[BAF_MAX_CONTEXTS][BAF_MAX_BRANCHES]
                                        [BAF_MAX_POSTPROCESS_STAGES + 1]; 
    /* Inter Stage Input buffers ( context, branch & stage dependant ) */
    BAF_HostBufInfo	  InterStageOutputBuf[BAF_MAX_CONTEXTS][BAF_MAX_BRANCHES]
                                        [BAF_MAX_POSTPROCESS_STAGES + 1]; 
    /* Inter Stage Output buffers ( context, branch & stage dependant ) */
    BAF_HostBufInfo	  InterStageInterfaceInputBuf[BAF_MAX_CONTEXTS][BAF_MAX_BRANCHES]
                                                [BAF_MAX_POSTPROCESS_STAGES + 1]; 
    /* Inter Stage Interface Input buffers ( context, branch & stage dependant ) */
    BAF_HostBufInfo	  InterStageInterfaceOutputBuf[BAF_MAX_CONTEXTS][BAF_MAX_BRANCHES]
                                                [BAF_MAX_POSTPROCESS_STAGES + 1]; 
    /* Inter Stage Interface Output buffers ( context, branch & stage dependant ) */
    BAF_HostBufInfo   *pFrameSync   [ BAF_HOST_MAX_DL_MODULE ]; /* frame sync module i/f */
    BAF_Download      *pDecode      [ BAF_HOST_MAX_DL_MODULE ]; /* decode module i/f */
    BAF_Download      *pPostProcess [ BAF_HOST_MAX_DL_MODULE ]; /* post process module i/f */
	BAF_Download      *pPassThru    [ BAF_HOST_MAX_DL_MODULE ]; /* Pass Thru i/f */
#if (BRAP_DSP_P_NEWMIT == 1) ||  (BRAP_DSP_P_7401_NEWMIT==1)
    BAF_Download      *pEncode      [ BAF_HOST_MAX_DL_MODULE ]; /* encode module i/f */    
#endif
}BAF_HostInfo;

#if (BRAP_DSP_P_NEWMIT == 1)
#if (BRAP_DVD_FAMILY == 1)
#define SIZE_OF_BAF_HOST_INFO ( (4 + 4 ) + ( 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO * 1 ) + \
    ( SIZE_OF_BAF_HOST_BUF_INFO * 5 * (BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) + \
    (4 * (BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE) ) )

#else
#define SIZE_OF_BAF_HOST_INFO ( (4 + 4 ) + ( SIZE_OF_BAF_HOST_BUF_INFO * 1 ) + \
    ( SIZE_OF_BAF_HOST_BUF_INFO * 5 * (BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) + \
    (4 * (BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE) ) )

#endif
#else
#define SIZE_OF_BAF_HOST_INFO ( (4 + 4 ) + ( 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO * 1 ) + \
    ( SIZE_OF_BAF_HOST_BUF_INFO * 5 * (BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) + \
    (4 * (BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE + BAF_HOST_MAX_DL_MODULE) ) )
#endif
#endif

/* SPDIF information per context */
typedef struct
{
	 uint16_t professional_mode_flag;
	 uint16_t software_copyright_asserted;
	 uint16_t category_code;
	 uint16_t clock_accuracy;  
  
}BAF_SPDIFInfo;
#define  SIZE_OF_BAF_SPDIF_INFO		(12)


/* 
   This data struture define DSSM (Dynamic System Shared Memory) 
   for a DSP context. This data structure can be changed dynamically 
   before/during/after decode.
 */

typedef struct
{

#define 	BAF_UPDATE_SPDIF			0x00000001
	uint32_t				  dirtyBits;
	BAF_SPDIFInfo		  	  sSPDIFParams;
 
}BAF_ContextInfo;

#define SIZE_OF_BAF_CONTEXT_INFO ( 4 + SIZE_OF_SPDIF_INFO )

#if (BRAP_SEC_METADATA_SUPPORT == 1)

typedef struct BRAP_SecMetadataInfo
{
    uint32_t ui32UserCoeff[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][2][BRAP_RM_P_MAX_OP_CHANNELS]; 
    uint32_t ui32BfRbusAddress[2]; 
    uint32_t ui32DpRbusAddress[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][2][BRAP_RM_P_MAX_OP_CHANNELS];
    uint32_t ui32NumPrimaryCh;
 
}BRAP_SecMetadataInfo;

#define BRAP_METADATA_INFO_SIZE ( \
    (4 * BRAP_RM_P_MAX_OP_CHANNEL_PAIRS * 2 * BRAP_RM_P_MAX_OP_CHANNELS) + \
    (4 * BRAP_RM_P_MAX_OP_CHANNEL_PAIRS * 2 * BRAP_RM_P_MAX_OP_CHANNELS) + \
    (4 * BRAP_RM_P_MAX_OP_CHANNELS) + \
    (4 * BRAP_RM_P_MAX_OP_CHANNEL_PAIRS * 2 * BRAP_RM_P_MAX_OP_CHANNELS) \
    )
#endif

#endif









