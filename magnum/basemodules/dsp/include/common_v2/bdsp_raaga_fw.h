/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BDSP_RAAGA_FW_H
#define BDSP_RAAGA_FW_H

#define BDSP_AF_P_MAX_CHANNELS           8
#define BDSP_AF_P_MAX_CHANNEL_PAIR       ((BDSP_AF_P_MAX_CHANNELS+1)>>1)
#define BDSP_AF_P_MAX_IP_FORKS           4
#define BDSP_AF_P_MAX_OP_FORKS           6
#define BDSP_AF_P_MAX_PORT_BUFFERS       4
#define BDSP_AF_P_MAX_STAGES             10
#define BDSP_AF_P_TOC_INVALID            ((uint32_t) -1)
#define BDSP_AF_P_BRANCH_INVALID         ((uint32_t) -1)
#define BDSP_AF_P_WRCNT_INVALID          ((uint32_t) -1)
#define BDSP_AF_P_MAX_LIB_NAME_SIZE           48

#define BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK 6 /*Maximum number of FMM Io buffer ports in a task*/

#define BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS      8

#define BDSP_FWMAX_VIDEO_BUFF_AVAIL               (uint32_t)(16)
#define BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL           (uint32_t)(3)
#define BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP     2

/* Interstage Size calculation macros. TODO:Need to make them better in Phase2*/
#define INTERSTAGE_TOC_SIZE             400
#define INTERSTAGE_TOC_PARTITIONS       20
#define INTERSTAGE_METADATA_SIZE        (4096*16)
#define INTERSTAGE_METADATA_PARTITIONS  1
#define INTERSTAGE_OBJECT_SIZE          100
#define INTERSTAGE_OBJECT_PARTITIONS    4
#define INTERSTAGE_PCM_PARTITIONS       3

#define INTERSTAGE_EXTRA_SIZE   ((INTERSTAGE_TOC_SIZE * INTERSTAGE_TOC_PARTITIONS) + (INTERSTAGE_METADATA_SIZE * INTERSTAGE_METADATA_PARTITIONS) + (INTERSTAGE_OBJECT_SIZE * INTERSTAGE_OBJECT_PARTITIONS))
#define INTERSTAGE_EXTRA_SIZE_ALIGNED ((((INTERSTAGE_EXTRA_SIZE) + 31) >> 5) << 5)

#define BDSP_AF_P_MAX_TOC                            1
#define BDSP_AF_P_MAX_METADATA                       1
#define BDSP_AF_P_MAX_OBJECTDATA                     2
#define BDSP_AF_P_TOC_BUFFER_SIZE                 8192
#define BDSP_AF_P_METADATA_BUFFER_SIZE          (16*4096)
#define BDSP_AF_P_OBJECTDATA_BUFFER_SIZE          4096
#define BDSP_AF_P_INTERTASK_BUFFER_PER_CHANNEL  (8192*6) /* (2048 samples * 4 bytes/sample * (8-48) multiplying factor) */
#define BDSP_RAAGA_MAX_NUM_HEAPS 8

