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


#include "bstd.h"
#include "bsagelib_region_map_ids.h"


#ifndef BSAGELIB_REGION_MAP_H_
#define BSAGELIB_REGION_MAP_H_


/* RegionMap shall define the following:
   Host side shall allocate memory blocks in GLR to carry :

   BSAGElib_RegionInfo regions_map[] = {
   {BSAGElib_RegionId_Glr,  Offset, Size}, // Main Region accessible inside GLR
   {BSAGElib_RegionId_Glr2, Offset, Size}, // Secondary Region accessible inside GLR (used to be referred as 'client')
   {BSAGElib_RegionId_Srr,  Offset, Size}, // SAGE Restricted Region
   {BSAGElib_RegionId_Crr,  Offset, Size}, // Compressed Restricted Region
   {BSAGElib_RegionId_Xrr,  Offset, Size}, // eXport Restricted Region
   {BSAGElib_RegionId_Urr0, Offset, Size}, // Uncompressed Restricted Region on MEMC0
   {BSAGElib_RegionId_Urr1, Offset, Size}, // Uncompressed Restricted Region on MEMC1
   {BSAGElib_RegionId_Urr2, Offset, Size}, // Uncompressed Restricted Region on MEMC2
   {BSAGElib_RegionId_... , Offset, Size}, // Any futur region
   {BSAGElib_RegionId_... , Offset, Size}, // Any futur region
   {BSAGElib_RegionId_eInvalid, 0x0, 0x0}, // Mark the end of the array
   };

   Each of SAGE binary using those values needs to cache them inside
   a private/secure memory before validating/using them */


typedef struct {
    uint32_t id;     /* BSAGElib_RegionId_* see bsagelib_shared_globalsram.h */
    uint32_t offset; /* Physical Address of the region, must feat in 32 bits */
    uint32_t size;   /* size of the region in bytes */
} BSAGElib_RegionInfo;


#endif /* BSAGELIB_REGION_MAP_H_ */
