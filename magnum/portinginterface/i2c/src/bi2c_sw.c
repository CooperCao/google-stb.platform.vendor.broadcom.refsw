/***************************************************************************
*     Copyright (c) 2002-2014, Broadcom Corporation
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
#include "bi2c_priv.h"

BDBG_MODULE(bi2c_sw);

static void BI2C_P_RegBitSet(BREG_Handle hReg, uint32_t addr, uint32_t mask, uint8_t val);
static BERR_Code _BI2C_P_ClkSet(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int clk, BI2C_Clk rate, int enable_clk_stretch);
static int _BI2C_P_DataGet(BREG_Handle hReg, BI2C_ChannelHandle  hChn);
static BERR_Code _BI2C_P_DataSet(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int data, BI2C_Clk rate);
static BERR_Code _BI2C_P_ByteWrite(BREG_Handle hReg, BI2C_ChannelHandle  hChn, unsigned char data, BI2C_Clk rate, int *ret_ack);
static BERR_Code _BI2C_P_ByteRead(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int generate_ack, BI2C_Clk rate, unsigned char *ret_data);

#define BI2C_P_ClkSet(clk) _BI2C_P_ClkSet(hReg, hChn, clk, rate, true)
#define BI2C_P_ClkSet_no_clk_stretch(clk) _BI2C_P_ClkSet(hReg, hChn, clk, rate, false)
#define BI2C_P_DataGet() _BI2C_P_DataGet(hReg, hChn)
#define BI2C_P_DataSet(clk) _BI2C_P_DataSet(hReg, hChn, clk, rate)
#define BI2C_P_ByteWrite(data, ret_ack) _BI2C_P_ByteWrite(hReg, hChn, data, rate, ret_ack)
#define BI2C_P_ByteRead(generate_ack, ret_data) _BI2C_P_ByteRead(hReg, hChn, generate_ack, rate, ret_data)

#define BI2C_P_IodirScl(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.scl.iodir, hChn->softI2cBus.scl.mask, val)
#define BI2C_P_IodirSda(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.sda.iodir, hChn->softI2cBus.sda.mask, val)
#define BI2C_P_OdenScl(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.scl.oden, hChn->softI2cBus.scl.mask, val)
#define BI2C_P_OdenSda(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.sda.oden, hChn->softI2cBus.sda.mask, val)
#define BI2C_P_DataScl(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.scl.data, hChn->softI2cBus.scl.mask, val)
#define BI2C_P_DataSda(val) BI2C_P_RegBitSet(hReg, hChn->softI2cBus.sda.data, hChn->softI2cBus.sda.mask, val)

/***********************************************************************func
 * Name: BI2C_P_RegBitSet
 *   - set register bit, return previous value.
 *
 * NOTE:
 ***************************************************************************/
static void BI2C_P_RegBitSet(BREG_Handle hReg, uint32_t addr, uint32_t mask, uint8_t val)
{
	BREG_Update32(hReg, addr, mask, val);

    return;
}

/***********************************************************************func
 * Name: BI2C_P_Delay
 *   -
 *
 * NOTE:  This function is used to delay the clock and data pulse to simulate
 * as best as possible the real clock and data pulse of the BSC.  It is
 * obtained through experimentation in the lab via a scope.
 ***************************************************************************/
static void BI2C_P_Delay(BI2C_Clk rate)
{
    /* If you get a compile error not having this function defined. You should have this defined
        in kernel interface. */
    /* bcmKNIDelay(10); */
    switch (rate)
    {
        case BI2C_Clk_eClk47Khz:
            BKNI_Delay(7); /* 43KHz*/
            break;
        case BI2C_Clk_eClk50Khz:
            BKNI_Delay(6); /* 48KHz*/
            break;
        case BI2C_Clk_eClk93Khz:
            BKNI_Delay(2); /* 81KHz*/
            break;
        case BI2C_Clk_eClk100Khz:
        case BI2C_Clk_eClk187Khz:
        case BI2C_Clk_eClk200Khz:
        case BI2C_Clk_eClk375Khz:
        case BI2C_Clk_eClk400Khz:
        default:
            break;
    }
}

/***********************************************************************func
 * Name: BI2C_P_ClkSet
 *   - set the CLOCK bit high/low.
 *
 * NOTE:
 ***************************************************************************/
