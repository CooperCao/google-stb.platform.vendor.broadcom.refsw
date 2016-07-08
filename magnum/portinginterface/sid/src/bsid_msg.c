/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bsid.h"
#include "bsid_priv.h"
#include "bsid_dbg.h"
#include "bsid_msg.h"
#include "bsid_platform.h"

#include "bchp_sid_arc.h"

BDBG_MODULE(BSID_MSG);
BDBG_FILE_MODULE(BSID_CMD);
BDBG_FILE_MODULE(BSID_INT);
BDBG_FILE_MODULE(BSID_CDB_ITB);
BDBG_FILE_MODULE(BSID_START_DECODE);
BDBG_FILE_MODULE(BSID_STOP_DECODE);
BDBG_FILE_MODULE(BSID_METADATA);

#define BSID_SIGNAL_MBX_RESPONSE_READY  BCHP_SID_ARC_CPU_INTGEN_SET_CPU_2_HST_MASK /* response ready isr */

#define BSID_ARC_INIT_MSG_ENABLE                                         false /* turn on arc dbg message on init command */
#define BSID_ARC_DECODE_MSG_ENABLE                                       false /* turn on arc dbg message on decode command */
#define BSID_ARC_GET_INFO_MSG_ENABLE                                     false /* turn on arc dbg message on getinfo command */
#define BSID_ARC_DECODE_SEGMENT_MSG_ENABLE                               false /* turn on arc dbg message on decode command */
#define BSID_ARC_OPENCHANNEL_MSG_ENABLE                                  false /* turn on arc dbg message on open channel */
#define BSID_ARC_CLOSECHANNEL_MSG_ENABLE                                 false /* turn on arc dbg message on close channel */
#define BSID_ARC_FLUSHCHANNEL_MSG_ENABLE                                 false /* turn on arc dbg message on flush channel */
#define BSID_ARC_SYNCCHANNEL_MSG_ENABLE                                  false /* turn on arc dbg message on sync channel */
#define BSID_ARC_STARTDECODE_MSG_ENABLE                                  false /* turn on arc dbg message on startdecode channel */
#define BSID_ARC_STOPDECODE_MSG_ENABLE                                   false /* turn on arc dbg message on stopdecode channel */
#define BSID_BITPERPIXEL_MAX                                                 9

/* max time (in milliseconds) to wait on mailbox answer before erroring out */
#define BSID_MAILBOX_MAX_WAITING_TIME                                     5000

#define BSID_ITB_ENTRY_TYPE_BASE_ADDR  0x20
#define BSID_ITB_ENTRY_TYPE_PTS_DTS    0x21
#define BSID_ITB_GET_ENTRY_TYPE(x)     (((x) >> 24) & 0xFF)

/* image format conversion table for image format between sid pi and sid arc */
/* NOTE: SID_RLE_SVCD and SID_RLE_NO_HEADER are not generated (not supported) */
static const BSID_ArcImageFormat ae_ImgFmtPi2Arc[BSID_ImageFormat_eLast] =
{
    SID_RLE_DVD,     /* BSID_ImageFormat_eRLE_Legacy_DVD */
    SID_RLE_HDDVD,   /* BSID_ImageFormat_eRLE_HD_DVD*/
    SID_RLE_BLURAY,  /* BSID_ImageFormat_eRLE_BD */
    SID_JPEG,        /* BSID_ImageFormat_eJPEG */
    SID_PNG,         /* BSID_ImageFormat_ePNG */
    SID_GIF,         /* BSID_ImageFormat_eGIF */
    SID_MPEG_IFRAME  /* BSID_ImageFormat_eMPEG_IFRAME */
};

static const BPXL_Format ae_ClutPxlFmtPi2Pxl[2][2] =
{
    /* 0: false if for little endian */
    {
        BPXL_eA8_R8_G8_B8,    /* SID_OUT_RGBA */
        BPXL_eA8_Y8_Cb8_Cr8   /* else */
    },

    /* 1: true if for big endian */
    {
        BPXL_eB8_G8_R8_A8,    /* SID_OUT_RGBA */
        BPXL_eCr8_Cb8_Y8_A8   /* else */
    }
};

typedef enum BSID_Pxl {
    BSID_Pxl_eInv = 0,
    BSID_Pxl_eACbCr,
    BSID_Pxl_eYCbYCr,
    BSID_Pxl_eP1,
    BSID_Pxl_eP2,
    BSID_Pxl_eP4,
    BSID_Pxl_eP8,
    BSID_Pxl_eARGB,
    BSID_Pxl_eAP
} BSID_Pxl;

/* conversion table from BSID_ArcOutFormat to BPXL format */
static const BPXL_Format ae_PxlFmtPi2Pxl[2][BSID_BITPERPIXEL_MAX] =
{
    /* 0: false if for little endian */
    {
        BPXL_INVALID,
#ifdef BSID_P_REVERSE_PXL_FMT
        BPXL_eA8_Cr8_Cb8_Y8,  /* OUT_YUVA, 8 */
#else
        BPXL_eA8_Y8_Cb8_Cr8,  /* OUT_YUVA, 8 */
#endif
        BPXL_eY18_Cr8_Y08_Cb8,
        BPXL_eP1,
        BPXL_eP2,
        BPXL_eP4,
        BPXL_eP8,
#ifdef BSID_P_REVERSE_PXL_FMT
        BPXL_eA8_B8_G8_R8,  /* OUT_RGBA, 8 */
#else
        BPXL_eA8_R8_G8_B8,  /* OUT_RGBA, 8 */
#endif
        BPXL_eA8_P8
    }

    /* 1: true if for big endian */
    ,
    {
        BPXL_INVALID,
#ifdef BSID_P_REVERSE_PXL_FMT
        BPXL_eY8_Cb8_Cr8_A8,  /* OUT_YUVA, 8 */
#else
        BPXL_eCr8_Cb8_Y8_A8,  /* OUT_YUVA, 8 */
#endif
        BPXL_eCb8_Y08_Cr8_Y18,
        BPXL_eP1,
        BPXL_eP2,
        BPXL_eP4,
        BPXL_eP8,
#ifdef BSID_P_REVERSE_PXL_FMT
        BPXL_eR8_G8_B8_A8,  /* OUT_RGBA, 8 */
#else
        BPXL_eB8_G8_R8_A8,  /* OUT_RGBA, 8 */
#endif
        BPXL_eA8_P8
    }
};

static const uint8_t ae_PxlFmtArc2Pi[SID_OUT_LAST][BSID_BITPERPIXEL_MAX] =
{
    {
        BSID_Pxl_eInv,  /* OUT_YUVA, 0 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 1 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 2 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 3 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 4 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 5 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 6 */
        BSID_Pxl_eInv,  /* OUT_YUVA, 7 */
        BSID_Pxl_eACbCr /* OUT_YUVA, 8 */
    },
    {
        BSID_Pxl_eInv,   /* OUT_YUYV, 0 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 1 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 2 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 3 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 4 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 5 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 6 */
        BSID_Pxl_eInv,   /* OUT_YUYV, 7 */
        BSID_Pxl_eYCbYCr /* OUT_YUVA, 8 */
    },
    {
        BSID_Pxl_eInv, /* OUT_GREY, 0 */
        BSID_Pxl_eP1,  /* OUT_GREY, 1 */
        BSID_Pxl_eP2,  /* OUT_GREY, 2 */
        BSID_Pxl_eInv, /* OUT_GREY, 3 */
        BSID_Pxl_eP4,  /* OUT_GREY, 4 */
        BSID_Pxl_eInv, /* OUT_GREY, 5 */
        BSID_Pxl_eInv, /* OUT_GREY, 6 */
        BSID_Pxl_eInv, /* OUT_GREY, 7 */
        BSID_Pxl_eP8   /* OUT_GREY, 8 */
    },
    {
        BSID_Pxl_eInv, /* OUT_PALETTE, 0 */
        BSID_Pxl_eP1,  /* OUT_PALETTE, 1 */
        BSID_Pxl_eP2,  /* OUT_PALETTE, 2 */
        BSID_Pxl_eInv, /* OUT_PALETTE, 3 */
        BSID_Pxl_eP4,  /* OUT_PALETTE, 4 */
        BSID_Pxl_eInv, /* OUT_PALETTE, 5 */
        BSID_Pxl_eInv, /* OUT_PALETTE, 6 */
        BSID_Pxl_eInv, /* OUT_PALETTE, 7 */
        BSID_Pxl_eP8   /* OUT_PALETTE, 8 */
    },
    {
        BSID_Pxl_eInv, /* OUT_RGBA, 0 */
        BSID_Pxl_eInv, /* OUT_RGBA, 1 */
        BSID_Pxl_eInv, /* OUT_RGBA, 2 */
        BSID_Pxl_eInv, /* OUT_RGBA, 3 */
        BSID_Pxl_eInv, /* OUT_RGBA, 4 */
        BSID_Pxl_eInv, /* OUT_RGBA, 5 */
        BSID_Pxl_eInv, /* OUT_RGBA, 6 */
        BSID_Pxl_eInv, /* OUT_RGBA, 7 */
        BSID_Pxl_eARGB /* OUT_RGBA, 8 */
    },
    {
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 0 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 1 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 2 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 3 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 4 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 5 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 6 */
        BSID_Pxl_eInv, /* OUT_GREY_ALPHA, 7 */
        BSID_Pxl_eAP   /* OUT_GREY_ALPHA, 8 */
    }
};


/**************************************************************************/

static BERR_Code SendCommand(
    BSID_Handle hSid,
    const BMMA_Block_Handle hCmdMbxBlock,
    const void *pCmd,
    uint32_t    ui32_CmdSize,
    const BMMA_Block_Handle hRspMbxBlock,
    void       *pRsp,
    uint32_t    ui32_RspSize,
    bool        waitForRsp);

static void UnlockRaveBlocks(BSID_ChannelHandle hSidCh);
static BERR_Code ReadItb4ByteAt_isr(BSID_ChannelHandle hSidCh, uint32_t ItbRead, uint32_t *WordRead, uint32_t *newReadPtr);
static bool GetNextPicItbInfo_isr(BSID_Handle hSid, BSID_ChannelHandle hSidCh, uint32_t *BaseAddress, uint32_t *Pts);
static void PrepareNextUnifiedPicture_isr(BSID_ChannelHandle  hSidCh, BXDM_Picture *pUnifiedPicture, uint32_t displayReadIndex);
static BSID_LinearBuffer * LookUp_Buffer_isr(BSID_ChannelHandle hSidCh, uint32_t offset);
static uint32_t NewMetadataEntry(BSID_ChannelHandle hSidCh, BSID_DataQueueEntry *psDataQueueEntry, void *pImageInfo);
static void FreeMetadataEntry_isr(BSID_ChannelHandle hSidCh, uint32_t uiMetadataIndex);

