/***************************************************************************
 *     (c)2006-2009 Broadcom Corporation
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "baudio_priv.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_audio_module.h"
#include "bape_dsp_ipc_priv.h"
#include <pthread.h>

BDBG_MODULE(nexus_audio_emulation);

BREG_Handle g_Audio_MemReg;
static BKNI_EventHandle g_ipcEvent;
static bool g_shutdown;
static NEXUS_ThreadHandle g_thread;
static BAPE_DspIpcBlock *g_pIpc;
static B_Audio_Handle g_audio;

static void NEXUS_AudioEmulationThread(void *pParam);
static void NEXUS_AudioEmulationInterrupt(void *pParam, const BAPE_Interrupt *pInterrupt);

extern void *B_Audio_AacAmdDecoderInit;
extern void *B_Audio_DdpInit;
extern void *B_Audio_MpegAmdDecoderInit;
extern void *B_Audio_WmaAmdDecoderInit;

uint32_t B_Audio_VirtToPhys(const void *addr)
{
    return NEXUS_AddrToOffset(addr);
}

void *B_Audio_PhysToVirt(uint32_t  addr)
{
    return NEXUS_OffsetToCachedAddr(addr);
}

void B_Audio_FlushInvalidateDCache(const void *addr, size_t size)
{
    NEXUS_FlushCache(addr, size);
}

BERR_Code BAPE_P_DspMessageReady(
    BAPE_Handle handle
    )
{
    BSTD_UNUSED(handle);
    BKNI_SetEvent(g_ipcEvent);
    return BERR_SUCCESS;
}

BERR_Code BAPE_P_DspBoot(
    BAPE_Handle handle,
    BAPE_DspIpcBlock *pIpc
    )
{
    BERR_Code errCode;
    B_Audio_Settings audioSettings;
    BAPE_Message msg;
    BAPE_Reply reply;

    BSTD_UNUSED(handle);
    BDBG_ASSERT(NULL == g_pIpc);

    g_shutdown = false;
    g_pIpc = pIpc;
    errCode = BKNI_CreateEvent(&g_ipcEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_event;
    }
    
    g_Audio_MemReg = g_NEXUS_pCoreHandles->reg;

    B_Audio_GetDefaultSettings(&audioSettings);
    audioSettings.maxModules = pIpc->maxModules;
    audioSettings.maxTasks = pIpc->maxTasks;
    audioSettings.interruptCallback = NEXUS_AudioEmulationInterrupt;
    audioSettings.pInterruptCallbackParam = pIpc;

    BDBG_MSG(("Opening audio interface (%u tasks, %u modules)", audioSettings.maxModules, audioSettings.maxTasks));
    g_audio = B_Audio_Open(&audioSettings);
    if ( NULL == g_audio )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_open;
    }

    /* Register all modules in the correct order */
    msg.id = BAPE_MessageId_eRegisterModule;
    msg.data.registerModule.pEntry = &B_Audio_AacAmdDecoderInit;
    msg.data.registerModule.pCodeSegmentBase = NULL;
    msg.data.registerModule.codeSegmentLength = 0;
    BDBG_MSG(("Registering AAC module - Entry 0x%08x", msg.data.registerModule.pEntry));
    errCode = B_Audio_ProcessMessage(g_audio, &msg, &reply);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    msg.data.registerModule.pEntry = &B_Audio_DdpInit;
    BDBG_MSG(("Registering DDP module - Entry 0x%08x", msg.data.registerModule.pEntry));
    errCode = B_Audio_ProcessMessage(g_audio, &msg, &reply);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    msg.data.registerModule.pEntry = &B_Audio_MpegAmdDecoderInit;
    BDBG_MSG(("Registering MPEG module - Entry 0x%08x", msg.data.registerModule.pEntry));
    errCode = B_Audio_ProcessMessage(g_audio, &msg, &reply);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    msg.data.registerModule.pEntry = &B_Audio_WmaAmdDecoderInit;
    BDBG_MSG(("Registering WMA module - Entry 0x%08x", msg.data.registerModule.pEntry));
    errCode = B_Audio_ProcessMessage(g_audio, &msg, &reply);
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    BDBG_MSG(("Creating Thread"));
    g_thread = NEXUS_Thread_Create("Audio Emulation", NEXUS_AudioEmulationThread, NULL, NULL);
    if ( NULL == g_thread )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_thread;
    }

    BDBG_MSG(("DSP Boot Successful"));

    return BERR_SUCCESS;