static BERR_Code _BI2C_P_ClkSet(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int clk, BI2C_Clk rate, int enable_clk_stretch)
{
#ifdef ORGINAL_CLOCK_STRETCH_IMPL
    uint32_t data, count=0;
#define CLK_STRETCH_TIMEOUT 5000000 /* 5 seconds */
#else
    uint32_t data, count=0, incr=1, timeout_ms=0, tmp=0, timeout_uS=0;
#define CLK_STRETCH_TIMEOUT 200 /* 200 milli seconds */
#define  CLK_STRETCH_DELAY_MICRO_SEC 200 /*200 Micro seconds*/
#endif
    uint32_t mask = ~hChn->softI2cBus.scl.mask;
    uint32_t addr = hChn->softI2cBus.scl.data;

    /* put in delay for timing purposes */
    BI2C_P_Delay(rate);

    BI2C_P_IodirScl(clk);

    /* clock stretching */
    if (clk)
    {
        /* we have to wait until we see clock is set before we leave. */
#ifdef ORGINAL_CLOCK_STRETCH_IMPL
        while (!(BREG_Read32(hReg, addr) & mask))
        {
            BKNI_Delay(1);
            if(++count > CLK_STRETCH_TIMEOUT)
            {
                BDBG_ERR(("clock stretch timer expired."));
                return BERR_TIMEOUT;
            }
        }
#else
        /* DRI-1661: Fix for system freeze issue when a defective TV in standby mode.
         * Fix for defective TVs which pulls down the clock and data line too low when
         * TV in standby mode and never exits from the clock stretch detection busy loop.
         */
        while (enable_clk_stretch)
        {
            data = (BREG_Read32(hReg, addr) & mask); /*  dummy */
            data = (BREG_Read32(hReg, addr) & mask); /* dummy2 */
            data = (BREG_Read32(hReg, addr) & mask); /* real */

            if (data)
            {
                break;
            }
            else
            {
                if(timeout_ms > CLK_STRETCH_TIMEOUT)
                {
                    BDBG_ERR(("clock stretch timer expired %d ms and %d uS.", timeout_ms, timeout_uS));
                    return BERR_TIMEOUT;
                }
                /* DRI-1661: if (1 clock cycle = 100khz = 10 uS) and so sleeping too long in Milliseconds
                 * might cause the slave to timeout in case of the actual clock stretch. So busy looping in
                 * Microseconds range initially for actual clock stretch detection with timeout of  < 1ms and
                 * sleeping in Milliseconds range afterwards in case of failure with defective TVs
                 */
                if(++timeout_uS < CLK_STRETCH_DELAY_MICRO_SEC)
                {
                    BKNI_Delay(1);
                }
                else
                {
                    tmp = count + incr;
                    count = incr;
                    incr = tmp;
                    timeout_ms += count;
                    BKNI_Sleep(count);
                }
            }
        }
#endif
    }
    else
    {
        /* dummy read to force the write to flush out */
        data = BREG_Read32(hReg, addr);
    }

    return BERR_SUCCESS;
}

/***********************************************************************func
 * Name: BI2C_P_DataSet
 *   - set the DATA bit high/low.
 *
 * NOTE:
 ***************************************************************************/
static BERR_Code _BI2C_P_DataSet(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int data, BI2C_Clk rate)
{

    /* put in delay for timing purposes */
    BI2C_P_Delay(rate);

    BI2C_P_IodirSda(data);

    return BERR_SUCCESS;
}

/***********************************************************************func
 * Name: BI2C_P_DataGet
 *   - returns the state of the DATA bit (high/low).
 *
 * NOTE:
 ***************************************************************************/
static int _BI2C_P_DataGet(BREG_Handle hReg, BI2C_ChannelHandle  hChn)
{
    uint32_t port_data;

    port_data = BREG_Read32(hReg, hChn->softI2cBus.sda.data);

    /* return state of DATA bit */
    if (port_data & (~hChn->softI2cBus.sda.mask))
        return 1;
    else return 0;
}

/***********************************************************************func
 * Name: _BI2C_P_ByteWrite
 *   - clocks out 1 bytes of data and returns the status
 *     of the Ack/Nack bit.
 *
 * NOTE:
 ***************************************************************************/