/******************************************************************************
* Function name: BSID_P_SetMailboxInfo
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_SetMailboxInfo(BSID_Handle hSid)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t *pui32_CachedAddr = NULL;

    BDBG_ENTER(BSID_P_SetMailboxInfo);

    hSid->sMailbox.hMailboxEvent = NULL;
    pui32_CachedAddr = (uint32_t *)hSid->sFwHwConfig.sMbxMemory.sCmd.pv_CachedAddr;
    hSid->sMailbox.pv_CmdQCachedAddr = pui32_CachedAddr;
    pui32_CachedAddr = (uint32_t *)hSid->sFwHwConfig.sMbxMemory.sRsp.pv_CachedAddr;
    hSid->sMailbox.pv_RspQCachedAddr = pui32_CachedAddr;
    hSid->sMailbox.ui32_CmdQPhysAddr  = hSid->sFwHwConfig.sMbxMemory.sCmd.ui32_PhysAddr;
    hSid->sMailbox.ui32_RspQPhysAddr  = hSid->sFwHwConfig.sMbxMemory.sRsp.ui32_PhysAddr;
    hSid->sMailbox.hCmdMbxBlock = hSid->sFwHwConfig.sMbxMemory.sCmd.hBlock;
    hSid->sMailbox.hRspMbxBlock = hSid->sFwHwConfig.sMbxMemory.sRsp.hBlock;

    BDBG_LEAVE(BSID_P_SetMailboxInfo);

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_P_Dispatch_isr
*
* Comments:
* Main interrupt handler for BSID interrupt
*
******************************************************************************/
void BSID_P_Dispatch_isr(void *pv_Param1, int iParam2)
{
    BSID_Handle hSid = (BSID_Handle)pv_Param1;
    uint32_t    ui32_IrqSource = BREG_Read32(hSid->hReg, BCHP_SID_ARC_CPU_INTGEN_CLR);
    uint32_t    ui32_ChannelNumber = 0;
    BSID_EventInfo s_EventInfo;

    BSTD_UNUSED(iParam2);

    BDBG_MODULE_MSG(BSID_INT, ("Irq Source: 0x%x", ui32_IrqSource));

   /* get out when the channel irq all served */
    while (ui32_IrqSource & BSID_INT_CH_ENABLE)
    {
        BDBG_MODULE_MSG(BSID_INT, ("IRQ Source = %x, Channel = %x", ui32_IrqSource, ui32_ChannelNumber));
        /* test each channel for irq */
        if (ui32_IrqSource & (1<<ui32_ChannelNumber))
        {
            BSID_ChannelHandle      hSidCh = hSid->ahChannel[ui32_ChannelNumber];
            BSID_CommandQueueHeader *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sRelQueue.sBuf.pv_CachedAddr;
            BMMA_Block_Handle       hRelQBlock = hSidCh->sRelQueue.sBuf.hBlock;
            BSID_RelQueueEntry      *psQueueEntry;
            BSID_CallbackFunc       pvCallbackFunc_isr;
            uint32_t                ui32_CallbackEvents;
            void                    *pv_CallbackData;
            BSID_NotificationEvent  eEvent = 0x0;
            bool                    ChannelOperationsCompleted = false;

            BDBG_MODULE_MSG(BSID_INT, ("Processing Channel Num: 0x%x, reqQ = %p", ui32_ChannelNumber, hSidCh->sReqQueue.sBuf.pv_CachedAddr));

            BMMA_FlushCache(hRelQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));

            /* NOTE: The queue can be empty during a "wait for data" request */
            if ((psQueueHeader->ui32_ReadIndex == psQueueHeader->ui32_WriteIndex) && !psQueueHeader->ui32_WaitForData)
            {
               /* this indicates something went wrong in the FW */
               BDBG_WRN(("Channel[%d]: IRQ but channel release Q is empty!", ui32_ChannelNumber));
            }

            while (psQueueHeader->ui32_ReadIndex != psQueueHeader->ui32_WriteIndex)
            {
                /* access an entry in the release queue */
                psQueueEntry = (BSID_RelQueueEntry *)((uint8_t *)psQueueHeader + sizeof(BSID_CommandQueueHeader) + (psQueueHeader->ui32_ReadIndex * sizeof(BSID_RelQueueEntry)));
                BMMA_FlushCache(hRelQBlock, (void *)psQueueEntry, sizeof(BSID_RelQueueEntry));

                BDBG_MODULE_MSG(BSID_INT, ("Release Q: header %x channel %d decode/getinfo done rd %d wr %d or waitfordata %d",
                psQueueEntry->rspHeader.header,
                ui32_ChannelNumber, psQueueHeader->ui32_ReadIndex, psQueueHeader->ui32_WriteIndex,
                (hSidCh->e_ChannelType == BSID_ChannelType_eStill) ? psQueueHeader->ui32_WaitForData : 0));

                /* decode and decode segment events */
                if (psQueueEntry->rspHeader.header == BSID_MSG_HDR_DECODE || psQueueEntry->rspHeader.header == BSID_MSG_HDR_DECODESEGMENT)
                {
                    eEvent = (psQueueEntry->rspHeader.errCode == 0) ? BSID_NotificationEvent_eDecodeDone : BSID_NotificationEvent_eDecodeError;

                    BDBG_MODULE_MSG(BSID_INT, ("servicing decode events seq %d crc 0x%x status %s", psQueueEntry->rspHeader.sequence, psQueueEntry->checkSumValue,
                    (eEvent == BSID_NotificationEvent_eDecodeDone) ? "DecodeDone" : "DecodeError"));

                    ChannelOperationsCompleted = psQueueEntry->operationCompleted;
                }

                /* get info events */
                if (psQueueEntry->rspHeader.header == BSID_MSG_HDR_GETSTREAMINFO)
                {
                    BSID_P_MetadataEntry *pMetadataEntry = &hSidCh->pMetadata[psQueueEntry->uiMetadataIndex];
                    /* where the FW has deposited the image header ... */
                    /* NOTE: This corresponds to an entry in the data queue */
                    SID_ImageHeader *pFWImageHeader = (SID_ImageHeader *)pMetadataEntry->pvStreamInfoFWAddr;
                    /* where the application wants the header to go ... */
                    BSID_ImageHeader *pUserImageHeaderBuffer = (BSID_ImageHeader *)pMetadataEntry->pStreamInfoDestAddr;
                    /* NOTE: This will error check the index and give an error if an attempt is made to free an invalid entry */
                    FreeMetadataEntry_isr(hSidCh, psQueueEntry->uiMetadataIndex);
                    /* read the header from the FW */
                    BMMA_FlushCache(hSidCh->sDataQueue.sBuf.hBlock, (void *)pFWImageHeader, sizeof(SID_ImageHeader));
                    BKNI_Memcpy((void *)pUserImageHeaderBuffer, (void *)pFWImageHeader, sizeof(BSID_ImageHeader));
                     /* convert enum fields from FW constants to magnum constants */
                    pUserImageHeaderBuffer->e_PixelFormat = ae_PxlFmtPi2Pxl[hSid->sFwHwConfig.b_EndianessSwap][ae_PxlFmtArc2Pi[pFWImageHeader->ePixelFormat][pFWImageHeader->ui32_BitPerPixel]];
                    pUserImageHeaderBuffer->e_ClutPixelFormat = ae_ClutPxlFmtPi2Pxl[hSid->sFwHwConfig.b_EndianessSwap][(pFWImageHeader->eClutPixelFormat == SID_OUT_RGBA) ? 0 : 1];

                    if (pUserImageHeaderBuffer->e_PixelFormat == BPXL_INVALID)
                    {
                        BDBG_WRN(("BSID_P_Dispatch_isr: ERROR pixel format unknown"));
                    }

                    eEvent = (psQueueEntry->rspHeader.errCode == 0) ? BSID_NotificationEvent_eGetInfoDone : BSID_NotificationEvent_eGetInfoError;

                    BDBG_MODULE_MSG(BSID_INT, ("servicing getinfo events seq %d status %s", psQueueEntry->rspHeader.sequence,
                    (eEvent == BSID_NotificationEvent_eGetInfoDone) ? "GetInfoDone" : "GetInfoError"));

                    ChannelOperationsCompleted = psQueueEntry->operationCompleted;
                }
                /* FIXME: Add generation of flush events in response to Flush Channel Command?? Only for Still, and currently not used */

                if (psQueueEntry->rspHeader.header == BSID_MSG_HDR_SYNCCHANNEL)
                {
                    BDBG_MODULE_MSG(BSID_INT, ("servicing sync channel event"));
                    BKNI_SetEvent(hSidCh->hSyncEvent);
                }

                if (psQueueEntry->rspHeader.header == BSID_MSG_HDR_DECODE ||
                    psQueueEntry->rspHeader.header == BSID_MSG_HDR_DECODESEGMENT ||
                    psQueueEntry->rspHeader.header == BSID_MSG_HDR_GETSTREAMINFO)
                {
                    if (hSidCh->bAbortInitiated)
                        BKNI_SetEvent(hSidCh->hAbortedEvent);

                    pvCallbackFunc_isr = hSidCh->sChSettings.u_ChannelSpecific.still.p_CallbackFunc_isr;
                    ui32_CallbackEvents = hSidCh->sChSettings.u_ChannelSpecific.still.ui32_CallbackEvents;
                    pv_CallbackData = hSidCh->sChSettings.u_ChannelSpecific.still.pv_CallbackData;

                    /* notify the caller via callback */
                    if ((pvCallbackFunc_isr != NULL) && ((ui32_CallbackEvents & eEvent) == eEvent))
                    {
                        BDBG_MODULE_MSG(BSID_INT, ("Notifying caller via callback for event %d", eEvent));
                        s_EventInfo.e_EventType = eEvent;
                        s_EventInfo.ui32_imageCheckSum = (eEvent == BSID_NotificationEvent_eDecodeDone) ? psQueueEntry->checkSumValue : 0;
                        s_EventInfo.ui32_DecodeSequenceID = psQueueEntry->rspHeader.sequence;
                        if (psQueueEntry->rspHeader.errCode != ERR_CODE_SUCCESS)
                           /* convert FW error code to SID PI error code */
                           s_EventInfo.ui32_errorCode = BERR_MAKE_CODE(BERR_SID_ID, psQueueEntry->rspHeader.errCode);
                        else
                           s_EventInfo.ui32_errorCode = BERR_SUCCESS;
                        (pvCallbackFunc_isr)(hSidCh, &s_EventInfo, pv_CallbackData);
                    }
                }

                /* free release Q entry (the response) */
                BSID_INCR_INDEX(psQueueHeader->ui32_ReadIndex, hSidCh->ui32_QueueTrueDepth);
                BMMA_FlushCache(hRelQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
            }

            if (psQueueHeader->ui32_WaitForData)
            {
                BDBG_MODULE_MSG(BSID_INT, ("Waiting for data (flush pending = %d, abort initiated = %d)", hSidCh->b_FlushPending, hSidCh->bAbortInitiated));
                /* if wait for more data occurs during pending abort, we ignore it */
                if ((hSidCh->e_ChannelType == BSID_ChannelType_eStill) && !hSidCh->bAbortInitiated)
                {
                    if (hSidCh->b_FlushPending == false)
                    {
                        pvCallbackFunc_isr = hSidCh->sChSettings.u_ChannelSpecific.still.p_CallbackFunc_isr;
                        ui32_CallbackEvents = hSidCh->sChSettings.u_ChannelSpecific.still.ui32_CallbackEvents;
                        pv_CallbackData = hSidCh->sChSettings.u_ChannelSpecific.still.pv_CallbackData;
                        if ((pvCallbackFunc_isr != NULL) &&
                            ((ui32_CallbackEvents & BSID_NotificationEvent_eWaitingForMoreInputData) \
                              == BSID_NotificationEvent_eWaitingForMoreInputData))
                        {
                            BDBG_MODULE_MSG(BSID_INT, ("wait for data callback"));

                            s_EventInfo.e_EventType = BSID_NotificationEvent_eWaitingForMoreInputData;
                            s_EventInfo.ui32_imageCheckSum = 0;
                            s_EventInfo.ui32_DecodeSequenceID = 0;

                            (pvCallbackFunc_isr)(
                                hSidCh,
                                &s_EventInfo,
                                pv_CallbackData
                             );
                        }
                    }
                    else
                    {
                       /* "Wait for more data" request happened during a flush, so we need
                          to request an abort
                          So, turn off the "flush pending" flag to indicate this to the flush API */
                       hSidCh->b_FlushPending = false;
                    }
                } /* end: still channel and not abort */
            } /* end: wait for data */

            /* clear channel request */
            {
                BSID_CommandQueueHeader *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
                BMMA_Block_Handle hReqQBlock = hSidCh->sReqQueue.sBuf.hBlock;

                BDBG_MODULE_MSG(BSID_INT, ("Clearing channel request for Channel %x", ui32_ChannelNumber));

                BMMA_FlushCache(hReqQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
                if (psQueueHeader->ui32_ReadIndex == psQueueHeader->ui32_WriteIndex)
                {
                    /* this indicates that all commands were read by the FW, so we need to clear the request */
                    uint32_t ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
                    ui32_hst2cpu &= ~(1 << ui32_ChannelNumber);
                    BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, ui32_hst2cpu);
                }
                /* otherwise, there are still commands to be processed by FW */
            }

            /* clear current channel request */
            BREG_Write32(hSid->hReg, BCHP_SID_ARC_CPU_INTGEN_CLR, (1 << ui32_ChannelNumber));

            /*
             * If the last operations serviced were all completed, change channel state to ready
             * OperationCompleted status is set by the FW based. Typically GetInfo or DecodeImage
             * gets executed in one pass so the operation is completed always. DecodeSegment are
             * executed in more than one pass. While they are executed the completed status is
             * set to zero to indicate that they are not done. Only the very last operation will
             * report completed state set to true. NOTE that DecodeSegment belonging to the same
             * decode sections must not be interrupted by GetInfo or DecodeImage operations.
             */
            if (ChannelOperationsCompleted == true)
            {
                hSidCh->e_ChannelState = BSID_ChannelState_eReady;
            }
        }

        /* get ready to test next channel */
        BSID_INCR_INDEX(ui32_ChannelNumber, BSID_MAX_CHANNELS);

        /* check if we have received a new irq */
        ui32_IrqSource = BREG_Read32(hSid->hReg, BCHP_SID_ARC_CPU_INTGEN_CLR);
        BDBG_MODULE_MSG(BSID_INT, ("New Irq Source: 0x%x", ui32_IrqSource));
    } /* end: while channel irq */

    /* test mbx irq */
    if (ui32_IrqSource & BSID_SIGNAL_MBX_RESPONSE_READY)
    {
        BDBG_MODULE_MSG(BSID_INT, ("Clearing mailbox"));
        BKNI_SetEvent(hSid->sMailbox.hMailboxEvent);
        BREG_Write32(hSid->hReg, BCHP_SID_ARC_CPU_INTGEN_CLR, (ui32_IrqSource & BSID_SIGNAL_MBX_RESPONSE_READY));
        ui32_IrqSource &= ~BSID_SIGNAL_MBX_RESPONSE_READY;
    }
    if (ui32_IrqSource)
        BDBG_WRN(("BSID_P_Dispatch_isr: not all sources serviced: IRQ = %x", ui32_IrqSource));
}

