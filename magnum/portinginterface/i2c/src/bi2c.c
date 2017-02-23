/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#include "bi2c_priv.h"
#include "bi2c_sw_priv.h"

BDBG_MODULE(bi2c);
/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
#if BCHP_IRQ0_REG_START
static const BINT_Id IntId[] =
{
    BCHP_INT_ID_iica_irqen
#if BSCB_AVAILABLE
    ,BCHP_INT_ID_iicb_irqen
#endif
#if BSCC_AVAILABLE
    ,BCHP_INT_ID_iicc_irqen
#endif
#if BSCD_AVAILABLE
    ,BCHP_INT_ID_iicd_irqen
#else
#endif
#if BSCE_AVAILABLE
    ,BCHP_INT_ID_iice_irqen
#endif
#if BSCF_AVAILABLE
    ,BCHP_INT_ID_iicf_irqen
#endif
#if BSCG_AVAILABLE
    ,BCHP_INT_ID_iicg_irqen
#endif
};
#elif BCHP_UPG_BSC_IRQ_REG_START
static const BINT_Id IntId[] =
{
    BCHP_INT_ID_iica
#if BSCB_AVAILABLE
    ,BCHP_INT_ID_iicb
#endif
#if BSCC_AVAILABLE
    ,BCHP_INT_ID_iicc
#endif
#if BSCD_AVAILABLE
    ,BCHP_INT_ID_iicd
#else
#endif
#if BSCE_AVAILABLE
    ,BCHP_INT_ID_iice
#endif
#if BSCF_AVAILABLE
    ,BCHP_INT_ID_iicf
#endif
#if BSCG_AVAILABLE
    ,BCHP_INT_ID_iicg
#endif
};
#endif

static const uint32_t coreOffsetVal[] =
{
    0
#if BSCB_AVAILABLE
    ,BCHP_BSCB_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
#if BSCC_AVAILABLE
    ,BCHP_BSCC_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
#if BSCD_AVAILABLE
    ,BCHP_BSCD_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
#if BSCE_AVAILABLE
    ,BCHP_BSCE_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
#if BSCF_AVAILABLE
    ,BCHP_BSCF_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
#if BSCG_AVAILABLE
    ,BCHP_BSCG_CHIP_ADDRESS - BCHP_BSCA_CHIP_ADDRESS
#endif
};

BDBG_OBJECT_ID(BI2C_P_Handle);
BDBG_OBJECT_ID(BI2C_P_ChannelHandle);

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
static BERR_Code BI2C_P_WaitForCompletion
(
    BI2C_ChannelHandle  hChn,                           /* Device channel handle */
    uint32_t            numBytes                        /* number of bytes to transfer */
)
{
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval, loopCnt;

    hDev = hChn->hI2c;
    BSTD_UNUSED(numBytes);

    if (hChn->chnSettings.intMode)
    {
        /*
         * Wait for event, set by ISR
         */
        if ( (retCode = BERR_TRACE(BKNI_WaitForEvent(hChn->hChnEvent, hChn->timeoutMs)) != BERR_SUCCESS) )
        {
            BKNI_ResetEvent(hChn->hChnEvent);
            goto done;
        }
        if (hChn->noAck)
        {
            BDBG_MSG(("BI2C_P_WaitForCompletion: BI2C_ERR_NO_ACK(0x110001) at line=%d", __LINE__));
            retCode = BI2C_ERR_NO_ACK;
            goto done;
        }
    }
    else
    {
        /*
         * Polling mode
         */
        loopCnt = ((hChn->timeoutMs * 1000) / I2C_POLLING_INTERVAL) + 1;
        while (1)
        {
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));
            if (lval & BCHP_BSCA_IIC_ENABLE_INTRP_MASK)
                break;

            if (loopCnt == 0)
            {
                retCode = BERR_TRACE (BERR_TIMEOUT);
                goto done;
            }
            BKNI_Delay(I2C_POLLING_INTERVAL);
            loopCnt--;
        }
    }

    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));
    if (lval & BCHP_BSCA_IIC_ENABLE_NO_ACK_MASK)
    {
        if ( hChn->nvramAck == 0 )
        {
            BDBG_MSG(("BI2C_P_WaitForCompletion: BI2C_ERR_NO_ACK(0x110001) at line=%d", __LINE__));
            retCode = BI2C_ERR_NO_ACK;
        }
        else
        {
            /* If we are waiting for the NVM to acknowledge, then we DO NOT
             * want to trace an error message, however, we do need to
             * tell the caller that it has still not acked.
             */
            BDBG_MSG(("BI2C_P_WaitForCompletion: BI2C_ERR_NO_ACK(0x110001) at line=%d", __LINE__));
            retCode = BI2C_ERR_NO_ACK;
        }
    }

done:
    /* stop execution */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));
    lval &= ~(BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
    BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);


    return retCode;
}

static BERR_Code BI2C_P_WaitForDataToLeaveFIFO
(
    BI2C_ChannelHandle  hChn,                           /* Device channel handle */
    uint32_t                 numBytes                       /* number of bytes to transfer */
)
{
    BI2C_Handle hDev      = hChn->hI2c;
    BERR_Code   retCode   = BERR_SUCCESS;
    uint32_t        lval      = 0;
    uint32_t    loopCnt   = 0;

    BSTD_UNUSED(numBytes);

    loopCnt = ((hChn->timeoutMs * 1000) / I2C_POLLING_INTERVAL) + 1;

    while (1)
    {
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));

        if (lval & BCHP_BSCA_IIC_ENABLE_INTRP_MASK)
        {
         /* Ladies and gentleman, DATA HAS NOW LEFT THE FIFO!!!!
          * We can now finish
          */
            break;
        }

        if (loopCnt == 0)
        {
            retCode = BERR_TRACE (BERR_TIMEOUT);
            break;
        }

        BKNI_Delay(I2C_POLLING_INTERVAL);

        loopCnt--;
   }

   /* With respect to the NVM, we do not want to check if there has been
    * an ack to the data leaving the FIFO, because there probably wont be */

    /* stop execution */

    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));

    lval &= ~(BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);

    BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);

    return retCode;
}

/***********************************************************************func
 * Name: BI2C_P_ReadCmd
 *   - Read data from i2c slave device.
 *       write (1 byte of slave device register's address, subAddr)
 *   and read  (0-8 bytes of data)
 *   Using i2c Write.& Read
 *
 * NOTE: This function will be called by BI2C_P_Read, BI2C_P_ReadA16, BI2C_P_ReadNoAddr
 ***************************************************************************/
static BERR_Code BI2C_P_ReadCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t        chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t        subAddr,            /* sub address */
    uint8_t         numSubAddrBytes,    /* number of bytes in register address */
    uint8_t         *pData,             /* storage */
    size_t          numBytes,           /* number of bytes to read */
    bool            mutex,              /* protect with a mutex */
    bool            ack,                /* check for ack? */
    bool            noStop              /* no stop at the end? */
)
{
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint32_t            bscRegAddr, readCmdWord, bufferIndex, lval = 0, i;
    size_t              cnt;

    BDBG_MSG(("chipAddr = 0x%x, subAddr = %d, numSubAddrBytes = %d, pData = %p, ack = %d, noStop = %d",
                (unsigned) chipAddr, subAddr, numSubAddrBytes, (void *) pData, ack, noStop));

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);
    BDBG_ASSERT( pData );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        lval = (uint32_t)(chipAddr & 0xff);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN1;
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN0;
    }

    if (numSubAddrBytes)    /* read with sub addr, must issue a write */
    {
        /* program slave device's register address */
        if (numSubAddrBytes == 3)
        {
            lval = (subAddr >> 16) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 3;
        }
        else if (numSubAddrBytes == 2)
        {
            lval = (uint32_t)(subAddr >> 8);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = (uint32_t)(subAddr & 0xff);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 2;
        }
        else if (numSubAddrBytes == 1)
        {
            lval = (uint32_t)(subAddr & 0xff);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes++;
        }

        /* program amount of bytes to write/read */
        lval = (uint32_t)(numWriteBytes & 0x0f);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );

        /* program data transfer type */
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
        lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
        lval |= BI2C_P_MODE_eWriteOnly ;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

        if (!ack)
        {
            /* Mao Neil: ignore ack */
            /* printf("Disable the acknowledge!\n\n"); */
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
            lval |= 0x2;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
        }

        /* start the write command */
        lval = (BCHP_BSCA_IIC_ENABLE_RESTART_MASK | BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK |
                BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval );

        retCode = BI2C_P_WaitForCompletion(hChn, numWriteBytes);
        if (retCode != BERR_SUCCESS)
        {
            /* We will have to quit.  But if no ack, at least send the stop condition. */
            if (retCode == BI2C_ERR_NO_ACK)
            {
                #if 0
                /* Finish the transaction and then quit */
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 0 );
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                            (BCHP_BSCA_IIC_ENABLE_NO_START_MASK | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));
                rc = BI2C_P_WaitForCompletion(hChn, 1);
                if (rc != BERR_SUCCESS)
                    goto done;
                #endif
            }
            goto done;
        }
    }

    /*
     * Now perform the read portion
     */

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eReadOnly;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    readCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;

    while (cnt)
    {
        if (cnt <= MAX_I2C_READ_REQUEST)
        {
            /* program amount of bytes to read */
            lval = cnt;
            cnt = 0;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
            if (noStop) {
                readCmdWord |= (BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK | BCHP_BSCA_IIC_ENABLE_RESTART_MASK);
            }
        }
        else
        {
            /* program amount of bytes to read */
            lval = MAX_I2C_READ_REQUEST;
            cnt -= MAX_I2C_READ_REQUEST;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );

            readCmdWord |= BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK;

        }
        /* start the read command */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                    (readCmdWord | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));

        retCode = BI2C_P_WaitForCompletion(hChn, lval);
        if (retCode != BERR_SUCCESS)
        {
            #if 0
            /* We will have to quit.  But if no ack, at least send the stop condition. */
            if ((retCode == BI2C_ERR_NO_ACK) && (readCmdWord & BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK))
            {
                /* Finish the transaction and then quit */
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 0 );
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                            (BCHP_BSCA_IIC_ENABLE_NO_START_MASK | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));
                rc = BI2C_P_WaitForCompletion(hChn, 1);
                if (rc != BERR_SUCCESS)
                    goto done;
            }
            #endif
            goto done;
        }

        /* copy data to buffer */
        for(i = 0; i < lval; i++)
        {
            pData[bufferIndex++] = (uint8_t)(BREG_Read32 (hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_OUT0 + (i * 4))));
        }

        if (cnt)
        {
            readCmdWord = BCHP_BSCA_IIC_ENABLE_NO_START_MASK;
        }
    }

