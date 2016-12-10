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

#include "bchp.h"
#include "bchp_common.h"
#include "breg_spi_priv.h"
#include "bspi.h"

#if (HAS_UPG_MSPI==1)
#include "bchp_mspi.h"
#endif

#if defined(BCHP_IRQ0_REG_START)
#include "bchp_irq0.h"
#include "bchp_int_id_irq0.h"
#include "bchp_int_id_irq0_aon.h"
#endif

#if defined(BCHP_UPG_SPI_AON_IRQ_REG_START) /* Later 64-bit ARM CPU chips e.g. 7260/68/71/78 */
#include "bchp_upg_spi_aon_irq.h"
#endif

#include "bkni_multi.h"

BDBG_MODULE(bspi);

static void BSPI_P_SetClk (BSPI_ChannelHandle channel, uint32_t baud, uint8_t clkConfig);
static BERR_Code BSPI_P_WaitForCompletion(BSPI_ChannelHandle  hChn, uint32_t numBytes);
static void BSPI_P_HandleInterrupt_Isr(void *pParam1, int parm2);
static void BSPI_P_GetBitsPerTransfer(void *pParam1, uint8_t *pBitsPerTransfer);
static BERR_Code BSPI_P_Multiple_Write(void *context, const BREG_SPI_Data *pWriteData, size_t count);
static BERR_Code BSPI_P_WriteAll(void *context, const uint8_t *pWriteData, size_t length);
static BERR_Code BSPI_P_Write(void *context, const uint8_t *pWriteData, size_t length);
static BERR_Code BSPI_P_Read(void *context, const uint8_t *pWriteData, uint8_t *pReadData, size_t length);
static BERR_Code BSPI_P_ReadAll(void *context, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength);

#define BSPI_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define MAX_SPI_XFER        16          /* maximum number of transfers allowed per transaction */

#define SPI_SYSTEM_CLK      27000000    /* 27 MHz */
#if defined(BCHP_MSPI_SPCR3)
#define MAX_SPI_BAUD        13500000     /* SPBR = 1,  CLK=27MHZ */
#else
#define MAX_SPI_BAUD        1687500     /* SPBR = 8, CLK=27MHZ */
#endif

#define MAX_SPI_TIMEOUT     5000        /* 5000 ms = 5 seconds. */

/* Format of the CDRAM, for some reason, this does not show up in RDB file */
#define SPI_CDRAM_CONT              0x80
#define SPI_CDRAM_BITSE             0x40
#define SPI_CDRAM_DT                0x20
#define SPI_CDRAM_DSCK              0x10
#define SPI_CDRAM_PCS_MASK          0x07

#define SPI_CDRAM_PCS_PCS0          0x01
#define SPI_CDRAM_PCS_PCS1          0x02
#define SPI_CDRAM_PCS_PCS2          0x04

#define SPI_CDRAM_PCS_DISABLE_ALL   (SPI_CDRAM_PCS_PCS0 | SPI_CDRAM_PCS_PCS1 | SPI_CDRAM_PCS_PCS2)

#define SPI_POLLING_INTERVAL          10      /* in usecs */
#define SPI_MUTEX_POLLING_INTERVAL    5       /* in millisecs */
#define SPI_MUTEX_POLLING_ITERATIONS  48000   /* So, 5ms*480000 = 240,000 msecs = 4 minutes. */

static void BSPI_P_EnableInterrupt( BSPI_ChannelHandle  hChn );
static void BSPI_P_DisableInterrupt( BSPI_ChannelHandle  hChn );

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

BDBG_OBJECT_ID(BSPI_P_Handle);

typedef struct BSPI_P_Handle
{
    BDBG_OBJECT(BSPI_P_Handle)
    BCHP_Handle     hChip;
    BREG_Handle     hRegister;
    BINT_Handle     hInterrupt;
    unsigned int    maxChnNo;
    BSPI_ChannelHandle hSpiChn[MAX_SPI_CHANNELS];
    BKNI_MutexHandle hUpgMutex;         /* UPG spi mutex handle for serialization */
    unsigned int    mutexChnNo;
    BSPI_ChannelHandle mutexChn; /* NULL means not in lbce mode */
} BSPI_P_Handle;

typedef struct BSPI_P_DelayLimits
{
    struct
    {
        unsigned            max;
        unsigned            min;
        unsigned            fastMax;
        unsigned            fastMin;
    } postTransfer;

    struct
    {
        unsigned            max;
        unsigned            min;
    } chipSelectToClock;

} BSPI_P_DelayLimits;

BDBG_OBJECT_ID(BSPI_P_ChannelHandle);

typedef struct BSPI_P_ChannelHandle
{
    BDBG_OBJECT(BSPI_P_ChannelHandle)
    BSPI_Handle         hSpi;
    unsigned int        chnNo;
    uint32_t            coreOffset;
    BKNI_EventHandle    hChnEvent;
    BINT_CallbackHandle hChnCallback;
    BSPI_Pcs            pcs;
    uint32_t            baud;
    uint8_t             clkConfig;
    bool                intMode;
    uint8_t             bitsPerTxfr;
    bool                continueAfterCommand;       /* Continue after command flag */
    bool                useUserDtlAndDsclk;         /* Use User specified DTL and DSCLK */
    BSPI_AssertSSFunc   assertSSFunc;               /* function to assert SS */
    BSPI_DeassertSSFunc deassertSSFunc;             /* function to deassert SS */
} BSPI_P_ChannelHandle;



/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
void BSPI_P_ACQUIRE_UPG_MUTEX(BSPI_ChannelHandle hChn )
{
    BSPI_Handle hDev;

    hDev = hChn->hSpi;

    if (hDev->mutexChn != hChn) {
        BKNI_AcquireMutex(hDev->hUpgMutex);
        hDev->mutexChn = hChn;
    }
}

