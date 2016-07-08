/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef _BHAB_45308_H_
#define _BHAB_45308_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"


/* HAB header macros */
#define BHAB_45308_ACK            0x80000000
#define BHAB_45308_WRITE 0
#define BHAB_45308_READ 1
#define BHAB_45308_MODULE_LEAP 0
#define BHAB_45308_MODULE_WFE  1
#define BHAB_45308_MODULE_SAT  2
#define BHAB_45308_MODULE_DSQ  3
#define BHAB_45308_MODULE_FSK  4
#define BHAB_45308_MODULE_MXT  5
#define BHAB_45308_MODULE_VSN  6

/* interrupt bits in LEAP_HOST_IRQ_STATUS0 */
#define BHAB_45308_HIRQ0_INIT_DONE            0x00000001
#define BHAB_45308_HIRQ0_HAB_DONE             0x00000002
#define BHAB_45308_HIRQ0_FW_ERROR             0x00000004
#define BHAB_45308_HIRQ0_FLASH_DONE           0x00000008
#define BHAB_45308_HIRQ0_FSK_RX               0x00000010
#define BHAB_45308_HIRQ0_FSK_TX               0x00000020
#define BHAB_45308_HIRQ0_WFE_SA_DONE          0x00000040
#define BHAB_45308_HIRQ0_WFE0_READY           0x00000080
#define BHAB_45308_HIRQ0_WFE1_READY           0x00000100
#define BHAB_45308_HIRQ0_SAT_INIT_DONE        0x00000200
#define BHAB_45308_HIRQ0_DSQ0_TX              0x00000400
#define BHAB_45308_HIRQ0_DSQ0_RX              0x00000800
#define BHAB_45308_HIRQ0_DSQ0_VSENSE          0x00001000
#define BHAB_45308_HIRQ0_DSQ1_TX              0x00002000
#define BHAB_45308_HIRQ0_DSQ1_RX              0x00004000
#define BHAB_45308_HIRQ0_DSQ1_VSENSE          0x00008000
#define BHAB_45308_HIRQ0_SAT10_MISC           0x00010000
#define BHAB_45308_HIRQ0_SAT11_LOCK_CHANGE    0x00020000
#define BHAB_45308_HIRQ0_SAT11_ACQ_DONE       0x00040000
#define BHAB_45308_HIRQ0_SAT11_MISC           0x00080000
#define BHAB_45308_HIRQ0_SAT12_LOCK_CHANGE    0x00100000
#define BHAB_45308_HIRQ0_SAT12_ACQ_DONE       0x00200000
#define BHAB_45308_HIRQ0_SAT12_MISC           0x00400000
#define BHAB_45308_HIRQ0_SAT13_LOCK_CHANGE    0x00800000
#define BHAB_45308_HIRQ0_SAT13_ACQ_DONE       0x01000000
#define BHAB_45308_HIRQ0_SAT13_MISC           0x02000000
#define BHAB_45308_HIRQ0_SAT14_LOCK_CHANGE    0x04000000
#define BHAB_45308_HIRQ0_SAT14_ACQ_DONE       0x08000000
#define BHAB_45308_HIRQ0_SAT14_MISC           0x10000000
#define BHAB_45308_HIRQ0_SAT15_LOCK_CHANGE    0x20000000
#define BHAB_45308_HIRQ0_SAT15_ACQ_DONE       0x40000000
#define BHAB_45308_HIRQ0_SAT15_MISC           0x80000000

