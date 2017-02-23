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
 *****************************************************************************/

#ifndef BDSP_COMMON_FW_H_
#define BDSP_COMMON_FW_H_

#include "bstd.h"
#include "bdsp_types.h"

/* Enable the Capture Port cfg to FW */
#define BDSP_CAP_PORT_SUPPORT
#define BDSP_DUMMY_PORT_SUPPORT

/*  Max channel for 7.1 + concurrent stereo o/p */
#define BDSP_AF_P_MAX_CHANNELS              ( (uint32_t) 8 )

#define BDSP_AF_P_MAX_CHANNEL_PAIR          (uint32_t)((BDSP_AF_P_MAX_CHANNELS+1)>>1)

/*  Based on the current DTS algorithm which has 5 decode stages
    One node is intially reserved for Frame sync */
#define BDSP_AF_P_MAX_NUM_NODES_IN_ALGO         ( 5 + 1 )

/*  Max number of nodes for the current worst case scenario */
#define BDSP_AF_P_MAX_NODES                 ( (uint32_t) 10 )

/*  Max number of output ports. Currently we have accomadated
    for three branches, each branch having two o/p ports*/
#define BDSP_AF_P_MAX_OP_PORTS              6

/*  Defines for the FMM o/p buff configuration */
#define BDSP_AF_P_MAX_NUM_DAC_PORTS         3   /* There are only 2 HIFIDACS for 7405/7325 family */
                                                /* There are only 3 HIFIDACS for 3548 family */
#define BDSP_AF_P_MAX_NUM_SPDIF_PORTS       2
#define BDSP_AF_P_MAX_NUM_PLLS              4
#define BDSP_AF_P_MAX_NUM_I2S_PORTS         4   /* 4 I2S ports: STEREO_0, STEREO_1, MULTI_0, MULTI_1 */
#define BDSP_AF_P_MAX_NUM_CAP_PORTS         4
#define BDSP_AF_P_MAX_NUM_DUMMY_PORTS       4   /* Dummy Ports */

/* Maximum number of FMM Io buffer ports in a task*/
#define  BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK (6)

/*  This defines the maximum number of different sources the node
    can take input from */
#define BDSP_AF_P_MAX_IP_FORKS              4
/*  This defines the maximum number of different destinations the node
    can output */
#define BDSP_AF_P_MAX_OP_FORKS              6

/*Maximum number of Adaptive blocks
    PPM logic is using BDSP_AF_P_MAX_CHANNEL_PAIR for its operations as the CIT-PI interface is based on BDSP_AF_P_MAX_CHANNEL_PAIR.
    And BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS is used for better readability in CIT-FW interface.
    Actual HW block number is 4
*/
#define BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS      8

/**************************************************************************
        Inter Task Communication Buffer allocation
***************************************************************************/
#define BDSP_AF_P_INTERTASK_IOBUFFER_SIZE       (uint32_t) ((6144*6*8)+4)/*(11200*4)*/
#define BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE    (uint32_t) ((4544*6*8)+4)/*(17408*2)*/

/**************************************************************************
Summary:
    Define to calculate the Read/Write offset for BDSP_AF_P_sDRAM_CIRCULAR_BUFFER
Description:
    structure is put in memory starting from address 0, a pointer to the element inside the
    structure is also the same as the offset, since the base address is 0; i.e the address of
    that element should be the same as offset, as the structure is assumed to be laid in the
    memory starting at address 0
See Also:
    NOTE: The linux kernel defines a similar macro in the include/linux/stddef.h called offsetof
***************************************************************************/
#define BDSP_CIRC_BUFF_OFFSETOF(TYPE, ELEMENT)      ((uint32_t)&((TYPE *)0)->ELEMENT)

