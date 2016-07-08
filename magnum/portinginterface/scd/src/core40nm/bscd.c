/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
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
 *
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bchp_pm.h"
#include "bchp_sca.h"
#include "bchp_scb.h"

#include "bchp_scirq0.h"
#include "bchp_sun_top_ctrl.h"

#include "bscd.h"
#include "bscd_priv.h"
#include "bint.h"
#include "bchp_int_id_scirq0.h"
#include "bchp_gio.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if (BCHP_CHIP==7344)||(BCHP_CHIP==7364) || (BCHP_CHIP==7250)
#include "bchp_aon_pin_ctrl.h"
#include "bchp_gio_aon.h"
#endif

#define BCHP_SCA_SC_CLK_CMD_clk_en_MASK BCHP_SCA_SC_CLK_CMD_1_clk_en_MASK

#if 1
#undef BDBG_ENTER
#undef BDBG_LEAVE
#define BDBG_ENTER(x)
#define BDBG_LEAVE(x)
#endif

BDBG_MODULE(BSCD);


/*******************************************************************************
*   Default Module and Channel Settings.  Note that we could only modify
*   Module settings during BSCD_Open.
*******************************************************************************/
static const BSCD_Settings BSCD_defScdSettings =
{
    /* This attribute indicates the source of clock and the value */
    {BSCD_ClockFreqSrc_eInternalClock, 27000000, false, false},

    /* maximum SCD channels supported */
    BSCD_MAX_SUPPOTED_CHANNELS,
    NULL,
};

static const BSCD_ChannelSettings BSCD_defScdChannelSettings =
{
        /* Smart Card Standard */
        BSCD_Standard_eNDS,

        /* Asynchronous Protocol Types. */
        BSCD_AsyncProtocolType_e0,

        /* This read-only attribute specifies the default
            source clock frequency in Hz. */
        BSCD_INTERNAL_CLOCK_FREQ,

        /* ICC CLK frequency in Hz which is
            source freq / SC_CLK_CMD[etu_clkdiv] / SC_CLK_CMD[sc_clkdiv] */
        BSCD_INTERNAL_CLOCK_FREQ/BSCD_DEFAULT_ETU_CLKDIV/BSCD_DEFAULT_SC_CLKDIV,

        /* ETU in microseconds which is source freq / SC_CLK_CMD[etu_clkdiv] */
            /* (SC_PRESCALE * external_clock_div + (external_clock_div - 1))  */
        BSCD_INTERNAL_CLOCK_FREQ/BSCD_DEFAULT_ETU_CLKDIV/(BSCD_DEFAULT_PRESCALE+1)/BSCD_DEFAULT_BAUD_DIV,

        /* This read-only attribute specifies the maximum IFSD.
            Should be 264. */
        BSCD_MAX_TX_SIZE,

        /* This attribute indicates the current IFSD */
        BSCD_DEFAULT_EMV_INFORMATION_FIELD_SIZE,

        /* Clock Rate Conversion Factor,
            F in 1,2,3,4,5,6,9, 10, 11, 12 or 13.
            Default is 1. */
        BSCD_DEFAULT_F,

        /* Baud Rate Adjustment Factor,
            D in 1,2,3,4,5,6,8 or 9.
            Default is 1. */
        BSCD_DEFAULT_D,

        /*  ETU Clock Divider in
            SC_CLK_CMD register. Valid value is
            from 1 to 8. Default is 6. */
        0,

        /*  SC Clock Divider in
            SC_CLK_CMD register. Valid value is
            1,2,3,4,5,8,10,16. Default is 1. */
        0,

        /* Prescale Value */
        0,

        /* external clock divisor */
        BSCD_DEFAULT_EXTERNAL_CLOCK_DIVISOR,

        /* Baud Divisor */
        0,

        /* Number of transmit parity retries per character in
            SC_UART_CMD_2 register. Default is 4 and max is 6.
            7 indicates infinite retries */
        BSCD_DEFAULT_TX_PARITY_RETRIES,

        /* Number of receive parity retries per character in
            SC_UART_CMD_2 register. Default is 4 and max is 6.
            7 indicates infinite retries */
        BSCD_DEFAULT_RX_PARITY_RETRIES,

        /* work waiting time in SC_TIME_CMD register. Other than EMV
            standard, only valid if current protocol is T=0. */
        {BSCD_DEFAULT_WORK_WAITING_TIME,   BSCD_TimerUnit_eETU},

        /* block Wait time in SC_TIME_CMD register. Only valid if
            current protocol is T=1. */
        {BSCD_DEFAULT_BLOCK_WAITING_TIME,   BSCD_TimerUnit_eETU},

        /* Extra Guard Time in SC_TGUARD register. */
        {BSCD_DEFAULT_EXTRA_GUARD_TIME,   BSCD_TimerUnit_eETU},

        /*  block Guard time in SC_BGT register.Other than EMV
            standard, only valid if current protocol is T=1.  */
        {BSCD_DEFAULT_BLOCK_GUARD_TIME,   BSCD_TimerUnit_eETU},

        /* character Wait time in SC_PROTO_CMD register. Only valid
            if current protocol is T=1. */
        BSCD_DEFAULT_CHARACTER_WAIT_TIME_INTEGER,

        /* EDC encoding. Only valid if current protocol is T=1. */
        {BSCD_EDCEncode_eLRC,   false},

        /* arbitrary Time Out value for any synchronous transaction. */
        {BSCD_DEFAULT_TIME_OUT,   BSCD_TimerUnit_eMilliSec},

        /* Specify if we need auto deactivation sequence */
        false,

        /* True if we receive 0x60 in T=0, we will ignore it.  Otherwise, we treat 0x60 as a valid data byte */
        false,

        /* Debounce info for IF_CMD_2 */
        {BSCD_ScPresMode_eMask, true, BSCD_DEFAULT_DB_WIDTH},

        BSCD_ResetCardAction_eReceiveAndDecode,   /* Specify if we want the driver to read, decode and program registers */

        {0,   BSCD_TimerUnit_eETU} , /* block wait time extension */
        true  /* pres is low */     ,
        {BSCD_MAX_ATR_START_IN_CLK_CYCLES, BSCD_TimerUnit_eCLK},
        false  , /* connect to TDA8024 */
        true,   /* pin setting */
        true,   /* VCC setting */
        BSCD_VccLevel_e5V ,      /* 5 Volts */
        BSCD_ClockFreq_e27MHZ,
        BSCD_MAX_RESET_IN_CLK_CYCLES,
        true,
        false,
        false,  /*digital interface */
};




/*******************************************************************************
*   Public Module Functions
*******************************************************************************/
BERR_Code BSCD_GetDefaultSettings(
        BSCD_Settings   *outp_sSettings,
        BCHP_Handle     in_chipHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_GetDefaultSettings);
    BDBG_ASSERT( in_chipHandle );

    *outp_sSettings = BSCD_defScdSettings;

    BSTD_UNUSED(in_chipHandle);

    BDBG_LEAVE(BSCD_GetDefaultSettings);
    return( errCode );
}


BERR_Code BSCD_Open(
        BSCD_Handle         *outp_handle,
        BREG_Handle         in_regHandle,
        BCHP_Handle         in_chipHandle,
            BINT_Handle         in_interruptHandle,
        const BSCD_Settings *inp_sSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BSCD_Handle moduleHandle;
    unsigned int channelNum;
    uint32_t ulVal = 0;

    BDBG_ENTER(BSCD_Open);
    BDBG_ASSERT( in_chipHandle );
    BDBG_ASSERT( in_regHandle );
    BDBG_ASSERT( in_interruptHandle );

    *outp_handle = NULL;

    /* on many chips, ulVal is actually used, but having an unconditional BSTD_UNUSED makes it easier to maintain this code. */
    BSTD_UNUSED(ulVal);

    /* Alloc memory from the system heap */
    if ((moduleHandle =
                (BSCD_Handle) BKNI_Malloc( sizeof( BSCD_P_Handle)))
                == NULL) {
        /* wrap initially detected error code */
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BSCD_P_DONE_LABEL;
    }

    BKNI_Memset(moduleHandle, 0, sizeof( BSCD_P_Handle ));

    moduleHandle->ulMagicNumber = BSCD_P_HANDLE_MAGIC_NUMBER;
    moduleHandle->chipHandle = in_chipHandle;
    moduleHandle->regHandle = in_regHandle;
    moduleHandle->interruptHandle = in_interruptHandle;

    if (inp_sSettings == NULL)
        moduleHandle->currentSettings = BSCD_defScdSettings;
    else {
        moduleHandle->currentSettings = *inp_sSettings;

    }

    /* Get the chip information for Bcm7038 */
  /*  BCHP_GetChipInfo( moduleHandle->chipHandle, &moduleHandle->chipId, &moduleHandle->chipRev );
    BDBG_MSG(( "chipId=%d, chipRev=%d\n", moduleHandle->chipId, moduleHandle->chipRev ));*/
/*** move clock setting  to channel OPen */

    /***************************************************************
        Warning:  Note that we have to modify the board to use GPIO and connect it
                 to pin 3 (3V/5V) of TDA chip and run this function.
                 Make sure to disconnect your QAM or    QPSK connection before
                 calling this function or your smartcard will be damaged.
    ***************************************************************/
/** move 3v?5V setting to channel Open */

    /*
        If inp_sSettings->maxChannels == 0, set it to
        BSCD_MAX_SUPPOTED_CHANNELS
    */
    if (moduleHandle->currentSettings.ucMaxChannels == 0)
        moduleHandle->currentSettings.ucMaxChannels =
                            BSCD_MAX_SUPPOTED_CHANNELS;

    for( channelNum = 0;
        channelNum < moduleHandle->currentSettings.ucMaxChannels;
        channelNum++ )
    {
        moduleHandle->channelHandles[channelNum] = NULL;
    }

    *outp_handle = moduleHandle;

    BKNI_EnterCriticalSection();
    moduleHandle->bIsOpen = true;
    BKNI_LeaveCriticalSection();

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Open);
    return( errCode );

}


BERR_Code BSCD_Close(
        BSCD_Handle inout_handle
)
{
    BERR_Code errCode = BERR_SUCCESS;
       /* 12/02/2006 QX: add this to avoid crash during BKNI_Free */
     void *pTemp;


    BDBG_ENTER(BSCD_Close);
    BDBG_ASSERT( inout_handle );

    BKNI_EnterCriticalSection();
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, (inout_handle ==  NULL) );
    BKNI_LeaveCriticalSection();

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (inout_handle->ulMagicNumber != BSCD_P_HANDLE_MAGIC_NUMBER ) );

    BKNI_EnterCriticalSection();
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (inout_handle->bIsOpen ==  false) );
    BKNI_LeaveCriticalSection();

BSCD_P_DONE_LABEL:
    BKNI_EnterCriticalSection();
    inout_handle->bIsOpen = false;
    BKNI_LeaveCriticalSection();

 /* 12/02/2006 QX: add this to avoid crash during BKNI_Free */
    BKNI_EnterCriticalSection();
    pTemp = inout_handle;
    inout_handle = NULL;
    BKNI_LeaveCriticalSection();

    BKNI_Free(pTemp);


    BDBG_LEAVE(BSCD_Close);
    return( errCode );
}


BERR_Code BSCD_GetTotalChannels(
        BSCD_Handle     in_handle,
        unsigned char       *outp_ucTotalChannels
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_GetTotalChannels);
    BDBG_ASSERT( in_handle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->ulMagicNumber != BSCD_P_HANDLE_MAGIC_NUMBER ) );

    *outp_ucTotalChannels = in_handle->currentSettings.ucMaxChannels;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_GetTotalChannels);
    return( errCode );
}


BERR_Code BSCD_GetChannelDefaultSettings(
        BSCD_Handle             in_handle,
        unsigned int            in_channelNo,
        BSCD_ChannelSettings    *outp_sSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;


    BDBG_ENTER(BSCD_GetChannelDefaultSettings);
    BDBG_ASSERT( in_handle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->ulMagicNumber != BSCD_P_HANDLE_MAGIC_NUMBER ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER,
        (in_channelNo >= in_handle->currentSettings.ucMaxChannels) );

    *outp_sSettings = BSCD_defScdChannelSettings;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_GetChannelDefaultSettings);
    return( errCode );
}


BERR_Code BSCD_Channel_Open(
        BSCD_Handle                 in_handle,
        BSCD_ChannelHandle          *outp_channelHandle,
        unsigned int                in_channelNo,
        const BSCD_ChannelSettings  *inp_channelDefSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BSCD_ChannelHandle channelHandle = NULL;
    uint32_t            ulVal = 0;

#if (BCHP_CHIP == 3548) || (BCHP_CHIP==3556)|| (BCHP_CHIP==35230)
    BINT_Id             intId = BCHP_INT_ID_CREATE(BCHP_SCIRQ0_SCIRQEN, BCHP_SCIRQ0_SCIRQEN_sca_irqen_SHIFT);

#else
    BINT_Id             intId = BCHP_INT_ID_sca_irqen;
#endif
#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
    BCHP_PWR_ResourceId resourceId = BCHP_PWR_RESOURCE_SMARTCARD0;
#ifdef BCHP_PWR_RESOURCE_SMARTCARD1
    if(in_channelNo == 1) {
        resourceId = BCHP_PWR_RESOURCE_SMARTCARD1;
    }
#endif
#endif

    BDBG_ENTER(BSCD_Channel_Open);
    BDBG_ASSERT( in_handle );
        BSTD_UNUSED(ulVal);

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->ulMagicNumber != BSCD_P_HANDLE_MAGIC_NUMBER ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER,
        (in_channelNo >= in_handle->currentSettings.ucMaxChannels) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->bIsOpen ==  false) );

    /* channel handle must be NULL.  */
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->channelHandles[in_channelNo]  != NULL) );

    *outp_channelHandle = NULL;

    /* Alloc memory from the system heap */
    if ((channelHandle =
            (BSCD_ChannelHandle) BKNI_Malloc(sizeof(BSCD_P_ChannelHandle)))
            == NULL) {
        /* wrap initially detected error code */
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BSCD_P_DONE_LABEL;
    }

    BKNI_Memset(channelHandle, 0, sizeof( BSCD_P_ChannelHandle ));

    channelHandle->ulMagicNumber = BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER;
    channelHandle->moduleHandle = in_handle;

    channelHandle->ucChannelNumber = in_channelNo;

