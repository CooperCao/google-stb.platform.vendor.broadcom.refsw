/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_SPI_PRIVATE_H__
#define NEXUS_SPI_PRIVATE_H__

#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_Error NEXUS_Spi_Write_private(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=length;reserved=8} pointer to write memory location */
    size_t length                   /* size of data in pWriteData[] in bytes */
    );

NEXUS_Error NEXUS_Spi_Read_private(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=length;reserved=8} pointer to memory location where data is to sent  */
    uint8_t *pReadData,             /* [out] attr{nelem=length;reserved=8} pointer to memory location to store read data  */
    size_t length                   /* length is the size of pWriteData[] in bytes and is also the size of pReadData[] in bytes. They have the same size. */
    );

NEXUS_Error NEXUS_Spi_ReadAll_private(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=writeLength;reserved=8} pointer to memory location where data is to sent  */
    size_t writeLength,             /* Total number of bytes to write to slave from pWriteData, in bytes */
    uint8_t *pReadData,             /* [out] attr{nelem=readLength;nelem_out=pActualReadLength;reserved=8} pointer to memory location to store read data  */
    size_t readLength,              /* Total number of bytes to read from slave into pReadData, in bytes */
    size_t *pActualReadLength       /* [out] actual number. will be less than readLength on timeout. */
    );

unsigned NEXUS_Spi_GetContinousSSDelay_private(
    NEXUS_SpiHandle spiHandle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SPI_PRIVATE_H__ */
