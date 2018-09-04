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
#include "bi2c_priv.h"

BDBG_MODULE(bi2c_auto);

#if AUTO_I2C_ENABLED
void BAUTO_I2C_P_HandleInterrupt_Isr(
    void *pParam1,                      /* [in] Device handle */
    int parm2                           /* [in] not used */
)
{
    BI2C_ChannelHandle  hChn;

    hChn = (BI2C_ChannelHandle) pParam1;
    BDBG_ASSERT( hChn );
    BSTD_UNUSED(parm2);

    BDBG_ENTER(BAUTO_I2C_P_HandleInterrupt_Isr) ;


    switch (hChn->autoI2c.channelNumber)
    {
    case 2 :
            BKNI_SetEvent_isr(hChn->hChnEvent);
        break ;

    default :
        BDBG_ERR(("Unknown Auto I2c channel number: %d", hChn->autoI2c.channelNumber)) ;
    }

    BDBG_LEAVE(BAUTO_I2C_P_HandleInterrupt_Isr) ;
    return ;
}

BERR_Code BI2C_Auto_P_Write
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{

    BERR_Code          retCode = BERR_SUCCESS;
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true){
        retCode = BI2C_AUTO_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
        return retCode;
    }
    else
#else
        BDBG_ERR(("Auto I2c Support only 4 byte transfer mode."));
#endif
    return BERR_INVALID_PARAMETER;
}

BERR_Code BI2C_Auto_P_ReadNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{

    BERR_Code          retCode = BERR_SUCCESS;
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true){
        retCode = BI2C_AUTO_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
        return retCode;
    }
    else
#else
        BDBG_ERR(("Auto I2c Support only 4 byte transfer mode."));
#endif

    return BERR_INVALID_PARAMETER;
}

BERR_Code BI2C_Auto_P_Read
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{

    BERR_Code          retCode = BERR_SUCCESS;
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true){
        retCode = BI2C_AUTO_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
        return retCode;
    }
    else
#else
        BDBG_ERR(("Auto I2c Support only 4 byte transfer mode."));
#endif

    return BERR_INVALID_PARAMETER;
}

static BERR_Code BAUTO_I2C_P_WaitForCompletion
(
    BI2C_ChannelHandle  hChn,                           /* Device channel handle */
    uint32_t            numBytes                        /* number of bytes to transfer */
)
{
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval=0, loopCnt=0;

    BSTD_UNUSED(numBytes);

    hDev = hChn->hI2c;

    if (hChn->chnSettings.intMode)
    {
        /* Wait for event, set by ISR */
        retCode = BKNI_WaitForEvent(hChn->hChnEvent, hChn->timeoutMs);
        if ( retCode != BERR_SUCCESS)
        {
            (void)BERR_TRACE(retCode);
            BKNI_ResetEvent(hChn->hChnEvent);
            goto done;
        }
    }
    else {
        while (loopCnt++ <= 5){
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_HDMI_TX_AUTO_I2C_TRANSACTION_DONE_STAT));
            if(lval >> hChn->autoI2c.channelNumber){
                BDBG_MSG(("BAUTO_I2C transaction complete."));
                goto done;
            }
            BKNI_Sleep(1);
        }
        BDBG_ERR(("BAUTO_I2C transaction incomplete."));
        retCode = BERR_TIMEOUT;
    }
done:
    return retCode;
}

void BAUTO_I2C_P_GetDefaultTriggerConfig(
    BAUTO_I2C_P_TriggerConfiguration *triggerConfig        /* [output] Returns default setting */
    )
{
    triggerConfig->enable = true;
    triggerConfig->triggerSource = 0; /* rdb write*/
    triggerConfig->eMode = BI2C_P_MODE_eReadOnly;
    triggerConfig->readAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT0;
    triggerConfig->timerMs = 0; /* 0 milli seconds*/
    return;
}