#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
        /* acquire power */
        BCHP_PWR_AcquireResource(channelHandle->moduleHandle->chipHandle, resourceId);
#endif

#ifdef BSCD_EMV2000_CWT_PLUS_4
    channelHandle->bIsReceive = false;
#endif
    switch (in_channelNo) {
        case 0:
            channelHandle->ulRegStartAddr = BCHP_SCA_SC_UART_CMD_1;

            break;
        case 1:
            channelHandle->ulRegStartAddr = BCHP_SCB_SC_UART_CMD_1;

            break;
        default:
            channelHandle->ulRegStartAddr = BCHP_SCA_SC_UART_CMD_1;
            break;
    }
    BKNI_EnterCriticalSection();
    BREG_Write32(
        channelHandle->moduleHandle->regHandle,
        (channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_1),
         0);

    BREG_Write32(
        channelHandle->moduleHandle->regHandle,
        (channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_2),
         0);
    channelHandle->ulStatus1 = 0x00;
    channelHandle->ulStatus2 = 0x00;
    channelHandle->ulIntrStatus1 = 0x00;
    channelHandle->ulIntrStatus2 = 0x00;
    BKNI_LeaveCriticalSection();

    /*  Enable smartcard interrupt   */
    switch (in_channelNo) {
        case 0:

            intId = BCHP_INT_ID_sca_irqen;
            BREG_Write32( in_handle->regHandle, BCHP_SCIRQ0_SCIRQEN,
                    (BCHP_SCIRQ0_SCIRQEN_sca_irqen_MASK |
                      BREG_Read32(in_handle->regHandle,  BCHP_SCIRQ0_SCIRQEN)) );
            break;
        case 1:
            intId = BCHP_INT_ID_scb_irqen;
            BREG_Write32(  in_handle->regHandle, BCHP_SCIRQ0_SCIRQEN,
                    (BCHP_SCIRQ0_SCIRQEN_scb_irqen_MASK |
                      BREG_Read32(in_handle->regHandle,  BCHP_SCIRQ0_SCIRQEN)) );
            break;

    }


    BSCD_P_CHECK_ERR_CODE_FUNC( errCode, BINT_CreateCallback(
            &(channelHandle->channelIntCallback), in_handle->interruptHandle, intId,
            BSCD_Channel_P_IntHandler_isr, (void *) channelHandle, 0x00 ) );
    BSCD_P_CHECK_ERR_CODE_FUNC( errCode, BINT_EnableCallback(channelHandle->channelIntCallback) );


    BDBG_MSG(("in_channelNo = %d\n", in_channelNo));
    BDBG_MSG(("channelHandle->ulRegStartAddr = 0x%x\n", (unsigned int)channelHandle->ulRegStartAddr));

	if (inp_channelDefSettings != NULL){
                BSCD_Channel_P_SetSrcClock(
                        channelHandle,
                        inp_channelDefSettings);
        BSCD_Channel_SetParameters(
            channelHandle,
            inp_channelDefSettings);
                }
    else {
                BSCD_Channel_P_SetSrcClock(
                        channelHandle,
                        &BSCD_defScdChannelSettings);
        BSCD_Channel_SetParameters(
            channelHandle,
            &BSCD_defScdChannelSettings);
    }
        /* set pin muxing */
    if( channelHandle->currentChannelSettings.setPinmux){

        switch (in_channelNo) {
            case 0:

            /* Pin muxing */

#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)|| (BCHP_CHIP==7435)
    ulVal = BREG_Read32 (in_handle->regHandle,
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
                ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_091_MASK |
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_090_MASK|
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_089_MASK);
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,
                    (ulVal |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_091_SHIFT) |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_090_SHIFT) |

                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_089_SHIFT)   ));


            ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_093_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_092_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_093_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_092_SHIFT)));
            BDBG_MSG(( "7422, need to switch io pin\n" ));
#elif (BCHP_CHIP==7445)
                #if (BCHP_VER >= BCHP_VER_D0)
        ulVal = BREG_Read32 (in_handle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
                ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_053_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_052_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_051_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_050_MASK  );
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,
                        (ulVal |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_053_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_052_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_051_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_050_SHIFT) ));
                ulVal = BREG_Read32 (in_handle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
                ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_MASK;
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
                        (ulVal |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_SHIFT)));
        #else
        ulVal = BREG_Read32 (in_handle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_053_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_052_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_051_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_050_MASK  );
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
                        (ulVal |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_053_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_052_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_051_SHIFT) |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_050_SHIFT) ));
                ulVal = BREG_Read32 (in_handle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
                ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_049_MASK;
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
                        (ulVal |
                                (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_049_SHIFT)));

                #endif
                BDBG_MSG(( "7445, need to switch io pin\n" ));

#elif (BCHP_CHIP==7145)
                ulVal = BREG_Read32 (in_handle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
                ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_073_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_074_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_075_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_076_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_077_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_078_MASK );
                BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
                        (ulVal |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_073_SHIFT) |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_074_SHIFT) |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_075_SHIFT) |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_076_SHIFT) |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_077_SHIFT) |
                                (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_078_SHIFT ) ));
                BDBG_MSG(( "7145, need to switch io pin\n" ));

#elif ((BCHP_CHIP==7346) || (BCHP_CHIP==73465))
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_012_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_011_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_010_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_009_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_008_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_012_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_011_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_010_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_009_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_008_SHIFT ) ));
            BDBG_MSG(( "7346[5], need to switch io pin\n" ));
#elif(BCHP_CHIP==7231)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_81_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_80_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_79_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_81_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_80_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_79_SHIFT) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_83_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_82_MASK  );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_83_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_82_SHIFT) ));
            BDBG_MSG(( "7231, need to switch io pin\n" ));
#elif(BCHP_CHIP==7358)||(BCHP_CHIP==7552)||(BCHP_CHIP==7360)||(BCHP_CHIP==7584)||(BCHP_CHIP==75845)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625)||(BCHP_CHIP==75525)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_83_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_82_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_81_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_80_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_79_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_83_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_82_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_81_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_80_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_79_SHIFT ) ));
            BDBG_MSG(( "7358, need to switch io pin\n" ));
#elif(BCHP_CHIP==7344)
                                                ulVal = BREG_Read32 (in_handle->regHandle,
                                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
                                                ulVal &= ~(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_MASK |
                                                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_MASK |
                                                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_MASK  );
                                                BREG_Write32 (in_handle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0,
                                                        (ulVal |
                                                                (0x00000004 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_SHIFT) |
                                                                (0x00000004 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_SHIFT) |
                                                                (0x00000004 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_SHIFT)  ));
                                                ulVal = BREG_Read32 (in_handle->regHandle,
                                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                                                ulVal &= ~(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_05_MASK |
                                                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_04_MASK  );
                                                BREG_Write32 (in_handle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1,
                                                        (ulVal |
                                                                (0x00000004 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_05_SHIFT) |
                                                                (0x00000004 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_04_SHIFT)  ));
                                                BDBG_MSG(( "7344, need to switch io pin\n" ));
#elif(BCHP_CHIP==7429)||(BCHP_CHIP==7241)||(BCHP_CHIP==74295)
                                        ulVal = BREG_Read32 (in_handle->regHandle,
                                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_080_MASK |
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_081_MASK |
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_082_MASK |
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_083_MASK );
                                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                                                (ulVal |
                                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_080_SHIFT) |
                                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_081_SHIFT) |
                                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_082_SHIFT) |
                                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_083_SHIFT ) ));
                                        ulVal = BREG_Read32 (in_handle->regHandle,
                                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_079_MASK;
                                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                                                (ulVal |
                                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_079_SHIFT)));
#if 0
                                        ulVal = BREG_Read32 (in_handle->regHandle,
                                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
                                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_062_MASK |
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_063_MASK);
                                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,ulVal);
#endif
                                        BDBG_MSG(( "7429, need to switch io pin\n" ));

#elif ((BCHP_CHIP==7563) || (BCHP_CHIP==75635))
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_80_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_81_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_82_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_79_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_80_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_81_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_82_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_79_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_83_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_83_SHIFT)));
                        BDBG_MSG(( "7563, need to switch io pin\n" ));
#elif(BCHP_CHIP==7366)

#if (BCHP_VER > BCHP_VER_A0)
				if(!channelHandle->currentChannelSettings.bIsAnalogIntf){
						ulVal = BREG_Read32 (in_handle->regHandle,
						BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
						ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_058_MASK |
							BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_059_MASK |
							BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_060_MASK |
							BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_061_MASK |
							BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_062_MASK );
						BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
							(ulVal |
							(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_058_SHIFT) |
							(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_059_SHIFT) |
							(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_060_SHIFT) |
							(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_061_SHIFT) |
							(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_062_SHIFT ) ));
						}else{
							ulVal = BREG_Read32 (in_handle->regHandle,
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
							ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_058_MASK |
										BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_059_MASK |
										BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_060_MASK |
										BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_061_MASK |
										BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_062_MASK );
							BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,ulVal);

							ulVal = BREG_Read32 (in_handle->regHandle,
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
							ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_137_MASK |
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_134_MASK |
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_133_MASK |
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_132_MASK |
								BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_131_MASK );
							BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0,
								(ulVal |
								(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_137_SHIFT) |
								(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_134_SHIFT) |
								(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_133_SHIFT) |
								(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_132_SHIFT) |
								(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_131_SHIFT ) ));
						}
#else
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_085_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_084_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_083_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_082_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_085_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_084_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_083_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_082_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_086_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_086_SHIFT)));
#endif
                        BDBG_MSG(( "7366, need to switch io pin\n" ));
#elif(BCHP_CHIP==7439)
        #if(BCHP_VER < BCHP_VER_B0)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_059_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_060_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_061_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_062_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_059_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_060_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_061_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_062_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_058_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_058_SHIFT)));
        #else
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_050_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_051_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_052_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_053_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,
                                (ulVal |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_053_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_050_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_051_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_052_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
                                (ulVal |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_SHIFT)));
        #endif
                        BDBG_MSG(( "7439, need to switch io pin\n" ));

#elif (BCHP_CHIP == 7364)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_086_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_085_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_084_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_083_MASK |
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_082_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                                        (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_086_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_085_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_084_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_083_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_onoff_gpio_082_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
#elif (BCHP_CHIP == 7250)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_059_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_060_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_061_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_062_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_059_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_060_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_061_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_062_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_onoff_gpio_058_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_onoff_gpio_058_SHIFT)));

#endif
            break;
        case 1:
            intId = BCHP_INT_ID_scb_irqen;
            BREG_Write32(  in_handle->regHandle, BCHP_SCIRQ0_SCIRQEN,
                    (BCHP_SCIRQ0_SCIRQEN_scb_irqen_MASK |
                      BREG_Read32(in_handle->regHandle,  BCHP_SCIRQ0_SCIRQEN)) );

            /* Pin muxing */
#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)|| (BCHP_CHIP==7435)
            ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_098_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_097_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_096_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_095_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_094_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_098_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_097_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_096_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_095_SHIFT) |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_094_SHIFT ) ));
            BDBG_MSG(( "7422, need to switch io pin\n" ));
#elif (BCHP_CHIP==7445)
            #if (BCHP_VER >= BCHP_VER_D0)
                ulVal = BREG_Read32 (in_handle->regHandle,
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_089_MASK |
                                   BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_088_MASK |
                   BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_087_MASK|
                   BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_086_MASK);
        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18,
                    (ulVal |
                     (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_089_SHIFT) |
                     (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_088_SHIFT) |
                     (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_087_SHIFT) |
                                 (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_086_SHIFT)   ));


            ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_090_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19,
                (ulVal |(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_090_SHIFT)));
        #else
            ulVal = BREG_Read32 (in_handle->regHandle,
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_089_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_088_MASK |
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_087_MASK|
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_086_MASK);
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,
                    (ulVal |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_089_SHIFT) |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_088_SHIFT) |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_087_SHIFT) |
                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_086_SHIFT)   ));


            ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_090_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,
                (ulVal |
                    (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_090_SHIFT)));
                        #endif
            BDBG_MSG(( "7445, need to switch io pin\n" ));
#elif (BCHP_CHIP==7145)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_049_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_050_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_051_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_052_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_053_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_056_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_049_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_050_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_051_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_052_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_053_SHIFT) |
                                        (0x00000003 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_056_SHIFT ) ));
                        BDBG_MSG(( "7145, need to switch io pin\n" ));

#elif ((BCHP_CHIP==7346) || (BCHP_CHIP==73465))
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_028_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_027_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_026_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_025_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_028_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_027_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_026_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_025_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_029_MASK);
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_029_SHIFT)  ));
                        BDBG_MSG(( "7346[5], need to switch io pin\n" ));

#elif(BCHP_CHIP==7231)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_37_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_36_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_35_MASK |
                                                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_34_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
                (ulVal |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_37_SHIFT) |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_36_SHIFT) |
                     (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_35_SHIFT) |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_34_SHIFT) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_33_MASK  );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
                (ulVal |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_33_SHIFT) ));
            BDBG_MSG(( "7231, need to switch io pin\n" ));
#elif(BCHP_CHIP==7358)||(BCHP_CHIP==7552)||(BCHP_CHIP==7360)||(BCHP_CHIP==7584)||(BCHP_CHIP==75845)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625)||(BCHP_CHIP==75525)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_36_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_35_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_34_MASK |
                                                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_33_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
                (ulVal |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_33_SHIFT) |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_36_SHIFT) |
                     (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_35_SHIFT) |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_34_SHIFT) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_37_MASK  );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,
                (ulVal |
                    (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_37_SHIFT) ));
            BDBG_MSG(( "7358, need to switch io pin\n" ));
#elif(BCHP_CHIP==7344)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_56_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_55_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_54_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_53_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_52_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_56_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_55_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_54_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_53_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_52_SHIFT ) ));
                        BDBG_MSG(( "7344, need to switch io pin\n" ));

#elif(BCHP_CHIP==7429)||(BCHP_CHIP==7241)||(BCHP_CHIP==74295)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_037_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_036_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_035_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_034_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_033_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
                                (ulVal |
                                        (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_037_SHIFT) |
                                        (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_036_SHIFT) |
                                        (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_035_SHIFT) |
                                        (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_034_SHIFT) |
                                        (0x00000002 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_033_SHIFT ) ));
                        BDBG_MSG(( "7429, need to switch io pin\n" ));
