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


#ifndef BDSP_RAAGA_H_
#define BDSP_RAAGA_H_

#include "bchp.h"
#include "bint.h"
#include "bbox.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_task.h"

/*#define RAAGA_UART_ENABLE*/

#define BDSP_RAAGA_ADDRESS_ALIGN_CDB  12 /* CDB is aligned to 2^12 Bytes*/
#define BDSP_RAAGA_ADDRESS_ALIGN_ITB  12 /* ITB is aligned to 2^12 Bytes*/

#define BDSP_RAAGA_ADDRESS_ALIGN      4096
#define BDSP_RAAGA_SIZE_ALIGN(x)  	\
	{                               \
		unsigned power = 2;         \
		x = (((x+4095)/4096)*4096);\
		while (x >>= 1) power <<= 1;\
		x=power;                    \
	}

/***************************************************************************
Summary:
    This enumeration defines various debug features that can be enabled in the firmware.

***************************************************************************/
typedef enum BDSP_Raaga_DebugType
{
    BDSP_Raaga_DebugType_eDramMsg = 0,
    BDSP_Raaga_DebugType_eUart,
    BDSP_Raaga_DebugType_eCoreDump,
    BDSP_Raaga_DebugType_eTargetPrintf,
    BDSP_Raaga_DebugType_eLast,
    BDSP_Raaga_DebugType_eInvalid = 0x7FFFFFFF
} BDSP_Raaga_DebugType;

/***************************************************************************
Summary:
Raaga DSP Status
***************************************************************************/
typedef enum BDSP_Raaga_FwStatus
{
    BDSP_Raaga_FwStatus_eRunnning = 0,
    BDSP_Raaga_FwStatus_eCoreDumpInProgress,
    BDSP_Raaga_FwStatus_eCoreDumpComplete,
    BDSP_Raaga_FwStatus_eInvalid = 0xFF
} BDSP_Raaga_FwStatus;

/*********************************************************************
Summary:
    This structure contain elements that is returned by
        CalcThreshold_BlockTime_AudOffset API.

Description:

        ui32Threshold and ui32BlockTime goes to FW and
        ui32AudOffset goes to Application and TSM user cfg

See Also:
**********************************************************************/
typedef struct BDSP_CTB_Output
{
    uint32_t ui32Threshold;                                 /* Interms of samples */
    uint32_t ui32BlockTime;                                 /* Interms of Time (msec)  */
    uint32_t ui32AudOffset;                                 /* AudOffset in Time (msec) */
}BDSP_CTB_Output;

typedef struct BDSP_CTB_Input
{
    BDSP_AudioTaskDelayMode 		audioTaskDelayMode;
    BDSP_TaskRealtimeMode 			realtimeMode;
    BDSP_Audio_AudioInputSource     eAudioIpSourceType;           /* Capture port Type    */
}BDSP_CTB_Input;

/***************************************************************************
Summary:
BDSP codec capabilities
***************************************************************************/
typedef struct BDSP_CodecCapabilities
{
    struct {
        bool dapv2;         /* Allow eDapv2 processing if set else restrict it */
        bool ddEncode;      /* Allow DD compressed1x output if set else restrict it */
        bool ddpEncode51;   /* Allow DDP 5.1 compressed4x  if set else restrict it */
        bool ddpEncode71;   /* Allow DDP 7.1 compressed4x  if set else restrict it */
        bool pcm71;         /* Allow DDP 7.1 decoding if set else restrict it */
    } dolbyMs;
}BDSP_CodecCapabilities;

/***************************************************************************
Summary:
Raaga Debug Type Settings
***************************************************************************/
typedef struct BDSP_Raaga_DebugTypeSettings
{
    bool enabled;        /* If true, debug of this type is enabled. */
    uint32_t bufferSize; /* Size of debug buffer (in bytes) for a particular type of debug.
                                                        Only required if you want to override the default value. */
} BDSP_Raaga_DebugTypeSettings;

