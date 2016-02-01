/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/

#ifndef BHSM_BSP_INTERFACE_LAGACY_H__
#define BHSM_BSP_INTERFACE_LAGACY_H__

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bmem.h"
#include "berr_ids.h"
#include "bhsm_datatypes.h"
#include "bsp_s_keycommon.h"
#include "bsp_s_commands.h"
#include "bsp_s_hw.h"
#include "bhsm_private.h"


#ifdef __cplusplus
extern "C" {
#endif

/*  Sage uses OLOAD1 vector; host uses OLOAD2 */
#if BHSM_SAGE_BSP_PI_SUPPORT
#define HSM_L2_INTR     BCHP_INT_ID_BSP_OLOAD1_INTR
#else
#define HSM_L2_INTR     BCHP_INT_ID_BSP_OLOAD2_INTR
#endif



/***************************************************************************
Summary:
Structure that defines Host Secure module handle.

Description:
Structure that defines Host Secure module handle.

See Also:
BHSM_Open()

****************************************************************************/
 typedef struct BHSM_P_CommandData
{
        BCMD_cmdType_e          cmdId;  /* Command id == tag id */
        uint32_t                unContMode;

        bool                    bLockWaive;

        /* This length (in bytes) does not include header . Max is 364 bytes. Padded with zero to make it word aligned*/
        uint32_t                unInputParamLen;
        uint32_t                unInputParamsBuf[BCMD_BUFFER_BYTE_SIZE/4];

        /* The following is for automated test */
        /* This length (in bytes) does not include header . Max is 364 bytes. Padded with zero to make it word aligned*/
        uint32_t                unOutputParamLen;
        uint32_t                unOutputParamsBuf[BCMD_BUFFER_BYTE_SIZE/4];

} BHSM_P_CommandData_t;



void BHSM_P_IntHandler_isr( void *inp_param1, int in_param2 );

/* Use this function to submit cmds.  This function takes care of continual modes, owner id, version number.
ucInCmdData includes parameter length and on.  After checking output cmd, return to the caller.
Need a mutex to protect the increment seq number.
Continual mode for diff cmds could be tricky: TBD.
Version number is from a shared aegis file ???
return error=> other than status, everything is undetermined.
 */
#if BHSM_SAGE_BSP_PI_SUPPORT

BERR_Code BHSM_P_SubmitCommand (
        struct BHSM_P_ChannelHandle     *    in_channelHandle,
        uint32_t    *inCmdBuf,
        uint32_t    *outCmdBuf
);

#else

BERR_Code BHSM_P_SubmitCommand (
        struct BHSM_P_ChannelHandle     *in_channelHandle
);

#endif

BERR_Code BHSM_P_CommonSubmitCommand (
        struct BHSM_P_ChannelHandle     *in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
);

BERR_Code BHSM_P_SubmitCommand_DMA (
        struct BHSM_P_ChannelHandle     *in_channelHandle
);


BERR_Code BHSM_P_CommonSubmitCommand_DMA (
        struct BHSM_P_ChannelHandle     *in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
);



#ifdef BHSM_AUTO_TEST
BERR_Code BHSM_P_CommonSubmitRawCommand (
        struct BHSM_P_ChannelHandle     *in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
);
#endif


#ifdef __cplusplus
}
#endif

#endif /* BHSM_BSP_INTERFACE_LAGACY_H__ */
