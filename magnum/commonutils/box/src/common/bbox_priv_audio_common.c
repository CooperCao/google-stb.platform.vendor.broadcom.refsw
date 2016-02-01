/***************************************************************************
 *     Copyright (c) 2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************/

#include "bstd.h"
#include "bbox.h"
#include "bkni.h"
#include "bchp_common.h"

BERR_Code BBOX_P_Audio_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Audio_Capabilities *pBoxAudio )
{
   BKNI_Memset( pBoxAudio, 0, sizeof(*pBoxAudio));

/* use RDB to determine default number of DSP's, but allow box mode to reduce */
#ifdef BCHP_RAAGA_DSP_MISC_1_REG_START
    pBoxAudio->numDsps = 2;
#else
    pBoxAudio->numDsps = 1;
#endif

#if BCHP_CHIP == 7445
    switch (ulBoxId) {
    case 2: /* 7252 box modes */
    case 4:
    case 5:
    case 6:
    case 10:
    case 1001:
        pBoxAudio->numDsps = 1;
        break;
    default:
        break;
    }
#elif BCHP_CHIP == 7439
    switch (ulBoxId) {
    case 1: /* 7251S box modes */
    case 12:
    case 13:
    case 17:
    case 19:
    case 20:
        pBoxAudio->numDsps = 1;
        break;
    default:
        break;
    }
#else
    BSTD_UNUSED(ulBoxId);
#endif
   return BERR_SUCCESS;
}