static BERR_Code _BI2C_P_ByteWrite(BREG_Handle hReg, BI2C_ChannelHandle  hChn, unsigned char data, BI2C_Clk rate, int *ret_ack)
{
    BERR_Code rc;
    unsigned char i;

    /* clock out data byte */
    for (i = 0; i < 8; i++)
    {
        BI2C_P_ClkSet(0);
        BI2C_P_DataSet(data & 0x80);
        data = data << 1;
        if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) return rc;
    }

    BI2C_P_ClkSet(0);
    BI2C_P_DataSet(1);

    /* read ack bit */
    if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) return rc;
    *ret_ack = BI2C_P_DataGet();
    BI2C_P_ClkSet(0);
    BI2C_P_DataSet(0);
    return BERR_SUCCESS;
}

/***********************************************************************func
 * Name: _BI2C_P_ByteRead
 *   - reads 1 bytes of data and returns the status of the
 *     Ack/Nack bit.
 *
 * NOTE:
 ***************************************************************************/
static BERR_Code _BI2C_P_ByteRead(BREG_Handle hReg, BI2C_ChannelHandle  hChn, int generate_ack, BI2C_Clk rate, unsigned char *ret_data)
{
    BERR_Code rc;
    unsigned char data;
    unsigned char i;

    /* initialize data to 0 */
    data = 0;

    /* clock in data byte */
    for (i = 0; i < 8; i++)
    {
        BI2C_P_ClkSet(0);
        BI2C_P_DataSet(1);
        if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) return rc;
        data = (data << 1) | BI2C_P_DataGet();
    }

    BI2C_P_ClkSet(0);

    /* generate Ack/Nack */
    if (generate_ack)
        BI2C_P_DataSet(0);
    else BI2C_P_DataSet(1);

    /* clock in Ack/Nack */
    if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) return rc;
    BI2C_P_ClkSet(0);
    BI2C_P_DataSet(0);

    *ret_data = data;
    return BERR_SUCCESS;
}

/***********************************************************************func
 * Name: BI2C_SwReset
 *   - Perform a software reset.
 *
 ***************************************************************************/
BERR_Code BI2C_SwReset
(
    BI2C_ChannelHandle  hChn           /* Device channel handle */
)
{
    uint16_t i;
    int no_ack = 0;
    BI2C_Handle hDev;
    BREG_Handle hReg;
    BI2C_Clk rate = hChn->chnSettings.clkRate;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;
    hReg = hDev->hRegister;

    /* Set open drain output */
    BI2C_P_OdenScl(1);
    BI2C_P_OdenSda(1);

    /* reset CLOCK and DATA */
    if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) goto no_success;
    BI2C_P_DataSet(1);

    /* Set gpio as output */
    BI2C_P_IodirScl(0);
    BI2C_P_IodirSda(0);

    /* generate start condition */
    BI2C_P_DataSet(0);
    BI2C_P_ClkSet(0);
    BI2C_P_DataSet(1);

    /* dummy clock cycles 1 through 9 */
    for (i=0; i<9; i++)
    {
        if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) goto no_success;
        BI2C_P_ClkSet(0);
    }

    /* start condition */
    if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) goto no_success;
    BI2C_P_DataSet(0);

    /* stop condition */
    BI2C_P_ClkSet(0);
    if ((rc = BI2C_P_ClkSet(1)) != BERR_SUCCESS) goto no_success;
    BI2C_P_DataSet(1);
    BI2C_P_ClkSet(0);   /* Needed ????*/

no_success:
    if(rc != BERR_SUCCESS) return rc;
    if (no_ack) return BI2C_ERR_NO_ACK;
    else return BERR_SUCCESS;
}