#elif(BCHP_CHIP==7584)||(BCHP_CHIP==75845)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_100_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_99_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_98_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_97_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_96_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
                                (ulVal |
                                        (0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_100_SHIFT) |
                                        (0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_99_SHIFT) |
                                        (0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_98_SHIFT) |
                                        (0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_97_SHIFT) |
                                        (0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_96_SHIFT ) ));
                        BDBG_MSG(( "7584, need to switch io pin\n" ));
#elif((BCHP_CHIP==7563)|| (BCHP_CHIP==75635))
        BDBG_WRN(("SC1 is not found in 7563 RDB!\n"));
#elif(BCHP_CHIP==7366)

#if (BCHP_VER > BCHP_VER_A0)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
            ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_048_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_049_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_050_MASK |
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_051_MASK );
            BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                (ulVal |
                    (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_048_SHIFT) |
                    (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_049_SHIFT) |
                    (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_050_SHIFT) |
                    (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_051_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_047_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                                (ulVal |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_047_SHIFT)));
#else

                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_093_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_092_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_091_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_090_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_093_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_092_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_091_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_onoff_gpio_090_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_onoff_gpio_094_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_onoff_gpio_094_SHIFT)));
#endif
                        BDBG_MSG(( "7366, need to switch io pin\n" ));
#elif(BCHP_CHIP==7439)
        #if(BCHP_VER < BCHP_VER_B0)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_047_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_048_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_049_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_050_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
                                (ulVal |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_047_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_048_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_049_SHIFT) |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_050_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_051_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                                (ulVal |
                                        (0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_051_SHIFT)));
        #else
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
                        ulVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_087_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_088_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_089_MASK |
                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_086_MASK );
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_087_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_088_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_089_SHIFT) |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_086_SHIFT ) ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
                        ulVal &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_090_MASK;
                        BREG_Write32 (in_handle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19,
                                (ulVal |
                                        (0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_090_SHIFT)));
        #endif
                        BDBG_MSG(( "7439, need to switch io pin\n" ));
#elif (BCHP_CHIP == 7364)
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
                        ulVal &= ~(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_03_MASK |
                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_MASK |
                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_MASK |
                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_MASK  );
                        BREG_Write32 (in_handle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0,
                                (ulVal |
                                        (0x00000003 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_03_SHIFT) |
                                        (0x00000003 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_SHIFT) |
                                        (0x00000003 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_SHIFT) |
                                        (0x00000003 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_SHIFT)      ));
                        ulVal = BREG_Read32 (in_handle->regHandle,
                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                        ulVal &= ~(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_08_MASK  );
                        BREG_Write32 (in_handle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1,
                                (ulVal |
                                        (0x00000003 << BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_08_SHIFT)      ));
                        BDBG_MSG(( "7364, need to switch io pin\n" ));


#endif
            break;

        default:
            errCode = BERR_INVALID_PARAMETER;
            goto BSCD_P_DONE_LABEL;
    }
    }

#ifdef SMARTCARD_32_BIT_REGISTER
	if(channelHandle->currentChannelSettings.bIsAnalogIntf){
		ulVal =  BREG_Read32(in_handle->regHandle,
					(channelHandle->ulRegStartAddr + BSCD_P_AFE_CMD_2)) ;
		ulVal |= BCHP_SCA_AFE_CMD_2_power_dn_MASK|BCHP_SCA_AFE_CMD_2_bp_modeb_MASK|BCHP_SCA_AFE_CMD_2_poradj_latch_rst_MASK;
		BREG_Write32(in_handle->regHandle,(channelHandle->ulRegStartAddr + BSCD_P_AFE_CMD_2), ulVal);
		ulVal &=~BCHP_SCA_AFE_CMD_2_power_dn_MASK;
		ulVal &=~BCHP_SCA_AFE_CMD_2_poradj_latch_rst_MASK;
		BREG_Write32(in_handle->regHandle,(channelHandle->ulRegStartAddr + BSCD_P_AFE_CMD_2), ulVal);
		BKNI_Sleep(10);
	}
#endif
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.cardWait)));
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.tdoneWait)));
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.rcvWait)));
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.atrStart)));
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.timerWait)));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.event1Wait)));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BKNI_CreateEvent( &(channelHandle->channelWaitEvent.event2Wait)));

    /*Set VCC level to inp_channelDefSettings->vcc*/
    if(  channelHandle->currentChannelSettings.setVcc){
        BSCD_Channel_SetVccLevel( channelHandle,channelHandle->currentChannelSettings.vcc);
    }
    in_handle->channelHandles[in_channelNo] = channelHandle;

    *outp_channelHandle = channelHandle;
    BKNI_EnterCriticalSection();
    channelHandle->bIsOpen = true;
    BKNI_LeaveCriticalSection();
	ulVal = BREG_Read32(in_handle->regHandle, (channelHandle->ulRegStartAddr + BSCD_P_STATUS_1));

	if(ulVal&BCHP_SCA_SC_STATUS_1_card_pres_MASK)
		channelHandle->channelStatus.bCardPresent = true;

	BKNI_EnterCriticalSection();
    BSCD_Channel_P_EnableInterrupts_isr(channelHandle);   /* Todo:  Only enable intr for ATR */
    BKNI_LeaveCriticalSection();
BSCD_P_DONE_LABEL:
    if( errCode != BERR_SUCCESS )
    {
        if( channelHandle != NULL )
        {
            if (channelHandle->channelWaitEvent.cardWait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.cardWait );
            if (channelHandle->channelWaitEvent.tdoneWait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.tdoneWait );
            if (channelHandle->channelWaitEvent.rcvWait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.rcvWait );
            if (channelHandle->channelWaitEvent.atrStart != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.atrStart );
            if (channelHandle->channelWaitEvent.timerWait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.timerWait );
            if (channelHandle->channelWaitEvent.event1Wait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.event1Wait );
            if (channelHandle->channelWaitEvent.event2Wait != NULL)
                BKNI_DestroyEvent( channelHandle->channelWaitEvent.event2Wait );

#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
            /* a failed open releases power */
            BCHP_PWR_ReleaseResource(channelHandle->moduleHandle->chipHandle, resourceId);
#endif

            BKNI_Free( channelHandle );

        }
    }

    BDBG_LEAVE(BSCD_Channel_Open);
    return( errCode );
}

BERR_Code BSCD_Channel_Close(
        BSCD_ChannelHandle inout_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BSCD_Handle moduleHandle;
    uint32_t ulValue = 0;
       /* 12/02/2006 QX: add this to avoid crash during BKNI_Free */
     void *pTemp;
#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
    BCHP_PWR_ResourceId resourceId = BCHP_PWR_RESOURCE_SMARTCARD0;
#ifdef BCHP_PWR_RESOURCE_SMARTCARD1
    if(inout_channelHandle->ucChannelNumber == 1) {
        resourceId = BCHP_PWR_RESOURCE_SMARTCARD1;
    }
#endif
#endif

    BDBG_ENTER(BSCD_Channel_Close);
    BDBG_ASSERT( inout_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (inout_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (inout_channelHandle->bIsOpen ==  false) );


BSCD_P_DONE_LABEL:

    inout_channelHandle->bIsOpen = false;

    ulValue = BREG_Read32(inout_channelHandle->moduleHandle->regHandle,  BCHP_SCIRQ0_SCIRQEN);
    /*  Disable smartcard interrupt   */
    switch (inout_channelHandle->ucChannelNumber) {
        case 0:
            ulValue &= ~BCHP_SCIRQ0_SCIRQEN_sca_irqen_MASK;
            break;

        case 1:
            ulValue &= ~BCHP_SCIRQ0_SCIRQEN_scb_irqen_MASK;
            break;


        default:
            errCode = BERR_INVALID_PARAMETER;
            break;
    }


    BREG_Write32(inout_channelHandle->moduleHandle->regHandle,  BCHP_SCIRQ0_SCIRQEN,
                    ulValue);
    BSCD_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DisableCallback( inout_channelHandle->channelIntCallback));
    BSCD_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DestroyCallback( inout_channelHandle->channelIntCallback ) );

    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.cardWait );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.tdoneWait );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.rcvWait );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.atrStart );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.timerWait );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.event1Wait );
    BKNI_DestroyEvent( inout_channelHandle->channelWaitEvent.event2Wait );

    BSCD_Channel_Deactivate(inout_channelHandle);

    moduleHandle = inout_channelHandle->moduleHandle;
    moduleHandle->channelHandles[inout_channelHandle->ucChannelNumber] = NULL;

#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
        /* release power */
        BCHP_PWR_ReleaseResource(inout_channelHandle->moduleHandle->chipHandle, resourceId);
#endif

     /* 12/02/2006 QX: add this to avoid crash during BKNI_Free */
    BKNI_EnterCriticalSection();
    pTemp = inout_channelHandle;
    inout_channelHandle = NULL;
    BKNI_LeaveCriticalSection();
    BKNI_Free(pTemp);

    BDBG_LEAVE(BSCD_Channel_Close);
    return( errCode );
}

BERR_Code BSCD_Channel_GetDevice(
        BSCD_ChannelHandle  in_channelHandle,
        BSCD_Handle         *outp_handle
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_GetDevice);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->moduleHandle->bIsOpen ==  false) );

    *outp_handle = NULL;

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    *outp_handle = in_channelHandle->moduleHandle;

BSCD_P_DONE_LABEL:
    BDBG_LEAVE(BSCD_Channel_GetDevice);
    return( errCode );
}


BERR_Code BSCD_GetChannel(
        BSCD_Handle         in_handle,
        unsigned int        in_channelNo,
        BSCD_ChannelHandle  *outp_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BSCD_ChannelHandle channelHandle = NULL;

    BDBG_ENTER(BSCD_GetChannel);
    BDBG_ASSERT( in_handle );

    *outp_channelHandle = NULL;
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_handle->ulMagicNumber != BSCD_P_HANDLE_MAGIC_NUMBER ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER,
        (in_channelNo >= in_handle->currentSettings.ucMaxChannels) );

    channelHandle = in_handle->channelHandles[in_channelNo];

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (channelHandle == NULL ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );


    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (channelHandle->bIsOpen ==  false) );

    *outp_channelHandle = channelHandle;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_GetChannel);
    return( errCode );
}


BERR_Code BSCD_Channel_DetectCard(
        BSCD_ChannelHandle  in_channelHandle,
        BSCD_CardPresent    in_eCardPresent
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_DetectCard);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->bIsOpen ==  false) );

    switch (in_eCardPresent) {
        case BSCD_CardPresent_eInserted:
        {
            BKNI_EnterCriticalSection();

            if ( in_channelHandle->ulStatus1 & BCHP_SCA_SC_STATUS_1_card_pres_MASK) {
                in_channelHandle->channelStatus.bCardPresent = true;
                BKNI_LeaveCriticalSection();
                goto BSCD_P_DONE_LABEL;
            }
            else {
                BDBG_MSG(("SmartCard Not Present"));
                BDBG_MSG(("Please insert the SmartCard"));
            }
            BKNI_LeaveCriticalSection();


            BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                BSCD_Channel_P_WaitForCardInsertion(in_channelHandle));
        }
        break;

        case BSCD_CardPresent_eRemoved:
        {
            BKNI_EnterCriticalSection();

            if ( !(in_channelHandle->ulStatus1 & BCHP_SCA_SC_STATUS_1_card_pres_MASK)) {
                in_channelHandle->channelStatus.bCardPresent = false;
                BKNI_LeaveCriticalSection();
                goto BSCD_P_DONE_LABEL;
            }
            else {
                BDBG_MSG(("SmartCard Present"));
                BDBG_MSG(("Please remove the SmartCard"));
            }
            BKNI_LeaveCriticalSection();

            BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                BSCD_Channel_P_WaitForCardRemove(in_channelHandle));
        }
        break;
    }

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_DetectCard);
    return( errCode );

}



