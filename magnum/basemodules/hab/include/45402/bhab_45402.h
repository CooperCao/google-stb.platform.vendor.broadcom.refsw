/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/
#ifndef _BHAB_45402_H_
#define _BHAB_45402_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"

#define BHAB_45402_SPI_READ_COMMAND  0x40
#define BHAB_45402_SPI_WRITE_COMMAND 0x41

/* HAB header macros */
#define BHAB_45402_ACK            0x80000000
#define BHAB_45402_WRITE 0
#define BHAB_45402_READ 1
#define BHAB_45402_MODULE_LEAP 0
#define BHAB_45402_MODULE_WFE  1
#define BHAB_45402_MODULE_SAT  2
#define BHAB_45402_MODULE_DSQ  3
#define BHAB_45402_MODULE_FSK  4
#define BHAB_45402_MODULE_MXT  5
#define BHAB_45402_MODULE_VSN  6

/* interrupt bits in LEAP_HOST_L2_0_STATUS0 */
#define BHAB_45402_HIRQ0_INIT_DONE                 0x00000001
#define BHAB_45402_HIRQ0_HAB_DONE                  0x00000002
#define BHAB_45402_HIRQ0_FSK_RX                    0x00000010
#define BHAB_45402_HIRQ0_FSK_TX                    0x00000020
#define BHAB_45402_HIRQ0_SA_DONE                   0x00000040
#define BHAB_45402_HIRQ0_WFE0_READY                0x00000080
#define BHAB_45402_HIRQ0_WFE1_READY                0x00000100
#define BHAB_45402_HIRQ0_SAT_INIT_DONE             0x00000200
#define BHAB_45402_HIRQ0_DSQ0_TX                   0x00000400
#define BHAB_45402_HIRQ0_DSQ0_RX                   0x00000800
#define BHAB_45402_HIRQ0_DSQ0_VSENSE               0x00001000
#define BHAB_45402_HIRQ0_DSQ1_TX                   0x00002000
#define BHAB_45402_HIRQ0_DSQ1_RX                   0x00004000
#define BHAB_45402_HIRQ0_DSQ1_VSENSE               0x00008000

/* interrupt bits in LEAP_HOST_L2_0_STATUS1 */
#define BHAB_45402_HIRQ1_SAT0_LOCK_CHANGE          0x00000001
#define BHAB_45402_HIRQ1_SAT0_ACQ_DONE             0x00000002
#define BHAB_45402_HIRQ1_SAT0_READY                0x00000004
#define BHAB_45402_HIRQ1_SAT0_SIGNAL_NOTIFICATION  0x00000008
#define BHAB_45402_HIRQ1_SAT1_LOCK_CHANGE          0x00000010
#define BHAB_45402_HIRQ1_SAT1_ACQ_DONE             0x00000020
#define BHAB_45402_HIRQ1_SAT1_READY                0x00000040
#define BHAB_45402_HIRQ1_SAT1_SIGNAL_NOTIFICATION  0x00000080
#define BHAB_45402_HIRQ1_SAT2_LOCK_CHANGE          0x00000100
#define BHAB_45402_HIRQ1_SAT2_ACQ_DONE             0x00000200
#define BHAB_45402_HIRQ1_SAT2_READY                0x00000400
#define BHAB_45402_HIRQ1_SAT2_SIGNAL_NOTIFICATION  0x00000800
#define BHAB_45402_HIRQ1_SAT3_LOCK_CHANGE          0x00001000
#define BHAB_45402_HIRQ1_SAT3_ACQ_DONE             0x00002000
#define BHAB_45402_HIRQ1_SAT3_READY                0x00004000
#define BHAB_45402_HIRQ1_SAT3_SIGNAL_NOTIFICATION  0x00008000
#define BHAB_45402_HIRQ1_SAT4_LOCK_CHANGE          0x00010000
#define BHAB_45402_HIRQ1_SAT4_ACQ_DONE             0x00020000
#define BHAB_45402_HIRQ1_SAT4_READY                0x00040000
#define BHAB_45402_HIRQ1_SAT4_SIGNAL_NOTIFICATION  0x00080000
#define BHAB_45402_HIRQ1_SAT5_LOCK_CHANGE          0x00100000
#define BHAB_45402_HIRQ1_SAT5_ACQ_DONE             0x00200000
#define BHAB_45402_HIRQ1_SAT5_READY                0x00400000
#define BHAB_45402_HIRQ1_SAT5_SIGNAL_NOTIFICATION  0x00800000
#define BHAB_45402_HIRQ1_SAT6_LOCK_CHANGE          0x01000000
#define BHAB_45402_HIRQ1_SAT6_ACQ_DONE             0x02000000
#define BHAB_45402_HIRQ1_SAT6_READY                0x04000000
#define BHAB_45402_HIRQ1_SAT6_SIGNAL_NOTIFICATION  0x08000000
#define BHAB_45402_HIRQ1_SAT7_LOCK_CHANGE          0x10000000
#define BHAB_45402_HIRQ1_SAT7_ACQ_DONE             0x20000000
#define BHAB_45402_HIRQ1_SAT7_READY                0x40000000
#define BHAB_45402_HIRQ1_SAT7_SIGNAL_NOTIFICATION  0x80000000