static void BAUTO_I2C_P_GetDefaultRegXSettings(
     BAUTO_I2C_P_ChxRegXSettings *pRegXSettings           /* [output] Returns default setting */
    )
{
    pRegXSettings->enable = true;
    pRegXSettings->data   = 0;
    pRegXSettings->writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCHIP_ADDR;
    return;
}

static BERR_Code BAUTO_I2C_P_ReadRegister(
    BI2C_ChannelHandle  hChn,       /* I2C channel handle. */
    BAUTO_I2C_P_CHX     eRegOffset, /*  HDMI_TX_AUTO_I2C_* register offset. */
    uint32_t            *data       /* data to be written to the  HDMI_TX_AUTO_I2C_* register */
    )
{
    BERR_Code    retCode = BERR_SUCCESS ;
    BI2C_Handle  hDev = hChn->hI2c;

    if (eRegOffset >= BAUTO_I2C_P_CHX_eMax)
    {
       BDBG_ERR(("Invalid register offset %d to read from.", eRegOffset));
       retCode = BERR_INVALID_PARAMETER; goto done;
    }

    *data = BREG_Read32(hDev->hRegister, hChn->autoI2c.baseAddress + (eRegOffset*4));
done:
    return retCode;

}

static BERR_Code BAUTO_I2C_P_WriteRegister(
    BI2C_ChannelHandle hChn,
    BAUTO_I2C_P_CHX    eRegOffset,
    uint32_t           data
    )
{
    BERR_Code   retCode = BERR_SUCCESS;
    BI2C_Handle hDev = hChn->hI2c;

    if (eRegOffset >= BAUTO_I2C_P_CHX_eMax)
    {
       BDBG_ERR(("Invalid register offset %d to read from.", eRegOffset));
       retCode = BERR_INVALID_PARAMETER; goto done;
    }

    BREG_Write32(hDev->hRegister, hChn->autoI2c.baseAddress+(eRegOffset*4), data) ;
done:
    return retCode;
}

static BERR_Code BAUTO_I2C_P_WriteChxRegX(
    BI2C_ChannelHandle              hChn,
    BAUTO_I2C_P_CHX_REG_OFFSET  eRegXOffset,
    BAUTO_I2C_P_ChxRegXSettings *pRegXSettings
    )
{
    BERR_Code   retCode = BERR_SUCCESS;
    BI2C_Handle hDev = hChn->hI2c;
    uint32_t    cfg=0, regXAddress = hChn->autoI2c.reg0BaseAddress+(eRegXOffset*8);

    if (eRegXOffset >= BAUTO_I2C_P_CHX_REG_OFFSET_eMax)
    {
       BDBG_ERR(("Invalid register offset %d to read from.", eRegXOffset));
       retCode =  BERR_INVALID_PARAMETER; goto done;
    }
    if (((pRegXSettings->writeAddressOffset >= BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT0) &&
        (pRegXSettings->writeAddressOffset <= BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT7)) ||
        (pRegXSettings->writeAddressOffset >= BAUTO_I2C_BSCX_P_OFFSET_eMax))
    {
       BDBG_ERR(("Invalid register offset %d to read from.", eRegXOffset));
       retCode =  BERR_INVALID_PARAMETER; goto done;
    }

    if(pRegXSettings->enable )
        cfg = 0x100;

    cfg |= (((uint8_t)pRegXSettings->writeAddressOffset) & 0x1f);
    BREG_Write32(hDev->hRegister, regXAddress, cfg) ;
    BREG_Write32(hDev->hRegister, regXAddress + 4, pRegXSettings->data) ;

done:
    return retCode;
}

static BERR_Code  BAUTO_I2C_P_SetTriggerConfiguration(
    BI2C_ChannelHandle  hChn,
    const BAUTO_I2C_P_TriggerConfiguration *triggerConfig
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval=0;

    lval = BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, ENABLE,         triggerConfig->enable)
         | BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, TRIGGER_SRC,    triggerConfig->triggerSource)
         | BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, MODE,           triggerConfig->eMode)
         | BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, RD_ADDR_OFFSET, triggerConfig->readAddressOffset)
         | BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, TIMER,          triggerConfig->timerMs);

    retCode = BAUTO_I2C_P_WriteRegister(hChn, BAUTO_I2C_P_CHX_CFG, lval);
    hChn->autoI2c.trigConfig = *triggerConfig ;

    return retCode;
}

