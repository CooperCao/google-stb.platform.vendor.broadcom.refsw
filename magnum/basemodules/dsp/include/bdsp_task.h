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


#ifndef BDSP_TASK_H_
#define BDSP_TASK_H_


#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_context.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"

/* Kept as 2 keeping in mind the ping-pong type of arrangement for video encoder */
#define BDSP_MAX_EXT_INTERRUPT_PER_TASK             2
#define BDSP_MAX_METADATA_REGISTERS_PER_TASK        2

/** \brief Maximum number of output capture port configurations that can be enabled per task */
#define BDSP_MAX_OUTPUT_CAPTURES_PER_TASK           4
/** \brief The capture buffer size has to be 1.5 MB to hold samples worth 2 seconds at 192 kHz */
#define BDSP_CAPTURE_BUFFER_SIZE                    (uint32_t)1536*1024

/*RAVE and FMM inputs and Outputs*/
#define BDSP_MAX_INPUTS_PER_TASK        2
#define BDSP_MAX_OUTPUTS_PER_TASK       8

#define BDSP_MAX_DEPENDENT_TASK         4
#define BDSP_MAX_AVL_CHANNLES           6

/*Maximum frame size for an algorithm. PI uses this for allocating FMM ring buffer */
#ifdef BDSP_FLAC_SUPPORT
#define BDSP_AF_P_MAX_THRESHOLD               	((uint32_t)(4*4608))  /*This is based on FLAC case where maximum samples of 4608 and if we attach SRC it get multiplied by 4*/
#else
#define BDSP_AF_P_MAX_THRESHOLD               	((uint32_t)(8192))  /*This is based on DTSHD HBR case */
#endif
#define BDSP_AF_P_MAX_SAMPLING_RATE             ((uint32_t)48)      /* 48Khz (Max Sampling Frequency /1000) (to make in line of ms) */
#define BDSP_AF_P_MAX_INDEPENDENT_DELAY         ((uint32_t)500)     /* Max independent delay in ms */
#define BDSP_AF_P_MAX_AUD_OFFSET 			    ((uint32_t)128)     /* in ms */
#define BDSP_AF_P_MAX_BLOCKING_TIME              BDSP_AF_P_MAX_AUD_OFFSET   /* AUD_OFFSET has to be >= to the worst case blocking time */
#define BDSP_AF_P_SAMPLE_PADDING                ((uint32_t)(1024))  /* Padding */
#define BDSP_AF_P_BLOCKING_TIME                 ((uint32_t)(84))    /* In msec */

#define BDSP_AF_P_MAT_BUF_SIZE                  ((uint32_t)(1536 * 1024 )) /*  In Bytes: Based on experiments done on MLP worst case streams */
#define BDSP_AF_P_MAT_SAMPRATE_kHz              ((uint32_t)768)
#define BDSP_AF_P_INDEPENDENT_MAT_THRESHOLD     ((BDSP_AF_P_MAT_BUF_SIZE >> 2 ) - (BDSP_AF_P_BLOCKING_TIME*BDSP_AF_P_MAT_SAMPRATE_kHz)) /*((uint32_t)(328704))  ((base ptr -end ptr)/4 - (84*768))*/

#define BDSP_AF_P_STANDARD_BUFFER_THRESHOLD            BDSP_AF_P_MAX_THRESHOLD
#define BDSP_AF_P_COMPRESSED4X_BUFFER_THRESHOLD        ((uint32_t)(1536*6*4))  /*Based on MS12 DDP Encoder where 6 blocks of 1536 samples are accumulated and since its 4X buffer we multiply by 4*/
#define BDSP_AF_P_COMPRESSED16X_BUFFER_THRESHOLD       BDSP_AF_P_MAX_THRESHOLD
#define BDSP_AF_P_COMPRESSED16X_MLP_BUFFER_THRESHOLD   BDSP_AF_P_INDEPENDENT_MAT_THRESHOLD
#define BDSP_AF_P_MAX_16X_BUF_SIZE                     BDSP_AF_P_MAT_BUF_SIZE /* MAT and DTSMA are the 16X compressed contents. MLP is the worst case now */

#define BDSP_AF_COMPUTE_RBUF_SIZE(delay, maxRate) \
    (BDSP_AF_P_MAX_BLOCKING_TIME * (maxRate) \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING \
    + (delay) * (maxRate)) \
    * 4                     /* 4 to make in bytes */

#define BDSP_AF_P_NON_DELAY_RBUF_SIZE   \
    (BDSP_AF_P_MAX_BLOCKING_TIME * BDSP_AF_P_MAX_SAMPLING_RATE \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING) \
    * 4                     /* 4 to make in bytes */

