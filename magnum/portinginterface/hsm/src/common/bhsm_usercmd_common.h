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


#include "bhsm.h"
#include "bhsm_bsp_interface_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BHSM_USERCMD_COMMON_H__
#define BHSM_USERCMD_COMMON_H__

#define UINT32_SIZE_IN_BYTES            4
#define BSP_NEED_MORE_DATA_PKE          0xa3
#define BSP_PKE_IN_PROGRESS             0xa4
#define BSP_NEED_MORE_DATA_HMACSHA1     0xa8
#define BHSM_MAX_EMBEDDED_DATA_LEN      336
#define BHSM_MAX_DMAAES_DATA_LEN        32768       /* 32 KB */


/* Module Specific Functions */

void BHSM_Pack_8BIT_TO_32BIT(
    uint32_t      *p_dest,
    unsigned char *p_src,
    unsigned int  len
);
void BHSM_UnPack_32BIT_TO_8BIT(
    unsigned char *p_dest,
    uint32_t      *p_src,
    unsigned int  len
);

void BHSM_MemcpySwap (
    unsigned char *pDest,
    unsigned char *pData,
    unsigned int  len,
    bool swap
);



typedef enum BHSM_PollingTarget_e
{
    BHSM_PollingTarget_eSecureRSA,
    BHSM_PollingTarget_eUserRSA,
    BHSM_PollingTarget_eMax

} BHSM_PollingTarget_e;


#define BHSM_SECURE_PKE_OUTPUT_LEN			384


typedef struct BHSM_PKEPollingCmdIO {

    /* In: Command target to poll for */
	BHSM_PollingTarget_e                    pollTarget;         /* to be used with Zeus 3.0 and above */

	/* In: Command code to poll for completion */
	uint32_t								cmdToPoll;

	/* Out: 0 for Completion, 0xA4 for still in progress,  otherwise failed. */
	uint32_t								unStatus;

	/* Out: output data size - in bytes - likely 128, 256, or 160 bytes  */
	uint32_t								outputBufSize;

	/* Out: output data - 1024, or 2048, or 1280 bit data   */
	unsigned char							aucOutputBuf[BHSM_SECURE_PKE_OUTPUT_LEN];

} BHSM_PKEPollingCmdIO_t;


/*****************************************************************************
Summary:


Description:


Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the
RSA crypto engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.


Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_PKEPollingCmdIO  - BHSM_PKEPollingCmdIO_t,

Output:
inoutp_PKEPollingCmdIO  - BHSM_PKEPollingCmdIO_t, the member in output section

	'unStatus' is modified to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:

*****************************************************************************/
BERR_Code      BHSM_PKEPollingCmd(
		BHSM_Handle				in_handle,
		BHSM_PKEPollingCmdIO_t	*inoutp_PKEPollingCmdIO
);





/* End of Module Specific Functions */



#ifdef __cplusplus
}
#endif

#endif /* BHSM_USERCMD_COMMON_H__ */
