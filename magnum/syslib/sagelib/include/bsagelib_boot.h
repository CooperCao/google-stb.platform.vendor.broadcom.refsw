/******************************************************************************
 * (c) 2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BSAGELIB_BOOT_H_
#define BSAGELIB_BOOT_H_

#include "bsagelib_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/* SAGElib boot settings
 * This structure is used to provide SAGE binaries and general 'middleware' configuration to SAGE system library
 * So that BSAGElib can kick off SAGE system. */
typedef struct
{
    /* The image buffers shall be allocated in a global region (general heap or other global heaps)    */
    uint8_t *pBootloader; /* SAGE bootloader image loaded into memory.                    */
    uint32_t bootloaderSize;

    uint8_t *pOsApp;      /* SAGE OS/APP image loaded into memory.                        */
    uint32_t osAppSize;

    /* Global Regions - 0: main, 1: client/secondary */
    uint64_t GLR0Offset;
    uint32_t GLR0Size;
    uint64_t GLR1Offset;
    uint32_t GLR1Size;

    /* SAGE Restricted Region */
    uint64_t SRROffset;
    uint32_t SRRSize;

    /* Compressed Restricted Region */
    uint64_t CRROffset;
    uint32_t CRRSize;

    /* Uncompressed Restricted Regions - one per memory controler */
    uint64_t URR0Offset;
    uint32_t URR0Size;
    uint64_t URR1Offset;
    uint32_t URR1Size;
    uint64_t URR2Offset;
    uint32_t URR2Size;

    /* Buffer holding the parameters of SAGE log buffer*/
    uint64_t logBufferOffset;
    uint32_t logBufferSize;

} BSAGElib_BootSettings;

typedef struct
{
    BSAGElib_BootStatus status; /* defined in bsagelib_types.h */
    uint32_t lastError;
} BSAGElib_BootState;


/* Get the default values for the BSAGElib_BootSettings structure
 * This must be use prior to call Boot Launc to ensure compatibility with new versions */
void
BSAGElib_Boot_GetDefaultSettings(
    BSAGElib_BootSettings *pBootSettings /* [in/out] */);

/* Launch the Given SAGE Firmwares:
 *  - Parse given binaries headers
 *  - set the Sage parameters through the SAGE Global Ram
 *  - Starts the SAGE Bootloader
 */
BERR_Code
BSAGElib_Boot_Launch(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootSettings *pBootSettings /* [in] */);

/* Get current boot status
 * An upper layer monitoring thread shall call this API
 * in order to determine if SAGE had booted.
 * Once booted or failed to, BSAGElib_Boot_Clean() shall be call. */
void
BSAGElib_Boot_GetState(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootState *pState /* [out] */);

/* Clean any ressource allocated during Launch process; by BSAGElib_Boot_Launch() */
void
BSAGElib_Boot_Clean(
    BSAGElib_Handle hSAGElib);

void
BSAGElib_Boot_GetBinariesVersion(
    BSAGElib_Handle hSAGElib,
    char **ppBLVer,
    char **ppOSVer);

BERR_Code
BSAGElib_Boot_Post(
    BSAGElib_Handle hSAGElib);

#ifdef __cplusplus
}
#endif

#endif /* BSAGELIB_BOOT_H_ */