done:
    if (mutex)
        BI2C_P_RELEASE_MUTEX( hChn );

    BDBG_MSG(("BI2C_P_ReadCmd:  ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d SubAddr=0x%x, numBytes=%d, pData=0x%x",
        hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, numSubAddrBytes ? subAddr : 0, (unsigned) numBytes, *(uint8_t *)pData));
    #ifdef BI2C_DEBUG
    {
        int i;
        for (i=0; i<(int)numBytes; i++)
            BDBG_MSG(("%d:  0x%x", i, *(pData+i)));
    }
    #endif

    return retCode;
}

/***********************************************************************func
* Name: BI2C_P_WaitForNVMToAcknowledge
 *   -
 *
 *
 ***************************************************************************/
static BERR_Code BI2C_P_WaitForNVMToAcknowledge
(
    BI2C_ChannelHandle  hChn,                 /* Device channel handle */
    uint16_t                 chipAddr,            /* i2c chip address.  this is unshifted */
    uint32_t                 subAddr,       /* pointer to register address */
    uint8_t                  numSubAddrBytes,   /* number of bytes in register address */
    size_t                   numBytes            /* number of bytes to write */
   )
{
    BERR_Code   retCode   = BERR_SUCCESS;
   uint32_t     loopCnt   = 0;
   uint8_t     myData[1] = { 0 };
   BSTD_UNUSED(numBytes);
   /*
    * method
    * 1) Set up timout values as appropriate
    * 2) Make a call to Read, reading some abitrary address
    * 3) If Read fails, check timeout hasn't been reached.
    * 4) If not timeout, then re-call Read.
   */

   loopCnt = ((hChn->timeoutMs * 1000) / I2C_POLLING_INTERVAL) + 1;

   hChn->nvramAck = 1;

   while (1)
   {
      retCode = BI2C_P_ReadCmd( hChn, chipAddr, subAddr, numSubAddrBytes, &myData[0], 1, true /*mutex*/, true /*ack*/, false /*no stop*/);

      if ( retCode != BERR_SUCCESS )
      {
         BKNI_Delay( I2C_POLLING_INTERVAL );

         loopCnt--;

         if (loopCnt == 0)
         {
            retCode = BERR_TRACE( BERR_TIMEOUT );
            break;
         }
      }
      else
      {
         /* NVM chip responded so it is no longer in its internal
          * write cycle. Can now finish
          */
         break;
      }
   }

   hChn->nvramAck = 0;

   return retCode;
}

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
/***********************************************************************func
 * Name: BI2C_P_WriteBy4BytesCmd
 *   - Write data to an i2c slave device.
 *
 * NOTE: This function will be called by BI2C_P_Write, BI2C_P_WriteA16, BI2C_P_WriteNoAddr
 ***************************************************************************/
static BERR_Code BI2C_P_WriteBy4BytesCmd
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
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint8_t             maxWriteBytes;
    uint32_t            bscRegAddr, writeCmdWord, bufferIndex, lval = 0;
    size_t              cnt;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    BDBG_MSG(("BI2C_P_WriteBy4BytesCmd:  ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d SubAddr=0x%x, numBytes=%d, pData=0x%x",
        hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, subAddr, (unsigned) numBytes, *(uint8_t *)pData));
    #ifdef BI2C_DEBUG
    {
        int i;
        for (i=0; i<(int)numBytes; i++)
            BDBG_MSG(("%d:  0x%x", i, *(pData+i)));
    }
    #endif

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    maxWriteBytes = MAX_I2C_WRITE_REQUEST*4;

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
    }

    numWriteBytes += numSubAddrBytes;

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteOnly ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    writeCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;
    bscRegAddr = BCHP_BSCA_DATA_IN0;

    do
    {
        if (cnt+numWriteBytes <= maxWriteBytes)
        {
            lval = cnt+numWriteBytes;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
            if (noStop) {
                writeCmdWord |= (BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK | BCHP_BSCA_IIC_ENABLE_RESTART_MASK);
            }
        }
        else
        {
            lval = maxWriteBytes;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
            writeCmdWord |= BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK;
        }

        /* copy data from buffer */
        if (lval) /* lval may be zero because of a zero byte write */
        {
            uint32_t i;
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
                        if (chipAddr & 0x0380)              /* 10-bit chip address */
                        {
                            tempData = (uint32_t)(chipAddr & 0xff);
                        }
                        else
                        {
                            tempData = subAddr;
                        }
                        for (j=0; (j<3) && (j<cnt); j++)
                        {
                            tempData |= (uint32_t)((pData[bufferIndex+j]<<(8*(j+1))));
                        }
                        cnt-=j;
                        bufferIndex += 3;
                        break;
                    case 2:
                        if (chipAddr & 0x0380)              /* 10-bit chip address */
                        {
                            tempData = (uint32_t)(chipAddr & 0x000000ff);
                            tempData |= ((subAddr << 8)    & 0x0000ff00);
                        }
                        else
                        {
                            tempData = (subAddr >> 8)      & 0x000000ff;
                            tempData |= ((subAddr << 8)    & 0x0000ff00);
                        }
                        for (j=0; (j<2) && (j<cnt); j++)
                        {
                            tempData |= (uint32_t)((pData[bufferIndex+j]<<(8*(j+2))));
                        }
                        cnt-=j;
                        bufferIndex += 2;
                        break;
                    case 3:
                        if (chipAddr & 0x0380)              /* 10-bit chip address */
                        {
                            tempData = (uint32_t)(chipAddr & 0x000000ff);
                            tempData |= (subAddr)          & 0x0000ff00;
                            tempData |= (subAddr << 16)    & 0x00ff0000;
                        }
                        else
                        {
                            tempData = ((subAddr >> 16)    & 0x000000ff);
                            tempData |= (subAddr           & 0x0000ff00);
                            tempData |= (subAddr << 16)    & 0x00ff0000;
                        }
                        for (j=0; (j<1) && (j<cnt); j++)
                        {
                            tempData |= (uint32_t)((pData[bufferIndex+j]<<(8*(j+3))));
                        }
                        cnt-=j;
                        bufferIndex += 1;
                        break;
                    case 4:
                        if (chipAddr & 0x0380)              /* 10-bit chip address */
                        {
                            tempData = (uint32_t)(chipAddr & 0x000000ff);
                            tempData |= ((subAddr >> 8)    & 0x0000ff00);
                            tempData |= ((subAddr << 8)    & 0x00ff0000);
                            tempData |= ((subAddr << 24)   & 0xff000000);
                        }
                        break;
                    default:
                        break;
                }

                numWriteBytes=0; /* done taking care of sub address */
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr ), tempData);
                bscRegAddr += 4;
            }
        }

        if (!ack)
        {
            /* Mao Neil: ignore ack */
            /* printf("Disable the acknowledge!\n\n"); */
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
            lval |= BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
        }

        /* start the write command */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                    (writeCmdWord | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));

        if (isNvram)
        {
            retCode = BI2C_P_WaitForDataToLeaveFIFO(hChn, lval);

            if ( writeCmdWord & BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK )
            {
                /* There is a no stop */

            }
            else
            {
                /* If the master is sending the stop command, then
                 * this signals the nvm device to begin its internal write
                 * cylce. Now would be a good time to poll for its
                 * completion
                 */

                retCode = BI2C_P_WaitForNVMToAcknowledge( hChn,
                                                                chipAddr,
                                                                subAddr,
                                                                numSubAddrBytes,
                                                                numBytes );
             }

        }
        else
        {
            BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, lval));
        }

        if (retCode != BERR_SUCCESS)
            goto done;

        if (cnt)
        {
            writeCmdWord = BCHP_BSCA_IIC_ENABLE_NO_START_MASK;
            bscRegAddr = BCHP_BSCA_DATA_IN0;
        }
    }
    while (cnt>0);

