/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio CRC Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
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
 
#endif /* #ifndef BAPE_CRC_H_ */

