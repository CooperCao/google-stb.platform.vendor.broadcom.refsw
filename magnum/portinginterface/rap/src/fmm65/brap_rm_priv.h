/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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
* Module Description: Refer Module Overview given below
*
* Revision History:
* 
* $brcm_Log: $
* 
***************************************************************************/

/*=************************ Module Overview ********************************
Resource Manager is an internal module of Raptor audio PI.

For various audio operations, Raptor audio PI makes use of following hardware 
/firmware/memory resources:
1. DSP contexts (Firmware resource)
2. DRAM ring buffers (Memory resource)
3. Source/Destination channels (FMM hardware resource)
4. SRCs (FMM hardware resource)
4. Mixers (FMM hardware resource)
5. SPDIF-Formatter (FMM hardware resource)
6. Output ports (FMM hardware resource)
7. Input/capture rate control ports (FMM hardware resource)

Above resources are shared for following audio operations:
1. Live Decode/Pass Thru/SRC/Simulmode in DSP       *TODO: Is simul mode reqd?? 
2. PCM Playback from hard-disk/memory
3. PCM Capture
4. Encode 

Resource Manager maintains above resources and provides them to Audio Manager 
as per the request. It allocates resources, for optimum usage. However, Resource
Manager doesn't know what all resources are getting used for a particular 
instance of audio operation. It is the responsibility of Audio Manager to 
provide a list of all the resources required. It is also Audio Manager's 
responsibility to provide a list of all the resources to be freed once an 
instance of audio operation is closed.

Other modules interact with Resource Manager for following purposes.
1. When a new audio channel (instance of audio operation) is formed (or closed).
   Audio Manager requests Resource Manager to allocate (or free) the required 
   resources for this operation.
2. Adding/Removing of the output ports.
3. For mixing operation.

***************************************************************************/
#ifndef _BRAP_RM_PRIV_H__
#define _BRAP_RM_PRIV_H__

#include "brap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Resource Manager module handle.
***************************************************************************/
typedef struct BRAP_RM_P_Object			*BRAP_RM_Handle;

/***************************************************************************
Summary:
	Used in array of resource indexes. If the array size is more than
	number of resource index entries, remaining entries are initialized
	to this macro value.
***************************************************************************/
#define BRAP_RM_P_INVALID_INDEX		    ((unsigned int)(-1))

/***************************************************************************
Summary:
	Macros defining maximum number of resources available.
***************************************************************************/
#if (BRAP_3548_FAMILY == 1)
#define BRAP_RM_P_MAX_DSPS				1	/* Total Number of DSP */	
#define BRAP_RM_P_MAX_CXT_PER_DSP		6	/* Maximum context per DSP*/ 
#define BRAP_RM_P_DEC_DSP_ID			0	/* DSP on which decode happens */
#define BRAP_RM_P_ENC_DSP_ID			1	/* DSP on which encode/transcode
											   happens */
#define BRAP_RM_P_MAX_FW_TASK_PER_DSP   4   /* Maximum number of FW tasks that											   
                                               can exist together in a DSP */
#define BRAP_RM_P_MAX_FMMS				1	/* Total number of FMMs */

/* Buffer block */
#define BRAP_RM_P_MAX_RBUFS				24	/* Total Ring buffers */
#define BRAP_RM_P_MAX_SRC_CHANNELS		8	/* Total Source Channel Fifos*/
#if (BCHP_VER==BCHP_VER_A0)
#define BRAP_RM_P_MAX_DST_CHANNELS		4	/* Total Dest Channel Fifos */
#else /* for B0 */
#define BRAP_RM_P_MAX_DST_CHANNELS		6	/* Total Dest Channel Fifos */
#endif
#define BRAP_RM_P_MAX_INTERNAL_CAPPORTS 4	/* Total Internal CapPorts */

#define BRAP_RM_P_TOTAL_SRCCH_DSTCH		(BRAP_RM_P_MAX_SRC_CHANNELS + \
						                    BRAP_RM_P_MAX_DST_CHANNELS)
												/* Total Src + Dest Fifos */
#define BRAP_RM_P_MAX_RBUFS_PER_SRCCH	2	/* Maximum rbuf per src channel */
#define BRAP_RM_P_MAX_RBUFS_PER_DSTCH	2	/* Maximum rbuf per dest channel */

/* SRC block */
#define BRAP_RM_P_MAX_SRC_BLCK			2	/* Num of SRC blocks in FMM */
#define BRAP_RM_P_MAX_SRC_PER_SRC_BLCK	12	/* Num of SRCs in each SRC block */
#define BRAP_RM_P_MAX_SRC_IN_CASCADE	3	/* Max SRCs that can be cascaded 
												back to back e.g. 12 -> 196 */

/* DP block */
#define BRAP_RM_P_MAX_DP_BLCK				1	/* Number of DP blocks in FMM */
#define BRAP_RM_P_MAX_MIXER_PER_DP_BLCK	8	/* Number of mixers in each DP 
												block */
#define BRAP_RM_P_MAX_MIXER_INPUTS		8	/* Number of inputs to a mixer */
#define BRAP_RM_P_MAX_MIXER_OUTPUTS		2	/* Maximum number of outputs to from
												a mixer */