BERR_Code BSCD_Channel_SetParameters(
        BSCD_ChannelHandle          in_channelHandle,
        const BSCD_ChannelSettings  *inp_sSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t        ulValue = 0;

    BDBG_ENTER(BSCD_Channel_SetParameters);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    in_channelHandle->currentChannelSettings.bIsPresHigh = inp_sSettings->bIsPresHigh;
     in_channelHandle->currentChannelSettings.setPinmux = inp_sSettings->setPinmux;
     in_channelHandle->currentChannelSettings.setVcc = inp_sSettings->setVcc;
     in_channelHandle->currentChannelSettings.vcc = inp_sSettings->vcc;
     in_channelHandle->currentChannelSettings.srcClkFreqInHz = inp_sSettings->srcClkFreqInHz;
         in_channelHandle->currentChannelSettings.eResetCycles =inp_sSettings->eResetCycles;

     in_channelHandle->currentChannelSettings.bDirectRstInverted =inp_sSettings->bDirectRstInverted;
     in_channelHandle->currentChannelSettings.bDirectVccInverted =inp_sSettings->bDirectVccInverted;
	 in_channelHandle->currentChannelSettings.bIsAnalogIntf =inp_sSettings->bIsAnalogIntf;

        if(inp_sSettings->srcClkFreqInHz !=in_channelHandle->moduleHandle->currentSettings.moduleClkFreq.ulClkFreq){
                BDBG_MSG(("changing the clock frequency...\n"));

     in_channelHandle->moduleHandle->currentSettings.moduleClkFreq.ulClkFreq =  inp_sSettings->srcClkFreqInHz;
	/*
    switch(inp_sSettings->srcClkFreqInHz){
        case 27000000:
            in_channelHandle->currentChannelSettings.eSrcClkFreq = BSCD_ClockFreq_e27MHZ;
            break;
        case 36864000:
            in_channelHandle->currentChannelSettings.eSrcClkFreq = BSCD_ClockFreq_e36P864MHZ;
            break;
        case 36000000:
            in_channelHandle->currentChannelSettings.eSrcClkFreq = BSCD_ClockFreq_e36MHZ;
            break;
        case 24000000:
            in_channelHandle->currentChannelSettings.eSrcClkFreq = BSCD_ClockFreq_e24MHZ;
            break;
        default:
            BDBG_ERR(("Unknown SC clock freq %d!\n",inp_sSettings->srcClkFreqInHz ));
            break;

    }*/
                BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetSrcClock(
                        in_channelHandle, inp_sSettings));


         }


    BDBG_MSG(("bIsPresHigh = %d", in_channelHandle->currentChannelSettings.bIsPresHigh));
        ulValue =  BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;


    if(in_channelHandle->currentChannelSettings.bIsPresHigh)
    {
		ulValue =  BCHP_SCA_SC_IF_CMD_1_pres_pol_MASK | BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
	}else{
		ulValue =  ~BCHP_SCA_SC_IF_CMD_1_pres_pol_MASK & BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
	}
	BKNI_EnterCriticalSection();
	BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
             ulValue);
	BKNI_LeaveCriticalSection();

    /*  Smart Card Standard */
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        ((inp_sSettings->scStandard <= BSCD_Standard_eUnknown)  || (inp_sSettings->scStandard >= BSCD_Standard_eMax)) );
    in_channelHandle->currentChannelSettings.scStandard =   inp_sSettings->scStandard;
    BDBG_MSG(("scStandard = %d", in_channelHandle->currentChannelSettings.scStandard));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetStandard(
        in_channelHandle, inp_sSettings));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetFreq(
        in_channelHandle, inp_sSettings));

    /* Set maximum IFSD */
    in_channelHandle->currentChannelSettings.unMaxIFSD =  BSCD_MAX_TX_SIZE ;
    BDBG_MSG(("unMaxIFSD = %d", (unsigned int)in_channelHandle->currentChannelSettings.unMaxIFSD));

    /* Set current IFSD */
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
                (inp_sSettings->unCurrentIFSD > BSCD_MAX_TX_SIZE));
    if (inp_sSettings->unMaxIFSD == 0) {
        in_channelHandle->currentChannelSettings.unCurrentIFSD =  BSCD_MAX_TX_SIZE ;
    }
    else {
        in_channelHandle->currentChannelSettings.unCurrentIFSD =  inp_sSettings->unCurrentIFSD ;
    }
    BDBG_MSG(("unCurrentIFSD = %d",(unsigned int) in_channelHandle->currentChannelSettings.unCurrentIFSD));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetEdcParity(
        in_channelHandle, inp_sSettings));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetWaitTime(
        in_channelHandle, inp_sSettings));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetGuardTime(
        in_channelHandle, inp_sSettings));

    /* Set transaction time out */
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode, BSCD_Channel_P_SetTransactionTimeout(
        in_channelHandle, inp_sSettings));

    /* auto deactivation sequence */
    in_channelHandle->currentChannelSettings.bAutoDeactiveReq =  inp_sSettings->bAutoDeactiveReq;
    BDBG_MSG(("bAutoDeactiveReq = %d", in_channelHandle->currentChannelSettings.bAutoDeactiveReq));

    /* nullFilter */
    in_channelHandle->currentChannelSettings.bNullFilter =  inp_sSettings->bNullFilter;
    BDBG_MSG(("bNullFilter = %d", in_channelHandle->currentChannelSettings.bNullFilter));

    /* connectDirectly */
        in_channelHandle->currentChannelSettings.bConnectDirectly =  inp_sSettings->bConnectDirectly;
        BDBG_MSG(("bConnectDirectly = %d", in_channelHandle->currentChannelSettings.bConnectDirectly));


    /* debounce info */
    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
                (inp_sSettings->scPresDbInfo.ucDbWidth > BSCD_MAX_DB_WIDTH ));
    in_channelHandle->currentChannelSettings.scPresDbInfo =  inp_sSettings->scPresDbInfo;
    BDBG_MSG(("scPresDbInfo.bIsEnabled = %d", in_channelHandle->currentChannelSettings.scPresDbInfo.bIsEnabled));
    BDBG_MSG(("scPresDbInfo.ucDbWidth = %d", in_channelHandle->currentChannelSettings.scPresDbInfo.ucDbWidth));
    BDBG_MSG(("scPresDbInfo.scPresMode = %d", in_channelHandle->currentChannelSettings.scPresDbInfo.scPresMode));

    /* Specify if we want the driver to read, decode and program registers */
    in_channelHandle->currentChannelSettings.resetCardAction = inp_sSettings->resetCardAction;
    BDBG_MSG(("resetCardAction = %d", in_channelHandle->currentChannelSettings.resetCardAction));


    in_channelHandle->currentChannelSettings.ATRRecvTimeInteger =  inp_sSettings->ATRRecvTimeInteger;

    /* Update the BSCD_P_PRESCALE */
    ulValue = BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PRESCALE));
    BDBG_MSG(("orig BSCD_P_PRESCALE = 0x%x\n", ulValue));



    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PRESCALE),
        in_channelHandle->currentChannelSettings.unPrescale);
    BDBG_MSG(("New BSCD_P_PRESCALE = 0x%x\n", (unsigned int)in_channelHandle->currentChannelSettings.unPrescale));

#if 1
    /* Don't enable clock here since auto_clk need to be set first in ResetIFD before
         clock enabling for auto_deactivation */
    ulValue = BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD));
    BDBG_MSG(("orig ucClkCmd = 0x%x\n",  ulValue));

    /* If enabled before, change the the value.  Otherwise leave it intact. */
    ulValue = ulValue & BCHP_SCA_SC_CLK_CMD_clk_en_MASK;
    if (ulValue == BCHP_SCA_SC_CLK_CMD_clk_en_MASK) {

        ulValue = ulValue | (BSCD_P_MapScClkDivToMaskValue(in_channelHandle->currentChannelSettings.ucScClkDiv))  |
                ((in_channelHandle->currentChannelSettings.ucEtuClkDiv - 1) << 1)  |
                ((in_channelHandle->currentChannelSettings.ucBaudDiv == 31) ? 0 : 1);

        BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD),
            ulValue);
        if(in_channelHandle->currentChannelSettings.ucBaudDiv == 25){
                ulValue = BREG_Read32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_FLOW_CMD));
                ulValue = 0x80 |ulValue;
                BREG_Write32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_FLOW_CMD),
                    ulValue);
        }
        BDBG_MSG(("New SC_CLK_CMD = 0x%x\n", ulValue));
    }
#endif

    BDBG_MSG(("address  = 0x%x\n", (unsigned int)in_channelHandle->ulRegStartAddr));
    BDBG_MSG(("BSCD_P_UART_CMD_2 address  = 0x%x\n", (unsigned int)(in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2)));
    /* Update the BSCD_P_UART_CMD_2 */
    ulValue = BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2));
    BDBG_MSG(("orig BSCD_P_UART_CMD_2 = 0x%x\n",    ulValue));

#ifndef SMARTCARD_32_BIT_REGISTER
        ulValue  &=  (BCHP_SCA_SC_UART_CMD_2_convention_MASK);
#else
        ulValue &= ((BCHP_SCA_SC_UART_CMD_2_convention_MASK)|0x00FF);
#endif

    if (inp_sSettings->eProtocolType == BSCD_AsyncProtocolType_e0 ) {
#ifndef SMARTCARD_32_BIT_REGISTER
        ulValue |= (in_channelHandle->currentChannelSettings.ucRxRetries << BCHP_SCA_SC_UART_CMD_2_rpar_retry_SHIFT) |
                (in_channelHandle->currentChannelSettings.ucTxRetries);
#else
                ulValue |= (in_channelHandle->currentChannelSettings.ucRxRetries << BCHP_SCA_SC_UART_CMD_2_rpar_retry_SHIFT) |
                (in_channelHandle->currentChannelSettings.ucTxRetries)<<BCHP_SCA_UART_CMD_tpar_retry_SHIFT;
#endif
    }
    else if ( (inp_sSettings->eProtocolType == BSCD_AsyncProtocolType_e1 )  ||
            (inp_sSettings->eProtocolType == BSCD_AsyncProtocolType_e14_IRDETO ) ) {
        /* No OP */ ;
    }

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2),
        ulValue);

    BDBG_MSG(("BSCD_P_UART_CMD_2 = 0x%x\n",     ulValue));

    /* Update the BSCD_P_PROTO_CMD */
    ulValue =  BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD));
    if ((inp_sSettings->eProtocolType == BSCD_AsyncProtocolType_e1 ) &&
            (in_channelHandle->currentChannelSettings.edcSetting.bIsEnabled))  {
        ulValue =  BCHP_SCA_SC_PROTO_CMD_edc_en_MASK;

        if (in_channelHandle->currentChannelSettings.edcSetting.edcEncode == BSCD_EDCEncode_eLRC ) {
            ulValue &=  ~BCHP_SCA_SC_PROTO_CMD_crc_lrc_MASK;
        }
        else if (in_channelHandle->currentChannelSettings.edcSetting.edcEncode == BSCD_EDCEncode_eCRC) {
            ulValue |=  BCHP_SCA_SC_PROTO_CMD_crc_lrc_MASK;
        }
    }
    else {
        ulValue &=  ~BCHP_SCA_SC_PROTO_CMD_edc_en_MASK;
    }

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD),
        ulValue);

    /* Update the BSCD_P_FLOW_CMD */
    ulValue = 0;
        /* flow control in enabled in TX/RV function, do not enable here */
        /*
    if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eNDS) {
        ulValue =  BCHP_SCA_SC_FLOW_CMD_flow_en_MASK;
    }
    else {
        ulValue &=  ~BCHP_SCA_SC_FLOW_CMD_flow_en_MASK;
    }

*/
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_FLOW_CMD),
        ulValue);


    /* Update the BSCD_P_IF_CMD_2 */
        #ifndef SMARTCARD_32_BIT_REGISTER
        ulValue = 0;
        #else
        ulValue =       BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2));
        #endif
    if (in_channelHandle->currentChannelSettings.scPresDbInfo.bIsEnabled == true) {
        ulValue |=  BCHP_SCA_SC_IF_CMD_2_db_en_MASK;
    }
    else {
        ulValue &=  ~BCHP_SCA_SC_IF_CMD_2_db_en_MASK;
    }

    if (in_channelHandle->currentChannelSettings.scPresDbInfo.scPresMode == BSCD_ScPresMode_eMask) {
        ulValue |= BCHP_SCA_SC_IF_CMD_2_db_mask_MASK;
    }
    else if (in_channelHandle->currentChannelSettings.scPresDbInfo.scPresMode == BSCD_ScPresMode_eDebounce) {
        ulValue &= ~BCHP_SCA_SC_IF_CMD_2_db_mask_MASK;
    }
#ifndef SMARTCARD_32_BIT_REGISTER
        ulValue |= ( in_channelHandle->currentChannelSettings.scPresDbInfo.ucDbWidth);
#else
        ulValue |= ( in_channelHandle->currentChannelSettings.scPresDbInfo.ucDbWidth<<BCHP_SCA_IF_CMD_db_width_SHIFT );
#endif
#ifdef SMARTCARD_32_BIT_REGISTER
        if(inp_sSettings->srcClkFreqInHz == 108000000)
                BREG_Write32(in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IOIF_TICK),0x87);
        else
                BREG_Write32(in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IOIF_TICK),0x2F);
/* wait for debounce take effect */
        BKNI_EnterCriticalSection();
#endif
         BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2),
        ulValue);
#ifdef SMARTCARD_32_BIT_REGISTER
        BKNI_Delay(20);
        BKNI_LeaveCriticalSection();
#endif

    /* Update the BSCD_P_TGUARD */
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_TGUARD),
        in_channelHandle->currentChannelSettings.extraGuardTime.ulValue);

    if( inp_sSettings->setVcc){
        BSCD_Channel_SetVccLevel( in_channelHandle,inp_sSettings->vcc);
    }

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_SetParameters);
    return( errCode );
}


BERR_Code BSCD_Channel_GetParameters(
        BSCD_ChannelHandle  in_channelHandle,
        BSCD_ChannelSettings    *outp_sSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_GetParameters);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    *outp_sSettings = in_channelHandle->currentChannelSettings;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_GetParameters);
    return( errCode );
}

char    BSCD_Channel_GetChannelNumber(
        BSCD_ChannelHandle  in_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_GetChannelNumber);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_GetChannelNumber);
    if (errCode == BERR_SUCCESS)
        return( in_channelHandle->ucChannelNumber );
    else
        return  -1;
}


BERR_Code BSCD_Channel_Deactivate(
        BSCD_ChannelHandle          in_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    ulValue;

    BDBG_ENTER(BSCD_Channel_Deactivate);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    BDBG_MSG(("In BSCD_Channel_Deactivate\n"));
    /* Disable all interrupts */
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_1), 0);

        #ifndef SMARTCARD_32_BIT_REGISTER
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_2), 0);
        #endif

        if(in_channelHandle->currentChannelSettings.bConnectDirectly == true){
          /* Turn off VCC */
          ulValue =  BREG_Read32(
              in_channelHandle->moduleHandle->regHandle,
              (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
          if(in_channelHandle->currentChannelSettings.bDirectVccInverted == true)
        ulValue |= BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
          else
          ulValue &= ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
        ulValue |= BCHP_SCA_SC_IF_CMD_1_io_MASK;
    }else{
    /* Turn off VCC */
    ulValue =  BCHP_SCA_SC_IF_CMD_1_vcc_MASK | BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
        }
        ulValue &= ~BCHP_SCA_SC_IF_CMD_1_auto_vcc_MASK;
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
         ulValue);

		BKNI_Delay(20);
        if(in_channelHandle->currentChannelSettings.bConnectDirectly == true){
          /* Set RST = 0.     */
                     if(in_channelHandle->currentChannelSettings.bDirectRstInverted ==true)
          ulValue =  BCHP_SCA_SC_IF_CMD_1_rst_MASK | BREG_Read32(
              in_channelHandle->moduleHandle->regHandle,
              (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
                         else
                                ulValue =  (~BCHP_SCA_SC_IF_CMD_1_rst_MASK) & BREG_Read32(
                in_channelHandle->moduleHandle->regHandle,
                 (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
        }
        else

        {
    /* Set RST = 0.     */
    ulValue =  (~BCHP_SCA_SC_IF_CMD_1_rst_MASK) & BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
        }

        ulValue &= ~BCHP_SCA_SC_IF_CMD_1_auto_rst_MASK;
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
         ulValue);

    /* Set CLK = 0.      */
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD),
         0);
#if 0
    /* Set IO = 0.     This will cause IO line in an unknown state in wamr reset, comment out temparily */
    ulValue =  (BCHP_SCA_SC_IF_CMD_1_io_MASK) | BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
         ulValue);
#endif
    /* Reset Tx & Rx buffers.   */
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1),
         ~BCHP_SCA_SC_UART_CMD_1_io_en_MASK );

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD),
         BCHP_SCA_SC_PROTO_CMD_rbuf_rst_MASK | BCHP_SCA_SC_PROTO_CMD_tbuf_rst_MASK);

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_Deactivate);
    return( errCode );
}