done:
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
static BERR_Code BI2C_P_ReadBy4BytesCmd
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
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint8_t             maxReadBytes;
    uint8_t             numReadBytes;
    uint32_t            tempData;
    uint32_t            bscRegAddr, readCmdWord, bufferIndex, lval = 0, i;
    size_t              cnt;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    BDBG_MSG(("chipAddr = 0x%x, subAddr = %d, numSubAddrBytes = %d, pData = %p, ack = %d, noStop = %d",
                chipAddr, subAddr, numSubAddrBytes, (void *) pData, ack, noStop));

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    maxReadBytes = MAX_I2C_READ_REQUEST*4;

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        lval = (uint32_t)(chipAddr & 0xff);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), lval );

        /* There are still 3 bytes we can write in BCHP_BSCA_DATA_IN0 */
        bscRegAddr = BCHP_BSCA_DATA_IN0;
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN0;
    }

    if (numSubAddrBytes)    /* read with sub addr, must issue a write */
    {
        /* Each BCHP_BSCA_DATA_IN register has 4 bytes */
        if (numSubAddrBytes == 3)
        {
            if (numWriteBytes == 0)
            {
                lval = (subAddr & 0xff0000) >> 16;
                lval |= (subAddr & 0x00ff00);
                lval |= ((subAddr & 0x0000ff) << 16);
            }
            else
            {
                /* byte 0 contains part of chip address. */
                lval = (uint32_t)((chipAddr & 0xff) | ((subAddr & 0xff) << 8));
                lval |= ((subAddr & 0xff0000) >> 8);
                lval |= ((subAddr & 0x00ff00) << 8);
                lval |= ((subAddr & 0x0000ff) << 16);
            }
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 3;
        }
        else if (numSubAddrBytes == 2)
        {
            if (numWriteBytes == 0)
            {
                lval = ((subAddr & 0xff00) >> 8);
                lval |= ((subAddr & 0xff) << 8);
            }
            else
            {
                /* byte 0 contains part of chip address. */
                lval = (chipAddr & 0xff);
                lval |= ((subAddr & 0xff00) << 8);
                lval |= ((subAddr & 0x00ff) << 16);
            }
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            numWriteBytes += 2;
        }
        else if (numSubAddrBytes == 1)
        {
            if (numWriteBytes == 0)
            {
                lval = subAddr & 0xff;
            }
            else
            {
                /* byte 0 contains part of chip address. */
                lval = (chipAddr & 0xff);
                lval |= ((subAddr & 0xff) << 8);
            }
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            numWriteBytes++;
        }

        /* program amount of bytes to write/read */
        lval = (uint32_t)(numWriteBytes & 0x0f);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );

        /* program data transfer type */
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
        lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
        lval |= BI2C_P_MODE_eWriteOnly ;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

        if (!ack)
        {
            /* Mao Neil: ignore ack */
            /* printf("Disable the acknowledge!\n\n"); */
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
            lval |= BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
        }

        /* start the write command */
        lval = (BCHP_BSCA_IIC_ENABLE_RESTART_MASK | BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK |
                BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval );

        BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, numWriteBytes));
    }

    /*
     * Now perform the read portion
     */

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eReadOnly;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    readCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;

    while (cnt)
    {
        if (cnt <= maxReadBytes)
        {
            /* program amount of bytes to read */
            lval = cnt;
            cnt = 0;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
            if (noStop) {
                readCmdWord |= (BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK | BCHP_BSCA_IIC_ENABLE_RESTART_MASK);
            }
        }
        else
        {
            /* program amount of bytes to read */
            lval = maxReadBytes;
            cnt -= maxReadBytes;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
            readCmdWord |= BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK;
        }
        /* start the read command */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                    (readCmdWord | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));

        BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, lval));

        /* copy data to buffer */
        /* Read operation is by 4 bytes */
        for(i = 0; i < ((lval-1)/4)+1; i++)
        {
            tempData = BREG_Read32 (hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_OUT0 + (i * 4)));

            if (i == ((lval-1)/4))
            {
                numReadBytes = ((lval-1)%4) + 1; /* last read may have less than 4 bytes */
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

        if (cnt)
        {
            readCmdWord = BCHP_BSCA_IIC_ENABLE_NO_START_MASK;
        }
    }

done:
    if (mutex)
        BI2C_P_RELEASE_MUTEX( hChn );

#ifdef BI2C_DEBUG
    {
        int i;
        for (i=0; i<(int)numBytes; i++)
            BDBG_MSG(("%d:  0x%x", i, *(pData+i)));
    }
#endif

    return retCode;
}
#else
static BERR_Code BI2C_P_BurstReadCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t        chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t        subAddr,            /* pointer to register address */
    uint8_t         numSubAddrBytes,    /* number of bytes in register address */
    uint8_t         *pData,             /* storage */
    size_t          numBytes            /* number of bytes to read */
)
{
    BI2C_Handle     hDev;
    BERR_Code       retCode = BERR_SUCCESS;
    uint8_t         numWriteBytes = 0;
    uint32_t        bscRegAddr, readCmdWord, bufferIndex, lval = 0, rval, i;
    size_t          cnt;

    BSTD_UNUSED( pData );
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( numBytes <= MAX_I2C_READ_REQUEST );
    BDBG_ASSERT( pData );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    BI2C_P_ACQUIRE_MUTEX( hChn );

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        lval = (uint32_t)(chipAddr & 0xff);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN1;
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN0;
    }

    /* Reset i2c enable register */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), 0 );

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteRead ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    if (numSubAddrBytes)    /* read with sub addr, must fill subaddr to transmiting register */
    {
        /* program slave device's register address */
        if (numSubAddrBytes == 3)
        {
            lval = (subAddr >> 16) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 3;
        }
        else if (numSubAddrBytes == 2)
        {
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 2;
        }
        else if (numSubAddrBytes == 1)
        {
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes++;
        }

        /* program amount of subaddr bytes to conter register */
        lval = (uint32_t)(numWriteBytes & 0x0f);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
    }

    readCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;

    if (cnt <= MAX_I2C_READ_REQUEST)
    {
        /* program amount of bytes to read */
        lval = cnt;
        cnt = 0;
        rval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG));
        rval &= 0x0f;
        rval |= (lval << 4);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), rval );
    }

    /* start the burst read command */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);

    BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, lval+ (uint32_t)(numWriteBytes & 0x0f)));

    /* copy data to buffer */
    for(i = 0; i < lval; i++)
    {
        pData[bufferIndex++] = (uint8_t)(BREG_Read32 (hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_OUT0 + (i * 4))));
    }

done:
    BI2C_P_RELEASE_MUTEX( hChn);
    return retCode;
}

static BERR_Code BI2C_P_BurstReadCmdNoAck
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t        chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t        subAddr,            /* pointer to register address */
    uint8_t         numSubAddrBytes,    /* number of bytes in register address */
    uint8_t         *pData,             /* storage */
    size_t          numBytes            /* number of bytes to read */
)
{
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint32_t            bscRegAddr, bufferIndex, lval = 0, rval, i;
    size_t              cnt;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( numBytes <= MAX_I2C_READ_REQUEST );
    BDBG_ASSERT( pData );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    BI2C_P_ACQUIRE_MUTEX( hChn );

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        lval = (uint32_t)(chipAddr & 0xff);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN1;
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN0;
    }

    /* Reset i2c enable register */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), 0 );

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteRead ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    if (numSubAddrBytes)    /* read with sub addr, must issue a write */
    {
        /* program slave device's register address */
        if (numSubAddrBytes == 3)
        {
            lval = (subAddr >> 16) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 3;
        }
        else if (numSubAddrBytes == 2)
        {
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 2;
        }
        else if (numSubAddrBytes == 1)
        {
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes++;
        }

        /* program amount of subaddr bytes to conter register */
        lval = (uint32_t)(numWriteBytes & 0x0f);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );

        /* ignore ack */
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
        lval |= 0x2;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
    }

    /*
     * Now perform the read portion
     */

    cnt = numBytes;
    bufferIndex = 0;
    {
        if (cnt <= MAX_I2C_READ_REQUEST)
        {
            /* program amount of bytes to read */
            lval = cnt;
            cnt = 0;
            rval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG));
            rval &= 0x0f;
            rval |= (lval << 4);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), rval );
        }

        /* start the burst read command */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                    BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);

        BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, lval+ (uint32_t)(numWriteBytes & 0x0f)));

        /* copy data to buffer */
        for(i = 0; i < lval; i++)
        {
            pData[bufferIndex++] = (uint8_t)(BREG_Read32 (hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_OUT0 + (i * 4))));
        }
    }

done:
    BI2C_P_RELEASE_MUTEX( hChn );
    return retCode;
}

#endif

static bool BI2C_P_IsAutoI2cSupported
(
    void
)
{
#if AUTO_I2C_ENABLED
    return true;
#else
    return false;
#endif
}

static BERR_Code BI2C_P_Read
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*mutex*/, false /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmdNoAck ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*mutex*/, false /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadA16
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 2, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 2, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 2, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadA24
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 3, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 3, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 3, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadNoAddr
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadNoAddrNoAck
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

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true)
        return BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, true /*mutex*/, false /*ack*/, false /*no stop*/);
    else
#else
    if (hChn->chnSettings.burstMode)
        return BI2C_P_BurstReadCmdNoAck ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length);
    else
#endif
        return BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, true /*mutex*/, false /*ack*/, false /*no stop*/);
}

/***********************************************************************func
 * Name: BI2C_P_Write
 *   - Write data to an i2c slave device.
 *
 * NOTE: This function will be called by BI2C_P_Write, BI2C_P_WriteA16, BI2C_P_WriteNoAddr
 ***************************************************************************/
