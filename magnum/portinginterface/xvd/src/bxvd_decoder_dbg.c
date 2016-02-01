/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_decoder.h"
#include "bxvd_decoder_timer.h"

#include "bxvd_vdec_info.h"

BDBG_MODULE(BXVD_DECODER_DBG);
BDBG_FILE_MODULE(BXVD_UP);
BDBG_FILE_MODULE(BXVD_SEI);

#if BDBG_DEBUG_BUILD

/*
 * Lookup tables for mapping variables to strings.
 */
static const char * const s_aPictureCodingToStrLUT[BXDM_Picture_Coding_eMax] =
{
   "u",  /* BXDM_Picture_Coding_eUnknown */
   "I",  /* BXDM_Picture_Coding_eI */
   "P",  /* BXDM_Picture_Coding_eP */
   "B"   /* BXDM_Picture_Coding_eB */
};

static const char * const s_aPulldownToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "err",   /* invalid */
   "T  ",   /* BXDM_Picture_PullDown_eTop = 1 */
   "B  ",   /* BXDM_Picture_PullDown_eBottom = 2 */
   "TB ",   /* BXDM_Picture_PullDown_eTopBottom = 3 */
   "BT ",   /* BXDM_Picture_PullDown_eBottomTop = 4 */
   "TBT",   /* BXDM_Picture_PullDown_eTopBottomTop = 5 */
   "BTB",   /* BXDM_Picture_PullDown_eBottomTopBottom = 6 */
   "X2 ",   /* BXDM_Picture_PullDown_eFrameX2 = 7 */
   "X3 ",   /* BXDM_Picture_PullDown_eFrameX3 = 8 */
   "X1 ",   /* BXDM_Picture_PullDown_eFrameX1 = 9 */
   "X4 "    /* BXDM_Picture_PullDown_eFrameX4 = 10 */
};

/* SW7445-586: H265/HEVC split interlaced. Use an "s" to highlight that there are
 * two separate picture buffers.  Should only see Ts, Bs, TBs or BTs. */
static const char * const s_aSiPulldownToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "err",   /* invalid */
   "Ts  ",   /* BXDM_Picture_PullDown_eTop = 1 */
   "Bs  ",   /* BXDM_Picture_PullDown_eBottom = 2 */
   "TBs ",   /* BXDM_Picture_PullDown_eTopBottom = 3 */
   "BTs ",   /* BXDM_Picture_PullDown_eBottomTop = 4 */
   "TBTs",   /* BXDM_Picture_PullDown_eTopBottomTop = 5 */
   "BTBs",   /* BXDM_Picture_PullDown_eBottomTopBottom = 6 */
   "X2s ",   /* BXDM_Picture_PullDown_eFrameX2 = 7 */
   "X3s ",   /* BXDM_Picture_PullDown_eFrameX3 = 8 */
   "X1s ",   /* BXDM_Picture_PullDown_eFrameX1 = 9 */
   "X4s "    /* BXDM_Picture_PullDown_eFrameX4 = 10 */
};

/* SW7445-1638: for split interlaced, highlight when the repeat flag is
 * set to aid with debug. */
static const char * const s_aSiRepeatPulldownToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "err",   /* invalid */
   "Tsr ",   /* BXDM_Picture_PullDown_eTop = 1 */
   "Bsr ",   /* BXDM_Picture_PullDown_eBottom = 2 */
   "TBsr",   /* BXDM_Picture_PullDown_eTopBottom = 3 */
   "BTsr",   /* BXDM_Picture_PullDown_eBottomTop = 4 */
   "TBTsr",   /* BXDM_Picture_PullDown_eTopBottomTop = 5 */
   "BTBsr",   /* BXDM_Picture_PullDown_eBottomTopBottom = 6 */
   "X2sr",   /* BXDM_Picture_PullDown_eFrameX2 = 7 */
   "X3sr",   /* BXDM_Picture_PullDown_eFrameX3 = 8 */
   "X1sr",   /* BXDM_Picture_PullDown_eFrameX1 = 9 */
   "X4sr"    /* BXDM_Picture_PullDown_eFrameX4 = 10 */
};

#define BXVD_DECODER_S_MAX_VIDEO_PROTOCOL 21