/* interrupt bits in LEAP_HOST_L2_1_STATUS0 */
#define BHAB_45402_HIRQ2_SAT8_LOCK_CHANGE          0x00000001
#define BHAB_45402_HIRQ2_SAT8_ACQ_DONE             0x00000002
#define BHAB_45402_HIRQ2_SAT8_READY                0x00000004
#define BHAB_45402_HIRQ2_SAT8_SIGNAL_NOTIFICATION  0x00000008
#define BHAB_45402_HIRQ2_SAT9_LOCK_CHANGE          0x00000010
#define BHAB_45402_HIRQ2_SAT9_ACQ_DONE             0x00000020
#define BHAB_45402_HIRQ2_SAT9_READY                0x00000040
#define BHAB_45402_HIRQ2_SAT9_SIGNAL_NOTIFICATION  0x00000080
#define BHAB_45402_HIRQ2_SAT10_LOCK_CHANGE         0x00000100
#define BHAB_45402_HIRQ2_SAT10_ACQ_DONE            0x00000200
#define BHAB_45402_HIRQ2_SAT10_READY               0x00000400
#define BHAB_45402_HIRQ2_SAT10_SIGNAL_NOTIFICATION 0x00000800
#define BHAB_45402_HIRQ2_SAT11_LOCK_CHANGE         0x00001000
#define BHAB_45402_HIRQ2_SAT11_ACQ_DONE            0x00002000
#define BHAB_45402_HIRQ2_SAT11_READY               0x00004000
#define BHAB_45402_HIRQ2_SAT11_SIGNAL_NOTIFICATION 0x00008000
#define BHAB_45402_HIRQ2_SAT12_LOCK_CHANGE         0x00010000
#define BHAB_45402_HIRQ2_SAT12_ACQ_DONE            0x00020000
#define BHAB_45402_HIRQ2_SAT12_READY               0x00040000
#define BHAB_45402_HIRQ2_SAT12_SIGNAL_NOTIFICATION 0x00080000
#define BHAB_45402_HIRQ2_SAT13_LOCK_CHANGE         0x00100000
#define BHAB_45402_HIRQ2_SAT13_ACQ_DONE            0x00200000
#define BHAB_45402_HIRQ2_SAT13_READY               0x00400000
#define BHAB_45402_HIRQ2_SAT13_SIGNAL_NOTIFICATION 0x00800000
#define BHAB_45402_HIRQ2_SAT14_LOCK_CHANGE         0x01000000
#define BHAB_45402_HIRQ2_SAT14_ACQ_DONE            0x02000000
#define BHAB_45402_HIRQ2_SAT14_READY               0x04000000
#define BHAB_45402_HIRQ2_SAT14_SIGNAL_NOTIFICATION 0x08000000
#define BHAB_45402_HIRQ2_SAT15_LOCK_CHANGE         0x10000000
#define BHAB_45402_HIRQ2_SAT15_ACQ_DONE            0x20000000
#define BHAB_45402_HIRQ2_SAT15_READY               0x40000000
#define BHAB_45402_HIRQ2_SAT15_SIGNAL_NOTIFICATION 0x80000000

#define BHAB_45402_HIRQ0_SAT_MASK (BHAB_45402_HIRQ0_SAT_INIT_DONE | BHAB_45402_HIRQ0_SA_DONE)
#define BHAB_45402_HIRQ0_WFE_MASK (BHAB_45402_HIRQ0_WFE0_READY | BHAB_45402_HIRQ0_WFE1_READY)
#define BHAB_45402_HIRQ0_FSK_MASK (BHAB_45402_HIRQ0_FSK_RX | BHAB_45402_HIRQ0_FSK_TX)
#define BHAB_45402_HIRQ0_DSQ_MASK (BHAB_45402_HIRQ0_DSQ0_TX | BHAB_45402_HIRQ0_DSQ0_RX | BHAB_45402_HIRQ0_DSQ0_VSENSE | BHAB_45402_HIRQ0_DSQ1_TX | BHAB_45402_HIRQ0_DSQ1_RX | BHAB_45402_HIRQ0_DSQ1_VSENSE)

