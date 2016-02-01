/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: InputCapture
*    Specific APIs related to Input Capture Control.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_INPUT_CAPTURE_H__
#define NEXUS_INPUT_CAPTURE_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Input Capture Handle
***************************************************************************/
typedef struct NEXUS_InputCapture *NEXUS_InputCaptureHandle;

/***************************************************************************
Summary:
Input Capture Timeout Handle
***************************************************************************/
typedef struct NEXUS_InputCaptureTimeout *NEXUS_InputCaptureTimeoutHandle;

/***************************************************************************
Summary:
Interrupt edge config
***************************************************************************/
typedef enum NEXUS_InputCaptureEdgeSelect
{
    NEXUS_InputCaptureEdgeSelect_eNegative,
    NEXUS_InputCaptureEdgeSelect_ePositive,
    NEXUS_InputCaptureEdgeSelect_eBoth,
    NEXUS_InputCaptureEdgeSelect_eMax
} NEXUS_InputCaptureEdgeSelect;

/***************************************************************************
Summary:
Input capture is supported in two distinct hardware cores: ICAP and BICAP.

BICAP adds several features including a hardware fifo and configurable timeouts.

However, both ICAP and BICAP are supported through the same Nexus interface. 
Both are have software buffering. Features that are only supported in BICAP 
or in certain versions of BICAP cores are noted.
***************************************************************************/
typedef enum NEXUS_InputCaptureType
{
    NEXUS_InputCaptureType_eIcap,
    NEXUS_InputCaptureType_eBicap,
    NEXUS_InputCaptureType_eMax
} NEXUS_InputCaptureType;

typedef enum NEXUS_InputCaptureCountMode
{
    NEXUS_InputCaptureCountMode_eSaturate,
    NEXUS_InputCaptureCountMode_eContinueToZero,
    NEXUS_InputCaptureCountMode_eFreeRunning,
    NEXUS_InputCaptureCountMode_eNoMaxCount,
    NEXUS_InputCaptureCountMode_eMax
} NEXUS_InputCaptureCountMode;

typedef enum NEXUS_InputCaptureClockSelect
{
    NEXUS_InputCaptureClockSelect_eBicapClock,   
    NEXUS_InputCaptureClockSelect_eTimeoutClock, /* also known as TOUT clock */
    NEXUS_InputCaptureClockSelect_eMax
} NEXUS_InputCaptureClockSelect;

typedef enum NEXUS_InputCaptureFilterClockSelect
{
    NEXUS_InputCaptureFilterClockSelect_eBicapClock,
    NEXUS_InputCaptureFilterClockSelect_eSystemClock,
    NEXUS_InputCaptureFilterClockSelect_eMax
} NEXUS_InputCaptureFilterClockSelect;

/***************************************************************************
Summary:
Input Capture settings 
***************************************************************************/
typedef struct NEXUS_InputCaptureSettings
{
    NEXUS_InputCaptureType type; /* Icap or Bicap */
    unsigned fifoSize; /* number of entries */

    NEXUS_InputCaptureEdgeSelect edgeSelect;
    NEXUS_CallbackDesc dataReady; /* called when a new entry is available with NEXUS_InputCapture_ReadData */

    /* filter settings */
    struct {
        bool enable;
        NEXUS_InputCaptureFilterClockSelect clockSelect;
        unsigned value; 
            /*
                for eSystemClock:
                    Programmable glitch rejecter based on system clock. Can be 
                    configured to ignore pulses shorter than (filter*512+2) system
                    clock cycles which is equal to (filter*4.72 + 0.018 us). Thus 
                    the filter gives minimum glitch rejectetion of 4.74 us and 
                    maximum glitch rejectetion of 606.72 us. This may reject any 
                    pulse in either direction.
                for eBicapClock:
                    Programmable glitch rejecter based on bicap clock. Can be 
                    configured to ignore pulses shorter than 2 to 130 BICAP clock 
                    cycles. This may reject any pulse in either direction.  
                    0: ignore pulse shorter than 2 bicap clock cycles. 
                    127: ignore pulse shorter than 130 bicap clock cycles.
            */
    } filter;

    struct {
        unsigned systemClockDivisor; /* Clock divisor for bicap clock.  Frequency = 108000000/(sysClkDiv+1).  sysClkDiv valid from 0 to 0x3fff */
        unsigned timeoutClockDivisor; /* Clock divisor for the timeout clock.  Frequency  = 108000000/((sysClkDiv+1)*(timeoutClkDiv+1)).  timeoutClkDiv valid from 0 to 0xff */
        NEXUS_InputCaptureCountMode countMode;
        bool invertSignalInput;
    } bicap;
} NEXUS_InputCaptureSettings;