static BERR_Code  BAUTO_I2C_P_SetTerminateChannel(
    BI2C_ChannelHandle  hChn,
    BAUTO_I2C_P_CHX_REG_OFFSET  eRegXOffset
)
{
    BAUTO_I2C_P_ChxRegXSettings regXSettings;

    /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
    regXSettings.data = 0;
    regXSettings.enable = false;
    regXSettings.writeAddressOffset = 0;
    return BAUTO_I2C_P_WriteChxRegX(hChn, eRegXOffset, &regXSettings);
}

static BERR_Code  BAUTO_I2C_P_SetCTLHIChannel(
    BI2C_ChannelHandle  hChn,
    BAUTO_I2C_P_CHX_REG_OFFSET  eRegXOffset,
    const BAUTO_I2C_P_CtlhiSettings *ctlhiSettings
)
{
    BAUTO_I2C_P_ChxRegXSettings regXSettings;

    regXSettings.enable = true;
    regXSettings.data = BCHP_FIELD_DATA(BSCA_CTLHI_REG, INPUT_SWITCHING_LEVEL, hChn->chnSettings.inputSwitching5V)
                      | BCHP_FIELD_DATA(BSCA_CTLHI_REG, DATA_REG_SIZE, hChn->chnSettings.fourByteXferMode)
                      | BCHP_FIELD_DATA(BSCA_CTLHI_REG, IGNORE_ACK, ctlhiSettings->ignoreAck);

    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCTLHI_REG;
    return BAUTO_I2C_P_WriteChxRegX(hChn, eRegXOffset, &regXSettings);
}

static BERR_Code BAUTO_I2C_P_SetCTLChannel(
    BI2C_ChannelHandle  hChn,
    BAUTO_I2C_P_CHX_REG_OFFSET  eRegXOffset,
    const BAUTO_I2C_P_CtlSettings  *ctlSettings
)
{
    BAUTO_I2C_P_ChxRegXSettings regXSettings;
    uint32_t ctlReg=0;
    uint8_t  divClk=0;

    switch (hChn->chnSettings.clkRate)
    {
        case BI2C_Clk_eClk100Khz:
        case BI2C_Clk_eClk400Khz:
            ctlReg = 0x01;
            break;

        case BI2C_Clk_eClk47Khz:
        case BI2C_Clk_eClk187Khz:
            ctlReg = 0x02;
            break;

        case BI2C_Clk_eClk50Khz:
        case BI2C_Clk_eClk200Khz:
            ctlReg = 0x03;
            break;

        case BI2C_Clk_eClk93Khz:
        case BI2C_Clk_eClk375Khz:
            break;

    }
    if (hChn->chnSettings.clkRate < BI2C_Clk_eClk187Khz)
        divClk = 0x1;

    regXSettings.enable = true;
    regXSettings.data = BCHP_FIELD_DATA(BSCA_CTL_REG, INT_EN, hChn->chnSettings.intMode)
                      | BCHP_FIELD_DATA(BSCA_CTL_REG, DTF, ctlSettings->mode)
                      | BCHP_FIELD_DATA(BSCA_CTL_REG, SCL_SEL, ctlReg)
                      | BCHP_FIELD_DATA(BSCA_CTL_REG, DIV_CLK, divClk) ;


    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCTL_REG;
    return BAUTO_I2C_P_WriteChxRegX(hChn, eRegXOffset, &regXSettings);
}

