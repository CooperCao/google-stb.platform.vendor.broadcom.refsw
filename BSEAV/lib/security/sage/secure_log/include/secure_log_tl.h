/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef SECURE_LOG_TL_H__
#define SECURE_LOG_TL_H__

#include "bstd.h"
#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Usage Guide
 *-------------
 *
 * In order to use Secure Log for logging Sage Framework (SSF) or any TA
 * Following are the requirements and API calling sequence.
 * (a) First make sure to have a valid certificate
 * (b) Also make sure that Valid Secure Log Type/Enable flag is in SSF header
 *     or TA header for Logging to get started.
 *
 * API Call sequence -
 *  (a) First call "Secure_Log_TlConfigureBuffer" API and provide valid
 *      Certificate binary file and size along with desired Logging Buffer Size
 *      Currently Buffer Size is limited to Max 1MB.
 *  (b) Call "Secure_Log_TlAttach" API with desired TA/Platform ID or SSF
 *      (Platform Id = 0) for which logging needs to be started.
 *      Note: Two conditions needs to be true for logging to really get started
 *      (i) TA or SSF header should have correct Secure Log Type Flag
 *      (ii) The Log Buffer is properly configured for SDL ID to which this TA ID
 *           belongs.
 *  (c) Call "Secure_Log_TlGetBuffer" API by providing
 *       (i) "Secure_Log_TlBufferContext" which is  GLR memory to hold the Log Header
 *       (ii) "pGLRBufferAddr" which points to GLR memory to hold the Encrypted Log Data
 *       (iii) "GLRBufferSize" This size needs to be same as while configuring Log Buffer
 *       (iv) "TA_Id" This is the Platform ID for which Data is needed.
 *        Note: Encrypted Data will be for all the TA's which belongs to that SDL range
 *              so even though TA ID is passed the encrypted data will contain all
 *              the logs of TA's which are logging and belongs to same SDL ID.
 *  (d) "Secure_Log_TlDetach" API provides flexibility to User to stop logging
 *      any TA or SSF during run time.
 *
 *
 * Simple Example:
 *
 * Secure_Log_TlConfigureBuffer(CertBinforSDL81, CertBinSizeSDL81, 0x1000);
 * Secure_Log_TlAttach(0);
 * Secure_Log_TlGetBuffer(pSecureLogBuffCtx,pGLRBufferAddr,0x1000,0);
 *
 *
 */

/* Each log is appended to below header. rootkeysel, sdl_id and seclogtype,
global own id, otp sel id all information will be filled after encryption */
typedef struct {
    uint8_t header_ver;
    uint8_t secure_log_type;
    uint8_t sdl_id;
    uint8_t root_key_sel;
    uint8_t opt_id_select;
    uint8_t global_owner_id;
    uint8_t rsvd0;
    uint8_t rsvd1;
    uint32_t secure_logbuf_len;
    uint32_t secure_logtotal_cnt;
}Secure_Log_TlSubH;

typedef struct {
    Secure_Log_TlSubH secHead;
    uint8_t EncLSEI[16];
    uint8_t KeyProcIn1[16];
    uint8_t KeyProcIn2[16];
    uint8_t KeyProcIn3[16];
}Secure_Log_TlBufferContext;

BERR_Code Secure_Log_TlInit(void);

BERR_Code Secure_Log_TlUninit(void);

/*
 * All the memory blocks are passed by application by allocating memory
 * using SRAI_Memory_Allocate API.
 */
BERR_Code Secure_Log_TlConfigureBuffer(uint8_t *certificateBin, uint32_t certificateSize, uint32_t ringBuffSize);

BERR_Code Secure_Log_TlAttach(uint32_t TA_Id);

BERR_Code Secure_Log_TlDetach(uint32_t TA_Id);

/* GLRBufferSize should be equal to RingBuffSize,
 * for Buffer attached to the TA Id. All the memory blocks
 * are passed by application by allocating memory using SRAI_Memory_Allocate API.
 */
BERR_Code Secure_Log_TlGetBuffer(Secure_Log_TlBufferContext *pSecureLogBuffCtx, uint8_t *pGLRBufferAddr, uint32_t GLRBufferSize, uint32_t TA_Id);

#ifdef __cplusplus
}
#endif

#endif /*SECURE_LOG_TL_H__*/