/* IOP block */
#define BRAP_RM_P_MAX_FS_TMG_SOURCES    4   /* Number of FS Timing Sources ex:
                                               FS0, FS1, FS2 and FS3 */
#define BRAP_RM_P_MAX_SPDIFFMS			1	/* Max SPDIF formaters */
#define BRAP_RM_P_MAX_SPDIFFM_STREAMS	2	/* Max streams across all SPDIFFM */ 
#define BRAP_RM_P_MAX_OUTPUTS			BRAP_OutputPort_eMax 
											/* Max number of output ports */
#define BRAP_RM_P_MAX_RBUFS_PER_PORT	BRAP_RM_P_MAX_RBUFS_PER_SRCCH
											/* Maximum ring buffers per output 
											   port. HDMI in an exception. */
#define BRAP_RM_P_MAX_OP_CHANNELS		8	/* Max o/p audio channels */
#define BRAP_RM_P_MAX_OP_CHANNEL_PAIRS	(BRAP_RM_P_MAX_OP_CHANNELS >> 1)	
										   	/* Max o/p audio channels pairs */
/* Others related to FMM */
#define BRAP_RM_P_MAX_PARALLEL_PATHS 	6	/* Max parallel paths supported in
												the system */
#define BRAP_RM_P_MAX_MIXING_LEVELS		7	/* Max SRC-Mixing levels in the 
												system */

#if BRAP_P_CUSTOM_MIXER_REQUIREMENT												
#define BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC 6
												/* Max resources that can be fed
												by a single SRC */
#else
#define BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC 4
												/* Max resources that can be fed
												by a single SRC */
#endif

#define BRAP_P_MAX_PATHS_IN_A_CHAN		8

/* Max channel types */
#define BRAP_RM_P_MAX_DEC_CHANNELS		3	/* Max decode channels */ 
#define BRAP_RM_P_MAX_PCM_CHANNELS		4	/* Max PCM playback channels */ 
#define BRAP_RM_P_MAX_CAP_CHANNELS		4	/* Max capture channels */ 

/* Encoder support in RM */ 
/* Make BRAP_RM_P_MAX_ENC_CHANNELS zero later */
#define BRAP_RM_P_MAX_ENC_CHANNELS			1	/* Max encode channels */
#define BRAP_RM_P_MAX_RBUFS_PER_ENC_INPUT	6	/* Maximum ring buffers per 
													encoder input channel */

/* Macros for maximum number of supported decode and passthru operations.
 * Decode part of simul mode is counted in BRAP_P_MAX_DECODE_OPERATIONS
 * and pass thru part of simul mode is count in BRAP_P_MAX_PASSTHRU_OPERATIONS
 */
#define BRAP_RM_P_MAX_DECODE_SUPPORTED		1	
#define BRAP_RM_P_MAX_PASSTHRU_SUPPORTED	1	
#define BRAP_RM_P_MAX_ENCODE_SUPPORTED		0

/* Internal Destination (Internal Capture port)*/
#define BRAP_RM_P_MAX_INTERNAL_DST			4

/* Simul mode linkage */
#define BRAP_RM_P_MAX_LINKAGE_SUPPORTED     1

#define BRAP_RM_P_MAX_MIXER_PB_FCI_ID       32

/* AdaptRateCtrl block */
#define BRAP_RM_P_MAX_ADAPTRATE_CTRL			4


#elif (BRAP_7405_FAMILY == 1)
#define BRAP_RM_P_MAX_DSPS				1	/* Total Number of DSP */	
#define BRAP_RM_P_MAX_CXT_PER_DSP		6	/* Maximum context per DSP*/ 
                                            						/* TODO: check with firmware team */
#define BRAP_RM_P_DEC_DSP_ID				0	/* DSP on which decode happens */
#define BRAP_RM_P_ENC_DSP_ID				0	/* DSP on which encode/transcode
												happens */
#define BRAP_RM_P_MAX_FW_TASK_PER_DSP   3       /* Maximum number of FW tasks that
                                                                                          can exist together in a DSP */
#define BRAP_RM_P_MAX_FMMS				1	/* Total number of FMMs */

/* Buffer block */
#if (BRAP_7420_FAMILY == 1)
#define BRAP_RM_P_MAX_RBUFS				28	/* Total Ring buffers */
#define BRAP_RM_P_MAX_SRC_CHANNELS		10	/* Total Source Channel Fifos*/
#define BRAP_RM_P_MAX_INTERNAL_CAPPORTS 5	/* Total Internal CapPorts */
/* AdaptRateCtrl block */
#define BRAP_RM_P_MAX_ADAPTRATE_CTRL	8
#define BRAP_RM_P_MAX_IOP_FCI_CFG    18   /* Number of IOP FCI CFG*/	
#elif (BRAP_7550_FAMILY == 1)
#define BRAP_RM_P_MAX_RBUFS				18	/* Total Ring buffers */
#define BRAP_RM_P_MAX_SRC_CHANNELS		9	/* Total Source Channel Fifos*/
#define BRAP_RM_P_MAX_ADAPTRATE_CTRL    8   /* AdaptRateCtrl block */
#define BRAP_RM_P_MAX_IOP_FCI_CFG    11   /* Number of IOP FCI CFG*/	
#else
#define BRAP_RM_P_MAX_RBUFS				24	/* Total Ring buffers */
#define BRAP_RM_P_MAX_SRC_CHANNELS		8	/* Total Source Channel Fifos*/
#define BRAP_RM_P_MAX_INTERNAL_CAPPORTS 4	/* Total Internal CapPorts */
/* AdaptRateCtrl block */
#define BRAP_RM_P_MAX_ADAPTRATE_CTRL	4
#define BRAP_RM_P_MAX_IOP_FCI_CFG    17   /* Number of IOP FCI CFG*/	
#endif
#define BRAP_RM_P_MAX_DST_CHANNELS		4	/* Total Dest Channel Fifos */

