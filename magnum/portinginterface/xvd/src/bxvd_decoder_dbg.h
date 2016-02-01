/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

#ifndef BXVD_DECODER_DBG_H__
#define BXVD_DECODER_DBG_H__

#include "bxvd.h"

#ifdef __cplusplus
extern "C" {
#endif

#if BDBG_DEBUG_BUILD

void BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Decoder_P_UnifiedPictureContext * pstUnifiedContext,
   bool bDropped
   );

void BXVD_DecoderDbg_P_PrintSeiMessage_isrsafe(
   BXVD_ChannelHandle hXvdCh,
   BXVD_P_SEI_Message * pSEIMessage,
   uint32_t uiSerialNumber
   );

#else

#define BXVD_DecoderDbg_P_PrintUnifiedPicture_isrsafe( hXvdCh, pstUnifiedContext, bDropped )
#define BXVD_DecoderDbg_P_PrintSeiMessage_isrsafe( hXvdCh, pSEIMessage, uiSerialNumber )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_DECODER_DBG_H__ */
