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
 ******************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ML507 SPI private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_ML507_PRIVATE_SPI_H
#define _BB_ML507_PRIVATE_SPI_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Group of ML507 SPI unit registers.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses SPI ON FPGA, PORT PINS ON ML507.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.1, figures 6-2, 6-3, table 12-4.
 */
/**@{*/
/**//**
 * \brief   ML507 SPI registers base.
 * \details The BASE address is used in conjunction with all the rest macros (used as indexes) to obtain absolute
 *  addresses of corresponding registers.
 * \note    The SPI_NSEL register is the only 8-bit register while all others are 32-bit. Use conversion to 8-bit-wide
 *  register pointer of the BASE to access the NSEL register.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
#define ML507_REG__SPI_BASE         (0x00C00000)

/**//**
 * \brief   ML507 SPI Configuration register, shift to 32-bit base.
 * \details The SPI_CFG value contains the following fields:
 *  - bits 7..0 - SCLK[7..0] - SCLK clock divider.
 *  - bits 12..8 - B2B[4..0] - Byte-to-byte delay on MOSI to MISO.
 *  - bits 18..13 - I2B[5..0] - Idle-to-busy delay on /SEL.
 *  - bit 19 - ENABLE[0] - SPI output enable.
 *  - bits 23..20 - Reserved[3..0].
 *  - bit 24 - NSEL[0] - /SEL pin level control.
 *  - bits 31..25 - Reserved[6..0].
 *
 * \details The SPI SCLK clock is derived from the main clock dividing it by the SCLK[7..0] factor according to the
 *  formula: f_SPI [Hz] = f_main [Hz] / (2 * (SCLK[7..0] + 1)). For example, to come with f_SPI = 6.75 MHz one has to
 *  assign SCLK[7..0] = 3 at f_main = 54 MHz, or SCLK[7..0] = 1 at f_main = 27 MHz. Valid values for SCLK[7..0] are from
 *  0 to 255, default 60. Indeed, the SCLK[7..0] specifies the number of main clock cycles (plus one) that produces one
 *  switch of the SPI clock (i.e., the half-period).
 * \note    For the case of Atmel AT86RF, if connected via SPI and having separate clocking with the CPU (asynchronous
 *  mode), the SPI SCLK must not overcome 7.5 MHz (f_async.max).
 * \details The byte-to-byte delay is a pause in SPI clocking automatically inserted by the SPI master (the ML507) after
 *  transmission of each byte prior to proceed with transmission of the next byte. This pause is necessary for the SPI
 *  slave (the Atmel AT86RF) to be able to process the just received byte on the MOSI and assert the byte of response on
 *  the MISO. This delay is performed by the SPI master as the number of additional main clock cycles forming the
 *  passive half-period of the SPI SCLK prior to assert it to active when transmitting the first bit (the MSB) of each
 *  byte on MOSI. Notice, that the active half-period of the SPI SCLK is not affected by this value. If B2B[4..0] is
 *  between 1 and SCLK[7..0] there is no additional delay. If B2B[4..0] is greater than SCLK[7..0] the additional delay
 *  equals (B2B - SCLK) main clock cycles. For example, to come with 112 ns byte-to-byte delay, which equals to 7 main
 *  clock cycles at 54 MHz or 4 main clock cycles at 27 MHz, one has to assign B2B[4..0] = 10 at f_main = 54 MHz (take
 *  into account that SCLK[7..0] was assigned with 3, so 10 is obtained as 7 + 3), or B2B[4..0] = 5 at f_main = 27 MHz
 *  (take into account that SCLK[7..0] was assigned with 1, so 5 is obtained as 4 + 1). Valid values for B2B[4..0] are
 *  from 1 to 31, default 31. Zero is not valid. Values from 1 to SCLK[7..0] produces zero byte-to-byte delay.
 * \note    For the case of Atmel AT86RF the lowest byte-to-byte delay may be expressed as (t5 + t3 - t_SPI) = 112 ns,
 *  where t5.min = 250 ns, t3.min = 10 ns, t_SPI = 148 ns.
 * \details The idle-to-busy delay is the minimum allowed SPI idle time - i.e., the minimum rising to falling edge delay
 *  at the /SEL pin performed by the SPI master (the ML507) between consecutive SPI accesses. This delay is performed by
 *  the SPI master as the number of main clock cycles forming the minimum guaranteed IDLE period. For example, to come
 *  with guaranteed 250 ns IDLE period, one has to assign I2B[5..0] = 14 at f_main = 54 MHz, or I2B[5..0] = 7 at
 *  f_main = 27 MHz. Take into account that the actual IDLE period length depends also on the instructions flow
 *  controlling the /SEL pin state and may easily overcome the minimum value guaranteed by the SPI hardware. Due to this
 *  reason the I2B values may be significantly reduced to I2B[5..0] = 4 at f_main = 54 MHz, or I2B[5..0] = 1 at
 *  f_main = 27 MHz. This delay does not depend on SCLK[7..0] and B2B[4..0] settings.
 * \note    For the case of Atmel AT86RF the lowest idle-to-busy delay is given with t8.min = 250 ns.
 * \details The SPI unit automatically guarantees the "Last SCLK to /SEL high" (t9 = 250 ns, for Atmel AT86RF) and "/SEL
 *  low to MISO active" (t1 = 180 ns, for Atmel AT86RF).
 * \details To enable SPI unit outputs assert the ENABLE[0] = 1. The default value is 0.
 * \details To drive the /SEL pin to active Low (BUSY) assert NSEL[0] = 0, and to drive the /SEL pin to passive High
 *  (IDLE) assert NSEL[0] = 1. The default value is 1.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA, PORT PINS ON ML507.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.1, figures 6-2, 6-3, table 12-4.
 */