#define BRAP_RM_P_TOTAL_SRCCH_DSTCH		(BRAP_RM_P_MAX_SRC_CHANNELS + \
						BRAP_RM_P_MAX_DST_CHANNELS)
												/* Total Src + Dest Fifos */
#define BRAP_RM_P_MAX_RBUFS_PER_SRCCH	2	/* Maximum rbuf per src channel */
#define BRAP_RM_P_MAX_RBUFS_PER_DSTCH	2	/* Maximum rbuf per dest channel */

/* SRC block */
#define BRAP_RM_P_MAX_SRC_BLCK			1	/* Num of SRC blocks in FMM */
#define BRAP_RM_P_MAX_SRC_PER_SRC_BLCK	12	/* Num of SRCs in each SRC block */
#define BRAP_RM_P_MAX_SRC_IN_CASCADE	2	/* Max SRCs that can be cascaded 
												back to back e.g. 12 -> 196 */

/* DP block */
#define BRAP_RM_P_MAX_DP_BLCK				1	/* Number of DP blocks in FMM */
#define BRAP_RM_P_MAX_MIXER_PER_DP_BLCK	8	/* Number of mixers in each DP 
												block */
#define BRAP_RM_P_MAX_MIXER_INPUTS		8	/* Number of inputs to a mixer */
#define BRAP_RM_P_MAX_MIXER_OUTPUTS		2	/* Maximum number of outputs to from
												a mixer */
#define BRAP_RM_P_MAX_MIXER_PB_FCI_ID   32
/* IOP block */
#define BRAP_RM_P_MAX_FS_TMG_SOURCES    4   /* Number of FS Timing Sources ex:
                                               FS0, FS1, FS2 and FS3 */
#define BRAP_RM_P_MAX_SPDIFFMS			1	/* Max SPDIF formaters */
#define BRAP_RM_P_MAX_SPDIFFM_STREAMS	2	/* Max streams across all SPDIFFM */ 
#define BRAP_RM_P_MAX_OUTPUTS			BRAP_OutputPort_eMax 
											/* Max number of output ports */
#define BRAP_RM_P_MAX_RBUFS_PER_PORT	BRAP_RM_P_MAX_RBUFS_PER_SRCCH
											/* Maximum ring buffers per output 
											   port. HDMI in an exception. */
#define BRAP_RM_P_MAX_OP_CHANNELS		8	/* Max o/p audio channels */
#define BRAP_RM_P_MAX_OP_CHANNEL_PAIRS	(BRAP_RM_P_MAX_OP_CHANNELS >> 1)	
										   	/* Max o/p audio channels pairs */

/* Others related to FMM */
#define BRAP_RM_P_MAX_PARALLEL_PATHS 	4	/* Max parallel paths supported in
												the system */
#define BRAP_RM_P_MAX_MIXING_LEVELS		4	/* Max SRC-Mixing levels in the 
												system */
#if (BRAP_7550_FAMILY == 1)
#define BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC 1
												/* Max resources that can be fed
												by a single SRCCH, not SRC.
												Same Macro is used to minimize
												the code changes*/
#else
#define BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC 4
												/* Max resources that can be fed
												by a single SRC */
#endif

#define BRAP_P_MAX_PATHS_IN_A_CHAN		BRAP_P_MAX_DST_PER_RAPCH

/* Max channel types */
#define BRAP_RM_P_MAX_DEC_CHANNELS		3	/* Max decode channels */ 
#define BRAP_RM_P_MAX_PCM_CHANNELS		4	/* Max PCM playback channels */ 
#define BRAP_RM_P_MAX_CAP_CHANNELS		4	/* Max capture channels */ 
#ifdef RAP_REALVIDEO_SUPPORT
#define BRAP_RM_P_MAX_VIDEO_DEC_CHANNELS	1	/* Max decode channels */ 
#endif
#ifdef RAP_GFX_SUPPORT
#define BRAP_RM_P_MAX_GFX_CHANNELS		2	/* Max capture channels */ 
#endif
#ifdef RAP_SCM_SUPPORT 
#define BRAP_RM_P_MAX_SCM_CHANNELS		1	/* Max SCM channels */ 
#endif


/* Encoder support in RM */ 
/* Make BRAP_RM_P_MAX_ENC_CHANNELS zero later */
#define BRAP_RM_P_MAX_ENC_CHANNELS			1	/* Max encode channels */
#define BRAP_RM_P_MAX_RBUFS_PER_ENC_INPUT	6	/* Maximum ring buffers per 
													encoder input channel */

/* Macros for maximum number of supported decode and passthru operations.
 * Decode part of simul mode is counted in BRAP_P_MAX_DECODE_OPERATIONS
 * and pass thru part of simul mode is count in BRAP_P_MAX_PASSTHRU_OPERATIONS
 */
