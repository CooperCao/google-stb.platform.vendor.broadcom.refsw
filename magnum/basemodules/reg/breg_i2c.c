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
#include "breg_i2c.h"
#include "breg_i2c_priv.h"

BDBG_MODULE(breg_i2c);

BERR_Code BREG_I2C_Write(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_Write_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_Write_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteSw(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteSw_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_WriteSw() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_Write() instead."));
    return (i2cHandle->BREG_I2C_WriteSw_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteNoAck(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteNoAck_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteNoAck_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteA16(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint16_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteA16_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteA16_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteSwA16(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint16_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteSwA16_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_WriteSwA16() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_WriteA16() instead."));
    return (i2cHandle->BREG_I2C_WriteSwA16_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteA24(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteA24_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteA24_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteNoAddr(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteNoAddr_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteNoAddr_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_WriteSwNoAddr(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteSwNoAddr_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_WriteSwNoAddr() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_WriteNoAddr() instead."));
    return (i2cHandle->BREG_I2C_WriteSwNoAddr_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_WriteNoAddrNoAck(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteNoAddrNoAck_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteNoAddrNoAck_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_WriteNvram(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteNvram_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteNvram_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_Read(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_Read_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_Read_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadSw(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadSw_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_ReadSw() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_Read() instead."));
    return (i2cHandle->BREG_I2C_ReadSw_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadNoAck(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadNoAck_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadNoAck_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadA16(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint16_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadA16_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadA16_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadSwA16(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint16_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadSwA16_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_ReadSwA16() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_ReadA16() instead."));
    return (i2cHandle->BREG_I2C_ReadSwA16_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadA24(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadA24_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadA24_Func)( i2cHandle->context, chipAddr, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadNoAddr(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadNoAddr_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadNoAddr_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_ReadSwNoAddr(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadSwNoAddr_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_ReadSwNoAddr() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_ReadNoAddr() instead."));
    return (i2cHandle->BREG_I2C_ReadSwNoAddr_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_ReadNoAddrNoAck(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadNoAddrNoAck_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadNoAddrNoAck_Func)( i2cHandle->context, chipAddr, pData, length );
}

BERR_Code BREG_I2C_ReadEDDC(BREG_I2C_Handle i2cHandle, uint8_t chipAddr, uint8_t segment, uint8_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadEDDC_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_ReadEDDC_Func)( i2cHandle->context, chipAddr, segment, subAddr, pData, length );
}

BERR_Code BREG_I2C_ReadSwEDDC(BREG_I2C_Handle i2cHandle, uint8_t chipAddr, uint8_t segment, uint8_t subAddr, uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_ReadSwEDDC_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    BDBG_WRN(("BREG_I2C_ReadSwEDDC() will be removed in the future.  To prepare for this, call BI2C_OpenChannel() with the softI2c field set to true.  Then call BREG_I2C_ReadEDDC() instead."));
    return (i2cHandle->BREG_I2C_ReadSwEDDC_Func)( i2cHandle->context, chipAddr, segment, subAddr, pData, length );
}

BERR_Code BREG_I2C_WriteEDDC(BREG_I2C_Handle i2cHandle, uint8_t chipAddr, uint8_t segment, uint8_t subAddr, const uint8_t *pData, size_t length)
{
    if (!i2cHandle->BREG_I2C_WriteEDDC_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteEDDC_Func)( i2cHandle->context, chipAddr, segment, subAddr, pData, length );
}

BERR_Code BREG_I2C_SetupHdmiHwAccess(BREG_I2C_Handle i2cHandle, uint32_t dataTransferFormat,uint32_t cnt1, uint32_t cnt2)
{
    if (!i2cHandle->BREG_I2C_SetupHdmiHwAccess_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_SetupHdmiHwAccess_Func)( i2cHandle->context, dataTransferFormat, cnt1, cnt2);
}

BERR_Code BREG_I2C_WriteRead(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength)
{
    if (!i2cHandle->BREG_I2C_WriteRead_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteRead_Func)( i2cHandle->context, chipAddr, subAddr, pWriteData, writeLength, pReadData, readLength );
}

BERR_Code BREG_I2C_WriteReadNoAddr(BREG_I2C_Handle i2cHandle, uint16_t chipAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength)
{
    if (!i2cHandle->BREG_I2C_WriteReadNoAddr_Func) return BERR_TRACE(BERR_NOT_SUPPORTED);
    return (i2cHandle->BREG_I2C_WriteReadNoAddr_Func)( i2cHandle->context, chipAddr, pWriteData, writeLength, pReadData, readLength );
}

/* End of File */