static const char * const s_aProtocolToStrLUT[BXVD_DECODER_S_MAX_VIDEO_PROTOCOL] =
{
   "H264 ",          /* H.264 */
   "MPEG2",          /* MPEG-2 */
   "H261 ",          /* H.261 */
   "H263 ",          /* H.263 */
   "VC1  ",          /* VC1 Advanced profile */
   "MPEG1",          /* MPEG-1 */
   "MPEG2DTV",       /* MPEG-2 DirecTV DSS ES */
   "VC1SimpleMain",  /* VC1 Simple & Main profile */
   "MPEG4Part2",     /* MPEG 4, Part 2. */
   "AVS ",           /* AVS Jinzhun profile. */
   "MPEG2_DSS_PES",  /* MPEG-2 DirecTV DSS PES */
   "SVC ",           /* Scalable Video Codec */
   "SVC_BL",         /* Scalable Video Codec Base Layer */
   "MVC ",           /* MVC Multi View Coding */
   "VP6 ",           /* VP6 */
   "VP7 ",           /* VP7 */
   "VP8 ",           /* VP8 */
   "RV9 ",           /* Real Video 9 */
   "SPARK",          /* Sorenson Spark */
   "MJPEG",          /* Motion Jpeg */
   "HEVC"            /* H.265 */
};

static const char * const s_aOrientationToStrLUT[BXDM_Picture_Orientation_eMax] =
{
   "2D ",   /* BXDM_Picture_Orientation_e2D */
   "Chk",   /* BXDM_Picture_Orientation_e3D_Checker */
   "Col",   /* BXDM_Picture_Orientation_e3D_Column */
   "Row",   /* BXDM_Picture_Orientation_e3D_Row */
   "SbS",   /* BXDM_Picture_Orientation_e3D_SideBySide */
   "ToB",   /* BXDM_Picture_Orientation_e3D_TopBottom */
   "Ful"    /* BXDM_Picture_Orientation_e3D_FullFrame */
};

static const char * const s_aFrameRelationShipLUT[BXDM_Picture_FrameRelationship_eMax] =
{
   "-ukn",     /* BXDM_Picture_FrameRelationship_eUnknown */
   "-LR ",     /* BXDM_Picture_FrameRelationship_eFrame0Left */
   "-RL!"      /* BXDM_Picture_FrameRelationship_eFrame0Right */
};

static const char * const s_aPictureSetTypeToStrLUT[BXVD_Decoder_P_PictureSet_eMax] =
{
   "S",  /* BXVD_Decoder_P_PictureSet_eSingle */
   "B",  /* BXVD_Decoder_P_PictureSet_eBase */
   "e"   /* BXVD_Decoder_P_PictureSet_eDependent */
};


/*
 * Print routines.
 */

void BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Decoder_P_UnifiedPictureContext * pstContext,
   bool bDropped
   )
{
   BXDM_Picture * pstPicture;
   bool           bProtocolValid;

#if BXVD_P_PPB_EXTENDED
   uint32_t uiFlagsExt0 = pstContext->pPPB->flags_ext0;
#else
   uint32_t uiFlagsExt0 = 0;
#endif

   BDBG_ENTER( BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe );

   BDBG_ASSERT( pstContext );

   pstPicture = &(pstContext->stUnifiedPicture);

   bProtocolValid = ( pstPicture->stProtocol.eProtocol < BXVD_DECODER_S_MAX_VIDEO_PROTOCOL ) ? true : false ;

   BDBG_ASSERT( pstPicture );

   BDBG_MODULE_MSG( BXVD_UP, ("%c%03x:[%01x.%03x] idx:%d pts:%08x(%d) pPPB:%08x pd:%x flg:%08x ext0:%08x %s,%s %s %s %s%s tg:%x(%d) ck:%x %c:%02d",
                                 ( bDropped ) ? 'D' : ' ',
                                 hXvdCh->stDecoderContext.stCounters.uiVsyncCount & 0xFFF,
                                 hXvdCh->ulChannelNum & 0xF,
                                 pstPicture->uiSerialNumber & 0xFFF,
                                 pstContext->uiIndex,
                                 pstPicture->stPTS.uiValue,
                                 pstPicture->stPTS.bValid,
                                 pstContext->pPPBPhysical,
                                 pstContext->pPPB->pulldown,
                                 pstContext->pPPB->flags,
                                 uiFlagsExt0,
                                 s_aPictureCodingToStrLUT[  pstPicture->stPictureType.eCoding ],
                                 s_aPictureSetTypeToStrLUT[ pstContext->eSetType ],
                                 ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstPicture->stBufferInfo.eBufferFormat ) ?
                                    ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstPicture->stBufferInfo.eBufferHandlingMode ) ?
                                        s_aSiRepeatPulldownToStrLUT[ pstPicture->stBufferInfo.ePulldown ] :
                                            s_aSiPulldownToStrLUT[ pstPicture->stBufferInfo.ePulldown ] :
                                     s_aPulldownToStrLUT[ pstPicture->stBufferInfo.ePulldown ],
                                 ( bProtocolValid ) ? s_aProtocolToStrLUT[ pstPicture->stProtocol.eProtocol ] : "ukn" ,
                                 s_aOrientationToStrLUT[ pstPicture->st3D.eOrientation ],
                                 ( BXDM_Picture_Orientation_e2D == pstPicture->st3D.eOrientation ) ?
                                          " " : s_aFrameRelationShipLUT[ pstPicture->st3D.eFrameRelationship ],
                                 pstPicture->stPictureTag.uiValue,
                                 pstPicture->stPictureTag.bValid,
                                 pstPicture->uiChunkId,
                                 ( pstPicture->stError.bThisPicture ) ? 'E' : ' ',
                                 pstPicture->stError.uiPercentError
                                 ));

   BDBG_LEAVE( BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe );

   return;

}  /* end of BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe() */