/***********************************************************************func
* Name: BI2C_P_Write
*   - Write data to an i2c slave device.
*
* NOTE: This function will be called by BI2C_P_Write, BI2C_P_WriteA16, BI2C_P_WriteNoAddr
***************************************************************************/
BERR_Code BI2C_P_WriteSwCmd
(
    BI2C_ChannelHandle  hChn,            /* Device channel handle */
    uint16_t             chipAddr,       /* i2c chip address.  this is unshifted */
    uint32_t             subAddr,        /* pointer to register address */
    uint8_t          numSubAddrBytes,    /* number of bytes in register address */
    const uint8_t        *pData,     /* storage */
    size_t           numBytes,       /* number of bytes to write */
    bool                 isNvram         /* is this a nvram access? */
)
{
    uint16_t i;
    int no_ack = 0;
    BI2C_Handle hDev;
    BREG_Handle hReg;
    BI2C_Clk rate = hChn->chnSettings.clkRate;
    int ret_ack;
    BERR_Code rc = BERR_SUCCESS;
    uint32_t lval = 0;

    BDBG_MSG(("BI2C_P_WriteSwCmd:   ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d, subAddr=0x%x, numBytes=%d, pData=0x%x.",
        hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, subAddr, numBytes, *(uint8_t *)pData));

    BSTD_UNUSED( isNvram );
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pData );

    BI2C_P_ACQUIRE_MUTEX( hChn );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;
    hReg = hDev->hRegister;

    /* Set open drain output */
    BI2C_P_OdenScl(1);
    BI2C_P_OdenSda(1);

    /* Set iodir to output so we can change data */
    BI2C_P_IodirScl(0);
    BI2C_P_IodirSda(0);

    /* Set data low for using iodir to control output */
    BI2C_P_DataScl(0);
    BI2C_P_DataSda(0);

    /* Set iodir to input so we can start with SCL and SDA high */
    BI2C_P_IodirScl(1);
    BI2C_P_IodirSda(1);

    /* Don't support 10 bit chip addr yet */
    if (chipAddr & 0x0380)           /* 10-bit chip address */
    {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("10 bit chip address not supported."));
        goto no_success;
    }


    /* Don't support sub address bytes yet */
    if (numSubAddrBytes > 1)
    {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Sub address not supported."));
        goto no_success;
    }

    chipAddr = (chipAddr & 0x7f) << 1;

    /* generate start condition */
    BI2C_P_DataSet(0);

    if ((rc = BI2C_P_ByteWrite(chipAddr & 0xfe, &ret_ack)) != BERR_SUCCESS) goto no_success;
    if (ret_ack != I2C_ACK)
    {
        BDBG_ERR(("no ack!"));
        no_ack = 1;
        goto no_ack_detected;
    }

    if (numSubAddrBytes == 1)
    {
        lval = subAddr & 0xff;
        if ((rc = BI2C_P_ByteWrite(lval, &ret_ack)) != BERR_SUCCESS) goto no_success;
        if (ret_ack != I2C_ACK)
        {
         BDBG_ERR(("no ack!"));
         no_ack = 1;
         goto no_ack_detected;
        }
    }

    for (i = 0; i < numBytes; i++)
    {
         if ((rc = BI2C_P_ByteWrite(pData[i], &ret_ack) != BERR_SUCCESS)) goto no_success;
         if (ret_ack != I2C_ACK)
         {
             BDBG_ERR(("no ack!"));
             no_ack = 1;
             goto no_ack_detected;
         }
    }

no_success:
no_ack_detected:
    /* generate stop condition */
    BI2C_P_ClkSet_no_clk_stretch(1);
    BI2C_P_DataSet(1);

    if(rc != BERR_SUCCESS){
        BDBG_ERR(("BI2C_P_WriteSwCmd: failed ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d, subAddr=0x%x, numBytes=%d, pData=0x%x.",
            hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, numSubAddrBytes ? subAddr : 0, numBytes, *(uint8_t *)pData));
    }

    BI2C_P_RELEASE_MUTEX( hChn );

    if (no_ack) return BI2C_ERR_NO_ACK;
    else return rc;
}

BERR_Code BI2C_P_WriteSw
(
    void *context,           /* Device channel handle */
    uint16_t chipAddr,               /* chip address */
    uint32_t subAddr,                     /* 8-bit sub address */
    const uint8_t *pData,                /* pointer to data to write */
    size_t length                        /* number of bytes to write */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_WriteSwCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/);
}

BERR_Code BI2C_P_WriteSwNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_WriteSwCmd (hChn, chipAddr, 0, 0, pData, length, false /*EDDC*/);
}

BERR_Code BI2C_P_WriteSwA16
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 16-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_WriteSwCmd (hChn, chipAddr, subAddr, 2, pData, length, false /*NVRAM*/);
}

BERR_Code BI2C_P_WriteSwA24
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 24-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_WriteSwCmd (hChn, chipAddr, subAddr, 3, pData, length, false /*NVRAM*/);
}

/***********************************************************************func
 * Name: BI2C_P_ReadSwCmd
 *   - Read data from i2c slave device.
 *       write (1 byte of slave device register's address, subAddr)
 *   and read  (0-8 bytes of data)
 *   Using i2c Write.& Read
 *
 * NOTE: This function will be called by BI2C_P_Read, BI2C_P_ReadA16, BI2C_P_ReadNoAddr
 ***************************************************************************/
