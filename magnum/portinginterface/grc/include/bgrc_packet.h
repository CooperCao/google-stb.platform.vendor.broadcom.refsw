/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description: GRC Packet header
 *
 ***************************************************************************/

#ifndef BGRC_PACKET_H__
#define BGRC_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bm2mc_packet.h"
#include "berr_ids.h"

/***************************************************************************/
typedef struct BGRC_P_PacketContext *BGRC_PacketContext_Handle;

/***************************************************************************/
#define BGRC_PACKET_MSG_PACKETS_INCOMPLETE   BERR_MAKE_CODE(BERR_GRC_ID, 0x0100)
#define BGRC_PACKET_MSG_BLITS_COMPLETE       BERR_MAKE_CODE(BERR_GRC_ID, 0x0101)

/***************************************************************************/
typedef struct
{
    BMEM_Handle packet_buffer_heap;      /* heap for packet buffer allocations */
    size_t packet_buffer_size;           /* size of packet ring buffer to allocate */
    size_t packet_buffer_store;          /* size of packet buffer to store before submitting */
    void *private_data;                  /* ptr passed back to the user when getting status */
    struct {
        uint32_t offset;                 /* bounded memory offset (0=no bounds check) */
        size_t size;                     /* bounded memory size  (0=no bounds check)*/
    } memory_bounds;                     /* verify blits do not violate memory bounds */
    bool secure;
}
BGRC_PacketContext_CreateSettings;

/***************************************************************************/
typedef struct
{
    BGRC_PacketContext_Handle hContext;
    void *private_data;               /* ptr to user's private data */
    size_t packet_buffer_available;   /* size of available packet memory */
    bool sync;                        /* indicates blit sync */
}
BGRC_Packet_ContextStatus;

/***************************************************************************/
typedef struct
{
    bool m2mc_busy;                   /* indicates m2mc is busy */
}
BGRC_Packet_Status;

/***************************************************************************
 PACKET API - MAIN FUNCTIONS
 ***************************************************************************/
/***************************************************************************
Set one callback for all contexts, which overrides any context callbacks.
Either a single callback will be called for all interrupts, or context's
can get their own callbacks using BGRC_Packet_SetContextCallbacks.
 ***************************************************************************/
BERR_Code BGRC_Packet_SetCallback(
    BGRC_Handle hGrc,
    BGRC_Callback callback_isr,   /* general callback for all contexts */
    void *callback_data           /* ptr to user's data */
);

/***************************************************************************/
BERR_Code BGRC_Packet_GetDefaultCreateContextSettings(
    BGRC_Handle hGrc,
    BGRC_PacketContext_CreateSettings *pSettings
);

/***************************************************************************/
BERR_Code BGRC_Packet_CreateContext(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle *phContext,
    BGRC_PacketContext_CreateSettings *pSettings
);

/***************************************************************************/
BERR_Code BGRC_Packet_DestroyContext(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext
);

/***************************************************************************
Set Advance and Sync callbacks per context. These callbacks will not be used
if setting a general callback through BGRC_Packet_SetCallback.
 ***************************************************************************/
BERR_Code BGRC_Packet_SetContextCallbacks(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext,
    BGRC_Callback advance_callback_isr,   /* callback indicating packet procssing can be */
    void *advance_callback_data,          /* advanced because device fifo memory is available */
    BGRC_Callback sync_callback_isr,      /* callback indicating blits are complete */
    void *sync_callback_data              /* up to the point of the sync */
);

/***************************************************************************
Gets packet memory from the packet ring buffer. If the specified minimum
size is not available, then the ring buffer will wrap if possible and
return memory from the front of the buffer, otherwise a size of zero will
be returned. The returned size will be the maximium amount available.
 ***************************************************************************/
BERR_Code BGRC_Packet_GetPacketMemory(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext, /* cannot be null */
    void **buffer,                      /* [out] ptr to packet buffer ptr */
    size_t *size_out,                   /* [out] ptr to size of buffer */
    size_t size_in                      /* minimum size requested (4-bytes or greater) */
);

/***************************************************************************
Submit packets for processing to a context. If all packets cannot be
processed, then _MSG_PACKETS_INCOMPLETE is returned, and the callback is
called when packet processing becomes possible again. If there is no
callback specified, remaining packets still need to be advanced.
 ***************************************************************************/
BERR_Code BGRC_Packet_SubmitPackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext, /* cannot be null */
    size_t size                         /* size of packet buffer submitted */
);

/***************************************************************************
Advances the context's uncompleted packet processing. If all the packets
cannot be finished, then _MSG_PACKETS_INCOMPLETE is returned, and the
callback is called when packet processing becomes possible again. If there
is no callback specified, then all packets will have finished processing
upon return.
 ***************************************************************************/
BERR_Code BGRC_Packet_AdvancePackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext /* deprecated */
);

/***************************************************************************
Synchronizes the context's blits. If there are any remaining packets that
still need to be Advanced, then _MSG_PACKETS_INCOMPLETE is returned. If all
the context's blits are already complete, then _MSG_BLITS_COMPLETE is
returned, otherwise the callback will be called when the blits are complete.
If there is no callback specified, then all outstanding blits will be
complete upon return. Sync can be called once per context, and cannot be
called again until the callback is called.
 ***************************************************************************/
BERR_Code BGRC_Packet_SyncPackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext /* cannot be null */
);

/***************************************************************************
Returns the how much packet memory is available for contexts with packets
that need advancing, and the blit sync status of contexts that have synced
since the last call to this function.
 ***************************************************************************/
BERR_Code BGRC_Packet_GetContextStatus(
    BGRC_Handle hGrc,
    BGRC_Packet_ContextStatus *status_array, /* [out] context status array */
    size_t *size_out,                        /* [out] size of array*/
    size_t size_in,                          /* size of array */
    bool *pSecureModeSwitchNeeded
);

/***************************************************************************
Returns the M2MC device status.
 ***************************************************************************/
BERR_Code BGRC_Packet_GetStatus(
    BGRC_Handle hGrc,
    BGRC_Packet_Status *status              /* [out] status */
);

/***************************************************************************
Call by upper layer to clear checkpoint state if it is forcing checkpoint
 ***************************************************************************/
BERR_Code BGRC_Packet_CheckpointWatchdog(BGRC_Handle hGrc, BGRC_PacketContext_Handle hContext, bool force);

/***************************************************************************
 PACKET API - STATE DATA CONVERSION
 ***************************************************************************/
BERR_Code BGRC_Packet_ConvertPixelFormat(
    BM2MC_PACKET_PixelFormat *format,       /* [out] packet pixel format */
    BPXL_Format pxl_format                  /* BPXL pixel format */
);

/***************************************************************************/
BERR_Code BGRC_Packet_ConvertFilter(
    BM2MC_PACKET_FilterCoeffs *coeffs,      /* [out] packet filter coeffs */
    BGRC_FilterCoeffs filter,               /* BGRC filter enum */
    size_t src_size,                        /* source width or height */
    size_t out_size                         /* output width or height */
);

/***************************************************************************/
BERR_Code BGRC_Packet_ConvertColorMatrix(
    BM2MC_PACKET_ColorMatrix *matrix_out,   /* [out] packet colormatrix */
    const int32_t *matrix_in,               /* BGRC color matrix */
    size_t shift                            /* BGRC matrix element shift */
);

/***************************************************************************/

void BGRC_Packet_PrintStatus(BGRC_PacketContext_Handle hContext);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PACKET_H__ */

/* end of file */