static BERR_Code BI2C_P_WriteCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t            chipAddr,       /* i2c chip address.  this is unshifted */
    uint32_t            subAddr,      /* pointer to register address */
    uint8_t             numSubAddrBytes,    /* number of bytes in register address */
    const uint8_t       *pData,     /* storage */
    size_t              numBytes,       /* number of bytes to write */
    bool                isNvram,         /* is this a nvram access? */
    bool                mutex,           /* protect with a mutex */
    bool                ack,             /* check for ack? */
    bool                noStop           /* Do we need a no stop at the end of the transfer? */
)
{
    BI2C_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS;
    uint8_t             numWriteBytes = 0;
    uint32_t            bscRegAddr, writeCmdWord, bufferIndex, maxTxfrSize, lval = 0, i;
    size_t              cnt;

    BDBG_MSG(("BI2C_P_WriteCmd:  ch=%d, clkRate=%d, chipAddr=0x%x, numSubAddrBytes=%d SubAddr=0x%x, numBytes=%d, pData=0x%x",
        hChn->chnNo, hChn->chnSettings.clkRate, chipAddr, numSubAddrBytes, subAddr, (unsigned) numBytes, *(uint8_t *)pData));
    #ifdef BI2C_DEBUG
    {
        int i;
        for (i=0; i<(int)numBytes; i++)
            BDBG_MSG(("%d:  0x%x", i, *(pData+i)));
    }
    #endif

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);
    BDBG_ASSERT( pData );

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    if (mutex)
        BI2C_P_ACQUIRE_MUTEX( hChn );

    /* program slave device's (id) */
    if (chipAddr & 0x0380)              /* 10-bit chip address */
    {
        lval = ((chipAddr & 0x0300) >> 7) | 0xF0;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        lval = (uint32_t)(chipAddr & 0xff);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN1;
        numWriteBytes++;
    }
    else
    {
        lval = (uint32_t) ((chipAddr & 0x7f) << 1);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), lval );
        bscRegAddr = BCHP_BSCA_DATA_IN0;
    }

    if (numSubAddrBytes)    /* read with sub addr, must issue a write */
    {
        /* program slave device's register address */
        if (numSubAddrBytes == 3)
        {
            lval = (subAddr >> 16) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 3;
        }
        else if (numSubAddrBytes == 2)
        {
            lval = (subAddr >> 8) & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes += 2;
        }
        else if (numSubAddrBytes == 1)
        {
            lval = subAddr & 0xff;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr), lval );
            bscRegAddr += 4;
            numWriteBytes++;
        }
    }

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteOnly ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    maxTxfrSize = MAX_I2C_WRITE_REQUEST - numWriteBytes;
    writeCmdWord = 0;
    cnt = numBytes;
    bufferIndex = 0;

    do
    {
        if (cnt <= maxTxfrSize)
        {
            lval = cnt;
            cnt = 0;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), (lval + numWriteBytes) );
            if (noStop) {
                writeCmdWord |= (BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK | BCHP_BSCA_IIC_ENABLE_RESTART_MASK);
            }
        }
        else
        {
            lval = maxTxfrSize;
            cnt -= maxTxfrSize;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), (lval + numWriteBytes) );

            writeCmdWord |= BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK;

        }

        /* copy data from buffer */
        for(i = 0; i < lval; i++)       /* lval is number of actual user data bytes to send */
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + bscRegAddr ), (uint32_t)(pData[bufferIndex++]));
            bscRegAddr += 4;
        }

        if (!ack)
        {
            /* Mao Neil: ignore ack */
            /* printf("Disable the acknowledge!\n\n"); */
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
            lval |= 0x2;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
        }

        /* start the write command */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                    (writeCmdWord | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));

        if (isNvram)
        {
            retCode = BI2C_P_WaitForDataToLeaveFIFO(hChn, (lval + numWriteBytes));

            if ( writeCmdWord & BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK )
            {
                /* There is a no stop */
            }
            else
            {
                /* If the master is sending the stop command, then
                 * this signals the nvm device to begin its internal write
                 * cylce. Now would be a good time to poll for its
                 * completion
                 */

                retCode = BI2C_P_WaitForNVMToAcknowledge( hChn,
                                                                chipAddr,
                                                                subAddr,
                                                                numSubAddrBytes,
                                                                numBytes );
            }
            if (retCode != BERR_SUCCESS)
                goto done;
        }
        else
        {
            retCode = BI2C_P_WaitForCompletion(hChn, (lval + numWriteBytes));
            if (retCode != BERR_SUCCESS)
            {
                #if 0
                /* We will have to quit.  But if no ack, at least send the stop condition. */
                if (((retCode == BI2C_ERR_NO_ACK) || (retCode == BERR_OS_ERROR)) && (writeCmdWord & BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK))
                {
                    /* Finish the transaction and then quit */
                    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 0 );
                    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                                (BCHP_BSCA_IIC_ENABLE_NO_START_MASK | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));
                    rc = BI2C_P_WaitForCompletion(hChn, 1);
                    if (rc != BERR_SUCCESS)
                        goto done;
                }
                #else
                if (retCode == BERR_OS_ERROR)
                {
                    /* Finish the transaction and then quit */
                    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 0 );
                    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE),
                                (BCHP_BSCA_IIC_ENABLE_NO_START_MASK | BCHP_BSCA_IIC_ENABLE_ENABLE_MASK));
                    BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, 1));
                }
                #endif
                goto done;
            }
        }

        if (cnt)
        {
            writeCmdWord = BCHP_BSCA_IIC_ENABLE_NO_START_MASK;
            bscRegAddr = BCHP_BSCA_DATA_IN0;
            maxTxfrSize = MAX_I2C_WRITE_REQUEST;
            numWriteBytes  = 0;
        }
    }
    while (cnt);

done:
    if (mutex)
        BI2C_P_RELEASE_MUTEX( hChn );
    return retCode;
}

static BERR_Code BI2C_P_WriteRead
(
    void * context,
    uint16_t chipAddr,
    uint32_t subAddr,
    const uint8_t *pWriteData,
    size_t writeLength,
    uint8_t *pReadData,
    size_t readLength
)
{
    BERR_Code err;

    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;           /* Device channel handle */

    BI2C_P_ACQUIRE_MUTEX( hChn );

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
    {
        err = BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pWriteData, writeLength, false /*NVRAM*/, false /*mutex*/, true /*ack*/, true /*no stop*/);
        if (err == BERR_SUCCESS)
            err = BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pReadData, readLength, false /*mutex*/, true /*ack*/, false /*no stop*/);
        else
            goto done;
    }
    else
#endif
    {
        err = BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pWriteData, writeLength, false /*NVRAM*/, false /*mutex*/, true /*ack*/, true /*no stop*/);
        if (err == BERR_SUCCESS)
            err = BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pReadData, readLength, false /*mutex*/, true /*ack*/, false /*no stop*/);
        else
            goto done;
    }

done:
    BI2C_P_RELEASE_MUTEX( hChn );
    return err;
}

static BERR_Code BI2C_P_WriteReadNoAddr
(
    void * context,
    uint16_t chipAddr,
    const uint8_t *pWriteData,
    size_t writeLength,
    uint8_t *pReadData,
    size_t readLength
)
{
    BERR_Code err;

    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;           /* Device channel handle */


    BI2C_P_ACQUIRE_MUTEX( hChn );

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
    {
        err = BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pWriteData, writeLength, false /*NVRAM*/, false /*mutex*/, true /*ack*/, true /*no stop*/);
        if (err == BERR_SUCCESS)
            err =  BI2C_P_ReadBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pReadData, readLength, false /*mutex*/, true /*ack*/, false /*no stop*/);
        else
            goto done;
    }
    else
#endif
    {
        err = BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pWriteData, writeLength, false /*NVRAM*/, false /*mutex*/, true /*ack*/, true /*no stop*/);
        if (err == BERR_SUCCESS)
            err = BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pReadData, readLength, false /*mutex*/, true /*ack*/, false /*no stop*/);
        else
            goto done;
    }

done:
    BI2C_P_RELEASE_MUTEX( hChn );
    return err;
}

static BERR_Code BI2C_P_Write
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, true /*mutex*/, false /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, true /*mutex*/, false /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteA16
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 16-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 2, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 2, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteA24
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 24-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 3, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 3, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, false /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, false /*NVRAM*/, true /*mutex*/, false /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteNoAddrNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, false /*NVRAM*/, true /*mutex*/, false /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, 0, 0, pData, length, false /*NVRAM*/, true /*mutex*/, false /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_WriteNvram
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
)
{
#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        return BI2C_P_WriteBy4BytesCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
        return BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, subAddr, 1, pData, length, true /*NVRAM*/, true /*mutex*/, true /*ack*/, false /*no stop*/);
}