/******************************************************************************
* Function name: BSID_P_SendCmdInit
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdInit(BSID_Handle hSid)
{
   BERR_Code      retCode     = BERR_SUCCESS;
   BSID_Cmd_Init  *ps_InitCmd = (BSID_Cmd_Init *)hSid->sMailbox.pv_CmdQCachedAddr;
   BSID_Rsp_Init  *ps_InitRsp = (BSID_Rsp_Init *)hSid->sMailbox.pv_RspQCachedAddr;
   BMMA_Block_Handle hCmdMbxBlock = hSid->sMailbox.hCmdMbxBlock;
   BMMA_Block_Handle hRspMbxBlock = hSid->sMailbox.hRspMbxBlock;
   bool needAck = true;

   BDBG_ENTER(BSID_P_SendCmdInit);

   /* prepare cmd body */
   ps_InitCmd->cmdHdr.header    = BSID_MSG_HDR_INIT;
   ps_InitCmd->cmdHdr.sequence  = 0;
   ps_InitCmd->cmdHdr.needAck   = needAck;

   ps_InitCmd->frequency        = BSID_P_ARC_CPU_FREQUENCY;
   ps_InitCmd->baudRate         = BSID_P_ARC_UART_BAUDRATE;
   ps_InitCmd->mbxCmdBufAddr    = hSid->sMailbox.ui32_CmdQPhysAddr;
   ps_InitCmd->mbxRspBufAddr    = hSid->sMailbox.ui32_RspQPhysAddr;
   ps_InitCmd->mbxCmdRspBufSize = hSid->sFwHwConfig.sMbxMemory.sCmd.ui32_Size;
   ps_InitCmd->memMode          = hSid->sFwHwConfig.eMemoryMode;
   ps_InitCmd->memBase          = hSid->sFwHwConfig.sDataMemory.ui32_PhysAddr;
   ps_InitCmd->memSize          = hSid->sFwHwConfig.sDataMemory.ui32_Size;
   ps_InitCmd->msBufBase        = 0x00000000; /* provided decode time */
   ps_InitCmd->msBufSize        = 0x00000000; /* provided decode time */
   ps_InitCmd->dmaInfoBufAddr   = hSid->sFwHwConfig.sInpDmaMemory.ui32_PhysAddr;
   ps_InitCmd->dmaInfoBufSize   = hSid->sFwHwConfig.sInpDmaMemory.ui32_Size;
   ps_InitCmd->endianessSwap    = hSid->sFwHwConfig.b_EndianessSwap;
   ps_InitCmd->jpegFiltMode     = hSid->sFwHwConfig.ui16_JPEGHorizAndVerFilt;
   ps_InitCmd->alphaValue       = hSid->sFwHwConfig.ui8_AlphaValue;
   ps_InitCmd->dbgMsgOn         = BSID_ARC_INIT_MSG_ENABLE;

   BDBG_MODULE_MSG(BSID_CMD, ("[MBOX] cmd @%p rsp @%p => init sent", (void *)ps_InitCmd, (void *)ps_InitRsp));
   BDBG_MODULE_MSG(BSID_CMD, ("frequency     = 0x%x", ps_InitCmd->frequency));
   BDBG_MODULE_MSG(BSID_CMD, ("baudRate      = 0x%x", ps_InitCmd->baudRate));
   BDBG_MODULE_MSG(BSID_CMD, ("cmd buf       = 0x%x", ps_InitCmd->mbxCmdBufAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("rsp buf       = 0x%x", ps_InitCmd->mbxRspBufAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("cmd rsp size  = 0x%x", ps_InitCmd->mbxCmdRspBufSize));
   BDBG_MODULE_MSG(BSID_CMD, ("memMode       = 0x%x", ps_InitCmd->memMode));
   BDBG_MODULE_MSG(BSID_CMD, ("memBase       = 0x%x", ps_InitCmd->memBase));
   BDBG_MODULE_MSG(BSID_CMD, ("memSize       = 0x%x", ps_InitCmd->memSize));
   BDBG_MODULE_MSG(BSID_CMD, ("endianessSwap = 0x%x", ps_InitCmd->endianessSwap));
   BDBG_MODULE_MSG(BSID_CMD, ("jpegFiltMode  = 0x%x", ps_InitCmd->jpegFiltMode));
   BDBG_MODULE_MSG(BSID_CMD, ("alphaValue    = 0x%x", ps_InitCmd->alphaValue));


   /* send cmd and wait for response. */
   retCode = SendCommand(hSid, hCmdMbxBlock, (void *)ps_InitCmd, sizeof(BSID_Cmd_Init), hRspMbxBlock, (void *)ps_InitRsp, sizeof(BSID_Rsp_Init), needAck);
   if (retCode != BERR_SUCCESS)
   {
       return BERR_TRACE(retCode);
   }

   /* FIXME: extract FW and HW revision information and store (in context) for status report */
   /* Also, need to have PNG widths in response since they are FW memory dependent */

   BDBG_MSG(("cmd init done"));

   BDBG_LEAVE(BSID_P_SendCmdInit);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdOpenChannel
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdOpenChannel(BSID_ChannelHandle hSidCh)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSID_Handle hSid = hSidCh->hSid;
   BSID_Cmd_OpenChannel *ps_OpenChannelCmd = (BSID_Cmd_OpenChannel *)hSid->sMailbox.pv_CmdQCachedAddr;
   BSID_Rsp_OpenChannel *ps_OpenChannelRsp = (BSID_Rsp_OpenChannel *)hSid->sMailbox.pv_RspQCachedAddr;
   BMMA_Block_Handle hCmdMbxBlock = hSid->sMailbox.hCmdMbxBlock;
   BMMA_Block_Handle hRspMbxBlock = hSid->sMailbox.hRspMbxBlock;
   bool needAck = true;

   BDBG_ENTER(BSID_P_SendCmdOpenChannel);

   /* prepare cmd body */
   ps_OpenChannelCmd->cmdHdr.header   = BSID_MSG_HDR_OPENCHANNEL;
   ps_OpenChannelCmd->cmdHdr.sequence = hSidCh->ui32_SequenceNumber++;
   ps_OpenChannelCmd->cmdHdr.needAck  = needAck;

   ps_OpenChannelCmd->channelID       = hSidCh->ui32_ChannelID;
   ps_OpenChannelCmd->channelNumber   = hSidCh->ui32_ChannelNum;
   ps_OpenChannelCmd->priority        = hSidCh->sChSettings.ui32_ChannelPriority;
   ps_OpenChannelCmd->reqQueueAddr    = hSidCh->sReqQueue.sBuf.ui32_PhysAddr;
   ps_OpenChannelCmd->relQueueAddr    = hSidCh->sRelQueue.sBuf.ui32_PhysAddr;
   ps_OpenChannelCmd->queueDepth      = hSidCh->ui32_QueueTrueDepth;
   ps_OpenChannelCmd->dbgMsgOn        = BSID_ARC_OPENCHANNEL_MSG_ENABLE;

   BDBG_MODULE_MSG(BSID_CMD, ("[MBOX] cmd @%p rsp @%p => open channel sent", (void *)ps_OpenChannelCmd, (void *)ps_OpenChannelRsp));
   BDBG_MODULE_MSG(BSID_CMD, ("channelID    = 0x%x", ps_OpenChannelCmd->channelID));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum   = 0x%x", ps_OpenChannelCmd->channelNumber));
   BDBG_MODULE_MSG(BSID_CMD, ("priority     = 0x%x", ps_OpenChannelCmd->priority));
   BDBG_MODULE_MSG(BSID_CMD, ("reqQueueAddr = 0x%x", ps_OpenChannelCmd->reqQueueAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("relQueueAddr = 0x%x", ps_OpenChannelCmd->relQueueAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("queueDepth   = 0x%x", ps_OpenChannelCmd->queueDepth));

   /* send command and wait for response. */
   retCode = SendCommand(hSid, hCmdMbxBlock, (void *)ps_OpenChannelCmd, sizeof(BSID_Cmd_OpenChannel), hRspMbxBlock, (void *)ps_OpenChannelRsp, sizeof(BSID_Rsp_OpenChannel), needAck);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("SendCommand failed"));
      return BERR_TRACE(retCode);
   }

   BDBG_MSG(("rsp open channel done"));

   BDBG_LEAVE(BSID_P_SendCmdOpenChannel);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdCloseChannel
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdCloseChannel(BSID_ChannelHandle hSidCh)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSID_Handle hSid = hSidCh->hSid;
   BSID_Cmd_CloseChannel *ps_CloseChannelCmd = (BSID_Cmd_CloseChannel *)hSid->sMailbox.pv_CmdQCachedAddr;
   BSID_Rsp_CloseChannel *ps_CloseChannelRsp = (BSID_Rsp_CloseChannel *)hSid->sMailbox.pv_RspQCachedAddr;
   BMMA_Block_Handle hCmdMbxBlock = hSid->sMailbox.hCmdMbxBlock;
   BMMA_Block_Handle hRspMbxBlock = hSid->sMailbox.hRspMbxBlock;
   bool needAck = true;

   BDBG_ENTER(BSID_P_SendCmdCloseChannel);

   /* prepare cmd body */
   ps_CloseChannelCmd->cmdHdr.header    = BSID_MSG_HDR_CLOSECHANNEL;
   ps_CloseChannelCmd->cmdHdr.sequence  = hSidCh->ui32_SequenceNumber++;
   ps_CloseChannelCmd->cmdHdr.needAck   = needAck;

   ps_CloseChannelCmd->channelID        = hSidCh->ui32_ChannelID;
   ps_CloseChannelCmd->channelNumber    = hSidCh->ui32_ChannelNum;
   ps_CloseChannelCmd->dbgMsgOn         = BSID_ARC_CLOSECHANNEL_MSG_ENABLE;

   BDBG_MODULE_MSG(BSID_CMD, ("[MBOX] cmd @%p rsp @%p => close channel sent", (void *)ps_CloseChannelCmd, (void *)ps_CloseChannelRsp));
   BDBG_MODULE_MSG(BSID_CMD, ("channelID         = 0x%x", ps_CloseChannelCmd->channelID));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum        = 0x%x", ps_CloseChannelCmd->channelNumber));

   /* send command and wait for response. */
   retCode = SendCommand(hSid, hCmdMbxBlock, (void *)ps_CloseChannelCmd, sizeof(BSID_Cmd_CloseChannel), hRspMbxBlock, (void *)ps_CloseChannelRsp, sizeof(BSID_Rsp_CloseChannel), needAck);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("SendCommand failed"));
      return BERR_TRACE(retCode);
   }

   BDBG_MSG(("rsp close channel done"));

   BDBG_LEAVE(BSID_P_SendCmdCloseChannel);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdFlushChannel
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdFlushChannel(BSID_ChannelHandle  hSidCh)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSID_Handle hSid = hSidCh->hSid;
   BSID_Cmd_FlushChannel *ps_FlushChannelCmd = (BSID_Cmd_FlushChannel *)hSid->sMailbox.pv_CmdQCachedAddr;
   BSID_Rsp_FlushChannel *ps_FlushChannelRsp = (BSID_Rsp_FlushChannel *)hSid->sMailbox.pv_RspQCachedAddr;
   BSID_CommandQueueHeader *psReqQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BSID_CommandQueueHeader *psRelQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sRelQueue.sBuf.pv_CachedAddr;
   BSID_CommandQueueHeader *psDataQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sDataQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle hCmdMbxBlock = hSid->sMailbox.hCmdMbxBlock;
   BMMA_Block_Handle hRspMbxBlock = hSid->sMailbox.hRspMbxBlock;
   BMMA_Block_Handle hReqQBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BMMA_Block_Handle hRelQBlock = hSidCh->sRelQueue.sBuf.hBlock;
   BMMA_Block_Handle hDataQBlock = hSidCh->sDataQueue.sBuf.hBlock;
   bool needAck = true;

   BDBG_ENTER(BSID_P_SendCmdFlushChannel);

   /* prepare cmd body */
   ps_FlushChannelCmd->cmdHdr.header    = BSID_MSG_HDR_FLUSHCHANNEL;
   ps_FlushChannelCmd->cmdHdr.sequence  = hSidCh->ui32_SequenceNumber++;
   ps_FlushChannelCmd->cmdHdr.needAck   = needAck;

   ps_FlushChannelCmd->channelID        = hSidCh->ui32_ChannelID;
   ps_FlushChannelCmd->channelNumber    = hSidCh->ui32_ChannelNum;
   ps_FlushChannelCmd->dbgMsgOn         = BSID_ARC_FLUSHCHANNEL_MSG_ENABLE;

   BDBG_MODULE_MSG(BSID_CMD, ("[MBOX] cmd @%p rsp @%p => flush channel sent", (void *)ps_FlushChannelCmd, (void *)ps_FlushChannelRsp));
   BDBG_MODULE_MSG(BSID_CMD, ("channelID         = 0x%x", ps_FlushChannelCmd->channelID));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum        = 0x%x", ps_FlushChannelCmd->channelNumber));
   BDBG_MODULE_MSG(BSID_CMD, ("request Q @ %p", (void *)psReqQueueHeader));
   BDBG_MODULE_MSG(BSID_CMD, ("release Q @ %p", (void *)psRelQueueHeader));
   BDBG_MODULE_MSG(BSID_CMD, ("data Q @ %p", (void *)psDataQueueHeader));

   /* send command and wait for response. */
   retCode = SendCommand(hSid, hCmdMbxBlock, (void *)ps_FlushChannelCmd, sizeof(BSID_Cmd_FlushChannel), hRspMbxBlock, (void *)ps_FlushChannelRsp, sizeof(BSID_Rsp_FlushChannel), needAck);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("SendCommand failed"));
      return BERR_TRACE(retCode);
   }

   BMMA_FlushCache(hReqQBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
   /* reset queue rd/wr index */

   /* sanity check, queues should be empty */
   if (psReqQueueHeader->ui32_ReadIndex != psReqQueueHeader->ui32_WriteIndex)
   {
       BDBG_WRN(("BSID_P_FlushChannel: reqQ rd and wr index are not the same"));
   }

   /* reset to beginning of the request queue */
   psReqQueueHeader->ui32_ReadIndex = psReqQueueHeader->ui32_WriteIndex = 0;

   /* reset to beginning of the release queue */
   psRelQueueHeader->ui32_ReadIndex = psRelQueueHeader->ui32_WriteIndex = 0;

   /* reset to beginning of the data queue */
   psDataQueueHeader->ui32_ReadIndex = psDataQueueHeader->ui32_WriteIndex = 0;

   BMMA_FlushCache(hReqQBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
   BMMA_FlushCache(hRelQBlock, (void *)psRelQueueHeader, sizeof(BSID_CommandQueueHeader));
   BMMA_FlushCache(hDataQBlock, (void *)psDataQueueHeader, sizeof(BSID_CommandQueueHeader));

   BDBG_MSG(("rsp flush channel done"));

   BDBG_LEAVE(BSID_P_SendCmdFlushChannel);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdDecodeStillImage
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdDecodeStillImage(
    BSID_ChannelHandle hSidCh,
    const BSID_DecodeImage *ps_DecodeImage)
{
   BERR_Code        retCode       = BERR_SUCCESS;
   BSID_Handle      hSid          = hSidCh->hSid;
   BSID_CommandQueueHeader *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle hQueueHeaderBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BSID_ReqQueueEntry *psQueueEntry = NULL;
   BSID_Cmd_Decode *ps_DecodeCmd = NULL;
   const BSID_ImageBuffer *ps_ImageBuffer  = &ps_DecodeImage->s_ImageBuffer;
   bool             needAck = false;
   unsigned char i;

   BDBG_ENTER(BSID_P_SendCmdDecodeStillImage);

   /* prepare request queue elem */
   /* FIXME: New function GetRequestEntry() => returns pointer to queue entry based on teh write Index */
   BMMA_FlushCache(hQueueHeaderBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
   psQueueEntry = (BSID_ReqQueueEntry *)((uint8_t *)psQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                  (psQueueHeader->ui32_WriteIndex * sizeof(BSID_ReqQueueEntry)));
   BKNI_Memset((void *)psQueueEntry, 0x00, sizeof(BSID_ReqQueueEntry));
   BMMA_FlushCache(hQueueHeaderBlock, (void *)psQueueEntry, sizeof(BSID_ReqQueueEntry));
   ps_DecodeCmd   = (BSID_Cmd_Decode *)psQueueEntry;

   /* prepare cmd body */
   ps_DecodeCmd->cmdHdr.header   = BSID_MSG_HDR_DECODE;
   ps_DecodeCmd->cmdHdr.needAck  = needAck;
   ps_DecodeCmd->cmdHdr.sequence = ps_DecodeImage->ui32_DecodeSequenceID;

   ps_DecodeCmd->imgFormat         = ae_ImgFmtPi2Arc[ps_DecodeImage->e_ImageFormat];
   ps_DecodeCmd->imgWidth          = ps_ImageBuffer->ui32_Width;
   ps_DecodeCmd->imgHeight         = ps_ImageBuffer->ui32_Height;
   ps_DecodeCmd->imgPitch          = ps_ImageBuffer->ui32_Pitch;
   ps_DecodeCmd->imgBpp            = BPXL_BITS_PER_PIXEL(ps_ImageBuffer->e_Format);
   ps_DecodeCmd->sidMemImgBufAddr  = ps_ImageBuffer->ui32_Offset;
   ps_DecodeCmd->sidMemImgBufSize  = ps_ImageBuffer->ui32_Pitch * ps_ImageBuffer->ui32_Height;
   ps_DecodeCmd->sidMemInpBufAddr0 = ((BSID_LinDataMap *)ps_DecodeImage->pv_InputDataMap)->ui32_Offset;
   ps_DecodeCmd->sidMemInpBufSize0 = ((BSID_LinDataMap *)ps_DecodeImage->pv_InputDataMap)->ui32_Size;
   ps_DecodeCmd->sidMemInpBufAddr1 = ps_DecodeCmd->sidMemInpBufAddr0 + ps_DecodeCmd->sidMemInpBufSize0;
   ps_DecodeCmd->sidMemInpBufSize1 = 0;
   ps_DecodeCmd->segMaxHeight      = 0;
   ps_DecodeCmd->dbgRleParseHeader = false;
   ps_DecodeCmd->dbgMsgOn          = BSID_ARC_DECODE_MSG_ENABLE;
   ps_DecodeCmd->lastDMAchunk      = ps_DecodeImage->b_LastDmaChunk;
   ps_DecodeCmd->msBufBase         = ps_DecodeImage->ui32_MultiScanBufAddr;
   ps_DecodeCmd->msBufSize         = ps_DecodeImage->ui32_MultiScanBufSize;
   ps_DecodeCmd->bypass            = false;
   ps_DecodeCmd->doCheckSum        = true;

   if (ps_DecodeImage->pui8_IntraQuantTable)
   {
         ps_DecodeCmd->isIntraQuantTablePresent = 1;
         for (i = 0; i < BSID_MPEG2IFRAME_INTRA_QUANT_ENTRIES; i++)
         {
            ps_DecodeCmd->intra_quant_table[i] = ps_DecodeImage->pui8_IntraQuantTable[i];

         }
   }
   else
   {
      ps_DecodeCmd->isIntraQuantTablePresent = 0;
   }

   BSID_P_BuildGammaCorrectionTable(hSid, ps_DecodeCmd->gamma_correction_table);

   BDBG_MODULE_MSG(BSID_CMD, ("[ReqQ] cmd @%p (%p[%d]) => Decode Still Image sent", (void *)ps_DecodeCmd, (void *)psQueueHeader, psQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum        = 0x%x", hSidCh->ui32_ChannelNum));
   BDBG_MODULE_MSG(BSID_CMD, ("imgFormat         = 0x%x", ps_DecodeCmd->imgFormat));
   BDBG_MODULE_MSG(BSID_CMD, ("imgWidth          = 0x%x", ps_DecodeCmd->imgWidth));
   BDBG_MODULE_MSG(BSID_CMD, ("imgHeight         = 0x%x", ps_DecodeCmd->imgHeight));
   BDBG_MODULE_MSG(BSID_CMD, ("imgPitch          = 0x%x", ps_DecodeCmd->imgPitch));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemImgBufAddr  = 0x%x", ps_DecodeCmd->sidMemImgBufAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemImgBufSize  = 0x%x", ps_DecodeCmd->sidMemImgBufSize));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr0 = 0x%x", ps_DecodeCmd->sidMemInpBufAddr0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize0 = 0x%x", ps_DecodeCmd->sidMemInpBufSize0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr1 = 0x%x", ps_DecodeCmd->sidMemInpBufAddr1));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize1 = 0x%x", ps_DecodeCmd->sidMemInpBufSize1));
   BDBG_MODULE_MSG(BSID_CMD, ("bLastDma          = %d", ps_DecodeCmd->lastDMAchunk));

   BMMA_FlushCache(hQueueHeaderBlock, (void *)ps_DecodeCmd, sizeof(BSID_Cmd_Decode));
   /* signal new queue entry to the ARC */
   /* FIXME: Put this in a function  BSID_P_SignalNewCmdRequest()
     takes queue header pointer(s)/blocks - if data queue pointer is non-null increment/flush it too */
   {
       uint32_t ui32_hst2cpu;

       BKNI_EnterCriticalSection();
       ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
       BSID_INCR_INDEX(psQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BMMA_FlushCache(hQueueHeaderBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
       BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, (ui32_hst2cpu | (1 << hSidCh->ui32_ChannelNum)));
       BKNI_LeaveCriticalSection();
   }

   BDBG_MSG(("rsp dec done"));

   BDBG_LEAVE(BSID_P_SendCmdDecodeStillImage);
   return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_P_SendCmdGetStreamInfo
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdGetStreamInfo(
    BSID_ChannelHandle hSidCh,
    const BSID_StreamInfo *ps_StreamInfo)

{
   BERR_Code               retCode       = BERR_SUCCESS;
   BSID_Handle             hSid          = hSidCh->hSid;
   BSID_CommandQueueHeader *psReqQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BSID_CommandQueueHeader *psDataQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sDataQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle       hReqQHeaderBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BMMA_Block_Handle       hDataQHeaderBlock = hSidCh->sDataQueue.sBuf.hBlock;
   BSID_Cmd_GetStreamInfo  *ps_GetInfoCmd = NULL;
   BSID_ReqQueueEntry      *psReqQueueEntry = NULL;
   BSID_DataQueueEntry     *psDataQueueEntry = NULL;
   uint32_t                dataQueueEntryPhysAddr = 0x0;
   uint32_t                uiMetadataIndex;
   bool                    needAck = false;

   BDBG_ENTER(BSID_P_SendCmdGetStreamInfo);

   /* prepare request queue elem */
   BMMA_FlushCache(hReqQHeaderBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
   psReqQueueEntry = (BSID_ReqQueueEntry *)((uint8_t *)psReqQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                  (psReqQueueHeader->ui32_WriteIndex * sizeof(BSID_ReqQueueEntry)));
   BKNI_Memset((void *)psReqQueueEntry, 0x00, sizeof(BSID_ReqQueueEntry));
   BMMA_FlushCache(hReqQHeaderBlock, (void *)psReqQueueEntry, sizeof(BSID_ReqQueueEntry));
   ps_GetInfoCmd   = (BSID_Cmd_GetStreamInfo *)psReqQueueEntry;

   /* prepare data queue elem */
   /* FIXME: NewDataQueueEntry() */
   BMMA_FlushCache(hDataQHeaderBlock, (void *)psDataQueueHeader, sizeof(BSID_CommandQueueHeader));
   psDataQueueEntry = (BSID_DataQueueEntry *)((uint8_t *)psDataQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                  (psDataQueueHeader->ui32_WriteIndex * sizeof(BSID_DataQueueEntry)));

   dataQueueEntryPhysAddr = hSidCh->sDataQueue.sBuf.ui32_PhysAddr + sizeof(BSID_CommandQueueHeader) + \
                  (psDataQueueHeader->ui32_WriteIndex * sizeof(BSID_DataQueueEntry));
   BKNI_Memset((void *)psDataQueueEntry, 0x00, sizeof(BSID_DataQueueEntry));
   BMMA_FlushCache(hDataQHeaderBlock, (void *)psDataQueueEntry, sizeof(BSID_DataQueueEntry));

   /* prepare a metadata entry to store the addresses to be used by the interrupt handler */
   uiMetadataIndex = NewMetadataEntry(hSidCh, psDataQueueEntry, ps_StreamInfo->ps_OutImageInfo);
   if (BSID_P_INVALID_METADATA_INDEX == uiMetadataIndex)
   {
      /* Shouldn't happen unless we ran out of queue entries also */
      BDBG_ERR(("Out of metadata entries"));
      BDBG_LEAVE(BSID_P_SendCmdGetStreamInfo);
      return BERR_TRACE(BERR_UNKNOWN);
   }

   /* prepare cmd body */
   ps_GetInfoCmd->cmdHdr.header   = BSID_MSG_HDR_GETSTREAMINFO;
   ps_GetInfoCmd->cmdHdr.needAck  = needAck;
   ps_GetInfoCmd->cmdHdr.sequence = ps_StreamInfo->ui32_DecodeSequenceID;

   ps_GetInfoCmd->imgFormat                   = ae_ImgFmtPi2Arc[ps_StreamInfo->e_ImageFormat];
   ps_GetInfoCmd->sidMemInpBufAddr0           = ((BSID_LinDataMap *)ps_StreamInfo->pv_InputDataMap)->ui32_Offset;
   ps_GetInfoCmd->sidMemInpBufSize0           = ((BSID_LinDataMap *)ps_StreamInfo->pv_InputDataMap)->ui32_Size;
   ps_GetInfoCmd->sidMemInpBufAddr1           = ps_GetInfoCmd->sidMemInpBufAddr0 + ps_GetInfoCmd->sidMemInpBufSize0;
   ps_GetInfoCmd->sidMemInpBufSize1           = 0;
   ps_GetInfoCmd->dbgMsgOn                    = BSID_ARC_GET_INFO_MSG_ENABLE;
   ps_GetInfoCmd->sidMemStreamInfoBufPhysAddr = dataQueueEntryPhysAddr;
   /* This is the index of the metadata entry containing the addresses
      where the FW writes the stream info and the address of where the app wants it to go
      This is simply passed to the FW which then passes it back in the release Q response
      for use by the interrupt handler */
   ps_GetInfoCmd->uiMetadataIndex             = uiMetadataIndex;
   ps_GetInfoCmd->lastDMAchunk                = ps_StreamInfo->b_LastDmaChunk;

   BDBG_MODULE_MSG(BSID_CMD, ("[ReqQ] cmd @%p (%p[%d]) => GetStreamInfo sent", (void *)ps_GetInfoCmd, (void *)psReqQueueHeader, psReqQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum              = 0x%x", hSidCh->ui32_ChannelNum));
   BDBG_MODULE_MSG(BSID_CMD, ("imgFormat               = 0x%x", ps_GetInfoCmd->imgFormat));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr0       = 0x%x", ps_GetInfoCmd->sidMemInpBufAddr0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize0       = 0x%x", ps_GetInfoCmd->sidMemInpBufSize0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr1       = 0x%x", ps_GetInfoCmd->sidMemInpBufAddr1));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize1       = 0x%x", ps_GetInfoCmd->sidMemInpBufSize1));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemStreamInfoBufAddr = 0x%x", ps_GetInfoCmd->sidMemStreamInfoBufPhysAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("Metadata Index          = 0x%x", ps_GetInfoCmd->uiMetadataIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("stream info @%p (%p[%d])", (void *)psDataQueueEntry, (void *)psDataQueueHeader, psDataQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("bLastDma          = %d", ps_GetInfoCmd->lastDMAchunk));

   BMMA_FlushCache(hReqQHeaderBlock, (void *)ps_GetInfoCmd, sizeof(BSID_Cmd_GetStreamInfo));

   /* signal new queue enty to the ARC */
   {
       uint32_t ui32_hst2cpu;

       BKNI_EnterCriticalSection();
       ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
       BSID_INCR_INDEX(psReqQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BSID_INCR_INDEX(psDataQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BMMA_FlushCache(hReqQHeaderBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
       BMMA_FlushCache(hDataQHeaderBlock, (void *)psDataQueueHeader, sizeof(BSID_CommandQueueHeader));
       BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, (ui32_hst2cpu | (1 << hSidCh->ui32_ChannelNum)));
       BKNI_LeaveCriticalSection();
   }

   BDBG_MSG(("rsp getinfo done"));

   BDBG_LEAVE(BSID_P_SendCmdGetStreamInfo);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdDecodeSegment
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdDecodeSegment(
    BSID_ChannelHandle hSidCh,
    const BSID_DecodeSegment *ps_DecodeSegment)
{
   BERR_Code                retCode       = BERR_SUCCESS;
   BSID_Handle              hSid          = hSidCh->hSid;
   BSID_CommandQueueHeader  *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle        hReqQBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BSID_ReqQueueEntry       *psQueueEntry = NULL;
   BSID_Cmd_DecodeSegment   *ps_DecodeCmd = NULL;
   const BSID_ImageBuffer   *ps_ImageBuffer  = &ps_DecodeSegment->s_ImageBuffer;
   bool                     needAck = false;

   BDBG_ENTER(BSID_P_SendCmdDecodeSegment);

   /* prepare request queue elem */
   BMMA_FlushCache(hReqQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
   psQueueEntry = (BSID_ReqQueueEntry *)((uint8_t *)psQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                  (psQueueHeader->ui32_WriteIndex * sizeof(BSID_ReqQueueEntry)));
   BKNI_Memset((void *)psQueueEntry, 0x00, sizeof(BSID_ReqQueueEntry));
   BMMA_FlushCache(hReqQBlock, (void *)psQueueEntry, sizeof(BSID_ReqQueueEntry));
   ps_DecodeCmd   = (BSID_Cmd_Decode *)psQueueEntry;

   /* prepare cmd body */
   ps_DecodeCmd->cmdHdr.header     = BSID_MSG_HDR_DECODESEGMENT;
   ps_DecodeCmd->cmdHdr.needAck    = needAck;
   ps_DecodeCmd->cmdHdr.sequence   = ps_DecodeSegment->ui32_DecodeSequenceID;

   ps_DecodeCmd->imgFormat         = ae_ImgFmtPi2Arc[ps_DecodeSegment->e_ImageFormat];
   ps_DecodeCmd->imgWidth          = ps_ImageBuffer->ui32_Width;
   ps_DecodeCmd->imgHeight         = ps_ImageBuffer->ui32_Height;
   ps_DecodeCmd->imgPitch          = ps_ImageBuffer->ui32_Pitch;
   ps_DecodeCmd->imgBpp            = BPXL_BITS_PER_PIXEL(ps_ImageBuffer->e_Format);
   ps_DecodeCmd->sidMemImgBufAddr  = ps_ImageBuffer->ui32_Offset;
   ps_DecodeCmd->sidMemImgBufSize  = ps_ImageBuffer->ui32_Pitch * ps_ImageBuffer->ui32_Height;
   ps_DecodeCmd->sidMemInpBufAddr0 = ((BSID_LinDataMap *)ps_DecodeSegment->pv_InputDataMap)->ui32_Offset;
   ps_DecodeCmd->sidMemInpBufSize0 = ((BSID_LinDataMap *)ps_DecodeSegment->pv_InputDataMap)->ui32_Size;
   ps_DecodeCmd->sidMemInpBufAddr1 = ps_DecodeCmd->sidMemInpBufAddr0 + ps_DecodeCmd->sidMemInpBufSize0;
   ps_DecodeCmd->sidMemInpBufSize1 = 0;
   ps_DecodeCmd->segMaxHeight      = ps_DecodeSegment->ui32_SegmentHeight;
   ps_DecodeCmd->firstSegment      = ps_DecodeSegment->b_FirstSegment;
   ps_DecodeCmd->dbgRleParseHeader = false;
   ps_DecodeCmd->dbgMsgOn          = BSID_ARC_DECODE_SEGMENT_MSG_ENABLE;
   ps_DecodeCmd->lastDMAchunk      = ps_DecodeSegment->b_LastDmaChunk;
   ps_DecodeCmd->msBufBase         = ps_DecodeSegment->ui32_MultiScanBufAddr;
   ps_DecodeCmd->msBufSize         = ps_DecodeSegment->ui32_MultiScanBufSize;
   ps_DecodeCmd->bypass            = false;
   ps_DecodeCmd->doCheckSum        = true;

   BDBG_MODULE_MSG(BSID_CMD, ("[ReqQ] cmd @%p (%p[%d]) => decode segment sent", (void *)ps_DecodeCmd, (void *)psQueueHeader, psQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum        = 0x%x", hSidCh->ui32_ChannelNum));
   BDBG_MODULE_MSG(BSID_CMD, ("imgFormat         = 0x%x", ps_DecodeCmd->imgFormat));
   BDBG_MODULE_MSG(BSID_CMD, ("imgWidth          = 0x%x", ps_DecodeCmd->imgWidth));
   BDBG_MODULE_MSG(BSID_CMD, ("imgHeight         = 0x%x", ps_DecodeCmd->imgHeight));
   BDBG_MODULE_MSG(BSID_CMD, ("imgPitch          = 0x%x", ps_DecodeCmd->imgPitch));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemImgBufAddr  = 0x%x", ps_DecodeCmd->sidMemImgBufAddr));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemImgBufSize  = 0x%x", ps_DecodeCmd->sidMemImgBufSize));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr0 = 0x%x", ps_DecodeCmd->sidMemInpBufAddr0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize0 = 0x%x", ps_DecodeCmd->sidMemInpBufSize0));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufAddr1 = 0x%x", ps_DecodeCmd->sidMemInpBufAddr1));
   BDBG_MODULE_MSG(BSID_CMD, ("sidMemInpBufSize1 = 0x%x", ps_DecodeCmd->sidMemInpBufSize1));
   BDBG_MODULE_MSG(BSID_CMD, ("segMaxHeight      = 0x%x (ns %d)", ps_DecodeCmd->segMaxHeight, ps_DecodeCmd->segMaxHeight/16));
   BDBG_MODULE_MSG(BSID_CMD, ("bLastDma          = %d", ps_DecodeCmd->lastDMAchunk));

   BMMA_FlushCache(hReqQBlock, (void *)ps_DecodeCmd, sizeof(BSID_Cmd_DecodeSegment));

   /* signal new queue enty to the ARC */
   {
       uint32_t ui32_hst2cpu;

       BKNI_EnterCriticalSection();
       ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
       BSID_INCR_INDEX(psQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BMMA_FlushCache(hReqQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
       BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, (ui32_hst2cpu | (1 << hSidCh->ui32_ChannelNum)));
       BKNI_LeaveCriticalSection();
   }

   BDBG_MSG(("rsp dec done"));

   BDBG_LEAVE(BSID_P_SendCmdDecodeSegment);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdSyncChannel
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdSyncChannel(
    BSID_ChannelHandle hSidCh)
{
   BERR_Code                retCode       = BERR_SUCCESS;
   BSID_Handle              hSid          = hSidCh->hSid;
   BSID_CommandQueueHeader  *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle        hReqQBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BSID_ReqQueueEntry       *psQueueEntry = NULL;
   BSID_Cmd_SyncChannel     *ps_SyncChannelCmd = NULL;
   bool                     needAck = false;

   BDBG_ENTER(BSID_P_SendSyncChannel);

   /* prepare request queue elem */
   BMMA_FlushCache(hReqQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
   psQueueEntry = (BSID_ReqQueueEntry *)((uint8_t *)psQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                  (psQueueHeader->ui32_WriteIndex * sizeof(BSID_ReqQueueEntry)));
   BKNI_Memset((void *)psQueueEntry, 0x00, sizeof(BSID_ReqQueueEntry));
   BMMA_FlushCache(hReqQBlock, (void *)psQueueEntry, sizeof(BSID_ReqQueueEntry));
   ps_SyncChannelCmd   = (BSID_Cmd_SyncChannel *)psQueueEntry;

   /* prepare cmd body */
   ps_SyncChannelCmd->cmdHdr.header     = BSID_MSG_HDR_SYNCCHANNEL;
   ps_SyncChannelCmd->cmdHdr.needAck    = needAck;
   ps_SyncChannelCmd->cmdHdr.sequence   = hSidCh->ui32_SequenceNumber++;
   ps_SyncChannelCmd->dbgMsgOn          = BSID_ARC_SYNCCHANNEL_MSG_ENABLE;
   ps_SyncChannelCmd->bypass            = true;

   BDBG_MODULE_MSG(BSID_CMD, ("[ReqQ] cmd @%p (%p[%d]) => channel sync sent", (void *)ps_SyncChannelCmd, (void *)psQueueHeader, psQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum        = 0x%x", hSidCh->ui32_ChannelNum));


   BMMA_FlushCache(hReqQBlock, (void *)ps_SyncChannelCmd, sizeof(BSID_Cmd_SyncChannel));
   /* signal new queue enty to the ARC */
   {
       uint32_t ui32_hst2cpu;

       BKNI_EnterCriticalSection();
       ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
       BSID_INCR_INDEX(psQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BMMA_FlushCache(hReqQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
       BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, (ui32_hst2cpu | (1 << hSidCh->ui32_ChannelNum)));
       BKNI_LeaveCriticalSection();
   }

   BDBG_MSG(("rsp dec done"));

   BDBG_LEAVE(BSID_P_SendCmdSyncChannel);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_SendCmdStartDecode
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_SendCmdStartDecode(BSID_ChannelHandle hSidCh)
{
   BERR_Code               retCode       = BERR_SUCCESS;
   BSID_Handle             hSid          = hSidCh->hSid;
   BSID_CommandQueueHeader *psReqQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
   BMMA_Block_Handle       hReqQueueBlock = hSidCh->sReqQueue.sBuf.hBlock;
   BSID_Cmd_StartDecode    *ps_StartDecodeCmd = NULL;
   BSID_ReqQueueEntry      *psReqQueueEntry = NULL;
   bool                    needAck = false;

   BDBG_ENTER(BSID_P_SendCmdStartDecode);

   /* prepare request queue elem */
   BMMA_FlushCache(hReqQueueBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
   psReqQueueEntry = (BSID_ReqQueueEntry *)((uint8_t *)psReqQueueHeader + sizeof(BSID_CommandQueueHeader) + \
                   (psReqQueueHeader->ui32_WriteIndex * sizeof(BSID_ReqQueueEntry)));
   BKNI_Memset((void *)psReqQueueEntry, 0x00, sizeof(BSID_ReqQueueEntry));
   BMMA_FlushCache(hReqQueueBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
   ps_StartDecodeCmd   = (BSID_Cmd_StartDecode *)psReqQueueEntry;

   /* prepare cmd body */
   ps_StartDecodeCmd->cmdHdr.header   = BSID_MSG_HDR_STARTDECODE;
   ps_StartDecodeCmd->cmdHdr.needAck  = needAck;
   ps_StartDecodeCmd->cmdHdr.sequence = hSidCh->ui32_SequenceNumber++;

   ps_StartDecodeCmd->imgFormat                   = ae_ImgFmtPi2Arc[BSID_ImageFormat_eJPEG];
   ps_StartDecodeCmd->raveReportBufAddr           = hSidCh->sRaveReport.ui32_PhysAddr;
   ps_StartDecodeCmd->playbackReadQueueAddr       = hSidCh->sPlaybackQueue.sReadBuf.ui32_PhysAddr;
   ps_StartDecodeCmd->playbackWriteQueueAddr      = hSidCh->sPlaybackQueue.sWriteBuf.ui32_PhysAddr;
   ps_StartDecodeCmd->queueDepth                  = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;
   ps_StartDecodeCmd->dbgMsgOn                    = BSID_ARC_STARTDECODE_MSG_ENABLE;
   ps_StartDecodeCmd->numPicToDecode              = 0xFFFFFFFF;
   ps_StartDecodeCmd->endianessInput              = !BSID_RAVE_CDB_ENDIAN; /* fw uses reverse notation */
   ps_StartDecodeCmd->endianessOutput             = hSid->sFwHwConfig.b_EndianessSwap;
   ps_StartDecodeCmd->bypass                      = false;

   BDBG_MODULE_MSG(BSID_CMD, ("[ReqQ] cmd @%p (%p[%d]) => start decode sent", (void *)ps_StartDecodeCmd, (void *)psReqQueueHeader, psReqQueueHeader->ui32_WriteIndex));
   BDBG_MODULE_MSG(BSID_CMD, ("channelNum              = 0x%x", hSidCh->ui32_ChannelNum));
   BDBG_MODULE_MSG(BSID_CMD, ("imgFormat               = 0x%x", ps_StartDecodeCmd->imgFormat));
   BDBG_MODULE_MSG(BSID_CMD, ("memoryReportBufAddr     = 0x%x", ps_StartDecodeCmd->raveReportBufAddr ));
   BDBG_MODULE_MSG(BSID_CMD, ("endianessInput          = 0x%x", ps_StartDecodeCmd->endianessInput));
   /*BDBG_MODULE_MSG(BSID_CMD, ("stream info @%p (%p[%d])", psDataQueueEntry, psDataQueueHeader, psDataQueueHeader->ui32_WriteIndex));*/

   BMMA_FlushCache(hReqQueueBlock, (void *)ps_StartDecodeCmd, sizeof(BSID_Cmd_StartDecode));

   /* signal new queue entry to the ARC */
   {
       uint32_t ui32_hst2cpu;

       BKNI_EnterCriticalSection();
       ui32_hst2cpu = BREG_Read32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT);
       BSID_INCR_INDEX(psReqQueueHeader->ui32_WriteIndex, hSidCh->ui32_QueueTrueDepth);
       BMMA_FlushCache(hReqQueueBlock, (void *)psReqQueueHeader, sizeof(BSID_CommandQueueHeader));
       BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_STAT, (ui32_hst2cpu | (1 << hSidCh->ui32_ChannelNum)));
       BKNI_LeaveCriticalSection();
   }

   BDBG_MODULE_MSG(BSID_START_DECODE, ("start decode done"));

   BDBG_LEAVE(BSID_P_SendCmdStartDecode);
   return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_P_SetDmaChunkInfo
*   -
* Comments:
*   -
******************************************************************************/
void BSID_P_SetDmaChunkInfo(
    BSID_ChannelHandle  hSidCh,
    const BSID_DmaChunkInfo *ps_DmaChunkInfo)
{
    BSID_Handle hSid = hSidCh->hSid;
    BSID_LinearBuffer *sDataMemory = &hSid->sFwHwConfig.sInpDmaMemory;
    BSID_DmaChunkInfo *pDmaInfo = (BSID_DmaChunkInfo *)sDataMemory->pv_CachedAddr;
    BSID_CommandQueueHeader  *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sRelQueue.sBuf.pv_CachedAddr;
    BMMA_Block_Handle hInpDmaBlock = hSid->sFwHwConfig.sInpDmaMemory.hBlock;
    BMMA_Block_Handle hRelQBlock = hSidCh->sRelQueue.sBuf.hBlock;

    if (hSidCh->e_ChannelState != BSID_ChannelState_eDecode)
    {
        BDBG_WRN(("SetDmaChunkInfo:: Decode is not active"));
    }

    if (hSidCh->bAbortInitiated)
    {
        BDBG_WRN(("SetDmaChunkInfo:: Abort in progress - DMA request ignored"));
        return;
    }

    BKNI_EnterCriticalSection();
    pDmaInfo->ui32_Offset       = ps_DmaChunkInfo->ui32_Offset;
    pDmaInfo->ui32_Size         = ps_DmaChunkInfo->ui32_Size;
    pDmaInfo->ui32_LastDmaChunk = (ps_DmaChunkInfo->ui32_LastDmaChunk > 0) ? 0x1 : 0x0;
    pDmaInfo->ui32_Reserved     = 0x0;  /* Reserved field is the AbortDma flag */

    BDBG_MSG(("next dma is off 0x%x size 0x%x last %d", pDmaInfo->ui32_Offset, pDmaInfo->ui32_Size, ps_DmaChunkInfo->ui32_LastDmaChunk));

    psQueueHeader->ui32_WaitForData = 0x0;

    BKNI_LeaveCriticalSection();

    BMMA_FlushCache(hInpDmaBlock, (void *)pDmaInfo, sizeof(BSID_DmaChunkInfo));
    BMMA_FlushCache(hRelQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
    return;
}

/******************************************************************************
* Function name: BSID_P_SetAbortDma_isr
*
* Comments:
*     Send an "abort" dma request to the Firmware
******************************************************************************/
void BSID_P_SetAbortDma_isr(BSID_ChannelHandle hSidCh)
{
    BSID_Handle hSid = hSidCh->hSid;
    BSID_LinearBuffer *sDataMemory = &hSid->sFwHwConfig.sInpDmaMemory;
    BSID_DmaChunkInfo *pDmaInfo = (BSID_DmaChunkInfo *)sDataMemory->pv_CachedAddr;
    BMMA_Block_Handle hInpDmaBlock = hSid->sFwHwConfig.sInpDmaMemory.hBlock;

    BSID_CommandQueueHeader  *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sRelQueue.sBuf.pv_CachedAddr;
    BMMA_Block_Handle hRelQBlock = hSidCh->sRelQueue.sBuf.hBlock;

    pDmaInfo->ui32_Offset = 0xFFFFFFFF;
    pDmaInfo->ui32_Size = 0x0;
    pDmaInfo->ui32_LastDmaChunk = 0x1;
    pDmaInfo->ui32_Reserved     = 0x1; /* Abort DMA */
    psQueueHeader->ui32_WaitForData = 0x0;

    BMMA_FlushCache(hInpDmaBlock, (void *)pDmaInfo, sizeof(BSID_DmaChunkInfo));
    BMMA_FlushCache(hRelQBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));
    return;
}


/******************************************************************************
* Function name: BSID_P_BuildGammaCorrectionTable
*   -
* Comments:
*   -
******************************************************************************/
void BSID_P_BuildGammaCorrectionTable(
    BSID_Handle hSid,
    unsigned char *gamma_correction_table
    )
{
    BSTD_UNUSED(hSid);
    BSTD_UNUSED(gamma_correction_table);

    return;
}


/******************************************************************************
* Function name: BSID_P_InitPlayback
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_InitPlayback(
    BSID_ChannelHandle  hSidCh,
    const BSID_DecodeMotion *ps_MotionSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned int ui32_QueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;

    /* RAVE related */
    {
        BSID_Handle hSid = hSidCh->hSid;
        BAVC_XptContextMap *pContextMap = ps_MotionSettings->ps_RaveContextMap;
        BSID_RaveStatusReport *pMemoryReport = (BSID_RaveStatusReport *)hSidCh->sRaveReport.pv_CachedAddr;
        BMMA_Block_Handle hMemoryReportBlock = hSidCh->sRaveReport.hBlock;

        BDBG_MODULE_MSG(BSID_CDB_ITB, ("SID ITB: b 0x%x, e 0x%x, r 0x%x, v 0x%x, w 0x%x",
            BREG_Read32(hSid->hReg, pContextMap->ITB_Base),
            BREG_Read32(hSid->hReg, pContextMap->ITB_End),
            BREG_Read32(hSid->hReg, pContextMap->ITB_Read),
            BREG_Read32(hSid->hReg, pContextMap->ITB_Valid),
            BREG_Read32(hSid->hReg, pContextMap->ITB_Wrap)));


        BDBG_MODULE_MSG(BSID_CDB_ITB, ("SID CDB: b 0x%x, e 0x%x, r 0x%x, v 0x%x, w 0x%x",
            BREG_Read32(hSid->hReg, pContextMap->CDB_Base),
            BREG_Read32(hSid->hReg, pContextMap->CDB_End),
            BREG_Read32(hSid->hReg, pContextMap->CDB_Read),
            BREG_Read32(hSid->hReg, pContextMap->CDB_Valid),
            BREG_Read32(hSid->hReg, pContextMap->CDB_Wrap)));

        pMemoryReport->CDB_Base                 = BREG_Read32(hSid->hReg, pContextMap->CDB_Read);
        pMemoryReport->CDB_End                  = BREG_Read32(hSid->hReg, pContextMap->CDB_End);
        pMemoryReport->from_pi.CDB_Read         = BREG_Read32(hSid->hReg, pContextMap->CDB_Read);
        pMemoryReport->from_pi.CDB_Valid        = BREG_Read32(hSid->hReg, pContextMap->CDB_Valid);
        pMemoryReport->from_pi.CDB_Wrap         = BREG_Read32(hSid->hReg, pContextMap->CDB_Wrap);
        pMemoryReport->from_fw.CDB_Read         = pMemoryReport->from_pi.CDB_Read;

        hSidCh->sContextMap = *pContextMap;
        UnlockRaveBlocks(hSidCh);
        hSidCh->hItbBlock = ps_MotionSettings->hItbBlock;
        hSidCh->pv_CachedItbAddr = BMMA_Lock(hSidCh->hItbBlock);
        hSidCh->hCdbBlock = ps_MotionSettings->hCdbBlock;
        hSidCh->pv_CachedCdbAddr = BMMA_Lock(hSidCh->hCdbBlock);

        if (NULL == hSidCh->pv_CachedItbAddr || NULL == hSidCh->pv_CachedCdbAddr)
        {
            BDBG_ERR(("SID ITB/CDB: Error locking ITB or CDB Block"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        hSidCh->last_ITB_Read = BREG_Read32(hSid->hReg, pContextMap->ITB_Read);
        BMMA_FlushCache(hMemoryReportBlock, (void *)pMemoryReport, sizeof(BSID_RaveStatusReport));
    }

    /* Decode and Xdm related */
    {
        BSID_PlaybackWriteQueueHeader *psPbWriteQueueHeader = (BSID_PlaybackWriteQueueHeader *)hSidCh->sPlaybackQueue.sWriteBuf.pv_CachedAddr;
        BSID_PlaybackReadQueueHeader *psPbReadQueueHeader = (BSID_PlaybackReadQueueHeader *)hSidCh->sPlaybackQueue.sReadBuf.pv_CachedAddr;
        BSID_PlaybackQueueState *psStateQueue = (BSID_PlaybackQueueState *)hSidCh->sPlaybackQueue.sStateBuf.pv_CachedAddr;
        BMMA_Block_Handle hPbWriteQueueBlock = hSidCh->sPlaybackQueue.sWriteBuf.hBlock;
        BMMA_Block_Handle hPbReadQueueBlock = hSidCh->sPlaybackQueue.sReadBuf.hBlock;
        BSID_PlaybackQueueState *psStateQueueEntry;
        uint32_t queue_elem = 0;

        switch (hSidCh->e_ChannelChangeOutputMode)
        {
            case BSID_ChannelChangeOutputMode_eMute:

                psPbWriteQueueHeader->ui32_NewPictWriteIndex = 0;
                psPbReadQueueHeader->ui32_DecodedReadIndex = 0;
                hSidCh->sPlaybackQueue.ui32_HostDecodeReadIndex = 0;
                hSidCh->sPlaybackQueue.ui32_DisplayReadIndex = 0;
                hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount = 0;
                hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount = 0;

                for (queue_elem = 0; queue_elem < ui32_QueueSize; queue_elem++)
                {
                    psStateQueueEntry = &psStateQueue[queue_elem];

                    psStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eIdle;
                    psStateQueueEntry->ps_UnifiedPicture = NULL;
                }
            break;

            case BSID_ChannelChangeOutputMode_eLastFramePreviousChannel:

                psPbWriteQueueHeader->ui32_NewPictWriteIndex = hSidCh->sPlaybackQueue.ui32_DisplayReadIndex;
                psPbReadQueueHeader->ui32_DecodedReadIndex = hSidCh->sPlaybackQueue.ui32_DisplayReadIndex;
                hSidCh->sPlaybackQueue.ui32_HostDecodeReadIndex = hSidCh->sPlaybackQueue.ui32_DisplayReadIndex;
                hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount = 0;
                hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount = 0;

                /* Reset all the entries in the queue minus the one that are still
                   owned by XDM and that it will eventually  release when playback
                   is resume or upon stop. */
                for (queue_elem = 0; queue_elem < ui32_QueueSize; queue_elem++)
                {
                    psStateQueueEntry = &psStateQueue[queue_elem];

                    if (psStateQueueEntry->ui32_OutputState != BSID_OutputBufferState_eDisplay) {
                        psStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eIdle;
                        psStateQueueEntry->ps_UnifiedPicture = NULL;
                    }
                }
            break;

            default:
                BDBG_ERR(("Unsupported channel change mode"));
                return BERR_TRACE(BERR_UNKNOWN);
            break;
        }

        psPbWriteQueueHeader->ui32_ChannelState = psPbReadQueueHeader->ui32_ChannelState = BSID_ChannelState_eDecode;

        BMMA_FlushCache(hPbWriteQueueBlock, (void *)psPbWriteQueueHeader, sizeof(BSID_PlaybackWriteQueueHeader));
        BMMA_FlushCache(hPbReadQueueBlock, (void *)psPbReadQueueHeader, sizeof(BSID_PlaybackReadQueueHeader));
    }

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_StopDecode
*   -
* Comments:
*   -
******************************************************************************/
static void UnlockRaveBlocks(BSID_ChannelHandle hSidCh)
{
    if (hSidCh->pv_CachedItbAddr != NULL)
    {
       BMMA_Unlock(hSidCh->hItbBlock, hSidCh->pv_CachedItbAddr);
       hSidCh->pv_CachedItbAddr = NULL;
    }
    if (hSidCh->pv_CachedCdbAddr != NULL)
    {
       BMMA_Unlock(hSidCh->hCdbBlock, hSidCh->pv_CachedCdbAddr);
       hSidCh->pv_CachedCdbAddr = NULL;
    }
}

/******************************************************************************
* Function name: BSID_P_StopDecode
*   -
* Comments:
*   -
******************************************************************************/
/* FIXME: This should probably be in bsid_priv.c (this is also specific to motion decode) */
BERR_Code BSID_P_StopDecode(BSID_ChannelHandle hSidCh)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BSID_P_StopDecode);

    BDBG_MODULE_MSG(BSID_STOP_DECODE, ("stop decode cmd"));

    UnlockRaveBlocks(hSidCh);

    BDBG_MODULE_MSG(BSID_STOP_DECODE, ("stop decode done"));

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: UpdatePlaybackInfo_isr
*   -
* Comments:
*   -
******************************************************************************/
void BSID_P_UpdatePlaybackInfo_isr(BSID_ChannelHandle  hSidCh)
{
    if (hSidCh->e_ChannelState == BSID_ChannelState_eDecode)
    {
        BSID_Handle hSid = hSidCh->hSid;
        BSID_RaveStatusReport *pMemoryReport = (BSID_RaveStatusReport *)hSidCh->sRaveReport.pv_CachedAddr;
        BSID_PlaybackWriteQueueHeader *psWriteQueueHdr = (BSID_PlaybackWriteQueueHeader *)hSidCh->sPlaybackQueue.sWriteBuf.pv_CachedAddr;
        BSID_PlaybackReadQueueHeader *psReadQueueHdr = (BSID_PlaybackReadQueueHeader *)hSidCh->sPlaybackQueue.sReadBuf.pv_CachedAddr;
        BSID_PlaybackQueueState *psStateQueue = (BSID_PlaybackQueueState *)hSidCh->sPlaybackQueue.sStateBuf.pv_CachedAddr;
        BMMA_Block_Handle hMemoryReportBlock = hSidCh->sRaveReport.hBlock;
        BMMA_Block_Handle hWriteQueueBlock = hSidCh->sPlaybackQueue.sWriteBuf.hBlock;
        BMMA_Block_Handle hReadQueueBlock = hSidCh->sPlaybackQueue.sReadBuf.hBlock;
        BMMA_Block_Handle hStateQueueBlock = hSidCh->sPlaybackQueue.sStateBuf.hBlock;
        BSID_PlaybackWriteQueueEntry *psPbWriteQueueEntryIdle = NULL;
        BSID_PlaybackReadQueueEntry *psPbQueueEntryDecode = NULL;
        BSID_PlaybackQueueState *psStateQueueEntry = NULL;
#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        bool decodedBufAvailable = false;
        bool idleBufAvailable = false;
        uint32_t newPicWriteIndex = 0;
#endif
        uint32_t decodedWriteIndex = 0;
        uint32_t decodedReadIndex = 0;
        BAVC_XptContextMap *pCtxMap = &hSidCh->sContextMap;
        unsigned int ui32_QueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;
        uint32_t CBD_Valid;
        uint32_t CBD_Wrap;

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        hSidCh->stDebugPlaybackTrace.display_time_current = BSID_P_Debug_Gettimeus();
#endif

        /*
         * update CDB valid pointers, always
         */
        pMemoryReport->from_pi.CDB_Read = BREG_Read32(hSid->hReg, pCtxMap->CDB_Read);
        CBD_Valid = BREG_Read32(hSid->hReg, pCtxMap->CDB_Valid);
        CBD_Wrap = BREG_Read32(hSid->hReg, pCtxMap->CDB_Wrap);
        pMemoryReport->from_pi.CDB_Wrap         = CBD_Wrap;
        pMemoryReport->from_pi.CDB_Valid        = CBD_Valid;
        BMMA_FlushCache(hMemoryReportBlock, (void *)pMemoryReport, sizeof(BSID_RaveStatusReport));

        BDBG_MODULE_MSG(BSID_CDB_ITB, ("ITB b 0x%x r 0x%x, v 0x%x, w 0x%x",
        BREG_Read32(hSid->hReg, pCtxMap->ITB_Base),
        BREG_Read32(hSid->hReg, pCtxMap->ITB_Read),
        BREG_Read32(hSid->hReg, pCtxMap->ITB_Valid),
        BREG_Read32(hSid->hReg, pCtxMap->ITB_Wrap)));

        /*
         * Do I have a new output buffer available, if yes post it to fw/hw
         */
        BMMA_FlushCache(hWriteQueueBlock, (void *)psWriteQueueHdr, sizeof(BSID_PlaybackWriteQueueHeader));
        BMMA_FlushCache(hStateQueueBlock, (void *)psStateQueue, sizeof(BSID_PlaybackQueueState));

        psPbWriteQueueEntryIdle = (BSID_PlaybackWriteQueueEntry *)((uint8_t *)psWriteQueueHdr + \
                                  sizeof(BSID_PlaybackWriteQueueHeader) + \
                                  (psWriteQueueHdr->ui32_NewPictWriteIndex * sizeof(BSID_PlaybackWriteQueueEntry)));

        psStateQueueEntry = (BSID_PlaybackQueueState *)((uint8_t *)psStateQueue + \
                                  (psWriteQueueHdr->ui32_NewPictWriteIndex * sizeof(BSID_PlaybackQueueState)));

        BMMA_FlushCache(hWriteQueueBlock, (void *)psPbWriteQueueEntryIdle, sizeof(BSID_PlaybackWriteQueueEntry));
        BMMA_FlushCache(hStateQueueBlock, (void *)psStateQueueEntry, sizeof(BSID_PlaybackQueueState));
        if (psStateQueueEntry->ui32_OutputState == BSID_OutputBufferState_eIdle)
        {
            bool validItbEntry = false;
            unsigned int CdbAddress;
            unsigned int Pts;

            /* find BaseAddress and PTS from ITB */
            validItbEntry = GetNextPicItbInfo_isr(hSid, hSidCh, &CdbAddress, &Pts);
            if (validItbEntry == true)
            {
                psPbWriteQueueEntryIdle->ui32_CdbAddress    = CdbAddress;
                psPbWriteQueueEntryIdle->ui32_Pts           = Pts;
#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
                newPicWriteIndex = psWriteQueueHdr->ui32_NewPictWriteIndex;
                idleBufAvailable = true;
#endif
                psStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eDecode;

                BSID_INCR_INDEX(psWriteQueueHdr->ui32_NewPictWriteIndex, ui32_QueueSize);
                hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount++;

                BMMA_FlushCache(hWriteQueueBlock, (void *)psPbWriteQueueEntryIdle, sizeof(BSID_PlaybackWriteQueueEntry));
                BMMA_FlushCache(hWriteQueueBlock, (void *)psWriteQueueHdr, sizeof(BSID_PlaybackWriteQueueHeader));
            }
        }

        /*
         * check if we have a new decoded buffer, if yes advance Cdb Read
         */
        BMMA_FlushCache(hReadQueueBlock, (void *)psReadQueueHdr, sizeof(BSID_PlaybackReadQueueHeader));
        decodedReadIndex = hSidCh->sPlaybackQueue.ui32_HostDecodeReadIndex;
        decodedWriteIndex = psReadQueueHdr->ui32_DecodedReadIndex;

        BMMA_FlushCache(hStateQueueBlock, (void *)psStateQueueEntry, sizeof(BSID_PlaybackQueueState));

        /*
         * let's check how many picture are ready
         */
        if ((decodedReadIndex != decodedWriteIndex) ||
           (decodedReadIndex == decodedWriteIndex && hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount == ui32_QueueSize))
        {
            psPbQueueEntryDecode = (BSID_PlaybackReadQueueEntry *)((uint8_t *)psReadQueueHdr + sizeof(BSID_PlaybackReadQueueHeader) + \
                                   (decodedReadIndex * sizeof(BSID_PlaybackReadQueueEntry)));

            psStateQueueEntry = (BSID_PlaybackQueueState *)((uint8_t *)psStateQueue + \
                                (decodedReadIndex * sizeof(BSID_PlaybackQueueState)));

            BMMA_FlushCache(hReadQueueBlock, (void *)psPbQueueEntryDecode, sizeof(BSID_PlaybackReadQueueEntry));

            if (psStateQueueEntry->ps_UnifiedPicture == NULL)
            {
#if defined(BSID_P_DEBUG_SAVE_BUFFER)
                uint32_t input_buffer_offset[2] = {0, 0};
                uint32_t input_buffer_size[2] = {0, 0};
                uint32_t num_input_buffer = 0;
                BSID_LinearBuffer* pSaveBuffer = NULL;
                BSID_PlaybackWriteQueueHeader *psSaveWriteQueueHdr = (BSID_PlaybackWriteQueueHeader *)hSidCh->sPlaybackQueue.sWriteBuf.pv_CachedAddr;
                BSID_PlaybackWriteQueueEntry *psSavePbWriteQueueEntry = (BSID_PlaybackWriteQueueEntry *)((uint8_t *)psSaveWriteQueueHdr + sizeof(BSID_PlaybackWriteQueueHeader) + \
                                      (decodedReadIndex * sizeof(BSID_PlaybackWriteQueueEntry)));
                BSID_PlaybackReadQueueEntry *psPbReadQueueEntry = (BSID_PlaybackReadQueueEntry *)((uint8_t *)psReadQueueHdr + sizeof(BSID_PlaybackReadQueueHeader) + \
                                                         (decodedReadIndex * sizeof(BSID_PlaybackReadQueueEntry)));

#endif
                /* fill up unified picture structure once, most important write RAVE reg only one */
                PrepareNextUnifiedPicture_isr(hSidCh, &hSidCh->a_DisplayBuffer[decodedReadIndex], decodedReadIndex);

                /* post it for XDM consumption  */
                psStateQueueEntry->ps_UnifiedPicture = &hSidCh->a_DisplayBuffer[decodedReadIndex];

                /* update RAVE pointer since we are done with that picture */
                BREG_Write32(hSid->hReg, pCtxMap->CDB_Read, psPbQueueEntryDecode->ui32_LastCdbRead);

                hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount--;
                hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount++;

                BSID_INCR_INDEX(hSidCh->sPlaybackQueue.ui32_HostDecodeReadIndex, ui32_QueueSize);
#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
                decodedBufAvailable = true;
#endif

#if defined(BSID_P_DEBUG_SAVE_BUFFER)
                pSaveBuffer = LookUp_Buffer_isr(hSidCh, psSavePbWriteQueueEntry->ui32_OutputBufferAddress);
                BKNI_Printf("Saving Image %d from buf=%p\n", psPbReadQueueEntry->ui32_PictureIdx, pSaveBuffer->pv_CachedAddr);
                BSID_P_Debug_SaveImageData(
                    hSidCh, psPbReadQueueEntry->ui32_PictureIdx, pSaveBuffer->pv_CachedAddr,
                    psPbReadQueueEntry->ui32_Width, psPbReadQueueEntry->ui32_Pitch, psPbReadQueueEntry->ui32_Height,
                    input_buffer_offset, input_buffer_size, num_input_buffer);

                /* release saved image immediately for reuse */
                psStateQueueEntry->ps_UnifiedPicture = NULL;
                psStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eIdle;
#endif
            }
       }

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        hSidCh->stDebugPlaybackTrace.itb_update_time = BSID_P_Debug_Gettimeus();
        if (pMemoryReport->from_pi.CDB_Read <= pMemoryReport->from_pi.CDB_Valid)
        {
            BKNI_Printf("[0x%08x,%d,%d]CDB: 0x%x,0x%x,0x%x,0x%x ",
            hSidCh->stDebugPlaybackTrace.vsync_index,
            (hSidCh->stDebugPlaybackTrace.display_time_current - hSidCh->stDebugPlaybackTrace.display_time_previous)/1000,
            (hSidCh->stDebugPlaybackTrace.itb_update_time - hSidCh->stDebugPlaybackTrace.display_time_current)/1000,
            pMemoryReport->from_pi.CDB_Read,
            pMemoryReport->from_pi.CDB_Valid,
            pMemoryReport->from_pi.CDB_Wrap,
            (pMemoryReport->from_pi.CDB_Valid - pMemoryReport->from_pi.CDB_Read));
        }
        else
        {
            BKNI_Printf("[0x%08x,%d,%d]CDBw: 0x%x,0x%x,0x%x,0x%x ",
            hSidCh->stDebugPlaybackTrace.vsync_index,
            (hSidCh->stDebugPlaybackTrace.display_time_current - hSidCh->stDebugPlaybackTrace.display_time_previous)/1000,
            (hSidCh->stDebugPlaybackTrace.itb_update_time - hSidCh->stDebugPlaybackTrace.display_time_current)/1000,
            pMemoryReport->from_pi.CDB_Read,
            pMemoryReport->from_pi.CDB_Valid,
            pMemoryReport->from_pi.CDB_Wrap,
            ((pMemoryReport->from_pi.CDB_Wrap - pMemoryReport->from_pi.CDB_Read) + (pMemoryReport->from_pi.CDB_Valid - pMemoryReport->CDB_Base)));
        }

        if (idleBufAvailable == true)
        {
            BKNI_Printf("BA[%d]:o 0x%x,i 0x%x ",
            newPicWriteIndex,
            psPbWriteQueueEntryIdle->ui32_OutputBufferAddress,
            psPbWriteQueueEntryIdle->ui32_CdbAddress);
        }

        if (decodedBufAvailable == true)
        {
            BKNI_Printf("PR[%d,%d]:o 0x%x,p 0x%x,dt %d,st %d",
            decodedReadIndex,
            psPbQueueEntryDecode->ui32_PictureIdx,
            psPbQueueEntryDecode->ui32_decode_time,
            psPbQueueEntryDecode->ui32_setup_time);
        }

        BKNI_Printf("\n");
#endif

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        hSidCh->stDebugPlaybackTrace.display_time_previous = hSidCh->stDebugPlaybackTrace.display_time_current;
#endif
    }

    return;
}

/******************************************************************************
* Function name: BSID_P_DisableForFlush
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_DisableForFlush(BSID_ChannelHandle hSidCh)
{
    BERR_Code retCode = BERR_SUCCESS;

    BSID_PlaybackWriteQueueHeader *psPbWriteQueueHeader = (BSID_PlaybackWriteQueueHeader *)hSidCh->sPlaybackQueue.sWriteBuf.pv_CachedAddr;
    BSID_PlaybackReadQueueHeader *psPbReadQueueHeader = (BSID_PlaybackReadQueueHeader *)hSidCh->sPlaybackQueue.sReadBuf.pv_CachedAddr;
    BMMA_Block_Handle hPbWriteQueueBlock = hSidCh->sPlaybackQueue.sWriteBuf.hBlock;
    BMMA_Block_Handle hPbReadQueueBlock = hSidCh->sPlaybackQueue.sReadBuf.hBlock;

    BDBG_ENTER(BSID_P_DisableForFlush);

    /*
     * Signal FW to suspend decode operation.
     */
    /* NOTE: in theory this does not need to be critical sectioned since channels are not manipulated under interrupt by motion decode */
    BSID_SET_CH_STATE(hSidCh, Suspend);
    psPbWriteQueueHeader->ui32_ChannelState = BSID_ChannelState_eSuspend;

    BMMA_FlushCache(hPbWriteQueueBlock, (void *)psPbWriteQueueHeader, sizeof(BSID_PlaybackWriteQueueHeader));
    BMMA_FlushCache(hPbReadQueueBlock, (void *)psPbReadQueueHeader, sizeof(BSID_PlaybackReadQueueHeader));

    /*
     * Wait for FW/HW acknowledge on suspend request.
     */
    while (psPbReadQueueHeader->ui32_ChannelState != BSID_ChannelState_eReady)
    {
        BKNI_Delay(500);
        BMMA_FlushCache(hPbReadQueueBlock, (void *)psPbReadQueueHeader, sizeof(BSID_PlaybackReadQueueHeader));
    }

    /*
     * FW suspended.
     */
    BDBG_ENTER(BSID_P_DisableForFlush);
    return BERR_TRACE(retCode);
}

/******************************************************************************
* Static Functions
*/

/******************************************************************************
* Function name: SendCommand
*   -
* Comments:
*   -
******************************************************************************/

static BERR_Code SendCommand(
    BSID_Handle hSid,
    const BMMA_Block_Handle hCmdMbxBlock,
    const void *pCmd,
    uint32_t    ui32_CmdSize,
    const BMMA_Block_Handle hRspMbxBlock,
    void       *pRsp,
    uint32_t    ui32_RspSize,
    bool        waitForRsp)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ENTER(SendCommand);

   BDBG_ASSERT(pCmd);

   /* flush command to dram to make available to SID */
   BMMA_FlushCache( hCmdMbxBlock, (void *)pCmd, ui32_CmdSize);
   BDBG_MODULE_MSG(BSID_CMD, ("Sending Msg: Hdr %x, Ack: %d, Seq: %x", ((BSID_Cmd_Hdr *)pCmd)->header, ((BSID_Cmd_Hdr *)pCmd)->needAck, ((BSID_Cmd_Hdr *)pCmd)->sequence));

   /* reset mailbox event */
   BKNI_ResetEvent(hSid->sMailbox.hMailboxEvent);

   /* fire command to hw */
   BREG_Write32(hSid->hReg, BCHP_SID_ARC_HST2CPU_MBX, hSid->sMailbox.ui32_CmdQPhysAddr);

   /* wait for response? */
   if (waitForRsp == true)
   {
       BDBG_ASSERT(pRsp);

       /* wait for mailbox event */
       retCode = BKNI_WaitForEvent(hSid->sMailbox.hMailboxEvent, BSID_MAILBOX_MAX_WAITING_TIME);
       if (retCode != BERR_SUCCESS)
       {
           BDBG_ERR(("Wait for response: event timeout reached"));
           return BERR_TRACE(BERR_UNKNOWN);
       }

       BDBG_MODULE_MSG(BSID_CMD, ("Received Mailbox event"));

       if (pRsp)
       {
           /* flush to invalidate rsp contents and force access to sdram to pick SID rsp */
           BMMA_FlushCache( hRspMbxBlock, pRsp, ui32_RspSize);
           BDBG_MODULE_MSG(BSID_CMD, ("Response Received: Hdr %x, Seq: %x, Err: %x", ((BSID_Rsp_Hdr *)pRsp)->header, ((BSID_Rsp_Hdr *)pRsp)->sequence, ((BSID_Rsp_Hdr *)pRsp)->errCode));
           /* sanity check: are we receiving the rsp corresponding to the cmd just sent? */
           if (((BSID_Rsp_Hdr *)pRsp)->header != (((BSID_Cmd_Hdr *)pCmd)->header))
           {
               BDBG_ERR(("Rsp header mismatch or header trashed rcvd: 0x%x, exp: 0x%x",
               ((BSID_Rsp_Hdr *)pRsp)->header,
               (((BSID_Cmd_Hdr *)pCmd)->header)));
               return BERR_TRACE(BERR_UNKNOWN);
           }

           /* has the command failed? */
           if (((BSID_Rsp_Hdr *)pRsp)->errCode != ERR_CODE_SUCCESS)
           {
               BDBG_ERR(("Cmd returned with errCode: 0x%x", ((BSID_Rsp_Hdr *)pRsp)->errCode));
               return BERR_TRACE(BERR_MAKE_CODE(BERR_SID_ID, ((BSID_Rsp_Hdr *)pRsp)->errCode));
           }
       }
   }

   BDBG_LEAVE(SendCommand);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: GetNextPicItbInfo_isr
*   -
* Comments:
*   -
******************************************************************************/
static bool GetNextPicItbInfo_isr(
    BSID_Handle hSid,
    BSID_ChannelHandle hSidCh,
    uint32_t *BaseAddress,
    uint32_t *Pts)
{
    bool validItbInfo    = false;
    BAVC_XptContextMap *pCtxMap = &hSidCh->sContextMap;
    uint32_t oRead   = BREG_Read32(hSid->hReg, pCtxMap->ITB_Read);
    uint32_t oValid  = BREG_Read32(hSid->hReg, pCtxMap->ITB_Valid);
    uint32_t header = 0x0;
    uint32_t oNewRead = 0x0;
    uint32_t lBaseAddress = 0x0;
    uint32_t lPts = 0x0;
    uint32_t lDummy = 0x0;
    BERR_Code rc = BERR_SUCCESS;

    oRead = hSidCh->last_ITB_Read;

    if ((oRead-1) == oValid)
    {
        return validItbInfo;
    }

    /* BKNI_Printf("oRead 0x%x oValid 0x%x\n", oRead, oValid); */

    /*
       Advance from current BaseAddress to next BaseAddress,
       extract BaseAddress for current picture and PTS
    */
    do
    {
        rc = ReadItb4ByteAt_isr(hSidCh, oRead, &header, &oNewRead);
        if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
        switch (BSID_ITB_GET_ENTRY_TYPE(header))
        {
            case BSID_ITB_ENTRY_TYPE_BASE_ADDR:
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lBaseAddress, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
            break;
            case BSID_ITB_ENTRY_TYPE_PTS_DTS:
                /* NOTE: only reads 32-bit DTS/PTS (equivalent to PTS[32:1] giving
                   PTS/DTS @ 45khz, since this is what XDM expects for the unified picture */
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lPts, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                validItbInfo = true;
            break;
            default:
                /* ignore all other ITB entries */
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
                rc = ReadItb4ByteAt_isr(hSidCh, oRead, &lDummy, &oNewRead);
                if (rc != BERR_SUCCESS) {return validItbInfo;} else { oRead = oNewRead; }
            break;
        }

    }
    while (validItbInfo == false);


    BREG_Write32(hSid->hReg, pCtxMap->ITB_Read, (oRead-1));

    hSidCh->last_ITB_Read = oRead;

    *BaseAddress = lBaseAddress;
    *Pts = lPts;

    /*BKNI_Printf("BaseAddress 0x%x, PTS 0x%x\n", *BaseAddress, *Pts);*/

    return validItbInfo;
}

/******************************************************************************
* Function name: ReadValidByteDistance_isr
*   -
* Comments:
*   -
******************************************************************************/
static int ReadValidByteDistance_isr(
    int oBase,
    int oRead,
    int oValid,
    int oWrap
)
{
    if (oValid < oRead)
    {
        return ((oWrap - oRead + 1) + (oValid - oBase + 1));
    }
    else
    {
        return (oValid - oRead + 1);
    }
}

/******************************************************************************
* Function name: ReadItb4ByteAt_isr
*   -
* Comments:
*   -
******************************************************************************/
static BERR_Code ReadItb4ByteAt_isr(
    BSID_ChannelHandle hSidCh,
    uint32_t ItbRead,
    uint32_t *WordRead,
    uint32_t *newReadPtr)
{
    BSID_Handle hSid = hSidCh->hSid;
    BAVC_XptContextMap *pCtxMap = &hSidCh->sContextMap;
    void *pITBAddress = hSidCh->pv_CachedItbAddr;
    uint32_t oBase  = BREG_Read32(hSid->hReg, pCtxMap->ITB_Base);
    uint32_t oValid = BREG_Read32(hSid->hReg, pCtxMap->ITB_Valid);
    uint32_t oRead  = ItbRead;
    uint32_t oWrap  = BREG_Read32(hSid->hReg, pCtxMap->ITB_Wrap);
    uint32_t word = 0;
    uint32_t auxWord = 0;
    unsigned char *pByte;
    int i;

    if (ReadValidByteDistance_isr(oBase, oRead, oValid, oWrap) < 4)
    {
        return BERR_UNKNOWN;
    }

    pByte = (unsigned char *) pITBAddress + (oRead-oBase);
    BMMA_FlushCache(hSidCh->hItbBlock, pByte, sizeof(unsigned char));

    /*BKNI_Printf("ReadItb4ByteAt: Read 0x%x, Valid 0x%x, Wrap 0x%x\n",
    oRead, oValid, oWrap);*/

    for (i = 0; i < 32; i+=8)
    {
        auxWord = *pByte;
        word = (auxWord << i) | word;

        if (oRead == oWrap)
        {
            oRead = oBase;
            pByte = (unsigned char *) pITBAddress + (oRead-oBase);
            BMMA_FlushCache(hSidCh->hItbBlock, pByte, sizeof(unsigned char));
        }
        else
        {
            oRead++;
            pByte++;
        }
    }

    *newReadPtr = oRead;
    *WordRead = word;

    return BERR_SUCCESS;
}

static BSID_LinearBuffer * LookUp_Buffer_isr(BSID_ChannelHandle hSidCh, uint32_t offset)
{
    BSID_LinearBuffer *sMemory = NULL;
    uint32_t buffer_index;

    for (buffer_index = 0x0; buffer_index < hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber; buffer_index++)
    {
        sMemory = &hSidCh->a_OutputBuffer[buffer_index];
        if (sMemory->ui32_PhysAddr == offset)
        {
            return sMemory;
        }
    }

    return NULL;
}

static void PrepareNextUnifiedPicture_isr(
    BSID_ChannelHandle  hSidCh,
    BXDM_Picture *pUnifiedPicture,
    uint32_t displayReadIndex)
{
    BSID_Handle hSid = hSidCh->hSid;
    BSID_PlaybackReadQueueHeader *psReadQueueHdr = (BSID_PlaybackReadQueueHeader *)hSidCh->sPlaybackQueue.sReadBuf.pv_CachedAddr;
    BSID_PlaybackWriteQueueHeader *psWriteQueueHdr = (BSID_PlaybackWriteQueueHeader *)hSidCh->sPlaybackQueue.sWriteBuf.pv_CachedAddr;
    BMMA_Block_Handle hReadQueueBlock = hSidCh->sPlaybackQueue.sReadBuf.hBlock;
    BMMA_Block_Handle hWriteQueueBlock = hSidCh->sPlaybackQueue.sWriteBuf.hBlock;

    BSID_PlaybackReadQueueEntry *psPbReadQueueEntry = NULL;
    BSID_PlaybackWriteQueueEntry *psPbWriteQueueEntry = NULL;

    /* to fill up the unified picture structure we need value from the picture input and output entry */
    psPbReadQueueEntry = (BSID_PlaybackReadQueueEntry *)((uint8_t *)psReadQueueHdr + sizeof(BSID_PlaybackReadQueueHeader) + \
                                                         (displayReadIndex * sizeof(BSID_PlaybackReadQueueEntry)));
    psPbWriteQueueEntry = (BSID_PlaybackWriteQueueEntry *)((uint8_t *)psWriteQueueHdr + sizeof(BSID_PlaybackWriteQueueHeader) + \
                                                         (displayReadIndex * sizeof(BSID_PlaybackWriteQueueEntry)));

    BMMA_FlushCache(hWriteQueueBlock, (void *)psPbWriteQueueEntry, sizeof(BSID_PlaybackWriteQueueEntry));
    BMMA_FlushCache(hReadQueueBlock, (void *)psPbReadQueueEntry, sizeof(BSID_PlaybackReadQueueEntry));

    BKNI_Memset((void *)pUnifiedPicture, 0x0, sizeof(BXDM_Picture));

    {
      BSID_LinearBuffer *sMemory = LookUp_Buffer_isr(hSidCh, psPbWriteQueueEntry->ui32_OutputBufferAddress);
      if (sMemory == NULL)
      {
         BDBG_WRN(("Lookup_Buffer failed to return a buffer"));
         pUnifiedPicture->stBufferInfo.hLuminanceFrameBufferBlock = NULL;
      }
      else
         pUnifiedPicture->stBufferInfo.hLuminanceFrameBufferBlock = sMemory->hBlock;
    }
    pUnifiedPicture->stBufferInfo.ulLuminanceFrameBufferBlockOffset = 0;
    pUnifiedPicture->stBufferInfo.hChrominanceFrameBufferBlock = NULL;
    pUnifiedPicture->stBufferInfo.ulChrominanceFrameBufferBlockOffset = 0;

    /* pixel format */
    pUnifiedPicture->stBufferInfo.stPixelFormat.ePixelFormat = ae_PxlFmtPi2Pxl[hSid->sFwHwConfig.b_EndianessSwap][ae_PxlFmtArc2Pi[psPbReadQueueEntry->ui32_PixelFormat][psPbReadQueueEntry->ui32_BitPerPixel]];
    pUnifiedPicture->stBufferInfo.stPixelFormat.bValid = true;

    /* chroma format: only 422 from SID for MJPEG */
    pUnifiedPicture->stBufferInfo.eYCbCrType = BAVC_YCbCrType_e4_2_2;

    /* true image width */
    pUnifiedPicture->stBufferInfo.stSource.uiWidth = psPbReadQueueEntry->ui32_TrueWidth;
    pUnifiedPicture->stBufferInfo.stSource.uiHeight = psPbReadQueueEntry->ui32_TrueHeight;
    pUnifiedPicture->stBufferInfo.stSource.bValid = true;

    /* display image width */
    pUnifiedPicture->stBufferInfo.stDisplay.uiWidth = psPbReadQueueEntry->ui32_Width;
    pUnifiedPicture->stBufferInfo.stDisplay.uiHeight = psPbReadQueueEntry->ui32_Height;
    pUnifiedPicture->stBufferInfo.stDisplay.bValid = true;

    /* pitch */
    pUnifiedPicture->stBufferInfo.uiRowStride = psPbReadQueueEntry->ui32_Pitch;

    /* buffer info valid or not */
    pUnifiedPicture->stBufferInfo.bValid = (psPbReadQueueEntry->ui32_DecodeErrors == 0);

    pUnifiedPicture->stPTS.uiValue = psPbWriteQueueEntry->ui32_Pts; /* pts in 45kHz units */
    pUnifiedPicture->stPTS.bValid = true;

    /* aspect ratio */
    pUnifiedPicture->stAspectRatio.eAspectRatio = BFMT_AspectRatio_eUnknown;
    pUnifiedPicture->stAspectRatio.bValid = false;

    pUnifiedPicture->stFrameRate.eType = BXDM_Picture_FrameRateType_eUnknown;

    pUnifiedPicture->stBufferInfo.ePulldown = BXDM_Picture_PullDown_eFrameX1;

    BDBG_MSG(("uni %p V %d, Block %p oAddr 0x%x (off 0x%x), sWxH %dx%d, dWxH %dx%d, pitch 0x%x, pf 0x%x (0x%x), PTS 0x%x",
    (void *)pUnifiedPicture,
    pUnifiedPicture->stBufferInfo.bValid,
    (void *)pUnifiedPicture->stBufferInfo.hLuminanceFrameBufferBlock,
    pUnifiedPicture->stBufferInfo.ulLuminanceFrameBufferBlockOffset,
    psPbWriteQueueEntry->ui32_OutputBufferAddress,
    pUnifiedPicture->stBufferInfo.stSource.uiWidth,
    pUnifiedPicture->stBufferInfo.stSource.uiHeight,
    pUnifiedPicture->stBufferInfo.stDisplay.uiWidth,
    pUnifiedPicture->stBufferInfo.stDisplay.uiHeight,
    pUnifiedPicture->stBufferInfo.uiRowStride,
    pUnifiedPicture->stBufferInfo.stPixelFormat.ePixelFormat,
    BPXL_eY18_Cr8_Y08_Cb8,
    pUnifiedPicture->stPTS.uiValue));

    return;
}

static uint32_t NewMetadataEntry(BSID_ChannelHandle hSidCh, BSID_DataQueueEntry *psDataQueueEntry, void *pImageInfo)
{
   uint32_t i;

   BDBG_ASSERT(hSidCh);

   for (i = 0; i < hSidCh->ui32_QueueTrueDepth; i++)
   {
      BSID_P_MetadataEntry *pMetadataEntry = &hSidCh->pMetadata[i];
      if (pMetadataEntry->bInUse)
         continue;
      BKNI_EnterCriticalSection();
      pMetadataEntry->bInUse = true;
      BKNI_LeaveCriticalSection();
      pMetadataEntry->pvStreamInfoFWAddr = psDataQueueEntry;   /* this is an entry in the data queue and
                                                                  is where the FW will write the info */
      pMetadataEntry->pStreamInfoDestAddr = pImageInfo;        /* this is where the app wants the info to go */
      BDBG_MODULE_MSG(BSID_METADATA, ("Allocating Metadata Entry %d", i));
      return i;
   }
   return BSID_P_INVALID_METADATA_INDEX;

}

static void FreeMetadataEntry_isr(BSID_ChannelHandle hSidCh, uint32_t uiMetadataIndex)
{
   BSID_P_MetadataEntry *pMetadataEntry;

   BDBG_ASSERT(hSidCh);

   if ((uiMetadataIndex == BSID_P_INVALID_METADATA_INDEX) || (uiMetadataIndex >= hSidCh->ui32_QueueTrueDepth))
   {
      BDBG_ERR(("Attempt to free an invalid metadata entry"));
   }
   else
   {
      BDBG_MODULE_MSG(BSID_METADATA, ("Freeing Metadata Entry %d", uiMetadataIndex));
      pMetadataEntry = &hSidCh->pMetadata[uiMetadataIndex];
      pMetadataEntry->bInUse = false;
   }
}