BERR_Code BSCD_Channel_ResetIFD(
        BSCD_ChannelHandle  in_channelHandle,
        BSCD_ResetType      in_resetType

)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    ulIFCmdVal = 0, ulValue;
    BSCD_Timer      timer = {BSCD_TimerType_eGPTimer, {BSCD_GPTimerMode_eIMMEDIATE}, true, true};
    BSCD_TimerValue    timeValue= {2, BSCD_TimerUnit_eETU};


    BDBG_ENTER(BSCD_Channel_ResetIFD);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    /* Reset all status */
    in_channelHandle->ulStatus1 = 0;
    in_channelHandle->ulStatus2 = 0;
    in_channelHandle->ulIntrStatus1= 0;
    in_channelHandle->ulIntrStatus2= 0;

    in_channelHandle->channelStatus.ulStatus1 = 0;

      if (in_resetType == BSCD_ResetType_eCold) {

    in_channelHandle->channelStatus.bCardPresent = false;

    /* 09/20/05,Allen.C, reset bIsCardRemoved after card removed and reinitialize*/
    in_channelHandle->bIsCardRemoved = false;

       }
    /* Reset some critical registers */
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CMD),
         0);

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_1),
         0);
        #ifndef SMARTCARD_32_BIT_REGISTER
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_2),
         0);
        #endif
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1),
         0);
        #ifndef SMARTCARD_32_BIT_REGISTER
    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2),
         0);
        #endif
    /* Set up debounce filter */
        #ifndef SMARTCARD_32_BIT_REGISTER
        ulValue = 0;
        #else
        ulValue =       BREG_Read32(
                in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2));
        #endif

    if (in_channelHandle->currentChannelSettings.scPresDbInfo.bIsEnabled == true) {


        ulValue |= BCHP_SCA_SC_IF_CMD_2_db_en_MASK;

        if (in_channelHandle->currentChannelSettings.scPresDbInfo.scPresMode == BSCD_ScPresMode_eMask) {
            ulValue |= BCHP_SCA_SC_IF_CMD_2_db_mask_MASK;
        }

#ifndef SMARTCARD_32_BIT_REGISTER
                ulValue |= ( in_channelHandle->currentChannelSettings.scPresDbInfo.ucDbWidth);
#else
                ulValue |= ( in_channelHandle->currentChannelSettings.scPresDbInfo.ucDbWidth<<BCHP_SCA_IF_CMD_db_width_SHIFT );
#endif
#ifdef SMARTCARD_32_BIT_REGISTER
                BREG_Write32(in_channelHandle->moduleHandle->regHandle,
                                (in_channelHandle->ulRegStartAddr + BSCD_P_IOIF_TICK),0x2F);
#endif
#ifndef SMARTCARD_32_BIT_REGISTER
                BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2), ulValue);
#endif
    }
    else {
#ifndef SMARTCARD_32_BIT_REGISTER
                BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2), 0);
#endif
        }

    BDBG_MSG(("Inside Reset: Before Cold Reset ulIFCmdVal = 0x%x\n", ulIFCmdVal));
    /* Cold Reset or Warm Reset */
    if (in_resetType == BSCD_ResetType_eCold) {
        BDBG_MSG(("Cold Reset\n"));
        in_channelHandle->resetType = BSCD_ResetType_eCold;  /* Cold Reset */

		if(in_channelHandle->currentChannelSettings.bConnectDirectly == true){
#ifdef BSCD_DSS_ICAM
			if(in_channelHandle->currentChannelSettings.bDirectRstInverted ==true)
				ulIFCmdVal = 0;
            else
                ulIFCmdVal = BCHP_SCA_SC_IF_CMD_1_rst_MASK; /*VCC L, RST H*/
            #ifdef SMARTCARD_32_BIT_REGISTER
            ulIFCmdVal |=ulValue;
            #endif
            if(in_channelHandle->currentChannelSettings.bDirectVccInverted ==true)
				ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
            else
                ulIFCmdVal &= ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK; /*VCC L, RST H*/
#else
		if(in_channelHandle->currentChannelSettings.bDirectRstInverted ==true)
			ulIFCmdVal = BCHP_SCA_SC_IF_CMD_1_rst_MASK; /*VCC L, RST L*/
        else
			ulIFCmdVal = 0; /*VCC L, RST L*/
        #ifdef SMARTCARD_32_BIT_REGISTER
        ulIFCmdVal |=ulValue;
        #endif
        if(in_channelHandle->currentChannelSettings.bDirectVccInverted ==true)
			ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
        else
            ulIFCmdVal &= ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK; /*VCC L, RST L*/
        ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_io_MASK;
#endif
		}
		else{
#ifndef SMARTCARD_32_BIT_REGISTER
#ifdef BSCD_DSS_ICAM
			ulIFCmdVal = BCHP_SCA_SC_IF_CMD_1_vcc_MASK | BCHP_SCA_SC_IF_CMD_1_rst_MASK;
#else
			ulIFCmdVal = BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
#endif
#else
#ifdef BSCD_DSS_ICAM
			ulIFCmdVal = ulValue|BCHP_SCA_SC_IF_CMD_1_vcc_MASK | BCHP_SCA_SC_IF_CMD_1_rst_MASK;
#else
			ulIFCmdVal = ulValue| BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
#endif
#endif
		}
		BREG_Write32(
				in_channelHandle->moduleHandle->regHandle,
				(in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
				ulIFCmdVal);
    /* waiting for TDA8024 to fully deactivate*/
    if(in_channelHandle->currentChannelSettings.bConnectDirectly == false) BKNI_Delay(20);

                 /*Disable CLK*/
        BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD),
         0);

    if(in_channelHandle->currentChannelSettings.bIsPresHigh){
        BDBG_MSG(("Change Presence Polarity\n"));

        ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_pres_pol_MASK;
        BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
             ulIFCmdVal);
            BDBG_MSG(("Finished Change Presence Polarity\n"));
        }


    }
    else {
        BDBG_MSG(("Warm Reset\n"));
        in_channelHandle->resetType = BSCD_ResetType_eWarm;  /* Warm Reset */

        ulIFCmdVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1));
    }

    BDBG_MSG(("Inside Reset: After Cold Reset ulIFCmdVal = 0x%x\n", ulIFCmdVal));
	ulValue = BREG_Read32(
				in_channelHandle->moduleHandle->regHandle,
				(in_channelHandle->ulRegStartAddr + BSCD_P_STATUS_1));
	BDBG_MSG(("Inside Reset: After Cold Reset STATUS 1 = 0x%x\n", ulValue));
    /* Use Auto Deactivation instead of TDA8004 */
    if (in_channelHandle->currentChannelSettings.bAutoDeactiveReq == true) {

        BDBG_MSG(("Inside Reset: Before auto clk  BSCD_P_CLK_CMD = 0x%x\n",
                BREG_Read32(
                in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD))));

        ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_auto_clk_MASK;
        BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
             ulIFCmdVal);
    }


if(in_channelHandle->currentChannelSettings.bConnectDirectly == true){
    /* wait for voltage on Vcc to completely low */
        BKNI_Sleep(BSCD_RESET_WAIT_TIME);

    ulIFCmdVal = BREG_Read32( in_channelHandle->moduleHandle->regHandle, (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1));
    /* Pull high Vcc, and start activation sequence */
        if(in_channelHandle->currentChannelSettings.bDirectVccInverted ==true)
        ulIFCmdVal &= ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK;
        else
    ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_vcc_MASK;

   #if 0
 ulIFCmdVal &= ~BCHP_SCA_SC_IF_CMD_1_io_MASK;
#endif
    if(in_channelHandle->currentChannelSettings.bAutoDeactiveReq == true){
      ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_auto_vcc_MASK;
    }
    BREG_Write32( in_channelHandle->moduleHandle->regHandle, (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1), ulIFCmdVal);

    /* From Vcc goes high, TDA8024 expects 50-220 us for CLK start */
    BKNI_Delay(100);
    ulIFCmdVal &= ~BCHP_SCA_SC_IF_CMD_1_io_MASK;
    BREG_Write32( in_channelHandle->moduleHandle->regHandle, (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1), ulIFCmdVal);

}


    /* Set Clk cmd */
    ulValue = BCHP_SCA_SC_CLK_CMD_clk_en_MASK |
                (BSCD_P_MapScClkDivToMaskValue(in_channelHandle->currentChannelSettings.ucScClkDiv))  |
                ((in_channelHandle->currentChannelSettings.ucEtuClkDiv - 1) << 1)  |
                ((in_channelHandle->currentChannelSettings.ucBaudDiv == 31) ? 0 : 1);

    BDBG_MSG(("Reset: BCM_SC_CLK_CMD = 0x%x\n", ulValue));

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD),
         ulValue);

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_PRESCALE),
         in_channelHandle->currentChannelSettings.unPrescale);
    BDBG_MSG(("Reset: BSCD_P_PRESCALE = 0x%lx\n", in_channelHandle->currentChannelSettings.unPrescale));

    /* Use Auto Deactivation instead of TDA8004 */
    if (in_channelHandle->currentChannelSettings.bAutoDeactiveReq == true) {

        BDBG_MSG(("Inside Reset: Before auto io ulIFCmdVal = 0x%x\n", ulIFCmdVal));
        ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_auto_io_MASK;
        BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
             ulIFCmdVal);
        BDBG_MSG(("Inside Reset: after auto io ulIFCmdVal = 0x%x\n", ulIFCmdVal));
    }

#if 0 /*ndef BSCD_DSS_ICAM*/
    if(in_channelHandle->currentChannelSettings.bConnectDirectly == false){

    ulIFCmdVal |= BCHP_SCA_SC_IF_CMD_1_rst_MASK;
    BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
             ulIFCmdVal);
        }
#endif


    ulValue = 0;
    BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1),
             ulValue);

    BDBG_MSG(("Inside Reset: Before SmartCardEnableInt\n"));
#if 1

		/* check presence status again if card is already inserted. */
		if(BCHP_SCA_SC_STATUS_1_card_pres_MASK & BREG_Read32( in_channelHandle->moduleHandle->regHandle, (in_channelHandle->ulRegStartAddr + BSCD_P_STATUS_1)) ){
			in_channelHandle->channelStatus.bCardPresent = true;
		}
#endif

    /* Enable 2 interrupts with callback */
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                BSCD_Channel_EnableIntrCallback_isr (
                in_channelHandle, BSCD_IntType_eCardInsertInt,
                       BSCD_Channel_P_CardInsertCB_isr));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                BSCD_Channel_EnableIntrCallback_isr (
                in_channelHandle, BSCD_IntType_eCardRemoveInt,
                       BSCD_Channel_P_CardRemoveCB_isr));

    BREG_Write32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1),
        BCHP_SCA_SC_UART_CMD_1_uart_rst_MASK);

    /******************************************************************
    **
    ** UART Reset should be set within 1 ETU (however, we are generous
    ** to give it 2 etus.
    **
    *****************************************************************/
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue));

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
        BSCD_Channel_P_WaitForTimerEvent(in_channelHandle));

    /* Disable timer */
    timer.bIsTimerInterruptEnable = false;
    timer.bIsTimerEnable = false;
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
            BSCD_Channel_ConfigTimer(in_channelHandle, &timer, NULL));

    ulValue = BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1));

    /* If equal to zero, then UART reset has gone low, so return success */
    if ((ulValue & BCHP_SCA_SC_UART_CMD_1_uart_rst_MASK) == 0) {
        BDBG_MSG(("Reset Success\n"));

        /*
        **   INITIAL_CWI_SC_PROTO_CMD = 0x0f is required so that
        **   CWI does not remain equal to zero, which causes an
        **   erroneous timeout, the CWI is set correctly in the
        **   SmartCardEMVATRDecode procedure
        */
        BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD),
             BCHP_SCA_SC_PROTO_CMD_tbuf_rst_MASK | BCHP_SCA_SC_PROTO_CMD_rbuf_rst_MASK);
    }



BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_ResetIFD);
    return( errCode );
}


BERR_Code BSCD_Channel_PowerICC(
        BSCD_ChannelHandle          in_channelHandle,
        BSCD_PowerICC               in_iccAction
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ulValue;

    BDBG_ENTER(BSCD_Channel_PowerICC);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    if(in_channelHandle->currentChannelSettings.bConnectDirectly == false){

        switch (in_iccAction) {
            case BSCD_PowerICC_ePowerDown:
                ulValue =  BCHP_SCA_SC_IF_CMD_1_vcc_MASK | BREG_Read32(
                in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
                                #ifndef BSCD_DSS_ICAM
                                ulValue &= ~BCHP_SCA_SC_IF_CMD_1_rst_MASK;
                                #endif
                BREG_Write32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
                    ulValue);
                /* QX delay to let TDA finished */
                BKNI_Delay(20);
            break;
        case BSCD_PowerICC_ePowerUp:
            ulValue =  BREG_Read32(
                in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
            BREG_Write32(
                in_channelHandle->moduleHandle->regHandle,
                (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
                 (ulValue & ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK));
            break;

        default:
            BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
            break;
        }
        }else{
        switch (in_iccAction) {
                    case BSCD_PowerICC_ePowerUp:
                        ulValue =  BCHP_SCA_SC_IF_CMD_1_vcc_MASK | BREG_Read32(
                        in_channelHandle->moduleHandle->regHandle,
                        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
    if(in_channelHandle->currentChannelSettings.bAutoDeactiveReq == true){
      ulValue |= BCHP_SCA_SC_IF_CMD_1_auto_vcc_MASK;
    }
                        BREG_Write32(
                            in_channelHandle->moduleHandle->regHandle,
                            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
                            ulValue);
                    break;
                case BSCD_PowerICC_ePowerDown:
                    ulValue =  BREG_Read32(
                        in_channelHandle->moduleHandle->regHandle,
                        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1)) ;
                        ulValue &= ~BCHP_SCA_SC_IF_CMD_1_auto_vcc_MASK;
                    BREG_Write32(
                        in_channelHandle->moduleHandle->regHandle,
                        (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1),
                         (ulValue & ~BCHP_SCA_SC_IF_CMD_1_vcc_MASK));
                    break;

                default:
                    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
                    break;
                }

        }


BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_ResetIFD);
    return( errCode );
}

BERR_Code BSCD_Channel_SetVccLevel(
        BSCD_ChannelHandle          in_channelHandle,
        BSCD_VccLevel                   in_vccLevel
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ulValue = 0;

        BSTD_UNUSED(ulValue);
    BDBG_ENTER(BSCD_Channel_SetVccLevel);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );


    /***************************************************************
        Warning:  Note that we have to modify the board to use VPP pin of NDS
                ICAM smartcard and connect it to pin 3 (3V/5V) of TDA chip
                and run this function.  Make sure to disconnect your QAM or
                QPSK connection before calling this function or your smartcard
                will be damaged.
    ***************************************************************/
    BDBG_MSG(("BSCD_Channel_SetVccLevel: in_vccLevel = 0x%x\n", in_vccLevel));

    BKNI_EnterCriticalSection();
        in_channelHandle->currentChannelSettings.vcc = in_vccLevel;
			/* set pin muxing */
	if( in_channelHandle->currentChannelSettings.setPinmux){

#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)|| (BCHP_CHIP==7435)
    ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    ulValue &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_074_MASK);

    BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,ulValue );

    BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle,BCHP_GIO_IODIR_EXT, 1 << (74 - 64 + 6), 0);