#define BDSP_AF_P_DELAY_RBUF_SIZE   \
    (BDSP_AF_P_MAX_BLOCKING_TIME * BDSP_AF_P_MAX_SAMPLING_RATE \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING  \
    + BDSP_AF_P_MAX_INDEPENDENT_DELAY * BDSP_AF_P_MAX_SAMPLING_RATE) \
    * 4                     /* 4 to make in bytes */


/***************************************************************************
Summary:
Task Scheduling mode
***************************************************************************/
typedef enum BDSP_TaskSchedulingMode
{
   BDSP_TaskSchedulingMode_eStandalone,    /* Default.  Standalone task */
   BDSP_TaskSchedulingMode_eMaster,            /* Master task in a master/slave relationship */
   BDSP_TaskSchedulingMode_eSlave,                /* Slave task in a master/slave relationship */
   BDSP_TaskSchedulingMode_eMax
} BDSP_TaskSchedulingMode;


/***************************************************************************
Summary:
Task Realtime mode
***************************************************************************/
typedef enum BDSP_TaskRealtimeMode
{
    BDSP_TaskRealtimeMode_eRealTime,                /* Realtime */
    BDSP_TaskRealtimeMode_eNonRealTime,                /* Non-Realtime */
    BDSP_TaskRealtimeMode_eSoftRealTime,                /* Soft-Realtime */
    BDSP_TaskRealtimeMode_eOnDemand,                /* On Demand Task */
    BDSP_TaskRealtimeMode_eMax
} BDSP_TaskRealtimeMode;


/***************************************************************************
Summary:
Audio Task Low delay  mode
***************************************************************************/
typedef enum BDSP_AudioTaskDelayMode
{
    BDSP_AudioTaskDelayMode_eDefault,  /* Fixed path delay of 128ms is used for all audio processing */
    BDSP_AudioTaskDelayMode_WD_eLow,   /* Fixed low path delay for the Wifi Display usage cases */
    BDSP_AudioTaskDelayMode_WD_eLowest,/* Variable lowest path delay depending on decode algorithm supported for Wifi Display. */
    BDSP_AudioTaskDelayMode_eMax
}BDSP_AudioTaskDelayMode;

typedef struct BDSP_TaskGateOpenSettings
{
    uint32_t    ui32NumPorts;

    uint32_t    ui32MaxIndepDelay ;

    void        *psFmmGateOpenConfig;

    BDSP_MMA_Memory PortAddr;
}BDSP_TaskGateOpenSettings;

/***************************************************************************
Summary:
    The structure contains all information regarding tasks which are depended on
    current task to open their gate

Description:
    This structure contains task handles of tasks for which gate open needs to be performed

See Also:
    None.
****************************************************************************/
typedef struct BDSP_DependentTaskInfo
{
    unsigned numTasks;
    BDSP_TaskHandle DependentTask[BDSP_MAX_DEPENDENT_TASK];
}BDSP_DependentTaskInfo;

/***************************************************************************
Summary:
    The structure contains all information regarding soft increment of STC

Description:
    This structure contains configuration info of soft STC increment.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_STCIncrementConfig
{
    /* If soft triggering is required. Default = BDSP_AF_P_eDisable */
    bool                        enableStcTrigger;
    /* High and Low part of registers to tell the amount of STC increment. */
    uint32_t                stcIncHiAddr;
    uint32_t                stcIncLowAddr;
    /* Address of register to send trigger for incrementing STC */
    uint32_t                stcIncTrigAddr;
    /* Trigger bit in the above register. Bit count [031]*/
    uint32_t                triggerBit;

}BDSP_STCIncrementConfig;

/***************************************************************************
Summary:
    The structure contains all information regarding soft external interrupts to DSP

Description:
    This structure contains configuration info of soft external interrupts to DSP.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_ExtInterruptConfig
{
    /* If the dsp task will be interrupted by external client */
    bool                    enableInterrupts;
    uint32_t                numInterrupts;
    /* only numInterrupts of following struct will be valid */
    struct
    {
        /* ESR_SI register address. Full 32bit address */
        uint32_t                interruptRegAddr;
        /* Trigger bit in the above register. Bit count [0...31]*/
        uint32_t                interruptBit;
    }interruptInfo[BDSP_MAX_EXT_INTERRUPT_PER_TASK];

}BDSP_ExtInterruptConfig;