#define BHAB_45308_HIRQ0_SAT_MASK             (0xFFFF0000 | BHAB_45308_HIRQ0_SAT_INIT_DONE)
#define BHAB_45308_HIRQ0_WFE_MASK             (BHAB_45308_HIRQ0_WFE_SA_DONE | BHAB_45308_HIRQ0_WFE0_READY | BHAB_45308_HIRQ0_WFE1_READY)
#define BHAB_45308_HIRQ0_FSK_MASK             (BHAB_45308_HIRQ0_FSK_RX | BHAB_45308_HIRQ0_FSK_TX)
#define BHAB_45308_HIRQ0_DSQ_MASK             (BHAB_45308_HIRQ0_DSQ0_TX | BHAB_45308_HIRQ0_DSQ0_RX | BHAB_45308_HIRQ0_DSQ0_VSENSE | BHAB_45308_HIRQ0_DSQ1_TX | BHAB_45308_HIRQ0_DSQ1_RX | BHAB_45308_HIRQ0_DSQ1_VSENSE)

/* interrupt bits in LEAP_HOST_IRQ_STATUS1 */
#define BHAB_45308_HIRQ1_SAT0_LOCK_CHANGE     0x00000001
#define BHAB_45308_HIRQ1_SAT0_ACQ_DONE        0x00000002
#define BHAB_45308_HIRQ1_SAT0_MISC            0x00000004
#define BHAB_45308_HIRQ1_SAT1_LOCK_CHANGE     0x00000008
#define BHAB_45308_HIRQ1_SAT1_ACQ_DONE        0x00000010
#define BHAB_45308_HIRQ1_SAT1_MISC            0x00000020
#define BHAB_45308_HIRQ1_SAT2_LOCK_CHANGE     0x00000040
#define BHAB_45308_HIRQ1_SAT2_ACQ_DONE        0x00000080
#define BHAB_45308_HIRQ1_SAT2_MISC            0x00000100
#define BHAB_45308_HIRQ1_SAT3_LOCK_CHANGE     0x00000200
#define BHAB_45308_HIRQ1_SAT3_ACQ_DONE        0x00000400
#define BHAB_45308_HIRQ1_SAT3_MISC            0x00000800
#define BHAB_45308_HIRQ1_SAT4_LOCK_CHANGE     0x00001000
#define BHAB_45308_HIRQ1_SAT4_ACQ_DONE        0x00002000
#define BHAB_45308_HIRQ1_SAT4_MISC            0x00004000
#define BHAB_45308_HIRQ1_SAT5_LOCK_CHANGE     0x00008000
#define BHAB_45308_HIRQ1_SAT5_ACQ_DONE        0x00010000
#define BHAB_45308_HIRQ1_SAT5_MISC            0x00020000
#define BHAB_45308_HIRQ1_SAT6_LOCK_CHANGE     0x00040000
#define BHAB_45308_HIRQ1_SAT6_ACQ_DONE        0x00080000
#define BHAB_45308_HIRQ1_SAT6_MISC            0x00100000
#define BHAB_45308_HIRQ1_SAT7_LOCK_CHANGE     0x00200000
#define BHAB_45308_HIRQ1_SAT7_ACQ_DONE        0x00400000
#define BHAB_45308_HIRQ1_SAT7_MISC            0x00800000
#define BHAB_45308_HIRQ1_SAT8_LOCK_CHANGE     0x01000000
#define BHAB_45308_HIRQ1_SAT8_ACQ_DONE        0x02000000
#define BHAB_45308_HIRQ1_SAT8_MISC            0x04000000
#define BHAB_45308_HIRQ1_SAT9_LOCK_CHANGE     0x08000000
#define BHAB_45308_HIRQ1_SAT9_ACQ_DONE        0x10000000
#define BHAB_45308_HIRQ1_SAT9_MISC            0x20000000
#define BHAB_45308_HIRQ1_SAT10_LOCK_CHANGE    0x40000000
#define BHAB_45308_HIRQ1_SAT10_ACQ_DONE       0x80000000

#define BHAB_45308_HIRQ1_SAT_MASK             0xFFFFFFFF
#define BHAB_45308_HIRQ1_WFE_MASK             0x00000000
#define BHAB_45308_HIRQ1_FSK_MASK             0x00000000
#define BHAB_45308_HIRQ1_DSQ_MASK             0x00000000