static BERR_Code BAUTO_I2C_P_SetIICEnableChannel(
    BI2C_ChannelHandle  hChn,
    BAUTO_I2C_P_CHX_REG_OFFSET  eRegXOffset,
    const BAUTO_I2C_P_IICEnableSettings *iicSettings
)
{
    BAUTO_I2C_P_ChxRegXSettings regXSettings;

    regXSettings.enable = true;
    regXSettings.data = BCHP_FIELD_DATA(BSCA_IIC_ENABLE, RESTART, iicSettings->restart)
                      | BCHP_FIELD_DATA(BSCA_IIC_ENABLE, NO_START, iicSettings->noStart)
                      | BCHP_FIELD_DATA(BSCA_IIC_ENABLE, NO_STOP, iicSettings->noStop)
                      | BCHP_FIELD_DATA(BSCA_IIC_ENABLE, ENABLE, iicSettings->enable);

    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eIIC_ENABLE;
    return BAUTO_I2C_P_WriteChxRegX(hChn, eRegXOffset, &regXSettings);
}

static void BAUTO_I2C_P_EnableChannelWrite(
    BI2C_ChannelHandle      hChn          /* I2C channel handle. */
)
{
    BI2C_Handle hDev = hChn->hI2c;
    uint32_t val=0x0;

    val = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_HDMI_TX_AUTO_I2C_START));
    val |=  0x1 << hChn->autoI2c.channelNumber;
    BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_HDMI_TX_AUTO_I2C_START), val) ;

}

static void BAUTO_I2C_P_ClearChannelDoneStatus(
    BI2C_ChannelHandle      hChn          /* I2C channel handle. */
)
{
    BI2C_Handle hDev = hChn->hI2c;
    uint32_t val=0x0;

    val =  0x1 << hChn->autoI2c.channelNumber;
    BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_HDMI_TX_AUTO_I2C_TRANSACTION_DONE_STAT_CLEAR), val) ;
}

static BERR_Code BAUTO_I2C_P_DisableChannel(
    BI2C_ChannelHandle      hChn          /* I2C channel handle. */
)
{
    BERR_Code   retCode = BERR_SUCCESS;
    BAUTO_I2C_P_IICEnableSettings iicSettings;
    BAUTO_I2C_P_CtlSettings  ctlSettings;
    BAUTO_I2C_P_TriggerConfiguration triggerConfig;

    /* Reset the IIC ENABLE register */
    iicSettings.restart = false;
    iicSettings.noStart = false;
    iicSettings.noStop  = false;
    iicSettings.enable  = false;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e0, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto done;}

    ctlSettings.mode = BI2C_P_MODE_eWriteOnly;
    retCode = BAUTO_I2C_P_SetCTLChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e1, &ctlSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto done;}

    /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
    retCode = BAUTO_I2C_P_SetTerminateChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e2);
    if(retCode){ (void)BERR_TRACE(retCode);goto done;}

    BAUTO_I2C_P_GetDefaultTriggerConfig(&triggerConfig);
    triggerConfig.eMode = BAUTO_I2C_P_MODE_eWrite;
    BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);

    BAUTO_I2C_P_EnableChannelWrite(hChn);
    retCode = BAUTO_I2C_P_WaitForCompletion(hChn, 1);
    if(retCode){ (void)BERR_TRACE(retCode);goto done;}
    BAUTO_I2C_P_ClearChannelDoneStatus(hChn);

done:
    return retCode;
}

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
/***********************************************************************func
 * Name: BI2C_AUTO_P_WriteBy4BytesCmd
 *   - Write data to an i2c slave device.
 *
 * NOTE: This function will be called by BI2C_P_Write, BI2C_P_WriteA16, BI2C_P_WriteNoAddr
 ***************************************************************************/