#define BRAP_RM_P_MAX_DECODE_SUPPORTED		1	
#define BRAP_RM_P_MAX_PASSTHRU_SUPPORTED	1	
#define BRAP_RM_P_MAX_ENCODE_SUPPORTED		0

/* Internal Destination (Internal Capture port)*/
#define BRAP_RM_P_MAX_INTERNAL_DST			4

#else
#error "Not support chip type"
#endif

/***************************************************************************
Summary:
    This enumeration lists the various usage paths that can exist in a 
    channel.

    Note: This declaration has been moved here from brap_priv.h as this is
    required to be used in RM data structure and brap_priv.h gets included 
    after rm_priv.h.
***************************************************************************/ 

typedef enum BRAP_P_UsgPath
{   
    BRAP_P_UsgPath_eDecodePcm,      /* Starts at DSP0 and ends at OP/CAP */
    BRAP_P_UsgPath_eDecodePcmPostMixing,
    BRAP_P_UsgPath_eDecodeCompressPostMixing,
    BRAP_P_UsgPath_eDecodeCompress, /* Starts at DSP0 and ends at OP */
    BRAP_P_UsgPath_eCapture,        /* Starts at CAP and ends at RBuf/OP */
    BRAP_P_UsgPath_ePPBranchPostMixing,                                       
    BRAP_P_UsgPath_ePPBranch,       /* Starts after Decode & ends at OP.*/                                       
    BRAP_P_UsgPath_eMixPath,    /* This Path is created if Similar branch in network goes 
                                                            to 2 different association */                                                                
    BRAP_P_UsgPath_eDownmixedPath,/*This Path is created if Destination to the branch is
                                                                    taking Stereo Downmixed data*/
    BRAP_P_UsgPath_eDownmixedMixPath,/*This Path is created if if Similar branch in 
                                                                    network goes to 2 different association and 
                                                                    Destination to the branch is taking Stereo 
                                                                    Downmixed data*/
    BRAP_P_UsgPath_eSharedPP,    /* This Path is created if for the O/P ports having indep delay =0 for PCM playback/Capture. In this path is shared till SRC*/                                                                    
#ifdef RAP_REALVIDEO_SUPPORT
    BRAP_P_UsgPath_eVideoDecode,    /* Video Decode*/          
#endif    
#ifdef RAP_GFX_SUPPORT
    BRAP_P_UsgPath_eGfx,    /* Gfx Path*/                                                                        
#endif    
#ifdef RAP_SCM_SUPPORT
	BRAP_P_UsgPath_eScm,    /* SCM Path*/                                                                        
#endif	
    BRAP_P_UsgPath_eMax             /* Invalid or max usage path */                           
}BRAP_P_UsgPath;

/***************************************************************************
Summary:
	Enumerations for states of an object
***************************************************************************/
typedef enum BRAP_RM_P_ObjectState
{
	BRAP_RM_P_ObjectState_eFree,	/* Free state */
	BRAP_RM_P_ObjectState_eBusy	    /* Busy state */
} BRAP_RM_P_ObjectState;

/***************************************************************************
Summary:
	SRC resource grant structure. This is a part of the main resource grant 
	structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcGrant
{
	unsigned int	uiSrcBlkId;     /* Alloted SRC block index */
    unsigned int    uiSrcId;        /* Alloted SRC index */
}BRAP_RM_P_SrcGrant;

/***************************************************************************
Summary:
	Mixer resource grant structure. This is a part of the main resource 
	grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_MixerGrant
{
    unsigned int	uiDpId;         /* Alloted DP index */
    unsigned int    uiMixerId;      /* Alloted mixer index */
    unsigned int    uiMixerInputId[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                    /* Alloted Mixer input indices */
    unsigned int    uiMixerOutputId[BRAP_RM_P_MAX_MIXER_OUTPUTS];
                                    /* Alloted mixer output indices */
}BRAP_RM_P_MixerGrant;

/***************************************************************************
Summary:
	Combined resource grant structure for SRC and mixer. This is a sub-
	structure in the main resource grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcMixerGrant
{
    BRAP_RM_P_SrcGrant      sSrcGrant[BRAP_RM_P_MAX_SRC_IN_CASCADE]
                                     [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                     [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Structure containing allotted SRC Ids */ 
    BRAP_RM_P_MixerGrant    sMixerGrant[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                       [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Structure containing allotted Mixer Ids*/
}BRAP_RM_P_SrcMixerGrant;

/***************************************************************************
Summary:
	Ouput port and corresponding SPDIFFM resource grant structure. This is a 
	sub-structure in the main resource grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcEqGrant
{
	unsigned int	uiSrcBlkId;     /* Alloted SRC block index */
    unsigned int    uiSrcId;        /* Alloted SRC index */
}BRAP_RM_P_SrcEqGrant;

/***************************************************************************
Summary:
	Ouput port and corresponding SPDIFFM resource grant structure. This is a 
	sub-structure in the main resource grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_OpPortGrant
{
    unsigned int        uiSpdiffmId;        /* Alloted SPDIF Formater index */
    unsigned int        uiSpdiffmStreamId;  /* Alloted SPDIF Formater stream 
                                              index */
    BRAP_OutputPort     eOutputPort;        /* Alloted output port */
}BRAP_RM_P_OpPortGrant;

