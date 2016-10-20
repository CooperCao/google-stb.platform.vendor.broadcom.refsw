/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#ifndef BDSP_H_
#define BDSP_H_
#include "bdsp_types.h"
#include "bdsp_context.h"
#include "bdsp_task.h"
#include "bdsp_stage.h"
#include "bdsp_audio_task.h"
#include "bdsp_video_task.h"
#include "bdsp_video_encode_task.h"
#include "bdsp_scm_task.h"
/*------------------------- ERROR CODES---------------------------------------*/
#define BDSP_ERR_DEVICE_UNINTIALIZED        BERR_MAKE_CODE(BERR_DSP_ID, 1)
#define BDSP_ERR_BAD_DEVICE_STATE           BERR_MAKE_CODE(BERR_DSP_ID, 2)
#define BDSP_ERR_RESOURCE_EXHAUSTED         BERR_MAKE_CODE(BERR_DSP_ID, 3)
#define BDSP_ERR_CHANNEL_ALREADY_OPENED     BERR_MAKE_CODE(BERR_DSP_ID, 4)
#define BDSP_ERR_CHANNEL_ALREADY_STARTED    BERR_MAKE_CODE(BERR_DSP_ID, 5)
#define BDSP_ERR_BUFFER_FULL                BERR_MAKE_CODE(BERR_DSP_ID, 6)
#define BDSP_ERR_BUFFER_EMPTY               BERR_MAKE_CODE(BERR_DSP_ID, 7)
#define BDSP_ERR_BUFFER_INVALID             BERR_MAKE_CODE(BERR_DSP_ID, 8)
#define BDSP_ERR_INVALID_TASK               BERR_MAKE_CODE(BERR_DSP_ID, 9)
#define BDSP_ERR_DOWNLOAD_FAILED            BERR_MAKE_CODE(BERR_DSP_ID, 10)


#define BDSP_RAAGA_RBUS_RDB_ADDRESS

#define BDSP_RAAGA_REGSET_BASE_MASK 0xFFFFF
#define BDSP_RAAGA_PMEM_BASE_MASK 0x80000000

#ifdef BDSP_RAAGA_RBUS_RDB_ADDRESS
    #define BDSP_RAAGA_REGSET_ADDR_FOR_DSP(offset) offset?( BDSP_RAAGA_PMEM_BASE_MASK + ((offset) & BDSP_RAAGA_REGSET_BASE_MASK)) : (offset)
#else
    #define BDSP_RAAGA_REGSET_ADDR_FOR_DSP(offset) ((offset) + BCHP_PHYSICAL_OFFSET)
#endif

#ifdef BDSP_RAAGA_RBUS_RDB_ADDRESS
    #define BDSP_ADDR_FOR_HOSTCPU_FROM_RAAGA_REGSET(Addr) (((Addr) - BDSP_RAAGA_PMEM_BASE_MASK)+ ((BCHP_RAAGA_DSP_RGR_REG_START) & (~BDSP_RAAGA_REGSET_BASE_MASK)))
#else
    #define BDSP_ADDR_FOR_HOSTCPU_FROM_RAAGA_REGSET(Addr) ((Addr) - BCHP_PHYSICAL_OFFSET )
#endif

#define BDSP_RAAGA_REGSET_PHY_ADDR_FOR_DSP(offset) ((offset) + BCHP_PHYSICAL_OFFSET)
#define BDSP_RAAGA_REGSET_OFFSET_ADDR_FOR_HOST(offset) ((offset) - BCHP_PHYSICAL_OFFSET)


/*=************************ Module Overview ********************************
BDSP is a magnum basemodule interface to a DSP.  The DSP interface can be
used by one or more porting interface (PI) modules in order to access
services available on the DSP.

In order to use a DSP device, you must open the DSP device handle that
you wish to use.  For example, on a Raaga-based system you need to call
BDSP_Raaga_GetDefaultSettings() and BDSP_Raaga_Open().
***************************************************************************/

/***************************************************************************
Summary:
Close a DSP device
***************************************************************************/
void BDSP_Close(
    BDSP_Handle handle      /* Handle to be closed */
    );

/***************************************************************************
Summary:
BDSP Version Settings
***************************************************************************/
typedef struct BDSP_Version
{
    uint32_t majorVersion;                       /* Major Version of BDSP Release   */
    uint32_t minorVersion;                       /* Minor Version of BDSP Release   */
    uint32_t branchVersion;                      /* Branch Version of BDSP Release  */
    uint32_t branchSubVersion;                   /* Branch SubVersion of BDSP Release */
} BDSP_Version;


/***************************************************************************
Summary:
DSP Status
***************************************************************************/
typedef struct BDSP_Status
{
    unsigned numDsp;                    /* Number of DSPs available, typically one on DTV/STB systems */
    unsigned numWatchdogEvents;         /* Number of watchdog events since the DSP handle was opened */
    BDSP_Version firmwareVersion;       /* Version information */
}BDSP_Status;

