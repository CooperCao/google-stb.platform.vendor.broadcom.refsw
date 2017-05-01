/***************************************************************************
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
 *
 * Module Description: Debug Status Interface
 *
 ***************************************************************************/


#ifndef BAPE_DEBUG_H_
#define BAPE_DEBUG_H_

#include "bape_fmt_priv.h"
#include "bape_chip_priv.h"

typedef struct BAPE_Debug *BAPE_DebugHandle;

typedef struct BAPE_DebugOpenSettings 
{
    bool temp; /* Future proofing the code, can't leave a structure empty */
} BAPE_DebugOpenSettings;

typedef enum BAPE_DebugSourceType
{
    BAPE_DebugSourceType_eOutput,
    BAPE_DebugSourceType_eVolume,
    BAPE_DebugSourceType_eMax
} BAPE_DebugSourceType;


typedef struct BAPE_DebugDigitalOutputStatus
{       
    char *pName;
    uint32_t index; 
    bool enabled;
    BAPE_DataType   type;       
    bool compressedAsPcm;
    unsigned int sampleRate;
    uint32_t cbits[2];          /* The 2nd D-WORD is not supported for compressed and expected to be 0's */
    uint8_t formatId;           /* Byte 0:Bit 0 Consumer/Professional */
    uint8_t audio;              /* Byte 0:Bit 1 Audio/Non-Audio */
    uint8_t copyright;          /* Byte 0:Bit 2 Copyright */
    uint8_t emphasis;           /* Byte 0:Bits 3-5 Pre-emphasis will be 000 if non-audio */
    uint8_t mode;               /* Byte 0:Bits 6-7 Channel Mode */
    uint8_t categoryCode;       /* Byte 1:Bits 0-7 Category Code and L Bit [7] is L Bit */    
    uint8_t samplingFrequency;  /* Byte 3:Bits 0-3 Sampling Frequency */    
    /* following are only valid for PCM output */
    uint8_t pcmWordLength;      /* Byte 4:Bit 0 Orginal Word Length 0 20bit, 1 24bit */
    uint8_t pcmSampleWordLength; /* Byte 4:Bits 1-3 Sample Word Length Value dependent on wordLength */
    uint8_t pcmOrigSamplingFrequency; /* Byte 4:Bits 4-7 Orginal Sampling Frequency */    
    uint8_t pcmCgmsA;           /* Byte 5:Bits 0-1 CGMSA */    

} BAPE_DebugDigitalOutputStatus;

typedef struct BAPE_DebugOutputStatus
{
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 
    BAPE_DebugDigitalOutputStatus spdif[BAPE_CHIP_MAX_SPDIF_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0 
    BAPE_DebugDigitalOutputStatus hdmi[BAPE_CHIP_MAX_MAI_OUTPUTS];
#endif
} BAPE_DebugOutputStatus;

typedef struct BAPE_DebugOutputVolume
{
    char *pName;
    uint32_t index;
    bool enabled;
    BAPE_DataType   type;
    BAPE_OutputVolume outputVolume;

} BAPE_DebugOutputVolume;

typedef struct BAPE_DebugVolumeStatus
{
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
    BAPE_DebugOutputVolume spdif[BAPE_CHIP_MAX_SPDIF_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
    BAPE_DebugOutputVolume hdmi[BAPE_CHIP_MAX_MAI_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_DACS > 0
    BAPE_DebugOutputVolume dac[BAPE_CHIP_MAX_DACS];
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    BAPE_DebugOutputVolume i2s[BAPE_CHIP_MAX_I2S_OUTPUTS];
#endif
} BAPE_DebugVolumeStatus;


typedef struct BAPE_DebugStatus
{
    BAPE_DebugSourceType type;

    union {
        BAPE_DebugOutputStatus outputStatus;
        BAPE_DebugVolumeStatus volumeStatus;
    } status;
    
} BAPE_DebugStatus;


/***************************************************************************
Summary: 
    Open and allocate handle to be used with requesting various debug;
***************************************************************************/
BERR_Code BAPE_Debug_Open(
    BAPE_Handle deviceHandle,
    const BAPE_DebugOpenSettings * pSettings, 
    BAPE_DebugHandle * pHandle /* [out] */
    );

/***************************************************************************
Summary:
    Close and free debug handle.
***************************************************************************/
void BAPE_Debug_Close(BAPE_DebugHandle handle);


/***************************************************************************
Summary:
    Retrieves the audio status based on the BAPE_DebugSourceType specified.
***************************************************************************/
BERR_Code BAPE_Debug_GetStatus(
    BAPE_DebugHandle handle,
    BAPE_DebugSourceType type,
    BAPE_DebugStatus * pStatus /* out */
    );

/***************************************************************************
Summary:
    Retrieves the status of the enabled outputs.
***************************************************************************/
BERR_Code BAPE_Debug_GetOutputStatus(
    BAPE_DebugHandle handle,    
    BAPE_DebugStatus * pStatus /* out */
    );


/***************************************************************************
Summary:
    Retrieves the volume of the enabled outputs.
***************************************************************************/
BERR_Code BAPE_Debug_GetOutputVolume(
    BAPE_DebugHandle handle,
    BAPE_DebugStatus * pStatus /* out */
    );

/**************************************************************************
Summary:
Debug Interrupt Handlers
***************************************************************************/
typedef struct BAPE_DebugInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } dataReady;
} BAPE_DebugInterruptHandlers;

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
***************************************************************************/
void BAPE_Debug_GetInterruptHandlers(
    BAPE_DebugHandle handle,
    BAPE_DebugInterruptHandlers *pInterrupts /* [out] */
    );

/***************************************************************************
Summary:
***************************************************************************/
BERR_Code BAPE_Debug_SetInterruptHandlers(
    BAPE_DebugHandle handle,
    const BAPE_DebugInterruptHandlers *pInterrupts
    );
#endif

/***************************************************************************
Summary:
    Add an attached output to the debug handle
***************************************************************************/
BERR_Code BAPE_Debug_AddOutput(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output);


/***************************************************************************
Summary:
    Remove a dettached output to the debug handle
***************************************************************************/
BERR_Code BAPE_Debug_RemoveOutput(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output);

/***************************************************************************
Summary:
    Get the channel status for HDMI and Spdif
***************************************************************************/
BERR_Code BAPE_Debug_GetChannelStatus(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output,
    BAPE_DebugDigitalOutputStatus *status);

/***************************************************************************
Summary:
    Get the value of range of bits
***************************************************************************/
uint32_t BAPE_Debug_BitRangeValue(
    uint32_t value,
    unsigned start,
    unsigned stop);

/***************************************************************************
Summary:
    Convert an integer to a string binary value
***************************************************************************/
char* BAPE_Debug_IntToBinaryString(
    unsigned value,
    unsigned signifDigits,
    char* ptr);

#endif /* #ifndef BAPE_DEBUG_H_ */
