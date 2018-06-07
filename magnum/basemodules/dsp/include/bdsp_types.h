/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BDSP_TYPES_H_
#define BDSP_TYPES_H_

#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"
#include "btmr.h"
#include "bimg.h"
#include "bavc.h"
#include "bavc_xpt.h"
#include "bdsp_algorithm.h"

/***************************************************************************
Summary:
DSP Handle
***************************************************************************/
typedef struct BDSP_Device *BDSP_Handle;

/***************************************************************************
Summary:
DSP Context Handle
***************************************************************************/
typedef struct BDSP_Context *BDSP_ContextHandle;

/***************************************************************************
Summary:
DSP Task Handle
***************************************************************************/
typedef struct BDSP_Task *BDSP_TaskHandle;


/***************************************************************************/
/* Handle for an inter-task buffer */
typedef struct BDSP_InterTaskBuffer *BDSP_InterTaskBufferHandle;

/***************************************************************************/
/* Handle for an Queue */
typedef struct BDSP_Queue *BDSP_QueueHandle;

/***************************************************************************
Summary:
DSP Stage Handle
***************************************************************************/
typedef struct BDSP_Stage *BDSP_StageHandle;

/***************************************************************************
Summary:
DSP External Interrupt Handle
***************************************************************************/
typedef struct BDSP_ExternalInterrupt *BDSP_ExternalInterruptHandle;

/***************************************************************************
Summary:
Placeholder Types
***************************************************************************/
typedef uint32_t BDSP_TIME_45KHZ_TICKS;
/***************************************************************************
Summary:
Type for declaring dram offset variable
***************************************************************************/
#ifdef BDSP_32BIT_ARCH
typedef uint32_t dramaddr_t;
#define BDSP_MSG_FMT "0x%x"
#define BDSP_MSG_ARG(x) (unsigned)(x)
#else
typedef uint64_t dramaddr_t;
#define BDSP_MSG_FMT BDBG_UINT64_FMT
#define BDSP_MSG_ARG(x) BDBG_UINT64_ARG(x)
#endif/*BDSP_32BIT_ARCH*/
/***************************************************************************
Summary:
Task Types
***************************************************************************/
typedef enum BDSP_ContextType
{
    BDSP_ContextType_eAudio,
    BDSP_ContextType_eVideo,
    BDSP_ContextType_eVideoEncode,
    BDSP_ContextType_eGraphics,
    BDSP_ContextType_eScm,
    BDSP_ContextType_eMax
} BDSP_ContextType;

/***************************************************************************
Summary:
BDSP Dolby Codec Version
**************************************************************************/
typedef enum BDSP_AudioDolbyCodecVersion
{
    BDSP_AudioDolbyCodecVersion_eAC3,
    BDSP_AudioDolbyCodecVersion_eDDP,
    BDSP_AudioDolbyCodecVersion_eMS10,
    BDSP_AudioDolbyCodecVersion_eMS11,
    BDSP_AudioDolbyCodecVersion_eMS12,
    BDSP_AudioDolbyCodecVersion_eMax
} BDSP_AudioDolbyCodecVersion;

/***************************************************************************
Summary:
    DSP Processing Algorithm Types.
***************************************************************************/
typedef enum BDSP_AlgorithmType
{
    BDSP_AlgorithmType_eAudioDecode,
    BDSP_AlgorithmType_eAudioPassthrough,
    BDSP_AlgorithmType_eAudioEncode,
    BDSP_AlgorithmType_eAudioMixer,
    BDSP_AlgorithmType_eAudioEchoCanceller,
    BDSP_AlgorithmType_eAudioProcessing,
    BDSP_AlgorithmType_eVideoDecode,
    BDSP_AlgorithmType_eVideoEncode,
    BDSP_AlgorithmType_eSecurity,
    BDSP_AlgorithmType_eCustom,
    BDSP_AlgorithmType_eMax
} BDSP_AlgorithmType;

/* Representation of data types (internally maps to BDSP_AF_P_DistinctOpType) */
typedef enum BDSP_DataType
{
    BDSP_DataType_ePcmMono,         /* True mono PCM Data.  */
    BDSP_DataType_ePcmStereo,       /* PCM 2.0 data */
    BDSP_DataType_ePcm5_1,          /* PCM 5.1 data */
    BDSP_DataType_ePcm7_1,          /* PCM 7.1 data */
    BDSP_DataType_ePcmRf,           /* RF Encoded PCM Data (e.g. BTSC encoder output).  Encoded as mono 32-bit PCM running at 4x the sample rate. */
    BDSP_DataType_eIec61937,        /* IEC-61937 compressed data for SPDIF/HDMI including preambles. */
    BDSP_DataType_eIec61937x4,      /* IEC-61937 compressed data for HDMI running at 4x the sample rate (e.g. AC3+ Passthrough) */
    BDSP_DataType_eIec61937x16,     /* HDMI HBR compressed data.  Runs at 4x the sample rate with 4 samples per clock.  */
    BDSP_DataType_eCompressedRaw,   /* RAW Compressed data without 61937 formatting (output from some encoders) */
    BDSP_DataType_eRave,            /* RAVE data, sent in separate CDB/ITB buffers. */
    BDSP_DataType_eDolbyTranscodeData,   /* Auxiliary data for DolbyTranscode */
    BDSP_DataType_eRdbCdb,               /* RDB Queue data, sent in separate CDB buffers. */
    BDSP_DataType_eRdbItb,               /* RDB Queue data, sent in separate ITB buffers. */
    BDSP_DataType_eRdbAnc,               /* Mpeg Ancillary data, RDB registers used.  */
    BDSP_DataType_eRDBPool,              /* Queue in RDB for pooled buffers in DRAM */
    BDSP_DataType_eMax
} BDSP_DataType;