/* channel interrupt mask used in BXXX_45402_P_EnableChannelInterrupt() */
#define BHAB_45402_CHAN_IRQ_LOCK_STATUS_CHANGE  0x1
#define BHAB_45402_CHAN_IRQ_ACQ_DONE            0x2
#define BHAB_45402_CHAN_IRQ_READY               0x4
#define BHAB_45402_CHAN_IRQ_SIGNAL_NOTIFICATION 0x8
#define BHAB_45402_CHAN_IRQ_ACQ                 0xB
#define BHAB_45402_CHAN_IRQ_ALL                 0xF
#define BHAB_45402_DSEC_IRQ_TX_DONE             0x1
#define BHAB_45402_DSEC_IRQ_RX                  0x2
#define BHAB_45402_DSEC_IRQ_VSENSE              0x4
#define BHAB_45402_DSEC_IRQ_ALL                 0x7


/***************************************************************************
Summary:
   This struct contains interrupt status information that is passed to the
   callback routine.
Description:
   A pointer to this struct will be passed as the first parameter (void*)
   to the callback routine.
Returns:
   BERR_Code
See Also:
   BHAB_IntCallbackFunc
****************************************************************************/
typedef struct BHAB_45402_IrqStatus
{
   uint32_t status0;
   uint32_t status1;
   uint32_t status2;
   uint32_t status3;
   void     *pParm1;
   int      parm2;
} BHAB_45402_IrqStatus;


/***************************************************************************
Summary:
   This function returns the default settings for the 45402 HAB module.
Description:
   This function returns the default settings for the 45402 HAB module.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45402_GetDefaultSettings(
   BHAB_Settings * pDefSetting     /* [in] Default settings */
);


/***************************************************************************
Summary:
   This function transmits the given string from the 45402 LEAP UART.
Description:
   This function transmits the given string from the 45402 LEAP UART.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BHAB_45402_PrintUart(
   BHAB_Handle h,  /* [in] BHAB handle */
   char *pStr      /* [in] null-terminated string */
);


/***************************************************************************
Summary:
   This function initializes the first 32-bit word of an HAB command.
Description:
   This function initializes the first 32-bit word of an HAB command.
Returns:
   BERR_Code
See Also:
   BHAB_SendHabCommand()
****************************************************************************/
uint32_t BHAB_45402_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module);


#if 0 /* implement later */
/***************************************************************************
Summary:
   This function generates an I2C write transaction from the master I2C controller.
Description:
   This function generates an I2C write transaction from the master I2C controller.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45402_BscWrite(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint8_t channel,      /* [in] BSC channel, 0=BSCA, 1=BSCB, 2=BSCC */
   uint16_t slave_addr,  /* [in] for 7-bit address: bits[6:0]; for 10-bit address: bits[9:0], bit 15 is set  */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint32_t n            /* [in] number of bytes to transmit after the i2c slave address, 0 to 8 */
);


/***************************************************************************
Summary:
   This function generates an I2C read transaction from the master I2C controller.
Description:
   This function generates an I2C read transaction from the master I2C controller.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45402_BscRead(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint8_t channel,      /* [in] BSC channel, 0=BSCA, 1=BSCB, 2=BSCC */
   uint16_t slave_addr,  /* [in] for 7-bit address: bits[6:0]; for 10-bit address: bits[9:0], bit 15 is set  */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit (<=8) before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] stores the data read */
   uint32_t in_n         /* [in] number of bytes to read after the i2c restart condition */
);


/***************************************************************************
Summary:
   Configure the direction (i.e. input or output) of the GPIO pins under host control.
Description:
   The write_mask and read_mask parameters specify the GPIO pins that are
   to be configured as output and input respectively.  Bit x in the mask
   corresponds to GPIO pin x.  The BCM453xx has 20 GPIO pins.  GPIO_0,
   GPIO_1, and GPIO_18 are reserved (i.e. controlled by the firmware).
   This function must be called prior to calling BHAB_45402_GpioWrite or
   BHAB_45402_GpioRead.
   Example:
   BHAB_45402_GpioConfig(0x21C0, 0x8) configures GPIO_13, GPIO_8, GPIO_7,
   and GPIO_6 as output, and GPIO_3 as input.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45402_GpioConfig(
   BHAB_Handle h,       /* [in] BHAB handle */
   uint32_t write_mask, /* [in] bitmask indicating which host-controlled GPIO pins are output */
   uint32_t read_mask   /* [in] bitmask indicating which host-controlled GPIO pins are input */
);