/***************************************************************************
Summary:
    Enum data type describing Enable and Disable

Description:

  Valid  =1;
  Invalid=0;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_EnableDisable
{
    BDSP_AF_P_eDisable = 0x0,
    BDSP_AF_P_eEnable  = 0x1,
    BDSP_AF_P_EnableDisable_eMax,
    BDSP_AF_P_EnableDisable_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_EnableDisable;

/***************************************************************************
Summary:
    Enum data types used to check a field is valid/invalid
Description:

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_ValidInvalid
{
    BDSP_AF_P_eInvalid  = 0x0,
    BDSP_AF_P_eValid    = 0x1,
    BDSP_AF_P_ValidInvalid_eMax,
    BDSP_AF_P_ValidInvalid_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_ValidInvalid;

/***************************************************************************
Summary:
    Enum data type describing the types of Port Enable Fields

Description:
    Enum data type describing the types of Port Enable Fields

  Enable =0;
  Disable=1;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_PortEnableDisable
{
    BDSP_AF_P_PortEnable  = 0x0,
    BDSP_AF_P_PortDisable = 0x1,
    BDSP_AF_P_PortEnableDisable_eMax,
    BDSP_AF_P_PortEnableDisable_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_PortEnableDisable;

/*********************************************************************
Summary:
    This enum defines the DSP mode output type

Description:

See Also:
**********************************************************************/
typedef enum BDSP_AF_P_DistinctOpType
{
    BDSP_AF_P_DistinctOpType_e7_1_MixPcm,
    BDSP_AF_P_DistinctOpType_e5_1_MixPcm,       /* mixed 5.1 PCM */
    BDSP_AF_P_DistinctOpType_eStereo_MixPcm,        /* mixed stereo PCM */
    BDSP_AF_P_DistinctOpType_e7_1_PCM,              /* 7.1 PCM */
    BDSP_AF_P_DistinctOpType_e5_1_PCM,              /* 5.1 PCM */
    BDSP_AF_P_DistinctOpType_eStereo_PCM,           /* Downmixed to stereo PCM */
    BDSP_AF_P_DistinctOpType_eMono_PCM,                 /* Mono output type to be used with speech codecs (G.7XX) and AEC modules */
    BDSP_AF_P_DistinctOpType_eCompressed,           /* Compressed Data */
    BDSP_AF_P_DistinctOpType_eAuxilaryDataOut,      /* Auxillary Data like 1) Converter input data 2) ITB generation data etc*/
    BDSP_AF_P_DistinctOpType_eGenericIsData,        /* Any kind of data on Interstage buffer*/
    BDSP_AF_P_DistinctOpType_eCdb,                  /* Compressed Elementary stream to be filled in CDB */
    BDSP_AF_P_DistinctOpType_eItb,                  /* ITB data going out of DSP */
    BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut,  /* Re-encode auxiliary data out */
    BDSP_AF_P_DistinctOpType_eCompressed4x,         /* 4x Compressed Rate - for DD+/DTS-HRA Passthru */
    BDSP_AF_P_DistinctOpType_eCompressedHBR,        /* HBR Passthru - True-HD/DTS-MA Passthru  */
    BDSP_AF_P_DistinctOpType_eAncillaryData,        /* MPEG Ancillary Data  */
    BDSP_AF_P_DistinctOpType_eDescriptorQueue,      /* Descriptor Queue for the Pooled buffers*/
    BDSP_AF_P_DistinctOpType_eMax,
    BDSP_AF_P_DistinctOpType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_DistinctOpType;

/***************************************************************************
Summary:
    Enum data type describing the FMM Destination's associated sinking rate

Description:

     FMM Destination can be. FMM port or Capture port.
     There are a few rates classified for FMM Destination.

                    1)BaseRate
                    2)StreamSamplingRate
                    3)2xBaseRate
                    4)4xBaseRate
                    5)HBRRate

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_FmmDstFsRate
{
    BDSP_AF_P_FmmDstFsRate_eBaseRate            = 0x0,
    BDSP_AF_P_FmmDstFsRate_eStreamSamplingRate  = 0x1,
    BDSP_AF_P_FmmDstFsRate_e2xBaseRate          = 0x2,
    BDSP_AF_P_FmmDstFsRate_e4xBaseRate          = 0x3,
    BDSP_AF_P_FmmDstFsRate_e16xBaseRate         = 0x4,
    BDSP_AF_P_FmmDstFsRate_eHBRRate             = BDSP_AF_P_FmmDstFsRate_e16xBaseRate,
    BDSP_AF_P_FmmDstFsRate_eInvalid,
    BDSP_AF_P_FmmDstFsRate_eMax                 = 0x7FFFFFFF

}BDSP_AF_P_FmmDstFsRate;


/*********************************************************************
Summary:
    This enum defines the FMM buffer content:
    1) PCM content
    2) Compressed content
Description:
    This enum is inially defined to inform FW to do Zero / Pause bursts
    during ZERO- Filling in Decode task

See Also:
**********************************************************************/

typedef enum BDSP_AF_P_FmmContentType
{

    BDSP_AF_P_FmmContentType_ePcm,              /* PCM */
    BDSP_AF_P_FmmContentType_eCompressed,       /* Compressed Data */
    BDSP_AF_P_FmmContentType_eAnalogCompressed, /* Analog Compressed Data for BTSC Encoder*/
    BDSP_AF_P_FmmContentType_eMax,
    BDSP_AF_P_FmmContentType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_FmmContentType;


/***************************************************************************
Summary:
    Enum to hold the type of the buffer.

Description:
    The buffer type indicates the type of i/o buffer being configured as
    input or output for the current node.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_BufferType
{
    BDSP_AF_P_BufferType_eDRAM      = 0,        /*  This is used in case of a DRAM ring buffer,
                                                where in the read, write pointers are also in DRAM. */
    BDSP_AF_P_BufferType_eRDB,                  /*  This is used in case of a ring buffer,
                                                where in the read, write pointers are in RDB registers. */
    BDSP_AF_P_BufferType_eFMM,                  /*  Used in case of FMM ring buffers. The address of the
                                                registers holding the read,write pointers etc will be
                                                programmed by the host.In this case, a GISB access in done
                                                by the firmware to get the actual read,write etc. pointers */
    BDSP_AF_P_BufferType_eRAVE,                 /*  Used in case of RAVE ring buffers. Need to differentiate
                                                between FMM and RAVE registers as RAVE registers use the
                                                wrap bit for buffer wrap around indication */
    BDSP_AF_P_BufferType_eDRAM_IS,              /*  DRAM inter stage buffer. Only the start address of the
                                                buffer has to be programmed by the host. */

    BDSP_AF_P_BufferType_eFMMSlave,             /* This type shows that the Buffer is FMM and its is configured
                                                as Slave */

    BDSP_AF_P_BufferType_eRDBPool,                 /* This type is used when a pool of buffers are allocated in DRAM,
                                                   the pointers to which are in DRAM, but the pointer is passed as data
                                                   through RDB registers */

    BDSP_AF_P_BufferType_eLAST,
    BDSP_AF_P_BufferType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_BufferType;


/* Time base of the System */
typedef enum BDSP_AF_P_TimeBaseType
{
    BDSP_AF_P_TimeBaseType_e45Khz = 0,
    BDSP_AF_P_TimeBaseType_e27Mhz,
    BDSP_AF_P_TimeBaseType_eLast,
    BDSP_AF_P_TimeBaseType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_TimeBaseType;


/***************************************************************************
Summary:
    Enum data type describing the types of inter-frame buffers in the audio
    firmware.

Description:
    This enum describes the type of the inter-frame buffer being used by the
    node. The inter-frame buffer can be present, absent or shared. when shared,
    it means that the current node of the algo shares the inter-frame buffer
    with the previous node in the same algo.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_InterFrameBuffType
{
    BDSP_AF_P_InterFrameBuffType_eAbsent = 0x0,
    BDSP_AF_P_InterFrameBuffType_ePresent,
    BDSP_AF_P_InterFrameBuffType_eShared,

    BDSP_AF_P_InterFrameBuffType_eMax,
    BDSP_AF_P_InterFrameBuffType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_InterFrameBuffType;


/***************************************************************************
Summary:
    Enum data type describing the types of Firmware-Status buffers in the audio
    firmware.

Description:
    This enum describes the type of the Firmware-Status buffer being used by the
    node. The Firmware-Status buffer can be present, absent or shared. when shared,
    it means that the current node of the algo shares the Firmware-Status buffer
    with the previous node in the same algo.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_FwStatus
{
    BDSP_AF_P_FwStatus_eAbsent = 0x0,
    BDSP_AF_P_FwStatus_ePresent,
    BDSP_AF_P_FwStatus_eShared,

    BDSP_AF_P_FwStatus_eMax,
    BDSP_AF_P_FwStatus_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_FwStatus;

/***************************************************************************
Summary:
    Enum data type describing PLL or NCO Select

Description:

        PLL Select
        NCO Select

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_PllNcoSelect
{
    BDSP_AF_P_NoneSelect = 0x0, /*This is for default case */
    BDSP_AF_P_PllSelect  = 0x1,
    BDSP_AF_P_NcoSelect  = 0x2,

    BDSP_AF_P_PllNcoSelect_eMax,
    BDSP_AF_P_PllNcoSelect_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_PllNcoSelect;

/***************************************************************************
Summary:
    Enum data type describing I2S MCLK Rate

Description:

  BDSP_AF_P_e256fs  = 0;
  BDSP_AF_P_e384fs  = 1;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_I2sMclkRate
{
    BDSP_AF_P_e256fs = 0x0,
    BDSP_AF_P_e384fs  = 0x1,
    BDSP_AF_P_I2sMclkRate_eMax,
    BDSP_AF_P_I2sMclkRate_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_I2sMclkRate;

/***************************************************************************
Summary:
    Enum data type describing MS decode mode

Description:
        Single Decode
        MS10/Ms11/MSSoundEffect
See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_DolbyMsUsageMode
{
    BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode=0,
    BDSP_AF_P_DolbyMsUsageMode_eMS10DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eMS11DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eMpegConformanceMode,
    BDSP_AF_P_DolbyMsUsageMode_eMS11SoundEffectMixing,
    BDSP_AF_P_DolbyMsUsageMode_eMS12DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eLAST,
    BDSP_AF_P_DolbyMsUsageMode_eINVALID=0x7fffffff

} BDSP_AF_P_DolbyMsUsageMode;

/***************************************************************************
Summary:
    Enum data type describing the types of Sampling Frequency in the audio
    firmware.

    Structure definition to map the input sampling frequency to output
    sampling frequency.

See Also:
    None.
****************************************************************************/

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


typedef enum BDSP_PtsType
{
    BDSP_PtsType_eCoded,
    BDSP_PtsType_eInterpolatedFromValidPTS,
    BDSP_PtsType_eInterpolatedFromInvalidPTS = 3,
    BDSP_PtsType_eLast,
    BDSP_PtsType_eInvalid = 0x7FFFFFFF
}BDSP_PtsType;

/***************************************************************************
Summary:
    Enum data type describing MS decoder Type

Description:
        Primary/Secondary/SoundEffect
See Also:
    None.
****************************************************************************/
typedef enum
{
    BDSP_AF_P_DecoderType_ePrimary = 0,
    BDSP_AF_P_DecoderType_eSecondary,
    BDSP_AF_P_DecoderType_eSoundEffects,
    BDSP_AF_P_DecoderType_eApplicationAudio,
    BDSP_AF_P_DecoderType_eLAST,
    BDSP_AF_P_DecoderType_eINVALID = 0x7fffffff
}BDSP_AF_P_DecoderType ;


/***************************************************************************
Summary:
    Node info structure between the firmware and the PI in the audio
    firmware.

Description:
    Each executable to provide info to the CIT generation module on the
    sizes of individual buffers. This is to be done through this structure.
    The max frame size and the max num channels supported will be used to
    internally partition the inter-stage I/O buffer into different channels

    How these two fields should be set for a post processing algorithm: The
    frame size must be set to the minimum chunk the post proc algo can
    work on. The number of channels supported must be the max it can support.
    Ex: DDBM works on 512 samples and 7.1 channels on 7440.

    To save DRAM buffer, the file where this structure sits for an algorithm
    can be made chip specific. Some chips requiring to support 7.1 will need
    additional memory

    To save some more DRAM buffer, while filling the data for post proc, we
    can ensure that the number of channels is set to a minimum (say stereo)
    this will ensure that the acmod of the decoder/encoder dictate the number
    of channel buffers required.
    The above is a drawback in a pure post process task where we may be
    working on 5.1 channels.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sNODE_INFO
{
    uint32_t                        ui32CodeSize;
    uint32_t                        ui32RomTableSize;
    uint32_t                        ui32InterFrmBuffSize;
    uint32_t                        ui32InterStgIoBuffSize;
    uint32_t                        ui32InterStgGenericBuffSize;
    uint32_t                        ui32ScratchBuffSize;
    uint32_t                        ui32UserCfgBuffSize;
    uint32_t                        ui32MaxSizePerChan;
    uint32_t                        ui32MaxNumChansSupported;
    BDSP_AF_P_InterFrameBuffType    eInterFrameBuffType;

    BDSP_AF_P_FwStatus              eStatusBuffType;
    uint32_t                        ui32StatusBuffSize;


} BDSP_AF_P_sNODE_INFO;

/***************************************************************************
Summary:
    Structure that holds all the genereic circular buffer parameters.

Description:
    This structure contains the unified circular buffer structure. All the
    circular buffer parameters like read/write base and end parameters are
    present in this structure. The circular buffer structure has been unified
    taking into account both the FMM and RAVE type of ciruclar buffer.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sDRAM_CIRCULAR_BUFFER
{
    dramaddr_t  ui32BaseAddr;       /*  Circular buffer's base address */
    dramaddr_t  ui32EndAddr;        /*  Circular buffer's End address */
    dramaddr_t  ui32ReadAddr;       /*  Circular buffer's read address */
    dramaddr_t  ui32WriteAddr;  /*  Circular buffer's write address */
    dramaddr_t  ui32WrapAddr;       /*  Circular buffer's wrap address */
}BDSP_AF_P_sDRAM_CIRCULAR_BUFFER;


/***************************************************************************
Summary:
    The single circ buffer structure used to provide detaios of one circular
    buffer of any buffer type

Description:

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sSINGLE_CIRC_BUFFER
{

    BDSP_AF_P_BufferType                eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer;    /*  All circular buffer parameters
                                                        wrt to each input buffer comes */

}BDSP_AF_P_sSINGLE_CIRC_BUFFER;


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

typedef struct BDSP_AF_P_sDRAM_BUFFER_isr
{
    dramaddr_t                  ui32DramBufferAddress;
    uint32_t                    ui32BufferSizeInBytes;
    void                       *pDramBufferAddress;
}BDSP_AF_P_sDRAM_BUFFER_isr;

/***************************************************************************
Summary:
    The I/O buffer structure used for as configuration for each node.

Description:
    This structure contains the I/O buffer configuration of a node.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sIO_BUFFER
{
    uint32_t                        ui32NumBuffers; /*  Defines the number of
                                                        channels in the input
                                                        or output */
    BDSP_AF_P_BufferType            eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer[BDSP_AF_P_MAX_CHANNELS];    /*  All circular buffer
                                                                                parameters wrt to each
                                                                                input buffer comes */

}BDSP_AF_P_sIO_BUFFER;

