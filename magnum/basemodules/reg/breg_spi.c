/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
#include "bstd.h"
#include "breg_spi.h"
#include "breg_spi_priv.h"

void BREG_SPI_GetBitsPerTransfer(BREG_SPI_Handle spiHandle, uint8_t *pBitsPerTransfer)
{
   spiHandle->BREG_SPI_Get_Bits_Per_Transfer_Func( spiHandle->context, pBitsPerTransfer);
}

BERR_Code BREG_SPI_Multiple_Write(BREG_SPI_Handle spiHandle, const BREG_SPI_Data *pWriteData, size_t count)
{
	return spiHandle->BREG_SPI_Multiple_Write_Func( spiHandle->context, pWriteData, count);
}

BERR_Code BREG_SPI_WriteAll(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, size_t length)
{
   return spiHandle->BREG_SPI_Write_All_Func( spiHandle->context, pWriteData, length);
}

BERR_Code BREG_SPI_Write(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, size_t length)
{
    return spiHandle->BREG_SPI_Write_Func( spiHandle->context, pWriteData, length);
}

BERR_Code BREG_SPI_Read(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, uint8_t *pReadData, size_t length)
{
    return spiHandle->BREG_SPI_Read_Func( spiHandle->context, pWriteData, pReadData, length);
}
BERR_Code BREG_SPI_ReadAll(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength)
{
    return spiHandle->BREG_SPI_Read_All_Func( spiHandle->context, pWriteData, writeLength, pReadData, readLength);
}

BERR_Code BREG_SPI_Write_isr(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, size_t length)
{
    return spiHandle->BREG_SPI_Write_Func_isr( spiHandle->context, pWriteData, length);
}

BERR_Code BREG_SPI_Read_isr(BREG_SPI_Handle spiHandle, const uint8_t *pWriteData, uint8_t *pReadData, size_t length)
{
    return spiHandle->BREG_SPI_Read_Func_isr( spiHandle->context, pWriteData, pReadData, length);
}
 
/* End of File */