void BSPI_P_RELEASE_UPG_MUTEX(BSPI_ChannelHandle hChn )
{
    BSPI_Handle hDev;

    hDev = hChn->hSpi;

    if (hDev->mutexChn == hChn) {
        hDev->mutexChn = NULL;
        BKNI_ReleaseMutex(hDev->hUpgMutex);
    }
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BSPI_Open(
    BSPI_Handle *pSpi,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BSPI_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSPI_Handle hDev;
    unsigned int chnIdx;

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );
    BSTD_UNUSED( pDefSettings );

    /* Alloc memory from the system heap */
    hDev = (BSPI_Handle) BKNI_Malloc( sizeof( BSPI_P_Handle ) );
    if( hDev == NULL )
    {
        *pSpi = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }
	BKNI_Memset(hDev, 0 , sizeof(*hDev));
    BDBG_OBJECT_SET(hDev, BSPI_P_Handle) ;

    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->maxChnNo  = MAX_SPI_CHANNELS;
    hDev->mutexChnNo = hDev->maxChnNo;
    for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
    {
        hDev->hSpiChn[chnIdx] = NULL;
    }
    retCode = BKNI_CreateMutex( &hDev->hUpgMutex );
    if( retCode != BERR_SUCCESS ) goto done;

    *pSpi = hDev;

done:
    if(retCode){
        BKNI_Free( hDev );
        hDev = NULL;
    }
    return( retCode );
}

BERR_Code BSPI_Close(
    BSPI_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);

    if(hDev->hUpgMutex)BKNI_DestroyMutex(hDev->hUpgMutex);
    hDev->hUpgMutex = NULL;

    BDBG_OBJECT_DESTROY(hDev, BSPI_P_Handle);
    BKNI_Free( (void *) hDev );

    return( retCode );
}

BERR_Code BSPI_GetDefaultSettings(
    BSPI_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);

	BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    return( retCode );
}

BERR_Code BSPI_GetTotalChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);

    *totalChannels = hDev->maxChnNo;

    return( retCode );
}

BERR_Code BSPI_GetTotalUpgSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalUpgChannels      /* [output] Returns total number of UPG SPI hannels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);

    *totalUpgChannels = MAX_SPI_CHANNELS;

    return( retCode );
}

BERR_Code BSPI_GetTotalHifSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalHifChannels      /* [output] Returns total number of HIF SPI hannels supported */
    )
{
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(totalHifChannels);
    BDBG_WRN(("HIF SPI is not supported"));
    return( BERR_NOT_SUPPORTED );
}

BERR_Code BSPI_GetChannelDefaultSettings(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BSPI_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);

	if(channelNo >= MAX_SPI_CHANNELS){
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	pChnDefSettings->baud = MAX_SPI_BAUD;
	pChnDefSettings->clkConfig = SPI_REG(SPCR0_MSB_CPOL_MASK) | SPI_REG(SPCR0_MSB_CPHA_MASK);
	pChnDefSettings->intMode = true;
	pChnDefSettings->bitsPerTxfr = 8;
	pChnDefSettings->useUserDtlAndDsclk = false;
	pChnDefSettings->spiCore = BSPI_SpiCore_Upg;

done:
    return( retCode );
}

BERR_Code BSPI_GetChannelDefaultSettings_Ext(
    BSPI_Handle         hDev,                   /* Device handle */
    BSPI_ChannelInfo    channel,                /* Channel number info */
    BSPI_ChannelSettings *pChnDefSettings       /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);
	if(channel.channelNo >= MAX_SPI_CHANNELS){
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
	}

	pChnDefSettings->baud = MAX_SPI_BAUD;
	pChnDefSettings->clkConfig = SPI_REG(SPCR0_MSB_CPOL_MASK) | SPI_REG(SPCR0_MSB_CPHA_MASK);
	pChnDefSettings->intMode = true;
	pChnDefSettings->bitsPerTxfr = 8;
	pChnDefSettings->useUserDtlAndDsclk = false;
	pChnDefSettings->spiCore = BSPI_SpiCore_Upg;

done:
    return( retCode );
}