/***************************************************************************
Summary:
    The Generic buffer structure used as configuration for each node.

Description:
    This structure contains the configuration for the genric buffer of a node.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sIO_GENERIC_BUFFER
{
    uint32_t                            ui32NumBuffers; /*  Defines the number of
                                                        channels in the input
                                                        or output */
    BDSP_AF_P_BufferType                eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer;    /*  All circular buffer parameters
                                                        wrt to each input buffer comes */

}BDSP_AF_P_sIO_GENERIC_BUFFER;

/* GATE OPEN CONFIGURATION */

/***************************************************************************
Summary:
    The gate open Configuration of a port

Description:

    This contains the Start write pointers of all port channels and the
    Dram address of IO Buffer configuration


See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sFMM_GATE_OPEN_CONFIG
{
    uint32_t                    ui32IndepDelay;

    dramaddr_t                  uin32RingBufStartWrPointAddr[BDSP_AF_P_MAX_CHANNELS];

    dramaddr_t                  uin32DramIoConfigAddr;

    /*FMM buffer content : Compressed / PCM */
    BDSP_AF_P_FmmContentType    eFMMContentType;

    /*FMM buffer Sinking rate */
    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;

    uint32_t                    ui32BufferDepthThreshold; /* Threshold value to determine the depth of the FMM buffer  */

} BDSP_AF_P_sFMM_GATE_OPEN_CONFIG;

