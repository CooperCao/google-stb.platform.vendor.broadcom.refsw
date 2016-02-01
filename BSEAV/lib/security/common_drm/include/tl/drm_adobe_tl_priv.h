/******************************************************************************
 *    (c)2013 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 *****************************************************************************/
/*
*   The Following defines must match the AXS HW API list
*/
#define DrmAdobe_CAPABILITY_HW_ROOT_OF_TRUST                (1 << 0)    /*< HW Supports a HW root of trust */
#define DrmAdobe_CAPABILITY_CAN_DO_HW_DECODE_AFTER_DECRYPT    (1 << 1)    /*< HW can pass decrypted samples directly to decoder */
#define DrmAdobe_CAPABILITY_HDCP_SUPPORT                    (1 << 2)    /*< HW supports HDCP output protection */
#define DrmAdobe_CAPABILITY_DEVICE_KEY_BINDING_SUPPORT        (1 << 3)    /*< HW supports custom binding of keys to the device */
#define DrmAdobe_CAPABILITY_ECC_CRYPTO                        (1 << 4)    /*< HW supports ECC (eliptical curve cryptography) */
#define DrmAdobe_CAPABILITY_API_AUTHENTICATION                (1 << 5)    /*< HW supports API Authentication */
#define DrmAdobe_CAPABILITY_TAKES_ENTROPY                    (1 << 6)    /*< HW supports taking external entropy */
#define DrmAdobe_CAPABILITY_HW_NEEDS_CTX                    (1 << 7)    /*< HW needs a CTX to be setup for all decrypt APIs */
#define DrmAdobe_CAPABILITY_HW_CAN_DO_PARTIAL_DECRYPT        (1 << 8)    /*< HW supports use of the Init, Update, and Final APIs for decryption of partial samples */
#define DrmAdobe_CAPABILITY_HW_HANDLES_MPEG2TS                (1 << 9)    /*< HW can decrypt and decode MPEG2-TS */
#define DrmAdobe_CAPABILITY_HW_HANDLES_MP4                    (1 << 10)    /*< HW can decrypt and decpde MP4 containers */
#define DrmAdobe_CAPABILITY_HW_HANDLES_HLS                    (1 << 11)    /*< HW can decrypt and decode HLS containers */
#define DrmAdobe_CAPABILITY_HW_REQUIRES_AES_MODE_AT_LOAD    (1 << 12)    /*< HW requires that the AES mode for decryption of content be set when the LoadKey API is called */
#define DrmAdobe_CAPABILITY_HW_ALLOWS_RAW_RSA_OPERATIONS    (1 << 13)    /*< HW exposes raw RSA primatives */
#define DrmAdobe_CAPABILITY_ASYNC_DRM_KEY                    (1 << 14)    /*< HW DRM key uses asynchronous crypto */
#define DrmAdobe_CAPABILITY_SYNC_DRM_KEY                    (1 << 15)    /*< HW DRM key uses synchronous crypto */
/* The next three bits are mutually exclusive. Only one memory management scheme can be asserted */
#define DrmAdobe_CAPABILITY_GLOBAL_MALLOC                    (1 << 16)    /**< Access framework or host runtime mallocs and frees everything. */
#define DrmAdobe_CAPABILITY_BI_DIRECTIONAL_MALLOC            (1 << 17)    /**< Buffers passed into sample decrypt are malloced by Access, buffers passed out malloced by HW */
#define DrmAdobe_CAPABILITY_HW_API_MALLOC                    (1 << 18)    /**< Malloc handled by HW API provided by vendor */
#define DrmAdobe_CAPABILITY_PKCS8_IMPORT_SUPPORT            (1 << 19)    /**< HW can handle PKCS8 structure */
#define DrmAdobe_CAPABILITY_DUAL_DRM_KEY_SUPPORT            (1 << 20)    /**< HW has been provisioned with two DRM keys. One for signing, one for encryption */


typedef uint64_t DrmAdobe_DeviceCapabilities;