BERR_Code BSPI_OpenChannel(
    BSPI_Handle hDev,                   /* Device handle */
    BSPI_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BSPI_ChannelSettings *pChnDefSettings /* Channel default setting */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BSPI_ChannelHandle  hChnDev;

    BDBG_OBJECT_ASSERT( hDev,  BSPI_P_Handle);

    hChnDev = NULL;

    if( channelNo < hDev->maxChnNo )
    {
        /* coverity[overrun_static] */
        if( hDev->hSpiChn[channelNo] == NULL )
        {
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
            if ((pChnDefSettings->bitsPerTxfr != 32) && (pChnDefSettings->bitsPerTxfr != 64))
#endif
            if ((pChnDefSettings->bitsPerTxfr != 8) && (pChnDefSettings->bitsPerTxfr != 16))
            {
                retCode = BERR_INVALID_PARAMETER;
                goto done;
            }
            /* Alloc memory from the system heap */
            hChnDev = (BSPI_ChannelHandle) BKNI_Malloc( sizeof( BSPI_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                *phChn = NULL;
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                goto done;
            }
            BKNI_Memset(hChnDev, 0, sizeof(*hChnDev));
            BDBG_OBJECT_SET(hChnDev, BSPI_P_ChannelHandle) ;

            BSPI_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChnDev->hChnEvent) ) );
            hChnDev->hSpi       = hDev;

            hChnDev->coreOffset = 0;
            switch (channelNo)
            {
                case 0:
                    hChnDev->pcs = BSPI_Pcs_eUpgSpiPcs0;
                    break;
                case 1:
                    hChnDev->pcs = BSPI_Pcs_eUpgSpiPcs1;
                    break;
            #if MAX_SPI_CHANNELS > 2
                case 2:
                    hChnDev->pcs = BSPI_Pcs_eUpgSpiPcs2;
                    break;
            #endif
                default:
                    retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                    goto done;
            }
            hChnDev->baud       = pChnDefSettings->baud;
            hChnDev->clkConfig  = pChnDefSettings->clkConfig;
            hChnDev->bitsPerTxfr = pChnDefSettings->bitsPerTxfr;
            hChnDev->useUserDtlAndDsclk = pChnDefSettings->useUserDtlAndDsclk;
            hChnDev->chnNo = channelNo;

            hChnDev->assertSSFunc = NULL;
            hChnDev->deassertSSFunc = NULL;

            hDev->hSpiChn[channelNo] = hChnDev;

            /* Program SPI clock setting */
            BSPI_P_ACQUIRE_UPG_MUTEX( hChnDev );
            BSPI_P_SetClk (hChnDev, pChnDefSettings->baud, pChnDefSettings->clkConfig);
            BSPI_P_RELEASE_UPG_MUTEX( hChnDev );
            BREG_Write32( hDev->hRegister, (hChnDev->coreOffset + SPI_REG(MSPI_STATUS)), 0);

            /*
             * Enable interrupt for this channel
             */
            hChnDev->intMode = pChnDefSettings->intMode;
            if (hChnDev->intMode == true)
            {
                /*
                 * Register and enable L2 interrupt.
                 */
#if !defined(BCHP_INT_ID_spi_irqen)

#if defined(BCHP_UPG_SPI_AON_IRQ_CPU_MASK_CLEAR)
#define BCHP_INT_ID_spi_irqen BCHP_INT_ID_CREATE(BCHP_UPG_SPI_AON_IRQ_CPU_MASK_CLEAR, BCHP_UPG_SPI_AON_IRQ_CPU_MASK_CLEAR_spi_SHIFT)
#else
#if HAS_UPG_MSPI
#define BCHP_INT_ID_spi_irqen           BCHP_INT_ID_spi
#else
/* HIF_MSPI not supported in Magnum */
#define BCHP_INT_ID_spi_irqen           0
#endif
#endif
#endif
                if (channelNo < MAX_SPI_CHANNELS)
                {
                    retCode = BINT_CreateCallback(&(hChnDev->hChnCallback), hDev->hInterrupt,
                                    BCHP_INT_ID_spi_irqen, BSPI_P_HandleInterrupt_Isr, (void *) hChnDev, 0x00);
                }

                if (retCode) {
                    BDBG_ERR(("Unable to enable IRQ0_SPI_IRQEN. See bint.inc and the BCHP_DISABLE_IRQ0_SPI option."));
                    retCode = BERR_TRACE(retCode);
                    goto done;
                }

                retCode = BINT_EnableCallback(hChnDev->hChnCallback);
                if (retCode) {
                    retCode = BERR_TRACE(retCode);
                    goto done;
                }

                BSPI_P_EnableInterrupt(hChnDev);
            }
            else
            {
                hChnDev->hChnCallback = NULL;
                /* No need to disable BCHP_IRQ0_SPI_IRQEN. No one should have enabled it. Be aware that direct access to BCHP_IRQ0_SPI_IRQEN is not allowed
                in this PI. A potential conflict with the kernel is possible. See bint.inc for a note. */

                BSPI_P_DisableInterrupt(hChnDev);
            }

            *phChn = hChnDev;

        }
        else
        {
            retCode = BSPI_ERR_NOTAVAIL_CHN_NO;
        }
    }
    else
    {
        retCode = BERR_INVALID_PARAMETER;
    }

done:

    if( retCode != BERR_SUCCESS )
    {
        if( hChnDev != NULL )
        {
            BKNI_DestroyEvent( hChnDev->hChnEvent );
            BKNI_Free( hChnDev );
            hDev->hSpiChn[channelNo] = NULL;
            *phChn = NULL;
        }
    }
    return( retCode );
}

BERR_Code BSPI_CloseChannel(
    BSPI_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSPI_Handle hDev;
    unsigned int chnNo;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    /*
     * Disable interrupt for this channel
     */
    BSPI_P_DisableInterrupt(hChn);

    if (hChn->hChnCallback != NULL)
    {
        (void)BINT_DestroyCallback( hChn->hChnCallback );
    }
    BKNI_DestroyEvent( hChn->hChnEvent );
    chnNo = hChn->chnNo;
    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    BDBG_OBJECT_DESTROY(hChn, BSPI_P_ChannelHandle);
    BKNI_Free( hChn );
    hDev->hSpiChn[chnNo] = NULL;

    return( retCode );
}

BERR_Code BSPI_GetDevice(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    BSPI_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    *phDev = hChn->hSpi;

    return( retCode );
}


BERR_Code BSPI_CreateSpiRegHandle(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    BREG_SPI_Handle *pSpiReg            /* [output]  */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    *pSpiReg = (BREG_SPI_Handle)BKNI_Malloc( sizeof(BREG_SPI_Impl) );
    if( *pSpiReg == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    (*pSpiReg)->context                                  = (void *)hChn;
    (*pSpiReg)->BREG_SPI_Get_Bits_Per_Transfer_Func      = BSPI_P_GetBitsPerTransfer;
    (*pSpiReg)->BREG_SPI_Multiple_Write_Func             = BSPI_P_Multiple_Write;
    (*pSpiReg)->BREG_SPI_Write_All_Func                  = BSPI_P_WriteAll;
    (*pSpiReg)->BREG_SPI_Write_Func                      = BSPI_P_Write;
    (*pSpiReg)->BREG_SPI_Read_Func                       = BSPI_P_Read;
    (*pSpiReg)->BREG_SPI_Read_All_Func                   = BSPI_P_ReadAll;

done:
    return( retCode );
}

BERR_Code BSPI_CloseSpiRegHandle(
    BREG_SPI_Handle     hSpiReg         /* SPI register handle */
    )
{

    BDBG_ASSERT( hSpiReg );
    BKNI_Free( (void *) hSpiReg );

    return BERR_SUCCESS ;
}

BERR_Code BSPI_AbortTxfr(
    BSPI_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval, i = 0, numIterations = 0, sleepTime = 100; /* 100 ms. */
    BSPI_Handle hDev;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)));
    if (!(lval & SPI_REG(SPCR2_spe_MASK)))         /* if no transfer is going on, we're done */
        goto done;

    /* Set the HALT bit to stop the transfer */
    lval |= SPI_REG(SPCR2_halt_MASK);
    BREG_Write32 (hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)), lval);
    numIterations = MAX_SPI_TIMEOUT/sleepTime;

    BDBG_ERR(("BSPI_AbortTxfr: sleepTime = %d", sleepTime));

    /* Now wait until the Halt Acknowledge bit is set */
    for(i=0; i < numIterations; i++){
        BKNI_Sleep(sleepTime);
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)));
        if(lval & SPI_REG(MSPI_STATUS_HALTA_MASK))
            break;
    }
    if(i == numIterations){
        retCode = BERR_TRACE(BERR_TIMEOUT);
        BDBG_ERR(("BSPI_AbortTxfr: Spi transfer abort failed"));
    }

    BKNI_EnterCriticalSection();
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), 0);
    BKNI_LeaveCriticalSection();
    BREG_Write32 (hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)), 0);

