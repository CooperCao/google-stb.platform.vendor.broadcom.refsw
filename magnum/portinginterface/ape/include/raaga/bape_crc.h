/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio CRC Interface
 *
 ***************************************************************************/

#ifndef BAPE_CRC_H_
#define BAPE_CRC_H_

#include "bape.h"
#include "bape_types.h"
#include "bape_mixer.h"


typedef struct BAPE_Crc *BAPE_CrcHandle;


/***************************************************************************
Summary:
CRC Source Type
***************************************************************************/
typedef enum BAPE_CrcSourceType
{
    BAPE_CrcSourceType_ePlaybackBuffer, /* Capture data entering the FMM from a BAPE_MixerInput */
    BAPE_CrcSourceType_eOutputPort,     /* Capture CRC data for Audio entering a BAPE_OutputPort */
    BAPE_CrcSourceType_eMax
} BAPE_CrcSourceType;
 
/***************************************************************************
Summary:
CRC Mode
***************************************************************************/
typedef enum BAPE_CrcMode
{
    BAPE_CrcMode_eFreeRun,
    BAPE_CrcMode_eSingle,
    BAPE_CrcMode_eMax
} BAPE_CrcMode;
 
/***************************************************************************
Summary:
CRC Data Width
***************************************************************************/
typedef enum BAPE_CrcDataWidth
{
    BAPE_CrcDataWidth_e16,
    BAPE_CrcDataWidth_e20,
    BAPE_CrcDataWidth_e24,
    BAPE_CrcDataWidth_eMax
} BAPE_CrcDataWidth;

/***************************************************************************
Summary:
CRC Open Settings
***************************************************************************/
typedef struct BAPE_CrcOpenSettings
{
    /*BAPE_CrcSourceType sourceType;*/      /* All open CRC objects must share the same source type */
    unsigned numChannelPairs;           /* Set to 1 for stereo or compressed capture, 3 for 5.1 or 4 for 7.1 */
    size_t bufferSize;                  /* Buffer size in bytes (allocated per channel pair) */
    BMEM_Handle memHandle;              /* BMEM heap to use for allocation of CRC buffer */
    unsigned samplingPeriod;            /* Number of samples to accumulate per CRC */
    BAPE_CrcDataWidth dataWidth;        /* Supported values are 16, 20, or 24 bits per sample */
    BAPE_CrcMode mode;                  /* Capture mode */
    uint16_t initialValue;              /* Initial value.  0x0000 and 0xffff are currently supported.
                                           reset to this value during each start */
} BAPE_CrcOpenSettings;
 
/***************************************************************************
Summary:
CRC Interrupt Handlers
***************************************************************************/
typedef struct BAPE_CrcInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } dataReady;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } captureStarted;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } captureStopped;
} BAPE_CrcInterruptHandlers;

/***************************************************************************
Summary:
CRC Input Settings
***************************************************************************/
typedef struct BAPE_CrcInputSettings
{
    union
    {
        struct
        {
            BAPE_MixerHandle mixer;
            BAPE_Connector input;
        } playbackBuffer;
        struct
        {
            BAPE_OutputPort outputPort;
        } outputPort;
    } source;    
} BAPE_CrcInputSettings;
 
/***************************************************************************
Summary:
CRC Entry
***************************************************************************/
typedef struct BAPE_CrcEntry
{
    uint16_t seqNumber;                 /* Sequence Number */
    uint16_t value;                     /* CRC Value */
} BAPE_CrcEntry;

/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultOpenSettings(
    BAPE_CrcOpenSettings *pSettings /*[out] */
    );

/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultInputSettings(
    BAPE_CrcSourceType sourceType,
    BAPE_CrcInputSettings *pInputSettings /*[out] */
    );

/***************************************************************************
Summary:
CRC Open
***************************************************************************/
BERR_Code BAPE_Crc_Open(
    BAPE_Handle deviceHandle,
    unsigned index, 
    const BAPE_CrcOpenSettings * pSettings, 
    BAPE_CrcHandle * pHandle /* [out] */
    );

/***************************************************************************
Summary:
CRC Close
***************************************************************************/
BERR_Code BAPE_Crc_Close(
    BAPE_CrcHandle handle
    );
 
/***************************************************************************
Summary:
CRC AddInput
***************************************************************************/
BERR_Code BAPE_Crc_AddInput(
    BAPE_CrcHandle handle, 
    BAPE_CrcSourceType sourceType,
    const BAPE_CrcInputSettings * pInputSettings
    );

/***************************************************************************
Summary:
CRC RemoveInput
***************************************************************************/
BERR_Code BAPE_Crc_RemoveInput(
    BAPE_CrcHandle handle
    );

/***************************************************************************
Summary:
CRC Get Buffer
***************************************************************************/
BERR_Code BAPE_Crc_GetBuffer(
    BAPE_CrcHandle handle, 
    BAPE_BufferDescriptor * pBuffers /* [out] */
    );

/***************************************************************************
Summary:
CRC Consume Data
***************************************************************************/
BERR_Code BAPE_Crc_ConsumeData(
    BAPE_CrcHandle handle, 
    unsigned numBytes
    );


#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
CRC Get Interrupt Handlers
***************************************************************************/
void BAPE_Crc_GetInterruptHandlers(
    BAPE_CrcHandle handle,
    BAPE_CrcInterruptHandlers *pInterrupts /* [out] */
    );

/***************************************************************************
Summary:
CRC Set Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_Crc_SetInterruptHandlers(
    BAPE_CrcHandle handle,
    const BAPE_CrcInterruptHandlers *pInterrupts
    );
#endif
#endif /* #ifndef BAPE_CRC_H_ */