/***************************************************************************
Summary:
    The gate open Configuration of a task

Description:

    This contains the Start write pointers of all port channels and the
    Dram address of IO Buffer configuration of a task.


See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG
{
    uint32_t    ui32NumPorts;

    uint32_t    ui32MaxIndepDelay ;

    BDSP_AF_P_sFMM_GATE_OPEN_CONFIG  sFmmGateOpenConfig[BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK];

}BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG;


/***************************************************************************
Summary:
    The structure that contains all the o/p port configurations.

Description:
    This is the address where the o/p buffer configuration for the port is
    present.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sOP_PORT_CFG
{
    uint32_t                    ui32OpPortCfgAddr;
    uint32_t                    ui32FmmAdaptRateCntrlRegAddr;
    uint32_t                    ui32IndependentDelaySign;
    uint32_t                    ui32IndependentDelayValue;
}BDSP_AF_P_sOP_PORT_CFG;

/***************************************************************************
Summary:
    The structure that contains the destination configuration for a Dummy port
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_DUMMY_PORT_CLK_CFG
{
    uint32_t                    ui32DummyPortEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32DummyPortClkMacroRegAddr;

}BDSP_AF_P_sFMM_DEST_DUMMY_PORT_CLK_CFG;


/***************************************************************************
Summary:
    The structure that contains the destination configuration for a Cap port
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG
{
    uint32_t                    ui32CapPortEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32AudioCapPortClkMacroRegAddr;

}BDSP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG;


typedef struct BDSP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG
{

    uint32_t                    ui32SpdifEnabled;               /* Valid True and its values*/

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;

    uint32_t                    ui32AudioSpdifClkMacroRegAddr;      /* If index is 0-->AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0 */
                                                                /* If index is 1-->AUD_FMM_OP_CTRL_MCLK_CFG_FLEX */
                                                                /*..BUT doesnt exist for 7405 */

    uint32_t                    ui32SpdifCbitCtrlReg;           /* If  index 0 -->AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0 */
                                                                /* If  index 1 -->AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1 */

    uint32_t                    ui32CbitBasePingAddr;           /* If index 0--> AUD_FMM_MS_CTRL_FW_CBITS0 */
                                                                /* If index 1--> AUD_FMM_MS_CTRL_FW_CBITS24 */

    uint32_t                    ui32CbitBasePongAddr;           /* If index 0--> AUD_FMM_MS_CTRL_FW_CBITS12 */
                                                                /* If index 1--> AUD_FMM_MS_CTRL_FW_CBITS36 */

    uint32_t                    ui32PingPongMask;               /* If Index 0-->AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG0_MASK */
                                                                /* If Index 1-->AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK */

    uint32_t                    ui32AudFmmMsCtrlHwSpdifCfg;

    dramaddr_t                  ui32SpdifDramConfigPtr;         /* Address to BDSP_AF_P_sSPDIF_USER_CFG Structure in Dram */

    uint32_t                    ui32PcmOnSpdif; /* 0=  PCM  and 1= Compressed*/

}BDSP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG;