done:
    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    return( retCode );
}

BERR_Code BSPI_SetBitsPerTransfer(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    uint8_t bitsPerTxfr                   /* clock config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    hChn->bitsPerTxfr = bitsPerTxfr;
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_GetClkConfig(
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    uint8_t *pClkConfig                     /* Pointer to clock config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    *pClkConfig = hChn->clkConfig;
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_SetClkConfig(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    uint8_t clkConfig                   /* clock config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    hChn->clkConfig = clkConfig;
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_GetDTLConfig (
    BSPI_ChannelHandle hChn,    /* Device channel handle */
    uint32_t *pDTLConfig        /* pointer to DTLConfig */
    )
{
    BSPI_Handle         hDev;
    uint32_t            data;
    BERR_Code           retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    data = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_LSB)));
    *pDTLConfig = (data & SPI_REG(SPCR1_LSB_DTL_MASK));
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_SetDTLConfig (
    BSPI_ChannelHandle context,             /* Device channel handle */
    const uint32_t data         /* data to write */
    )
{
    BSPI_Handle         hDev=NULL;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;

    if (data > 255)
    {
        BDBG_ERR(("BSPI_SetDTLConfig: DTL value needs to be between 0 and 255."));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    hChn = context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_LSB)), data );
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

done:
    return retCode;
}


BERR_Code BSPI_GetRDSCLKConfig (
    BSPI_ChannelHandle hChn,    /* Device channel handle */
    uint32_t *pRDSCLKConfig     /* pointer to RDSCLKConfig */
    )
{
    BSPI_Handle         hDev;
    uint32_t            data;
    BERR_Code           retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    data = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_MSB)));
    *pRDSCLKConfig = (data & SPI_REG(SPCR1_MSB_RDSCLK_MASK));
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_SetRDSCLKConfig(
    BSPI_ChannelHandle hChn,             /* Device channel handle */
    const uint32_t data         /* data to write */
    )
{
    BSPI_Handle         hDev=NULL;
    BERR_Code           retCode = BERR_SUCCESS;

    if (data > 255)
    {
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_MSB)), data );
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

done:
    return retCode;
}

void BSPI_RegisterSSFunctions (
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    BSPI_AssertSSFunc assertFunc,           /* Assert SS function */
    BSPI_DeassertSSFunc deassertFunc        /* Deassert SS function */
    )
{
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hChn->assertSSFunc = assertFunc;
    hChn->deassertSSFunc = deassertFunc;
}

void BSPI_SetLastByteContinueEnable(
    BSPI_ChannelHandle hChn,
    bool bEnable
)
{
	BSTD_UNUSED(bEnable);
	BSTD_UNUSED(hChn);
	BDBG_WRN(("This api is deprecated. Use BREG_SPI_SetContinueAfterCommand() instead."));
}

void  BSPI_GetDelay(
    BSPI_ChannelHandle hChn,         /* Device channel handle */
    BSPI_Delay *pDelay               /* pointer to DTLConfig */
    )
{
    BSPI_Handle         hDev=NULL;
    uint32_t dtl=0, rdsclk=0;
#if defined(BCHP_MSPI_SPCR3)
    uint32_t lval=0;
#endif

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

#if defined(BCHP_MSPI_SPCR3)
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
#endif

    if(hChn->useUserDtlAndDsclk){
        dtl = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_LSB)));
#if defined(BCHP_MSPI_SPCR3)
        if(lval & SPI_REG(SPCR1_LSB_DTL_MASK)){
            pDelay->postTransfer = ((dtl + 5)*1000)/(SPI_SYSTEM_CLK/1000000);
        }
        else{
#endif
            pDelay->postTransfer = (dtl * 32 * 1000)/(SPI_SYSTEM_CLK/1000000);
#if defined(BCHP_MSPI_SPCR3)
        }
#endif
        rdsclk = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_MSB)));
        pDelay->chipSelectToClock = (rdsclk * 1000)/(SPI_SYSTEM_CLK/1000000);
    }
    else{
        pDelay->postTransfer = (32 * 1000)/(SPI_SYSTEM_CLK/1000000);
        pDelay->chipSelectToClock = (1 * 1000)/(2*SPI_SYSTEM_CLK/1000000);
    }

    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return ;
}

static void BSPI_P_GetDelayLimits(
    BSPI_P_DelayLimits  *limits
    )
{
    unsigned            freqMHz=0;

    freqMHz = SPI_SYSTEM_CLK/1000000;

    limits->postTransfer.max = (32 * 256 * 1000)/freqMHz;
    limits->postTransfer.min = (32 * 1 * 1000)/freqMHz;
    limits->postTransfer.fastMax = ((256+5) * 1000)/freqMHz;
    limits->postTransfer.fastMin = ((1+5) * 1000)/freqMHz;
    limits->chipSelectToClock.max = (256 * 1000)/freqMHz;
    limits->chipSelectToClock.min = (1 * 1000)/freqMHz;

    return;
}

BERR_Code BSPI_SetDelay(
    BSPI_ChannelHandle hChn,         /* Device channel handle */
    const BSPI_Delay *delay           /* pointer to DTLConfig */
    )
{
    BSPI_Handle         hDev=NULL;
    BERR_Code           retCode = BERR_SUCCESS;
    BSPI_P_DelayLimits  limits;
    uint32_t dtl=0, rdsclk=0;
#if defined(BCHP_MSPI_SPCR3)
    uint32_t lval=0;
#endif

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;

    BSPI_P_GetDelayLimits(&limits);

    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

#if defined(BCHP_MSPI_SPCR3)
    if ((delay->postTransfer > limits.postTransfer.fastMin) && (delay->postTransfer <= limits.postTransfer.fastMax))
    {
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
        lval |= 0x2;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval );
        dtl = ((delay->postTransfer * 27)/1000) - 5;
    }
    else if ((delay->postTransfer > limits.postTransfer.fastMax) && (delay->postTransfer <= limits.postTransfer.max))
    {
        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
        lval &= ~0x2;
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval );
        dtl = (delay->postTransfer * 27)/(32*1000);
    }
    else
    {
        BDBG_ERR(("With %d clock frequency, %dns is the maximum and %dns is the minimum post transfer delays supported.", \
            SPI_SYSTEM_CLK, limits.postTransfer.max, limits.postTransfer.fastMin));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }
