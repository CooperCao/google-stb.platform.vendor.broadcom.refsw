/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef BHSM_USER_RSA_H__
#define BHSM_USER_RSA_H__

#include "bhsm.h"

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)

#include "bhsm_keyladder.h"
#include "bsp_s_user_rsa.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_USER_RSA_MAX_KEY_LEN             256      /* 2048 bit key -  size in bytes*/

typedef enum BHSM_USERRSA_RandomStall_e
{
    BHSM_USERRSA_RandomStall_eDisable    = 0,
    BHSM_USERRSA_RandomStall_e50PctOvr   = 1,
    BHSM_USERRSA_RandomStall_e25PctOvr   = 2,
    BHSM_USERRSA_RandomStall_e12PctOvr   = 3,

    BHSM_USERRSA_RandomStall_eMax        = 4
} BHSM_USERRSA_RandomStall_e ;


typedef struct BHSM_UserRSAIO
{
    /* input */
    bool                        bHWModExpProtect;      /* anti-DPA   - ModExp protection enable */
    BHSM_USERRSA_RandomStall_e  HWRandomStall;         /* anti-DPA   - Random Stall insertion      */
    bool                        bFWRSAMode1;           /* FW exponet blinding enable - private RSA key protection */
    BCMD_User_RSA_Size_e        rsaKeySize;
    BCMD_User_RSA_SubCommand_e  rsaSubCmd;
    unsigned char               rsaData[BHSM_USER_RSA_MAX_KEY_LEN];

    /* Output */
    uint32_t                    unStatus;             /* DEPRECATED */
    uint32_t                    unOutputDataLen;      /* 128 bytes for 1024 bit key; 256 bytes for 2048 bit key  */
    unsigned char               aucOutputData[BHSM_USER_RSA_MAX_KEY_LEN]; /* actual byte stream of returned RSA output data */

} BHSM_UserRSAIO_t;


/*****************************************************************************
Summary:

This function is provided for MIPS host to use the PKE RSA engine inside BSP to do RSA Public key based verification(decryption),
RSA private key based signing (encryption). RSA cryptographic functionality could be disabled via setting specific MSP bits.
Since the performance is not very fast, may not be widely used.

Though this function can be used to do RSA encryprtion using public key and RSA decryption using private key, it's rarely
used this way due to its very slow processing speed compared to other regular (de)encryption algorithms.

Description:


Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the
PKE RSA engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  No testing data is available, and no
estimated time either. Here's one for the reference.
PublicKeyVerify,   n=256 bytes, e=4 bytes, Msg=256bytes    time= 9270 us  rate = 220.9 kbps


Input:
hHsm             - BHSM_Handle, Host Secure module handle.
inoutp_userRsaIO  - BHSM_UserRSAIO_t, members in the input section are always needed,

Output:
inoutp_userRsaIO  - BHSM_UserRSAIO_t, members in output section

    'unStatus' is modified to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.
    'unOutputDataLen' is 16 bit value, in bits, the length of returned cleartext or ciphertext
    'aucOutputData' contains the returned PKE processed cleartext or ciphertext

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
*****************************************************************************/
BERR_Code      BHSM_UserRSA (
        BHSM_Handle        hHsm,
        BHSM_UserRSAIO_t  *pRsa
);

#ifdef __cplusplus
}
#endif


#endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)  */

#endif /* BHSM_USER_CMDS_H__ */