BERR_Code BI2C_P_ReadSwCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t        chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t        subAddr,            /* pointer to register address */
    uint8_t         numSubAddrBytes,    /* number of bytes in register address */
    uint8_t         *pData,             /* storage */
    size_t          numBytes,           /* number of bytes to read */
    bool            eddc,               /* EDDC mode */
    uint8_t         segment,            /* EDDC segment */
    bool            checkForAck         /* Check for ack? */
)
{
    uint16_t i;
    int no_ack = 0;
    BI2C_Handle hDev;
    BREG_Handle hReg;
    BI2C_Clk rate = hChn->chnSettings.clkRate;
    uint8_t lval;
    int ret_ack;
    BERR_Code rc = BERR_SUCCESS;
    unsigned char ret_data;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pData );

    BI2C_P_ACQUIRE_MUTEX( hChn );

#define RD 0x01
#define WR 0x00

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;
    hReg = hDev->hRegister;

    /* Set open drain output */
    BI2C_P_OdenScl(1);
    BI2C_P_OdenSda(1);

    /* Set iodir to output so we can change data */
    BI2C_P_IodirScl(0);
    BI2C_P_IodirSda(0);

    /* Set data low for using iodir to control output */
    BI2C_P_DataScl(0);
    BI2C_P_DataSda(0);

    /* Set iodir to input so we can start with SCL and SDA high */
    BI2C_P_IodirScl(1);
    BI2C_P_IodirSda(1);

    /* Write does not support 10 bit addresses, while read does support for now. */
    /* Don't support sub address bytes yet */
    if (numSubAddrBytes > 1)
    {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Sub address not supported."));
        goto no_success;
    }

    /* generate start condition */
    BI2C_P_DataSet(0);

    if (eddc)
    {
        if (segment)
        {
            /* program slave device's (id) */
            /* BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), (uint32_t)EDDC_SEGMENT_CHIP_ADDR ); */
            if ((rc = BI2C_P_ByteWrite((unsigned char)(EDDC_SEGMENT_CHIP_ADDR), &ret_ack)) != BERR_SUCCESS) goto no_success;
            if (ret_ack != I2C_ACK)
            {
                /*
                BDBG_ERR(("ignore no ack!\n"));
                no_ack = 1;
                */
            }

            /* write segment pointer into fifo */
            /* BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), (uint32_t)segment ); */
            if ((rc = BI2C_P_ByteWrite((unsigned char)(segment), &ret_ack)) != BERR_SUCCESS) goto no_success;
            if (ret_ack != I2C_ACK)
            {
                BDBG_ERR(("no ack!"));
                no_ack = 1;
                goto no_ack_detected;
            }

            /* generate repeated start condition */
            BI2C_P_DataSet(1);
            BI2C_P_ClkSet(1);
            BI2C_P_DataSet(0);
        }
    }

    if (numSubAddrBytes==1)
    {
        /* Send out the chip address with a write */
        if (chipAddr & 0x0380)              /* 10-bit chip address */
        {
            /* Send bits 9 and 8 */
            lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
            if ((rc = BI2C_P_ByteWrite(lval | WR, &ret_ack)) != BERR_SUCCESS) goto no_success;
            if (ret_ack != I2C_ACK)
            {
                BDBG_ERR(("no ack!"));
                no_ack = 1;
                goto no_ack_detected;
            }
            /* Send the rest of the bits */
            lval = chipAddr & 0xff;
            if ((rc = BI2C_P_ByteWrite(lval, &ret_ack)) != BERR_SUCCESS) goto no_success;
            if (checkForAck)
            {
                if (ret_ack != I2C_ACK)
                {
                    BDBG_ERR(("no ack!"));
                    no_ack = 1;
                    goto no_ack_detected;
                }
            }
        }
        else
        {
            lval = (chipAddr & 0x7f) << 1;
            if ((rc = BI2C_P_ByteWrite(lval | WR, &ret_ack)) != BERR_SUCCESS) goto no_success;
            if (checkForAck)
            {
                if (ret_ack != I2C_ACK)
                {
                    BDBG_ERR(("no ack!"));
                    no_ack = 1;
                    goto no_ack_detected;
                }
            }
        }

        lval = (uint8_t)(subAddr & 0xff);
        if ((rc = BI2C_P_ByteWrite(lval, &ret_ack)) != BERR_SUCCESS) goto no_success;
        if (checkForAck)
        {
            if (ret_ack != I2C_ACK)
            {
                BDBG_ERR(("no ack!"));
                no_ack = 1;
                goto no_ack_detected;
            }
        }

        /* generate repeated start condition */
        BI2C_P_DataSet(1);
        BI2C_P_ClkSet(1);
        BI2C_P_DataSet(0);
    }

    /* Start the read transaction.  Must send out the address again. */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        /* Send bits 9 and 8.  Do not need to send the rest of the 10 bit address. */
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        if ((rc = BI2C_P_ByteWrite(lval | RD, &ret_ack)) != BERR_SUCCESS) goto no_success;
        if (checkForAck)
        {
            if (ret_ack != I2C_ACK)
            {
                BDBG_ERR(("no ack!"));
                no_ack = 1;
                goto no_ack_detected;
            }
        }
    }
    else
    {
        lval = (chipAddr & 0x7f) << 1;
        if ((rc = BI2C_P_ByteWrite(lval | RD, &ret_ack)) != BERR_SUCCESS) goto no_success;
        if (checkForAck)
        {
            if (ret_ack != I2C_ACK)
            {
                BDBG_ERR(("no ack!"));
                no_ack = 1;
                goto no_ack_detected;
            }
        }
    }

    for (i = 0; i < (numBytes-1); i++)
    {
        if ((rc = BI2C_P_ByteRead(1, &ret_data) != BERR_SUCCESS)) goto no_success;
        pData[i] = ret_data;
    }
    if ((rc = BI2C_P_ByteRead(0, &ret_data) != BERR_SUCCESS)) goto no_success;
    pData[i] = ret_data;