#endif
    if ((delay->chipSelectToClock > limits.chipSelectToClock.min) && (delay->chipSelectToClock <= limits.chipSelectToClock.max))
    {
        rdsclk = (delay->chipSelectToClock * 27)/1000;
        if (rdsclk)
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_MSB)), rdsclk );
    }
    else
    {
        BDBG_ERR(("With %d clock frequency, %dns is the maximum and %dns is the minimum delays from chip select to clock transition supported.", \
            SPI_SYSTEM_CLK, limits.chipSelectToClock.max, limits.chipSelectToClock.min));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }
    if (dtl)
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR1_LSB)), dtl );

done:
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return( retCode );
}

BERR_Code BSPI_P_Start(
    BSPI_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval=0;
    BSPI_Handle hDev;

    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;

    lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)));

    lval |= SPI_REG(SPCR2_spe_MASK);

#if defined(BCHP_MSPI_SPCR2_cont_after_cmd_MASK)
	if(hChn->continueAfterCommand) {
        lval |= SPI_REG(SPCR2_cont_after_cmd_MASK);
	}
	else {
        lval &= (~SPI_REG(SPCR2_cont_after_cmd_MASK));
	}
#endif
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)), lval);

    return( retCode );
}

void BSPI_P_SetContinueAfterCommand(
    void *context,              /* Device channel handle */
    bool bEnable
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;

    if (bEnable == hChn->continueAfterCommand) {
        BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hChn->continueAfterCommand = bEnable;
}


/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
void BSPI_P_GetBitsPerTransfer
(
    void *context,             /* Device channel handle */
    uint8_t *pBitsPerTransfer  /* Number of bits transferred per command byte */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    *pBitsPerTransfer = hChn->bitsPerTxfr;

    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    return;
}


/* Only supported for 8 bit transfer mode for now. */
BERR_Code BSPI_P_ReadAll
(
    void *context,              /* Device channel handle */
    const uint8_t *pWriteData,      /* pointer to memory location where data is to sent  */
    size_t writeLength,             /* size of *pWriteData buffer */
    uint8_t *pReadData,             /* pointer to memory location to store read data  */
    size_t readLength               /* size of *pReadData buffer */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval_cont = 0, lval_stop = 0, lval = 0, i = 0, j = 0, k = 0, count = 0;

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    if (hChn->assertSSFunc)
    {
        (*(hChn->assertSSFunc)) ();
    }

    if( hChn->useUserDtlAndDsclk == true)
    {
        lval_stop = SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_DT | SPI_CDRAM_DSCK;
    }
    else
    {
        lval_stop = SPI_CDRAM_PCS_DISABLE_ALL;
    }
    lval_stop &= ~(1 << hChn->pcs);

    lval_cont = lval_stop | SPI_CDRAM_CONT;

    for (; i < readLength+writeLength; i++, count++)
    {
        count  %= MAX_SPI_XFER;

        if(i < writeLength)
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (count * 8)), (uint32_t)(pWriteData[i]) );

        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (count * 4)), lval_cont );

        if(i == (readLength+writeLength-1))
        {
            /* Skip for slower SPI device  */
            if( 0){
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (count * 4)), lval_stop );
            }
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), count );

            BSPI_P_Start(hChn);
            /* Wait for SPI to finish */
            BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, MAX_SPI_XFER));

            if(i>=writeLength){
                for (k=0; k<=count; k++)
                {
                    lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (k * 8) + 4));
                    /* j is unsigned and hence pReadData[j++ - writeLength] overwritten till j >= writelength */
                    pReadData[j++ - writeLength] = (uint8_t)lval;
                }
            }
            break;
        }

        if(count == (MAX_SPI_XFER - 1)) {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), count );

            BSPI_P_Start(hChn);
            /* Wait for SPI to finish */
            BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, MAX_SPI_XFER));

            if(i>=writeLength){
                for (k=0; k<=count; k++)
                {
                    lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (k * 8) + 4));
                    /* j is unsigned and hence pReadData[j++ - writeLength] overwritten till j >= writelength */
                    pReadData[j++ - writeLength] = (uint8_t)lval;
                }
            }
        }
    }
done:
    if (hChn->deassertSSFunc)
    {
        (*(hChn->deassertSSFunc)) ();
    }
    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    return retCode;
}