#define ML507_REG__SPI_CFG          ((0x00C00000 - ML507_REG__SPI_BASE) / 4)

/**//**
 * \brief   ML507 SPI /SEL pin register, shift to 8-bit base.
 * \details The SPI_NSEL value contains the following fields:
 *  - bit 0 - NSEL[0] - /SEL pin level control.
 *  - bits 7..1 - Reserved[6..0].
 *
 * \details This register just provides the 8-bit access to the NSEL field of the SPI_CFG register.
 * \details To drive the /SEL pin to active Low (READY/BUSY) assert NSEL[0] = 0, and to drive the /SEL pin to passive
 *  High (IDLE) assert NSEL[0] = 1. The default value is 1.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA, PORT PINS ON ML507.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.1, figures 6-2, 6-3, table 12-4.
 */
#define ML507_REG__SPI_NSEL         ((0x00C00003 - ML507_REG__SPI_BASE) / 1)

/**//**
 * \brief   ML507 SPI Status register, shift to 32-bit base.
 * \details The SPI_STATUS value contains the following fields:
 *  - bit 0 - READY[0] - SPI Master FSM is READY to perform the next operation.
 *  - bit 1 - IDLE[0] - SPI Bus is in the IDLE state. Set in (I2B[5..0] * f_main) after asserting /SEL = 1 (passive).
 *  - bits 7..2 - Reserved[5..0].
 *
 * \note    Skip one cycle (or just read and ignore STATUS.READY[0] once) after a Write or Read operation is requested
 *  prior to analyze the STATUS.READY[0] state. The very first attempt to read this register returns READY instead of
 *  BUSY due to the internal propagation delay of the BUSY signal.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
#define ML507_REG__SPI_STATUS       ((0x00C00004 - ML507_REG__SPI_BASE) / 4)

/**//**
 * \brief   ML507 SPI Reset register, shift to 32-bit base.
 * \details The SPI_NRST value contains the following fields:
 *  - bit 0 - NRST[0] - SPI FIFO internal RESET signal. Active Low (0), passive High (1).
 *  - bits 7..1 - Reserved[6..0].
 *
 * \details To reset the SPI FIFO assert the NRST[0] = 0 and then set NRST[0] = 1. The SPI FIFO must be reset prior to
 *  be used after switching SPI from IDLE to READY (i.e., asserting NSEL[0] = 0).
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
#define ML507_REG__SPI_NRST         ((0x00C00008 - ML507_REG__SPI_BASE) / 4)

/**//**
 * \brief   ML507 SPI FIFO register, shift to 32-bit base.
 * \details The SPI_FIFO value contains the following fields:
 *  - bits 7..0 - FIFO[7..0] - SPI FIFO Read/Write data.
 *
 * \details To put data on MOSI (Master Output Slave Input pin) write the array of bytes (up to 256 bytes, at least one
 *  byte) into the FIFO[7..0] register and then assign CTRL[0] = 0. All the data will be automatically clocked out on
 *  MOSI by the SPI unit. The next portion of data may be put after the STATUS.READY[0] became 1. If there were data
 *  from MISO (Master Input Slave Output pin) clocked in, it may be read from the FIFO[7..0] by bytes one-by-one as soon
 *  as the STATUS.READY[0] became 1.
 * \details To get N bytes of data from MISO separately, assign the CTRL[0] = N, where N is from 1 to 256. Then read the
 *  received data from FIFO[7..0] by bytes one-by-one as soon as the STATUS.READY[0] became 1. The next portion may be
 *  read after exactly all the N previously received bytes were read by the software from FIFO[7..0]. In this case the
 *  MOSI pin state must be ignored by the Slave.
 * \details Read and Write operations may be mixed in a single run or sequentially. For example, one may need to put two
 *  bytes on MOSI for the first, and read the first byte received from MISO at the same time, and then to read another N
 *  bytes. To do this: (1) wait for STATUS.IDLE[0] to become 1, assert NSEL[0] = 0 to switch SPI into the READY state,
 *  assert NRST[0] = 0 and then NRST[0] = 1 to reset the SPI FIFO, (2) put two bytes for MOSI into the FIFO[7..0]
 *  one-by-one, assign CTRL[0] = 0 to perform the Write operation with the whole FIFO (with two bytes contained in it),
 *  read and ignore STATUS.READY[0] once, wait for STATUS.READY[0] to become 1, (3) read the first byte, received from
 *  MISO simultaneously with writing two bytes on MOSI, from FIFO[7..0] and save its value, read the second received
 *  byte from FIFO[7..0] and ignore it (it is necessary to synchronize the FIFO), (4) assign CTRL[0] = N to perform the
 *  Read operation for N bytes from MISO, read and ignore STATUS.READY[0] once, wait for STATUS.READY[0] to become 1,
 *  (5) read N received bytes from FIFO[7..0] one-by-one and save them, (6) assert NSEL[0] = 1 to return SPI into the
 *  IDLE state.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA, PORT PINS ON ML507.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.1, figures 6-2, 6-3, table 12-4.
 */