/***************************************************************************
Summary:
Standby settings
***************************************************************************/
typedef struct BDSP_StandbySettings
{
    bool dummy; /* placeholder to avoid compiler warning */
}BDSP_StandbySettings;

/***************************************************************************
Summary:
Get DSP Status
***************************************************************************/
void BDSP_GetStatus(
    BDSP_Handle handle,
    BDSP_Status *pStatus             /* [out] Current Status */
    );

/***************************************************************************
Summary:
    Enter standby mode with  DSP.

Description:
    This function puts the  DSP into standby mode,
    if supported. All DSP tasks must be in a stopped state
    in order to successfully enter standby mode.
    If standby mode is not supported, calling this function has no effect.

    When in standby mode, the device clocks are turned off,
    resulting in a minimal power state.

    No BDSP_* calls should be made until standby mode exitted by calling
    BDSP_Resume().

Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

See Also:
    BDSP_Resume
***************************************************************************/
BERR_Code BDSP_Standby(
    BDSP_Handle             handle,         /* [in] DSP device handle */
    BDSP_StandbySettings    *pSettings  /* [in] standby settings */
);

/***************************************************************************
Summary:
    Exit standby mode with the  DSP

Description:
    This function exits the DSP from standby mode.
    After exitting standby mode, upper-level SW is free to call
    BDSP_* functions.

Returns:
    BERR_SUCCESS - If resume is successful, otherwise error

See Also:
    BDSP_Standby
***************************************************************************/

BERR_Code BDSP_Resume(
    BDSP_Handle             handle      /* [in] DSP device handle */
    );

/***************************************************************************
Summary:
    Algorithm info
***************************************************************************/
typedef struct BDSP_AlgorithmInfo
{
        bool supported;             /* true if this algorithm is supported, false otherwise */
        const char *pName;          /* Name of the algorithm */
        BDSP_AlgorithmType type;    /* Type of algorithm (e.g. audio decode, audio passthrough, etc.) */
        size_t settingsSize;        /* Size of settings structure in bytes */
        size_t statusSize;          /* Size of status structure in bytes */

} BDSP_AlgorithmInfo;

/***************************************************************************
Summary:
    Get Algorithm Information
***************************************************************************/
BERR_Code BDSP_GetAlgorithmInfo(
    BDSP_Handle handle,
    BDSP_Algorithm algorithm, /* [in] */
    BDSP_AlgorithmInfo *pInfo /* [out] */
    );

/***************************************************************************
Summary:
    Description of an external interrupt to the DSP
***************************************************************************/
typedef struct BDSP_ExternalInterruptInfo
{
    /* BCHP address. CHIP Offset to be added to get full 32 bit address */
    uint32_t address;
    /* Bit number starts with 0 as LSB and goes upwards */
    uint32_t bitNum;
} BDSP_ExternalInterruptInfo;

/***************************************************************************
Summary:
    Allocate an external interrupt.

Description:
    This function allocates one interrupt handle that can be used to interrupt DSP.

Returns:
    BERR_SUCCESS - If allocation is successful, otherwise error

See Also:
    BDSP_FreeExternalInterrupt
    BDSP_GetExternalInterruptInfo
***************************************************************************/
BERR_Code BDSP_AllocateExternalInterrupt(
    BDSP_Handle hDsp,
    uint32_t    dspIndex,
    BDSP_ExternalInterruptHandle *pInterruptHandle /* [out] */
    );

/***************************************************************************
Summary:
    Frees an allocated external interrupt.

Description:
    This function frees one interrupt handle that was already allocated.

Returns:
    BERR_SUCCESS if sucessful else error

See Also:
    BDSP_AllocateExternalInterrupt
    BDSP_GetExternalInterruptInfo
***************************************************************************/
BERR_Code BDSP_FreeExternalInterrupt(
    BDSP_ExternalInterruptHandle    hInterrupt
    );

/***************************************************************************
Summary:
    Retrieve external interrupt information.

Description:
    This function provides the interrupt register and the bit that can be used to interrupt DSP.

Returns:
    BERR_SUCCESS - If successful, otherwise error

See Also:
    BDSP_FreeExternalInterrupt
    BDSP_AllocateExternalInterrupt
***************************************************************************/
BERR_Code BDSP_GetExternalInterruptInfo(
    BDSP_ExternalInterruptHandle hInterrupt,
    BDSP_ExternalInterruptInfo **pInfo /* [out] */
    );

/***************************************************************************
Summary:
    Process audio capture data

Description:
    This function must be called in a background thread at a frequent
    interval (e.g. 10ms). for the data to be copied into the capture buffers.

Returns:
    BERR_SUCCESS - If audio capture is successful

See Also:
    BDSP_AudioCapture_GetBuffer
    BDSP_AudioCapture_ConsumeData
***************************************************************************/
BERR_Code BDSP_ProcessAudioCapture(
    BDSP_Handle hDsp
    );

#endif
