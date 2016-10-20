/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * $brcm_Workfile: sage_manufacturing_module_ids.h $
 * $brcm_Revision: 1 $
 * $brcm_Date: 1/14/14 12:40p $
 *
 * Module Description: Manufacturing Platform for SAGE - shared IDs
 *
 * Revision History:
 *
 * $brcm_Log: /BSEAV/lib/security/sage/manufacturing\sage_manufacturing_module_ids.h $
 *
 *
 *
 **************************************************************************/

#ifndef SAGE_MANUFACTURING_MODULE_IDS_H__
#define SAGE_MANUFACTURING_MODULE_IDS_H__

/* Manufacturing platform module IDs */

enum {
    PROVISIONING_MODULE = 0x1,
    VALIDATION_MODULE = 0x2
};


/* module command list */

enum {

    /* Provisioning module commands start at 0x0001   */
    PROVISIONING_COMMAND_ProcessBinFile   = 0x0001,

    /* Validation module commands start at 0x1001   */
    VALIDATION_COMMAND_ValidateHdcp22   = 0x1001,

    /* Validation module commands start at 0x1001   */
    VALIDATION_COMMAND_ValidateEdrm   = 0x1002,

     /* Validation module commands start at 0x1001   */
    VALIDATION_COMMAND_ValidateEcc   = 0x1004,

     /* Validation module commands start at 0x1001   */
    VALIDATION_COMMAND_ValidateDtcpIp   = 0x1008
};


#endif /* #ifndef SAGE_MANUFACTURING_MODULE_IDS_H__ */