void BXVD_DecoderDbg_P_PrintSeiMessage_isrsafe(
   BXVD_ChannelHandle hXvdCh,
   BXVD_P_SEI_Message * pSEIMessage,
   uint32_t uiSerialNumber
   )
{
   BDBG_ENTER( BXVD_DecoderDbg_P_PrintSeiMessage_isrsafe );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pSEIMessage );

   switch( pSEIMessage->uiMsgType )
   {
      case BXVD_P_PPB_SEI_MSG_MVC_GRAPHICS_OFFSET:
      {
         BXVD_P_MVC_Offset_Meta * pMetaData = (BXVD_P_MVC_Offset_Meta *)&(pSEIMessage->data.stOffsetMeta);

         BDBG_MODULE_MSG( BXVD_SEI, (" %04x:[%01x.%03x] GraphicsOffset: num values %d: %02x %02x %02x %02x ...",
                                 hXvdCh->stDecoderContext.stCounters.uiVsyncCount & 0xFFFF,
                                 hXvdCh->ulChannelNum & 0xF,
                                 uiSerialNumber & 0xFFF,
                                 pMetaData->size,
                                 pMetaData->offset[0],
                                 pMetaData->offset[1],
                                 pMetaData->offset[2],
                                 pMetaData->offset[3]
                                 ));

         break;
      }

      case BXVD_P_PPB_SEI_MSG_FRAMEPACKING:
      {
         BXVD_P_SEI_FramePacking * pstAvdSEIData = (BXVD_P_SEI_FramePacking *)&(pSEIMessage->data.stSEIFramePacking);

         BDBG_MODULE_MSG( BXVD_SEI, (" %04x:[%01x.%03x] FramePacking: flags:%08x arrange:%08x interp:%08x",
                                 hXvdCh->stDecoderContext.stCounters.uiVsyncCount & 0xFFFF,
                                 hXvdCh->ulChannelNum & 0xF,
                                 uiSerialNumber & 0xFFF,
                                 pstAvdSEIData->flags,
                                 pstAvdSEIData->frame_packing_arrangement_type,
                                 pstAvdSEIData->content_interpretation_type
                                ));
         break;
      }

      default:
         BDBG_MODULE_MSG( BXVD_SEI, (" %04x:[%01x.%03x] invalid SEI uiMsgType: %d",
                                 hXvdCh->stDecoderContext.stCounters.uiVsyncCount & 0xFFFF,
                                 hXvdCh->ulChannelNum & 0xF,
                                 uiSerialNumber & 0xFFF,
                                 pSEIMessage->uiMsgType
                                 ));
         break;
   }


   BDBG_LEAVE( BXVD_DecoderDbg_P_PrintSeiMessage_isrsafe );

   return;

}  /* end of BXVD_DecoderDbg_P_PrintSeiMessage() */



#endif   /* if debug build */