/***************************************************************************
Summary:
Raaga DSP Settings
***************************************************************************/
typedef struct BDSP_RaagaSettings
{
    bool authenticationEnabled; /* If authentication is enabled, all the firmware execs needs to be downloaded at open time and
                                   the DSP must be booted via a separate call to BDSP_Raaga_Boot() after HSM has been authenticated. */

	bool preloadImages;		   /* If true, all firmware images will be loaded on startup.  Default=false. */

    BDSP_Raaga_DebugTypeSettings debugSettings[BDSP_Raaga_DebugType_eLast]; /* Debug information for the different types of debug logs */

    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */

    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */
    unsigned maxAlgorithms[BDSP_AlgorithmType_eMax] ;

    unsigned NumDsp;    /*Number of DSP supported in the System, currently used for estimation of Memory*/

    unsigned numCorePerDsp;  /* Number of cores per DSP supported in the System, Used for estimation of Memory for newer chips */
    struct {
        BSTD_DeviceOffset baseAddress; /* Physical base address of the lowest physical address region for each MEMC.
            [0] is always 0 and it is assumed to always exist. For [1] and [2], an address of 0 means the MEMC is not populated.
            RAAGA is unable to access a discontiguous upper memory region, so its base address and size is not needed. */
        uint64_t size; /* Memc module size */
        unsigned stripeWidth;
    } memc[3]; /* for each MEMC */
} BDSP_RaagaSettings;

/***************************************************************************
Summary:
Firmware download status.
***************************************************************************/
typedef struct BDSP_Raaga_DownloadStatus
{
    void    *pBaseAddress;      /* Pointer to base of downloaded firmware executables */
    dramaddr_t physicalAddress;   /* Physical memory offset of downloaded firmware executables */
    size_t   length;            /* Length of executables in bytes */
} BDSP_Raaga_DownloadStatus;

/***************************************************************************
Summary:
Use case scenario provided by APE
***************************************************************************/
typedef struct BDSP_RaagaUsageOptions
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
} BDSP_RaagaUsageOptions;

/***************************************************************************
Summary:
Memory Requirement Status
***************************************************************************/
typedef struct BDSP_RaagaMemoryEstimate
{
    unsigned GeneralMemory; /* Number of bytes from the general system heap */
    unsigned FirmwareMemory; /* Number of bytes from the firmware heap */
} BDSP_RaagaMemoryEstimate;

void BDSP_Raaga_GetDefaultSettings(
	BDSP_RaagaSettings *pSettings
);
BERR_Code BDSP_Raaga_Open(
   BDSP_Handle *pDsp,                      /* [out] */
   BCHP_Handle chpHandle,
   BREG_Handle regHandle,
   BMMA_Heap_Handle memHandle,
   BINT_Handle intHandle,
   BTMR_Handle tmrHandle,
   BBOX_Handle boxHandle,
   const BDSP_RaagaSettings *pSettings
);
BERR_Code BDSP_Raaga_GetDebugBuffer(
    BDSP_Handle 			handle,
    BDSP_Raaga_DebugType 	debugType,
    uint32_t 				dspIndex,
    BDSP_MMA_Memory 	   *pBuffer,
    size_t 				   *pSize
);
BERR_Code BDSP_Raaga_ConsumeDebugData(
    BDSP_Handle 			handle,
    BDSP_Raaga_DebugType 	debugType,
    uint32_t 				dspIndex,
    size_t 					bytesConsumed
);
BDSP_Raaga_FwStatus BDSP_Raaga_GetCoreDumpStatus (
    BDSP_Handle handle,
    uint32_t dspIndex /* [in] Gives the DSP Id for which the core dump status is required */
);
BERR_Code BDSP_Raaga_GetDownloadStatus(
    BDSP_Handle handle,
    BDSP_Raaga_DownloadStatus *pStatus /* [out] */
);
BERR_Code BDSP_Raaga_GetAudioDelay_isrsafe(
    BDSP_CTB_Input   *pCtbInput,
    BDSP_StageHandle  hStage,
    BDSP_CTB_Output  *pCTBOutput
);
void BDSP_Raaga_GetCodecCapabilities(
	BDSP_CodecCapabilities *pSetting
);
BERR_Code BDSP_Raaga_Initialize(
	BDSP_Handle handle
);
BERR_Code BDSP_Raaga_GetDefaultAlgorithmSettings(
    BDSP_Algorithm algorithm,
    void *pSettingsBuffer,        /* [out] */
    size_t settingsBufferSize   /*[In]*/
);
BERR_Code BDSP_Raaga_GetMemoryEstimate(
    const BDSP_RaagaSettings     *pSettings,
    const BDSP_RaagaUsageOptions *pUsage,
    BBOX_Handle                   boxHandle,
    BDSP_RaagaMemoryEstimate     *pEstimate /*[out]*/
);
BERR_Code BDSP_AudioTask_GetDefaultDatasyncSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
);
BERR_Code BDSP_AudioTask_GetDefaultTsmSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
);
#endif /* BDSP_RAAGA_H_ */