typedef struct BDSP_AF_P_sFMM_ESR_REGISTER
{
    uint32_t                    ui32AudFmmMsEsrStatusRegAddr;       /**From RDB: AUD_FMM_MS_ESR_STATUS **/
    uint32_t                    ui32AudFmmMsEsrStatusClearRegAddr;  /**From RDB: AUD_FMM_MS_ESR_STATUS_CLEAR*/
}BDSP_AF_P_sFMM_ESR_REGISTER;


typedef struct BDSP_AF_P_sFMM_DEST_SPDIF_CFG
{
    BDSP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG          sFmmDestSpdifClkCbitCfg[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];
    BDSP_AF_P_sFMM_ESR_REGISTER                     sFmmEsrRegister;

}BDSP_AF_P_sFMM_DEST_SPDIF_CFG;


/***************************************************************************
Summary:
    The structure that contains the destination configuration for a DAC o/p
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_DAC_CFG
{
    uint32_t                    ui32DacEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32HifidacRmSampleIncRegAddr;
    uint32_t                    ui32HifidacRmPhaseIncRegAddr;
    uint32_t                    ui32HifidacRmRateRatioRegAddr;

    uint32_t                    ui32HifidacRateManagerCfgAddr;  /* HIFIDAC CONTROL REGISTER  */

}BDSP_AF_P_sFMM_DEST_DAC_CFG;