/* LEAP_CTRL_SW_SPARE0 bits */
#define BHAB_45308_SPARE0_CH0_LOCK            0x00000001
#define BHAB_45308_SPARE0_CH1_LOCK            0x00000002
#define BHAB_45308_SPARE0_CH2_LOCK            0x00000004
#define BHAB_45308_SPARE0_CH3_LOCK            0x00000008
#define BHAB_45308_SPARE0_CH4_LOCK            0x00000010
#define BHAB_45308_SPARE0_CH5_LOCK            0x00000020
#define BHAB_45308_SPARE0_CH6_LOCK            0x00000040
#define BHAB_45308_SPARE0_CH7_LOCK            0x00000080
#define BHAB_45308_SPARE0_CH8_LOCK            0x00000100
#define BHAB_45308_SPARE0_CH9_LOCK            0x00000200
#define BHAB_45308_SPARE0_CH10_LOCK           0x00000400
#define BHAB_45308_SPARE0_CH11_LOCK           0x00000800
#define BHAB_45308_SPARE0_CH12_LOCK           0x00001000
#define BHAB_45308_SPARE0_CH13_LOCK           0x00002000
#define BHAB_45308_SPARE0_CH14_LOCK           0x00004000
#define BHAB_45308_SPARE0_CH15_LOCK           0x00008000
#define BHAB_45308_SPARE0_CH0_ACQ_DONE        0x00010000
#define BHAB_45308_SPARE0_CH1_ACQ_DONE        0x00020000
#define BHAB_45308_SPARE0_CH2_ACQ_DONE        0x00040000
#define BHAB_45308_SPARE0_CH3_ACQ_DONE        0x00080000
#define BHAB_45308_SPARE0_CH4_ACQ_DONE        0x00100000
#define BHAB_45308_SPARE0_CH5_ACQ_DONE        0x00200000
#define BHAB_45308_SPARE0_CH6_ACQ_DONE        0x00400000
#define BHAB_45308_SPARE0_CH7_ACQ_DONE        0x00800000
#define BHAB_45308_SPARE0_CH8_ACQ_DONE        0x01000000
#define BHAB_45308_SPARE0_CH9_ACQ_DONE        0x02000000
#define BHAB_45308_SPARE0_CH10_ACQ_DONE       0x04000000
#define BHAB_45308_SPARE0_CH11_ACQ_DONE       0x08000000
#define BHAB_45308_SPARE0_CH12_ACQ_DONE       0x10000000
#define BHAB_45308_SPARE0_CH13_ACQ_DONE       0x20000000
#define BHAB_45308_SPARE0_CH14_ACQ_DONE       0x40000000
#define BHAB_45308_SPARE0_CH15_ACQ_DONE       0x80000000

