/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_PLAYBACK_H_
#define BAPE_PLAYBACK_H_

/***************************************************************************
Summary:
Playback Handle
***************************************************************************/
typedef struct BAPE_Playback *BAPE_PlaybackHandle;

/***************************************************************************
Summary:
Playback Open settings
***************************************************************************/
typedef struct BAPE_PlaybackOpenSettings
{
    unsigned fifoSize;              /* FIFO Size in bytes, must be a multiple of 256 */
    unsigned watermarkThreshold;    /* FIFO interrupt threshold in bytes.  
                                       When an amuont <= this threshold is available, 
                                       an interrupt will be raised. This value should
                                       be a multiple of 256. */    
} BAPE_PlaybackOpenSettings;

/***************************************************************************
Summary:
Get Default Playback Open Settings
***************************************************************************/
void BAPE_Playback_GetDefaultOpenSettings(
    BAPE_PlaybackOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Open a playback channel
***************************************************************************/
BERR_Code BAPE_Playback_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_PlaybackOpenSettings *pSettings,
    BAPE_PlaybackHandle *pHandle                    /* [out] */
    );

/***************************************************************************
Summary:
Close a playback channel
***************************************************************************/
void BAPE_Playback_Close(
    BAPE_PlaybackHandle handle
    );

/***************************************************************************
Summary:
Playback Start Settings
***************************************************************************/
typedef struct BAPE_PlaybackStartSettings
{
    BAVC_AudioSamplingRate sampleRate;
    unsigned bitsPerSample;                 /* Supports 8, 16, or 32 */
    bool isStereo;                          /* If true, data will be treated as stereo.  If false, data will be treated as mono. */
    bool isSigned;                          /* If true, data will be treated as signed.  If false, data will be treated as unsigned. */
    bool reverseEndian;                     /* If true, data will be endian-swapped in hardware.  Otherwise, data should match host endian mode. */
    bool loopEnabled;                       /* If true, data will loop continuously without requiring host intervention. */
    unsigned startThreshold;                /* Size in bytes that must be committed to the hardware before data will flow into the mixer.  
                                               Must be a multiple of 256. */    
} BAPE_PlaybackStartSettings;

/***************************************************************************
Summary:
Get Default Playback Start Settings
***************************************************************************/
void BAPE_Playback_GetDefaultStartSettings(
    BAPE_PlaybackStartSettings *pSettings       /* [out] */
    );

/***************************************************************************
Summary:
Start Playback
***************************************************************************/
BERR_Code BAPE_Playback_Start(
    BAPE_PlaybackHandle handle,
    const BAPE_PlaybackStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop Playback
***************************************************************************/
void BAPE_Playback_Stop(
    BAPE_PlaybackHandle handle
    );

/***************************************************************************
Summary:
Flush Playback Buffer
***************************************************************************/
void BAPE_Playback_Flush(
    BAPE_PlaybackHandle handle
    );

/***************************************************************************
Summary:
Get Playback Buffer 
 
Description: 
This routine will return the next contiguous buffer address and size.  If 
interrupts are enabled, the caller should call this routine and commit data 
until the size returned from this function is zero. 
***************************************************************************/
BERR_Code BAPE_Playback_GetBuffer(
    BAPE_PlaybackHandle handle,
    void **pBuffer,                     /* [out] Address data should be written to (uncached) */
    unsigned *pBufferSize               /* [out] Contiguous buffer size in bytes */
    );

/***************************************************************************
Summary:
Commit Data to the playback hardware
***************************************************************************/
BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    );

/***************************************************************************
Summary:
Playback Status
***************************************************************************/
typedef struct BAPE_PlaybackStatus
{
    unsigned queuedBytes;
    unsigned fifoSize;
} BAPE_PlaybackStatus;

/***************************************************************************
Summary:
Get Playback Status
***************************************************************************/
void BAPE_Playback_GetStatus(
    BAPE_PlaybackHandle handle,
    BAPE_PlaybackStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_Playback_GetConnector(
    BAPE_PlaybackHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Playback Interrupt Handlers
***************************************************************************/
typedef struct BAPE_PlaybackInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } watermark;
} BAPE_PlaybackInterruptHandlers;

/***************************************************************************
Summary:
Get Playback Interrupt Handlers
***************************************************************************/
void BAPE_Playback_GetInterruptHandlers(
    BAPE_PlaybackHandle handle,
    BAPE_PlaybackInterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Playback Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_Playback_SetInterruptHandlers(
    BAPE_PlaybackHandle handle,
    const BAPE_PlaybackInterruptHandlers *pInterrupts
    );

#endif /* #ifndef BAPE_PLAYBACK_H_ */

