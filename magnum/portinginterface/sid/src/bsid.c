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
#include "bdbg.h"
#include "bkni.h"

#include "bsid.h"
#include "bsid_priv.h"
#include "bsid_msg.h"
#include "bsid_picture.h"
#include "bsid_platform.h"
#include "bsid_power.h"
#include "bsid_img.h"  /* for access to default image interface/context */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BSID);

typedef enum {
    BSID_Operation_eSetDmaChunk,          /* still only */
    BSID_Operation_eFlushChannel,         /* either */
    BSID_Operation_eStartDecode,          /* either */
    BSID_Operation_eStopDecode,           /* either */
    BSID_Operation_eLast
} BSID_Operation;


/*****************************************************************************/
static const BSID_RevisionInfo stMyRevisionInfo =
{
    BSID_FW_API_VERSION,      /* FW API version - No need for this, or for the PI version - what do we need this for? */
    BSID_FW_VERSION,          /* FW version */
    BSID_RLELEGACY_MAXWIDTH,  /* image width max for legacy RLE */
    BSID_RLEHDDVD_MAXWIDTH,   /* image width max for HD-DVD RLE */
    BSID_RLEBD_MAXWIDTH,      /* image width max for BD RLE */
    BSID_JPEG_MAXWIDTH,       /* image width max for JPEG */
    BSID_PNGPALETTE_MAXWIDTH, /* image width max for PNG Palletized */
    BSID_PNGRGBA_MAXWIDTH,    /* image width max for PNG RGBA */
    BSID_PNG16BPP_MAXWIDTH,   /* image width max for PNG 16 BitPerPixel */
    BSID_GIF_MAXWIDTH,        /* image width max for GIF */
};