/***************************************************************************
Summary:
   Set the state of GPIO (output) pins.
Description:
   This function has 2 bitmask parameters, pin_mask and state_mask, where
   bit x in the mask corresponds to GPIO (output) pin x.
   The pin_mask parameter specifies the GPIO pins whose output state are to
   be changed.  These pins should have been configured as output in a
   previous call to BHAB_45402_GpioConfig.  The state_mask parameter
   specifies the output state (0=low, 1-high) corresponding to each pin set
   in pin_mask.
   Example:
   // configure GPIO_13, GPIO_8, GPIO_7, and GPIO_6 as output, and GPIO_3 as input.
   BHAB_45402_GpioConfig(0x21C0, 0x8);
   BHAB_45402_GpioWrite(h, 0xC0, 0x80); // set GPIO_6 low and GPIO_7 high
   BHAB_45402_GpioWrite(h, 0x2000, 0x2000);  // set GPIO_13 high
   BHAB_45402_GpioWrite(h, 0x2000, 0);  // set GPIO_13 low
Returns:
   BERR_Code
See Also:
   BHAB_45402_GpioConfig()
****************************************************************************/
BERR_Code BHAB_45402_GpioWrite(
   BHAB_Handle h,      /* [in] BHAB handle */
   uint32_t pin_mask,  /* [in] bitmask indicating which output pins are to be controlled */
   uint32_t state_mask /* [in] state of the output pins specified by pin_mask, 0=low, 1=high */
);


/***************************************************************************
Summary:
   Read the state of GPIO (input) pins.
Description:
   The pin_mask parameter specifies the GPIO (input) pins to read.  Bit x in
   the mask corresponds to GPIO pin x.  These pins should have been
   configured as input in a previous call to BHAB_45402_GpioConfig.  The
   state of each pin specified in pin_mask will be written in the
   corresponding bit in pstate_mask.
   Example:
   // configure GPIO_13, GPIO_8, GPIO_7, and GPIO_6 as output, and GPIO_3 as input.
   BHAB_45402_GpioConfig(0x21C0, 0x8);
   BHAB_45402_GpioRead(0x8, &state_mask); // read GPIO_3
   Bit 3 in state_mask indicates the state of GPIO_3, i.e. bit 3 = 0 means
   GPIO_3 is low and bit 3 = 1 means GPIO_3 is high.
Returns:
   BERR_Code
See Also:
   BHAB_45402_GpioConfig()
****************************************************************************/
BERR_Code BHAB_45402_GpioRead(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint32_t pin_mask,    /* [in] bitmask indicating which input pins are to be read */
   uint32_t *pstate_mask /* [out] state of the pins specified by pin_mask, 0=pin is low, 1=pin is high */
);


/***************************************************************************
Summary:
   Specifies the GPO pins under host control.
Description:
   This function enables and configures GPO pins for host control.  There are
   15 GPO pins in the BCM453xx.  This function must be called prior to calling
   BHAB_45402_GpoWrite.  The write_mask parameter specifies the GPO pin(s)
   that are to be controlled by the host.  The ctl_mask configures the GPO
   pins specified by write_mask to be either CMOS or open-drain.  Bit x in
   the mask parameters corresponds to GPO pin x.
   Example:
   BHAB_45402_GpoConfig(0x7, 0x5) tells the firmware that GPO_0, GPO_1, and
   GPO_2 are to be controlled by the host, and GPO_0 and GPO_2 are open-drain
   and GPO_1 is CMOS.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45402_GpoConfig(
   BHAB_Handle h,       /* [in] BHAB handle */
   uint32_t write_mask, /* [in] bitmask indicating which GPO pins are host-controlled */
   uint32_t ctl_mask    /* [in] GPO configuration bitmask: 0=GPO is CMOS, 1=GPO is open-drain */
);


/***************************************************************************
Summary:
   Set the state of GPO pins.
Description:
   This function has 2 bitmask parameters, pin_mask and state_mask, where
   bit x in the mask corresponds to GPO_x.  The pin_mask parameter specifies
   the GPO pins whose output state are to be changed.  These pins should
   have been configured in a previous call to BHAB_45402_GpoConfig.  The
   state_mask parameter specifies the output state (0=low, 1-high)
   corresponding to each pin set in pin_mask.
   Example:
   BHAB_45402_GpoConfig(0x7, 0x5); // GPO_0, GPO_1, GPO_2 are host-controlled
   BHAB_45402_GpoWrite(0x7, 0x2); // set GPO_0 low, GPO_1 high, GPO_2 low
   BHAB_45402_GpoWrite(0x1, 0); // only set GPO_0 low
Returns:
   BERR_Code
See Also:
   BHAB_45402_GpoConfig()
****************************************************************************/
BERR_Code BHAB_45402_GpoWrite(
   BHAB_Handle h,      /* [in] BHAB handle */
   uint32_t pin_mask,  /* [in] bitmask indicating which GPO pins are to be controlled */
   uint32_t state_mask /* [in] state of the GPO pins specified by pin_mask, 0=low, 1=high */
);
#endif

#ifdef __cplusplus
}
#endif

#endif