typedef enum BDSP_AF_P_Boolean
{
    BDSP_AF_P_Boolean_eFalse  = 0x0,
    BDSP_AF_P_Boolean_eTrue   = 0x1,
    BDSP_AF_P_Boolean_eMax,
    BDSP_AF_P_Boolean_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_Boolean;

typedef enum BDSP_AF_P_ValidInvalid
{
    BDSP_AF_P_eInvalid  = 0x0,
    BDSP_AF_P_eValid    = 0x1,
    BDSP_AF_P_ValidInvalid_eMax,
    BDSP_AF_P_ValidInvalid_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_ValidInvalid;

typedef enum BDSP_AF_P_EnableDisable
{
    BDSP_AF_P_eDisable = 0x0,
    BDSP_AF_P_eEnable  = 0x1,
    BDSP_AF_P_EnableDisable_eMax,
    BDSP_AF_P_EnableDisable_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_EnableDisable;

typedef enum BDSP_AF_P_DistinctOpType
{
    BDSP_AF_P_DistinctOpType_e7_1_PCM,                  /* 7.1 PCM */
    BDSP_AF_P_DistinctOpType_e5_1_PCM,                  /* 5.1 PCM */
    BDSP_AF_P_DistinctOpType_eStereo_PCM,               /* Downmixed to stereo PCM */
    BDSP_AF_P_DistinctOpType_eMono_PCM,                 /* Mono output type to be used with speech codecs (G.7XX) and AEC modules */
    BDSP_AF_P_DistinctOpType_eCompressed,               /* Compressed Data */
    BDSP_AF_P_DistinctOpType_eCompressed4x,             /* 4x Compressed Rate - for DD+/DTS-HRA Passthru */
    BDSP_AF_P_DistinctOpType_eCompressedHBR,            /* HBR Passthru - True-HD/DTS-MA Passthru  */
    BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut,  /* Re-encode auxiliary data out */
    BDSP_AF_P_DistinctOpType_eCdb,                      /* Compressed Elementary stream to be filled in CDB */
    BDSP_AF_P_DistinctOpType_eItb,                      /* ITB data going out of DSP */
    BDSP_AF_P_DistinctOpType_eAncillaryData,            /* MPEG Ancillary Data  */
    BDSP_AF_P_DistinctOpType_eDescriptorQueue,          /* Descriptor Queue for the Pooled buffers*/
    BDSP_AF_P_DistinctOpType_eGenericIsData,            /* Any kind of data on Interstage buffer*/
    BDSP_AF_P_DistinctOpType_eMax,
    BDSP_AF_P_DistinctOpType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_DistinctOpType;

typedef enum BDSP_AF_P_SampFreq
{
    BDSP_AF_P_SampFreq_e8Khz = 0,
    BDSP_AF_P_SampFreq_e11_025Khz,
    BDSP_AF_P_SampFreq_e12Khz,
    BDSP_AF_P_SampFreq_e16Khz,
    BDSP_AF_P_SampFreq_e22_05Khz,
    BDSP_AF_P_SampFreq_e24Khz,
    BDSP_AF_P_SampFreq_e32Khz,
    BDSP_AF_P_SampFreq_e44_1Khz,
    BDSP_AF_P_SampFreq_e48Khz,
    BDSP_AF_P_SampFreq_e64Khz,
    BDSP_AF_P_SampFreq_e88_2Khz,
    BDSP_AF_P_SampFreq_e96Khz,
    BDSP_AF_P_SampFreq_e128Khz,
    BDSP_AF_P_SampFreq_e176_4Khz,
    BDSP_AF_P_SampFreq_e192Khz,
    BDSP_AF_P_SampFreq_eMax,
    BDSP_AF_P_SampFreq_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_SampFreq;

/* Time base of the System */
typedef enum BDSP_AF_P_TimeBaseType
{
    BDSP_AF_P_TimeBaseType_e45Khz = 0,
    BDSP_AF_P_TimeBaseType_e27Mhz,
    BDSP_AF_P_TimeBaseType_eLast,
    BDSP_AF_P_TimeBaseType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_TimeBaseType;

typedef enum BDSP_PtsType
{
    BDSP_PtsType_eCoded,
    BDSP_PtsType_eInterpolatedFromValidPTS,
    BDSP_PtsType_eInterpolatedFromInvalidPTS = 3,
    BDSP_PtsType_eLast,
    BDSP_PtsType_eInvalid = 0x7FFFFFFF
}BDSP_PtsType;

typedef enum BDSP_AF_P_FmmDstFsRate
{
    BDSP_AF_P_FmmDstFsRate_eBaseRate            = 0x0,
    BDSP_AF_P_FmmDstFsRate_eStreamSamplingRate  = 0x1,
    BDSP_AF_P_FmmDstFsRate_e2xBaseRate          = 0x2,
    BDSP_AF_P_FmmDstFsRate_e4xBaseRate          = 0x3,
    BDSP_AF_P_FmmDstFsRate_e16xBaseRate         = 0x4,
    BDSP_AF_P_FmmDstFsRate_eHBRRate             = BDSP_AF_P_FmmDstFsRate_e16xBaseRate,
    BDSP_AF_P_FmmDstFsRate_eMax,
    BDSP_AF_P_FmmDstFsRate_eInvalid             = 0x7FFFFFFF
}BDSP_AF_P_FmmDstFsRate;

typedef enum BDSP_AF_P_FmmContentType
{
    BDSP_AF_P_FmmContentType_ePcm,              /* PCM */
    BDSP_AF_P_FmmContentType_eCompressed,       /* Compressed Data */
    BDSP_AF_P_FmmContentType_eAnalogCompressed, /* Analog Compressed Data for BTSC Encoder*/
    BDSP_AF_P_FmmContentType_eMax,
    BDSP_AF_P_FmmContentType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_FmmContentType;

typedef enum BDSP_AF_P_PortType
{
    BDSP_AF_P_PortType_eFMM,           /* FMM Port */
    BDSP_AF_P_PortType_eRAVE,          /* Rave Port */
    BDSP_AF_P_PortType_eRDB,           /* RDB Port */
    BDSP_AF_P_PortType_eInterTask,     /* InterTask Port */
    BDSP_AF_P_PortType_eInterStage,    /* InterStage Port */
    BDSP_AF_P_PortType_eAndroidAudio,  /* Android Audio port */
    BDSP_AF_P_PortType_eLast,
    BDSP_AF_P_PortType_eInvalid = 0x7fffffff
}BDSP_AF_P_PortType;

typedef enum BDSP_AF_P_PortBuffer_Type{
    BDSP_AF_P_PortBuffer_Type_Data,
    BDSP_AF_P_PortBuffer_Type_TOC,
    BDSP_AF_P_PortBuffer_Type_MetaData,
    BDSP_AF_P_PortBuffer_Type_ObjectData,
    BDSP_AF_P_PortBuffer_Type_Max,
    BDSP_AF_P_PortBuffer_Type_Invalid = 0x7fffffff
}BDSP_AF_P_PortBuffer_Type;

typedef enum BDSP_AF_P_BufferType
{
    BDSP_AF_P_BufferType_eFMM,       /* Fmm buffer */
    BDSP_AF_P_BufferType_eFMMSlave,  /* FMM Slave buffer */
    BDSP_AF_P_BufferType_eRAVE,      /* Rave Buffer */
    BDSP_AF_P_BufferType_eRDB,       /* RDB buffer typically used in inter task / RDB port*/
    BDSP_AF_P_BufferType_eDRAM,      /* DRAM Buffer */
    BDSP_AF_P_BufferType_eBufferPool,/* Pool of buffers in android audio */
    BDSP_AF_P_BufferType_eLinear,    /* Linear buffer typically used in interstage */
    BDSP_AF_P_BufferType_eLast,
    BDSP_AF_P_BufferType_eInvalid = 0x7fffffff
}BDSP_AF_P_BufferType;

typedef enum BDSP_AF_P_Port_DataAccessType
{
    BDSP_AF_P_Port_eNone = 0,
    BDSP_AF_P_Port_eStandard = 1,
    BDSP_AF_P_Port_eSampledInterleavedPCM = 2,
    BDSP_AF_P_Port_eChannelInterleavedPCM = 3,
    BDSP_AF_P_Port_eMetaDataAccess = 4,
    BDSP_AF_P_Port_eLast,
    BDSP_AF_P_Port_eInvalid = 0x7fffffff
}BDSP_AF_P_Port_DataAccessType;

typedef enum BDSP_AF_P_NumPortBuffers
{
    BDSP_AF_P_NumPortBuffers_One = 1,
    BDSP_AF_P_NumPortBuffers_Two = 2,
    BDSP_AF_P_NumPortBuffers_Three = 3,
    BDSP_AF_P_NumPortBuffers_Four = 4,
    BDSP_AF_P_NumPortBuffers_MAX = 4,
    BDSP_AF_P_NumPortBuffers_INVALID = 0x7fffffff
}BDSP_AF_P_NumPortBuffers;

/* LEGACY CODE START */
typedef enum
{
   BDSP_VF_P_EncodeFrameRate_eUnknown = 0,
   BDSP_VF_P_EncodeFrameRate_e23_97,
   BDSP_VF_P_EncodeFrameRate_e24,
   BDSP_VF_P_EncodeFrameRate_e25,
   BDSP_VF_P_EncodeFrameRate_e29_97,
   BDSP_VF_P_EncodeFrameRate_e30,
   BDSP_VF_P_EncodeFrameRate_e50,
   BDSP_VF_P_EncodeFrameRate_e59_94,
   BDSP_VF_P_EncodeFrameRate_e60,
   BDSP_VF_P_EncodeFrameRate_e14_985,
   BDSP_VF_P_EncodeFrameRate_e7_493,
   BDSP_VF_P_EncodeFrameRate_e15,
   BDSP_VF_P_EncodeFrameRate_e10,
   BDSP_VF_P_EncodeFrameRate_e12_5,
   BDSP_VF_P_EncodeFrameRate_eMax,
   BDSP_VF_P_EncodeFrameRate_eInvalid = 0x7FFFFFFF
}BDSP_VF_P_eEncodeFrameRate;

/***************************************************************************
Summary:
    This is a version configuration structure.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VERSION_Table_Entry
{
    uint32_t AlgoId;
    char reference_version[32];
    char brcm_algo_version[32];
    char brcm_system_version[32];
}BDSP_VERSION_Table_Entry;


typedef struct BDSP_VERSION_Table
{
    BDSP_VERSION_Table_Entry sVersionTableDetail[1];
}BDSP_VERSION_Table;

typedef struct BDSP_AF_P_sDRAM_CIRCULAR_BUFFER
{
    dramaddr_t  ui32BaseAddr;       /*  Circular buffer's base address */
    dramaddr_t  ui32EndAddr;        /*  Circular buffer's End address */
    dramaddr_t  ui32ReadAddr;       /*  Circular buffer's read address */
    dramaddr_t  ui32WriteAddr;      /*  Circular buffer's write address */
    dramaddr_t  ui32WrapAddr;       /*  Circular buffer's wrap address */
}BDSP_AF_P_sDRAM_CIRCULAR_BUFFER;

typedef struct BDSP_AF_P_sOpSamplingFreq
{
    uint32_t ui32OpSamplingFrequency[BDSP_AF_P_SampFreq_eMax];
}BDSP_AF_P_sOpSamplingFreq;

typedef struct BDSP_AF_P_sSINGLE_CIRC_BUFFER
{

    BDSP_AF_P_BufferType                eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer;    /*  All circular buffer parameters
                                                        wrt to each input buffer comes */

}BDSP_AF_P_sSINGLE_CIRC_BUFFER;

typedef struct BDSP_AF_P_sIO_BUFFER
{
    uint32_t                        ui32NumBuffers;
    BDSP_AF_P_BufferType            eBufferType;
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER sCircBuffer[BDSP_AF_P_MAX_CHANNELS];
}BDSP_AF_P_sIO_BUFFER;

typedef struct BDSP_AF_P_sIO_GENERIC_BUFFER
{
    uint32_t                        ui32NumBuffers;
    BDSP_AF_P_BufferType            eBufferType;
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER sCircBuffer;
}BDSP_AF_P_sIO_GENERIC_BUFFER;

/***************************************************************************
Summary:
    The simple DRAM buffer structure which contains a DRAM address and Size.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sDRAM_BUFFER
{
    dramaddr_t                  ui32DramBufferAddress;
    uint32_t                    ui32BufferSizeInBytes;
}BDSP_AF_P_sDRAM_BUFFER;


/***************************************************************************
Summary:
    The structure contains the Picture Buffer addrs

Description:

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VF_P_sFrameBuffParams
{
    BDSP_AF_P_sDRAM_BUFFER        sFrameBuffLuma;
    BDSP_AF_P_sDRAM_BUFFER        sFrameBuffChroma;

}BDSP_VF_P_sFrameBuffParams;

/***************************************************************************
Summary:
    The structure contains the buffer parameters

Description:

See Also:
    None.
****************************************************************************/

typedef struct
{
    uint32_t                    ui32NumBuffAvl;     /* Number of Valid entries in sBuffParams array */

    /* Stripe height of frame buffer allocated */
    uint32_t                    ui32LumaStripeHeight;
    uint32_t                    ui32ChromaStripeHeight;

    /* These structure will have DRAM start addresses of the different frame buffers */
    BDSP_VF_P_sFrameBuffParams  sBuffParams[BDSP_FWMAX_VIDEO_BUFF_AVAIL];
}sFrameBuffHandle;

typedef struct BDSP_sEncodeParams
{
    BDSP_VF_P_eEncodeFrameRate          eEncodeFrameRate; /* Frame rate at which the encoder is expected to encode */
    uint32_t                            ui32Frames2Accum; /* = Gop2FrameAccumConvArray[Algo][BDSP_Raaga_Audio_eGOP_STRUCT] */

    /* This will hold the bit number [0...31] of ESR_SI register that will be used to interrupt DSP.
    * ping-pong design for interrupting raaga-dsp. Jason to confirm! */
    uint32_t                            ui32InterruptBit[BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP];
    /* 32bit RDB address of ENCODER's STC */
    uint32_t                            ui32StcAddr;
    /* 32bit RDB address of ENCODER's STC_HI for 42bit STC. In case of 32bit stc, ui32StcAddr_hi should be zero */
    uint32_t                            ui32StcAddr_hi;
    /* 32bit RDB address from DSP page where pic metadata address will be updated. It will hold a DRAM address */
    uint32_t                            ui32RdbForPicDescp[BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP];
    uint32_t                            IsGoBitInterruptEnabled;
    uint32_t                            IsNrtModeEnabled;
}BDSP_VF_P_sEncodeParams;

/*********************************************************************
Summary:
    This enum defines the FMM buffer Pause Burst Content:
    1) Zeroes
    2) Spdif formatted packet of Zeroes
Description:
    This enum is defined to inform FW about type of Pause bursts
    to use during ZERO- Filling in Decode task

See Also:
**********************************************************************/
typedef enum BDSP_AF_P_BurstFillType
{
    BDSP_AF_P_BurstFill_eZeroes,
    BDSP_AF_P_BurstFill_ePauseBurst,
    BDSP_AF_P_BurstFill_eNullBurst,
    BDSP_AF_P_BurstFill_eInvalid =0x7FFFFFFF
}BDSP_AF_P_BurstFillType;

/*********************************************************************
Summary:
    This enum defines the SPDIF Pause Burst Width:
    1) Six Words (16-bit Word)
    2) Eight Words (16-bit Word)
Description:
    This enum is defined to inform FW about the width of SPDIF Pause
    burst to use during ZERO- Filling in Decode task
**********************************************************************/
typedef enum BDSP_AF_P_SpdifPauseWidth
{
    BDSP_AF_P_SpdifPauseWidth_eSixWord,
    BDSP_AF_P_SpdifPauseWidth_eEightWord,
    BDSP_AF_P_SpdifPauseWidth_eHundredEightyEightWord,
    BDSP_AF_P_SpdifPauseWidth_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_SpdifPauseWidth;

typedef struct BDSP_Raaga_P_SCM_CmdPayload
{
    dramaddr_t  ui32DramCmdBufAddress;
    uint32_t    ui32DramCmdBufSize;
    dramaddr_t  ui32DramRespBufAddress;
    uint32_t    ui32DramRespBufSize;

}BDSP_Raaga_P_SCM_CmdPayload;

/* LEGACY CODE END */
typedef struct BDSP_P_MemoryInfo{
    dramaddr_t BaseAddr; /* Physical Start Address */
    uint32_t   Size;     /* Size*/
    uint32_t   Dummy;
}BDSP_P_MemoryInfo;

typedef struct BDSP_AF_P_sCIRCULAR_BUFFER
{
    dramaddr_t  BaseAddr;       /*  Circular buffer's base address */
    dramaddr_t  EndAddr;        /*  Circular buffer's End address */
    dramaddr_t  ReadAddr;       /*  Circular buffer's read address */
    dramaddr_t  WriteAddr;      /*  Circular buffer's write address */
    dramaddr_t  WrapAddr;       /*  Circular buffer's wrap address */
}BDSP_AF_P_sCIRCULAR_BUFFER;

typedef struct BDSP_AF_P_sEXTENDED_CIRCULAR_BUFFER
{
    BDSP_AF_P_sCIRCULAR_BUFFER sCircularBuffer;
    dramaddr_t startWriteAddr;
}BDSP_AF_P_sEXTENDED_CIRCULAR_BUFFER;

typedef struct BDSP_AF_P_sIoBuffer
{
    uint32_t ui32NumBuffer;
    BDSP_AF_P_BufferType eBufferType;
    dramaddr_t sCircularBuffer[BDSP_AF_P_MAX_CHANNELS];
}BDSP_AF_P_sIoBuffer;

typedef struct BDSP_AF_P_Port_sDataAccessAttributes
{
    BDSP_AF_P_Port_DataAccessType eDataAccessType;
    uint32_t ui32maxChannelSize;
    uint32_t ui32numChannels;
    uint32_t ui32bytesPerSample;
}BDSP_AF_P_Port_sDataAccessAttributes;

typedef struct BDSP_AF_P_sIoPort
{
    BDSP_AF_P_PortType ePortType;
    BDSP_AF_P_DistinctOpType ePortDataType;

    uint32_t ui32numPortBuffer; /* PCM, TOC, Metadata, Objectdata*/
    uint32_t ui32tocIndex;
    uint32_t ui32numBranchfromPort;
    uint32_t ui32Dummy;/*Element Added to make structure size multiple of 8*/

    dramaddr_t sIOGenericBuffer; /*FMM cases in future */
    BDSP_AF_P_sIoBuffer sIoBuffer[BDSP_AF_P_MAX_PORT_BUFFERS];

    BDSP_AF_P_Port_sDataAccessAttributes sDataAccessAttributes;
}BDSP_AF_P_sIoPort;

typedef struct BDSP_AF_P_sIoConfig
{
    uint32_t ui32NumInputs;
    uint32_t ui32NumOutputs;

    BDSP_AF_P_sIoPort sInputPort[BDSP_AF_P_MAX_IP_FORKS];
    BDSP_AF_P_sIoPort sOutputPort[BDSP_AF_P_MAX_OP_FORKS];
}BDSP_AF_P_sIOConfig;

typedef struct BDSP_AF_P_sSchedulingInfo
{
    uint32_t ui32IndependentDelay;
    uint32_t ui32MaxIndependentDelay;
    uint32_t ui32BlockingTime;
    uint32_t ui32FixedSampleRate;

    BDSP_AF_P_Boolean      bFixedSampleRate;
    BDSP_AF_P_FmmDstFsRate eBaseRateMultiplier;
    BDSP_AF_P_FmmContentType eFMMContentType;
    uint32_t ui32Dummy;
}BDSP_AF_P_sSchedulingInfo;

typedef struct BDSP_AF_P_sRING_BUFFER_INFO
{
    uint32_t                  ui32NumBuffers;
    uint32_t                  ui32IndependentDelay;

    uint32_t                  ui32FixedSampleRate;
    BDSP_AF_P_Boolean         bFixedSampleRate;

    BDSP_AF_P_FmmContentType  eFMMContentType;
    BDSP_AF_P_FmmDstFsRate    eBaseRateMultiplier;
    BDSP_AF_P_BufferType      eBufferType;
    uint32_t                  ui32Dummy0;

    BDSP_AF_P_sEXTENDED_CIRCULAR_BUFFER sExtendedBuffer[BDSP_AF_P_MAX_CHANNELS];
} BDSP_AF_P_sRING_BUFFER_INFO;

typedef struct BDSP_AF_P_sTASK_GATEOPEN_CONFIG
{
    uint32_t ui32NumPorts;
    uint32_t ui32MaxIndependentDelay;
    uint32_t ui32BlockingTime;
    uint32_t ui32Dummy;

    BDSP_AF_P_sRING_BUFFER_INFO sRingBufferInfo[BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK];
}BDSP_AF_P_sTASK_GATEOPEN_CONFIG;

typedef struct BDSP_AF_P_sTASK_SCHEDULING_CONFIG
{
    BDSP_AF_P_BufferType  eBufferType;
    uint32_t              ui32Dummy;

    BDSP_AF_P_sEXTENDED_CIRCULAR_BUFFER sExtendedBuffer;
    BDSP_AF_P_sSchedulingInfo       sSchedulingInfo;
}BDSP_AF_P_sTASK_SCHEDULING_CONFIG;

typedef struct BDSP_AF_P_sHW_PPM_CONFIG
{
    BDSP_AF_P_EnableDisable ePPMChannel;
    uint32_t                ui32Dummy;

    dramaddr_t              PPMCfgAddr;
}BDSP_AF_P_sHW_PPM_CONFIG;

typedef struct BDSP_AF_P_sSTC_TRIGGER_CONFIG
{
    /* If soft triggering is required. Default = BDSP_AF_P_eDisable */
    BDSP_AF_P_EnableDisable eStcTrigRequired;
    /* High and Low part of registers to tell the amount of STC increment. */
    uint32_t                ui32StcIncHiAddr;
    uint32_t                ui32StcIncLowAddr;
    /* Address of register to send trigger for incrementing STC */
    uint32_t                ui32stcIncTrigAddr;
    /* Trigger bit in the above register. Bit count [031]*/
    uint32_t                ui32TriggerBit;
    uint32_t                ui32Dummy;
}BDSP_AF_P_sSTC_TRIGGER_CONFIG;

typedef struct BDSP_AF_P_sGLOBAL_TASK_CONFIG
{
    uint32_t                    ui32NumStagesInTask;
    uint32_t                    ui32ScratchSize;
    uint32_t                    ui32InterStageSize;
    uint32_t                    ui32Dummy;

    BDSP_P_MemoryInfo           sGateOpenConfigInfo;
    BDSP_P_MemoryInfo           sSchedulingInfo;
    BDSP_P_MemoryInfo           sStcTriggerInfo;
    BDSP_P_MemoryInfo           sDataSyncUserConfigInfo;
    BDSP_P_MemoryInfo           sTsmConfigInfo;
}BDSP_AF_P_sGLOBAL_TASK_CONFIG;

typedef struct BDSP_AF_P_sPRIMARYSTAGE_INFO
{
    uint32_t                    ui32NumZeroFillSamples;
    BDSP_AF_P_EnableDisable     ePPMCorrectionEnable;
    BDSP_AF_P_EnableDisable     eOpenGateAtStart;
    BDSP_AF_P_TimeBaseType      eTimeBaseType;

    BDSP_AF_P_sHW_PPM_CONFIG    sPPMConfig[BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS];
    BDSP_P_MemoryInfo           sSamplingFrequencyLutInfo;
    BDSP_P_MemoryInfo           sDataSyncStatusInfo;
    BDSP_P_MemoryInfo           sTsmStatusInfo;
    BDSP_P_MemoryInfo           sIdsCodeInfo;
}BDSP_AF_P_sPRIMARYSTAGE_INFO;

typedef struct BDSP_AF_P_sSTAGE_CONFIG
{
    uint32_t                ui32StageId;
    BDSP_AF_P_EnableDisable eCollectResidual;
    BDSP_Algorithm          eAlgorithm;
    uint32_t                ui32Dummy;

    BDSP_P_MemoryInfo       sStageMemoryInfo;
    BDSP_P_MemoryInfo       sAlgoUserConfigInfo;
    BDSP_P_MemoryInfo       sAlgoStatusInfo;
    BDSP_P_MemoryInfo       sInterFrameInfo;
    BDSP_P_MemoryInfo       sLookUpTableInfo;
    BDSP_P_MemoryInfo       sAlgoCodeInfo;

    BDSP_AF_P_sIOConfig     sIOConfig;
}BDSP_AF_P_sSTAGE_CONFIG;

typedef struct BDSP_AF_P_sTASK_CONFIG
{
    BDSP_AF_P_sGLOBAL_TASK_CONFIG   sGlobalTaskConfig;
    BDSP_AF_P_sPRIMARYSTAGE_INFO    sPrimaryStageInfo;
    BDSP_AF_P_sSTAGE_CONFIG         sStageConfig[BDSP_AF_P_MAX_STAGES];
}BDSP_AF_P_sTASK_CONFIG;

#endif /*BDSP_RAAGA_FW_H*/
