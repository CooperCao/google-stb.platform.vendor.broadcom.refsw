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
 * Module Description: Debug Status Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
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
    BAPE_DebugSourceType type;
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 
    BAPE_DebugDigitalOutputStatus spdif[BAPE_CHIP_MAX_SPDIF_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0 
    BAPE_DebugDigitalOutputStatus hdmi[BAPE_CHIP_MAX_MAI_OUTPUTS];
#endif
} BAPE_DebugOutputStatus;

typedef struct BAPE_DebugStatus
{
    BAPE_DebugSourceType type;

    union {
        BAPE_DebugOutputStatus outputStatus;
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

