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
 *
 ******************************************************************************/


/* This file must be compatible with Assembly */


#ifndef BSAGELIB_SHARED_GLOBALSRAM_H_
#define BSAGELIB_SHARED_GLOBALSRAM_H_

#include "bchp_scpu_globalram.h"

/*
  GlobalRAM is partitioned by BFW.
  Partition boundaries are indicated in SUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR register
  and in GlobalSRAM[0,1].

  Order and size of partitions chnages between Zeus 4.x and later versions
  For expedience, these are hard-coded here.

  Major partitions include:
  Global SRAM pointers    Two words describing index offset of other regions,
                          SAGE Scratch | BSP Data| HS Shared | General, r|r|r| Misc Global

  Fixed Data -            Mostly RO to all, select words writeable by SAGE. Incldues following tables
    R-Region Info         Start/(inclusinve) end offsets of CRR, SRR, etc. [4K page indicies in Z5]
    Max Res Video Info    2 words managed by SAGE
    BSP IP Lic. bitmap
    Secure RAAGA Info

  Host/Sage DRAM        Host/SAGE shared data, including SAGE boot parameters, etc
  Misc Global Comm      R/W all cores
  SAGE Scratchpad       R/W SAGE BSP only
  BSP Async Data        R/W SAGE BSP only
  R-Region Extension    Extended Restricted Region Info tables [Zeus<5; for new Zeus, this is part of R-Region Info]

*/

/* Size: first Out of bound index */
#define SAGE_GlobalSram_TotalSize             0x100

#define GlobalSram_Pointers                   0x00    /*  */
#define GlobalSram_FixedDataHeader            0x02    /* version |r|r| #words in Licensing / General fixed area */
#define GlobalSram_RRegion_Info               0x03    /* Feature table entry 2 */

#if BHSM_ZEUS_VER_MAJOR>=5
/* 7278A0 GlobalSRAM[0] = 0x70E03402 (expected 8BFA3C02 per document THIS MAY NOT BE CORRECT) */
#define GlobalSram_FixedData_size             0x32    /* #words in fixed area, RO to all but BSP, SAGE */
#define GlobalSram_MaxVideoRes_Info           0x20    /* Feature table entry 6 */
#define GlobalSram_IPLicensing_Info           0x22    /* Feature table entry 1 */
#define GlobalSram_IPLicensing_Info_size      0x08    /* GlobalSram_IPLicensing_Info[7:0]/32 used, but more reserved*/
#define GlobalSram_HostSage_Scratchpad        0x34    /* GlobalSram_Pointers [15:8]  THIS MAY NOT BE CORRECT */
#define GlobalSram_HostSage_Scratchpad_size   0x30
#define GlobalSram_Misc_Comm                  0x50    /* GlobalSram_Pointers [31:24] THIS MAY NOT BE CORRECT */
#define GlobalSram_Misc_Comm_size             0x1D
#define GlobalSram_SAGE_Scratchpad            0x70    /* GlobalSram_Pointers [31:24] THIS MAY NOT BE CORRECT */
#define GlobalSram_SAGE_Scratchpad_size       0x70    /* Expect 6F THIS MAY NOT BE CORRECT */
#define GlobalSram_BSP_AsyncData              0xFA    /* GlobalSram_Pointers [31:24] THIS MAY NOT BE CORRECT */
#define GlobalSram_BSP_AsyncData_size         0x06
#define GlobalSram_RRegion_Extension          (GlobalSram_RRegion_Info+7*2) /* continuation of RRegion struct */
#define GlobalSram_RRegion_Extension_size     (GlobalSram_RRegion_Info+7*2) /* continuation of RRegion struct */

#define RESTRICTED_REGION_OFFSET(RR_Start) ((RR_Start)<<12)
#define RESTRICTED_REGION_SIZE(RR_Start,RR_End) ((RR_End)<<12)
#else
/* GlobalSRAM[0] = 0x70E02002 */
#define GlobalSram_FixedData_size             0x1E
#define GlobalSram_MaxVideoRes_Info           0x12    /* Feature table entry 6 */
#define GlobalSram_IPLicensing_Info           0x14    /* Feature table entry 1 */
#define GlobalSram_IPLicensing_Info_size      0x08    /* GlobalSram_IPLicensing_Info[7:0]/32 */
#define GlobalSram_HostSage_Scratchpad        0x20    /* GlobalSram_Pointers [15:8] */
#define GlobalSram_HostSage_Scratchpad_size   0x30
#define GlobalSram_SAGE_Scratchpad            0x70    /* GlobalSram_Pointers [31:24] */
#define GlobalSram_SAGE_Scratchpad_size       0x70
#define GlobalSram_Misc_Comm                  0x50
#define GlobalSram_Misc_Comm_size             0x20
#define GlobalSram_BSP_AsyncData              0xE0
#define GlobalSram_BSP_AsyncData_size         0x02
#define GlobalSram_RRegion_Extension          (0xE2+0x01) /* continuation of RRegion struct, skip header*/
#define GlobalSram_RRegion_Extension_size     (0x1D)      /* Allocated Size */