/* Represents the interface buffer type */
typedef enum BDSP_BufferType
{
  BDSP_BufferType_eDRAM   = 0,    /*  This is used in case of a DRAM ring buffer,   where in the read, write pointers are also in DRAM. */
  BDSP_BufferType_eRDB,         /*  This is used in case of a DRAM ring buffer, where in the read, write pointers are in RDB registers. */
  BDSP_BufferType_eLAST,
  BDSP_BufferType_eInvalid = 0x7FFFFFFF
} BDSP_BufferType;

typedef enum BDSP_ConnectionType
{
    BDSP_ConnectionType_eFmmBuffer,
    BDSP_ConnectionType_eRaveBuffer,
    BDSP_ConnectionType_eStage,
    BDSP_ConnectionType_eInterTaskBuffer,
    BDSP_ConnectionType_eRDBBuffer,
    BDSP_ConnectionType_eMax
} BDSP_ConnectionType;

typedef enum BDSP_MMA_Alignment{
    BDSP_MMA_Alignment_8bit    =   1, /*   1 -byte alignment */
    BDSP_MMA_Alignment_16bit   =   2, /*   2 -byte alignment */
    BDSP_MMA_Alignment_32bit   =   4, /*   4 -byte alignment */
    BDSP_MMA_Alignment_64bit   =   8, /*   8 -byte alignment */
    BDSP_MMA_Alignment_128bit  =  16, /*  16 -byte alignment */
    BDSP_MMA_Alignment_256bit  =  32, /*  32 -byte alignment */
    BDSP_MMA_Alignment_512bit  =  64, /*  64 -byte alignment */
    BDSP_MMA_Alignment_1024bit = 128, /* 128 -byte alignment */
    BDSP_MMA_Alignment_2048bit = 256, /* 256 -byte alignment */
    BDSP_MMA_Alignment_4096bit = 512, /* 512 -byte alignment */
	BDSP_MMA_Alignment_1KByte  = 1024, /* 1K -byte alignment */
    BDSP_MMA_Alignment_2KByte  = 2048, /* 2K -byte alignment */
	BDSP_MMA_Alignment_4KByte  = 4096, /* 4K -byte alignment */
    BDSP_MMA_Alignment_Max,
    BDSP_MMA_Alignment_Invalid = 0x7FFFFFFF
}BDSP_MMA_Alignment;

/***************************************************************************
Summary:
    This enumeration defines various debug features that can be enabled in the firmware.

***************************************************************************/
typedef enum BDSP_DebugType
{
    BDSP_DebugType_eDramMsg = 0,
    BDSP_DebugType_eUart,
    BDSP_DebugType_eCoreDump,
    BDSP_DebugType_eTargetPrintf,
    BDSP_DebugType_eLast,
    BDSP_DebugType_eInvalid = 0x7FFFFFFF
} BDSP_DebugType;

/***************************************************************************
Summary:
Debug Type Settings
***************************************************************************/
typedef struct BDSP_DebugTypeSettings
{
    bool enabled;        /* If true, debug of this type is enabled. */
    uint32_t bufferSize; /* Size of debug buffer (in bytes) for a particular type of debug.
                                                        Only required if you want to override the default value. */
} BDSP_DebugTypeSettings;

typedef struct BDSP_StageCreateSettings
{
    BDSP_AlgorithmType algoType;
    bool algorithmSupported[BDSP_Algorithm_eMax];
    unsigned maxInputs;
    unsigned maxOutputs;
} BDSP_StageCreateSettings;

typedef struct BDSP_MMA_Memory{
    BMMA_Block_Handle hBlock;
    BMMA_DeviceOffset offset;
    void *pAddr;
}BDSP_MMA_Memory;

/* Add an output buffer to a stage */
typedef struct BDSP_FmmBufferDescriptor
{
    unsigned numBuffers;
    struct
    {
        uint32_t base;
        uint32_t end;
        uint32_t read;
        uint32_t write;
        uint32_t wrpoint;
    } buffers[8];
    unsigned numRateControllers;
    struct
    {
        uint32_t wrcnt;
    } rateControllers[4];
    unsigned delay; /* Independent delay in milli seconds */
} BDSP_FmmBufferDescriptor;

/***************************************************************************
Summary:
Use case scenario provided by APE
***************************************************************************/
typedef struct BDSP_UsageOptions
{
    bool           Codeclist[BDSP_Algorithm_eMax];  /* Total list containing the Codecs enabled or disabled */
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion;
    BDSP_DataType IntertaskBufferDataType;
    unsigned NumAudioDecoders;
    unsigned NumAudioPostProcesses;
    unsigned NumAudioEncoders;
    unsigned NumAudioMixers;
    unsigned NumAudioPassthru;
    unsigned NumAudioEchocancellers;
    unsigned NumVideoDecoders;
    unsigned NumVideoEncoders;
} BDSP_UsageOptions;

/***************************************************************************
Summary:
Memory Requirement Status
***************************************************************************/
typedef struct BDSP_MemoryEstimate
{
    unsigned GeneralMemory; /* Number of bytes from the general system heap */
    unsigned FirmwareMemory; /* Number of bytes from the firmware heap */
} BDSP_MemoryEstimate;

#define BDSP_RaagaUsageOptions   BDSP_UsageOptions
#define BDSP_RaagaMemoryEstimate BDSP_MemoryEstimate
#endif