#elif ((BCHP_CHIP == 7346) || (BCHP_CHIP==73465))
        switch( in_channelHandle->ucChannelNumber){
                case 0:
                        ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                        ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_015_MASK;
						ulValue |=(0x00000007 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_015_SHIFT);
                        BREG_Write32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, ulValue);
                        break;
                case 1:
                        ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
                        ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_032_MASK;
						ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_032_SHIFT);
                        BREG_Write32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, ulValue);
                        break;
        }
#elif (BCHP_CHIP == 7231)
        switch( in_channelHandle->ucChannelNumber){
                case 0:
                        ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                        ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_86_MASK;
                        BREG_Write32 (in_channelHandle->moduleHandle->regHandle,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, ulValue);

                        BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT, 1 << (86 - 64 + 6), 0);
                        break;
                case 1:
                        BDBG_WRN(("Slot 1 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                        break;
        }
#elif (BCHP_CHIP == 7358)||(BCHP_CHIP==7552)||(BCHP_CHIP==7360)||(BCHP_CHIP==7584)||(BCHP_CHIP==75845)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625)||(BCHP_CHIP==75525)
                switch( in_channelHandle->ucChannelNumber){
                        case 0:
                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_86_MASK;
                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle,
                                        BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, ulValue);

                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT, 1 << ( 86 - 64), 0);
                                break;
                        case 1:
                                BDBG_WRN(("Slot 1 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                                break;
                }
#elif (BCHP_CHIP == 7344)
                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                                                ulValue &= ~BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_08_MASK ;

                                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_AON_IODIR_LO, 1 << 8, 0);
                                                break;
                                        case 1:
                                                BDBG_WRN(("Slot 1 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                                                break;
                                }

#elif (BCHP_CHIP == 7429)||(BCHP_CHIP==7241)||(BCHP_CHIP==74295)
                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_086_MASK ;

                                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT, 1 << (86 - 64 + 4), 0);
                                                break;
                                        case 1:
                                                BDBG_WRN(("Slot 1 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                                                break;
                                        }
#elif (BCHP_CHIP == 7445)
                                switch( in_channelHandle->ucChannelNumber){
                case 1:
                #if (BCHP_VER >= BCHP_VER_D0)
                        ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_093_MASK ;
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19,ulValue );
                #else
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_093_MASK ;

                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,ulValue );
                #endif
                                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT_HI, 1 << (93 - 88), 0);
                                                break;
                                        case 0:
                                                BDBG_WRN(("Slot 0 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                                                break;
                                                }
#elif (BCHP_CHIP == 7439)
                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                #if(BCHP_VER < BCHP_VER_B0)
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_065_MASK ;
                                                ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_065_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,ulValue );
                                                #else
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_056_MASK ;
                                                ulValue |=(0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_056_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,ulValue );
                                                #endif
                                                break;
                                        case 1:
                                                #if(BCHP_VER < BCHP_VER_B0)
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_054_MASK ;
                                                ulValue |=(0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_054_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,ulValue );
                                                #else
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_093_MASK ;
                                                ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19_gpio_093_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19,ulValue );
                                                #endif
                                                break;
                                                }
#elif (BCHP_CHIP == 7366)
#if (BCHP_VER > BCHP_VER_A0)
                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_065_MASK ;
                                                ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_065_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,ulValue );
                                                break;
                                        case 1:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_054_MASK ;
                                                ulValue |=(0x00000004 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_054_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,ulValue );
                                                break;
                                                }
#endif
#elif ((BCHP_CHIP == 7563)|| (BCHP_CHIP==75635))
                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_86_MASK ;
												ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_86_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,ulValue );
                                                break;
                                        case 1:
                                                BDBG_WRN(("Slot 1 not supported in BRCM reference board, please use customer callback function to set the correct voltage\n"));
                                                break;
                                }
#elif (BCHP_CHIP == 7364)

                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_onoff_gpio_089_MASK ;
                                                ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_onoff_gpio_089_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,ulValue );
                                                break;
                                        case 1:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                        BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                                                ulValue &= ~BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_MASK ;
                                                ulValue |=(0x00000003<<BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1,ulValue );
                                                break;
                                }
#elif (BCHP_CHIP == 7250)

                                switch( in_channelHandle->ucChannelNumber){
                                        case 0:
                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_065_MASK ;
                                                ulValue |=(0x00000001 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_065_SHIFT);
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,ulValue );

                                                ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
                                                                                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
                                                ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_onoff_gpio_092_MASK ;
                                                BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,ulValue );

                                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT_HI, 1 << (92 - 64), 0);
                                                BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_IODIR_EXT_HI, 1 << (92 - 64), 1 << (92 - 64));
                                                break;
                                        case 1:
											ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
											ulValue &= ~BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_MASK ;
											ulValue |=(0x00000006<<BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_SHIFT);
											BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1,ulValue );
											break;
                                }
#elif (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268)
							switch( in_channelHandle->ucChannelNumber){
										case 0:
												ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
																		BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
												ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_020_MASK ;
												ulValue |=(0x00000008 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_020_SHIFT);
												BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,ulValue );
												break;
										case 1:
												ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,
																	BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
												ulValue &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_008_MASK ;
												ulValue |=(0x00000005 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_008_SHIFT);
												BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1,ulValue );

												break;
								}


#else
        BDBG_MSG(("Chip is not supported or it may use default pin mux setting!\n"));
#endif

    switch (in_vccLevel) {
        case BSCD_VccLevel_e3V:

#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)|| (BCHP_CHIP==7435)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (74 - 64 + 6), 0);
#elif (BCHP_CHIP==7231)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64 + 6), 0);
#elif (BCHP_CHIP==7358) ||(BCHP_CHIP==7552)||(BCHP_CHIP==7360)||(BCHP_CHIP==7584)||(BCHP_CHIP==75845)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625)||(BCHP_CHIP==75525)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64), 0);
#elif (BCHP_CHIP==7429)||(BCHP_CHIP==7241)||(BCHP_CHIP==74295)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64 + 4), 0);
#elif (BCHP_CHIP==7445)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT_HI, 1 << (93 - 88), 0);
#elif (BCHP_CHIP==7344)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_AON_DATA_LO, 1 << 8, 0);
#elif (BCHP_CHIP==7145)||(BCHP_CHIP==7366)||(BCHP_CHIP==7439)||(BCHP_CHIP==7364)||(BCHP_CHIP==7250)|| (BCHP_CHIP==7586)||(BCHP_CHIP==7563)|| (BCHP_CHIP==75635)||(BCHP_CHIP == 7346) || (BCHP_CHIP==73465)
            ulValue =  BREG_Read32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3)) ;
            ulValue &=~BCHP_SCA_SC_IF_CMD_3_vpp_MASK;
            BREG_Write32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3),
                    ulValue);
#else
            BDBG_WRN(("Chip is not supported!\n"));
#endif
            break;

        case BSCD_VccLevel_e5V:
#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)|| (BCHP_CHIP==7435)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (74 - 64 + 6), 1 << (74 - 64 + 6));
#elif (BCHP_CHIP==7231)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64 + 6), 1 << (86 - 64 + 6));
#elif (BCHP_CHIP==7358) ||(BCHP_CHIP==7552)||(BCHP_CHIP==7360)||(BCHP_CHIP==7584)||(BCHP_CHIP==75845)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625)||(BCHP_CHIP==75525)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64), 1 << (86 - 64));
#elif (BCHP_CHIP==7344)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_AON_DATA_LO, 1 << 8, 1 << 8);
#elif (BCHP_CHIP==7445)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT_HI, 1 << (93 - 88), 1 << (93 - 88));
#elif (BCHP_CHIP==7429)||(BCHP_CHIP==7241)||(BCHP_CHIP==74295)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT, 1 << (86 - 64 + 4), 1 << (86 - 64 + 4));
#elif (BCHP_CHIP==7439)||(BCHP_CHIP==7366)||(BCHP_CHIP==7145)||(BCHP_CHIP==7364)||(BCHP_CHIP==7250)|| (BCHP_CHIP==7586)||(BCHP_CHIP==7563)|| (BCHP_CHIP==75635)||(BCHP_CHIP == 7346) || (BCHP_CHIP==73465)||(BCHP_CHIP==7271)||(BCHP_CHIP==7268)
            ulValue =  BCHP_SCA_SC_IF_CMD_3_vpp_MASK | BREG_Read32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3)) ;
            BREG_Write32(
                    in_channelHandle->moduleHandle->regHandle,
                    (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3),
                    ulValue);
#else
            BDBG_WRN(("Chip is not supported!\n"));
#endif
            break;

        case  BSCD_VccLevel_e1P8V:
#if (BCHP_CHIP==7250)
            BREG_AtomicUpdate32 (in_channelHandle->moduleHandle->regHandle, BCHP_GIO_DATA_EXT_HI, 1 << (92 - 64), 0);
#else
            BDBG_WRN(("1.8V is not supported in BRCM ref board, please add customer code here.\n"));
#endif
            break;

        default:
            errCode = BERR_TRACE(BSCD_STATUS_FAILED);
            BDBG_ERR(("BSCD_Channel_SetVccLevel: Do not support VCC Level switch = 0x%x, \n", in_vccLevel));
            goto BSCD_P_DONE_LABEL;
        }

		}else{
			/* Need to set pin to sc_vpp */
			switch (in_vccLevel) {
				case BSCD_VccLevel_e3V:
					ulValue =  BREG_Read32(in_channelHandle->moduleHandle->regHandle,
										(in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3)) ;
					ulValue &=~BCHP_SCA_SC_IF_CMD_3_vpp_MASK;
					BREG_Write32(in_channelHandle->moduleHandle->regHandle,
								(in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3),ulValue);
					break;

				case BSCD_VccLevel_e5V:
					ulValue =  BREG_Read32(in_channelHandle->moduleHandle->regHandle,
										(in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3)) ;
					ulValue |=BCHP_SCA_SC_IF_CMD_3_vpp_MASK;
					BREG_Write32(in_channelHandle->moduleHandle->regHandle,
								(in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_3),ulValue);
					break;
				case BSCD_VccLevel_e1P8V:
					BDBG_WRN(("1.8V is not supported in BRCM ref board, please set your own pin mux.\n"));
					break;
			}
		}




BSCD_P_DONE_LABEL:
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BSCD_Channel_SetVccLevel);
    return( errCode );

}

BERR_Code BSCD_Channel_InsCardHwReset(
        BSCD_ChannelHandle          in_channelHandle,
        bool                          in_enableHwRst
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ulValue = 0;

    BDBG_ENTER(BSCD_Channel_InsCardHwReset);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

#if   !defined(LINUX) && ((BCHP_CHIP==7400) )
    ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,  BCHP_SUN_TOP_CTRL_RESET_CTRL);

        switch (in_channelHandle->ucChannelNumber) {
            case 0:
                if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                    ulValue |= ( 1 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );
                else
                    ulValue &= ~( 1 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );

                break;

            case 1:
                if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                    ulValue |= ( 2 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );
                else
                    ulValue &= ~( 2 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );

                break;

            case 2:
                if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                    ulValue |= ( 4 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );
                else
                    ulValue &= ~( 4 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );

                break;

            default:
                return -1;
        }

    BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_RESET_CTRL, ulValue);

#elif   !defined(LINUX) && (BCHP_CHIP==7401)
    ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,  BCHP_SUN_TOP_CTRL_RESET_CTRL);

        switch (in_channelHandle->ucChannelNumber) {
            case 0:
                if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                    ulValue |= ( 1 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );
                else
                    ulValue &= ~( 1 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );

                break;

            case 1:
                if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                    ulValue |= ( 2 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );
                else
                    ulValue &= ~( 2 << BCHP_SUN_TOP_CTRL_RESET_CTRL_sc_insert_reset_en_SHIFT );

                break;

            default:
                return -1;
        }

    BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_RESET_CTRL, ulValue);
/*#elif !defined(LINUX) &&  (defined(BCM97038) || defined(BCM7038)) */
#elif !defined(LINUX) &&   ((BCHP_CHIP==7038) || (BCHP_CHIP==7438) )

    ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,  BCHP_SUN_TOP_CTRL_RESET_CTRL);

    switch (in_channelHandle->ucChannelNumber) {
        case 2:
            if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                ulValue |= BCHP_SUN_TOP_CTRL_RESET_CTRL_sc2_insert_reset_en_MASK;
            else
                ulValue &= ~(BCHP_SUN_TOP_CTRL_RESET_CTRL_sc2_insert_reset_en_MASK);
            break;

        case 1:
            if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                ulValue |= BCHP_SUN_TOP_CTRL_RESET_CTRL_sc1_insert_reset_en_MASK;
            else
                ulValue &= ~(BCHP_SUN_TOP_CTRL_RESET_CTRL_sc1_insert_reset_en_MASK);
            break;

        case 0:
            if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                ulValue |= BCHP_SUN_TOP_CTRL_RESET_CTRL_sc0_insert_reset_en_MASK;
            else
                ulValue &= ~(BCHP_SUN_TOP_CTRL_RESET_CTRL_sc0_insert_reset_en_MASK);
            break;

        default:
            return -1;
    }

    BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_RESET_CTRL, ulValue);