/***************************************************************************
Summary:
    The structure contains all the global configurations of a task that comes
    as input from PI to CIT.

Description:
    All the input global configuration parameters that comes directly from PI
    to CIT are included in this structure. This structure will be passed as
    input argument to CIT generation function for Video.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_sVDecoderIPBuffCfg
{
    /* Max Picture size supported */
    unsigned                MaxFrameHeight;
    unsigned                MaxFrameWidth;

    /* Decode and display buffer handles */
    sFrameBuffHandle        sDisplayFrameBuffParams;
    sFrameBuffHandle        sReferenceBuffParams;

    /* These structures will give DRAM start addresses of the UPBs
       Note: The virtual address of the Display buffer (luma and chroma) will be populated by CIT
     */
    BDSP_AF_P_sDRAM_BUFFER  sUPBs[BDSP_FWMAX_VIDEO_BUFF_AVAIL];

}BDSP_sVDecoderIPBuffCfg;
/***************************************************************************
Summary:
   The structure contains addressing range for diffrent SCB's

Description:

See Also:
   None.
****************************************************************************/
typedef struct BDSP_sVEncoderIPConfig
{
    BDSP_VF_P_sEncodeParams sEncoderParams;

    /* Reference Buffer Params */
    /* Max Picture size supported */
    unsigned                MaxFrameHeight;
    unsigned                MaxFrameWidth;

     /* Reference buff 2 buffers in case of B pictures */
    sFrameBuffHandle        sReferenceBuffParams;

    /* These structures will give DRAM start addresses of the PPBs  (picture parameter buffers)
       Note: The virtual address of the Display buffer (luma and chroma) will be populated by CIT
     */
    BDSP_AF_P_sDRAM_BUFFER  sPPBs[BDSP_FWMAX_VIDEO_BUFF_AVAIL];

}BDSP_sVEncoderIPConfig;

/***************************************************************************
Summary:
Create A DSP Task
***************************************************************************/
typedef struct BDSP_TaskCreateSettings
{
    unsigned dspIndex;              /* Which DSP to create this task on */
    bool masterTask;             /* If this task can be a master task, set to true.  Else set to false (default). */
    unsigned numSrc;
    unsigned numDst;
} BDSP_TaskCreateSettings;

typedef struct BDSP_TaskStartSettings
{
    BDSP_StageHandle primaryStage;

    BDSP_TaskSchedulingMode schedulingMode;
    BDSP_TaskRealtimeMode realtimeMode;
    BDSP_TaskHandle masterTask;
    bool gateOpenReqd; /* Parameter to explicity disable the gateopen for the FMM ports attached and the responsibility is delegated to some other task*/
    BDSP_DependentTaskInfo DependentTaskInfo; /* Parameter to specify number of tasks for which gate open has to happen*/
    BDSP_AudioTaskDelayMode     audioTaskDelayMode;
    BDSP_sVDecoderIPBuffCfg     *psVDecoderIPBuffCfg;
    BDSP_sVEncoderIPConfig      *psVEncoderIPConfig;
    BDSP_AF_P_TimeBaseType  timeBaseType;       /* Time base type for a task 45Khz or 27 Mhz (Direct TV)*/
    bool    ppmCorrection;  /*Enable/Disable 2ms tight lypsinc*/
    BDSP_STCIncrementConfig    stcIncrementConfig;  /* Soft increment of STC*/
    BDSP_ExtInterruptConfig     extInterruptConfig; /* External interrupt to DSP configuration */
    bool    openGateAtStart; /* Enable mute frame rendering at the start of audio decode */
    BDSP_AF_P_sOpSamplingFreq *pSampleRateMap; /* Pointer to the input -> output sample rate mapping table */
    unsigned maxIndependentDelay; /* Maximum independent delay value in ms (default=500).  Used only for audio tasks. */
    bool    eZeroPhaseCorrEnable; /*Flag to enable/disable zero phase output-default is true!*/
	BDSP_AF_P_BurstFillType        eFMMPauseBurstType;
    BDSP_AF_P_SpdifPauseWidth       eSpdifPauseWidth;
} BDSP_TaskStartSettings;

/***************************************************************************
Summary:
Get Default DSP Task Settings
***************************************************************************/
void BDSP_Task_GetDefaultCreateSettings(
    BDSP_ContextHandle context,
    BDSP_TaskCreateSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create a DSP task
***************************************************************************/
BERR_Code BDSP_Task_Create(
    BDSP_ContextHandle context,
    const BDSP_TaskCreateSettings *pSettings,
    BDSP_TaskHandle *pTask    /* [out] */
    );

/***************************************************************************
Summary:
Destroy a DSP task
***************************************************************************/
void BDSP_Task_Destroy(
    BDSP_TaskHandle task
    );

/***************************************************************************
Summary:
Start a DSP task
***************************************************************************/
BERR_Code BDSP_Task_Start(
    BDSP_TaskHandle task,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Stop a DSP task
***************************************************************************/
BERR_Code BDSP_Task_Stop(
    BDSP_TaskHandle task
    );

/***************************************************************************
Summary:
Get default task start settings
***************************************************************************/
void BDSP_Task_GetDefaultStartSettings(
    BDSP_TaskHandle task,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Get the Gate Open Param for the dependent Task
***************************************************************************/
BERR_Code BDSP_Task_RetrieveGateOpenSettings(
    BDSP_TaskHandle task,
    BDSP_TaskGateOpenSettings *pSettings
    );
#endif
