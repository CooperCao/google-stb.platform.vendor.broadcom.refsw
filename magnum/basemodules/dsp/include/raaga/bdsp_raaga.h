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
#include "breg_mem.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_types.h"
#include "bdsp.h"

/* #define RAAGA_UART_ENABLE */

#define BDSP_RAAGA_ADDRESS_ALIGN_CDB  8 /* CDB is aligned to 2^8 Bytes*/
#define BDSP_RAAGA_ADDRESS_ALIGN_ITB  7 /* ITB is aligned to 2^7 Bytes*/

#define BDSP_RAAGA_ADDRESS_ALIGN      32
#define BDSP_RAAGA_SIZE_ALIGN(x)
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

    bool preloadImages;         /* If true, all firmware images will be loaded on startup.  Default=false. */

    BDSP_Raaga_DebugTypeSettings debugSettings[BDSP_DebugType_eLast]; /* Debug information for the different types of debug logs */

    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */

    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */
    unsigned maxAlgorithms[BDSP_AlgorithmType_eMax] ;

    unsigned NumDsp;    /*Number of DSP supported in the System, currently used for estimation of Memory*/

    struct {
        BSTD_DeviceOffset baseAddress; /* Physical base address accessible heap */
        uint64_t size;
    } heap[BDSP_RAAGA_MAX_NUM_HEAPS]; /* heaps that DSP is expected to access */
    BCHP_MemoryLayout memoryLayout; /* all addressable memory per MEMC */
} BDSP_RaagaSettings;

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

/***************************************************************************
Summary:
Buffers used to capture Firmware output
***************************************************************************/
typedef struct BDSP_AF_P_CIRCULAR_BUFFER
{
    uint8_t     *pBasePtr;      /*  Circular buffer's base address */
    uint8_t     *pEndPtr;       /*  Circular buffer's End address */
    uint8_t     *pReadPtr;      /*  Circular buffer's read address */
    uint8_t     *pWritePtr;     /*  Circular buffer's write address */
    uint8_t     *pWrapPtr;      /*  Circular buffer's wrap address */
}BDSP_AF_P_CIRCULAR_BUFFER;
/***************************************************************************
Summary:
Get Default DSP Context Settings
***************************************************************************/
void BDSP_Raaga_GetDefaultSettings(
    BDSP_RaagaSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Open a Raaga DSP

Description:
Opens a Raaga DSP device.  BDSP_Close() should be called to close the handle.

See Also:
BDSP_Close
***************************************************************************/
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

/***************************************************************************
Summary:
Get a Estimate of the memory required by BDSP.

Description:
Calculate the total memory required by the BDSP and return the value to PI.

See Also:
None
***************************************************************************/
BERR_Code BDSP_Raaga_GetMemoryEstimate(
    const BDSP_RaagaSettings     *pSettings,
    const BDSP_RaagaUsageOptions *pUsage,
    BBOX_Handle                   boxHandle,
    BDSP_RaagaMemoryEstimate     *pEstimate /*[out]*/
);

/***************************************************************************
Summary:
Get default algorithm settings.
***************************************************************************/
BERR_Code BDSP_GetDefaultAlgorithmSettings(
        BDSP_Algorithm algorithm,
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
    );


/***************************************************************************
Summary:
Trigger DBG_service as part of on-chip debug support
***************************************************************************/
BERR_Code BDSP_Raaga_RunDebugService(
    BDSP_Handle hDsp,
    uint32_t dspIndex
);
#endif