/***************************************************************************
Summary:
	FS Timing Source grant structure. This is one of the sub-structures in 
	the  main resource grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_FsTmgSrcGrant
{
    unsigned int        uiFsTmgSrc;     /* Alloted FS Timing Source.  */
}BRAP_RM_P_FsTmgSrcGrant;

/***************************************************************************
Summary:
	Capture port resource grant structure. This is a sub-structure in the 
	main resource grant structure.
***************************************************************************/
typedef struct BRAP_RM_P_CapPortGrant
{
    BRAP_CapInputPort           eCapPort;       /* Alloted Capture port */ 
}BRAP_RM_P_CapPortGrant;

/***************************************************************************
Summary:
	Resource grant structure. 

	Note: 
	1. Every time there is a resource request from the Audio Manager, 
	corresponding resources will be allocated and returned to the AM. It is AM's
	duty to add the newly allocated resources into it's already allocated 
	resource list.

	2. All unused entries will be initialized to BRAP_RM_P_INVALID_INDEX. 	
***************************************************************************/
typedef struct BRAP_RM_P_ResrcGrant
{
	unsigned int	        uiDspId;		/* Allotted DSP index */
	unsigned int	        uiDspContextId;	/* Allotted DSP context index */
	unsigned int	        uiFmmId;		/* Allotted FMM index */
    unsigned int	        uiRbufId[BRAP_RM_P_MAX_OP_CHANNELS]; /* Allotted RBuf Ids */
    unsigned int	        uiSrcChId[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];/* Allotted Source Fifo Ids */                                            
    unsigned int	        uiDstChId[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];/* Allotted Destination Fifo Ids */
    unsigned int	        uiAdaptRateCtrlId[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];/* Allotted Rate manger block Ids */    
    BRAP_RM_P_SrcMixerGrant sSrcMixerGrnt[BRAP_RM_P_MAX_MIXING_LEVELS];
                                            /* Structure containing allotted 
                                               SRC and Mixer Ids */    
    BRAP_RM_P_SrcEqGrant    sSrcEqGrant[BRAP_RM_P_MAX_SRC_IN_CASCADE]
                                       [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                       [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                            /* Structure containing allotted 
                                               Equalizer SRCs */
    BRAP_RM_P_OpPortGrant   sOpPortGrnt[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                            /* Stucture containing allotted 
                                               Output Ports and SPDIFFM Ids */
    BRAP_RM_P_CapPortGrant  sCapPortGrnt[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                        [BRAP_P_MAX_OUT_PARALLEL_FMM_PATH];
                                            /* Stucture containing allotted 
                                               Capture Ports */
    BRAP_RM_P_FsTmgSrcGrant sFsTmgGrnt;     /* Alloted FS Timing Source */
} BRAP_RM_P_ResrcGrant;

/***************************************************************************
Summary:
	Ring buffer resource request structure. This is a sub-structure in the
	main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_RbufReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
    BRAP_BufDataMode    eBufDataMode;   /* Buffer data mode */
}BRAP_RM_P_RbufReq;

/***************************************************************************
Summary:
	Source Fifo (or Source Channel) resource request structure. This is a 
	sub-structure in the main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcChReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
}BRAP_RM_P_SrcChReq;

/***************************************************************************
Summary:
	Rate manger block resource request structure. This is a 
	sub-structure in the main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_AdaptRateCtrlReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
}BRAP_RM_P_AdaptRateCtrlReq;

/***************************************************************************
Summary:
	SRC resource request structure. This is a part of the main resource 
	request structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
    BRAP_RM_P_SrcGrant  sReallocateSrc; /* AM can indicate to reallocate an
                                           SRC using this. Else, fill it up 
                                           with BRAP_RM_P_INVALID_INDEX */
}BRAP_RM_P_SrcReq;

/***************************************************************************
Summary:
	SRC Equalizer resource request structure. This is a part of the main resource 
	request structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcEqReq
{
    bool                    bAllocate;          /* TRUE indicates a valid request */
    unsigned int            uiBlkId;            /* Indicates SRC Block from where
                                                   an SRC need to be allocated */
    BRAP_RM_P_SrcEqGrant    sReallocateSrcEq;   /* AM can indicate to reallocate this SRCEq */
}BRAP_RM_P_SrcEqReq;

/***************************************************************************
Summary:
	Mixer resource request structure. This is a part of the main resource 
	request structure.
***************************************************************************/
typedef struct BRAP_RM_P_MixerReq
{
    bool                    bAllocate;          /* TRUE indicates a valid 
                                                   request */
    unsigned int            uiNumNewInput;      /* Number of mixer input */
    unsigned int            uiNumNewOutput;     /* Number of mixer output */
    bool                    bInputChPair[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
    BRAP_RM_P_MixerGrant    sReallocateMixer;   /* AM can indicate to reallocate
                                                   a mixer along with its inputs
                                                   and outputs using this. AM 
                                                   can also request to 
                                                   reallocate a mixer but with 
                                                   fresh inputs and/or outputs.
                                                   If AM is interested in having
                                                   an entirely fresh mixer, it 
                                                   has to fill up this strucure
                                                   with BRAP_RM_P_INVALID_INDEX 
                                                   */
    unsigned int            uiMixerOutputId[BRAP_RM_P_MAX_MIXER_OUTPUTS];
                                                /* Alloted mixer output indices */

}BRAP_RM_P_MixerReq;

/***************************************************************************
Summary:
	Common resource request structure for SRC and Mixer. This is a 
	sub-structure in the main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_SrcMixerReq
{
    BRAP_RM_P_SrcReq    sSrcReq[BRAP_RM_P_MAX_SRC_IN_CASCADE]
                               [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                               [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                        /* SRC resource request */
    BRAP_RM_P_MixerReq  sMixerReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                 [BRAP_RM_P_MAX_PARALLEL_PATHS];   
                                        /* Mixer resource request */
}BRAP_RM_P_SrcMixerReq;

/***************************************************************************
Summary:
	Output port resource request structure. This is a sub-structure in the
	main resource request structure. There is no explicit resource request
	for SPDIFFM. It is decided based on the output port type and MuxSelect 
	provided using this structure.
***************************************************************************/
typedef struct BRAP_RM_P_OpPortReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */    
	BRAP_OutputPort		eOpPortType;	/* Output port type */
	BRAP_OutputPort		eMuxSelect;		/* If output port type is MAI then 
	                                       which input to use. */
}BRAP_RM_P_OpPortReq;

