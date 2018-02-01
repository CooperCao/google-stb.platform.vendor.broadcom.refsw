/******************************************************************************
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
 *****************************************************************************/
#ifndef PMC_CORE_H_
#define PMC_CORE_H_

#include "drm_types.h"

typedef enum pmc_type_e
{
    PMC_RESERVED_0  = 0,
    PMC_RESERVED_1  = 1,
    PMC_RESERVED_2  = 2,
    PMC_RESERVED_3  = 3,
    PMC_GENERIC     = 4,
    PMC_RESERVED_5  = 5,
    PMC_RESERVED_6  = 6,
    PMC_RESERVED_7  = 7,
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

/******************************************************************************
 ** FUNCTION
 **   DRM_Pmc_CoreInit
 **
 ** DESCRIPTION:
 **   Initializes the PMC core module with the pmc.bin file and if specified the
 **   DRM bin file as well.  If the 'pmc_bin_filepath' is NULL, the pmc_core module
 **   will look for /mnt/pmc.bin at runtime.
 **
 ** PARAMETERS:
 **   pmc_bin_filepath[in] - Pointer to the pmc.bin file path in the root filesystem
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Pmc_CoreInit(char * pmc_bin_file_path);

/******************************************************************************
 ** FUNCTION
 **   DRM_Pmc_CoreUninit
 **
 ** DESCRIPTION:
 **   Closes pmc_core module
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_Pmc_CoreUninit(void);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Pmc_CoreFetchData
 **
 ** DESCRIPTION:
 **   Extract data from pmc credential
 **
 **   NOTE: Be sure to free pData in calling function after done with the data
 **
 ** PARAMETERS:
 **   pmc_type[in] - Type of data to fetch
 **   pData[out]   - Pointer to destination buffer to copy to
 **   size[out]    - Pointer to uint32_t to return data size
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- an error code
 **
 ******************************************************************************/
DrmRC DRM_Pmc_CoreFetchData(  pmc_type_e pmc_type,
                              uint8_t **pData,
                              uint32_t *size);

#endif /*PMC_CORE_H_*/