BERR_Code BSPI_P_Read
(
    void *context,              /* Device channel handle */
    const uint8_t *pWriteData,      /* pointer to write data  */
    uint8_t *pReadData,         /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read + number of bytes to write */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval = 0, cdRamVal = 0, i;
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    uint32_t            j;
#endif

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    if (((hChn->bitsPerTxfr == 8) && (length > MAX_SPI_XFER)) ||
        ((hChn->bitsPerTxfr == 16) && ((length > (MAX_SPI_XFER * 2)) || (length & 0x01))) ||
        ((hChn->bitsPerTxfr == 32) && ((length > (MAX_SPI_XFER * 4)) || (length & 0x03))) ||
        ((hChn->bitsPerTxfr == 64) && ((length > (MAX_SPI_XFER * 8)) || (length & 0x07))))
#else
    if (((hChn->bitsPerTxfr == 8) && (length > MAX_SPI_XFER))  ||
        ((hChn->bitsPerTxfr == 16) && ((length > (MAX_SPI_XFER * 2)) || (length & 0x01))))
#endif
    {
        if(length % (hChn->bitsPerTxfr/8)){
            BDBG_ERR(("Reading %u bytes in %d bitsPerTransfer mode is not supported.", (unsigned) length, hChn->bitsPerTxfr));
            BDBG_ERR(("Use BSPI_P_ReadAll() to read data in 8 bitsperTransfer mode or use BSPI_SetBitsPerTransfer() to set bitsPerTransfer to 8."));
        }
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }

    if ((hChn->bitsPerTxfr == 8) || (hChn->bitsPerTxfr == 64))
    {
        if( hChn->useUserDtlAndDsclk == true)
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_DT | SPI_CDRAM_DSCK;
        }
        else
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL;
        }
    }
    else {
        if( hChn->useUserDtlAndDsclk == true)
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_DT | SPI_CDRAM_DSCK | SPI_CDRAM_BITSE;
        }
        else
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_BITSE;
        }
    }

    cdRamVal &= ~(1 << hChn->pcs);

    /* Fill the TX and CMD buffers */
    if (hChn->bitsPerTxfr == 8)
    {
        /*
         * 8-bit transfer mode
         */
        for (i=0; i<length; i++)
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (i * 8)), (uint32_t)(pWriteData[i]) );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (i * 4)), cdRamVal );
        }

        /* Set queue pointers */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)(length - 1) );
    }
    else if (hChn->bitsPerTxfr == 16)
    {
        /*
         * 16-bit transfer mode
         */
        lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
        lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);

        for (i=0; i<length; i+=2)
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (i * 4)),     (uint32_t)(pWriteData[i]) );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (i * 4) + 4), (uint32_t)(pWriteData[i+1]) );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((i/2) * 4)), cdRamVal );
        }

        /* Set queue pointers */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/2) - 1) );
    }
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    else if (hChn->bitsPerTxfr == 32)
    {
        /*
         * 32-bit transfer mode
         */
        lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
        lval |= SPI_REG(SPCR3_data_reg_size_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval);

        lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
        lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
        lval |= (0x20 << SPI_REG(SPCR0_MSB_BitS_SHIFT));
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);

        for (i=0, j=0; i<length; i+=4, j+=2)
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4)),
                        ((uint32_t)(pWriteData[i])  << 24) | ((uint32_t)(pWriteData[i+1]) << 16) |
                        ((uint32_t)(pWriteData[i+2]) << 8) |  (uint32_t)(pWriteData[i+3]));

            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((j/2) * 4)), cdRamVal );
        }

        /* Set queue pointers */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/4) - 1) );
    }
    else if (hChn->bitsPerTxfr == 64)
    {
        /*
         * 64-bit transfer mode
         */
        lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
        lval |= SPI_REG(SPCR3_data_reg_size_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval);

        lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
        lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);

        for (i=0, j=0; i<length; i+=8, j+=2)
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4)),
                        ((uint32_t)(pWriteData[i]) << 24)  | ((uint32_t)(pWriteData[i+1]) << 16) |
                        ((uint32_t)(pWriteData[i+2]) << 8) |  (uint32_t)(pWriteData[i+3]));
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4) + 4),
                        ((uint32_t)(pWriteData[i+4]) << 24) | ((uint32_t)(pWriteData[i+5]) << 16) |
                        ((uint32_t)(pWriteData[i+6]) << 8)  |  (uint32_t)(pWriteData[i+7]));

            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((j/2) * 4)), cdRamVal );
        }

        /* Set queue pointers */
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/8) - 1) );
    }
#endif


    if (hChn->assertSSFunc)
    {
        (*(hChn->assertSSFunc)) ();
    }

    /* Start SPI transfer */
    BSPI_P_Start(hChn);

    /* Wait for SPI to finish */
    BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, length));

    if (hChn->deassertSSFunc)
    {
        (*(hChn->deassertSSFunc)) ();
    }

    /* Transfer finished, clear SPIF bit */
    BKNI_EnterCriticalSection();
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), 0);
    BKNI_LeaveCriticalSection();
    /* Copy data to user buffer */
    if (hChn->bitsPerTxfr == 8)
    {
        /*
         * 8-bit transfer mode
         */
        for (i=0; i<length; i++)
        {
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (i * 8) + 4));
            pReadData[i] = (uint8_t)lval;
        }
    }
    else if (hChn->bitsPerTxfr == 16)
    {
        /*
         * 16-bit transfer mode
         */
        for (i=0; i<length; i+=2)
        {
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (i * 4)));
            pReadData[i] = (uint8_t)lval;
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (i * 4) + 4));
            pReadData[i+1] = (uint8_t)lval;
        }
    }
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    else if (hChn->bitsPerTxfr == 32)
    {
        /*
         * 32-bit transfer mode
         */
        for (i=0, j=0; i<length; i+=4, j+=2)
        {
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (j * 4) + 4));
            pReadData[i]   = (uint8_t)(lval >> 24);
            pReadData[i+1] = (uint8_t)(lval >> 16);
            pReadData[i+2] = (uint8_t)(lval >>  8);
            pReadData[i+3] = (uint8_t)(lval & 0xff);
        }
    }
    else if (hChn->bitsPerTxfr == 64)
    {
        /*
         * 64-bit transfer mode
         */
        for (i=0, j=0; i<length; i+=8, j+=2)
        {
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (j * 4)));
            pReadData[i]   = (uint8_t)(lval >> 24);
            pReadData[i+1] = (uint8_t)(lval >> 16);
            pReadData[i+2] = (uint8_t)(lval >>  8);
            pReadData[i+3] = (uint8_t)lval;
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(RXRAM00) + (j * 4) + 4));
            pReadData[i+4] = (uint8_t)(lval >> 24);
            pReadData[i+5] = (uint8_t)(lval >> 16);
            pReadData[i+6] = (uint8_t)(lval >>  8);
            pReadData[i+7] = (uint8_t)lval;
        }
    }
#endif

done:
    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return retCode;
}

