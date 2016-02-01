/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef BREG_I2C_PRIV_H
#define BREG_I2C_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BREG_I2C_Impl
{
    void * context;
    BERR_Code (*BREG_I2C_Write_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteSw_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteNoAck_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteA16_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteSwA16_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteA24_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteSwA24_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteNoAddr_Func)(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteSwNoAddr_Func)(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteNoAddrNoAck_Func)(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteNvram_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_Read_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadSw_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadNoAck_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadA16_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadSwA16_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadA24_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadSwA24_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadNoAddr_Func)(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadSwNoAddr_Func)(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadNoAddrNoAck_Func)(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadEDDC_Func)(void * context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_ReadSwEDDC_Func)(void * context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_WriteEDDC_Func)(void * context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*BREG_I2C_SetupHdmiHwAccess_Func)(void * context, uint32_t dataTransferFormat,uint32_t cnt1, uint32_t cnt2);
    BERR_Code (*BREG_I2C_WriteRead_Func)(void * context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength );
    BERR_Code (*BREG_I2C_WriteReadNoAddr_Func)(void * context, uint16_t chipAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength );

} BREG_I2C_Impl;

#ifdef __cplusplus
}
#endif

#endif
/* End of File */