/* LEAP_CTRL_SPARE1 bits */
#define BHAB_45308_SPARE1_CH0_SIG_NOT         0x00000001
#define BHAB_45308_SPARE1_CH0_READY           0x00000002
#define BHAB_45308_SPARE1_CH1_SIG_NOT         0x00000004
#define BHAB_45308_SPARE1_CH1_READY           0x00000008
#define BHAB_45308_SPARE1_CH2_SIG_NOT         0x00000010
#define BHAB_45308_SPARE1_CH2_READY           0x00000020
#define BHAB_45308_SPARE1_CH3_SIG_NOT         0x00000040
#define BHAB_45308_SPARE1_CH3_READY           0x00000080
#define BHAB_45308_SPARE1_CH4_SIG_NOT         0x00000100
#define BHAB_45308_SPARE1_CH4_READY           0x00000200
#define BHAB_45308_SPARE1_CH5_SIG_NOT         0x00000400
#define BHAB_45308_SPARE1_CH5_READY           0x00000800
#define BHAB_45308_SPARE1_CH6_SIG_NOT         0x00001000
#define BHAB_45308_SPARE1_CH6_READY           0x00002000
#define BHAB_45308_SPARE1_CH7_SIG_NOT         0x00004000
#define BHAB_45308_SPARE1_CH7_READY           0x00008000
#define BHAB_45308_SPARE1_CH8_SIG_NOT         0x00010000
#define BHAB_45308_SPARE1_CH8_READY           0x00020000
#define BHAB_45308_SPARE1_CH9_SIG_NOT         0x00040000
#define BHAB_45308_SPARE1_CH9_READY           0x00080000
#define BHAB_45308_SPARE1_CH10_SIG_NOT        0x00100000
#define BHAB_45308_SPARE1_CH10_READY          0x00200000
#define BHAB_45308_SPARE1_CH11_SIG_NOT        0x00400000
#define BHAB_45308_SPARE1_CH11_READY          0x00800000
#define BHAB_45308_SPARE1_CH12_SIG_NOT        0x01000000
#define BHAB_45308_SPARE1_CH12_READY          0x02000000
#define BHAB_45308_SPARE1_CH13_SIG_NOT        0x04000000
#define BHAB_45308_SPARE1_CH13_READY          0x08000000
#define BHAB_45308_SPARE1_CH14_SIG_NOT        0x10000000
#define BHAB_45308_SPARE1_CH14_READY          0x20000000
#define BHAB_45308_SPARE1_CH15_SIG_NOT        0x40000000
#define BHAB_45308_SPARE1_CH15_READY          0x80000000

/* LEAP_CTRL_SPARE2 bits */
/* TBD... */

/* LEAP_CTRL_SPARE3 bits */
#define BHAB_45308_SPARE3_FW_ERROR_CODE_MASK     0x0000000F
#define BHAB_45308_SPARE3_INIT_DONE              0x00008000
#define BHAB_45308_SPARE3_LAST_HAB_CMD_RCVD_MASK 0x00FF0000
#define BHAB_45308_SPARE3_LAST_HAB_CMD_SVCD_MASK 0xFF000000


/* channel interrupt mask used in BXXX_45308_P_EnableChannelInterrupt() */
#define BHAB_45308_CHAN_IRQ_LOCK_STATUS_CHANGE  0x1
#define BHAB_45308_CHAN_IRQ_ACQ_DONE            0x2
#define BHAB_45308_CHAN_IRQ_MISC                0x4
#define BHAB_45308_CHAN_IRQ_ACQ                 0x7
#define BHAB_45308_CHAN_IRQ_ALL                 0x7
#define BHAB_45308_DSEC_IRQ_TX_DONE             0x1
#define BHAB_45308_DSEC_IRQ_RX                  0x2
#define BHAB_45308_DSEC_IRQ_VSENSE              0x4
#define BHAB_45308_DSEC_IRQ_ALL                 0x7


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
typedef struct BHAB_45308_IrqStatus
{
   uint32_t status0;
   uint32_t status1;
   void     *pParm1;
   int      parm2;
} BHAB_45308_IrqStatus;


/***************************************************************************
Summary:
   This function returns the default settings for the 45308 HAB module.
Description:
   This function returns the default settings for the 45308 HAB module.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45308_GetDefaultSettings(
   BHAB_Settings * pDefSetting     /* [in] Default settings */
);


/***************************************************************************
Summary:
   This function transmits the given string from the 45308 LEAP UART.
Description:
   This function transmits the given string from the 45308 LEAP UART.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BHAB_45308_PrintUart(
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
uint32_t BHAB_45308_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module);


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
BERR_Code BHAB_45308_BscWrite(
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
BERR_Code BHAB_45308_BscRead(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint8_t channel,      /* [in] BSC channel, 0=BSCA, 1=BSCB, 2=BSCC */
   uint16_t slave_addr,  /* [in] for 7-bit address: bits[6:0]; for 10-bit address: bits[9:0], bit 15 is set  */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit (<=8) before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] stores the data read */
   uint32_t in_n         /* [in] number of bytes to read after the i2c restart condition */
);

#ifdef __cplusplus
}
#endif

#endif