static BERR_Code BI2C_P_ReadEDDC(
    void                *context,               /* Device channel handle */
    uint8_t             chipAddr,               /* chip address */
    uint8_t             segment,                /* EDDC segment */
    uint32_t            subAddr,                /* 8-bit sub address */
    uint8_t             *pData,                 /* pointer to memory location to store read data */
    size_t              length                  /* number of bytes to read */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BI2C_Handle         hDev;
    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;
    uint32_t            lval;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    BI2C_P_ACQUIRE_MUTEX( hChn );

    /***********************************
     * Step 1: Write the segment pointer
     **********************************/
    /* program slave device's (id) */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), (uint32_t)EDDC_SEGMENT_CHIP_ADDR );

    /* write segment pointer into fifo */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), (uint32_t)segment );

    /* program amount of bytes to write/read */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 1 );

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteOnly ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    /* ignore ack */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
    lval |= BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );

    /* start the write command */
    lval = (BCHP_BSCA_IIC_ENABLE_RESTART_MASK | BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK |
            BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval );

    BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, 1));

    /* get rid of ignore ack */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
    lval &= ~BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );

    /****************************************
     * Step 2: Read the data with sub address
     ***************************************/

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode(hChn) == true){
        retCode = BI2C_P_ReadBy4BytesCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*mutex*/, true /*ack*/, false /*no stop*/);
    }
    else
#endif
        retCode = BI2C_P_ReadCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*mutex*/, true /*ack*/, false /*no stop*/);

done:
    BI2C_P_RELEASE_MUTEX( hChn );
    return( retCode );
}

static BERR_Code BI2C_P_WriteEDDC(
    void                *context,       /* Device channel handle */
    uint8_t             chipAddr,       /* chip address */
    uint8_t             segment,        /* EDDC segment */
    uint32_t             subAddr,        /* 8-bit sub address */
    const uint8_t       *pData,         /* pointer to data to write */
    size_t              length          /* number of bytes to read */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BI2C_Handle         hDev;
    BI2C_ChannelHandle  hChn = (BI2C_ChannelHandle)context;
    uint32_t            lval;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    /* Get I2C handle from channel handle */
    hDev = hChn->hI2c;

    BI2C_P_ACQUIRE_MUTEX( hChn );

    /***********************************
     * Step 1: Write the segment pointer
     **********************************/
    /* program slave device's (id) */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CHIP_ADDRESS), (uint32_t)EDDC_SEGMENT_CHIP_ADDR );

    /* write segment pointer into fifo */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_DATA_IN0), (uint32_t)segment );

    /* program amount of bytes to write/read */
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 1 );

    /* program data transfer type */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    lval &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_DTF_MASK);
    lval |= BI2C_P_MODE_eWriteOnly ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

    /* ignore ack */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
    lval |= BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );

    /* start the write command */
    lval = (BCHP_BSCA_IIC_ENABLE_RESTART_MASK | BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK |
            BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval );

    BI2C_CHK_RETCODE( retCode, BI2C_P_WaitForCompletion(hChn, 1));

    /* get rid of ignore ack */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
    lval &= ~BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK ;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );

    /****************************************
     * Step 2: Write the data with sub address
     ***************************************/

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
    if (BI2C_Is4ByteXfrMode((BI2C_ChannelHandle)context) == true)
        retCode = BI2C_P_WriteBy4BytesCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, false /*mutex*/, true /*ack*/, false /*no stop*/);
    else
#endif
    retCode = BI2C_P_WriteCmd (hChn, chipAddr, subAddr, 1, pData, length, false /*NVRAM*/, false /*mutex*/, true /*ack*/, false /*no stop*/);

done:
    BI2C_P_RELEASE_MUTEX( hChn );
    return( retCode );
}

void BI2C_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BI2C_ChannelHandle  hChn;
    BI2C_Handle         hDev;
    uint32_t            lval;

    hChn = (BI2C_ChannelHandle) pParam1;
    BDBG_ASSERT( hChn );

    BSTD_UNUSED(parm2);

    hDev = hChn->hI2c;
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));

    hChn->noAck = (lval & BCHP_BSCA_IIC_ENABLE_NO_ACK_MASK) ? true : false; /* save no ack status */
    lval &= ~(BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
    BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);

    BKNI_SetEvent( hChn->hChnEvent );

    return;
}

#ifdef BI2C_USES_SETUP_HDMI_HW_ACCESS

static BERR_Code BI2C_P_SetupHdmiHwAccess(
    void *context,                      /* Device channel handle */
    uint32_t dataTransferFormat,        /* Data Transfer Format */
    uint32_t cnt1,                      /* Counter 1 value */
    uint32_t cnt2                       /* Counter 2 value */
)
{
    BI2C_ChannelHandle hChn;
    BI2C_Handle hDev;
    uint32_t lval;
    BERR_Code rc = BERR_SUCCESS;

    if (context == NULL)
    {
        BDBG_ERR(("Invalid I2C handle."));
        rc = BERR_INVALID_PARAMETER;
        BSTD_UNUSED(dataTransferFormat);
        BSTD_UNUSED(cnt1);
        BSTD_UNUSED(cnt1);
    }
    else
    {
        hChn = (BI2C_ChannelHandle)context;
        BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

        /* Get I2C handle from channel handle */
        hDev = hChn->hI2c;

        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
        lval &= ~BCHP_BSCA_CTL_REG_DTF_MASK;
        lval |= dataTransferFormat << BCHP_BSCA_CTL_REG_DTF_SHIFT;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG));
        lval &= ~(BCHP_BSCA_CNT_REG_CNT_REG1_MASK | BCHP_BSCA_CNT_REG_CNT_REG2_MASK);
        lval |= cnt1 << BCHP_BSCA_CNT_REG_CNT_REG1_SHIFT;
        lval |= cnt2 << BCHP_BSCA_CNT_REG_CNT_REG2_SHIFT;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), lval );
    }

    return rc;
}

#else

static BERR_Code BI2C_P_SetupHdmiHwAccess(
    void *context,                      /* Device channel handle */
    uint32_t dataTransferFormat,        /* Data Transfer Format */
    uint32_t cnt1,                      /* Counter 1 value */
    uint32_t cnt2                       /* Counter 2 value */
    )
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(dataTransferFormat);
    BSTD_UNUSED(cnt1);
    BSTD_UNUSED(cnt2);

    BDBG_ERR(("%s: Not Supported", __FUNCTION__));
    return BERR_NOT_SUPPORTED;
}

#endif

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BI2C_Open(
    BI2C_Handle *pI2c,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BI2C_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BI2C_Handle hDev = NULL;

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );
    BSTD_UNUSED( pDefSettings );

    /* Alloc memory from the system heap */
    hDev = (BI2C_Handle) BKNI_Malloc( sizeof( BI2C_P_Handle ) );
    if( hDev == NULL )
    {
        *pI2c = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BI2C_Open: BKNI_malloc() failed."));
        goto done;
    }

    BKNI_Memset((void*)hDev, 0x0, sizeof(BI2C_P_Handle));
    BDBG_OBJECT_SET(hDev, BI2C_P_Handle);

    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->maxChnNo  = BI2C_MAX_I2C_CHANNELS;

    /* init channel handle list */
    BLST_S_INIT(&hDev->chnHandleHead);

    *pI2c = hDev;

done:
    return( retCode );
}

BERR_Code BI2C_Close(
    BI2C_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT(hDev, BI2C_P_Handle);

    BDBG_OBJECT_DESTROY(hDev, BI2C_P_Handle);
    BKNI_Free( (void *) hDev );

    return( retCode );
}

BERR_Code BI2C_GetDefaultSettings(
    BI2C_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED( hChip );

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    return( retCode );
}

BERR_Code BI2C_GetTotalChannels(
    BI2C_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT(hDev, BI2C_P_Handle);

    *totalChannels = hDev->maxChnNo;

    return( retCode );
}

static BERR_Code BI2C_P_BscIndexToChannelNum(
    BI2C_Handle hDev,                   /* Device handle */
    unsigned bscIndex,
    unsigned *channelNo
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hDev);

    switch (bscIndex)
    {
#if BSCA_AVAILABLE
        case 0:
            break;
#endif
#if BSCB_AVAILABLE
        case 1:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
#if BSCC_AVAILABLE
        case 2:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
#if BSCB_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
#if BSCD_AVAILABLE
        case 3:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
#if BSCB_AVAILABLE
            *channelNo += 1;
#endif
#if BSCC_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
#if BSCE_AVAILABLE
        case 4:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
#if BSCB_AVAILABLE
            *channelNo += 1;
#endif
#if BSCC_AVAILABLE
            *channelNo += 1;
#endif
#if BSCD_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
#if BSCF_AVAILABLE
        case 5:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
#if BSCB_AVAILABLE
            *channelNo += 1;
#endif
#if BSCC_AVAILABLE
            *channelNo += 1;
#endif
#if BSCD_AVAILABLE
            *channelNo += 1;
#endif
#if BSCE_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
#if BSCG_AVAILABLE
        case 6:
#if BSCA_AVAILABLE
            *channelNo += 1;
#endif
#if BSCB_AVAILABLE
            *channelNo += 1;
#endif
#if BSCC_AVAILABLE
            *channelNo += 1;
#endif
#if BSCD_AVAILABLE
            *channelNo += 1;
#endif
#if BSCE_AVAILABLE
            *channelNo += 1;
#endif
#if BSCF_AVAILABLE
            *channelNo += 1;
#endif
            break;
#endif
        default:
            BDBG_MSG(("BSC Index %d is not supported on this platoform.", bscIndex));
            retCode = BERR_NOT_AVAILABLE;
            goto done;
    }

done:
    return retCode;
}