/***************************************************************************
Summary:
    The structure that contains the destination configuration for a I2S o/p
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_I2S_CLK_CFG
{
    uint32_t                    ui32I2SEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */
    uint32_t                    ui32AudioI2SClkMacroRegAddr;/* If Index 0 -> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO_0   */
                                                            /* If Index 1 -> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO_1   */
                                                            /* If Index 2 --> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_0   */
                                                            /* If Index 3 --> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_1   */
}BDSP_AF_P_sFMM_DEST_I2S_CLK_CFG;

/***************************************************************************
Summary:
    The structure that contains the destination configuration for o/p PLL.

See Also:
    None.
****************************************************************************/
/*

There are 2 plls in system . But can be 4...
In a task only one PLL
*/

typedef struct BDSP_AF_P_sFMM_DEST_PLL_CFG
{                                                            /* Valid True and its values*/
    uint32_t                        ui32PllEnabled;
    uint32_t                        ui32PllIndex;           /* PLL Index . This is for individual PLL configuration*/
                                                            /* Used in I2S/SPDIF configuration */

    uint32_t                        ui32AudioPllMacroRegAddr; /*If Index 0 -->AUD_FMM_PLL0_MACRO */
                                                              /*If Index 1 -->AUD_FMM_PLL1_MACRO */

    /*
     * For Raaga ui32AudioPllMacroRegAddr is BCHP_VCXO_MISC_AUDIO_MODE_CTRL in both cases.
     */
    uint32_t                        ui32AudioPllCtrl0Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_0*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_0*/
    uint32_t                        ui32AudioPllCtrl1Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl2Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl3Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl4Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl5Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl6Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrlOut0Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH0*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH0*/
    uint32_t                        ui32AudioPllCtrlOut1Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH1*/
    uint32_t                        ui32AudioPllCtrlOut2Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut3Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut4Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut5Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    BDSP_AF_P_I2sMclkRate           eFmmI2sMclRate;
}BDSP_AF_P_sFMM_DEST_PLL_CFG;