no_success:
no_ack_detected:
    /* generate stop condition */
    BI2C_P_ClkSet(1);
    BI2C_P_DataSet(1);

    if(rc != BERR_SUCCESS){
       BDBG_ERR(("BI2C_P_ReadSwCmd: failed ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d, subAddr=0x%x, numBytes=%d, pData=0x%x.",
           hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, numSubAddrBytes ? subAddr : 0, numBytes, *(uint8_t *)pData));
    }

    BI2C_P_RELEASE_MUTEX( hChn );

    if (no_ack) return BI2C_ERR_NO_ACK;
    else return rc;
}


BERR_Code BI2C_P_ReadSw
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,          /* chip address */
    uint32_t subAddr,           /* 8-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;
    return BI2C_P_ReadSwCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*EDDC*/, 0, true /*ack*/);
}

BERR_Code BI2C_P_ReadSwNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,          /* chip address */
    uint32_t subAddr,           /* 8-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_ReadSwCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*EDDC*/, 0, false /*ack*/);
}

BERR_Code BI2C_P_ReadSwA16
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 16-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_ReadSwCmd (hChn, chipAddr, subAddr, 2, pData, length, false /*EDDC*/, 0, true /*ack*/);
}

BERR_Code BI2C_P_ReadSwA24
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 24-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_ReadSwCmd (hChn, chipAddr, subAddr, 3, pData, length, false /*EDDC*/, 0, true /*ack*/);
}

BERR_Code BI2C_P_ReadSwNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_ReadSwCmd (hChn, chipAddr, 0, 0, pData, length, false /*EDDC*/, 0, true /*ack*/);
}

BERR_Code BI2C_P_ReadSwNoAddrNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
)
{
    BI2C_ChannelHandle hChn;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        return BERR_INVALID_PARAMETER;
    }

    hChn = (BI2C_ChannelHandle)context;

    return BI2C_P_ReadSwCmd (hChn, chipAddr, 0, 0, pData, length, false /*EDDC*/, 0, false /*ack*/);
}

BERR_Code BI2C_P_ReadSwEDDC(
    void                *context,               /* Device channel handle */
    uint8_t             chipAddr,               /* chip address */
    uint8_t             segment,                /* EDDC segment */
    uint32_t             subAddr,                /* 8-bit sub address */
    uint8_t             *pData,                 /* pointer to memory location to store read data */
    size_t              length                  /* number of bytes to read */
)
{
    BERR_Code           retCode = BERR_SUCCESS;
    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    retCode = BI2C_P_ReadSwCmd (hChn, chipAddr, subAddr, 1, pData, length, true /*EDDC*/, segment, true /*ack*/);

    return( retCode );
}
