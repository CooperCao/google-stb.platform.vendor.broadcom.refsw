/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdtu.h"

void BDTU_GetDefaultCreateSettings( BDTU_CreateSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}
BERR_Code BDTU_Create( BDTU_Handle *pHandle, const BDTU_CreateSettings *pSettings )
{
    BSTD_UNUSED(pHandle);
    BSTD_UNUSED(pSettings);
    return BERR_NOT_SUPPORTED;
}
void BDTU_Destroy( BDTU_Handle handle )
{
    BSTD_UNUSED(handle);
}
void BDTU_GetDefaultRemapSettings( BDTU_RemapSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}
BERR_Code BDTU_Remap( BDTU_Handle handle, const BDTU_RemapSettings *pSettings )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_NOT_SUPPORTED;
}
BERR_Code BDTU_ReadOriginalAddress( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BSTD_DeviceOffset *orgPhysAddr )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(physAddr);
    BSTD_UNUSED(orgPhysAddr);
    return BERR_NOT_SUPPORTED;
}
void BDTU_PrintMap( BDTU_Handle handle, BSTD_DeviceOffset addr, uint64_t size )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
}
BERR_Code BDTU_Own( BDTU_Handle handle, BSTD_DeviceOffset addr, bool own)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(own);
    return BERR_NOT_SUPPORTED;
}
BERR_Code BDTU_Scrub( BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned size )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
    return BERR_NOT_SUPPORTED;
}
BERR_Code BDTU_ReadInfo( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BDTU_PageInfo *info)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(physAddr);
    BSTD_UNUSED(info);
    return BERR_NOT_SUPPORTED;
}
BERR_Code BDTU_GetStatus( BDTU_Handle handle, BDTU_Status *pStatus )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_NOT_SUPPORTED;
}

BERR_Code BDTU_GetState( BREG_Handle reg, unsigned memcIndex, BDTU_State *pState )
{
    BSTD_UNUSED(reg);
    BSTD_UNUSED(memcIndex);
    *pState = BDTU_State_eUnset;
    return BERR_SUCCESS;
}
