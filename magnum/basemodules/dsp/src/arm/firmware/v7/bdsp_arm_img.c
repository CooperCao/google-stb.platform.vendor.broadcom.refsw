/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/


#include "bdsp.h"
/*#include "../include/bdsp_arm.h"*/ /* FIXME: Remove the _priv dependency */
#include "bdsp_arm_img.h"

BDBG_MODULE(bdsp_arm_img);


extern const void * BDSP_ARM_IMG_system_code [];
extern const void * BDSP_ARM_IMG_hbc_monitor_code [];

#if 0
/*extern const void * BDSP_IMG_system_rdbvars [];*/
extern const void * BDSP_ARM_IMG_algolib_code[];
extern const void * BDSP_ARM_IMG_syslib_code [];
extern const void * BDSP_ARM_IMG_idscommon_code [];

extern const void * BDSP_ARM_IMG_vididscommon_code[];
extern const void * BDSP_ARM_IMG_vididscommon_inter_frame[];
extern const void * BDSP_ARM_IMG_scm_task_code [];
extern const void * BDSP_ARM_IMG_video_decode_task_code [];
extern const void * BDSP_ARM_IMG_video_encode_task_code [];
extern const void * BDSP_ARM_IMG_cdb_passthru_code[];
extern const void * BDSP_ARM_IMG_cdb_passthru_tables[];
extern const void * BDSP_ARM_IMG_cdb_passthru_inter_frame[];
#endif

#ifdef BDSP_ARM_DDPENC_SUPPORT
/*extern const void *BDSP_ARM_IMG_ddp_encode_code[];*/
extern const void *BDSP_ARM_IMG_ddp_encode_tables[];
extern const void *BDSP_ARM_IMG_ddp_encode_inter_frame[];
#endif

static void *BDSP_ARM_IMG_P_GetArray(unsigned imgId)
{
    switch ( imgId )
    {
        /* Special cases for system ID's (don't follow standard AlgoId->img convention) */
        case BDSP_ARM_SystemImgId_eSystemCode:                                  return BDSP_ARM_IMG_system_code;
		case BDSP_ARM_SystemImgId_eHbcMonitorCode:                              return BDSP_ARM_IMG_hbc_monitor_code;
#if 0
        /*case BDSP_SystemImgId_eSystemRdbvars:                               return BDSP_IMG_system_rdbvars;*/
        case BDSP_ARM_SystemImgId_eSyslibCode:                                  return BDSP_ARM_IMG_syslib_code;
        case BDSP_ARM_SystemImgId_eAlgolibCode:                                 return BDSP_ARM_IMG_algolib_code;
        case BDSP_ARM_SystemImgId_eCommonIdsCode:                               return BDSP_ARM_IMG_idscommon_code;

        case BDSP_SystemImgId_eCommonVideoEncodeIdsCode:                    return BDSP_IMG_vididscommon_code;
        case BDSP_SystemImgId_eCommonVideoEncodeIdsInterframe:              return BDSP_IMG_vididscommon_inter_frame;
        case BDSP_SystemImgId_eScm_Task_Code:                               return BDSP_IMG_scm_task_code;
#ifdef BDSP_SCM1_SUPPORT
        case BDSP_SystemImgId_eScm1_Digest:                                 return BDSP_IMG_scm1_digest;
#endif
#ifdef BDSP_SCM2_SUPPORT
        case BDSP_SystemImgId_eScm2_Digest:                                 return BDSP_IMG_scm2_digest;
#endif
        case BDSP_SystemImgId_eVideo_Decode_Task_Code:                      return BDSP_IMG_video_decode_task_code;
        case BDSP_SystemImgId_eVideo_Encode_Task_Code:                      return BDSP_IMG_video_encode_task_code;
#endif
#ifdef BDSP_ARM_DDPENC_SUPPORT
        /*case BDSP_ARM_IMG_ID_CODE(BDSP_ARM_AF_P_AlgoId_eDDPEncode):             return BDSP_ARM_IMG_ddp_encode_code;*/
        case BDSP_ARM_IMG_ID_TABLE(BDSP_ARM_AF_P_AlgoId_eDDPEncode):            return BDSP_ARM_IMG_ddp_encode_tables;
        case BDSP_ARM_IMG_ID_IFRAME(BDSP_ARM_AF_P_AlgoId_eDDPEncode):           return BDSP_ARM_IMG_ddp_encode_inter_frame;
#endif

        default:
            BDBG_WRN(("IMG %u not supported (algo %u)", imgId, BDSP_ARM_IMG_ID_TO_ALGO(imgId)));
            return NULL;
    }
 }

 /*  This context is used by other modules to make use of this interface. They
     need to supply this as a parameter to BDSP_IMG_Open */
 static void *pDummy = NULL;
 void *BDSP_ARM_IMG_Context = &pDummy;


 /* This function has to be used for opening a firmware image. The pointer to the
     firmware image array is given that contains the header and the chunks.
     This works based on the firmware image id  that is supplied.
 */

 static BERR_Code BDSP_ARM_IMG_Open(void *context, void **image, unsigned image_id)
 {
     BSTD_UNUSED(context);
     BDBG_ASSERT(NULL != context);

     *image = BDSP_ARM_IMG_P_GetArray(image_id);

     if (NULL == *image)
     {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
     }

     return BERR_SUCCESS;
 }

 /*  After opening the firmware image, the user of this interface will then be
     interested in getting the chunks and the header giving information about the
     chunks.
 */

 static BERR_Code BDSP_ARM_IMG_Next(void *image, unsigned chunk, const void **data, uint16_t length)
 {

     BDBG_ASSERT(data);
     BSTD_UNUSED(length);

     *data = ((const void **)image)[chunk];

     if (NULL == *data)
     {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
     }

     return BERR_SUCCESS;
 }

 /* This function is used to close the firmware image that was opened using
     BDSP_ARM_IMG_Open */
 static void BDSP_ARM_IMG_Close(void *image)
 {

     BSTD_UNUSED(image);
     return;
 }

 /* The interface is actually a set of three functions open, next and close.
     The same has been implemented here and their function pointers supplied */
 const BIMG_Interface BDSP_ARM_IMG_Interface = {
     BDSP_ARM_IMG_Open,
     BDSP_ARM_IMG_Next,
     BDSP_ARM_IMG_Close
     };