BERR_Code BI2C_GetChannelDefaultSettings(
    BI2C_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BI2C_ChannelSettings *pChnSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hDev);
    BKNI_Memset(pChnSettings, 0, sizeof(*pChnSettings));

    if( channelNo < hDev->maxChnNo )
    {
        pChnSettings->softI2c = false;

        pChnSettings->clkRate = BI2C_Clk_eClk400Khz;
        pChnSettings->intMode = true;
        pChnSettings->timeoutMs = 0;
        pChnSettings->burstMode = false;
        pChnSettings->autoI2c.enabled = false;
        pChnSettings->fourByteXferMode = true;
        pChnSettings->inputSwitching5V = false;
    }
    else if(channelNo == BI2C_SOFT_I2C_CHANNEL_NUMBER){
        pChnSettings->softI2c = true;

        pChnSettings->clkRate = BI2C_Clk_eClk400Khz;
        pChnSettings->intMode = true;
        pChnSettings->timeoutMs = 0;
        pChnSettings->burstMode = false;
        pChnSettings->autoI2c.enabled = false;
        pChnSettings->fourByteXferMode = false;
        pChnSettings->inputSwitching5V = false;
        pChnSettings->gpio.scl.address = BCHP_GIO_ODEN_LO;
        pChnSettings->gpio.scl.shift = 0;
        pChnSettings->gpio.sda.address = BCHP_GIO_ODEN_LO;
        pChnSettings->gpio.sda.shift = 0;
    }
    else{
        if( channelNo >= hDev->maxChnNo )
            BDBG_ERR(("BSC Index %d is not supported on this platoform.", channelNo));
        else
            BDBG_ERR(("For soft I2C, Channel Number %d is not supported. Set channelNo to BI2C_SOFT_I2C_CHANNEL_NUMBER.", channelNo));
        retCode = BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    return( retCode );
}


BERR_Code BI2C_GetBscIndexDefaultSettings(
    BI2C_Handle hDev,                     /* Device handle */
    unsigned int bscIndex,                /* Channel number to default setting for */
    bool isSoftI2c,                       /* If true, get the defaults for soft i2c channel. */
    BI2C_ChannelSettings *pChnSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    unsigned channelNo=0;

    if(!isSoftI2c){
        retCode = BI2C_P_BscIndexToChannelNum(hDev, bscIndex, &channelNo);
        if( retCode != BERR_SUCCESS )
            goto done;

        BSTD_UNUSED(hDev);

        if( channelNo < hDev->maxChnNo )
        {
            BI2C_GetChannelDefaultSettings(hDev, channelNo, pChnSettings);
        }
        else
        {
            BDBG_WRN(("BI2C_GetBscIndexDefaultSettings: Channel No %d not supported on this platform", channelNo));
            retCode = BERR_NOT_AVAILABLE;
        }
    }
    else if(isSoftI2c && (bscIndex == BI2C_SOFT_I2C_CHANNEL_NUMBER)){
        BI2C_GetChannelDefaultSettings(hDev, BI2C_SOFT_I2C_CHANNEL_NUMBER, pChnSettings);
    }
    else {
        BDBG_ERR(("For soft I2C, Channel Number %d is not supported. Set channelNo to BI2C_SOFT_I2C_CHANNEL_NUMBER.", channelNo));
        retCode = BERR_TRACE(BERR_NOT_AVAILABLE);
    }

done:
    return retCode ;
}

BERR_Code BI2C_OpenBSCChannel(
    BI2C_Handle hDev,                   /* Device handle */
    BI2C_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int bscIndex,             /* Channel number to open */
    const BI2C_ChannelSettings *pChnSettings /* Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned channelNo=0;

    if(pChnSettings->softI2c){
        if(bscIndex == BI2C_SOFT_I2C_CHANNEL_NUMBER){
            retCode = BI2C_OpenChannel(hDev, phChn, bscIndex, pChnSettings);
        }
        else{
            BDBG_ERR(("For soft I2C, Channel Number %d is not supported. Set channelNo to BI2C_SOFT_I2C_CHANNEL_NUMBER.", channelNo));
            retCode = BERR_TRACE(BERR_NOT_AVAILABLE);
        }
    }
    else{
        retCode = BI2C_P_BscIndexToChannelNum(hDev, bscIndex, &channelNo);
        if( retCode != BERR_SUCCESS )
            goto done;

        retCode = BI2C_OpenChannel(hDev, phChn, channelNo, pChnSettings);
    }

done:
    return retCode;

}

BI2C_ChannelHandle BI2C_P_GetChannelHandle(
    BI2C_Handle  hDev,
    unsigned int channelNo,             /* Channel number to open */
    const BI2C_ChannelSettings *pChnSettings)
{
    BI2C_P_ChannelHandle *  phChn;

    phChn = BLST_S_FIRST(&hDev->chnHandleHead);
    while ( NULL != phChn )
    {
        if(pChnSettings->softI2c){
            if((phChn->chnSettings.softI2c) && !BKNI_Memcmp(&phChn->chnSettings.gpio.scl, &pChnSettings->gpio.scl, sizeof(BI2C_gpio_params)) &&
                !BKNI_Memcmp(&phChn->chnSettings.gpio.sda, &pChnSettings->gpio.sda, sizeof(BI2C_gpio_params))){
                return phChn;
            }
        }
        else if(phChn->chnNo  == channelNo) {
            if(!BKNI_Memcmp(&phChn->chnSettings, pChnSettings, sizeof(BI2C_ChannelSettings))){
                return phChn;
            }
        }
        phChn = BLST_S_NEXT(phChn, link);
    }

    /* not found */
    return NULL;
}

#ifdef BCHP_PWR_RESOURCE_HDMI_TX0_CLK
#define BCHP_PWR_RESOURCE_HDMI_TX_CLK BCHP_PWR_RESOURCE_HDMI_TX0_CLK
#endif

BERR_Code BI2C_OpenChannel(
    BI2C_Handle hDev,                   /* Device handle */
    BI2C_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BI2C_ChannelSettings *pChnSettings /* Channel default setting */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BI2C_ChannelHandle  hChn=NULL;
    uint32_t            lval, sleepCount=0;
    unsigned            offset=0;

    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT(hDev, BI2C_P_Handle);


    if((!pChnSettings->softI2c) && ( channelNo >= hDev->maxChnNo ))
    {
        retCode = BERR_TRACE(BERR_NOT_AVAILABLE);
        BDBG_ERR(("Channel number %d exceeds available channels.", channelNo));
        goto done;

    }
    else if(pChnSettings->softI2c && (channelNo != BI2C_SOFT_I2C_CHANNEL_NUMBER)){
        BDBG_ERR(("For soft I2C, Channel Number %d is not supported. Set channelNo to BI2C_SOFT_I2C_CHANNEL_NUMBER.", channelNo));
        retCode = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto done;
    }
    else {
        hChn = BI2C_P_GetChannelHandle(hDev, channelNo, pChnSettings);
        if( hChn != NULL ){
            retCode = BERR_TRACE (BI2C_ERR_NOTAVAIL_CHN_NO);
            BDBG_ERR(("Channel number %d is already opened.", channelNo));
            goto done;
        }
    }

    /* Alloc memory from the system heap */
    hChn = (BI2C_ChannelHandle) BKNI_Malloc( sizeof( BI2C_P_ChannelHandle ) );
    if( hChn == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BI2C_OpenChannel: BKNI_malloc() failed."));
        goto done;
    }

    BKNI_Memset((void*)hChn, 0x0, sizeof(BI2C_P_ChannelHandle));
    BDBG_OBJECT_SET(hChn, BI2C_P_ChannelHandle);

    BLST_S_INSERT_HEAD(&hDev->chnHandleHead , hChn, link);

    hChn->chnSettings = *pChnSettings;
    hChn->timeoutMs = hChn->chnSettings.timeoutMs ? hChn->chnSettings.timeoutMs : BI2C_P_MAX_TIMEOUT_MS;

    retCode = BKNI_CreateMutex( &hChn->hMutex );
    if( retCode != BERR_SUCCESS )
        goto done;

    BI2C_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChn->hChnEvent) ) );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);
    hChn->hI2c = hDev;
    hChn->nvramAck = 0;
    hChn->chnNo = channelNo;

    /* Enable/Disable auto i2c for this channel */
    if(pChnSettings->autoI2c.enabled){
        if(!BI2C_P_IsAutoI2cSupported()){
            BDBG_ERR(("Auto-I2c is not supported for this platform."));
            goto done;
        }
    }

    hChn->hChnCallback = NULL;

    if(!pChnSettings->softI2c){
        if(pChnSettings->autoI2c.enabled){
#if AUTO_I2C_ENABLED
            hChn->autoI2c.channelNumber = BAUTO_I2C_BSC_CHANNEL;
            switch (hChn->autoI2c.channelNumber)
            {
                case 0:
                    hChn->autoI2c.baseAddress = BAUTO_I2C_P_BASE_ADDRESS_eCH0;
                    hChn->autoI2c.reg0BaseAddress = BAUTO_I2C_P_REG0_ADDRESS_eCH0;
                    break;
                case 1:
                    hChn->autoI2c.baseAddress = BAUTO_I2C_P_BASE_ADDRESS_eCH1;
                    hChn->autoI2c.reg0BaseAddress = BAUTO_I2C_P_REG0_ADDRESS_eCH1;
                    break;
                case 2:
                    hChn->autoI2c.baseAddress = BAUTO_I2C_P_BASE_ADDRESS_eCH2;
                    hChn->autoI2c.reg0BaseAddress = BAUTO_I2C_P_REG0_ADDRESS_eCH2;
                    break;
                case 3:
                    hChn->autoI2c.baseAddress = BAUTO_I2C_P_BASE_ADDRESS_eCH3;
                    hChn->autoI2c.reg0BaseAddress = BAUTO_I2C_P_REG0_ADDRESS_eCH3;
                    break;
            }
            hChn->autoI2c.currentRegOffset = BAUTO_I2C_P_CHX_REG_OFFSET_e0;

            BAUTO_I2C_P_GetDefaultTriggerConfig(&hChn->autoI2c.trigConfig);
            if (hChn->chnSettings.intMode == true)
            {
                /* Register and enable L2 interrupt. */
                if (hChn->hChnCallback == NULL)
                {
#if BCHP_PWR_RESOURCE_HDMI_TX_CLK
                    BCHP_PWR_AcquireResource(hDev->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
                    BI2C_CHK_RETCODE( retCode,
                        BINT_CreateCallback(&(hChn->hChnCallback), hDev->hInterrupt, BCHP_INT_ID_I2C_CH2_DONE_INTR, BAUTO_I2C_P_HandleInterrupt_Isr, (void *) hChn, 0x00)
                    );

                    /* clear interrupt callback */
                    BI2C_CHK_RETCODE(retCode, BINT_ClearCallback(hChn->hChnCallback));
                    BI2C_CHK_RETCODE( retCode, BINT_EnableCallback(hChn->hChnCallback));
                }
            }
#endif
        }
        else {
            /* Offsets are based off of BSCA */
            hChn->coreOffset = coreOffsetVal[channelNo];

            /* Program I2C clock rate */
            BI2C_SetClk (hChn, pChnSettings->clkRate);

#ifdef BI2C_USES_SET_SDA_DELAY
            BI2C_SetSdaDelay(hChn, BI2C_eSdaDelay815ns);
#endif

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
            if (pChnSettings->fourByteXferMode)
                BI2C_Set4ByteXfrMode(hChn, true);
            else
                BI2C_Set4ByteXfrMode(hChn, false);
#endif

            /* Reset the i2c channel/controller if the previous transaction was incomplete. */
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE));
            if (lval){
                while(lval & BCHP_BSCA_IIC_ENABLE_ENABLE_MASK)
                {
                    if (lval & BCHP_BSCA_IIC_ENABLE_INTRP_MASK)
                        break;
                    else {
                        BKNI_Sleep(I2C_RESET_INTERVAL);
                        if(++sleepCount >= I2C_MAX_RESET_COUNT) {
                            BDBG_WRN(("Unable to complete previous transaction for I2C channel %d", channelNo));
                            break;
                        }
                    }
                };

                lval = 0;
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);

                lval = (BCHP_BSCA_IIC_ENABLE_NO_ACK_MASK | BCHP_BSCA_IIC_ENABLE_NO_START_MASK);
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CNT_REG), 1 );

                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), BCHP_BSCA_IIC_ENABLE_ENABLE_MASK);
                BKNI_Delay(I2C_RESET_INTERVAL);
                lval = 0;
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_IIC_ENABLE), lval);
            }

            if (hChn->chnSettings.intMode == true)
            {
                BI2C_CHK_RETCODE( retCode,
                    BINT_CreateCallback(&(hChn->hChnCallback), hDev->hInterrupt, IntId[channelNo], BI2C_P_HandleInterrupt_Isr, (void *) hChn, 0x00 )
                );
                BI2C_CHK_RETCODE( retCode, BINT_EnableCallback( hChn->hChnCallback ) );

                /* Enable I2C interrupt in I2C Core*/

                lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
                lval |= BCHP_BSCA_CTL_REG_INT_EN_MASK;
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), lval );

            }