BERR_Code BSPI_P_Multiple_Write
(
	void *context,			/* Device channel handle */
	const BREG_SPI_Data *pWriteData,
	size_t count
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            i = 0;
    BREG_SPI_Data       *tempData = NULL;

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    hDev = hChn->hSpi;
    tempData = (BREG_SPI_Data *)pWriteData;

    if(count < 2) {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    BSPI_P_SetContinueAfterCommand(context , true);

    for(i=0; i < count; i++){
        if(i == (count - 1))
            BSPI_P_SetContinueAfterCommand(context , false);

        BSPI_CHK_RETCODE( retCode, BSPI_P_WriteAll(context, tempData->data, (size_t)tempData->length));
        tempData++;
    }

done:
    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    return retCode;
}


BERR_Code BSPI_P_WriteAll
(
    void *context,          /* Device channel handle */
    const uint8_t *pData,   /* pointer to data to write */
    size_t length           /* number of bytes to write */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval= 0, i = 0, count = 0;
    bool	            internal=false;

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

    if (hChn->assertSSFunc)
    {
        (*(hChn->assertSSFunc)) ();
    }

    if((hChn->continueAfterCommand == false) && (length > MAX_SPI_XFER)) {
        hChn->continueAfterCommand = true;
		internal = true;
    }

    if( hChn->useUserDtlAndDsclk == true)
    {
        lval = SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_DT | SPI_CDRAM_DSCK | SPI_CDRAM_CONT;
    }
    else
    {
        lval = SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_CONT;
    }
    lval &= ~(1 << hChn->pcs);

    for (i=0, count = 0; i<length; i++, count++)
    {
        count  %= MAX_SPI_XFER;

        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (count * 8)), (uint32_t)(pData[i]) );
        BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (count * 4)), lval );

        if(i == (length - 1))
        {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), count );

            if(internal) {
                hChn->continueAfterCommand = false;
            }

            BSPI_P_Start(hChn);

            /* Wait for SPI to finish */
            BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, MAX_SPI_XFER));
            break;
        }

        if(count == (MAX_SPI_XFER - 1)) {
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), count );

            BSPI_P_Start(hChn);
            /* Wait for SPI to finish */
            BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, MAX_SPI_XFER));
        }
    }

done:
    if (hChn->deassertSSFunc)
    {
        (*(hChn->deassertSSFunc)) ();
    }
    BSPI_P_RELEASE_UPG_MUTEX( hChn );
    return retCode;
}

BERR_Code BSPI_P_Write
(
    void *context,              /* Device channel handle */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length               /* number of bytes to write */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;
    BERR_Code           retCode = BERR_SUCCESS;
    uint32_t            lval = 0, cdRamVal = 0, cdRamVal_bitse = 0, i, num8BitBytes=0;
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    uint32_t            j;
#endif

    hChn = (BSPI_ChannelHandle)context;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );

    hDev = hChn->hSpi;
    BSPI_P_ACQUIRE_UPG_MUTEX( hChn );

#if BCHP_MSPI_SPCR3_data_reg_size_MASK
    if (((hChn->bitsPerTxfr == 8) && (length > MAX_SPI_XFER)) ||
        ((hChn->bitsPerTxfr == 16) && ((length > (MAX_SPI_XFER * 2)) || (length & 0x01))) ||
        ((hChn->bitsPerTxfr == 32) && ((length > (MAX_SPI_XFER * 4)) || (length & 0x03))) ||
        ((hChn->bitsPerTxfr == 64) && ((length > (MAX_SPI_XFER * 8)) || (length & 0x07))))
#else
    if (((hChn->bitsPerTxfr == 8) && (length > MAX_SPI_XFER))  ||
        ((hChn->bitsPerTxfr == 16) && ((length > (MAX_SPI_XFER * 2)) || (length & 0x01))))
#endif

    {
        if(length % (hChn->bitsPerTxfr/8)){
            BDBG_ERR(("Reading %u bytes in %d bitsPerTransfer mode is not supported.", (unsigned) length, hChn->bitsPerTxfr));
            BDBG_ERR(("Use BSPI_P_WriteAll() to read data in 8 bitsperTransfer mode or use BSPI_SetBitsPerTransfer() to set bitsPerTransfer to 8."));
        }

        BDBG_ERR(("BSPI_P_Write: Either the total bytes transferrable is less than requested number of bytes or it is not a even multiple of (bitsPerTxfr/8)."));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

        if( hChn->useUserDtlAndDsclk == true)
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL | SPI_CDRAM_DT | SPI_CDRAM_DSCK;
        }
        else
        {
            cdRamVal = SPI_CDRAM_CONT | SPI_CDRAM_PCS_DISABLE_ALL;
        }
        cdRamVal_bitse = cdRamVal | SPI_CDRAM_BITSE;

        cdRamVal &= ~(1 << hChn->pcs);
        cdRamVal_bitse &= ~(1 << hChn->pcs);

        /* Fill the TX and CMD buffers */
        if (hChn->bitsPerTxfr == 8)
        {
            /*
             * 8-bit transfer mode
             */
            for (i=0; i<length; i++)
            {
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (i * 8)), (uint32_t)(pData[i]) );
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (i * 4)), cdRamVal );
            }

            /* Set queue pointers */
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)(length - 1) );
        }
        else if (hChn->bitsPerTxfr == 16)
        {
            num8BitBytes = length % (hChn->bitsPerTxfr/8);
            BDBG_MSG(("length = %u, num8BitBytes = %d", (unsigned) length, num8BitBytes));
            if(num8BitBytes==1){
                /* 16-bit transfer mode */
                lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
                lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);
            }

            for (i=0; i<length/2; i++)
            {
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (2 * i * 4)),     (uint32_t)(pData[2*i]) );
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (2 * i * 4) + 4), (uint32_t)(pData[(2*i)+1]) );
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + (i * 4)), cdRamVal_bitse );
            }

            if(num8BitBytes==1){
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + ((length - 1) * 4)), (uint32_t)(pData[length-1]));
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((length/2) * 4)), cdRamVal );
            }

            /* Set queue pointers */
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            if(length==1)
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), 0 );
            else if(num8BitBytes)
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)(length/2) );
            else
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/2) - 1) );
        }