err_thread:
    B_Audio_Close(g_audio);
    g_audio = NULL;
err_open:
    BKNI_DestroyEvent(g_ipcEvent);
    g_ipcEvent = NULL;
err_event:
    g_pIpc = NULL;
    return errCode;
}

void BAPE_P_DspHalt(
    BAPE_Handle handle
    )
{
    BSTD_UNUSED(handle);
    if ( g_thread )
    {
        g_shutdown = true;
        BKNI_SetEvent(g_ipcEvent);
        NEXUS_Thread_Destroy(g_thread);
        g_thread = NULL;
        g_shutdown = false;
    }
    if ( g_audio )
    {
        B_Audio_Close(g_audio);
        g_audio = NULL;
    }
    if ( g_ipcEvent )
    {
        BKNI_DestroyEvent(g_ipcEvent);
        g_ipcEvent = NULL;
    }    
    g_pIpc = NULL;
}

static void NEXUS_AudioEmulationInterrupt(void *pParam, const BAPE_Interrupt *pInterrupt)
{
    BAPE_DspIpcBlock *pIpc = pParam;
    uint32_t rd,wr;

    rd = pIpc->interruptRead;
    wr = pIpc->interruptWrite;

    /* Check for overflow */
    if ( rd == wr && pIpc->interrupts[wr].id != BAPE_InterruptId_eNone )
    {
        BDBG_ERR(("interrupt queue overflow"));
        BDBG_ASSERT(0);
    }
    pIpc->interrupts[wr] = *pInterrupt;
    wr++;
    if ( wr == BAPE_MAX_INTERRUPTS )
    {
        wr = 0;
    }
    pIpc->interruptWrite = wr;

    BKNI_EnterCriticalSection();
    BAPE_P_DspIpcInterrupt_isr(NEXUS_AUDIO_DEVICE_HANDLE);
    BKNI_LeaveCriticalSection();
}

static void B_Barrier(void)
{
    (void)pthread_self();
}

static void NEXUS_AudioEmulationThread(void *pParam)
{
    BAPE_DspIpcBlock *pIpc = g_pIpc;
    B_Audio_TaskStatus status;
    BERR_Code errCode;
    B_Audio_Handle audio = g_audio;
    pthread_t myself;
    struct sched_param schedparam;

    BDBG_MSG(("DSP Task Init"));

    myself = pthread_self();
    schedparam.sched_priority = sched_get_priority_max(SCHED_FIFO)-1;
    pthread_setschedparam(myself, SCHED_FIFO, &schedparam);

    while ( !g_shutdown )
    {
        /* Bump run counter */
        pIpc->targetTicks++;

        B_Barrier();

        /* Check for message */
        if ( pIpc->messageNum != pIpc->prevMessageNum )
        {
            BAPE_Message msg;
            uint32_t messageNum;

            /* New message.  Convert pointers and dispatch. */
            msg = pIpc->message;
            messageNum = pIpc->messageNum;

            BDBG_MSG(("Processing MSG ID %u", msg.id));
            errCode = B_Audio_ProcessMessage(audio, &msg, &pIpc->reply);
            if ( errCode )
            {
                BDBG_ERR(("Error processing message ID %u", msg.id));
                BDBG_ASSERT(errCode == BERR_SUCCESS);
            }
            BDBG_MSG(("Processing MSG ID %u successful", msg.id));

            pIpc->prevMessageNum = messageNum;
            pIpc->replyNum = messageNum;

            BKNI_EnterCriticalSection();
            BAPE_P_DspIpcInterrupt_isr(NEXUS_AUDIO_DEVICE_HANDLE);
            BKNI_LeaveCriticalSection();
        }

        errCode = B_Audio_RunTask(audio, &status);
        if ( errCode )
        {
            BDBG_ERR(("Error running task"));
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }

        B_Barrier();

        /* If we have a timeout, wait for it. */
        if ( status.nextTimeoutUs > 0 )
        {
            unsigned timeout = status.nextTimeoutUs/1000;

            /* Don't wait more than 10ms */
            timeout = (timeout > 10)?10:timeout;

            if ( timeout >= 2 && BKNI_WaitForEvent(g_ipcEvent, 0) )
            {
                BKNI_Sleep(2);
            }
        }
    }
    
    BSTD_UNUSED(pParam);
}