#define RESTRICTED_REGION_OFFSET(RR_Start) (RR_Start)
#define RESTRICTED_REGION_SIZE(RR_Start,RR_End) (((RR_End)|0x07)+1-(RR_Start))
#endif

/*
 * SAGE global SRAM register indexes
 */

/************************************************************/
/* From 0x00 to 0x1F : General purpose
 */
/* FIRST of general purpose*/
#define BSAGElib_GlobalSram_eGeneralPurposeFirst        (GlobalSram_Pointers)

#define BSAGElib_GlobalSram_eDataHeader                 (GlobalSram_FixedDataHeader)  /* fixed data header */
#define BSAGElib_GlobalSram_eRRegion                    (GlobalSram_RRegion_Info)

#define BSAGElib_GlobalSram_eCRRStartOffset             (GlobalSram_RRegion_Info+0x01) /* Set by BSP: physical address of the first byte of CRR */
#define BSAGElib_GlobalSram_eCRREndOffset               (GlobalSram_RRegion_Info+0x02) /* Set by BSP: physical address of the last byte of CRR */
#define BSAGElib_GlobalSram_eSRRStartOffset             (GlobalSram_RRegion_Info+0x03) /* Set by BSP: physical address of the first byte of SRR */
#define BSAGElib_GlobalSram_eSRREndOffset               (GlobalSram_RRegion_Info+0x04) /* Set by BSP: physical address of the last byte of SRR */

#define BSAGElib_GlobalSram_eMaxResGlr                  (GlobalSram_MaxVideoRes_Info)     /* Set by SAGE: Maximum resolution */
#define BSAGElib_GlobalSram_eMaxRes                     (GlobalSram_MaxVideoRes_Info+0x01) /* Set by SAGE: Maximum resolution */

#define BSAGElib_GlobalSram_eIPLicensing                (GlobalSram_IPLicensing_Info)
#define BSAGElib_GlobalSram_eBP3VideoFeatureList0       (GlobalSram_IPLicensing_Info+0x01) /* Set by SAGE: BP3 */
#define BSAGElib_GlobalSram_eBP3AudioFeatureList0       (GlobalSram_IPLicensing_Info+0x02) /* Set by SAGE: BP3 */
#define BSAGElib_GlobalSram_eBP3HostFeatureList         (GlobalSram_IPLicensing_Info+0x03) /* Set by SAGE: BP3 */
#define BSAGElib_GlobalSram_eBP3SAGEFeatureList         (GlobalSram_IPLicensing_Info+0x04) /* Set by SAGE: BP3 */
/* [0x19 reserved for future BP3 usage */
#define BSAGElib_GlobalSram_eBP3VideoFeatureList1       (GlobalSram_IPLicensing_Info+0x06) /* Set by SAGE: BP3 */
#define BSAGElib_GlobalSram_eBP3ReservedLast            (GlobalSram_IPLicensing_Info+GlobalSram_IPLicensing_Info_size-1)
/* [0x1C reserved */
#define BSAGElib_GlobalSram_eSAGEBootFlag               (GlobalSram_IPLicensing_Info+0x09)

/* [0x1E -- > 0x1F] reserved */

#define BSAGElib_GlobalSram_eGeneralPurposeLast         (GlobalSram_FixedDataHeader+GlobalSram_FixedData_size-1)
/************************************************************/


/************************************************************/
/* From 0x20 to 0x4F: Host < -- > SAGE scratchdpad
 */
#define BSAGElib_GlobalSram_eHostSageScratchpadFirst    (GlobalSram_HostSage_Scratchpad)

/* scheme versioning defines how the GlobalSram is used and what to expect at which index */
#define BSAGElib_GlobalSram_eSchemeVersion              (GlobalSram_HostSage_Scratchpad+0x00)

    /* SAGE < -- > Host communication buffers */
#define BSAGElib_GlobalSram_eHostSageBuffers            (GlobalSram_HostSage_Scratchpad+0x01)
#define BSAGElib_GlobalSram_eHostSageBuffersSize        (GlobalSram_HostSage_Scratchpad+0x02)

    /* Regions configuration */
#define BSAGElib_GlobalSram_eRegionMapOffset            (GlobalSram_HostSage_Scratchpad+0x03)
#define BSAGElib_GlobalSram_eRegionMapSize              (GlobalSram_HostSage_Scratchpad+0x04)

    /* SAGE Secure Logging */
#define BSAGElib_GlobalSram_eSageLogBufferOffset        (GlobalSram_HostSage_Scratchpad+0x05)
#define BSAGElib_GlobalSram_eSageLogBufferSize          (GlobalSram_HostSage_Scratchpad+0x06)
#define BSAGElib_GlobalSram_eSageLogWriteCountLSB       (GlobalSram_HostSage_Scratchpad+0x07)
#define BSAGElib_GlobalSram_eSageLogWriteCountMSB       (GlobalSram_HostSage_Scratchpad+0x08)

    /* SAGE Anti Rollback */
