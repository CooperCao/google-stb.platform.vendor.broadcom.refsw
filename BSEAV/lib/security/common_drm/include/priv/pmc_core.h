/******************************************************************************
 *    (c)2010-2011 Broadcom Corporation
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
 * $brcm_Workfile: pmc_core.h $
 * $brcm_Revision: 2 $
 * $brcm_Date: 5/31/12 5:38p $
 *
 * Module Description:
 *
 * Revision History:
 *
 *****************************************************************************/
#ifndef PMC_CORE_H_
#define PMC_CORE_H_

#include "drm_types.h"
#include "drm_metadata.h"
#include "bdbg.h"


typedef enum pmc_type_e
{
    PMC_PANDORA     = 0,
    PMC_NAPSTER     = 1,
    PMC_NETFLIX     = 2,
    PMC_VUDU        = 3,
    PMC_GENERIC     = 4,
    PMC_BBC         = 5,
    PMC_RHAPSODY    = 6,
    PMC_AMAZON      = 7,
    PMC_ADOBE       = 8,
    PMC_INVALID     = 9
}pmc_type_e;

typedef struct pmc_entry_header_t
{
    uint8_t pmc_name[12];
    uint8_t pmc_entry_data_size[4];
    uint8_t encrypted_rpk[16];
    uint8_t serial_number[16];
    uint8_t *ptr_enc_data;
    uint8_t encrypted_hash[32];
}pmc_entry_header_t;




DrmRC DRM_Pmc_CoreInit(char * pmc_bin_file_path);

void DRM_Pmc_CoreUninit(void);

DrmRC DRM_Pmc_CoreFetchData(  pmc_type_e pmc_type,
                            uint8_t **pData,
                            uint32_t *size);




#endif /*PMC_CORE_H_*/
