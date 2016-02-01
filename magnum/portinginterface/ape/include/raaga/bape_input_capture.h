/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
 * Module Description: Audio Input Capture Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_INPUT_CAPTURE_H_
#define BAPE_INPUT_CAPTURE_H_

/***************************************************************************
Summary:
InputCapture Handle
***************************************************************************/
typedef struct BAPE_InputCapture *BAPE_InputCaptureHandle;

/***************************************************************************
Summary:
InputCapture open settings
***************************************************************************/
typedef struct BAPE_InputCaptureOpenSettings
{
    BMEM_Heap_Handle heap;          /* Heap used for allocating buffers.  If not set, the heap
                                       provided in BAPE_Open will be used. */
    unsigned numBuffers;            /* Number of buffers to allocate.  For mono or interleaved stereo,
                                       one buffer is required.  For non-interleaved 7.1 data 8 buffers
                                       are required.  For interleaved 7.1 data, 4 buffers are required. */
    unsigned bufferSize;            /* Buffer size in bytes, must be a multiple of 256.  If more than one
                                       buffer is allocated, all buffers will be this size.  If this size
                                       is 0, buffers will be allocated from the internal buffer pool. */
    unsigned watermarkThreshold;    /* FIFO interrupt threshold in bytes.
                                       When an amuont >= this threshold is available,
                                       an interrupt will be raised. This value should
                                       be a multiple of 256. */
    bool useLargeRingBuffers;       /* If true, large ringbuffers will be used,
                                       otherwise standard buffers will be used. */
} BAPE_InputCaptureOpenSettings;

/***************************************************************************
Summary:
Get Default InputCapture Settings
***************************************************************************/
void BAPE_InputCapture_GetDefaultOpenSettings(
    BAPE_InputCaptureOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Open an input capture channel
***************************************************************************/
BERR_Code BAPE_InputCapture_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_InputCaptureOpenSettings *pSettings,
    BAPE_InputCaptureHandle *pHandle                    /* [out] */
    );

/***************************************************************************
Summary:
Close an input capture channel
***************************************************************************/
void BAPE_InputCapture_Close(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
InputCapture Settings
***************************************************************************/
typedef struct BAPE_InputCaptureSettings
{
    unsigned bitsPerSample;                 /* Currently, 32-bit PCM capture is supported on all chips and
                                               16 bit captures is supported on HW that supports it.  */
    bool interleaveData;                    /* If true, PCM data for a stereo pair will be interleaved
                                               into a single buffer.  False will use a separate buffer
                                               for each channel. */
    unsigned watermark;                     /* FIFO interrupt threshold in bytes.
                                               When an amuont >= this threshold is available,
                                               an interrupt will be raised. This value should
                                               be a multiple of 256. */
} BAPE_InputCaptureSettings;

/***************************************************************************
Summary:
Get InputCapture Settings
***************************************************************************/
void BAPE_InputCapture_GetSettings(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureSettings *pSettings       /* [out] */
    );

/***************************************************************************
Summary:
Set InputCapture Settings
***************************************************************************/
BERR_Code BAPE_InputCapture_SetSettings(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureSettings *pSettings
    );

/***************************************************************************
Summary:
Flush InputCapture Buffer
***************************************************************************/
void BAPE_InputCapture_Flush(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
Flush InputCapture Buffer from interrupt context
***************************************************************************/
void BAPE_InputCapture_Flush_isr(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
InputCapture Start Settings
***************************************************************************/
typedef struct BAPE_InputCaptureStartSettings
{
    BAPE_InputPort input;
} BAPE_InputCaptureStartSettings;

/***************************************************************************
Summary:
Get Default InputCapture Start Settings
***************************************************************************/
void BAPE_InputCapture_GetDefaultStartSettings(
    BAPE_InputCaptureStartSettings *pSettings       /* [out] */
    );

/***************************************************************************
Summary:
Start InputCapture
***************************************************************************/
BERR_Code BAPE_InputCapture_Start(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop InputCapture
***************************************************************************/
void BAPE_InputCapture_Stop(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_InputCapture_GetConnector(
    BAPE_InputCaptureHandle handle,
    BAPE_Connector *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Input Capture Interrupt Handlers
***************************************************************************/
typedef struct BAPE_InputCaptureInterruptHandlers
{
    /* This interrupt fires when the active input has changed to an incompatible format and been halted.
       The application must call BAPE_InputCapture_Stop() and BAPE_InputCapture_Start() to resume processing. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } inputHalted;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } watermark;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } overflow;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned sampleRate);
        void *pParam1;
        int param2;
    } sampleRate;
} BAPE_InputCaptureInterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BAPE_InputCapture_GetInterruptHandlers(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureInterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BAPE_InputCapture_SetInterruptHandlers(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureInterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Audio Decoder Status
***************************************************************************/
typedef struct BAPE_InputCaptureStatus
{
    bool running;
    bool halted;
} BAPE_InputCaptureStatus;

/***************************************************************************
Summary:
Get Audio InputCapture Status
***************************************************************************/
void BAPE_InputCapture_GetStatus(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureStatus *pStatus     /* [out] */
    );

/***************************************************************************
Summary:
Get Capture Buffer

Description:
This routine will return the next contiguous buffer address and size.  If
interrupts are enabled, the caller should call this routine and commit data
until the size returned from this function is zero.
***************************************************************************/
BERR_Code BAPE_InputCapture_GetBuffer(
    BAPE_InputCaptureHandle hInputCapture,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    );

/***************************************************************************
Summary:
Mark data as consumed from the InputCapture hardware
***************************************************************************/
BERR_Code BAPE_InputCapture_ConsumeData(
    BAPE_InputCaptureHandle hInputCapture,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    );

#endif /* #ifndef BAPE_INPUT_CAPTURE_H_ */

