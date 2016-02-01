/***************************************************************************
 *     Copyright (c) 2004-2006, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BGRC_ERRORS_H__
#define BGRC_ERRORS_H__

#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BGRC_ERR_NO_OUTPUT_SURFACE               BERR_MAKE_CODE(BERR_GRC_ID, 0x0000)
#define BGRC_ERR_SOURCE_RECT_OUT_OF_BOUNDS       BERR_MAKE_CODE(BERR_GRC_ID, 0x0001)
#define BGRC_ERR_DESTINATION_RECT_OUT_OF_BOUNDS  BERR_MAKE_CODE(BERR_GRC_ID, 0x0002)
#define BGRC_ERR_OUTPUT_RECT_OUT_OF_BOUNDS       BERR_MAKE_CODE(BERR_GRC_ID, 0x0003)
#define BGRC_ERR_SOURCE_DIMENSIONS_INVALID       BERR_MAKE_CODE(BERR_GRC_ID, 0x0004)
#define BGRC_ERR_DESTINATION_DIMENSIONS_INVALID  BERR_MAKE_CODE(BERR_GRC_ID, 0x0005)
#define BGRC_ERR_OUTPUT_DIMENSIONS_INVALID       BERR_MAKE_CODE(BERR_GRC_ID, 0x0006)
#define BGRC_ERR_YCBCR422_SURFACE_HAS_ODD_EDGE   BERR_MAKE_CODE(BERR_GRC_ID, 0x0007)
#define BGRC_ERR_MAX_SCALE_DOWN_LIMIT_EXCEEDED   BERR_MAKE_CODE(BERR_GRC_ID, 0x0008)
#define BGRC_ERR_M2MC_DEVICE_IS_HUNG             BERR_MAKE_CODE(BERR_GRC_ID, 0x0010)
#define BGRC_ERR_M2MC_DEVICE_NUM_INVALID         BERR_MAKE_CODE(BERR_GRC_ID, 0x0011)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_ERRORS_H__ */

/* End of file. */