#ifdef BCHP_BSCA_CTLHI_REG_INPUT_SWITCHING_LEVEL_MASK
            /* Reset the CTLHI register and set the INPUT_SWITCHING flag accordingly */
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG) );
            if (pChnSettings->inputSwitching5V)
                lval |= BCHP_BSCA_CTLHI_REG_INPUT_SWITCHING_LEVEL_MASK;
            else
                lval &= ~BCHP_BSCA_CTLHI_REG_INPUT_SWITCHING_LEVEL_MASK;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), lval );
#endif
        }
    }
    else{
        offset = pChnSettings->gpio.scl.address - BCHP_GIO_ODEN_LO;
        hChn->softI2cBus.scl.mask = (1<<pChnSettings->gpio.scl.shift);
        hChn->softI2cBus.scl.mask = ~hChn->softI2cBus.scl.mask;
        hChn->softI2cBus.scl.data = BCHP_GIO_DATA_LO+offset;
        hChn->softI2cBus.scl.iodir = BCHP_GIO_IODIR_LO+offset;
        hChn->softI2cBus.scl.oden = BCHP_GIO_ODEN_LO+offset;

        offset = pChnSettings->gpio.sda.address - BCHP_GIO_ODEN_LO;
        hChn->softI2cBus.sda.mask = (1<<pChnSettings->gpio.sda.shift);
        hChn->softI2cBus.sda.mask = ~hChn->softI2cBus.sda.mask;
        hChn->softI2cBus.sda.data = BCHP_GIO_DATA_LO+offset;
        hChn->softI2cBus.sda.iodir = BCHP_GIO_IODIR_LO+offset;
        hChn->softI2cBus.sda.oden = BCHP_GIO_ODEN_LO+offset;
    }
    *phChn = hChn;

done:
    if( retCode != BERR_SUCCESS ) {
        if( hChn != NULL ) {
            if( hChn->hChnEvent != NULL )
            {
                BKNI_DestroyEvent( hChn->hChnEvent );
            }

            BLST_S_REMOVE(&hDev->chnHandleHead, hChn, BI2C_P_ChannelHandle, link);

            BDBG_OBJECT_DESTROY(hChn, BI2C_P_ChannelHandle);
            BKNI_Free(hChn);
            *phChn = NULL;
        }
    }
    return( retCode );
}

BERR_Code BI2C_CloseChannel(
    BI2C_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BI2C_Handle hDev;
    uint32_t    ctlReg;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    hDev = hChn->hI2c;

    if (hChn->hChnCallback != NULL)
    {
        BI2C_CHK_RETCODE( retCode, BINT_DisableCallback( hChn->hChnCallback ) );
        BI2C_CHK_RETCODE( retCode, BINT_DestroyCallback( hChn->hChnCallback ) );
        hChn->hChnCallback = NULL;
    }

    if(hChn->chnSettings.autoI2c.enabled){
#if BCHP_PWR_RESOURCE_HDMI_TX_CLK
        BCHP_PWR_ReleaseResource(hDev->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
    }
    else {
/* FIX THIS EVERYWHERE */
#if B_REFSW_NUM_GPIO_SOFT_I2C
        if(hChn->chnNo < (BI2C_MAX_I2C_CHANNELS - B_REFSW_NUM_GPIO_SOFT_I2C)){
#endif
            /* Disable interrupt for this channel */
            BKNI_EnterCriticalSection();
            ctlReg = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
            ctlReg &= ~BCHP_BSCA_CTL_REG_INT_EN_MASK;
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), ctlReg );
            BKNI_LeaveCriticalSection();
#if B_REFSW_NUM_GPIO_SOFT_I2C
        }
#endif
    }

    if( hChn->hChnEvent != NULL )
    {
        BKNI_DestroyEvent( hChn->hChnEvent );
    }

    BKNI_DestroyMutex(hChn->hMutex);

    BLST_S_REMOVE(&hDev->chnHandleHead, hChn, BI2C_P_ChannelHandle, link);

    BDBG_OBJECT_DESTROY(hChn, BI2C_P_ChannelHandle);
    BKNI_Free(hChn);

done:
    return( retCode );
}

BERR_Code BI2C_GetDevice(
    BI2C_ChannelHandle hChn,            /* Device channel handle */
    BI2C_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    *phDev = hChn->hI2c;

    return( retCode );
}