/* PORT CONFIGURATION */
/***************************************************************************
Summary:
    The structure that contains the destination configuration for a NCO
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_NCO_CFG
{
    uint32_t                    ui32NcoSampleIncRegAddr;
    uint32_t                    ui32NcoPhaseIncRegAddr;
    uint32_t                    ui32NcoRateRatioRegAddr;

}BDSP_AF_P_sFMM_DEST_NCO_CFG;


/***************************************************************************
Summary:
    The structure that contains the FMM destination configuration registers.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_CFG
{

    BDSP_AF_P_PllNcoSelect                  eFmmPllNcoSelect;
    BDSP_AF_P_sFMM_DEST_NCO_CFG             sFmmDestNcoCfg;
    BDSP_AF_P_sFMM_DEST_PLL_CFG             sFmmDestPllCfg;
    BDSP_AF_P_sFMM_DEST_I2S_CLK_CFG         sFmmDestI2SClkCfg[BDSP_AF_P_MAX_NUM_I2S_PORTS];
    BDSP_AF_P_sFMM_DEST_DAC_CFG             sFmmDestDacCfg[BDSP_AF_P_MAX_NUM_DAC_PORTS];
    BDSP_AF_P_sFMM_DEST_SPDIF_CFG           sFmmDestSpdifCfg;
    /*uint32_t                              ui32HbrEnable;*/
    uint32_t                                ui32HwCbitsEnable;

    BDSP_AF_P_FmmDstFsRate                  eHdmiFsRate; /* Sinking Rate of HDMI.
                                                            This information is used by Data Sync to raise Fs interrupt.
                                                          */

    /* Capture port Cfg */
#if defined BDSP_CAP_PORT_SUPPORT
    BDSP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG    sFmmCapPortClkCfg[BDSP_AF_P_MAX_NUM_CAP_PORTS];
#endif
#if defined BDSP_DUMMY_PORT_SUPPORT
    BDSP_AF_P_sFMM_DEST_DUMMY_PORT_CLK_CFG  sFmmDummyPortClkCfg[BDSP_AF_P_MAX_NUM_DUMMY_PORTS];
#endif
}BDSP_AF_P_sFMM_DEST_CFG;


/***************************************************************************
Summary:
    The structure that contains the destination configuration for a SPDIF o/p
    port.

See Also:
    None.
****************************************************************************/

/* Host info for SPDIF parameters from Scheduler */
typedef struct BDSP_AF_P_sSPDIF_USER_CFG
{
    int32_t i32ProfessionalModeFlag;
    int32_t i32SoftwareCopyrightAsserted;
    int32_t i32CategoryCode;
    int32_t i32ClockAccuracy;
    int32_t i32bSeparateLRChanNum;
    int32_t i32CgmsA;
    /* Range values for this field.

        0       CopyFreely      Unlimited copies may be made of the content.
        1       CopyNoMore      One generation of copies has already been made; no further copying is allowed.
        2       CopyOnce        One generation of copies may be made
        3       CopyNever       No copies may be made of the content.

        default =0
    */
} BDSP_AF_P_sSPDIF_USER_CFG;


/***************************************************************************
Summary:
    The structure that contains the PPM valid flag and the PPM cfg address.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sPPM_CFG
{
    BDSP_AF_P_EnableDisable ePPMChannel;
    dramaddr_t              ui32PPMCfgAddr;

}BDSP_AF_P_sPPM_CFG;

/***************************************************************************
Summary:

    The structure that contains all hardware cfg RDBs used by FW .

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sFW_HW_CFG
{
    /*PPM Configuration */
    BDSP_AF_P_sPPM_CFG      sPpmCfg[BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS];

}BDSP_AF_P_sFW_HW_CFG;