/***************************************************************************
Summary:
	Structure for resource request associated with capture port
***************************************************************************/
typedef struct BRAP_RM_P_CapPortReq
{
    bool                    bAllocate;  /* TRUE indicates a valid request */    
    BRAP_CapInputPort       eCapPort;   /* BRAP_CapInputPort_eMax indicates 
                                           request for fresh internal cap-ports.
                                           Else explicit request for a 
                                           particular cap-port */
} BRAP_RM_P_CapPortReq;

/***************************************************************************
Summary:
	Destination Fifo (or Destination Channel) resource request structure. 
	This is a sub-structure in the main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_DstChReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
}BRAP_RM_P_DstChReq;

/***************************************************************************
Summary:
	FS Timing Source resource request structure. This is used to provide FS 
	Timing Clock to Dummy Sinks and Internal Capture Ports.
	
	This is a sub-structure in the main resource request structure.
***************************************************************************/
typedef struct BRAP_RM_P_FsTmgSrcReq
{
    bool                bAllocate;      /* TRUE indicates a valid request */
    unsigned int        uiFsTmgSrcId;   /* If bAllocate is TRUE
                                           then BRAP_RM_P_INVALID_INDEX 
                                           indicates a fresh FS Timing Src 
                                           request while a valid uiFsTmgSrcId 
                                           indicates a re-allocation request */
}BRAP_RM_P_FsTmgSrcReq;
    