BERR_Code BI2C_AUTO_P_WriteBy4BytesCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t            chipAddr,       /* i2c chip address.  this is unshifted */
    uint32_t            subAddr,        /* sub address */
    uint8_t             numSubAddrBytes,    /* number of bytes in register address */
    const uint8_t       *pData,         /* storage */
    size_t              numBytes,       /* number of bytes to write */
    bool                isNvram,        /* is this a nvram access? */
    bool                mutex,           /* protect with a mutex */
    bool                ack,            /* check for ack? */
    bool                noStop          /* do we need a stop at the end of the transfer */
)
{
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint8_t             maxWriteBytes;
    uint32_t            writeCmdWord, bufferIndex, lval = 0, i=0;
    uint32_t            cnt=0;
    BAUTO_I2C_P_ChxRegXSettings regXSettings;
    BAUTO_I2C_P_TriggerConfiguration triggerConfig;
    BAUTO_I2C_P_IICEnableSettings iicSettings;
    BAUTO_I2C_P_CtlSettings  ctlSettings;
    BAUTO_I2C_BSCX_P_OFFSET writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN0;
    BAUTO_I2C_P_CHX_REG_OFFSET regChannel;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    BSTD_UNUSED(isNvram);
    BSTD_UNUSED(ack);
    BSTD_UNUSED(noStop);

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    maxWriteBytes = MAX_I2C_WRITE_REQUEST*4;

    BAUTO_I2C_P_GetDefaultRegXSettings(&regXSettings);

    /* Reset the IIC ENABLE register */
    iicSettings.restart = false;
    iicSettings.noStart = false;
    iicSettings.noStop  = false;
    iicSettings.enable  = false;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e0, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    /* Set the ADDRESS register */
    regXSettings.data = (uint32_t) ((chipAddr & 0x7f) << 1);
    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCHIP_ADDR;
    retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e1, &regXSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    numWriteBytes += numSubAddrBytes;

    ctlSettings.mode = BI2C_P_MODE_eWriteOnly;
    retCode = BAUTO_I2C_P_SetCTLChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e2, &ctlSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    writeCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;
    regChannel = BAUTO_I2C_P_CHX_REG_OFFSET_e4;

    do
    {
        i = 0;
        if (cnt+numWriteBytes <= maxWriteBytes)
        {
            lval = cnt+numWriteBytes;
            iicSettings.noStop  = false;
        }
        else
        {
            lval = maxWriteBytes;

            iicSettings.noStop  = true;
            writeCmdWord |= BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK;
        }

        iicSettings.restart = false;
        iicSettings.noStart = false;
        iicSettings.enable  = true;

        /* Set the COUNT register */
        regXSettings.data = lval;
        regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCNT_REG;
        retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e3, &regXSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        /* copy data from buffer */
        if (lval) /* lval may be zero because of a zero byte write */
        {
            for(i = 0; i < ((lval-1)/4)+1; i++)
            {
                uint32_t j;
                uint32_t tempData=0;
                switch (numWriteBytes)
                {
                    case 0:
                        for (j=0; (j<4) && (j<cnt); j++)
                        {
                            tempData |= (uint32_t)((pData[bufferIndex+j]<<(8*j)));
                        }
                        cnt-=j;
                        bufferIndex += 4;
                        break;
                    case 1:
                        tempData = subAddr;
                        for (j=0; (j<3) && (j<cnt); j++)
                        {
                            tempData |= (uint32_t)((pData[bufferIndex+j]<<(8*(j+1))));
                        }
                        cnt-=j;
                        bufferIndex += 3;
                        break;
                    default:
                        break;
                }

                numWriteBytes=0; /* done taking care of sub address */

                regXSettings.data = tempData;
                regXSettings.writeAddressOffset = writeAddressOffset+i;
                retCode = BAUTO_I2C_P_WriteChxRegX(hChn, regChannel+i, &regXSettings);
                if(retCode){ (void)BERR_TRACE(retCode);goto error;}
            }
        }
        /* start the write command */
        retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, regChannel + i, &iicSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
        retCode = BAUTO_I2C_P_SetTerminateChannel(hChn, regChannel + i + 1);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        BAUTO_I2C_P_GetDefaultTriggerConfig(&triggerConfig);
        triggerConfig.eMode = BAUTO_I2C_P_MODE_eWrite;
        retCode = BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        BAUTO_I2C_P_EnableChannelWrite(hChn);
        retCode = BAUTO_I2C_P_WaitForCompletion(hChn, 1);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}
        BAUTO_I2C_P_ClearChannelDoneStatus(hChn);

        if (cnt)
        {
            writeCmdWord = BCHP_BSCA_IIC_ENABLE_NO_START_MASK;
            regChannel = BAUTO_I2C_P_CHX_REG_OFFSET_e4;
            writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN0;
        }
    }
    while (cnt);