/***************************************************************************
Summary:
Get default settings for an Input Capture.
***************************************************************************/
void NEXUS_InputCapture_GetDefaultSettings(
    NEXUS_InputCaptureSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open an Input Capture.
***************************************************************************/
NEXUS_InputCaptureHandle NEXUS_InputCapture_Open( /* attr{destructor=NEXUS_InputCapture_Close} */
    unsigned index, /* relative to NEXUS_InputCaptureSettings.type */
    const NEXUS_InputCaptureSettings *pSettings  /* attr{null_allowed=y} May be passed as NULL for defaults */
    );

/***************************************************************************
Summary:
Close an Input Capture.
***************************************************************************/
void NEXUS_InputCapture_Close(
    NEXUS_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
Data returned by NEXUS_InputCapture_ReadData
***************************************************************************/
typedef struct NEXUS_InputCaptureData
{
    bool polarity; /* true for rising edge, false for falling edge. only applies for BICAP. */
    unsigned clockCycles; /* the number of clock cycles counted by hardware for the specified transition */
} NEXUS_InputCaptureData;

/***************************************************************************
Summary:
Get data entries from the internal software fifo
***************************************************************************/
NEXUS_Error NEXUS_InputCapture_ReadData(
    NEXUS_InputCaptureHandle inputCapture,
    NEXUS_InputCaptureData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumRead} */
    unsigned numEntries, /* max number that can be read */
    unsigned *pNumRead /* [out] actual number that were read */
    );
    
/***************************************************************************
Summary:
Get number of edges 
***************************************************************************/
NEXUS_Error NEXUS_InputCapture_GetEdgeCount(
    NEXUS_InputCaptureHandle inputCapture,
    unsigned *pNumEdges /* [out] number of edges */
    );

/***************************************************************************
Summary:
Get the input capture settings.
***************************************************************************/
void NEXUS_InputCapture_GetSettings(
    NEXUS_InputCaptureHandle inputCapture, 
    NEXUS_InputCaptureSettings *settings /* [out] */
    );

/***************************************************************************
Summary:
Set the input capture settings.
***************************************************************************/
NEXUS_Error NEXUS_InputCapture_SetSettings(
    NEXUS_InputCaptureHandle inputCapture, 
    const NEXUS_InputCaptureSettings *settings
    );

/***************************************************************************
Summary:
Enable the capture
***************************************************************************/
NEXUS_Error NEXUS_InputCapture_Enable(
    NEXUS_InputCaptureHandle inputCapture
    );

/***************************************************************************
Summary:
Disable the capture
***************************************************************************/
void NEXUS_InputCapture_Disable(
    NEXUS_InputCaptureHandle inputCapture
    );

/***************************************************************************
Summary:
Input Capture Timeout settings 
***************************************************************************/
typedef struct NEXUS_InputCaptureTimeoutSettings
{
    NEXUS_InputCaptureHandle inputCapture; /* the input capture selected. this only applies to BICAP. */
    NEXUS_InputCaptureEdgeSelect edgeSelect;
    NEXUS_InputCaptureClockSelect clockSelect;
    unsigned clockCycles;
        /* 'clockCycles' is based on the clock frequency of the clock selected for clockSelect, above */
        /* See systemClockDivisor or timeoutClockDivisor definition in the structure, */
        /* "NEXUS_InputCaptureSettings" for more information.  Value limited from 0 to 4095 */
} NEXUS_InputCaptureTimeoutSettings;

/***************************************************************************
Summary:
Get default settings
***************************************************************************/
void NEXUS_InputCaptureTimeout_GetDefaultSettings(
    NEXUS_InputCaptureTimeoutSettings *pSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_InputCaptureTimeoutHandle NEXUS_InputCaptureTimeout_Open( /* attr{destructor=NEXUS_InputCaptureTimeout_Close} */
    unsigned index, /* supports NEXUS_ANY_ID */
    const NEXUS_InputCaptureTimeoutSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_InputCaptureTimeout_Close(
    NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout
    );

/***************************************************************************
Summary:
Gets one of the four timeouts.
***************************************************************************/
void NEXUS_InputCaptureTimeout_GetSettings(
    NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout, 
    NEXUS_InputCaptureTimeoutSettings *pSettings
    );

/***************************************************************************
Summary:
Sets one of the four timeouts.
***************************************************************************/
NEXUS_Error NEXUS_InputCaptureTimeout_SetSettings(
    NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout, 
    const NEXUS_InputCaptureTimeoutSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_INPUT_CAPTURE_H__ */