BERR_Code BI2C_CreateI2cRegHandle(
    BI2C_ChannelHandle hChn,            /* Device channel handle */
    BREG_I2C_Handle *pI2cReg            /* [output]  */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    *pI2cReg = (BREG_I2C_Handle)BKNI_Malloc( sizeof(BREG_I2C_Impl) );
    if( *pI2cReg == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BI2C_CreateI2cRegHandle: BKNI_malloc() failed"));
        goto done;
    }

    BKNI_Memset(*pI2cReg, 0, sizeof(BREG_I2C_Impl));

    if (hChn->chnSettings.softI2c)
    {
        (*pI2cReg)->context                     = (void *)hChn;
        (*pI2cReg)->BREG_I2C_WriteSw_Func       = BI2C_P_WriteSw;
        (*pI2cReg)->BREG_I2C_Write_Func         = BI2C_P_WriteSw;
        (*pI2cReg)->BREG_I2C_WriteNoAddr_Func   = BI2C_P_WriteSwNoAddr;
        (*pI2cReg)->BREG_I2C_WriteSwNoAddr_Func = BI2C_P_WriteSwNoAddr;
        (*pI2cReg)->BREG_I2C_WriteSwA16_Func    = BI2C_P_WriteSwA16;
        (*pI2cReg)->BREG_I2C_WriteA16_Func      = BI2C_P_WriteSwA16;
        (*pI2cReg)->BREG_I2C_WriteSwA24_Func    = BI2C_P_WriteSwA24;
        (*pI2cReg)->BREG_I2C_WriteA24_Func      = BI2C_P_WriteSwA24;
        (*pI2cReg)->BREG_I2C_ReadSw_Func        = BI2C_P_ReadSw;
        (*pI2cReg)->BREG_I2C_Read_Func          = BI2C_P_ReadSw;
        (*pI2cReg)->BREG_I2C_ReadSwA16_Func     = BI2C_P_ReadSwA16;
        (*pI2cReg)->BREG_I2C_ReadA16_Func       = BI2C_P_ReadSwA16;
        (*pI2cReg)->BREG_I2C_ReadSwA24_Func     = BI2C_P_ReadSwA24;
        (*pI2cReg)->BREG_I2C_ReadA24_Func       = BI2C_P_ReadSwA24;
        (*pI2cReg)->BREG_I2C_ReadSwNoAddr_Func  = BI2C_P_ReadSwNoAddr;
        (*pI2cReg)->BREG_I2C_ReadNoAddr_Func    = BI2C_P_ReadSwNoAddr;
        (*pI2cReg)->BREG_I2C_ReadSwEDDC_Func    = BI2C_P_ReadSwEDDC;
        (*pI2cReg)->BREG_I2C_ReadEDDC_Func      = BI2C_P_ReadSwEDDC;
    }
#if AUTO_I2C_ENABLED
    else if(hChn->chnSettings.autoI2c.enabled) {
        (*pI2cReg)->context                     = (void *)hChn;
        (*pI2cReg)->BREG_I2C_Read_Func          = BI2C_Auto_P_Read;
        (*pI2cReg)->BREG_I2C_ReadNoAddr_Func    = BI2C_Auto_P_ReadNoAddr;
        (*pI2cReg)->BREG_I2C_Write_Func         = BI2C_Auto_P_Write;
        (*pI2cReg)->BREG_I2C_ReadEDDC_Func      = BI2C_Auto_P_ReadEDDC;
    }
#endif
    else
    {
        (*pI2cReg)->context                     = (void *)hChn;
        (*pI2cReg)->BREG_I2C_Write_Func         = BI2C_P_Write;
        (*pI2cReg)->BREG_I2C_WriteNoAck_Func    = BI2C_P_WriteNoAck;
        (*pI2cReg)->BREG_I2C_WriteA16_Func      = BI2C_P_WriteA16;
        (*pI2cReg)->BREG_I2C_WriteA24_Func      = BI2C_P_WriteA24;
        (*pI2cReg)->BREG_I2C_WriteNoAddr_Func   = BI2C_P_WriteNoAddr;
        (*pI2cReg)->BREG_I2C_WriteNoAddrNoAck_Func  = BI2C_P_WriteNoAddrNoAck;
        (*pI2cReg)->BREG_I2C_WriteNvram_Func    = BI2C_P_WriteNvram;
        (*pI2cReg)->BREG_I2C_Read_Func          = BI2C_P_Read;
        (*pI2cReg)->BREG_I2C_ReadNoAck_Func     = BI2C_P_ReadNoAck;
        (*pI2cReg)->BREG_I2C_ReadA16_Func       = BI2C_P_ReadA16;
        (*pI2cReg)->BREG_I2C_ReadA24_Func       = BI2C_P_ReadA24;
        (*pI2cReg)->BREG_I2C_ReadNoAddr_Func    = BI2C_P_ReadNoAddr;
        (*pI2cReg)->BREG_I2C_ReadNoAddrNoAck_Func  = BI2C_P_ReadNoAddrNoAck;
        (*pI2cReg)->BREG_I2C_ReadEDDC_Func      = BI2C_P_ReadEDDC;
        (*pI2cReg)->BREG_I2C_WriteEDDC_Func     = BI2C_P_WriteEDDC;
        (*pI2cReg)->BREG_I2C_SetupHdmiHwAccess_Func = BI2C_P_SetupHdmiHwAccess;
        (*pI2cReg)->BREG_I2C_WriteRead_Func     = BI2C_P_WriteRead;
        (*pI2cReg)->BREG_I2C_WriteReadNoAddr_Func     = BI2C_P_WriteReadNoAddr;
    }

done:
    return( retCode );
}

BERR_Code BI2C_CloseI2cRegHandle(
    BREG_I2C_Handle     hI2cReg
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hI2cReg );
    BKNI_Free( (void *) hI2cReg );

    return( retCode );
}

void BI2C_SetClk(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    BI2C_Clk            clkRate         /* pointer to clock rate setting */
    )
{
    BI2C_Handle hDev;
    uint32_t    ctlReg;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    hDev = hChn->hI2c;

    hChn->chnSettings.clkRate = clkRate;

    ctlReg = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    ctlReg &= ~(BCHP_BSCA_CTL_REG_SCL_SEL_MASK | BCHP_BSCA_CTL_REG_DIV_CLK_MASK);
    switch (clkRate)
    {
        case BI2C_Clk_eClk100Khz:
        case BI2C_Clk_eClk400Khz:
            ctlReg |= (0x01 << BCHP_BSCA_CTL_REG_SCL_SEL_SHIFT);
            break;

        case BI2C_Clk_eClk47Khz:
        case BI2C_Clk_eClk187Khz:
            ctlReg |= (0x02 << BCHP_BSCA_CTL_REG_SCL_SEL_SHIFT);
            break;

        case BI2C_Clk_eClk50Khz:
        case BI2C_Clk_eClk200Khz:
            ctlReg |= (0x03 << BCHP_BSCA_CTL_REG_SCL_SEL_SHIFT);
            break;

        case BI2C_Clk_eClk93Khz:
        case BI2C_Clk_eClk375Khz:
            break;

    }
    if (clkRate < BI2C_Clk_eClk187Khz)
        ctlReg |= BCHP_BSCA_CTL_REG_DIV_CLK_MASK;

    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), ctlReg );
}

#ifdef BI2C_USES_SET_SDA_DELAY
void BI2C_SetSdaDelay(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    BI2C_SdaDelay       sdaDelay        /* Sda delay value */
    )
{
    BI2C_Handle hDev;
    uint32_t    ctlReg;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    hDev = hChn->hI2c;

    ctlReg = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG));
    ctlReg &= ~(BCHP_BSCA_CTL_REG_reserved0_MASK | BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_MASK);
    switch (sdaDelay)
    {
        /*
        case BI2C_eSdaDelay815ns:
            ctlReg |= (0x0 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        */
        case BI2C_eSdaDelay370ns:
            ctlReg |= (0x1 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay482ns:
            ctlReg |= (0x2 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay593ns:
            ctlReg |= (0x3 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay704ns:
            ctlReg |= (0x4 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay815ns:
            ctlReg |= (0x5 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay926ns:
            ctlReg |= (0x6 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
        case BI2C_eSdaDelay1037ns:
            ctlReg |= (0x7 << BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT);
            break;
    }
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTL_REG), ctlReg );
}
#endif

BI2C_Clk BI2C_GetClk(
    BI2C_ChannelHandle  hChn            /* Device channel handle */
)
{
    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    /* TODO: how to handle the Single-Master/Multi-Channel configuration */
    return hChn->chnSettings.clkRate;
}

BERR_Code BI2C_SetBurstMode(
    BI2C_ChannelHandle hChn,    /* [in] I2C channel handle */
    bool bBurstMode             /* [out] Enable or Disable burst mode */
    )
{
    if (hChn == NULL)
    {
        BDBG_ERR(("Invalid I2C channel handle"));
        return BERR_INVALID_PARAMETER;
    }

    hChn->chnSettings.burstMode = bBurstMode;

    return BERR_SUCCESS;
}

BERR_Code BI2C_GetBurstMode(
    BI2C_ChannelHandle hChn,    /* [in] I2C channel handle */
    bool *pbBurstMode           /* [out] current burst mode? */
    )
{
    if ((hChn == NULL) || (pbBurstMode == NULL))
    {
        BDBG_ERR(("Invalid parameters."));
        return BERR_INVALID_PARAMETER;
    }

    *pbBurstMode = hChn->chnSettings.burstMode;

    return BERR_SUCCESS;
}

#ifdef BI2C_SUPPORT_4_BYTE_XFR_MODE
void BI2C_Set4ByteXfrMode(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    bool b4ByteMode                     /* Enable or Disable 4 byte xfr mode */
    )
{
    BI2C_Handle hDev;
    uint32_t    ctlReg;

    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    hDev = hChn->hI2c;

    ctlReg = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG));
    if (b4ByteMode)
    {
        ctlReg |= BCHP_BSCA_CTLHI_REG_DATA_REG_SIZE_MASK;
    }
    else
    {
        ctlReg &= ~BCHP_BSCA_CTLHI_REG_DATA_REG_SIZE_MASK;
    }

    BREG_Write32( hDev->hRegister, (hChn->coreOffset + BCHP_BSCA_CTLHI_REG), ctlReg );
    hChn->chnSettings.fourByteXferMode = b4ByteMode;

}

bool BI2C_Is4ByteXfrMode(
    BI2C_ChannelHandle  hChn           /* Device channel handle */
    )
{
    BDBG_ASSERT( hChn );
    BDBG_OBJECT_ASSERT(hChn, BI2C_P_ChannelHandle);

    return hChn->chnSettings.fourByteXferMode;
}
#endif

/* End of file */
