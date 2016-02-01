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
#ifndef BREG_SPI_PRIV_H
#define BREG_SPI_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BREG_SPI_Impl
{
    void * context;
    void (*BREG_SPI_Set_Continue_After_Command_Func)( void *context, bool bEnable);
    void (*BREG_SPI_Get_Bits_Per_Transfer_Func)(void * context, uint8_t *pBitsPerTransfer);
    BERR_Code (*BREG_SPI_Write_All_Func)(void * context, const uint8_t *pWriteData, size_t length);
    BERR_Code (*BREG_SPI_Write_Func)(void * context, const uint8_t *pWriteData, size_t length);
    BERR_Code (*BREG_SPI_Read_Func)(void * context, const uint8_t *pWriteData, uint8_t *pReadData, size_t length);
    BERR_Code (*BREG_SPI_Read_All_Func)(void * context, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength);
    BERR_Code (*BREG_SPI_Write_Func_isr)(void * context, const uint8_t *pWriteData, size_t length);
    BERR_Code (*BREG_SPI_Read_Func_isr)(void * context, const uint8_t *pWriteData, uint8_t *pReadData, size_t length);
} BREG_SPI_Impl;

#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */

