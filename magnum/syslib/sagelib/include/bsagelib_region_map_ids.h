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

/* This file must be compatible with Assembly */


#ifndef BSAGELIB_REGION_MAP_IDS_H_
#define BSAGELIB_REGION_MAP_IDS_H_

/* Use the upper 16 bits as modifier flags to heap index's */
#define REGION_ID_FLAG_MASK 0xFF
#define REGION_ID_FLAG_SHIFT 24

#define REGION_ID_GET_ID(region) ((region) & ~(REGION_ID_FLAG_MASK << REGION_ID_FLAG_SHIFT))
#define REGION_ID_GET_FLAGS(region) ((region) >> REGION_ID_FLAG_SHIFT)
#define REGION_ID_CHECK_FLAG(region, flag) (REGION_ID_GET_FLAGS(region) & ((flag) & REGION_ID_FLAG_MASK))
#define REGION_ID_SET_FLAG(region, flag) ((region) | (((flag) & REGION_ID_FLAG_MASK) << REGION_ID_FLAG_SHIFT))
#define REGION_ID_CLEAR_FLAG(region, flag) ((region) & ~(((flag) & REGION_ID_FLAG_MASK) << REGION_ID_FLAG_SHIFT))

/* Flag and helper macro's for passing up 40bit offset instead of 32bit */
#define REGION_ID_FLAG_40BIT 0x80
#define REGION_ID_FLAG_40BIT_OFFSET_SET(offset) ((uint64_t)(offset) >> 8)
#define REGION_ID_FLAG_40BIT_OFFSET_GET(offset) ((uint64_t)(offset) << 8)
#define REGION_ID_FLAG_40BIT_OFFSET_GET_WCHECK(region) REGION_ID_CHECK_FLAG(region.id, REGION_ID_FLAG_40BIT) ? \
                                                REGION_ID_FLAG_40BIT_OFFSET_GET(region.offset) : \
                                                (uint64_t)(region.offset)

#define BSAGElib_RegionId_eInvalid 0x00
#define BSAGElib_RegionId_Glr      0x01
#define BSAGElib_RegionId_Glr2     0x02
#define BSAGElib_RegionId_Srr      0x10
#define BSAGElib_RegionId_Crr      0x11
#define BSAGElib_RegionId_Xrr      0x20
#define BSAGElib_RegionId_Urr0     0x30
#define BSAGElib_RegionId_Urr1     0x31
#define BSAGElib_RegionId_Urr2     0x32
#define BSAGElib_RegionId_SecGfx   0x33
#define BSAGElib_RegionId_Urrt0    0x34
#define BSAGElib_RegionId_Urrt1    0x35
#define BSAGElib_RegionId_Urrt2    0x36
#define BSAGElib_RegionId_Crrt     0x40
#define BSAGElib_RegionId_Fwrr     0x50


#endif /* BSAGELIB_REGION_MAP_H_ */