#define BSAGElib_GlobalSram_eSageBootloaderEpochVersion (GlobalSram_HostSage_Scratchpad+0x09)
#define BSAGElib_GlobalSram_eSageFrameworkEpochVersion  (GlobalSram_HostSage_Scratchpad+0x0A)

    /* Control SAGE life cycle */
#define BSAGElib_GlobalSram_eReset                      (GlobalSram_HostSage_Scratchpad+0x0B) /* DO NOT MOVE : this is used by BCHP and SAGE for restart/reset feature */
#define BSAGElib_GlobalSram_eSuspend                    (GlobalSram_HostSage_Scratchpad+0x0C)

    /* Sage Status and versioning */
#define BSAGElib_GlobalSram_eLastError                  (GlobalSram_HostSage_Scratchpad+0x0D)
#define BSAGElib_GlobalSram_eBootStatus                 (GlobalSram_HostSage_Scratchpad+0x0E)
#define BSAGElib_GlobalSram_eSageBootloaderVersion      (GlobalSram_HostSage_Scratchpad+0x0F)
#define BSAGElib_GlobalSram_eSageFrameworkVersion       (GlobalSram_HostSage_Scratchpad+0x10)

    /* Misc */
#define BSAGElib_GlobalSram_eSageStatusFlags            (GlobalSram_HostSage_Scratchpad+0x11)

    /* SAGE Services parameters - resources */
#define BSAGElib_GlobalSram_eSageVklMask                (GlobalSram_HostSage_Scratchpad+0x12)
#define BSAGElib_GlobalSram_eSageDmaChannel             (GlobalSram_HostSage_Scratchpad+0x13)

    /* SAGE Secure Boot */
    /* TODO TBD: Total: 28 (0x1e indexes
       step1: replace those entries by a structure pointed to GLR
       step2: replace all of this with a pointer to the header
              (BL with authenticate header etc */
#define BSAGElib_GlobalSram_eSageFrameworkBin           (GlobalSram_HostSage_Scratchpad+0x14)
#define BSAGElib_GlobalSram_eSageFrameworkBinSize       (GlobalSram_HostSage_Scratchpad+0x15)
#define BSAGElib_GlobalSram_eSageFrameworkBinSignature  (GlobalSram_HostSage_Scratchpad+0x16)
#define BSAGElib_GlobalSram_eTextSectionSize            (GlobalSram_HostSage_Scratchpad+0x17)
#define BSAGElib_GlobalSram_eSageFrameworkHeader        (GlobalSram_HostSage_Scratchpad+0x18)
#define BSAGElib_GlobalSram_eSageFrameworkHeaderSize    (GlobalSram_HostSage_Scratchpad+0x19)

/* [0x40 -- > 0x4F] unused */

/* On HOST reset (SAGE remain active), overload the following 2 fields
* for SAGE->HOST to store GLR offset. HOST will check this to make sure
* that basic HSI will function */
#define BSAGElib_GlobalSram_eLastRegionGlrOffset        BSAGElib_GlobalSram_eRegionMapOffset
#define BSAGElib_GlobalSram_eLastRegionGlrSize          BSAGElib_GlobalSram_eRegionMapSize

/* LAST of general purpose */
#define BSAGElib_GlobalSram_eHostSageScratchpadLast     (GlobalSram_HostSage_Scratchpad+GlobalSram_HostSage_Scratchpad_size-1)
/************************************************************/


/* FIRST of Misc global comm */
#define BSAGElib_GlobalSram_eMiscCommFirst              (GlobalSram_Misc_Comm)

#if 0
/* HSM STASH will be removed; do not use for now [0x50, 0x5F] */
#define BSAGElib_GlobalSram_eHsmReserved                0x50 /* uses 16 indexes */ /* TODO: check with HSM folks the right value */
        /* note: BSAGElib_GlobalSram_eHsmReserved consumes 16 indexes */

    /* new entries could be add here in the future
       in order to remain backwards compatible, all GlobalSram will be set to 0x0 and 0x0 is not a valid value for any field from here
       all the parts that will interpret new values from here needs to add logic in order to be backward compatible */

    /* Reserved entries to expand GlobalSram into GLR memory block if need to pass more parameters */
#define BSAGElib_GlobalSram_eExtensionOffset            0x68
#define BSAGElib_GlobalSram_eExtensionSize              0x69
#endif

/* LAST of Misc global comm */
#define BSAGElib_GlobalSram_eMiscCommLast               (GlobalSram_Misc_Comm+GlobalSram_Misc_Comm_size-1)
/************************************************************/



#define BSAGElib_GlobalSram_GetOffset(INDEX) ((INDEX)*4) /* convert index to offset */
#define BSAGElib_GlobalSram_GetRegister(INDEX) (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BSAGElib_GlobalSram_GetOffset(INDEX)) /* convert index to register */

#endif /* BSAGELIB_SHARED_GLOBALSRAM_H_ */