/******************************************************************************
* Function name: CheckParams_isr
*
* Comments:
*     Verify that parameters passed to the operation-specific "start decode"
*     APIs are valid for the specified operation
*
******************************************************************************/
static BERR_Code CheckParams(
    BSID_ChannelHandle hSidChannel,
    BSID_Operation Operation,
    void *pv_OpParam)
{
    BDBG_ASSERT(hSidChannel);

    switch (Operation)
    {
        case BSID_Operation_eStartDecode:
            {
                BSID_StartDecodeSettings *pSettings = (BSID_StartDecodeSettings *)pv_OpParam;
                BDBG_ASSERT(pSettings);

                switch(pSettings->eDecodeMode)
                {
                   case BSID_DecodeMode_eMotion:       /* motion only */
                       {
                          BSID_DecodeMotion *ps_Motion = &pSettings->uDecodeSettings.stMotion;

                          BDBG_ASSERT(NULL != ps_Motion); /* theoretically not possible */

                          if (hSidChannel->e_ChannelType != BSID_ChannelType_eMotion)
                          {
                              BDBG_ERR(("StartDecode: Motion decode specified for a Still Channel"));
                              return BERR_TRACE(BERR_NOT_SUPPORTED);
                          }

                          if (ps_Motion->ps_RaveContextMap == NULL)
                          {
                              BDBG_ERR(("Null pointers in Rave Context Map"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_Motion->hCdbBlock == NULL || ps_Motion->hItbBlock == NULL)
                          {
                              BDBG_ERR(("CDB/ITB Block handles not specified"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }
                       } /* end: Start Decode:: Decode Mode: motion */
                       break;

                   case BSID_DecodeMode_eStillImage:   /* still only */
                       {
                          BSID_DecodeImage *ps_DecodeImage  = &pSettings->uDecodeSettings.stImage;
                          BSID_ImageBuffer *ps_ImageBuffer  = &ps_DecodeImage->s_ImageBuffer;

                          BDBG_ASSERT(NULL != ps_DecodeImage); /* theoretically not possible */
                          BDBG_ASSERT(NULL != ps_ImageBuffer); /* theoretically not possible */

                          if (hSidChannel->e_ChannelType != BSID_ChannelType_eStill)
                          {
                              BDBG_ERR(("StartDecode: Still Image Decode specified for a Motion Channel"));
                              return BERR_TRACE(BERR_NOT_SUPPORTED);
                          }

                          if (ps_DecodeImage->e_ImageFormat >= BSID_ImageFormat_eLast)
                          {
                              BDBG_ERR(("Invalid image format"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_DecodeImage->pv_InputDataMap == NULL)
                          {
                              BDBG_ERR(("Input Data Map is NULL"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_DecodeImage->e_DataMapType != BSID_DataMapType_eLinear)
                          {
                              BDBG_ERR(("Only Linear Data Map Supported"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_ImageBuffer->ui32_Offset == 0)
                          {
                              BDBG_ERR(("Null surface buffer pointer"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_ImageBuffer->ui32_Offset & BSID_MEM_ALIGN_MASK)
                          {
                              BDBG_ERR(("Surface buffer offset of %d is not %d byte aligned", ps_ImageBuffer->ui32_Offset, BSID_MEM_ALIGN_BYTES));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_ImageBuffer->ui32_Pitch == 0)
                          {
                              BDBG_ERR(("Surface pitch is 0"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_ImageBuffer->ui32_Pitch & BSID_MEM_ALIGN_MASK)
                          {
                              BDBG_ERR(("Pitch of %d is not %d bytes aligned", ps_ImageBuffer->ui32_Pitch, BSID_MEM_ALIGN_BYTES));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

#if 0
                          if ((ps_ImageBuffer->ui32_Height * ps_ImageBuffer->ui32_Pitch) > hSid->sFwHwConfig.ui32_MultiScanBufSize)
                          {
                              BDBG_ERR(("Surface size exceed maximum image size decodable"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }
#endif

                          if (ps_ImageBuffer->ui32_Width == 0)
                          {
                              BDBG_ERR(("Surface width is 0"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          if (ps_ImageBuffer->ui32_Height == 0)
                          {
                              BDBG_ERR(("Surface height is 0"));
                              return BERR_TRACE(BERR_INVALID_PARAMETER);
                          }

                          /* check for size of zero to disable bounds check (offset of zero is valid offset) */
                          if (hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size != 0x0)
                          {
                              uint32_t ui32_InpDataMinAddr   = ((BSID_LinDataMap *)ps_DecodeImage->pv_InputDataMap)->ui32_Offset;
                              uint32_t ui32_InpDataMaxAddr   = ui32_InpDataMinAddr + (((BSID_LinDataMap *)ps_DecodeImage->pv_InputDataMap)->ui32_Size);
                              uint32_t ui32_ImageMinAddr     = ps_ImageBuffer->ui32_Offset;
                              uint32_t ui32_ImageMaxAddr     = ui32_ImageMinAddr + (ps_ImageBuffer->ui32_Height * ps_ImageBuffer->ui32_Pitch);
                              uint32_t ui32_MemBoundsMinAddr = hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset;
                              uint32_t ui32_MemBoundsMaxAddr = ui32_MemBoundsMinAddr + hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size;

                              if ((ui32_InpDataMinAddr < ui32_MemBoundsMinAddr) ||
                                  (ui32_InpDataMaxAddr > ui32_MemBoundsMaxAddr) ||
                                  (ui32_ImageMinAddr < ui32_MemBoundsMinAddr) ||
                                  (ui32_ImageMaxAddr > ui32_MemBoundsMaxAddr))
                              {
                                  BDBG_ERR(("Memory out of bounds"));
                                  return BERR_TRACE(BERR_INVALID_PARAMETER);
                              }
                          }
                       }
                       break;
                       /* end: StartDecode:: decode mode: still image */

                   case BSID_DecodeMode_eStillSegment: /* still only */
                       {
                           BSID_DecodeSegment *ps_DecodeSegment  = &pSettings->uDecodeSettings.stSegment;
                           BSID_ImageBuffer *ps_ImageBuffer  = &ps_DecodeSegment->s_ImageBuffer;

                           BDBG_ASSERT(NULL != ps_DecodeSegment); /* theoretically not possible */
                           BDBG_ASSERT(NULL != ps_ImageBuffer); /* theoretically not possible */

                           if (hSidChannel->e_ChannelType != BSID_ChannelType_eStill)
                           {
                               BDBG_ERR(("StartDecode: Still Segment decode specified for a Motion Channel"));
                               return BERR_TRACE(BERR_NOT_SUPPORTED);
                           }

                           if (ps_DecodeSegment->pv_InputDataMap == NULL)
                           {
                               BDBG_ERR(("Input Data Map is NULL"));
                               return BERR_TRACE(BERR_NOT_SUPPORTED);
                           }

                           if (ps_DecodeSegment->e_ImageFormat >= BSID_ImageFormat_eLast)
                           {
                               BDBG_ERR(("Invalid image format"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_DecodeSegment->e_DataMapType != BSID_DataMapType_eLinear)
                           {
                               BDBG_ERR(("Only Linear Data Map Supported"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Offset == 0)
                           {
                               BDBG_ERR(("Null surface buffer pointer"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Offset & BSID_MEM_ALIGN_MASK)
                           {
                               BDBG_ERR(("Surface buffer offset of %d is not %d byte aligned", ps_ImageBuffer->ui32_Offset, BSID_MEM_ALIGN_BYTES));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Pitch == 0)
                           {
                               BDBG_ERR(("Surface pitch is 0"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Pitch & BSID_MEM_ALIGN_MASK)
                           {
                               BDBG_ERR(("Pitch of %d is not %d bytes aligned", ps_ImageBuffer->ui32_Pitch, BSID_MEM_ALIGN_BYTES));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Width == 0)
                           {
                               BDBG_ERR(("Surface width is 0"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_ImageBuffer->ui32_Height == 0)
                           {
                               BDBG_ERR(("Surface height is 0"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           /* check for size of zero to disable bounds check (offset of zero is valid offset) */
                           if (hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size != 0x0)
                           {
                               uint32_t ui32_InpDataMinAddr   = ((BSID_LinDataMap *)ps_DecodeSegment->pv_InputDataMap)->ui32_Offset;
                               uint32_t ui32_InpDataMaxAddr   = ui32_InpDataMinAddr + (((BSID_LinDataMap *)ps_DecodeSegment->pv_InputDataMap)->ui32_Size);
                               uint32_t ui32_ImageMinAddr     = ps_ImageBuffer->ui32_Offset;
                               uint32_t ui32_ImageMaxAddr     = ui32_ImageMinAddr + (ps_ImageBuffer->ui32_Height * ps_ImageBuffer->ui32_Pitch);
                               uint32_t ui32_MemBoundsMinAddr = hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset;
                               uint32_t ui32_MemBoundsMaxAddr = ui32_MemBoundsMinAddr + hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size;

                               if ((ui32_InpDataMinAddr < ui32_MemBoundsMinAddr) ||
                                   (ui32_InpDataMaxAddr > ui32_MemBoundsMaxAddr) ||
                                   (ui32_ImageMinAddr < ui32_MemBoundsMinAddr) ||
                                   (ui32_ImageMaxAddr > ui32_MemBoundsMaxAddr))
                               {
                                   BDBG_ERR(("Memory out of bounds"));
                                   return BERR_TRACE(BERR_INVALID_PARAMETER);
                               }
                           }
                       }
                       break;
                       /* end: StartDecode:: decode mode: still segment */

                   case BSID_DecodeMode_eStillInfo:    /* still only */
                       {
                           BSID_StreamInfo *ps_StreamInfo = &pSettings->uDecodeSettings.stStreamInfo;

                           BDBG_ASSERT(NULL != ps_StreamInfo); /* theoretically not possible */

                           if (hSidChannel->e_ChannelType != BSID_ChannelType_eStill)
                           {
                               BDBG_ERR(("StartDecode: Still Info requested for a Motion Channel"));
                               return BERR_TRACE(BERR_NOT_SUPPORTED);
                           }

                           if (ps_StreamInfo->pv_InputDataMap == NULL)
                           {
                               BDBG_ERR(("Input Data Map is NULL"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_StreamInfo->e_DataMapType != BSID_DataMapType_eLinear)
                           {
                               BDBG_ERR(("Only Linear Data Map Supported"));
                               return BERR_TRACE(BERR_NOT_SUPPORTED);
                           }

                           if (ps_StreamInfo->e_ImageFormat >= BSID_ImageFormat_eLast)
                           {
                               BDBG_ERR(("Invalid image format"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           if (ps_StreamInfo->ps_OutImageInfo == NULL)
                           {
                               BDBG_ERR(("Ps_OutImageInfo is null"));
                               return BERR_TRACE(BERR_INVALID_PARAMETER);
                           }

                           /* NOTE: since stream info is only returning data and not supplying image info, this
                              does not need to verify memory bounds related to the supplied image (since there isn't one) */
                           if (hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset != 0x0)
                           {
                               uint32_t ui32_InpDataMinAddr   = ((BSID_LinDataMap *)ps_StreamInfo->pv_InputDataMap)->ui32_Offset;
                               uint32_t ui32_InpDataMaxAddr   = ui32_InpDataMinAddr + (((BSID_LinDataMap *)ps_StreamInfo->pv_InputDataMap)->ui32_Size);
                               uint32_t ui32_MemBoundsMinAddr = hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset;
                               uint32_t ui32_MemBoundsMaxAddr = ui32_MemBoundsMinAddr + hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size;

                               if ((ui32_InpDataMinAddr < ui32_MemBoundsMinAddr) ||
                                   (ui32_InpDataMaxAddr > ui32_MemBoundsMaxAddr))
                               {
                                   BDBG_ERR(("Memory out of bounds"));
                                   return BERR_TRACE(BERR_INVALID_PARAMETER);
                               }
                           }
                       }
                       break;
                       /* end: StartDecode:: decode mode: still info */

                   default:
                       BDBG_ERR(("StartDecode: Invalid Decode Mode: %d", pSettings->eDecodeMode));
                       return BERR_TRACE(BERR_INVALID_PARAMETER);
                } /* end: switch Decode Mode */
            } /* end: StartDecode */
            break;

        case BSID_Operation_eSetDmaChunk: /* still only */
            {
                BSID_DmaChunkInfo *ps_DmaChunkInfo = (BSID_DmaChunkInfo *)pv_OpParam;
                BDBG_ASSERT(ps_DmaChunkInfo);

                if (hSidChannel->e_ChannelType != BSID_ChannelType_eStill)
                {
                    BDBG_ERR(("SetDmaChunk not supported on Motion Channel"));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }

                /* NOTE: Ignore the value of the reserved field - it does not affect
                   DMA operation (used internally for abort only) */
                if (hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset != 0x0)
                {
                    uint32_t ui32_DmaMinAddr   = ps_DmaChunkInfo->ui32_Offset;
                    uint32_t ui32_DmaMaxAddr   = ui32_DmaMinAddr + ps_DmaChunkInfo->ui32_Size;
                    uint32_t ui32_MemBoundsMinAddr = hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset;
                    uint32_t ui32_MemBoundsMaxAddr = ui32_MemBoundsMinAddr + hSidChannel->sChSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size;

                    if ((ui32_DmaMinAddr < ui32_MemBoundsMinAddr) ||
                        (ui32_DmaMaxAddr > ui32_MemBoundsMaxAddr))
                    {
                        BDBG_ERR(("Memory out of bounds"));
                        return BERR_TRACE(BERR_INVALID_PARAMETER);
                    }
                }
            }
            break;

        case BSID_Operation_eFlushChannel:
            /* verify BSID_FlushSettings - currently, nothing to verify */
            /* valid for both motion and still decode */
            break;

        case BSID_Operation_eStopDecode:
            /* nothing to be done */
            /* valid for both motion and still decode */
            break;

        default:
            /* Should not happen - coding error */
            BDBG_ERR(("CheckParams:: Invalid Operation %d", Operation));
            BDBG_ASSERT(0);
            break;
    } /* end: switch operation */

    return BERR_SUCCESS;
}


/******************************************************************************
* Function name: VerifyEvents
*
* Comments:
*     Verify specified callback events are within allowed values
*
* Return:
*     True if events OK, else False
******************************************************************************/
static bool VerifyEvents(uint32_t ui32_CallbackEvents)
{
   uint32_t uiAllowedEvents = BSID_NotificationEvent_eDecodeDone | BSID_NotificationEvent_eDecodeError | \
      BSID_NotificationEvent_eGetInfoDone | BSID_NotificationEvent_eGetInfoError | BSID_NotificationEvent_eWaitingForMoreInputData | \
      BSID_NotificationEvent_eFlushDone | BSID_NotificationEvent_eFlushError;
   if (ui32_CallbackEvents & (BSID_NotificationEvent_eFlushDone | BSID_NotificationEvent_eFlushError))
      BDBG_WRN(("Unused callback events have been specified: Callback events for Flush are currently unused so will not occur!"));
   return ((ui32_CallbackEvents & ~uiAllowedEvents) == 0);
}

/******************************************************************************
* Function name: BSID_Open
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_Open(
    BSID_Handle         *phSid,
    BCHP_Handle          hChp,
    BREG_Handle          hReg,
    BMEM_Handle          hMem, /* Obsolete - do not use */
    BMMA_Heap_Handle     hMma,
    BINT_Handle          hInt,
    const BSID_Settings *psOpenSettings)
{
    BERR_Code     retCode = BERR_SUCCESS;
    BSID_Handle   hSid    = NULL;

    BDBG_ENTER(BSID_Open);

    /* sanity check */
    BDBG_ASSERT(phSid);
    BDBG_ASSERT(hChp);
    BDBG_ASSERT(hReg);
    BDBG_ASSERT(hMma);
    BDBG_ASSERT(hInt);
    BDBG_ASSERT(psOpenSettings);
    BDBG_ASSERT(psOpenSettings->uiSignature == BSID_P_SIGNATURE_OPENSETTINGS);

    if (hMem != NULL)
      BDBG_WRN(("hMem is obsolete; use hMma and set hMem to NULL"));

    /* The following goes away for release build
       (verifies consistency between FW API and PI defs) */
    BSID_P_CheckFirmwareAPI();

    /* make sure instance number is valid */
    if (psOpenSettings->ui32_SidInstance >= BSID_NUM_DEVICES)
    {
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* allocate device handle */
    hSid = (BSID_Handle)BKNI_Malloc(sizeof (BSID_P_Context));
    if (hSid == NULL)
    {
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* init device handle structure to default value */
    BKNI_Memset((void *)hSid, 0x00, sizeof (BSID_P_Context));

    /* save instance number */
    hSid->ui32_SidInstance = psOpenSettings->ui32_SidInstance;

    /* save base modules handles for future use */
    hSid->hChp = hChp;
    hSid->hReg = hReg;
    hSid->hInt = hInt;
    hSid->hMma = hMma;

    /* extern boot book keeping */
    hSid->pExternalBootCallbackData = psOpenSettings->pv_ExternalBootCallbackData;
    hSid->pExternalBootCallback = psOpenSettings->p_ExternalBootCallback;

    /* Watchdog */
    hSid->fWatchdogCallback_isr = psOpenSettings->p_WatchdogFunc_isr;
    hSid->pWatchdogCallbackData = psOpenSettings->pWatchdogData;

   /* Check for Image Interface/Context */
#if !(BSID_USE_CUSTOM_IMAGE)
    if ((psOpenSettings->pImgInterface != &BSID_ImageInterface)
        || (psOpenSettings->pImgContext != BSID_ImageContext))
    {
       BDBG_WRN(("*******************"));
       BDBG_WRN(("You've linked in the default SID BIMG interface and context."));
       BDBG_WRN(("However, you are providing your own version(s) to BSID_Open()."));
       BDBG_WRN(("You should compile with BSID_USE_CUSTOM_IMAGE=1 to prevent linkage"));
       BDBG_WRN(("of the default BIMG interface and context to reduce the binary size"));
       BDBG_WRN(("*******************"));
    }
#endif

    if ((psOpenSettings->pImgInterface == NULL) ||
        (psOpenSettings->pImgContext == NULL))
    {
       BDBG_ERR(("*******************"));
       BDBG_ERR(("You've compiled with BSID_USE_CUSTOM_IMAGE=1 "));
       BDBG_ERR(("However, you have NOT provided your own version(s) of"));
       BDBG_ERR(("the BIMG interface and context to BSID_Open()."));
       BDBG_ERR(("If you want to use the default BIMG, use BSID_USE_CUSTOM_IMAGE=0"));
       BDBG_ERR(("Otherwise, you MUST provide your own implementation of BIMG."));
       BDBG_ERR(("*******************"));
       return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hSid->pImgInterface = psOpenSettings->pImgInterface;
    hSid->pImgContext = psOpenSettings->pImgContext;

    hSid->bStandby = true; /* startup assumes the ARC needs to be resumed */

    retCode = BSID_P_PowerInit(hSid);
    if (BERR_SUCCESS != retCode)
    {
        BDBG_ERR(("BSID_P_PowerInit returned with error 0x%x", retCode));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* set firmware/hardware configuration and buffers */
    retCode = BSID_P_SetFwHwDefault(hSid, *psOpenSettings);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BSID_P_SetFwHwDefault returned with error 0x%x", retCode));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* set mailbox information */
    retCode = BSID_P_SetMailboxInfo(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BSID_P_SetMailboxInfo failed"));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* create and associate dispatch callback for sid general purpose interrupt */
    retCode = BINT_CreateCallback(
        &hSid->hServiceIsr,
        hSid->hInt,
        BCHP_INT_ID_SID_INTR,
        BSID_P_Dispatch_isr,
        hSid,
        0);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BINT_CreateCallback failed"));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* create watchdog interrupt handler */
    retCode = BINT_CreateCallback(
        &hSid->hWatchdogIsr,
        hSid->hInt,
        BCHP_INT_ID_SID_WATCHDOG_INTR,
        BSID_P_Watchdog_isr,
        hSid,
        0);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BINT_CreateCallback failed"));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* create event for maibox handling */
    retCode = BKNI_CreateEvent(&hSid->sMailbox.hMailboxEvent);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BKNI_CreateEvent failed"));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }

    /* reset mailbox event */
    BKNI_ResetEvent(hSid->sMailbox.hMailboxEvent);

    /* Resume the core (i.e. boot the ARC) */
    retCode = BSID_P_Resume(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to resume the SID ARC"));
        BSID_Close(hSid);
        BDBG_LEAVE(BSID_Open);
        return BERR_TRACE(retCode);
    }
    /* FIXME: Print Log: BSID: FW revision, (API version?) and HW revision from INit command response */
    /* Verify FW version returned by init command is consistent with FW version from API header file
       else, FW Version mismatch error */

    /* prepare device handle */
    *phSid = hSid;

    BDBG_LEAVE(BSID_Open);
    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_Close
*
* Comments:
*
******************************************************************************/
void BSID_Close(
    BSID_Handle hSid
    )
{
    BDBG_ASSERT(hSid);
    BDBG_ENTER(BSID_Close);

    if (!hSid->bStandby)
    {
       /* This will put the core to sleep */
       BSID_P_Standby(hSid); /* NOTE: Open() does the equivalent of Resume() */
       /* NOTE: even if this fails, it should still release the memory etc.,
          so this just 'falls-through' */
    }
    /* else, core is already suspended, so we do nothing */
    /* ... so now we just take care of host-side resources ... */

    /* destroy sid interrupt callback */
    if (NULL != hSid->hServiceIsr)
       BINT_DestroyCallback(hSid->hServiceIsr);

    if (NULL != hSid->hWatchdogIsr)
       BINT_DestroyCallback(hSid->hWatchdogIsr);

    /* destroy mailbox event */
    if (NULL != hSid->sMailbox.hMailboxEvent)
       BKNI_DestroyEvent(hSid->sMailbox.hMailboxEvent);

    /* free sid memory buffer(s) */
    BSID_P_ResetFwHwDefault(hSid);

    /* free device handle memory */
    BKNI_Free((void *)hSid);

    BDBG_LEAVE(BSID_Close);
    return;
}

/******************************************************************************
* Function name: BSID_GetDefaultSettings
*
* Comments:
*
******************************************************************************/
void BSID_GetDefaultSettings(
    BSID_Settings *ps_DefSettings
    )
{
    BDBG_ENTER(BSID_GetDefaultSettings);

    BDBG_ASSERT(ps_DefSettings);

    BKNI_Memset(ps_DefSettings, 0, sizeof(BSID_Settings));
    ps_DefSettings->uiSignature = BSID_P_SIGNATURE_OPENSETTINGS;
    ps_DefSettings->ui8_AlphaValue = 0xFF; /* transparency value (alpha) that will be attached in case of pixel format RGB or YUV444 */
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    ps_DefSettings->b_EndianessSwap = true;  /* Endianess selection: true for big endian */
#else
    ps_DefSettings->b_EndianessSwap = false; /* Endianess selection: false for little endian */
#endif
    ps_DefSettings->b_SelfTest = true,       /* When uploading the SID ARC code to DRAM perfom a sanity test */

#if !(BSID_USE_CUSTOM_IMAGE)
    ps_DefSettings->pImgInterface = &BSID_ImageInterface;
#if !(BSID_USE_CUSTOM_CONTEXT)
    ps_DefSettings->pImgContext = BSID_ImageContext;
#endif
#endif /* else, expect that the user is going to provide their own interface/context */
    BDBG_LEAVE(BSID_GetDefaultSettings);
}

/******************************************************************************
* Function name: BSID_OpenChannel
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_OpenChannel(
    BSID_Handle                 hSid,
    BSID_ChannelHandle         *phSidChannel,
    uint32_t                    ui32_ChannelId,
    const BSID_OpenChannelSettings *ps_OpenChannelSettings
)
{
    BERR_Code           retCode = BERR_SUCCESS;
    BERR_Code           failCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidChannel = NULL;
    uint32_t            ui32_IdleChannel;

    /**
     ** Fix ui32_ChannelId to channel number conversion
     **/

    BDBG_ENTER(BSID_OpenChannel);

    BDBG_ASSERT(hSid);
    BDBG_ASSERT(phSidChannel);
    BDBG_ASSERT(ps_OpenChannelSettings);
    BDBG_ASSERT(ps_OpenChannelSettings->uiSignature == BSID_P_SIGNATURE_OPENCHSETTINGS);
    *phSidChannel = NULL;

    if (hSid->bStandby)
    {
        BDBG_ERR(("SID is Suspended!"));
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Verify channel type is valid */
    if (ps_OpenChannelSettings->e_ChannelType >= BSID_ChannelType_eLast)
    {
        BDBG_ERR(("Invalid Channel Type: %d", ps_OpenChannelSettings->e_ChannelType));
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* make sure channel ID is not already in use by another active channel */
    for (ui32_IdleChannel = 0; ui32_IdleChannel < BSID_MAX_CHANNELS; ui32_IdleChannel++)
    {
        if (hSid->ahChannel[ui32_IdleChannel] != NULL && hSid->ahChannel[ui32_IdleChannel]->ui32_ChannelID == ui32_ChannelId)
        {
            BDBG_ERR(("Channel ID %d is already in use by channel %d", ui32_ChannelId, ui32_IdleChannel));
            BDBG_LEAVE( BSID_OpenChannel );
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* make sure there are sufficient channels available */
    if (!BSID_P_AnyChannelAvailable(hSid, &ui32_IdleChannel))
    {
        BDBG_ERR(("Exceeded maximum number of supported channels"));
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* verify channel-specific settings */
    switch(ps_OpenChannelSettings->e_ChannelType)
    {
       case BSID_ChannelType_eStill:
           /* NOTE: memory bounds checking requires image info, so is done at decode time by CheckParams() */

           if (ps_OpenChannelSettings->u_ChannelSpecific.still.ui32_QueueDepth == 0)
           {
               BDBG_ERR(("Still Channel: Queue Depth must support at least one element"));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }

           if (ps_OpenChannelSettings->u_ChannelSpecific.still.p_CallbackFunc_isr == NULL)
           {
               BDBG_ERR(("Still Channel requires a callback function to be registered"));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           if (!VerifyEvents(ps_OpenChannelSettings->u_ChannelSpecific.still.ui32_CallbackEvents))
           {
               BDBG_ERR(("Still Channel: Invalid Event found in 0x%x",ps_OpenChannelSettings->u_ChannelSpecific.still. ui32_CallbackEvents));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           break;
       case BSID_ChannelType_eMotion:
           if (ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputBuffersNumber == 0)
           {
               BDBG_ERR(("Motion Channel: At least one output buffer required"));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           /* verify that output buffers number does not exceed BSID_P_MAX_OUTPUT_BUFFERS since
              this is also used as an index limit on a_outputBuffers lookup */
           if (ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputBuffersNumber >= BSID_P_MAX_OUTPUT_BUFFERS)
           {
               BDBG_ERR(("Motion Channel: Output buffers count exceeds maximum supported (%d)", BSID_P_MAX_OUTPUT_BUFFERS));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           if (ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxHeight == 0 ||
               ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxWidth == 0)
           {
               BDBG_ERR(("Motion Channel: Output Width and Height must be non-zero"));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           /* Here, it assumes the JPEG size is the largest image size supported by the HW */
           if (ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxHeight >= BSID_JPEG_MAXWIDTH ||
               ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxWidth >= BSID_JPEG_MAXWIDTH)
           {
               BDBG_ERR(("Motion Channel: Output Width and Height must be less than 8K pixels"));
               BDBG_LEAVE( BSID_OpenChannel );
               return BERR_TRACE(BERR_INVALID_PARAMETER);
           }
           if (ps_OpenChannelSettings->u_ChannelSpecific.motion.hOutputBuffersMemHeap != NULL)
           {
               BDBG_WRN(("OpenChannelSettings: hOutputBuffersMemHeap is deprecated (exists for backward compatibility only)"));
           }
           break;
       default:
           BDBG_ASSERT(0); /* Shouldn't happen if all cases accounted for */
           break;
    }

    /* allocate device channel handle */
    hSidChannel = (BSID_ChannelHandle)BKNI_Malloc(sizeof(BSID_P_Channel));
    if (hSidChannel == NULL)
    {
        BDBG_ERR(("Channel handle allocation failed"));
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* init device channel handle structure to default value */
    BKNI_Memset((void *)hSidChannel, 0x00, sizeof(BSID_P_Channel));

    /* register some device channel information */
    BKNI_Memcpy((void *)&hSidChannel->sChSettings, (void *)ps_OpenChannelSettings, sizeof(BSID_OpenChannelSettings));
    hSidChannel->ui32_ChannelNum     = ui32_IdleChannel;
    hSidChannel->hSid                = hSid;
    hSidChannel->ui32_ChannelID      = ui32_ChannelId;
    hSidChannel->e_ChannelType       = hSidChannel->sChSettings.e_ChannelType;
    hSidChannel->e_ChannelChangeOutputMode = BSID_ChannelChangeOutputMode_eLastFramePreviousChannel;

    /* configure channel-specific state */
    switch(hSidChannel->e_ChannelType)
    {
       case BSID_ChannelType_eStill:
           /* since the "queue" is a circular buffer with read/write pointers, actual usable space in the
              "queue" is one less than the size of the queue, so we make the size of the queue one more
              than the requested depth */
           hSidChannel->ui32_QueueTrueDepth = (1 + hSidChannel->sChSettings.u_ChannelSpecific.still.ui32_QueueDepth);
           break;
       case BSID_ChannelType_eMotion:
           hSidChannel->ui32_QueueTrueDepth = (1 + hSidChannel->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber);
           hSidChannel->hChMmaHeap = (hSidChannel->sChSettings.u_ChannelSpecific.motion.hOutputBuffersMmaHeap == NULL) ? \
                           hSid->hMma : hSidChannel->sChSettings.u_ChannelSpecific.motion.hOutputBuffersMmaHeap;
           break;
       default:
           /* Shouldn't happen if all cases accounted for */
           break;
    }

    /* Reset the channel settings that can change */
    BSID_P_ResetChannel(hSidChannel);

    /* register channel to device handle */
    hSid->ahChannel[hSidChannel->ui32_ChannelNum] = hSidChannel;

    /* Clear the DMA info to avoid "sticky" abort status */
    /* FIXME: Something weird about this shared memory - it is shared for
       all channels, so can't this be stomped on if two active decodes at once? */
    BSID_P_ResetDmaInfo(hSid);

    /* create sync event */
    retCode = BKNI_CreateEvent(&hSidChannel->hSyncEvent);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BKNI_CreateEvent failed with error 0x%x", retCode));
        BSID_P_DestroyChannel(hSidChannel);
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(retCode);
    }

   /* create abort event */
    retCode = BKNI_CreateEvent(&hSidChannel->hAbortedEvent);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BKNI_CreateEvent failed with error 0x%x", retCode));
        BSID_P_DestroyChannel(hSidChannel);
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(retCode);
    }

    /* create device channel queue */
    retCode = BSID_P_CreateChannelMemory(hSidChannel);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BSID_P_CreateChannelMemory failed with error 0x%x", retCode));
        BSID_P_DestroyChannel(hSidChannel);
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(retCode);
    }

    /* NOTE: clock is acquired here because previous actions are all host-side */
#ifdef BSID_P_CLOCK_CONTROL
    retCode = BSID_P_Power_AcquireResource(hSid, BSID_P_ResourceType_eClock);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("OpenChannel failed to acquire clock with error 0x%x", retCode));
        BSID_P_DestroyChannel(hSidChannel);
        BDBG_LEAVE( BSID_OpenChannel );
        return BERR_TRACE(retCode);
    }
#endif

    retCode = BSID_P_ResumeChannel(hSidChannel);
    if (BERR_SUCCESS != retCode)
    {
        BDBG_ERR(("OpenChannel: Unable to resume channel (error code 0x%x)", retCode));
        BSID_P_DestroyChannel(hSidChannel);
        failCode = retCode;
        /* NOTE: This falls through so it can release the clock */
    }

#ifdef BSID_P_CLOCK_CONTROL
    /* NOTE: We can do this since SendCmdOpenChannel (in ResumeChannel) is blocking */
    retCode = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_eClock);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("OpenChannel failed to release clock with error 0x%x", retCode));
        if (BERR_SUCCESS != failCode)
        {
            BSID_P_DestroyChannel(hSidChannel);
            failCode = retCode;
        }
    }
    /* NOTE: clock is re-acquired when a decode is started (still or motion)
       This ensures that the SID remains idle until actually used regardless
       of whether a channel is opened */
#endif

    if (BERR_SUCCESS == failCode)
       /* return device handle */
       *phSidChannel = hSidChannel;

    BDBG_LEAVE(BSID_OpenChannel);
    return BERR_TRACE(failCode);

}

/******************************************************************************
* Function name: BSID_CloseChannel
*
* Comments:
*
******************************************************************************/
void BSID_CloseChannel(BSID_ChannelHandle hSidChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_Handle  hSid;
    BDBG_ASSERT(hSidChannel);

    BDBG_ENTER(BSID_CloseChannel);
    hSid = hSidChannel->hSid;

    if (hSid->bStandby)
    {
        BDBG_ERR(("SID is Suspended!"));
        BDBG_LEAVE( BSID_CloseChannel );
        return;
    }

    /* Check the status of the channel and if necessary perform the
       basic equivalent of Stop() to cleanup - it is not guaranteed that
       the caller will stop() before closing, especially upon failure,
       or upon watchdog - this is necessary to avoid resource leaks */
    if ((hSidChannel->e_ChannelState != BSID_ChannelState_eStop)
         && (hSidChannel->e_ChannelState != BSID_ChannelState_eReady))
    {
        BSID_StopDecodeSettings stStopDecodeSettings;
        BDBG_WRN(("Channel Not stopped - Stopping Decode!"));
        BSID_GetDefaultStopSettings(&stStopDecodeSettings);
        BSID_StopDecode(hSidChannel, &stStopDecodeSettings);
    }

#ifdef BSID_P_CLOCK_CONTROL
    retCode = BSID_P_Power_AcquireResource(hSid, BSID_P_ResourceType_eClock);
#endif
    if (BERR_SUCCESS == retCode)
    {
       /* FIXME: Should this be doing a sync before close? Does sync when closing for standby, so why not here */

       /* send close channel command to SID ARC */
       retCode = BSID_P_SendCmdCloseChannel(hSidChannel);
       if (retCode != BERR_SUCCESS)
       {
          BDBG_ERR(("BSID_P_SendCmdCloseChannel failed with error 0x%x", retCode));
       }

#ifdef BSID_P_CLOCK_CONTROL
       /* NOTE: We can do this here since SendCmdCloseChannel is blocking, and
                subsequent actions are all host-side functionality
                If all channels are closed, this will release the resource */
       retCode = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_eClock);
       if (retCode != BERR_SUCCESS)
       {
          BDBG_ERR(("CloseChannel failed to release clock with error code 0x%x", retCode));
       }
#endif
    }
    else
    {
        BDBG_ERR(("CloseChannel failed to acquire clock with error code 0x%x", retCode));
    }

    BSID_P_DestroyChannel(hSidChannel);

    BDBG_LEAVE(BSID_CloseChannel);
    return;
}

/******************************************************************************
* Function name: BSID_GetDefaultOpenChannelSettings
*
* Comments:
*
******************************************************************************/
void BSID_GetDefaultOpenChannelSettings(
    BSID_ChannelType e_ChannelType,
    BSID_OpenChannelSettings *ps_OpenChannelSettings
    )
{
    BDBG_ENTER(BSID_GetDefaultOpenChannelSettings);

    BDBG_ASSERT(ps_OpenChannelSettings);

    BKNI_Memset(ps_OpenChannelSettings, 0, sizeof(BSID_OpenChannelSettings));
    ps_OpenChannelSettings->uiSignature = BSID_P_SIGNATURE_OPENCHSETTINGS;

    /* Set this first incase it is invalid - this will allow subsequent APIs to
       catch the error if this struct is passed into them */
    ps_OpenChannelSettings->e_ChannelType = e_ChannelType;
    switch (e_ChannelType)
    {
        case BSID_ChannelType_eStill:
        {
            ps_OpenChannelSettings->u_ChannelSpecific.still.ui32_QueueDepth              = 1;
            /* Don't perform memory bound check */
            ps_OpenChannelSettings->u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset   = 0x0;
            ps_OpenChannelSettings->u_ChannelSpecific.still.s_MemoryBounds.ui32_Size     = 0x0;
            ps_OpenChannelSettings->u_ChannelSpecific.still.p_CallbackFunc_isr           = NULL;
            ps_OpenChannelSettings->u_ChannelSpecific.still.ui32_CallbackEvents          = 0x0;
            ps_OpenChannelSettings->u_ChannelSpecific.still.pv_CallbackData              = NULL;
        }
        break;
        case BSID_ChannelType_eMotion:
        {
            ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputBuffersNumber    = 3;
            ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxWidth         = 1920;
            ps_OpenChannelSettings->u_ChannelSpecific.motion.ui32_OutputMaxWidth         = 1080;
            ps_OpenChannelSettings->u_ChannelSpecific.motion.hOutputBuffersMemHeap       = NULL;
            ps_OpenChannelSettings->u_ChannelSpecific.motion.hOutputBuffersMmaHeap       = NULL;
        }
        break;
        default:
            BDBG_ERR(("OpenChannel:: Invalid Channel Type: %d", e_ChannelType));
            BDBG_ASSERT(0);   /* Invalid Channel type here is catastrophic (coding error) */
            BDBG_LEAVE(BSID_GetOpenChannelSettings);
            return;
    }
    ps_OpenChannelSettings->ui32_ChannelPriority = 0x0;

    BDBG_LEAVE(BSID_GetDefaultOpenChannelSettings);
    return;
}

#if !B_REFSW_MINIMAL
/******************************************************************************
* Function name: BSID_GetChannelSettings
*
* Comments:
*     Fetch the current channel settings being used for this channel
*     So that settins can be changed at run time using SetChannelSettings()
*
******************************************************************************/
void BSID_GetChannelSettings(
    BSID_ChannelHandle hSidChannel,
    BSID_ChannelSettings *ps_ChannelSettings
    )
{
    BDBG_ENTER(BSID_GetChannelSettings);

    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_ChannelSettings);

    BKNI_Memset(ps_ChannelSettings, 0, sizeof(BSID_ChannelSettings));
    ps_ChannelSettings->uiSignature = BSID_P_SIGNATURE_CHANNELSETTINGS;

    switch (hSidChannel->e_ChannelType)
    {
        case BSID_ChannelType_eStill:
            ps_ChannelSettings->u_ChannelSpecific.still.p_CallbackFunc_isr = hSidChannel->sChSettings.u_ChannelSpecific.still.p_CallbackFunc_isr;
            ps_ChannelSettings->u_ChannelSpecific.still.ui32_CallbackEvents = hSidChannel->sChSettings.u_ChannelSpecific.still.ui32_CallbackEvents;
            ps_ChannelSettings->u_ChannelSpecific.still.pv_CallbackData = hSidChannel->sChSettings.u_ChannelSpecific.still.pv_CallbackData;
            break;

        case BSID_ChannelType_eMotion:
            /* Nothing to be done for motion channel */
            break;

        default:
            /* Note: Invalid channel type is checked for at OpenChannel() so we know this should not occur */
            BDBG_ERR(("GetChannelSettings[%d]:: invalid channel type: %d", hSidChannel->ui32_ChannelNum, hSidChannel->e_ChannelType));
            BDBG_ASSERT(0);
            break;
    }
    ps_ChannelSettings->ui32_ChannelPriority = hSidChannel->sChSettings.ui32_ChannelPriority;

    BDBG_LEAVE(BSID_GetChannelSettings);
    return;
}

/******************************************************************************
* Function name: BSID_SetChannelSettings
*
* Comments:
*    Allows channel settings (such as priority level or callback info)
*    to be altered at run-time
*
******************************************************************************/
BERR_Code BSID_SetChannelSettings(
    BSID_ChannelHandle  hSidChannel,
    const BSID_ChannelSettings *ps_ChannelSettings
    )
{
    BDBG_ENTER( BSID_SetChannelSettings );

    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_ChannelSettings);
    BDBG_ASSERT(ps_ChannelSettings->uiSignature == BSID_P_SIGNATURE_CHANNELSETTINGS);

    switch (hSidChannel->e_ChannelType)
    {
        case BSID_ChannelType_eStill:
            if (ps_ChannelSettings->u_ChannelSpecific.still.p_CallbackFunc_isr != NULL)
               if (!VerifyEvents(ps_ChannelSettings->u_ChannelSpecific.still.ui32_CallbackEvents))
               {
                  BDBG_ERR(("SetChannelSettings[%d]: Invalid Event found in 0x%x",hSidChannel->ui32_ChannelNum, ps_ChannelSettings->u_ChannelSpecific.still. ui32_CallbackEvents));
                  BDBG_LEAVE( BSID_SetChannelSettings );
                  return BERR_TRACE(BERR_UNKNOWN);
               }
            hSidChannel->sChSettings.u_ChannelSpecific.still.p_CallbackFunc_isr   = ps_ChannelSettings->u_ChannelSpecific.still.p_CallbackFunc_isr;
            hSidChannel->sChSettings.u_ChannelSpecific.still.ui32_CallbackEvents  = ps_ChannelSettings->u_ChannelSpecific.still.ui32_CallbackEvents;
            hSidChannel->sChSettings.u_ChannelSpecific.still.pv_CallbackData      = ps_ChannelSettings->u_ChannelSpecific.still.pv_CallbackData;
            break;

        case BSID_ChannelType_eMotion:
            /* Nothing to set at run-time for motion decode */
            break;

        default:
            /* Note: Invalid channel type is checked for at OpenChannel() so we know this should not occur */
            BDBG_ERR(("SetChannelSettings[%d]:: invalid channel type: %d", hSidChannel->ui32_ChannelNum, hSidChannel->e_ChannelType));
            BDBG_LEAVE( BSID_SetChannelSettings );
            return BERR_TRACE(BERR_UNKNOWN);
    }
    hSidChannel->sChSettings.ui32_ChannelPriority = ps_ChannelSettings->ui32_ChannelPriority;

    BDBG_LEAVE( BSID_SetChannelSettings );

    return BERR_SUCCESS;
}
#endif

/******************************************************************************
* Function name: BSID_GetStreamInfo
*
* Comments:
*    *** DEPRECATED -
*        Please use BSID_StartDecode() with decode mode of "StillInfo"
*    ***
*    This is a non-blocking call - it does NOT wait for the response.
*
******************************************************************************/
BERR_Code BSID_GetStreamInfo(
    BSID_ChannelHandle  hSidChannel,
    BSID_StreamInfo     *ps_StreamInfo
)
{
    BERR_Code retCode;
    BSID_StartDecodeSettings ps_StartDecodeSettings;
    BDBG_ENTER(BSID_GetStreamInfo);

    BDBG_WRN(("BSID_GetStreamInfo is deprecated.  Please use BSID_StartDecode with a BSID_DecodeMode of BSID_DecodeMode_eStillInfo"));

    ps_StartDecodeSettings.eDecodeMode = BSID_DecodeMode_eStillInfo;
    ps_StartDecodeSettings.uDecodeSettings.stStreamInfo = *ps_StreamInfo;
    retCode = BSID_StartDecode(hSidChannel, &ps_StartDecodeSettings);

    BDBG_LEAVE(BSID_GetStreamInfo);
    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_DecodeStillImage
*
* Comments:
*    *** DEPRECATED -
*        Please use BSID_StartDecode() with decode mode of "StillImage"
*    ***
*    This is a non-blocking call - it does NOT wait for the response.
*
******************************************************************************/
BERR_Code BSID_DecodeStillImage(
    BSID_ChannelHandle  hSidChannel,
    const BSID_DecodeImage    *ps_DecodeImage
    )
{
    BERR_Code retCode;
    BSID_StartDecodeSettings ps_StartDecodeSettings;

    BDBG_ENTER(BSID_DecodeStillImage);

    BDBG_WRN(("BSID_DecodeStillImage is deprecated.  Please use BSID_StartDecode with a BSID_DecodeMode of BSID_DecodeMode_eStillImage"));

    ps_StartDecodeSettings.eDecodeMode = BSID_DecodeMode_eStillImage;
    ps_StartDecodeSettings.uDecodeSettings.stImage = *ps_DecodeImage;
    retCode = BSID_StartDecode(hSidChannel, &ps_StartDecodeSettings);

    BDBG_LEAVE(BSID_DecodeStillImage);
    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_DecodeImageSegment
*
* Comments:
*    *** DEPRECATED -
*        Please use BSID_StartDecode() with decode mode of "StillSegment"
*    ***
*    This is a non-blocking call - it does NOT wait for the response.
*
******************************************************************************/
BERR_Code BSID_DecodeImageSegment(
    BSID_ChannelHandle  hSidChannel,
    const BSID_DecodeSegment *ps_DecodeSegment
    )
{
    BERR_Code retCode;
    BSID_StartDecodeSettings ps_StartDecodeSettings;

    BDBG_ENTER(BSID_DecodeImageSegment);

    BDBG_WRN(("BSID_DecodeImageSegment is deprecated.  Please use BSID_StartDecode with a BSID_DecodeMode of BSID_DecodeMode_eStillSegment"));

    ps_StartDecodeSettings.eDecodeMode = BSID_DecodeMode_eStillSegment;
    ps_StartDecodeSettings.uDecodeSettings.stSegment = *ps_DecodeSegment;
    retCode = BSID_StartDecode(hSidChannel, &ps_StartDecodeSettings);

    BDBG_LEAVE(BSID_DecodeImageSegment);
    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_SetDMAChunkInfo
*
* Comments:
*     Supplies the next chunk of data to the decode when a waitformoredata
*     event occurs
*
******************************************************************************/
BERR_Code BSID_SetDmaChunkInfo(
    BSID_ChannelHandle  hSidChannel,
    const BSID_DmaChunkInfo   *ps_DmaChunkInfo
    )
{
    BERR_Code   retCode = BERR_SUCCESS;
    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_DmaChunkInfo);

    BDBG_ENTER(BSID_SetDmaChunkInfo);

    if (hSidChannel->e_ChannelState != BSID_ChannelState_eDecode)
    {
        BDBG_WRN(("Attempt to Set DMA Chunk Info when not decoding!"));
        BDBG_LEAVE( BSID_SetDmaChunkInfo_isr );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (hSidChannel->hSid->bStandby)
    {
        BDBG_ERR(("SID is Suspended!"));
        BDBG_LEAVE( BSID_SetDmaChunkInfo_isr );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* sanity check */
    retCode = CheckParams(hSidChannel, BSID_Operation_eSetDmaChunk, (void *)ps_DmaChunkInfo);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("CheckParams failed"));
        return BERR_TRACE(retCode);
    }

    BSID_P_SetDmaChunkInfo(hSidChannel, ps_DmaChunkInfo);

    BDBG_LEAVE(BSID_SetDmaChunkInfo);
    return BERR_SUCCESS;
}


/******************************************************************************
* Function name: BSID_GetRevisionInfo
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_GetRevisionInfo(
    BSID_Handle        hSid,
    BSID_RevisionInfo *pst_RevisionInfo
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER( BSID_GetRevisionInfo );

    BSTD_UNUSED(hSid);

    BDBG_ASSERT(pst_RevisionInfo);

    BKNI_Memcpy((void *)pst_RevisionInfo,
        (void *)&stMyRevisionInfo,
        sizeof(BSID_RevisionInfo));
    /* FIXME: return revision info from Init Command (stored in context) */

    BDBG_LEAVE( BSID_GetRevisionInfo );

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_DisableForFlush
*
* Comments:  Motion Decode Only
*
******************************************************************************/
BERR_Code BSID_DisableForFlush(
    BSID_ChannelHandle  hSidChannel
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER( BSID_DisableForFlush );

    BDBG_ASSERT(hSidChannel);

    if (hSidChannel->e_ChannelState != BSID_ChannelState_eDecode)
    {
        BDBG_WRN(("Attempt to Disable for flush when not decoding!"));
        BDBG_LEAVE( BSID_DisableForFlush );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (hSidChannel->hSid->bStandby)
    {
        BDBG_ERR(("SID is Suspended!"));
        BDBG_LEAVE( BSID_DisableForFlush );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    retCode = BSID_P_DisableForFlush(hSidChannel);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Unable to Disable for Flush"));
    }

    /* NOTE: This does not release the clock - clock is released only upon Stop() */
    BDBG_LEAVE( BSID_DisableForFlush );
    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_GetDefautlFlushSettings
*
* Comments:
*     Fills the structure pointed to by ps_DefFlushSettings with the default
*     values necessary for flushing decode
*
******************************************************************************/
void BSID_GetDefaultFlushSettings(BSID_FlushSettings *ps_DefFlushSettings)
{
    BDBG_ENTER(BSID_GetDefaultFlushSettings);

    BDBG_ASSERT(ps_DefFlushSettings);

    BKNI_Memset((void *)ps_DefFlushSettings, 0, sizeof(BSID_FlushSettings));
    ps_DefFlushSettings->uiSignature = BSID_P_SIGNATURE_FLUSHSETTINGS;
    /* For now, we don't need any flush settings */

    BDBG_LEAVE(BSID_GetDefaultFlushSettings);
}

/******************************************************************************
* Function name: BSID_FlushChannel
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_FlushChannel(
    BSID_ChannelHandle  hSidChannel,
    const BSID_FlushSettings *ps_FlushSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER( BSID_FlushChannel );
    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_FlushSettings);
    BDBG_ASSERT(ps_FlushSettings->uiSignature == BSID_P_SIGNATURE_FLUSHSETTINGS);

    if (hSidChannel->e_ChannelState != BSID_ChannelState_eDecode)
    {
        BDBG_WRN(("Attempt to flush channel when not decoding!"));
        BDBG_LEAVE( BSID_FlushChannel );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (hSidChannel->hSid->bStandby)
    {
        BDBG_ERR(("SID is Suspended!"));
        BDBG_LEAVE( BSID_FlushChannel );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* sanity check */
    /* NOTE: Currently this does nothing, since no parameters to check for flush yet
       but, this is retained for consistency incase flush settings added in the future */
    retCode = CheckParams(hSidChannel, BSID_Operation_eFlushChannel, (void *)ps_FlushSettings);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("CheckParams failed"));
        BDBG_LEAVE( BSID_FlushChannel );
        return BERR_TRACE(retCode);
    }

   /* FIXME: move this to BSID_P_FlushDecode() and have a common API for both?? */

    if (hSidChannel->e_ChannelType == BSID_ChannelType_eStill)
    {
       /* NOTE: This API doesn't seem to be used by nexus or rockford for Still Picture!! */
       hSidChannel->b_FlushPending = true;

       /* prepare flush command and send it to sid arc (this is a blocking call) */
       /* NOTE: FW discards any pending pictures in request Q when this is sent */
       retCode = BSID_P_SendCmdFlushChannel(hSidChannel);
       if (!hSidChannel->b_FlushPending)
       {
           /* wait for data request occurred during flush, so initiate an abort
              NOTE: this is a blocking call and will not return
              until aborted or the equivalent of a watchdog is invoked */
           BSID_P_AbortDecode(hSidChannel);
       }
       hSidChannel->b_FlushPending = false;
       if (retCode != BERR_SUCCESS)
       {
          BDBG_LEAVE( BSID_FlushChannel );
          return BERR_TRACE(retCode);
       }
    }
    else
    {
       /* NOTE: This API is called by Nexus' sid video decoder extension
         and does DisableForFlush first then calls FlushChannel */

       BSID_DecodeMotion stMotionSettings;
       /* use current start decode state to re-start the channel */
       stMotionSettings.ps_RaveContextMap = &hSidChannel->sContextMap;
       stMotionSettings.hItbBlock = hSidChannel->hItbBlock;
       stMotionSettings.hCdbBlock = hSidChannel->hCdbBlock;

       retCode = BSID_P_MotionDecode(hSidChannel, &stMotionSettings);
       if (retCode != BERR_SUCCESS)
       {
          /* un-lock status of sid arc: sid arc status is idle */
          BDBG_LEAVE( BSID_FlushChannel );
          return BERR_TRACE(retCode);
       }
       /* FIXME: do we need to wait for the flush to complete here? */
       /* Does flush imply that the channel will be deactivated (and hence clocks can be released)?
          Probably not - can flush and then ... what continue?  Start another decode?
          Will Flush always be followed by Stop? Yes, any Start must always be followed by Stop()
          => there may or may not be a flush in between, so clocks controlled on Start/Stop, not Flush */
    }

    /* NOTE: This does not release the clock - clock is released only upon Stop() */
    BDBG_LEAVE( BSID_FlushChannel );

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_GetDefaultStartDecodeSettings
*
* Comments:
*
******************************************************************************/
void BSID_GetDefaultStartDecodeSettings(
    BSID_DecodeMode eDecodeMode,
    BSID_StartDecodeSettings *ps_DefStartDecodeSettings
    )
{
    BDBG_ENTER(BSID_GetDefaultStartDecodeSettings);

    BDBG_ASSERT(ps_DefStartDecodeSettings);

    BKNI_Memset((void *)ps_DefStartDecodeSettings, 0, sizeof(BSID_StartDecodeSettings));
    ps_DefStartDecodeSettings->uiSignature = BSID_P_SIGNATURE_STARTDECSETTINGS;
    ps_DefStartDecodeSettings->eDecodeMode = eDecodeMode;

    BDBG_LEAVE(BSID_GetDefaultStartDecodeSettings);
}

/******************************************************************************
* Function name: BSID_StartDecode
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_StartDecode(
    BSID_ChannelHandle  hSidChannel,
    const BSID_StartDecodeSettings *ps_StartDecodeSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER( BSID_StartDecode );

    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_StartDecodeSettings);
    BDBG_ASSERT(ps_StartDecodeSettings->uiSignature == BSID_P_SIGNATURE_STARTDECSETTINGS);

    if (hSidChannel->hSid->bStandby)
    {
        BDBG_ERR(("StartDecode:: SID is Suspended!"));
        BDBG_LEAVE( BSID_StartDecode );
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* sanity check */
    retCode = CheckParams(hSidChannel, BSID_Operation_eStartDecode, (void *)ps_StartDecodeSettings);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("StartDecode:: CheckParams failed"));
        BDBG_LEAVE( BSID_StartDecode );
        return BERR_TRACE(retCode);
    }
    BDBG_MSG(("StartDecode:: Mode: %d", ps_StartDecodeSettings->eDecodeMode));

#ifdef BSID_P_CLOCK_CONTROL
    retCode = BSID_P_Power_AcquireResource(hSidChannel->hSid, BSID_P_ResourceType_eClock);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("StartDecode failed to acquire clock with error 0x%x", retCode));
        BDBG_LEAVE( BSID_StartDecode );
        return BERR_TRACE(retCode);
    }
#endif

    hSidChannel->eDecodeMode = ps_StartDecodeSettings->eDecodeMode;
    hSidChannel->bAbortInitiated = false;

    /* Check settings to determine desired operation */
    switch (ps_StartDecodeSettings->eDecodeMode)
    {
        case BSID_DecodeMode_eMotion:
            /* check state: start decode can occur in ready state only */
            if (BSID_P_IsMotionOperationAllowed(hSidChannel->hSid) == false)
            {
                BDBG_ERR(("StartDecode:: Operation can't be executed. Still decode operations in progress!"));
                BDBG_LEAVE( BSID_StartDecode );
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            retCode = BSID_P_MotionDecode(hSidChannel, &ps_StartDecodeSettings->uDecodeSettings.stMotion);
            break;
        case BSID_DecodeMode_eStillInfo:
            /*
             * This operation is not executed in multiple passes (like segmented decode).
             * If a motion jpeg channel was opened before this call we reject.
            */
            if (BSID_P_IsStillOperationAllowed(hSidChannel->hSid) == false)
            {
               BDBG_ERR(("StartDecode:: Operation can't be executed. Motion decode channel is opened!"));
               BDBG_LEAVE( BSID_StartDecode );
               return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            /* is the sid ready to accept this request? */
            if (!BSID_P_IsChannelQueueFull(hSidChannel))
            {
               /* post request to channel queue */
               retCode = BSID_P_SendCmdGetStreamInfo(hSidChannel, &ps_StartDecodeSettings->uDecodeSettings.stStreamInfo);
            }
            else
            {
               BDBG_ERR(("StartDecode:: Unable to send command - Channel Queue is full"));
               retCode = BERR_OUT_OF_DEVICE_MEMORY;
            }
            break;
        case BSID_DecodeMode_eStillImage:
            /*
            * This operation is not executed in multiple passes (like segmented decode)
            * If a motion jpeg channel was opened before this call we reject.
            */
            if (BSID_P_IsStillOperationAllowed(hSidChannel->hSid) == false)
            {
               BDBG_ERR(("StartDecode:: Operation can't be executed. Motion decode channel is opened!"));
               BDBG_LEAVE( BSID_StartDecode );
               return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            /* is the sid ready to accept this request? */
            if (!BSID_P_IsChannelQueueFull(hSidChannel))
            {
               /* prepare decode command and send it to sid arc */
               retCode = BSID_P_SendCmdDecodeStillImage(hSidChannel, &ps_StartDecodeSettings->uDecodeSettings.stImage);
            }
            else
            {
               BDBG_ERR(("StartDecode:: Unable to send command - Channel Queue is full"));
               retCode = BERR_OUT_OF_DEVICE_MEMORY;
            }
            break;
        case BSID_DecodeMode_eStillSegment:
            /*
            * This operation may be executed in multiple passes. Reject all the
            * new segmented decode operation. Allow the ones that are happening
            * for which b_FirstSegment is set to false.
            */
            if ((BSID_P_IsStillOperationAllowed(hSidChannel->hSid) == false) && (ps_StartDecodeSettings->uDecodeSettings.stSegment.b_FirstSegment == true))
            {
               BDBG_ERR(("StartDecode:: Operation can't be executed. Motion decode channel is opened!"));
               BDBG_LEAVE( BSID_StartDecode );
               return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            /* is the sid ready to accept this request? */
            if (!BSID_P_IsChannelQueueFull(hSidChannel))
            {
               /* prepare decode command and send it to sid arc */
               retCode = BSID_P_SendCmdDecodeSegment(hSidChannel, &ps_StartDecodeSettings->uDecodeSettings.stSegment);
            }
            else
            {
               BDBG_ERR(("StartDecode:: Unable to send command - Channel Queue is full"));
               retCode = BERR_OUT_OF_DEVICE_MEMORY;
            }
            break;
        default:
            BDBG_ERR(("StartDecode:: Invalid DecodeMode: %d", ps_StartDecodeSettings->eDecodeMode));
            retCode = BERR_INVALID_PARAMETER;
    }

    if (BERR_SUCCESS == retCode)
    {
       BSID_SET_CH_STATE(hSidChannel, Decode);
    }
    BDBG_LEAVE( BSID_StartDecode );
    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_GetDefaultStopSettings
*
* Comments:
*     Default is to NOT force a stop
*
******************************************************************************/
void BSID_GetDefaultStopSettings(
    BSID_StopDecodeSettings *ps_DefStopDecodeSettings
    )
{
    BDBG_ENTER(BSID_GetDefaultStopSettings);

    BDBG_ASSERT(ps_DefStopDecodeSettings);

    BKNI_Memset((void *)ps_DefStopDecodeSettings, 0, sizeof(BSID_StopDecodeSettings));
    ps_DefStopDecodeSettings->uiSignature = BSID_P_SIGNATURE_STOPDECSETTINGS;

    BDBG_LEAVE(BSID_GetDefaultStopSettings);
}


/******************************************************************************
* Function name: BSID_StopDecode
*
* Comments:
*    For a still decode (or get stream info), this API assumes that the picture
*    has been decoded and has been consumed. This API will NOT wait for
*    decoding to be done - it must already be done.  If not, it will force
*    an abort.
*    Also, it will force an abort if bForceStop is set in the settings.
*    This API is a "cleanup" after a decode is done
*
******************************************************************************/
void BSID_StopDecode(
    BSID_ChannelHandle hSidChannel,
    const BSID_StopDecodeSettings *ps_StopDecodeSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BSID_StopDecode);
    BDBG_ASSERT(hSidChannel);
    BDBG_ASSERT(ps_StopDecodeSettings->uiSignature == BSID_P_SIGNATURE_STOPDECSETTINGS);

    if (hSidChannel->hSid->bStandby)
    {
        BDBG_ERR(("StopDecode:: SID is Suspended!"));
        BDBG_LEAVE( BSID_StopDecode );
        return;
    }

    /* Don't do anything if channel already stopped
       (avoids releasing an unacquired clock, but allows
        this API to be used as a forced "cleanup") */
    if (hSidChannel->e_ChannelState != BSID_ChannelState_eStop)
    {
        /* FIXME: Move this to the private StopDecode() */
        if (hSidChannel->e_ChannelType == BSID_ChannelType_eStill)
        {
            /* force abort if channel not ready and force abort flag set or if not segment decode and not ready */
            if ((hSidChannel->e_ChannelState != BSID_ChannelState_eReady) &&
                (ps_StopDecodeSettings->bForceStop ||
                (hSidChannel->eDecodeMode != BSID_DecodeMode_eStillSegment)))
            {
                /* decode active - need to force an abort */
                BDBG_WRN(("StopDecode:: Channel is still active.  Forcing abort ..."));
                BSID_P_AbortDecode(hSidChannel);
                /* NOTE: This will not return until decode is aborted, or unless watchdog invoked */
            }
        }
        else
        {
            /* motion decode */
            /* existing close - is DisableFor Flush sufficient? Do we need to do anything else
               to abort/force close the decode? */

            /* FIXME: why is this disabling for flush, but not flushing and not stopping?
              => FlushChannel is used for this purpose - it is expected to be called
              after this call */
            retCode = BSID_P_DisableForFlush(hSidChannel);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BSID_P_DisableForFlush failed retCode 0x%x", retCode));
                /* NOTE: This falls thru - regardless of success of DisableForFlush
                   this will subsequently attempt to stop the decode */
            }

            /* NOTE: This is unlocking Rave blocks, so we could make this the generic API for stop() */
            /* NOTE: For motion decode, this currently cannot fail (always succeeds) */
            retCode = BSID_P_StopDecode(hSidChannel);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BSID_P_StopDecode failed retCode 0x%x", retCode));
                return;
            }
        }
        BSID_SET_CH_STATE(hSidChannel, Stop);

        /* SID has stopped, so we can release clocks now ...*/
#ifdef BSID_P_CLOCK_CONTROL
        retCode = BSID_P_Power_ReleaseResource(hSidChannel->hSid, BSID_P_ResourceType_eClock);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("StopDecode failed to release clock with error 0x%x", retCode));
            BDBG_LEAVE( BSID_StopDecode );
            return;
        }
#endif
    } /* end: if not stopped */

    BDBG_LEAVE( BSID_StopDecode );
    return;
}

/******************************************************************************
* Function name: BSID_GetRaveItbCdbConfigInfo
*
* Comments:
*
******************************************************************************/
void BSID_GetRaveItbCdbConfigInfo(
    BAVC_CdbItbConfig  *ps_ConfigInfo
    )
{
    BDBG_ENTER( BSID_GetRaveItbCdbConfigInfo );

    ps_ConfigInfo->Cdb.Length           = BSID_RAVE_CDB_LENGTH;
    ps_ConfigInfo->Cdb.Alignment        = BSID_RAVE_CDB_ALIGNMENT;
    ps_ConfigInfo->Cdb.LittleEndian     = BSID_RAVE_CDB_ENDIAN;
    ps_ConfigInfo->Itb.Length           = BSID_RAVE_ITB_LENGTH;
    ps_ConfigInfo->Itb.Alignment        = BSID_RAVE_ITB_ALIGNMENT;
    ps_ConfigInfo->Itb.LittleEndian     = BSID_RAVE_ITB_ENDIAN;
    ps_ConfigInfo->UsePictureCounter = false;

    BDBG_LEAVE( BSID_GetRaveItbCdbConfigInfo );

    return;
}

/******************************************************************************
* Function name: BSID_GetXdmInterface
*
* Comments:
*
******************************************************************************/
void BSID_GetXdmInterface(
    BSID_ChannelHandle  hSidChannel,
    BXDM_Decoder_Interface *ps_XdmInterface,
    void **pCtx
    )
{
    BDBG_ENTER( BSID_GetXdmInterface );

    BKNI_Memset((void *)ps_XdmInterface, 0, sizeof(BXDM_Decoder_Interface));
    ps_XdmInterface->getPictureCount_isr              = BSID_P_GetPictureCount_isr;
    ps_XdmInterface->peekAtPicture_isr                = BSID_P_PeekAtPicture_isr;
    ps_XdmInterface->getNextPicture_isr               = BSID_P_GetNextPicture_isr;
    ps_XdmInterface->releasePicture_isr               = BSID_P_ReleasePicture_isr;
    ps_XdmInterface->getPictureDropPendingCount_isr   = NULL;     /* not used. BSID_P_GetPictureDropPendingCount_isr */
    ps_XdmInterface->requestPictureDrop_isr           = NULL;     /* not used. BSID_P_RequestPictureDrop_isr */
    ps_XdmInterface->displayInterruptEvent_isr        = BSID_P_DisplayInterruptEvent_isr;

   *pCtx = hSidChannel;

   BDBG_LEAVE( BSID_GetXdmInterface );

   return;
}

/******************************************************************************
* Function name: BSID_Standby
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_Standby(BSID_Handle hSid)
{
    if (!BSID_P_Power_SupportsResource(hSid, BSID_P_ResourceType_ePower))
    {
        BDBG_WRN(("Standby/Resume Not Supported - SID ARC will be idled, but not powered down"));
    }
    return BSID_P_Standby(hSid);
}

/******************************************************************************
* Function name: BSID_Resume
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_Resume(BSID_Handle hSid)
{
    if (!BSID_P_Power_SupportsResource(hSid, BSID_P_ResourceType_ePower))
    {
       BDBG_WRN(("Standby/Resume Not Supported - SID ARC will be rebooted/restarted"));
    }
    return BSID_P_Resume(hSid);
}

/******************************************************************************
* Function name: BSID_ProcessWatchdog
*
* Comments:
*    Essentially resets the SID ARC/HW and restarts any active channels
*
******************************************************************************/
BERR_Code BSID_ProcessWatchdog(BSID_Handle hSid)
{
   BERR_Code rc = BERR_SUCCESS;
   BERR_Code failCode = BERR_SUCCESS;

   BDBG_ENTER(BSID_ProcessWatchdog);

   BDBG_ASSERT(hSid);

   if (false == hSid->bWatchdogOccurred)
   {
      BDBG_WRN(("BSID_ProcessWatchdog called but watchdog did not occur"));
   }
   else
   {
      /* NOTE: this will attempt to resume the firmware even if any
         prior steps fail to occur correctly */
      rc = BINT_DisableCallback(hSid->hServiceIsr);
      if (rc != BERR_SUCCESS)
      {
         BDBG_ERR(("BSID_ProcessWatchdog:: BINT_DisableCallback returned with error 0x%x", rc));
         if (BERR_SUCCESS == failCode)
            failCode = rc;
      }

      /* Standby to shutdown/reset core */
      rc = BSID_P_Standby(hSid);
      if (BERR_SUCCESS != rc)
      {
         BDBG_ERR(("BSID_ProcessWatchdog:: Unable to standby SID"));
         if (BERR_SUCCESS == failCode)
            failCode = rc;
      }

      hSid->bWatchdogOccurred = false;  /* only used by Standby() */

      /* Resume to reboot core and reopen channels */
      /* NOTE: we must force the firmware loader to reload everything
         (i.e. as if its a cold boot) - we cannot rely upon any firmware state
         even the code! */
      hSid->bArcReLoad = false;
      rc = BSID_P_Resume(hSid);
      if (BERR_SUCCESS != rc)
      {
         BDBG_ERR(("BSID_ProcessWatchdog:: Unable to resume to restore SID functionality"));
         if (BERR_SUCCESS == failCode)
            failCode = rc;
      }
   }

   BDBG_LEAVE(BSID_ProcessWatchdog);
   return BERR_TRACE(failCode);
}

/*******************
  End Of File
********************/