#elif !defined(LINUX) &&  (BCHP_CHIP==3560)

    ulValue = BREG_Read32 (in_channelHandle->moduleHandle->regHandle,  BCHP_SUN_TOP_CTRL_RESET_CTRL);

    switch (in_channelHandle->ucChannelNumber) {
        case 0:
            if (  in_enableHwRst ) /*Enable Inser Card Hardware Reset*/
                ulValue |= BCHP_SUN_TOP_CTRL_RESET_CTRL_sc0_insert_reset_en_MASK;
            else
                ulValue &= ~(BCHP_SUN_TOP_CTRL_RESET_CTRL_sc0_insert_reset_en_MASK);
            break;

        default:
            return -1;
    }

    BREG_Write32 (in_channelHandle->moduleHandle->regHandle, BCHP_SUN_TOP_CTRL_RESET_CTRL, ulValue);
#else
    BSTD_UNUSED( ulValue );
    BSTD_UNUSED( in_enableHwRst);

#endif


BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_InsCardHwReset);
    return( errCode );

}


static BERR_Code BSCD_startATRTimers (BSCD_ChannelHandle  in_channelHandle)
{
    BERR_Code               errCode = BERR_SUCCESS;
    BSCD_EventTimerMode      eEventTimerMode;
    BSCD_Timer              timer;
    BSCD_TimerValue timeValue= {BSCD_MIN_ETU_PER_ATR_BYTE + 1, BSCD_TimerUnit_eETU};

    eEventTimerMode.int_after_reset = false;
    eEventTimerMode.int_after_compare = true;
    eEventTimerMode.run_after_reset = false;
    eEventTimerMode.run_after_compare = true;
    eEventTimerMode.start_event = BSCD_P_RX_START_BIT_EVENT_SRC;
    eEventTimerMode.incr_event = BSCD_P_RX_ETU_TICK_EVENT_SRC;
    eEventTimerMode.reset_event = BSCD_P_NO_EVENT_EVENT_SRC;

    timer.eTimerType = BSCD_TimerType_eEvent1Timer;
    timer.timerMode.eEventTimerMode = &eEventTimerMode;
    timer.bIsTimerEnable = true;
    timer.bIsTimerInterruptEnable = false;

    BDBG_MSG(("Activating: Event1 timer \n"));
    if(BERR_SUCCESS != BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue))
    {
        BDBG_ERR(("not able to configure BSCD_TimerType_eEvent1Timer Timer\n"));
        return BSCD_STATUS_FAILED;
    }

    timer.eTimerType = BSCD_TimerType_eEvent2Timer;
    timeValue.ulValue = 0xff;
    eEventTimerMode.int_after_reset = false;
    eEventTimerMode.int_after_compare = false;
    eEventTimerMode.run_after_reset = false;
    eEventTimerMode.run_after_compare = false;
    eEventTimerMode.start_event = BSCD_P_EVENT1_INTR_EVENT_SRC;
    eEventTimerMode.incr_event = BSCD_P_EVENT1_INTR_EVENT_SRC;

    BDBG_MSG(("Activating: Event2 timer \n"));
    if(BERR_SUCCESS != BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue))
    {
        BDBG_ERR(("not able to configure BSCD_TimerType_eEvent1Timer Timer\n"));
    }

    timer.eTimerType = BSCD_TimerType_eWaitTimer;
    timer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;
    timer.bIsTimerEnable = true;
    timer.bIsTimerInterruptEnable = true;

    /* Enable WWT to ensure the max interval between 2 consecutive ATR chars of 10080 ETU */
    BDBG_MSG(("Activating: Set WWT timer \n"));
    if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eEMV2000)
        timeValue.ulValue = BSCD_MAX_ETU_PER_ATR_BYTE_EMV2000;
    else /* EMV 96 or the rest */
        timeValue.ulValue = BSCD_MAX_ETU_PER_ATR_BYTE;

    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                    BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue));

BSCD_P_DONE_LABEL:

    return BERR_SUCCESS;
}

static BERR_Code BSCD_stopATRTimers(BSCD_ChannelHandle  in_channelHandle)
{
    BSCD_Timer              timer;
    BERR_Code       errCode = BERR_SUCCESS;

    timer.eTimerType = BSCD_TimerType_eEvent1Timer;
    timer.timerMode.eEventTimerMode = NULL;
    timer.bIsTimerEnable = false;
    timer.bIsTimerInterruptEnable = false;

    if(BSCD_Channel_ConfigTimer(in_channelHandle, &timer, NULL) != BERR_SUCCESS)
    {
        errCode = BSCD_STATUS_FAILED;
    }

    timer.eTimerType = BSCD_TimerType_eEvent2Timer;
    if(BSCD_Channel_ConfigTimer(in_channelHandle, &timer, NULL) != BERR_SUCCESS)
    {
        errCode = BSCD_STATUS_FAILED;
    }

    timer.eTimerType = BSCD_TimerType_eWaitTimer;
    timer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;
    if(BSCD_Channel_ConfigTimer(in_channelHandle, &timer, NULL) != BERR_SUCCESS)
    {
        errCode = BSCD_STATUS_FAILED;
    }

    return errCode;
}

BERR_Code BSCD_Channel_ResetCard(
        BSCD_ChannelHandle          in_channelHandle,
        BSCD_ResetCardAction               in_iccAction
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_ResetCard);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

		/* SWSECSUPPORT-1850, we shouldn't power up smart card interface if card not present */

	BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
			(in_channelHandle->channelStatus.bCardPresent != true ) );
	if( in_channelHandle->currentChannelSettings.setVcc){

        if(BSCD_Channel_SetVccLevel(in_channelHandle,
                                in_channelHandle->currentChannelSettings.vcc)){
                                return BERR_UNKNOWN;
                                };
        }


    if ((errCode = BSCD_startATRTimers(in_channelHandle)) != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto BSCD_P_DONE_LABEL;
    }
    in_channelHandle->bOnAtr = true;

    switch (in_iccAction) {
        case BSCD_ResetCardAction_eNoAction:
            if (  (errCode = BSCD_Channel_P_Activating(in_channelHandle)) != BERR_SUCCESS) {
                   errCode = BERR_TRACE(errCode);
                goto BSCD_P_DONE_LABEL;
            }
           break;
        case BSCD_ResetCardAction_eReceiveAndDecode:
            BDBG_MSG(("BSCD_ResetCardAction_eReceiveAndDecode \n"));
            if (  ((errCode = BSCD_Channel_P_Activating(in_channelHandle)) != BERR_SUCCESS) ||
                   ((errCode = BSCD_Channel_P_ReceiveAndDecode(in_channelHandle)) != BERR_SUCCESS) ) {
                   errCode = BERR_TRACE(errCode);
                goto BSCD_P_DONE_LABEL;
            }
            break;

        default:
            BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
            break;
    }


BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_ResetCard);
        if(errCode == BSCD_STATUS_DEACTIVATE){
            BSCD_Channel_Deactivate(in_channelHandle);
			BREG_Write32( in_channelHandle->moduleHandle->regHandle, (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_1), BCHP_SCA_SC_INTR_EN_1_pres_ien_MASK);
        }
    BDBG_MSG(("Leave ResetCard erroCode = 0x%x\n", errCode));
    return( errCode );
}


BERR_Code BSCD_Channel_GetStatus(
        BSCD_ChannelHandle          in_channelHandle,
        BSCD_Status                 *outp_status
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_GetStatus);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    *outp_status = in_channelHandle->channelStatus;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_GetStatus);
    return( errCode );
}

BERR_Code BSCD_Channel_Transmit(
        BSCD_ChannelHandle          in_channelHandle,
        uint8_t                     *inp_ucXmitData,
        unsigned long                    in_ulNumXmitBytes
)
{
    if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eIrdeto) {
        return (BSCD_Channel_P_T14IrdetoTransmit(
                in_channelHandle,
                inp_ucXmitData,
                in_ulNumXmitBytes));
    }
    else {
        return (BSCD_Channel_P_T0T1Transmit(
                in_channelHandle,
                inp_ucXmitData,
                in_ulNumXmitBytes));
    }
}


BERR_Code BSCD_Channel_Receive(
        BSCD_ChannelHandle       in_channelHandle,
        uint8_t                         *outp_ucRcvData,
        unsigned long                  *outp_ulNumRcvBytes,
        unsigned long                    in_ulMaxReadBytes
)
{
    BERR_Code       errCode = BSCD_STATUS_READ_SUCCESS;

    BDBG_ENTER(BSCD_Channel_Receive);
    BDBG_ASSERT( in_channelHandle );

    BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    *outp_ulNumRcvBytes = 0;

    /* Coverity: 35203 */
    if(outp_ucRcvData == NULL) {
        BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
    }

    if ((in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e0) ||
        (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e14_IRDETO) ) {


        /* you could have a timeout when receiving the ATR
         * only parsing ATR will tell if it correct */
        errCode = BSCD_Channel_P_T0ReadData(in_channelHandle, outp_ucRcvData, outp_ulNumRcvBytes,
                            in_ulMaxReadBytes);


        if(in_channelHandle->bOnAtr)
        {
            /* only standard that retrieve ATR in a single shot can
             * check the 12 ETU intra-char delay
             */
            if(in_channelHandle->currentChannelSettings.scStandard != BSCD_Standard_eNDS)
            {
                unsigned int ev2cnt;

                /* wait for a while, otherwise the counter will not be increased
                 * after the last char
                 */
                if(*outp_ulNumRcvBytes == in_ulMaxReadBytes)
                {
                    BKNI_Delay(20);
                }
                ev2cnt = BREG_Read32(
                        in_channelHandle->moduleHandle->regHandle,
                        (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CNT));
                if(ev2cnt != *outp_ulNumRcvBytes) {
                    BDBG_ERR(("events 2 counter %d != atr len %d\n",  ev2cnt, (unsigned int)*outp_ulNumRcvBytes));
                    *outp_ulNumRcvBytes = 0;
                    errCode = BSCD_STATUS_FAILED;
                }
            }
            in_channelHandle->bOnAtr = false;
            if (BSCD_stopATRTimers(in_channelHandle) != BERR_SUCCESS)
            {
                *outp_ucRcvData = 0;
                errCode = BSCD_STATUS_FAILED;
            }
        }
    }/* BSCD_AsyncProtocolType_e0 */

    else { /* BSCD_AsyncProtocolType_e1 */

        BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
                    BSCD_Channel_P_T1ReadData(in_channelHandle, outp_ucRcvData, outp_ulNumRcvBytes,
                            in_ulMaxReadBytes));

    } /* BSCD_AsyncProtocolType_e1 */

    if (*outp_ulNumRcvBytes > 0) {

        /* Ignore the ReadTimeOut error returned by SmartCardByteRead */
        /* BDBG_MSG (("success in SmartCardReadCmd\n")); */
    }

    else {
        BDBG_MSG (("No Response detected...deactivating, scerr = %02x\n",errCode));
        BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
    }



BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_Receive);
    /* BDBG_MSG(("Leave BSCD_Channel_Receive = 0x%x\n", errCode)); */
    return( errCode );
}


BERR_Code BSCD_Channel_ConfigTimer(
        BSCD_ChannelHandle   in_channelHandle,
        BSCD_Timer                  *inp_timer,
        BSCD_TimerValue             *inp_unCount
)
{
    return BSCD_Channel_P_ConfigTimer_generic(in_channelHandle, inp_timer, inp_unCount, true);
}