#define ML507_REG__SPI_FIFO         ((0x00C0000C - ML507_REG__SPI_BASE) / 4)

/**//**
 * \brief   ML507 SPI Control register, shift to 32-bit base.
 * \details The SPI_CTRL value contains the following fields:
 *  - bits 7..0 - CTRL[7..0] - SPI Command register.
 *
 * \details Assign CTRL[7..0] = 0 to commence the Write operation. All the data previously put into the SPI FIFO (up to
 *  256 bytes) will be clocked out on MOSI. Simultaneously the same number of bytes will be clocked in from the MISO and
 *  may be read from FIFO[7..0] one-by-one (in general it must be ignored by the Master).
 * \details Assign CTRL[7..0] = N, where N is from 1 to 255, to commence the Read operation. The data clocked in from
 *  the MISO is put into the SPI FIFO and may be further read from FIFO[7..0] one-by-one. Simultaneously MOSI clocks out
 *  the data previously put into the SPI FIFO (in general it must be ignored by the Slave).
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
#define ML507_REG__SPI_CTRL         ((0x00C00010 - ML507_REG__SPI_BASE) / 4)
/**@}*/

/**//**
 * \name    Enumeration of the ML507 SPI unit statuses.
 * \details These values describes different SPI unit statuses read from the STATUS register. SPI unit may persist in
 *  three different states:
 *  - IDLE (0x3) - SPI Master FSM is free, and /SEL pin is High (passive).
 *  - READY (0x1) - SPI Master FSM is free and ready to perform the next operation, /SEL pin is Low (active).
 *  - BUSY (0x0) - SPI Master FSM is performing the requested operation on MOSI and MISO pins, SCLK is clocking.
 *
 * \details To commence a single SPI transaction the SPI Master has to be switched from IDLE to READY. It is performed
 *  by asserting NSEL[0] = 0. To finalize (or abort) an SPI transaction the SPI Master has to be switched from READY to
 *  IDLE. It is performed by asserting NSEL[0] = 1. Within a single transaction a number of Write/Read operations may be
 *  performed. To start the next operation (or to finalize, or abort the transaction) the software has to wait for the
 *  SPI Master FSM to enter the READY state. A next operation is started by assigning the CTRL[7..0] register either
 *  with 0 or N. As soon as a next operation is started, the SPI Master FSM enters the BUSY state (skip one cycle to
 *  compensate the internal propagation delay of the STATUS). When the requested operation is accomplished, the SPI
 *  Master FSM returns to the READY state.
 * \note    Skip one cycle (or just read and ignore STATUS.READY[0] once) after a Write or Read operation is requested
 *  prior to analyze the STATUS.READY[0] state. The very first attempt to read this register returns READY instead of
 *  BUSY due to the internal propagation delay of the BUSY signal.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
enum ML507_SPI__STATUS_Code_t {
    ML507_SPI_STATUS__BUSY      = 0x0,      /*!< SPI is busy with transferring data. */
    ML507_SPI_STATUS__READY     = 0x1,      /*!< SPI is idle but /SEL = 0 (active). */
    ML507_SPI_STATUS__IDLE      = 0x3,      /*!< SPI is idle and /SEL = 1 (passive). */
};

/**//**
 * \name    Enumeration of the ML507 SPI unit control commands.
 * \details These values describes different SPI unit commands assigned to the CTRL register. This register accepts the
 *  following commands:
 *  - WRITE (0x0) - write to MOSI all the data previously put into the SPI FIFO.
 *  - READ (N, codes from 1 to 255) - read N bytes from MISO into the SPI FIFO.
 *
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SPI ON FPGA.
 */
enum ML507_SPI__CTRL_Code_t {
    ML507_SPI_CTRL__WRITE   = 0,        /*!< Write all bytes from FIFO to MOSI line. */
};

#endif /* _BB_ML507_PRIVATE_SPI_H */
