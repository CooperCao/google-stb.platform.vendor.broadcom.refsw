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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bape_priv.h"
#include "bape_dsp_ipc_priv.h"
#include "belf.h"

#ifndef BAPE_DSP_PRIV_H_
#define BAPE_DSP_PRIV_H_

/***************************************************************************
Summary:
DSP handle
***************************************************************************/
typedef struct BAPE_Dsp *BAPE_DspHandle;

/***************************************************************************
Summary:
DSP context handle
***************************************************************************/
typedef struct BAPE_DspContext *BAPE_DspContextHandle;

/***************************************************************************
Summary:
Invalid task ID
***************************************************************************/
#define BAPE_DSP_TASK_ID_INVALID (0xffffffff)

/***************************************************************************
Summary:
Invalid module ID
***************************************************************************/
#define BAPE_DSP_MODULE_ID_INVALID (0xffffffff)

/***************************************************************************
Summary:
DSP context
***************************************************************************/
typedef struct BAPE_DspContext
{
    BAPE_DspHandle dsp;
    BAPE_DecoderHandle decoder;
    unsigned index;
    uint32_t taskId;

    BAPE_TaskDescriptor *pDescriptor;

    volatile uint32_t *pTsmStatusBegin;
    const BAPE_TaskTsmStatus *pTsmStatus;
    volatile uint32_t *pTsmStatusEnd;

    volatile uint32_t *pTsmSettingsBegin;
    BAPE_TaskTsmSettings *pTsmSettings;
    volatile uint32_t *pTsmSettingsEnd;

    volatile uint32_t *pFmmSampleRate;
    volatile uint8_t *pPlaybackStcValid;
} BAPE_DspContext;

/***************************************************************************
Summary:
Image Names
***************************************************************************/
typedef enum BAPE_Img_Id
{
    BAPE_Img_Id_eBkernel,
    BAPE_Img_Id_eAacDecode,
    BAPE_Img_Id_eAc3Decode,
    BAPE_Img_Id_eDdpDecode,
    BAPE_Img_Id_eMpegDecode,
    BAPE_Img_Id_eWmaDecode,
    BAPE_Img_Id_eAc3Passthrough,
    BAPE_Img_Id_ePcmWav,
    BAPE_Img_Id_eMax
} BAPE_Img_Id;

/***************************************************************************
Summary:
DSP Settings
***************************************************************************/
typedef struct BAPE_DspSettings
{
    const BIMG_Interface *pImageInterface;              
    void **pImageContext;
    unsigned heapSize;  /* Total memory size for the DSP kernel heap */
    unsigned stackSize; /* DSP Stack size in bytes */
    unsigned sharedMemorySize;  /* DSP shared memory size in bytes */
    uint32_t (*readReserved)(void *pParam1, int param2, uint32_t physicalAddress);
    void (*writeReserved)(void *pParam1, int param2, uint32_t physicalAddress, uint32_t data);
    uint32_t (*readCoreRegister)(void *pParam1, int param2, uint32_t offset);
    void (*writeCoreRegister)(void *pParam1, int param2, uint32_t offset, uint32_t data);
    uint32_t (*readCmtControl)(void *pParam1, int param2);
    void (*writeCmtControl)(void *pParam1, int param2, uint32_t data);
    void *pParam1;
    int param2;
} BAPE_DspSettings;

/***************************************************************************
Summary:
DSP handle
***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BAPE_Dsp);
typedef struct BAPE_Dsp
{
    BDBG_OBJECT(BAPE_Dsp)

    BAPE_Handle deviceHandle;

    BAPE_DspSettings settings;

    BAPE_DspContext contexts[BAPE_CHIP_MAX_DECODERS];

    /* messaging is synchronous */
    BAPE_Message msg;
    BAPE_Reply reply;

    BAPE_DspIpcBlock *pIpc;
    BKNI_EventHandle replyEvent;

    /* Watchdog Info */
    BTMR_TimerHandle timerHandle;
    uint32_t prevTicks;
    uint32_t violations, totalViolations;
    bool watchdogAsserted;
    size_t logReadCnt; 

#ifdef BAPE_EMULATION
    void *pIpcBase;             /* Uncached */
