/******************************************************************************
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
 ******************************************************************************/


/* This file must be compatible with Assembly */


#ifndef BSAGELIB_SHARED_GLOBALSRAM_H_
#define BSAGELIB_SHARED_GLOBALSRAM_H_

#include "bchp_scpu_globalram.h"

/*
 * SAGE global SRAM register indexes
 */

/* From 0x00 to 0x19 : Read only values */

#define BSAGElib_GlobalSram_eCRRStartOffset             0x04 /* Set by BSP: physical address of the first byte of CRR */
#define BSAGElib_GlobalSram_eCRREndOffset               0x05 /* Set by BSP: physical address of the last byte of CRR */
#define BSAGElib_GlobalSram_eSRRStartOffset             0x06 /* Set by BSP: physical address of the first byte of SRR */
#define BSAGElib_GlobalSram_eSRREndOffset               0x07 /* Set by BSP: physical address of the last byte of SRR */


/* From 0x20 to 0x69 is the Host < -- > SAGE scratchdpad */

    /* scheme versioning defines how the GlobalSram is used and what to expect at which index */
#define BSAGElib_GlobalSram_eSchemeVersion              0x20

    /* SAGE < -- > Host communication buffers */
#define BSAGElib_GlobalSram_eHostSageBuffers            0x21
#define BSAGElib_GlobalSram_eHostSageBuffersSize        0x22

    /* Regions configuration */
#define BSAGElib_GlobalSram_eRegionMapOffset            0x23
#define BSAGElib_GlobalSram_eRegionMapSize              0x24

    /* SAGE Secure Logging */
#define BSAGElib_GlobalSram_eSageLogBufferOffset        0x25
#define BSAGElib_GlobalSram_eSageLogBufferSize          0x26
#define BSAGElib_GlobalSram_eSageLogWriteCountLSB       0x27
#define BSAGElib_GlobalSram_eSageLogWriteCountMSB       0x28

    /* SAGE Anti Rollback */
#define BSAGElib_GlobalSram_eSageBootloaderEpochVersion 0x29
#define BSAGElib_GlobalSram_eSageFrameworkEpochVersion  0x2A

    /* Control SAGE life cycle */
#define BSAGElib_GlobalSram_eReset                      0x2B /* DO NOT MOVE : this is used by BCHP and SAGE for restart/reset feature */
#define BSAGElib_GlobalSram_eSuspend                    0x2C

    /* Sage Status and versioning */
#define BSAGElib_GlobalSram_eLastError                  0x2D
#define BSAGElib_GlobalSram_eBootStatus                 0x2E
#define BSAGElib_GlobalSram_eSageBootloaderVersion      0x2F
#define BSAGElib_GlobalSram_eSageFrameworkVersion       0x30

    /* Misc */
#define BSAGElib_GlobalSram_eSageStatusFlags            0x31

    /* SAGE Services parameters - resources */
#define BSAGElib_GlobalSram_eSageVklMask                0x32
#define BSAGElib_GlobalSram_eSageDmaChannel             0x33

    /* SAGE Secure Boot */
    /* TODO TBD: Total: 28 (0x1e indexes
       step1: replace those entries by a structure pointed to GLR
       step2: replace all of this with a pointer to the header
              (BL with authenticate header etc */
#define BSAGElib_GlobalSram_eSageFrameworkBin           0x34
#define BSAGElib_GlobalSram_eSageFrameworkBinSize       0x35
#define BSAGElib_GlobalSram_eSageFrameworkBinSignature  0x36
#define BSAGElib_GlobalSram_eTextSectionSize            0x37
#define BSAGElib_GlobalSram_eSageFrameworkHeader        0x38
#define BSAGElib_GlobalSram_eSageFrameworkHeaderSize    0x39


/* HSM STASH will be removed; do not use for now [0x50, 0x5F] */
#define BSAGElib_GlobalSram_eHsmReserved                0x50 /* uses 16 indexes */ /* TODO: check with HSM folks the right value */
        /* note: BSAGElib_GlobalSram_eHsmReserved consumes 16 indexes */

    /* new entries could be add here in the future
       in order to remain backwards compatible, all GlobalSram will be set to 0x0 and 0x0 is not a valid value for any field from here
       all the parts that will interpret new values from here needs to add logic in order to be backward compatible */

    /* Reserved entries to expand GlobalSram into GLR memory block if need to pass more parameters */
#define BSAGElib_GlobalSram_eExtensionOffset            0x68
#define BSAGElib_GlobalSram_eExtensionSize              0x69
/* Note: Nothing can be added > 0x69 */

#define BSAGElib_GlobalSram_Size                        0x70

/* On HOST reset (SAGE remain active), overload the following 2 fields
* for SAGE->HOST to store GLR offset. HOST will check this to make sure
* that basic HSI will function */
#define BSAGElib_GlobalSram_eLastRegionGlrOffset        BSAGElib_GlobalSram_eRegionMapOffset
#define BSAGElib_GlobalSram_eLastRegionGlrSize          BSAGElib_GlobalSram_eRegionMapSize


#define BSAGElib_GlobalSram_GetOffset(INDEX) ((INDEX)*4) /* convert index to offset */
#define BSAGElib_GlobalSram_GetRegister(INDEX) (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BSAGElib_GlobalSram_GetOffset(INDEX)) /* convert index to register */

#endif /* BSAGELIB_SHARED_GLOBALSRAM_H_ */
