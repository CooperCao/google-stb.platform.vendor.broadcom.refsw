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

/* base modules */
#include "bstd.h"           /* standard types */
#include "bvce_core.h"
#include "bvce_fw_api.h"

const BVCE_P_SupportedProtocolEntry BVCE_P_SupportedProtocols[] =
{
   { BAVC_VideoCompressionStd_eH264, ENCODING_STD_H264 },
   { BAVC_VideoCompressionStd_eMPEG2, ENCODING_STD_MPEG2},
   { BAVC_VideoCompressionStd_eVP8, ENCODING_STD_VP8 },

   /* The following should always be the last entry */
   { BAVC_VideoCompressionStd_eMax, BVCE_P_VIDEOCOMPRESSIONSTD_UNSUPPORTED}
};