#if BCHP_MSPI_SPCR3_data_reg_size_MASK
        else if (hChn->bitsPerTxfr == 32)
        {
            /*
             * 32-bit transfer mode
             */
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
            lval |= SPI_REG(SPCR3_data_reg_size_MASK);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval);

            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
            lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
            lval |= (0x20 << SPI_REG(SPCR0_MSB_BitS_SHIFT));
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);

            for (i=0, j=0; i<length; i+=4, j+=2)
            {
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4)),
                            ((uint32_t)(pData[i])  << 24) | ((uint32_t)(pData[i+1]) << 16) |
                            ((uint32_t)(pData[i+2]) << 8) |  (uint32_t)(pData[i+3]));

                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((j/2) * 4)), cdRamVal_bitse );
            }

            /* Set queue pointers */
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/4) - 1) );
        }
        else if (hChn->bitsPerTxfr == 64)
        {
            /*
             * 64-bit transfer mode
             */
            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
            lval |= SPI_REG(SPCR3_data_reg_size_MASK);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval);

            lval = BREG_Read32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
            lval &= ~SPI_REG(SPCR0_MSB_BitS_MASK);
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval);

            for (i=0, j=0; i<length; i+=8, j+=2)
            {
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4)),
                            ((uint32_t)(pData[i]) << 24)  | ((uint32_t)(pData[i+1]) << 16) |
                            ((uint32_t)(pData[i+2]) << 8) |  (uint32_t)(pData[i+3]));
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(TXRAM00) + (j * 4) + 4),
                            ((uint32_t)(pData[i+4]) << 24) | ((uint32_t)(pData[i+5]) << 16) |
                            ((uint32_t)(pData[i+6]) << 8)  |  (uint32_t)(pData[i+7]));

                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(CDRAM00) + ((j/2) * 4)), cdRamVal );
            }

            /* Set queue pointers */
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(NEWQP)), 0 );
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(ENDQP)), (uint32_t)((length/8) - 1) );
        }
#endif

    if (hChn->assertSSFunc)
    {
        (*(hChn->assertSSFunc)) ();
    }

    /* Start SPI transfer */
    BSPI_P_Start(hChn);

    /* Wait for SPI to finish */
    BSPI_CHK_RETCODE( retCode, BSPI_P_WaitForCompletion(hChn, length));

    if (hChn->deassertSSFunc)
    {
        (*(hChn->deassertSSFunc)) ();
    }

    /* Transfer finished, clear SPIF bit */
    BKNI_EnterCriticalSection();
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), 0);
    BKNI_LeaveCriticalSection();

done:

    BSPI_P_RELEASE_UPG_MUTEX( hChn );

    return retCode;
}

void BSPI_P_SetClk(
    BSPI_ChannelHandle  hChn,           /* Device channel handle */
    uint32_t            baud,           /* baud rate */
    uint8_t             clkConfig       /* clock configuration */
    )
{
    BSPI_Handle hDev;
    uint32_t    lval;
    BERR_Code   retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BSPI_P_ChannelHandle);

    hDev = hChn->hSpi;

    if (baud > MAX_SPI_BAUD)
    {
        retCode = BERR_INVALID_PARAMETER;
        return;
    }

#if defined(BCHP_MSPI_SPCR3)
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)));
    lval |= 0x3;
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR3)), lval );
#endif

    lval = SPI_SYSTEM_CLK / (2 * baud);
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_LSB)), lval );

    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)));
    lval &= ~(SPI_REG(SPCR0_MSB_CPOL_MASK) | SPI_REG(SPCR0_MSB_CPHA_MASK));

#if defined(BCHP_MSPI_SPCR0_MSB_MSTR_MASK)
    lval |= (SPI_REG(SPCR0_MSB_MSTR_MASK) | clkConfig);
#else
    lval |= clkConfig;
#endif

    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR0_MSB)), lval );
}

static void BSPI_P_EnableInterrupt( BSPI_ChannelHandle  hChn )
{
    uint32_t    lval;
    BSPI_Handle hDev;

    hDev = hChn->hSpi;
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)));
    lval |= SPI_REG(SPCR2_spifie_MASK);
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)), lval );

}

static void BSPI_P_DisableInterrupt( BSPI_ChannelHandle  hChn )
{
    uint32_t    lval;
    BSPI_Handle hDev;

    hDev = hChn->hSpi;
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)));
    lval &= ~SPI_REG(SPCR2_spifie_MASK);
    BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(SPCR2)), lval );

}

BERR_Code BSPI_P_WaitForCompletion
(
    BSPI_ChannelHandle  hChn,                           /* Device channel handle */
    uint32_t            numBytes                        /* number of bytes to transfer */
)
{
    BSPI_Handle         hDev;
    BERR_Code           retCode = BERR_SUCCESS, i = 0, numIterations = 0, delayTime = 5; /* 5 micro secs. */
    uint32_t            lval=0, timeoutMs;

    hDev = hChn->hSpi;

    if (hChn->intMode)
    {
        /*
         * Wait for event, set by ISR
         */

        /* Calculate the timeout value */
        lval = numBytes * 9;                        /* number of clks per byte, assume delay between each byte xfer accounts for 1 extra clock per byte */
        timeoutMs = (lval * 1000) / hChn->baud;     /* get timeout in milliseconds */
        timeoutMs = ((timeoutMs * 110) / 100) + 1;  /* add 10% fudge factor and round up */

        if (timeoutMs < MAX_SPI_TIMEOUT)
            timeoutMs = MAX_SPI_TIMEOUT;

        BSPI_CHK_RETCODE (retCode, BKNI_WaitForEvent(hChn->hChnEvent, timeoutMs));

        lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)));
        if (lval & SPI_REG(MSPI_STATUS_SPIF_MASK)){
            BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), 0);
        }
    }
    else
    {
        /*
         * Polling mode
         */
        numIterations = (MAX_SPI_TIMEOUT*1000)/delayTime;
        for(i=0; i < numIterations; i++){
            BKNI_Delay(delayTime);
            lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)));
            if (lval & SPI_REG(MSPI_STATUS_SPIF_MASK)){
                BREG_Write32( hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), 0);
                break;
            }
        }
        if(i == numIterations){
            retCode = BERR_TRACE(BERR_TIMEOUT);
            BDBG_ERR(("BSPI_P_WaitForCompletion: Spi transfer finish status not received withing %d seconds. Aborting wait", MAX_SPI_TIMEOUT/1000));
        }
    }

done:
    return retCode;
}

void BSPI_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BSPI_ChannelHandle  hChn;
    BSPI_Handle         hDev;
    uint32_t            lval;

    hChn = (BSPI_ChannelHandle) pParam1;
    BDBG_OBJECT_ASSERT( hChn, BSPI_P_ChannelHandle );
    BSTD_UNUSED(parm2);

    hDev = hChn->hSpi;

    /* Clear interrupt */
    lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)));
    lval &= ~SPI_REG(MSPI_STATUS_SPIF_MASK);
    BREG_Write32(hDev->hRegister, (hChn->coreOffset + SPI_REG(MSPI_STATUS)), lval);

    BKNI_SetEvent( hChn->hChnEvent );
    return;
}
