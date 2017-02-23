/****************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#ifndef _DSP_RAAGA_OCTAVE_ATU_H_
#define _DSP_RAAGA_OCTAVE_ATU_H_

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"



/**
 * Read an ATU entry from the HW and store it locally.
 *
 * @param dsp[in]    the DSP instance
 * @param num[in]    which entry in the table to read
 * @param entry[out] the DSP_RAAGA_ATU_ENTRY to fill with data read from the HW
 */
void DSP_readAtuEntry(DSP *dsp, unsigned num, DSP_RAAGA_ATU_ENTRY *entry);


/**
 * Write a locally stored ATU entry to the HW.
 *
 * @param dsp[in]   the DSP instance
 * @param num[in]   which entry in the table to write
 * @param entry[in] the DSP_RAAGA_ATU_ENTRY data to write into the the HW
 */
void DSP_writeAtuEntry(DSP *dsp, unsigned num, const DSP_RAAGA_ATU_ENTRY *entry);


/**
 * Read the ATU status from the HW and store it locally.
 *
 * @param dsp[in]  the DSP instance
 * @param atu[out] the DSP_RAAGA_ATU_ENTRY to fill with data read from the HW
 */
void DSP_readAtu(DSP *dsp, DSP_RAAGA_ATU *atu);


/**
 * Write a locally stored ATU status to the HW.
 *
 * @param dsp[in] the DSP instance
 * @param atu[in] the DSP_RAAGA_ATU_ENTRY data to write into the the HW
 */
void DSP_writeAtu(DSP *dsp, const DSP_RAAGA_ATU *atu);


/**
 * Fill in an ATU index relative to a DSP_RAAGA_ATU structure.
 *
 * @param atu[in]    the local copy of the ATU content
 * @param index[out] the index structure to fill in
 */
void DSP_createAtuIndex(const DSP_RAAGA_ATU *atu, DSP_RAAGA_ATU_INDEX *index);


/**
 * Look in the ATU index and translate the provided virtual address into its
 * physical equivalent. If a length is provided, it's modified in place to
 * indicate how much of the provided [virtual_addr, virtual_addr+length[ range
 * fits in a single ATU entry.
 *
 * @param index[in]          ATU entries index
 * @param virtual_addr[in]   start of the virtual range to be translated
 * @param physical_addr[out] physical address corresponding to virtual_addr
 * @param length[in,out]     if not NULL, length of the virtual range to be
 *                           translated, available physical range length returned
 * @return DSP_FAILURE is the address couldn't be translated, DSP_SUCCESS otherwise
 */
DSP_RET DSP_atuVirtualToPhysical(const DSP_RAAGA_ATU_INDEX *index,
                                 DSP_ADDR virtual_addr, uint64_t *physical_addr,
                                 uint32_t *length);


/**
 * Look in the ATU index and translate the provided virtual address into its
 * physical equivalent. If a length is provided, it's modified in place to
 * indicate how much of the provided [virtual_addr, virtual_addr+length[ range
 * fits in a single ATU entry.
 *
 * @param index[in]          ATU entries index
 * @param physical_addr[in]  start of the physical range to be translated
 * @param virtual_addr[out]  virtual address corresponding to physical_addr
 * @param length[in,out]     if not NULL, length of the physical range to be
 *                           translated, available virtual range length returned
 * @return DSP_FAILURE is the address couldn't be translated, DSP_SUCCESS otherwise
 */
DSP_RET DSP_atuPhysicalToVirtual(const DSP_RAAGA_ATU_INDEX *index,
                                 uint64_t physical_addr, DSP_ADDR *virtual_addr,
                                 uint32_t *length);


#endif  /* _DSP_RAAGA_OCTAVE_ATU_H_ */
