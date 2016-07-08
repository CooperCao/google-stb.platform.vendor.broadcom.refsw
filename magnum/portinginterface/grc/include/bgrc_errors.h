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
 * Module Description:
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