typedef struct BDSP_AF_P_sOpSamplingFreq
{
    uint32_t ui32OpSamplingFrequency[BDSP_AF_P_SampFreq_eMax];

}BDSP_AF_P_sOpSamplingFreq;


/***************************************************************************
Summary:
    The structure contains all information regarding soft increment of STC

Description:
    This structure contains configuration info of soft STC increment.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sStcTrigConfig
{
    /* If soft triggering is required. Default = BDSP_AF_P_eDisable */
    BDSP_AF_P_EnableDisable eStcTrigRequired;
    /* High and Low part of registers to tell the amount of STC increment. */
    uint32_t                ui32StcIncHiAddr;
    uint32_t                ui32StcIncLowAddr;
    /* Address of register to send trigger for incrementing STC */
    dramaddr_t                ui32StcIncTrigAddr;
/* Trigger bit in the above register. Bit count [031]*/
    uint32_t                ui32TriggerBit;

}BDSP_AF_P_sStcTrigConfig;

/***************************************************************************
Summary:
    The structure contains all the global configurations of a task.

Description:
    All the configuration which are common to the entire task are placed
    in the global task configuration.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sGLOBAL_TASK_CONFIG
{
    uint32_t                    ui32NumberOfNodesInTask;
    uint32_t                    ui32NumberOfZeroFillSamples;
    uint32_t                    ui32StartNodeIndexOfCoreAudioAlgorithm; /*  This node id defines
                                                                            the starting index of the
                                                                            core audio processing stages.
                                                                            For a decoder task, which has
                                                                            frame sync and TSM codes in
                                                                            separate executable, Node 0
                                                                            and Node 1 will be reserved
                                                                            for frame sync and TSM. The
                                                                            field "StartNodeIndexOfCoreAudioAlgorithm"
                                                                            shall be set to 2 */



    dramaddr_t                    ui32FmmDestCfgAddr;
                                                                    /*  FMM destination configuration information. This structure is required
                                                                        one per Task. How to associate an o/p port with the correct sampling
                                                                        frequency to be programmed in the case of SRC? */

    dramaddr_t                    ui32FmmGateOpenConfigAddr;

    dramaddr_t                    ui32FwOpSamplingFreqMapLutAddr;     /*  This is the FW Input-Output sampling frequency mapping LUT*/

    uint32_t                    ui32NumOpPorts;                     /*  This tells the number of output ports */

    BDSP_AF_P_sOP_PORT_CFG      sOpPortCfg[BDSP_AF_P_MAX_OP_PORTS]; /*  This will have the DRAM address of Output port's IO_buffer
                                                                        Structure **/
    BDSP_AF_P_sDRAM_BUFFER      sDramScratchBuffer;                 /*  The scratch buffer is being moved to global task config */

    dramaddr_t                    ui32TaskFwHwCfgAddr;                /*  This address contains the structure of BDSP_AF_P_sFW_HW_CFG*/

    BDSP_AF_P_TimeBaseType      eTimeBaseType;                      /*  Time base type for a task 45Khz or 27 Mhz (Direct TV) */

    dramaddr_t                    ui32StcTrigConfigAddr;              /* DRAM address where STC trigger configuratio is passed */

    /*  These fields are reserved for future usage */
    uint32_t                    ui32Reserved0;
    uint32_t                    ui32Reserved1;
    uint32_t                    ui32Reserved2;
    uint32_t                    ui32Reserved3;
}BDSP_AF_P_sGLOBAL_TASK_CONFIG;

/***************************************************************************
Summary:
    Enum data type describing Scheduling group of a task

Description:
        default/InterruptModeAxVideoEncode
See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_eSchedulingGroup
{
    BDSP_AF_P_eSchedulingGroup_Default = 0, /* default scheduling group, for normal audio and video tasks */
    BDSP_AF_P_eSchedulingGroup_IntrModeAxVidEncode,/* Interrupt Mode Ax video encode */
    BDSP_AF_P_eSchedulingGroup_Max,
    BDSP_AF_P_eSchedulingGroup_Invalid = 0x7FFFFFFF

}BDSP_AF_P_eSchedulingGroup;
#endif /*BDSP_COMMON_FW_H_*/
