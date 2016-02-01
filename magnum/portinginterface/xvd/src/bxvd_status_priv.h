/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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

#ifndef BXVD_STATUS_PRIV_H__
#define BXVD_STATUS_PRIV_H__

#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

typedef struct BXVD_P_Status_Context
{
      BXVD_Handle hXvd;
      
      uint32_t auiOpenChannelCount[BXVD_MAX_VIDEO_CHANNELS];
      uint32_t auiCloseChannelCount[BXVD_MAX_VIDEO_CHANNELS];
      uint32_t auiStartDecodeCount[BXVD_MAX_VIDEO_CHANNELS];
      uint32_t auiStopDecodeCount[BXVD_MAX_VIDEO_CHANNELS];
      
      BXVD_ChannelStatus astChannelStatus[BXVD_MAX_VIDEO_CHANNELS];
} BXVD_P_Status_Context;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_STATUS_PRIV_H__ */
/* End of File */