error:
    BAUTO_I2C_P_DisableChannel(hChn);

    if (mutex)
        BI2C_P_RELEASE_MUTEX( hChn );

    return retCode;
}

/***********************************************************************func
 * Name: BI2C_P_ReadBy4BytesCmd
 *   - Read data from i2c slave device.
 *       write (1 byte of slave device register's address, subAddr)
 *   and read  (0-8 bytes of data)
 *   Using i2c Write.& Read
 *
 * NOTE: This function will be called by BI2C_P_Read, BI2C_P_ReadA16, BI2C_P_ReadNoAddr
 ***************************************************************************/
BERR_Code BI2C_AUTO_P_ReadBy4BytesCmd
(
    BI2C_ChannelHandle  hChn,               /* Device channel handle */
    uint16_t            chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t            subAddr,            /* sub address */
    uint8_t             numSubAddrBytes,    /* number of bytes in register address */
    uint8_t             *pData,             /* storage */
    size_t              numBytes,           /* number of bytes to read */
    bool                mutex,              /* protect with a mutex */
    bool                ack,                /* check for ack? */
    bool                noStop              /* no stop at the end? */
)
{
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             maxReadBytes;
    uint8_t             numReadBytes;
    uint32_t            tempData=0;
    uint32_t            bufferIndex, readNum = 0, i;
    uint32_t            cnt=0;
    BAUTO_I2C_P_ChxRegXSettings regXSettings;
    BAUTO_I2C_P_TriggerConfiguration triggerConfig;
    BAUTO_I2C_P_IICEnableSettings iicSettings;
    BAUTO_I2C_P_CtlhiSettings ctlhiSettings;
    BAUTO_I2C_P_CtlSettings  ctlSettings;

    BSTD_UNUSED(ack);
    BSTD_UNUSED(noStop);

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    maxReadBytes = MAX_AUTO_I2C_READ_REQUEST;

    BAUTO_I2C_P_GetDefaultRegXSettings(&regXSettings);

    /* Reset the IIC ENABLE register */
    iicSettings.restart = false;
    iicSettings.noStart = false;
    iicSettings.noStop  = false;
    iicSettings.enable  = false;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e0, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    /* Set the ADDRESS register */
    regXSettings.data = (uint32_t) ((chipAddr & 0x7f) << 1);
    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCHIP_ADDR;
    retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e1, &regXSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}


   if(numSubAddrBytes){
       /* Set the OFFSET register */
        regXSettings.data = (uint32_t)(subAddr & 0xff);
        regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN0;
        retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e2, &regXSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        /* Set the COUNT register */
        regXSettings.data = 1;
        regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCNT_REG;
        retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e3, &regXSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        ctlSettings.mode = BI2C_P_MODE_eWriteOnly;
        retCode = BAUTO_I2C_P_SetCTLChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e4, &ctlSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        ctlhiSettings.ignoreAck = true;
        retCode = BAUTO_I2C_P_SetCTLHIChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e5, &ctlhiSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        iicSettings.restart = true;
        iicSettings.noStart = false;
        iicSettings.noStop  = true;
        iicSettings.enable  = true;
        retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e6, &iicSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
        retCode = BAUTO_I2C_P_SetTerminateChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e7);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        BAUTO_I2C_P_GetDefaultTriggerConfig(&triggerConfig);
        triggerConfig.eMode = BAUTO_I2C_P_MODE_eWrite;
        BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);

        /* Above I configured the ADDRESS and OFFSET register to be WRITTEN. Now, I am triggering the transaction. */
        BAUTO_I2C_P_EnableChannelWrite(hChn);
        retCode = BAUTO_I2C_P_WaitForCompletion(hChn, 1);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}
        BAUTO_I2C_P_ClearChannelDoneStatus(hChn);
    }

    /* Trigger COMPLETE. Address and offset written. */
    /*Now, setting the channel for reading.  */
    ctlSettings.mode = BI2C_P_MODE_eReadOnly;
    retCode = BAUTO_I2C_P_SetCTLChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e3, &ctlSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    ctlhiSettings.ignoreAck = false;
    retCode = BAUTO_I2C_P_SetCTLHIChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e4, &ctlhiSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    iicSettings.restart = false;
    iicSettings.noStart = false;
    iicSettings.noStop  = true;
    iicSettings.enable  = true;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e5, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
    retCode = BAUTO_I2C_P_SetTerminateChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e6);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    BAUTO_I2C_P_GetDefaultTriggerConfig(&triggerConfig);
    triggerConfig.eMode = BAUTO_I2C_P_MODE_eRead;
    BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);

    cnt = numBytes;
    bufferIndex = 0;

    while (cnt)
    {
        if (cnt <= maxReadBytes)
        {
            /* program amount of bytes to read */
            readNum = cnt;
            cnt = 0;

            iicSettings.restart = false;
            if(numBytes>maxReadBytes)
                iicSettings.noStart = true;
            else
                iicSettings.noStart = false;
            iicSettings.noStop  = false;
            iicSettings.enable  = true;
            retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e5, &iicSettings);
            if(retCode){ (void)BERR_TRACE(retCode);goto error;}
        }
        else
        {
            /* program amount of bytes to read */
            readNum = maxReadBytes;
            cnt -= maxReadBytes;
            /* delay improves recovery of AutoI2C HW core when simultaneous operation of individual AutoI2C channels..
             * Simultaneous operation is defined as I2C read/write channel x (like EDID read) and HDCP 2.2 Authentication on channel y.
             */
            /*
             * The value of 100us was determine using a test application simulating simultaneous operation.
             * Less 100us caused a lock-up on the AutoI2C HW, requiring a Hotplug to recovery.
             *
             * */
             BKNI_Delay(100);
        }

        /* MAKE THIS PART OF THE SUB FUNCTION SETTING THE CNT REG. */
        if(readNum <= 4){
            triggerConfig.eMode = BAUTO_I2C_P_MODE_ePollScdcUpdate0;
            retCode = BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);
            if(retCode){ (void)BERR_TRACE(retCode);goto error;}
        }

        regXSettings.data = readNum;
        regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCNT_REG;
        retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e2, &regXSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}

        BAUTO_I2C_P_EnableChannelWrite(hChn);
        retCode = BAUTO_I2C_P_WaitForCompletion(hChn, 1);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}
        BAUTO_I2C_P_ClearChannelDoneStatus(hChn);

        /* copy data to buffer */
        /* Read operation is by 4 bytes */
        for(i = 0; i < ((readNum-1)/4)+1; i++)
        {
            retCode = BAUTO_I2C_P_ReadRegister(hChn, BAUTO_I2C_P_CHX_RD_0 + i, &tempData );
            if(retCode){ (void)BERR_TRACE(retCode);goto error;}

            if (i == ((readNum-1)/4))
            {
                numReadBytes = ((readNum-1)%4) + 1; /* last read may have less than 4 bytes */
            }
            else
            {
                numReadBytes = 4;
            }

            pData[bufferIndex++] = (uint8_t)(tempData & 0x000000FF);

            if ((numReadBytes == 2) || (numReadBytes == 3) || (numReadBytes == 4))
            {
                pData[bufferIndex++] = (uint8_t)((tempData & 0x0000FF00) >> 8);
            }
            if ((numReadBytes == 3) || (numReadBytes == 4))
            {
                pData[bufferIndex++] = (uint8_t)((tempData & 0x00FF0000) >> 16);
            }
            if (numReadBytes == 4)
            {
                pData[bufferIndex++] = (uint8_t)((tempData & 0xFF000000) >> 24);
            }
        }

        iicSettings.restart = false;
        iicSettings.noStart = true;
        iicSettings.noStop  = true;
        iicSettings.enable  = true;
        retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e5, &iicSettings);
        if(retCode){ (void)BERR_TRACE(retCode);goto error;}
    }