#else
    BINT_CallbackHandle stateChangeInterrupt;
    BINT_CallbackHandle ipcInterrupt;
    void *pInitMemory;
    struct
    {
        void *pImageBase;
        unsigned imageSize;
        int imageParam;
        BELF_ImageHandle elf;
        unsigned usageCount;
        uint32_t moduleId;
        void *pBase;
        void *pEntry;
        unsigned size;
        void *pTextBase;
        void *pTextBaseUncached;
        unsigned textSize;
    } images[BAPE_Img_Id_eMax];
#endif
} BAPE_Dsp;

/***************************************************************************
Summary:
Get Default DSP Settings
***************************************************************************/
void BAPE_Dsp_P_GetDefaultSettings(
    BAPE_DspSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a DSP handle
***************************************************************************/
BERR_Code BAPE_Dsp_P_Open(
    BAPE_Handle deviceHandle,
    const BAPE_DspSettings *pSettings,
    BAPE_DspHandle *pHandle     /* [out] */
    );

/***************************************************************************
Summary:
Close a DSP handle
***************************************************************************/
void BAPE_Dsp_P_Close(
    BAPE_DspHandle handle
    );

/***************************************************************************
Summary:
Create a DSP context
***************************************************************************/
BERR_Code BAPE_DspContext_P_Create(
    BAPE_DspHandle dspHandle,
    BAPE_DecoderHandle decoder,
    BAPE_DspContextHandle *pHandle
    );

/***************************************************************************
Summary:
Destroy a DSP context
***************************************************************************/
void BAPE_DspContext_P_Destroy(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Prepare a DSP task
***************************************************************************/
BERR_Code BAPE_DspContext_P_PrepareTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Start a DSP task
***************************************************************************/
BERR_Code BAPE_DspContext_P_StartTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Stop a task
***************************************************************************/
void BAPE_DspContext_P_StopTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Cleanup (Un-Prepare) a task
***************************************************************************/
void BAPE_DspContext_P_CleanupTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Pause a DSP task
***************************************************************************/
BERR_Code BAPE_DspContext_P_PauseTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Resume a DSP task
***************************************************************************/
BERR_Code BAPE_DspContext_P_ResumeTask(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Advance a DSP task
***************************************************************************/
BERR_Code BAPE_DspContext_P_AdvanceTask(
    BAPE_DspContextHandle handle,
    unsigned ms
    );

/***************************************************************************
Summary:
Set task TSM settings
***************************************************************************/
BERR_Code BAPE_DspContext_P_SetTsmSettings_isr(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    );

/***************************************************************************
Summary:
Indicate a playback STC is valid to the decoder in isr context
***************************************************************************/
BERR_Code BAPE_DspContext_P_SetStcValid_isr(
    BAPE_DspContextHandle handle
    );

/***************************************************************************
Summary:
Get task TSM status
***************************************************************************/
BERR_Code BAPE_DspContext_P_GetTsmStatus_isr(
    BAPE_DspContextHandle handle,
    BAPE_DecoderTsmStatus *pStatus      /* [out] */
    );

/***************************************************************************
Summary:
Set task settings
***************************************************************************/
BERR_Code BAPE_DspContext_P_SetSettings(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderSettings *pSettings
    );

/***************************************************************************
Summary:
Set task settings
***************************************************************************/
void BAPE_DspContext_P_GetStatus(
    BAPE_DspContextHandle handle,
    BAPE_DecoderStatus *pStatus      /* [out] */
    );

/***************************************************************************
Summary:
Set task codec-specific settings
***************************************************************************/
BERR_Code BAPE_DspContext_P_SetCodecSettings(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    );

/***************************************************************************
Summary:
Process a watchdog interrupt
***************************************************************************/
BERR_Code BAPE_Dsp_P_ProcessWatchdog(
    BAPE_DspHandle handle
    );

/***************************************************************************
Summary:
Produce a backtrace from the DSP
***************************************************************************/
#if BDBG_DEBUG_BUILD
void BAPE_Dsp_P_Backtrace(
    BAPE_DspHandle handle
    );
#endif
const char *BAPE_Dsp_P_ImgIdToName(BAPE_Img_Id id);

#endif /* #ifndef BAPE_DSP_PRIV_H_ */