/***************************************************************************
Summary:
	Structure for requesting required resources. 
***************************************************************************/
typedef struct BRAP_RM_P_ResrcReq
{
    BRAP_ChannelType	    eChType;	/* Channel type */
    BRAP_ChannelSubType     eChSubType; /* Sub-channel type */
    BRAP_P_UsgPath          ePath;      /* Path type for which resource request
                                           has been sent */
    bool                    bAllocateDSP;
                                        /* true to allocate DSP, else false */
    BRAP_RM_P_RbufReq       sRbufReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                        /* Rbuf resource request */
    BRAP_RM_P_SrcChReq      sSrcChReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                        /* SrcCh resource request */    
    BRAP_RM_P_AdaptRateCtrlReq   sAdaptRateCtrlReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                        /* Rate manager resource request */                                            
    BRAP_RM_P_SrcMixerReq   sSrcMixerReq[BRAP_RM_P_MAX_MIXING_LEVELS];
                                        /* SRC and Mixer resource request */

    BRAP_RM_P_SrcEqReq      sSrcEqReq[BRAP_RM_P_MAX_SRC_IN_CASCADE]
                                     [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                     [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                            
    BRAP_RM_P_OpPortReq     sOpReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                        /* Output port resource request */
    BRAP_RM_P_CapPortReq    sCapReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                   [BRAP_P_MAX_OUT_PARALLEL_FMM_PATH];
                                        /* Capture Port request */
    BRAP_RM_P_DstChReq      sDstChReq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
                                        /* Destination Channel Request */
    BRAP_RM_P_FsTmgSrcReq   sFsTmgSrcReq;
                                        /* FS Timing Source resource request */
}BRAP_RM_P_ResrcReq;    


/***************************************************************************
Summary:
	Structure for SPDIF Formater usage
***************************************************************************/
typedef struct BRAP_RM_P_SpdiffmUsage
{ 
    unsigned int			uiUsageCount[BRAP_RM_P_MAX_SPDIFFM_STREAMS];	
								/* Total number of Clients using this SPDIFFM
								   channel. Zero indicates this is free */
} BRAP_RM_P_SpdiffmUsage;

/***************************************************************************
Summary:
	Structure for CapPort usage
***************************************************************************/
typedef struct BRAP_RM_P_CapPortUsage
{ 
    unsigned int			uiUsageCount; 
                                /* Total number of Clients using this CapPort.
								   Zero indicates this is free */
}BRAP_RM_P_CapPortUsage;


/***************************************************************************
Summary:
	Structure for mixer usage
***************************************************************************/
typedef struct BRAP_RM_P_MixerUsage
{
	unsigned int	        uiInputUsgCnt[BRAP_RM_P_MAX_MIXER_INPUTS];
								/* Total number of Clients using this mixer 
								   input. Zero indicates this is free */
	unsigned int	        uiOutputUsgCnt[BRAP_RM_P_MAX_MIXER_OUTPUTS];
								/* Total number of Clients using this mixer 
								   output. Zero indicates this is free */
} BRAP_RM_P_MixerUsage;

/***************************************************************************
Summary:
	Structure for SRC usage
***************************************************************************/
typedef struct BRAP_RM_P_SrcUsage
{
	unsigned int			uiUsageCount;	
								/* Total number of Clients using this SRC.
								   Zero indicates this is free */
    bool                    bSrcEq;	
                                /* If the allocated SRC is used as an 
                                   Equalizer SRC(SRC type - IIR) */
}BRAP_RM_P_SrcUsage;

/***************************************************************************
Summary:
	Structure for FS Timing Source usage
***************************************************************************/
typedef struct BRAP_RM_P_FsTmgSrcUsage
{
	unsigned int			uiUsageCount;	
								/* Total number of Clients using this FS Timing
								   Source. Zero indicates this is free */
}BRAP_RM_P_FsTmgSrcUsage;

/***************************************************************************
Summary:
	Structure for DSP object and DSP context usage
***************************************************************************/
typedef struct BRAP_RM_P_DspUsage
{
	BRAP_RM_P_ObjectState	eContext[BRAP_RM_P_MAX_CXT_PER_DSP];
								            /* Array to maintain the states 
								               of the DSP contexts */
	unsigned int			uiDecCxtCount;	/* Number of decoder contexts
											   currently opened in DSP */
	unsigned int			uiSrcCxtCount;	/* Number of SRC contexts 
											   currently opened in DSP */ 
                                            /* TODO: Is this reqd?? */
	unsigned int			uiPtCxtCount;	/* Number of pass-thru contexts
											   currently opened in DSP */ 
} BRAP_RM_P_DspUsage;

/***************************************************************************
Summary:
	Handle structure for Resource Manager.
***************************************************************************/
typedef struct BRAP_RM_P_Object
{
	BRAP_RM_P_DspUsage		sDspUsage[BRAP_RM_P_MAX_DSPS];
								            /* Array of DSP usage strucrure */
	BRAP_RM_P_ObjectState	eRBufState[BRAP_RM_P_MAX_RBUFS];
								            /* Array to maintain the states of 
								               the Ring buffers */
	BRAP_RM_P_ObjectState	eSrcChState[BRAP_RM_P_MAX_SRC_CHANNELS];
								            /* Array of source channel usage 
								               strucrure */
	BRAP_RM_P_ObjectState	eDstChState[BRAP_RM_P_MAX_DST_CHANNELS];
								            /* Array to maintain the states of 
								               the Ring buffers */
    BRAP_RM_P_ObjectState	eAdaptRateCtrlState[BRAP_RM_P_MAX_ADAPTRATE_CTRL];
    							            /* Array of source channel usage 
    							               strucrure */								               
    BRAP_RM_P_SrcUsage      sSrcUsage[BRAP_RM_P_MAX_SRC_BLCK]
                                     [BRAP_RM_P_MAX_SRC_PER_SRC_BLCK];
								            /* Array to maintain the states of 
								               the SRCs */
	BRAP_RM_P_MixerUsage	sMixerUsage[BRAP_RM_P_MAX_DP_BLCK]
	                                   [BRAP_RM_P_MAX_MIXER_PER_DP_BLCK];
								            /* Array of Mixer usage usage 
								               strucrure */
	BRAP_RM_P_SpdiffmUsage	sSpdiffmUsage[BRAP_RM_P_MAX_SPDIFFMS];
								            /* Array of SPDIF formater usage 
								               strucrure */
    BRAP_RM_P_CapPortUsage  sCapPortUsage[BRAP_CapInputPort_eMax];
                                            /* Array of CapPort usage 
                                                strucrure */
    BRAP_RM_P_FsTmgSrcUsage sFsTmgSrcUsage[BRAP_RM_P_MAX_FS_TMG_SOURCES];
                                            /* Array of FS Timing Source usage 
                                               strucrure */
} BRAP_RM_P_Object;

/***************************************************************************
Summary:	
	Structure for SPDIF Formater mappings
***************************************************************************/
typedef struct BRAP_RM_P_SpdiffmMapping
{	
	unsigned int	uiSpdiffmId;	/* Index of the SPDIF Formater used,	
								       BRAP_RM_P_INVALID_INDEX indicates 
								       SPDIF Formater not used */
	unsigned int	uiSpdiffmChId;	/* Index of the SPDIF Formater channel 
	                                   used. This field is valid if 
	                                   uiSpdiffmId != INVALID_INDEX */
} BRAP_RM_P_SpdiffmMapping;


/***************************************************************************
Summary:
	Opens the resource manager.

Description:
	This function initializes Resource Manager data structures. This function 
	should be called before calling any other Resource Manager function.

Returns:
	BERR_SUCCESS if successful else error

See Also:
	BRAP_RM_P_Close
**************************************************************************/
BERR_Code 
BRAP_RM_P_Open( 
	BRAP_Handle		        hRap,		/* [in] RAP Device handle */
	BRAP_RM_Handle	        *phRm		/* [out]Resource Manager handle */
	);

/***************************************************************************
Summary:
	Closes the resource manager.
	
Description:
	This function shuts downs the Resource Manager and frees up the Resource 
	Manager handle. This function should be called while closing the audio 
	device.

Returns:
	BERR_SUCCESS if successful else error

See Also:
	BRAP_RM_P_Open
**************************************************************************/
BERR_Code 
BRAP_RM_P_Close( 
	BRAP_Handle		        hRap,		/* [in] Device Handle */
	BRAP_RM_Handle	        hRm			/* [in] The Resource Manager handle */
	);

/***************************************************************************
Summary:
    Allocates requested resources.
    
Description:
	This function allocates all the resources required for opening a particular
	instance of audio operation.

Returns:
	BERR_SUCCESS if successful else error

See Also:
	BRAP_RM_P_FreeResources
**************************************************************************/
BERR_Code
BRAP_RM_P_AllocateResources (
	BRAP_RM_Handle				hRm,		/* [in] Resource Manager handle */
	const BRAP_RM_P_ResrcReq	*pResrcReq,	/* [in] Required resources */
	BRAP_RM_P_ResrcGrant		*pResrcGrant/* [out] Indices of the allocated 
	                                           resources */
	);

/***************************************************************************
Summary:
    Frees allocated resources

Description:
	This function frees all the resources corresponding to a particular 
	instance of audio operation that is getting closed.

Returns:
	BERR_SUCCESS - if successful else error

See Also:
	BRAP_RM_P_AllocateResources
**************************************************************************/
BERR_Code
BRAP_RM_P_FreeResources (
	BRAP_RM_Handle	            hRm,		/* [in] Resource Manager handle */
	const BRAP_RM_P_ResrcGrant  *pResrcGrant,/* [in] Indices of the resources
											    to be freed */
        bool   		bFreeSrcChAndRbuf /*If this function is called for Playback Channel,
                                                                Decode PCM path And if bOpenTimeWrToRbuf = true 
                                                                then don't free Srcch and Rbuf which allocated in
                                                                ChannelOPen of Pb*/
	);

/***************************************************************************
Summary:
	Checks if an output port is supported. 
	
Description:
 	This function checks if an output port is supported or not.
 
Returns:
	BERR_SUCCESS - If successful
 	BERR_NOT_SUPPORTED - otherwise
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_RM_P_IsOutputPortSupported (
	BRAP_OutputPort eOutputPort
    );

/***************************************************************************
Summary:
    Private function that returns the preferred Spdif formater Id and stream
    Id for a particular output port.  
**************************************************************************/
BERR_Code
BRAP_RM_P_GetSpdifFmForOpPort(
    BRAP_RM_Handle      hRm,            /* [in] Resource Manager handle */
    BRAP_OutputPort     eOutputPort,    /* [in] Output port */
    BRAP_OutputPort     eMuxOpPort,     /* [in] Mux For MAI port */
    unsigned int	    *pSpdifFmId,	/* [out]Index of the SPDIF Formater
                                           used, BRAP_RM_P_INVALID_INDEX
                                           indicates SPDIF Formater not used */	
    unsigned int	    *pSpdifFmStreamId 
                                        /* [out]Index of the SPDIF Formater
                                           channel used. This field is valid
                                           if uiSpdiffmId != INVALID_INDEX */
    );

/***************************************************************************
Summary:
    Private function that returns Spdif formater Id corresponding to the 
    stream Id.  
**************************************************************************/
BERR_Code 
BRAP_RM_P_GetSpdifFmId (
	unsigned int uiStreamIndex,         /*[in] SPDIFFM Stream index */
	unsigned int * pSpdifFmIndex        /*[out] SPDIFFM corresponding to this 
	                                      SPDIFFM stream */
    );

/***************************************************************************
Summary:
    Private function that initializes the resource Request to invalid values.
**************************************************************************/
void 
BRAP_RM_P_InitResourceReq(
	BRAP_RM_P_ResrcReq          *pResrcReq/* [in] Resource Request structure 
                                                  to be inited to invalid
                                                  values */
    );

/***************************************************************************
Summary:
    Private function that initializes the resource grant to invalid values.
**************************************************************************/
void 
BRAP_RM_P_InitResourceGrant(
	BRAP_RM_P_ResrcGrant		*pResrcGrant,/* [in] Resource grant structure 
	                                                to be inited to invalid 
	                                                values */
        bool   		bFreeSrcChAndRbuf /*If this function is called for Playback Channel,
                                                                Decode PCM path And if bOpenTimeWrToRbuf = true 
                                                                then don't free Srcch and Rbuf which allocated in
                                                                ChannelOPen of Pb*/	                                                
   );

/***************************************************************************
Summary:
    Private function that updates the old resource grant structure with the 
    new one.  
**************************************************************************/
BERR_Code
BRAP_RM_P_UpdateResrcGrant (
    BRAP_RM_P_ResrcGrant        *pOldResrcGrant,
    BRAP_RM_P_ResrcGrant        *pNewResrcGrant
);

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_RM_PRIV_H__ */
/* End of File */

