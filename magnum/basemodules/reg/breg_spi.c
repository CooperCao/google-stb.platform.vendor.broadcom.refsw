/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "breg_spi.h"
#include "breg_spi_priv.h"

void BREG_SPI_SetContinueAfterCommand(BREG_SPI_Handle spiHandle, bool bEnable )
{
    spiHandle->BREG_SPI_Set_Continue_After_Command_Func( spiHandle->context, bEnable);
}

void BREG_SPI_GetBitsPerTransfer(BREG_SPI_Handle spiHandle, uint8_t *pBitsPerTransfer)
{
   spiHandle->BREG_SPI_Get_Bits_Per_Transfer_Func( spiHandle->context, pBitsPerTransfer);
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