BERR_Code BSCD_Channel_EnableIntrCallback_isr(
    BSCD_ChannelHandle  in_channelHandle,
    BSCD_IntrType       in_eIntType,
    BSCD_IsrCallbackFunc in_callback
)
{
    uint32_t  ulVal;
    unsigned int  unReg = BSCD_P_INTR_EN_1, i;
    BERR_Code errCode = BERR_SUCCESS;

    if ( (in_eIntType == BSCD_IntType_eTParityInt)    ||
        (in_eIntType == BSCD_IntType_eTimerInt)      ||
        (in_eIntType == BSCD_IntType_eCardInsertInt) ||
        (in_eIntType == BSCD_IntType_eCardRemoveInt) ||
        (in_eIntType == BSCD_IntType_eBGTInt)        ||
        (in_eIntType == BSCD_IntType_eTDoneInt)      ||
        (in_eIntType == BSCD_IntType_eRetryInt)      ||
        (in_eIntType == BSCD_IntType_eTEmptyInt) ||
        (in_eIntType == BSCD_IntType_eEvent1Int)) {
        unReg = BSCD_P_INTR_EN_1;
        /* BDBG_MSG(("BSCD_P_INTR_EN_1: ")); */
    }
    else if ( (in_eIntType == BSCD_IntType_eRParityInt) ||
        (in_eIntType == BSCD_IntType_eATRInt)          ||
        (in_eIntType == BSCD_IntType_eCWTInt)          ||
        (in_eIntType == BSCD_IntType_eRLenInt)         ||
        (in_eIntType == BSCD_IntType_eWaitInt)         ||
        (in_eIntType == BSCD_IntType_eRcvInt)          ||
        (in_eIntType == BSCD_IntType_eRReadyInt) ||
        (in_eIntType == BSCD_IntType_eEvent2Int)) {
        unReg = BSCD_P_INTR_EN_2;
        /* BDBG_MSG(("BSCD_P_INTR_EN_2: ")); */
    }
    else if (in_eIntType == BSCD_IntType_eEDCInt) {
        unReg = BSCD_P_PROTO_CMD;
    }
    else {
        BDBG_ERR(("Interrupt not supported, in_eIntType = %d\n", in_eIntType));
        BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,  true);
    }

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + unReg));

    /* BDBG_MSG(("ulVal = 0x%x", ulVal)); */

    switch (in_eIntType) {

        case BSCD_IntType_eTParityInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.tParityIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.tParityIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.tParityIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.tParityIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_tpar_intr_MASK;
            break;

        case BSCD_IntType_eTimerInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.timerIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.timerIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.timerIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.timerIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_timer_intr_MASK;
            break;

        case BSCD_IntType_eCardInsertInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.cardInsertIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.cardInsertIsrCBFunc[i] = in_callback;
                    BDBG_MSG(("new BSCD_IntType_eCardInsertInt  callback "));
                    break;
                }
                else if ((in_channelHandle->callBack.cardInsertIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.cardInsertIsrCBFunc[i] == in_callback) ) {
                    BDBG_MSG(("BSCD_IntType_eCardInsertInt same callback "));
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_pres_intr_MASK;
            break;

        case BSCD_IntType_eCardRemoveInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.cardRemoveIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.cardRemoveIsrCBFunc[i] = in_callback;
                    BDBG_MSG(("new BSCD_IntType_eCardRemoveInt  callback "));
                    break;
                }
                else if ((in_channelHandle->callBack.cardRemoveIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.cardRemoveIsrCBFunc[i] == in_callback) ) {
                    BDBG_MSG(("BSCD_IntType_eCardRemoveInt same callback "));
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_pres_intr_MASK;
            break;

        case BSCD_IntType_eBGTInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.bgtIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.bgtIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.bgtIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.bgtIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_bgt_intr_MASK;
            break;

        case BSCD_IntType_eTDoneInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.tDoneIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.tDoneIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.tDoneIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.tDoneIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_tdone_intr_MASK;
            break;

        case BSCD_IntType_eRetryInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.retryIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.retryIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.retryIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.retryIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_retry_intr_MASK;
            break;

        case BSCD_IntType_eTEmptyInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.tEmptyIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.tEmptyIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.tEmptyIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.tEmptyIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_tempty_intr_MASK;
            break;

        case BSCD_IntType_eRParityInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.rParityIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.rParityIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.rParityIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.rParityIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_rpar_intr_MASK;
            break;

        case BSCD_IntType_eATRInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.atrIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.atrIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.atrIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.atrIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_atrs_intr_MASK;
            break;

        case BSCD_IntType_eCWTInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.cwtIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.cwtIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.cwtIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.cwtIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_cwt_intr_MASK;
            break;

        case BSCD_IntType_eRLenInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.rLenIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.rLenIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.rLenIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.rLenIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_rlen_intr_MASK;
            break;

        case BSCD_IntType_eWaitInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.waitIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.waitIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.waitIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.waitIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_wait_intr_MASK;
            break;

        case BSCD_IntType_eRcvInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.rcvIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.rcvIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.rcvIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.rcvIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_rcv_intr_MASK;
            break;

        case BSCD_IntType_eRReadyInt:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.rReadyIsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.rReadyIsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.rReadyIsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.rReadyIsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_rready_intr_MASK;
            break;

        case BSCD_IntType_eEDCInt:
            if (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e0 ) {
                for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                    if (in_channelHandle->callBack.edcIsrCBFunc[i] == NULL) {
                        in_channelHandle->callBack.edcIsrCBFunc[i] = in_callback;
                        break;
                    }
                    else if ((in_channelHandle->callBack.edcIsrCBFunc[i] != NULL) &&
                        (in_channelHandle->callBack.edcIsrCBFunc[i] == in_callback) ) {
                        break;
                    }
                }
                ulVal |=  BCHP_SCA_SC_PROTO_CMD_edc_en_MASK;
            }
            break;

        case BSCD_IntType_eEvent1Int:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.event1IsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.event1IsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.event1IsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.event1IsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_1_event1_intr_MASK;
            break;

        case BSCD_IntType_eEvent2Int:
            for (i=0; i< BSCD_MAX_NUM_CALLBACK_FUNC; i++)  {
                if (in_channelHandle->callBack.event2IsrCBFunc[i] == NULL) {
                    in_channelHandle->callBack.event2IsrCBFunc[i] = in_callback;
                    break;
                }
                else if ((in_channelHandle->callBack.event2IsrCBFunc[i] != NULL) &&
                    (in_channelHandle->callBack.event2IsrCBFunc[i] == in_callback) ) {
                    break;
                }
            }
            ulVal |=  BCHP_SCA_SC_INTR_STAT_2_event2_intr_MASK;
            break;

        default:
            BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
    }

    BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + unReg),
             ulVal);


    /*BDBG_MSG((", final ulVal = 0x%x\n ", ulVal)); */

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_EnableIntrCallback_isr);
    return( errCode );

}

BERR_Code BSCD_Channel_DisableIntrCallback_isr(
    BSCD_ChannelHandle  in_channelHandle,
    BSCD_IntrType    in_eIntType
)
{
    uint32_t ulVal;
    unsigned int  unReg = BSCD_P_INTR_EN_1;
    BERR_Code errCode = BERR_SUCCESS;

    if ( (in_eIntType == BSCD_IntType_eTParityInt)    ||
        (in_eIntType == BSCD_IntType_eTimerInt)      ||
        (in_eIntType == BSCD_IntType_eCardInsertInt) ||
        (in_eIntType == BSCD_IntType_eCardRemoveInt) ||
        (in_eIntType == BSCD_IntType_eBGTInt)        ||
        (in_eIntType == BSCD_IntType_eTDoneInt)      ||
        (in_eIntType == BSCD_IntType_eRetryInt)      ||
        (in_eIntType == BSCD_IntType_eTEmptyInt) ||
            (in_eIntType == BSCD_IntType_eEvent1Int)) {
        unReg = BSCD_P_INTR_EN_1;
        /* BDBG_MSG(("BSCD_P_INTR_EN_1: "));         */
    }
    else if ( (in_eIntType == BSCD_IntType_eRParityInt) ||
        (in_eIntType == BSCD_IntType_eATRInt)          ||
        (in_eIntType == BSCD_IntType_eCWTInt)          ||
        (in_eIntType == BSCD_IntType_eRLenInt)         ||
        (in_eIntType == BSCD_IntType_eWaitInt)         ||
        (in_eIntType == BSCD_IntType_eRcvInt)          ||
        (in_eIntType == BSCD_IntType_eRReadyInt) ||
         (in_eIntType == BSCD_IntType_eEvent2Int)) {
        unReg = BSCD_P_INTR_EN_2;
        /* BDBG_MSG(("BSCD_P_INTR_EN_2: "));         */
    }
    else if (in_eIntType == BSCD_IntType_eEDCInt) {
        unReg = BSCD_P_PROTO_CMD;
    }
    else {
        BDBG_MSG(("Interrupt not supported\n"));
        BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED,  true);
    }

    ulVal = BREG_Read32(
        in_channelHandle->moduleHandle->regHandle,
        (in_channelHandle->ulRegStartAddr + unReg));

    /* BDBG_MSG(("ulVal = 0x%x", ulVal)); */

    switch (in_eIntType) {


        case BSCD_IntType_eTParityInt:
             ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_tpar_intr_MASK;
             break;

        case BSCD_IntType_eTimerInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_timer_intr_MASK;
            break;

        case BSCD_IntType_eCardInsertInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_pres_intr_MASK;
            break;

        case BSCD_IntType_eCardRemoveInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_pres_intr_MASK;
            break;

        case BSCD_IntType_eBGTInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_bgt_intr_MASK;
            break;

        case BSCD_IntType_eTDoneInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_tdone_intr_MASK;
            break;

        case BSCD_IntType_eRetryInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_retry_intr_MASK;
            break;

        case BSCD_IntType_eTEmptyInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_tempty_intr_MASK;
            break;

        case BSCD_IntType_eRParityInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_rpar_intr_MASK;
            break;

        case BSCD_IntType_eATRInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_atrs_intr_MASK;
            break;

        case BSCD_IntType_eCWTInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_cwt_intr_MASK;
            break;

        case BSCD_IntType_eRLenInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_rlen_intr_MASK;
            break;

        case BSCD_IntType_eWaitInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_wait_intr_MASK;
            break;

        case BSCD_IntType_eRcvInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_rcv_intr_MASK;
            break;

        case BSCD_IntType_eRReadyInt:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_2_rready_intr_MASK;
            break;

        case BSCD_IntType_eEDCInt:
            ulVal &= ~BCHP_SCA_SC_PROTO_CMD_edc_en_MASK;
            break;

        case BSCD_IntType_eEvent1Int:
            ulVal &= ~BCHP_SCA_SC_INTR_STAT_1_event1_intr_MASK;
            break;

        case BSCD_IntType_eEvent2Int:
            ulVal &=  ~BCHP_SCA_SC_INTR_STAT_2_event2_intr_MASK;
            break;

        default:
            BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, true);
    }

    BREG_Write32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + unReg),
             ulVal);

    /* BDBG_MSG((", final ulVal = 0x%x\n ", ulVal)); */
BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_DisableIntrCallback_isr);
    return( errCode );
}

BERR_Code BSCD_Channel_EnableInterrupts(
    BSCD_ChannelHandle  in_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_EnableInterrupts);
    BDBG_ASSERT( in_channelHandle );

    BKNI_EnterCriticalSection();
    if ( (errCode = BSCD_Channel_P_EnableInterrupts_isr(in_channelHandle)) != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto BSCD_P_DONE_LABEL;
    }
    BKNI_LeaveCriticalSection();

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_EnableInterrupts);
    return( errCode );
}

BERR_Code BSCD_Channel_ResetBlockWaitTimer(
        BSCD_ChannelHandle          in_channelHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BSCD_Timer      timer = {BSCD_TimerType_eWaitTimer, {BSCD_WaitTimerMode_eBlockWaitTime},
                            false, true};
    BSCD_TimerValue    timeValue= {BSCD_DEFAULT_BLOCK_WAITING_TIME, BSCD_TimerUnit_eETU};

    BDBG_ENTER(BSCD_Channel_ResetBlockWaitTimer);
    BDBG_ASSERT( in_channelHandle );

    timeValue.ulValue = in_channelHandle->currentChannelSettings.blockWaitTime.ulValue ;
    BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
            BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue));

    in_channelHandle->currentChannelSettings.blockWaitTimeExt.ulValue = 0;

BSCD_P_DONE_LABEL:

    BDBG_LEAVE(BSCD_Channel_ResetBlockWaitTimer);
    return( errCode );
}


BERR_Code BSCD_Channel_SetBlockWaitTimeExt(
        BSCD_ChannelHandle          in_channelHandle,
        uint32_t                    in_ulBlockWaitTimeExtInETU
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BSCD_Channel_SetBlockWaitTimeExt);
    BDBG_ASSERT( in_channelHandle );

    in_channelHandle->currentChannelSettings.blockWaitTimeExt.ulValue = in_ulBlockWaitTimeExtInETU;

    BDBG_LEAVE(BSCD_Channel_SetBlockWaitTimeExt);
    return( errCode );
}

void BSCD_Channel_DumpRegisters(
        BSCD_ChannelHandle       in_channelHandle
)
{
    uint32_t ulVal;

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_1));
    BDBG_MSG(("UART_CMD_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2));
    BDBG_MSG(("UART_CMD_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD));
    BDBG_MSG(("PROTO = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_FLOW_CMD));
    BDBG_MSG(("FLOW_CMD = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_1));
    BDBG_MSG(("IF_CMD_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_IF_CMD_2));
    BDBG_MSG(("IF_CMD_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_STAT_1));
    BDBG_MSG(("INTR_STAT_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_STAT_2));
    BDBG_MSG(("INTR_STAT_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_1));
    BDBG_MSG(("INTR_EN_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_INTR_EN_2));
    BDBG_MSG(("INTR_EN_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_CLK_CMD));
    BDBG_MSG(("CLK_CMD = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_PRESCALE));
    BDBG_MSG(("PRESCALE = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CMD));
    BDBG_MSG(("TIMER_CMD = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_BGT));
    BDBG_MSG(("BGT = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CNT_1));
    BDBG_MSG(("TIMER_CNT_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CNT_2));
    BDBG_MSG(("TIMER_CNT_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CMP_1));
    BDBG_MSG(("TIMER_CMP_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CMP_2));
    BDBG_MSG(("TIMER_CMP_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_WAIT_1));
    BDBG_MSG(("SC_WAIT_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_WAIT_2));
    BDBG_MSG(("SC_WAIT_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_WAIT_3));
    BDBG_MSG(("SC_WAIT_3 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_TGUARD));
    BDBG_MSG(("TGUARD = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT1_CMD_1));
    BDBG_MSG(("BSCD_P_EVENT1_CMD_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT1_CMD_2));
    BDBG_MSG(("BSCD_P_EVENT1_CMD_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT1_CMD_3));
    BDBG_MSG(("BSCD_P_EVENT1_CMD_3 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT1_CMD_4));
    BDBG_MSG(("BSCD_P_EVENT1_CMD_4 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT1_CMP));
    BDBG_MSG(("BSCD_P_EVENT1_CMP = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_1));
    BDBG_MSG(("BSCD_P_EVENT2_CMD_1 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_2));
    BDBG_MSG(("BSCD_P_EVENT2_CMD_2 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_3));
    BDBG_MSG(("BSCD_P_EVENT2_CMD_3 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_4));
    BDBG_MSG(("BSCD_P_EVENT2_CMD_4 = %02x \n",ulVal));

    ulVal = BREG_Read32(
            in_channelHandle->moduleHandle->regHandle,
            (in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMP));
    BDBG_MSG(("BSCD_P_EVENT2_CMP = %02x \n",ulVal));

    /*
    bcmDeviceRead(inp_device, eSize8, BCM_SC_TRANSMIT, (void *)&ulVal);
    BDBG_MSG(("TRANSMIT = %02x \n",ulVal));
    bcmDeviceRead(inp_device, eSize8, BCM_SC_RECEIVE, (void *)&ulVal);
    BDBG_MSG(("RECEIVE = %02x \n",ulVal));


    bcmDeviceRead(inp_device, eSize8, BCM_SC_TLEN_1, (void *)&ulVal);
    BDBG_MSG(("TLEN_1 = %02x \n",ulVal));
    bcmDeviceRead(inp_device, eSize8, BCM_SC_TLEN_2, (void *)&ulVal);
    BDBG_MSG(("TLEN_2 = %02x \n",ulVal));
    bcmDeviceRead(inp_device, eSize8, BCM_SC_RLEN_1, (void *)&ulVal);
    BDBG_MSG(("RLEN_1 = %02x \n",ulVal));
    bcmDeviceRead(inp_device, eSize8, BCM_SC_RLEN_2, (void *)&ulVal);
    BDBG_MSG(("RLEN_2 = %02x \n",ulVal));

    */
}