error:
    BAUTO_I2C_P_DisableChannel(hChn);

    if (mutex)
        BI2C_P_RELEASE_MUTEX( hChn );

    return retCode;
}
#endif

BERR_Code BI2C_Auto_P_ReadEDDC(
    void                *context,               /* Device channel handle */
    uint8_t             chipAddr,               /* chip address */
    uint8_t             segment,                /* EDDC segment */
    uint32_t            subAddr,                /* 8-bit sub address */
    uint8_t             *pData,                 /* pointer to memory location to store read data */
    size_t              length                  /* number of bytes to read */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;
    BAUTO_I2C_P_TriggerConfiguration triggerConfig;
    BAUTO_I2C_P_IICEnableSettings iicSettings;
    BAUTO_I2C_P_CtlhiSettings ctlhiSettings;
    BAUTO_I2C_P_CtlSettings  ctlSettings;

    BAUTO_I2C_P_ChxRegXSettings regXSettings;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    BI2C_P_ACQUIRE_MUTEX( hChn );

    BAUTO_I2C_P_GetDefaultRegXSettings(&regXSettings);

    /***********************************
     * Step 1: Write the segment pointer
     **********************************/
    iicSettings.restart = false;
    iicSettings.noStart = false;
    iicSettings.noStop  = false;
    iicSettings.enable  = false;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e0, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    regXSettings.data = (uint32_t)EDDC_SEGMENT_CHIP_ADDR;
    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCHIP_ADDR;
    retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e1, &regXSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    regXSettings.data = (uint32_t)segment;
    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN0;
    retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e2, &regXSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    /* Understand the difference between CNT_REG1 and CNT_REG2 */
    regXSettings.data = 1;
    regXSettings.writeAddressOffset = BAUTO_I2C_BSCX_P_OFFSET_eCNT_REG;
    retCode = BAUTO_I2C_P_WriteChxRegX(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e3, &regXSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    ctlSettings.mode = BI2C_P_MODE_eWriteOnly;
    retCode = BAUTO_I2C_P_SetCTLChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e4, &ctlSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    ctlhiSettings.ignoreAck = true;
    retCode = BAUTO_I2C_P_SetCTLHIChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e5, &ctlhiSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    iicSettings.restart = true;
    iicSettings.noStart = false;
    iicSettings.noStop  = true;
    iicSettings.enable  = true;
    retCode = BAUTO_I2C_P_SetIICEnableChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e6, &iicSettings);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    /*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
    retCode = BAUTO_I2C_P_SetTerminateChannel(hChn, BAUTO_I2C_P_CHX_REG_OFFSET_e7);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    BAUTO_I2C_P_GetDefaultTriggerConfig(&triggerConfig);
    triggerConfig.eMode = BAUTO_I2C_P_MODE_eWrite;
    retCode = BAUTO_I2C_P_SetTriggerConfiguration(hChn, &triggerConfig);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}

    BAUTO_I2C_P_EnableChannelWrite(hChn);
    retCode = BAUTO_I2C_P_WaitForCompletion(hChn, 1);
    if(retCode){ (void)BERR_TRACE(retCode);goto error;}
    BAUTO_I2C_P_ClearChannelDoneStatus(hChn);

    /****************************************
     * Step 2: Read the data with sub address
     ***************************************/
    retCode = BI2C_AUTO_P_ReadBy4BytesCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*mutex*/, true /*ack*/, false /*no stop*/);
    if(retCode) (void)BERR_TRACE(retCode);

error:
    BAUTO_I2C_P_DisableChannel(hChn);

    BI2C_P_RELEASE_MUTEX( hChn );
    return( retCode );
}
#endif
