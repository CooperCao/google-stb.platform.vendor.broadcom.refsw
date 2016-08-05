/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include <stdio.h>
#include "bsu_prompt.h"
#include "common2.h"
#include "uart.h"
#include "pcie_dma.h"
#include "cache_ops.h"

// Avoid compiling lib_print for now
// #include "lib_printf.h"

//#define DEBUG_PCIE_DMA
//#define DEBUG_PCIE_TO_MEM

#include "bchp_common.h"
#include "bchp_pcie_0_ext_cfg.h"
#include "bchp_hif_top_ctrl.h"
#include "bchp_pcie_0_misc.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_timer.h"

#define DO_WATCHDOG
#define DO_UART0
//#define DO_UART1
#define SDRAM_MEM_BASE 0x0
#define DEBUG_BUFFER (SDRAM_MEM_BASE + 	    0x800000)
#define DEBUG_BUFFER_INDEX (SDRAM_MEM_BASE + 0x7ffffc)

// mbox logical addr (used in test driver)
#define MBOX_CMD	0x40
#define MBOX_STS	0x80

#define MBOX_CMD_0      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x00))
#define MBOX_CMD_1      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x04))
#define MBOX_CMD_2      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x08))
#define MBOX_CMD_3      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x0c))
#define MBOX_CMD_4      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x10))
#define MBOX_CMD_5      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x14))
#define MBOX_CMD_6      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x18))
#define MBOX_CMD_7      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x1c))
#define MBOX_CMD_8      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x20))
#define MBOX_CMD_9      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x24))
#define MBOX_CMD_10     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x28))
#define MBOX_CMD_11     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x2c))
#define MBOX_CMD_12     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x30))
#define MBOX_CMD_13     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x34))
#define MBOX_CMD_14     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x38))
#define MBOX_CMD_15     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_CMD + 0x3c))

#define MBOX_STS_0      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x00))
#define MBOX_STS_1      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x04))
#define MBOX_STS_2      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x08))
#define MBOX_STS_3      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x0c))
#define MBOX_STS_4      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x10))
#define MBOX_STS_5      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x14))
#define MBOX_STS_6      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x18))
#define MBOX_STS_7      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x1c))
#define MBOX_STS_8      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x20))
#define MBOX_STS_9      (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x24))
#define MBOX_STS_10     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x28))
#define MBOX_STS_11     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x2c))
#define MBOX_STS_12     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x30))
#define MBOX_STS_13     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x34))
#define MBOX_STS_14     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x38))
#define MBOX_STS_15     (volatile uint32 *) (SDRAM_MEM_BASE | (MBOX_STS + 0x3c))

#define STS_DONE        (uint32) 0x600d0000
#define STS_BAD         (uint32) 0xbadd0000
#define STS_QUIT        (uint32) 0xffff0000
#define CMD_MASK        (uint32) 0xffffff00

#define CMD_VALID       (uint32) 0x0000ab00

// to MIPS
#define CMD_HELLO       (uint32) 0x00
#define CMD_QUIT	(uint32) 0x01
#define CMD_READ32      (uint32) 0x02
#define CMD_READ16      (uint32) 0x03
#define CMD_READ8       (uint32) 0x04
#define CMD_WRITE32     (uint32) 0x05
#define CMD_WRITE16     (uint32) 0x06
#define CMD_WRITE8      (uint32) 0x07
#define CMD_CLR_MEM     (uint32) 0x08
#define CMD_FILL_MEM    (uint32) 0x09
#define CMD_CHECK_MEM   (uint32) 0x0a
#define CMD_MARCH_MEM   (uint32) 0x0b
#define CMD_ACCESS_SWEEP (uint32) 0x0c
#define CMD_MIPS_DMA_WR (uint32) 0x0d
#define CMD_MIPS_DMA_RD (uint32) 0x0e
#define CMD_MARCH_MEM_DEBUG   	(uint32) 0x0f
#define CMD_WLAN_ACCESS_SWEEP 	(uint32) 0x10
#define CMD_PCIE_MEM_PERF 	(uint32) 0x11
#define CMD_MIPS_DMA_PERF 	(uint32) 0x12
#define CMD_PCIE_DMA_PERF 	(uint32) 0x13

#define TIMER_RESET     	0x3FFFFFFF
#define TIMER_START     	0x80000000
#define TIMER_STOP      	0x7FFFFFFF

#define WATCHDOG_START0		0xFF00
#define WATCHDOG_START1		0x00FF
#define WATCHDOG_STOP0 		0xEE00
#define WATCHDOG_STOP1 		0x00EE
#define WATCHDOG_TIMEOUT 	0x608F3D00 	// 60s at 27Mhz

#ifdef ARM_TLB
void map_pcie_extended_space (void );
#endif

void dvt_cpu_main(void);
void cmd_loop (void);

void delay(uint32 dly);
uint32 pcie_swap(uint32 data, uint32 swap);
void srand32( uint32 seed );
uint32 rand32( void );

uint32 virt2phys( uint32 addr );

uint32 cmd_hello     (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_read32    (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_read16    (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_read8     (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_write32   (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_write16   (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_write8    (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_clr_mem   (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_fill_mem  (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_check_mem (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_march_mem (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_mips_dma_wr (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_mips_dma_rd (volatile uint32 *cmd, volatile uint32 *sts);

uint32 cmd_wlan_access_sweep (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_pcie_mem_perf (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_mips_dma_perf (volatile uint32 *cmd, volatile uint32 *sts);
uint32 cmd_pcie_dma_perf (volatile uint32 *cmd, volatile uint32 *sts);


void execute_uart_cmd( uint32 chan, char *s );
void uart_prompt( uint32 chan );
void sw_master_reset( void );
//=============================================================================
// Main setups up TLB mapping useful for PCIe block testing,
// and calls the command loop. See comments for command loop (and commands)
// for more information.
//=============================================================================

void dvt_cpu_main( ){

  srand32( 1L );

  clear_all_d_cache();
  invalidate_all_i_cache();
  disable_caches();

#ifdef ARM_TLB
  /* map_pcie_extended_space (); */
#endif

  cmd_loop ();


}


//=============================================================================
// Helper functions
//=============================================================================

// swaps data within a word
//   0=no swap, 1=half-word swap, 2=byte swap, 3 and above=no swap
uint32 pcie_swap(uint32 data, uint32 swap)
{
  uint32 new_data, upr, lwr, b0, b1, b2, b3;

  if (swap==0) {   // quick path
    new_data = data;
  } else if (swap==1) {
    upr = (data >> 16) & 0xffff;
    lwr = (data      ) & 0xffff;
    new_data = (lwr<<16) | upr;
  } else if (swap==2) {
    b3 = (data>>24) & 0xff;
    b2 = (data>>16) & 0xff;
    b1 = (data>> 8) & 0xff;
    b0 = (data    ) & 0xff;
    new_data = (b0<<24) | (b1<<16) | (b2<<8) | b3;
  } else {
    new_data  = data;
  }

  return (new_data);

}

// ----------------------------------------------------------------
// sub      srand32(seed)
// function rand32()
//
// Parameters
//    seed  ... initialize the LFSR CRC to the value of seed
//
// Description
//    Subroutine srand32 seeds the LFSR CRC to an initial value, and
//    subsequent calls to rand32 returns random values generated from the LFSR
//
// ----------------------------------------------------------------

#define  CRC32_POLY 0x04c11db7
uint32 g_crc;
void srand32 (uint32 seed)
{
  g_crc = seed;
}

uint32 rand32 (void)
{
  uint32 v;
  v = g_crc;

  if (g_crc &  0x80000000)
    g_crc = ((g_crc & 0x7fffffff)<<1) ^ CRC32_POLY;
  else
    g_crc <<= 1;

  return(v);
}

//=============================================================================
// Command Functions
//=============================================================================

// ---------------------------------------------------------------------
// cmd_hello:
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       This command is used just as a 'ping' to confirm the CPU command
//       server is operational. It copies input parameter cmd[1] to
//       output return space sts[0]. There is no failure mechanism and so
//       the command always returns successfully.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------


uint32 cmd_hello  (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 parm1;

  parm1 = cmd[1];
  sts[0] = parm1;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_read32
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 32-bit read to the address specified in cmd[1].
//       The result is placed in sts[0].
//
//       A check is performed to confirm the address is 32-bit aligned,
//       and the function returns with a failure if not.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------


uint32 cmd_read32 (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 data;

  addr = cmd[1];

  if ((addr & 3) != 0)
    return 1;

  data = *(uint32 volatile *)addr;
  sts[0] = data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_read16
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 16-bit read to the address specified in cmd[1].
//       The result zero-extended and placed in sts[0].
//
//       A check is performed to confirm the address is 16-bit aligned,
//       and the function returns with a failure if not.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_read16 (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint16 data;

  addr = cmd[1];

  if ((addr & 1) != 0)
    return 1;

  data = *(uint16 volatile *)addr;
  sts[0] = (uint32)data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_read8
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 8-bit read to the address specified in cmd[1].
//       The result zero-extended and placed in sts[0].
//
//       No checks are performed; the function always returns success.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------


uint32 cmd_read8 (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint8 data;

  addr = cmd[1];
  data = *(uint8 volatile *)addr;
  sts[0] = (uint32)data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_write32
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 32-bit write to the address specified in cmd[1],
//       with write data specified in cmd[2].
//
//       A check is performed to confirm the address is 32-bit aligned,
//       and the function returns with a failure if not.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------


uint32 cmd_write32 (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 data;
  ((void)sts); /* unused for now */

  addr = cmd[1];

  if ((addr & 3) != 0)
    return 1;

  data = cmd[2];
  *((uint32 volatile *)addr) = data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_write16
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 16-bit write to the address specified in cmd[1],
//       with lower justified write data specified in cmd[2].
//
//       A check is performed to confirm the address is 16-bit aligned,
//       and the function returns with a failure if not.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_write16 (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 data;
  ((void)sts); /* unused for now */

  addr = cmd[1];

  if ((addr & 1) != 0)
    return 1;

  data = cmd[2];
  *((uint16 volatile *)addr) = (uint16)data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_write8
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a 8-bit write to the address specified in cmd[1],
//       with lower justified write data specified in cmd[2].
//
//       No checks are performed; the function always returns success.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_write8  (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 data;
  ((void)sts); /* unused for now */

  addr = cmd[1];
  data = cmd[2];
  *((uint8 volatile *)addr) = (uint8)data;

  return 0;
}

// ---------------------------------------------------------------------
// cmd_clr_mem
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Clears a range of memory specified by lower address cmd[1], and
//       number of bytes cmd[2]. Write data is specified in cmd[3].
//       32-bit writes are performed.
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_clr_mem (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 nbytes;
  uint32 data;
  uint32 i;
  ((void)sts); /* unused for now */

  addr   = cmd[1];
  nbytes = cmd[2];

  if ( ((addr & 3) != 0) ||
       ((nbytes & 3) != 0) ||
       (nbytes > 0x10000000))
    return 1;

  data   = cmd[3];
  for (i=0; i<nbytes; i+=4, addr+=4) {
    *((volatile uint32*) addr) = data;
  }

  return 0;
}


// ---------------------------------------------------------------------
// cmd_fill_mem
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Writes a range of memory specified by lower address cmd[1], and
//       number of bytes cmd[2]. Write data is a 16-bit incrementing pattern:
//       0x00010000, 0x00030002, etc. 32-bit writes are performed.
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_fill_mem (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 nbytes;
  uint32 data;
  uint32 i, lwr, upr;
  ((void)sts); /* unused for now */

  addr   = cmd[1];
  nbytes = cmd[2];

  if ( ((addr & 3) != 0) ||
       ((nbytes & 3) != 0) ||
       (nbytes > 0x10000000))
    return 1;

  lwr    = 0x0000;
  upr    = 0x0001;
  for (i=0; i<nbytes; i+=4, addr+=4) {
    data = (upr<<16) | lwr;
    *((volatile uint32*) addr) = data;
    lwr = (lwr+2) & 0xffff;
    upr = (upr+2) & 0xffff;
  }

  return 0;
}

// ---------------------------------------------------------------------
// cmd_check_mem
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Verifies a range of memory specified by lower address cmd[1], and
//       number of bytes cmd[2]. Expected data is a 16-bit incrementing pattern:
//       0x00010000, 0x00030002, etc. 32-bit reads are performed.
//       A swap mode is specified in cmd[3] - 0=no swap, 1=16-bit swap, 2=byte swap, 3-=no swap
//       (A swap mode in the check is for convenience - this routine has traditionally
//       been used to verify DMA data transfers that can have a built-in swap).
//       The result is placed in sts[0], 0(pass) or 1(fail).
//       Upon a failure, the address is placed into sts[1], expected data in sts[2], received in sts[3].
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_check_mem (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr;
  uint32 nbytes;
  uint32 swap;
  uint32 i, data, lwr, upr, exp, rec, status;

  addr   = cmd[1];
  nbytes = cmd[2];

  if ( ((addr & 3) != 0) ||
       ((nbytes & 3) != 0) ||
       (nbytes > 0x10000000))
    return 1;

  swap   = cmd[3];
  lwr    = 0x0000;
  upr    = 0x0001;

  status = 0;
  for (i=0; i<nbytes; i+=4, addr+=4) {
    data = (upr<<16) | lwr;
    exp  = pcie_swap(data, swap);
    rec  = *((volatile uint32*) addr);
    if (exp != rec) {
      status = 1;
      break;
    }
    lwr = (lwr+2) & 0xffff;
    upr = (upr+2) & 0xffff;
  }


  sts[0] = status;
  if (status) {
    sts[1] = addr;
    sts[2] = exp;
    sts[3] = rec;
  }

  return 0;
}

// ---------------------------------------------------------------------
// cmd_march_mem
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a half-march data memory test over the range of memory specified by lower address cmd[1], and
//       number of bytes cmd[2]. The 'zero' data is specified in cmd[3].
//       Algorithm has been modified to alternate data (0->ZERO, 4->ONE, etc.)
//       (could be reviewed ... weighing datapath regularity vs. address pin failure detection.)
//
//       Algorithm is as follows:
//
//          Phase 1:  wrdata = ZERO
//                    For address = addr to (addr+size-4), step 4
//                       *address = wrdata   // 32-bit write
//                       wrdata   = ~wrdata  // checkerboard
//
//          Phase 2:  expected = ZERO
//                    For address = addr to (addr+size-4), step 4
//                       received = *address    // 32-bit read
//                       if (expected <> received) exit with error
//                       expected = ~expected   // checkerboard
//
//          Phase 3:  expected = last data // value written to upper address
//                    For address = (addr+size-4) to addr, step-4
//                       received = *address    // 32-bit read
//                       if (expected <> received) exit with error
//                       expected = ~expected   // checkerboard
//
//       The result is placed in sts[0], 0(pass), 1(fail phase2), 2(fail phase3)
//       Upon a failure, the address is placed into sts[1], expected data in sts[2], received in sts[3].
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_march_mem (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 start_addr;
  uint32 nbytes;
  uint32 zero_data;
  uint32 i, addr, data, exp, rec, status;

  start_addr = cmd[1];
  nbytes     = cmd[2];

  if ( ((start_addr & 3) != 0) ||
       ((nbytes & 3) != 0) ||
       (nbytes > 0x10000000))
    return 1;


  zero_data  = cmd[3];

  // phase 1, write the range with ZERO data (checkerboard)
  addr = start_addr;
  data = zero_data;
  for (i=0; i<nbytes; i+=4, addr+=4) {
    *((volatile uint32*) addr) = data;
    data = ~data;
  }

  // phase 2, read/verify ZEROS, write ONES data (checkerboard), bottom up order
  status = 0;
  addr   = start_addr;
  exp    = zero_data;
  for (i=0; i<nbytes; i+=4, addr+=4) {
    rec  = *((volatile uint32*) addr);
    if (exp != rec) {
      status = 1;     // fail phase 2
      break;
    } else {
      *((volatile uint32*) addr) = ~exp;
      exp = ~exp;
    }
  }

  if (status == 0) {
    // phase 3, read/verify ONES, write ZEROs data (checkerboard), top down order
    addr   = start_addr;
    addr   = addr + nbytes - 4;
    // exp is set correctly to very upper most location
    for (i=0; i<nbytes; i+=4, addr-=4) {
      rec  = *((volatile uint32*) addr);
      if (exp != rec) {
        status = 2;                     // fail phase 3
        break;
      } else {
        *((volatile uint32*) addr) = ~exp;
        exp = ~exp;
      }
    }
  }

  sts[0] = status;
  if (status) {
    sts[1] = addr;
    sts[2] = exp;
    sts[3] = rec;
  }

  return 0;
}

// ---------------------------------------------------------------------
// cmd_march_mem_debug
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs a half-march data memory test over the range of memory specified by lower address cmd[1], and
//       number of bytes cmd[2]. The 'zero' data is specified in cmd[3].
//       Algorithm has been modified to alternate data (0->ZERO, 4->ONE, etc.)
//       (could be reviewed ... weighing datapath regularity vs. address pin failure detection.)
//
//       Algorithm is as follows:
//
//          Phase 1:  wrdata = ZERO
//                    For address = addr to (addr+size-4), step 4
//                       *address = wrdata   // 32-bit write
//                       wrdata   = ~wrdata  // checkerboard
//
//          Phase 2:  expected = ZERO
//                    For address = addr to (addr+size-4), step 4
//                       received = *address    // 32-bit read
//                       if (expected <> received) exit with error
//                       expected = ~expected   // checkerboard
//
//          Phase 3:  expected = last data // value written to upper address
//                    For address = (addr+size-4) to addr, step-4
//                       received = *address    // 32-bit read
//                       if (expected <> received) exit with error
//                       expected = ~expected   // checkerboard
//
//           cmd[1] ... address (checked for 32-bit alignment)
//           cmd[2] ... number of bytes (expected to be word aligned)
//           cmd[3] ... zeros data
//           cmd[4] ... iterations
//           cmd[5] ... test flag
//                         bit-0= test address is a configuration address
//                         bit-1= call delay of cmd[6] between operations
//                         bit-2= read address specified in cmd[6] between operations
//           cmd[6] ... flag data0 (delay or read address)
//           cmd[7] ... flag data1 (b0=port, bits 27:12=BDF)
//
//       The result is placed in sts[0], 0(pass), 1(fail phase2), 2(fail phase3)
//       Upon a failure, the address is placed into sts[1], expected data in sts[2],
//       received in sts[3], iteration in sts[4]
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_march_mem_debug (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 start_addr;
  uint32 nbytes;
  uint32 zero_data;
  uint32 flags, flag_data0, flag_data1;
  uint32 iter, iterations;
  uint32 dmy;
  uint32 bdf, cfg_index_addr, cfg_data_addr;
  // uint32 port;
  uint32 i, addr, data, exp, rec, status;

  start_addr = cmd[1];
  nbytes     = cmd[2];

  if ( ((start_addr & 3) != 0) ||
       ((nbytes & 3) != 0) ||
       (nbytes > 0x10000000))
    return 1;


  zero_data  = cmd[3];
  iterations = cmd[4];
  flags      = cmd[5];
  flag_data0  = cmd[6];
  flag_data1  = cmd[7];
  //  port       = flag_data1 & 1;
  bdf        = flag_data1 & 0x0ffff000;

  //  if (port==0) {
    cfg_index_addr = BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_EXT_CFG_PCIE_EXT_CFG_INDEX;
    cfg_data_addr  = BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_EXT_CFG_PCIE_EXT_CFG_DATA_0;
    //  } else {
    //    cfg_index_addr = PCIE1_EXT_CFG_INDEX;
    //    cfg_data_addr  = PCIE1_EXT_CFG_DATA;
    //  }

  status = 0;
  for (iter = 0; iter<iterations; iter++) {
    // phase 1, write the range with ZERO data (checkerboard)
    addr = start_addr;
    data = zero_data;
    for (i=0; i<nbytes; i+=4, addr+=4) {
      if (flags&1) {
        *((volatile uint32*) cfg_index_addr) = bdf | addr;
        *((volatile uint32*) cfg_data_addr)  = data;
      } else {
        *((volatile uint32*) addr) = data;
      }
      data = ~data;

      if (flags&2)
        delay(flag_data0);

      if (flags&4)
        dmy = *((volatile uint32*) flag_data0);
    }

    // phase 2, read/verify ZEROS, write ONES data (checkerboard), bottom up order
    addr   = start_addr;
    exp    = zero_data;
    for (i=0; i<nbytes; i+=4, addr+=4) {
      if (flags & 1) {
        *((volatile uint32*) cfg_index_addr) = bdf | addr;
        rec = *((volatile uint32*) cfg_data_addr);
      } else {
        rec = *((volatile uint32*) addr);
      }
      if (exp != rec) {
        status = 1;     // fail phase 2
        break;
      }

      if (flags&2)
        delay(flag_data0);

      if (flags&4)
        dmy = *((volatile uint32*) flag_data0);


      if (flags&1) {
        *((volatile uint32*) cfg_index_addr) = bdf | addr;
        *((volatile uint32*) cfg_data_addr) = ~exp;
      } else {
        *((volatile uint32*) addr) = ~exp;
      }
      exp = ~exp;

      if (flags&2)
        delay(flag_data0);

      if (flags&4)
        dmy = *((volatile uint32*) flag_data0);

    }

    if (status == 0) {
      // phase 3, read/verify ONES, write ZEROs data (checkerboard), top down order
      addr   = start_addr;
      addr   = addr + nbytes - 4;
      // exp is set correctly to very upper most location
      for (i=0; i<nbytes; i+=4, addr-=4) {
        if (flags & 1) {
          *((volatile uint32*) cfg_index_addr) = bdf | addr;
          rec = *((volatile uint32*) cfg_data_addr);
        } else {
          rec = *((volatile uint32*) addr);
        }

        if (exp != rec) {
          status = 2;                     // fail phase 3
          break;
        }

        if (flags&2)
          delay(flag_data0);

        if (flags&4)
          dmy = *((volatile uint32*) flag_data0);

        if (flags&1) {
          *((volatile uint32*) cfg_index_addr) = bdf | addr;
          *((volatile uint32*) cfg_data_addr)  = ~exp;
        } else {
          *((volatile uint32*) addr) = ~exp;
        }

        exp = ~exp;

        if (flags&2)
          delay(flag_data0);

        if (flags&4)
          dmy = *((volatile uint32*) flag_data0);


      }
    }

    if (status != 0)
      break;
  }

  sts[0] = status;
  if (status) {
    sts[1] = addr;
    sts[2] = exp;
    sts[3] = rec;
    sts[4] = iter;
  }

  return 0;
}



// ---------------------------------------------------------------------
// cmd_access_sweep
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs repeated accesses separated by loop delay.
//       This test can be used to test boundary conditions in accesses
//       in power management modes setup externally.
//
//       Inputs:
//           cmd[1] ... test address (checked for 32-bit alignment)
//           cmd[2] ... test flag (bit-0=reads enabled, bit-1=writes enabled)
//           cmd[3] ... test zeros data (or expected data if only reads are being done)
//           cmd[4] ... iterations
//           cmd[5] ... inner loop minimum
//           cmd[6] ... inner loop maximum
//           cmd[7] ... inner loop step
//
//       Outputs:
//           sts[0] ... pass(0) or fail(1)
//           sts[1] ... failing address
//           sts[2] ... expected data
//           sts[3] ... received data
//           sts[4] ... iteration loop count of failure
//           sts[5] ... inner loop count of failure
//
//       The result is placed in sts[0], 0(pass), 1(fail)
//       Upon a failure, the address is placed into sts[1], expected data in sts[2], received in sts[3],
//       Inner count in sts[4], Outer count in sts[5].
//
//       A check is performed to confirm the address is 32-bit aligned.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

void delay(uint32 dly) {
  uint32 i;

  // Doesn't appear to work ... optimized out?
  // For now, do volatile register read instead
  //  for (i=0; i<dly; i++)
  //;
  uint32 data;
  for (i=0; i<dly; i++)
    data = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_HIF_TOP_CTRL_SCRATCH);
}

uint32 cmd_access_sweep (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr, flag, zero_data, iter, loop_min, loop_max, loop_step;
  uint32 i, j, exp, rec, status;

  addr = cmd[1];
  flag = cmd[2];
  zero_data = cmd[3];
  iter = cmd[4];
  loop_min = cmd[5];
  loop_max = cmd[6];
  loop_step = cmd[7];

  if ( (addr & 3) != 0 )
    return 1;

  status = 0;
  exp = zero_data;
  for (i=0; i<iter; i++) {

    for (j=loop_min; j<=loop_max; j+=loop_step) {

      if (flag & 2) { // Do writes
        *((volatile uint32*) addr) = exp;
        delay(j);
      }

      if (flag & 1) { // Do reads
        rec = *(volatile uint32*)addr;
        if (exp != rec) {
           status = 1;
           break;
        }
        delay(j);
      }

      if (flag & 2) // If we're doing writes, toggle data
        exp = ~exp;

    }
    if (status)
      break;
  }

  sts[0] = status;
  if (status) {
    sts[1] = addr;
    sts[2] = exp;
    sts[3] = rec;
    sts[4] = i;
    sts[5] = j;
  }

  return 0;
}
// ---------------------------------------------------------------------
// cmd_wlan_access_sweep
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs repeated accesses separated by loop delay.
//       This test can be used to test boundary conditions in accesses
//       in power management modes setup externally.
//
//       Inputs:
//           cmd[1] ... test address (checked for 32-bit alignment)
//           cmd[2] ... test flag
//           cmd[3] ... test zeros data (or expected data if only reads are being done)
//           cmd[4] ... iterations
//           cmd[5] ... inner loop minimum
//           cmd[6] ... inner loop maximum
//           cmd[7] ... inner loop step
//
//       Outputs:
//           sts[0] ... pass(0) or fail(1)
//           sts[1] ... failing address
//           sts[2] ... expected data
//           sts[3] ... received data
//           sts[4] ... iteration loop count of failure
//           sts[5] ... inner loop count of failure
//
//       The result is placed in sts[0], 0(pass), 1(fail)
//       Upon a failure, the address is placed into sts[1], expected data in sts[2], received in sts[3],
//       Inner count in sts[4], Outer count in sts[5].
//
//       A check is performed to confirm the address is 32-bit aligned.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

#define WLAN_SPARE0 8

uint32 cmd_wlan_access_sweep (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 addr, flag, zero_data, iter, loop_min, loop_max, loop_step;
  uint32 i, j, exp, rec=0, status;



  addr = cmd[1];   // address of the indirection register
  flag = cmd[2];
  zero_data = cmd[3]; // expected value of read
  iter = cmd[4];
  loop_min = cmd[5];
  loop_max = cmd[6];
  loop_step = cmd[7];

  if ( (addr & 3) != 0 )
    return 1;

  status = 0;
  exp = zero_data;
  for (i=0; i<iter; i++) {

#ifdef DO_WATCHDOG
  // Re-start watchdog per iteration
  *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START0; // 1st of 2 writes needed to restart it
  *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START1; // 2nd of 2 writes
#endif

    for (j=loop_min; j<=loop_max; j+=loop_step) {

      *((volatile uint32*) addr) = WLAN_SPARE0;    // write indirection address register
      if ((flag & 8)==0) // skip delay after write
        delay(j);


    }
    if (status)
      break;
  }

  sts[0] = status;
  if (status) {
    sts[1] = addr;
    sts[2] = exp;
    sts[3] = rec;
    sts[4] = i;
    sts[5] = j;
  }

  return 0;
}

// ---------------------------------------------------------------------
// cmd_pcie_mem_perf
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs repeated accesses separated by loop delay.
//       This test can be used to test boundary conditions in accesses
//       in power management modes setup externally.
//
//       Inputs:
//           cmd[1] ... test address (checked for 32-bit alignment)
//           cmd[2] ... length of test reigon (in bytes )
//           cmd[3] ... test data (or expedcted data if only reads are being done)
//           cmd[4] ... test flag (bit-1 to enable phase 0, etc. )
//           cmd[5] ... iterations
//       Outputs:
//           sts[0] ... pass(0) or fail(1)
//
//           If the test failed, the following status is written
//           sts[1] ... failing address
//           sts[2] ... expected data
//           sts[3] ... received data
//           sts[4] ... iteration loop count of failure
//
//           If the test passed, the following status is written
//           sts[1] ... max time of phase 0 test
//           sts[2] ... max time of phase 1 test
//           sts[3] ... max time of phase 2 test
//           sts[4] ... max time of phase 3 test
//
//
//       A check is performed to confirm the address is 32-bit aligned.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

uint32 cmd_pcie_mem_perf (volatile uint32 *cmd, volatile uint32 *sts)
{
  uint32 start_addr, length, test_data, flag, iter;
  uint32 i, exp, rec, addr, write_data, status;
  uint32 time, total_time;
  uint32 phase_time[4] ={0,0,0,0}       ;

  start_addr = cmd[1];
  length = cmd[2];
  test_data = cmd[3];
  flag = cmd[4];
  iter = cmd[5];

  if ( (start_addr & 3) != 0 )  // bits 1:0 should be 0 for it to be a 32-bit aligned test
    return 1;

  exp=0;
  status = 0;
  total_time = 0;

  for (i=0; i<iter; i++) {


        // If the test takes too long, WATCHDOG will timeout
        #ifdef DO_WATCHDOG
          // Re-start watchdog per iteration
          *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START0; // 1st of 2 writes needed to restart it
          *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START1; // 2nd of 2 writes
        #endif


        //initialize timer2
        *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) = TIMER_RESET;

        // phase 0: start timer, write, stop timer
        if(( flag & 1 ) == 1 ){
                write_data = test_data;

                //start timer
                *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) |= TIMER_START;

                for( addr = start_addr; addr <= (start_addr + length - 4); addr+=4 ){
                        // write address as 32 bitword with write data
                        *((volatile uint32*) addr) = write_data;
                        // invert write data
                        write_data = ~write_data;
                }
                // check timer value
                time = *((volatile uint32*)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_STAT));
                // stop timer
                *(volatile uint32*)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) &= TIMER_STOP;

                total_time += time;
                if( time > phase_time[0] ){
                        phase_time[0] = time;
                }
        }


        // phase 1: start timer, read, stop timer
        if( (flag & 2 ) == 2 ){
                *(volatile uint32* )(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) = TIMER_RESET;
                exp = test_data;
                *((volatile uint32* )(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL)) |= TIMER_START;

                for( addr = start_addr; addr <= (start_addr + length - 4); addr+=4 ){
                        rec = *((volatile uint32*) addr);
                        if( rec != exp ){
                                status = 1;
                                goto done;
                        }

                        exp = ~exp;
                }

                time =  *(volatile uint32*)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_STAT);
                *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) &= TIMER_STOP;

                total_time += time;
                if( time > phase_time[1] ){
                        phase_time[1] = time;
                }
        }


        // phase 2: start timer, write, read, stop timer
        if( (flag & 4 ) == 4 ){
                *(volatile uint32* )(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) = TIMER_RESET;
                write_data = test_data;
                *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) |= TIMER_START;

               for( addr = start_addr; addr <= (start_addr + length - 4); addr+=4 ){

                        *(( volatile uint32* ) addr ) = write_data;

                        rec = *((volatile uint32*) addr);
                        if( rec != write_data ){
                                status = 1;
                                goto done;
                        }

                        write_data = ~write_data;
                }

                time =  *(volatile uint32*)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_STAT);
                *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) &= TIMER_STOP;

                total_time += time;
                if( time > phase_time[2] ){
                        phase_time[2] = time;
                }
        }
  }

  phase_time[3] = total_time;

  done:
  sts[0] = status;
  if (status) {
        sts[1] = addr;
        sts[2] = exp;
        sts[3] = rec;
        sts[4] = i;     // the failing iteration
  } else if( status == 0 ){
        sts[1] = phase_time[0];
        sts[2] = phase_time[1];
        sts[3] = phase_time[2];
        sts[4] = phase_time[3];
  }

  return 0;
}


// ---------------------------------------------------------------------
// cmd_mips_dma_wr
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Performs DMA write
//
//       Inputs:
//           cmd[1] ... target address (checked for 32-bit alignment)
//           cmd[2] ... number of bytes to transfer (32-bit aligned)
//           cmd[3] ... test flag Bits 3:0 pattern (0=data,1=alt data, 2=inc bytes, 3=inc hword, 4=rand)
//                                bits 15:8 burst len in bytes
//           cmd[4] ... data (test data or random seed)
//           cmd[5] ... unused
//           cmd[6] ... unused
//           cmd[7] ... unused
//
//       Outputs:
//           sts[0] ... Last DMA Status
//           sts[1] ...
//           sts[2] ...
//           sts[3] ...
//           sts[4] ...
//           sts[5] ...
//
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------

#define SWAP_SHIFT  16
#define SWAP_MASK   0x3

#define BLEN_SHIFT  8
#define BLEN_MASK   0xff

#define PAT_SHIFT   0
#define PAT_MASK    0xf
#define PAT_DATA    0
#define PAT_ALTDATA 1
#define PAT_INC8    2
#define PAT_INC16   3
#define PAT_RAND    4

uint32 cmd_mips_dma_wr (volatile uint32 *cmd, volatile uint32 *sts)
{
  ((void)cmd); /* unused for now */
  ((void)sts); /* unused for now */
  return 1;
}

// ---------------------------------------------------------------------
// cmd_mips_dma_rd
//
//   Parameters:
//       *cmd ... pointer to input parameters
//       *sts ... pointer to output return values
//
//   Description:
//       Verifies a range of memory using DMA burst bank 1
//
//       Inputs:
//           cmd[1] ... target address (checked for 32-bit alignment)
//           cmd[2] ... number of bytes to transfer (32-bit aligned)
//           cmd[3] ... test flag Bits 3:0 pattern (0=data,1=alt data, 2=inc bytes, 3=inc hword, 4=rand)
//                                bits 15:8 burst len in bytes
//                                bits 17:16 swap (0=no swap, 1=16-bit swap, 2=byte swap, 3-=no swap)
//           cmd[4] ... data (test data or random seed)
//           cmd[5] ... unused
//           cmd[6] ... unused
//           cmd[7] ... unused
//
//       Outputs:
//           sts[0] ... 0(pass), 1(fail), 9 (timeout), 0xc(DMA error)
//           sts[1] ... address
//           sts[2] ... expected
//           sts[3] ... received
//           sts[4] ...
//           sts[5] ...
//
//       A check is performed to confirm the address is 32-bit aligned,
//       the number of bytes is 32-bit aligned, and the range size is
//       reasonable, less than or equal to 256MB.
//       The function returns with a failure if any of these checks fail.
//
//   Returns:
//       0 (command successul), 1 (command not successful)
//
// ---------------------------------------------------------------------


uint32 cmd_mips_dma_rd (volatile uint32 *cmd, volatile uint32 *sts)
{
  ((void)cmd); /* unused for now */
  ((void)sts); /* unused for now */
  return 1;
}

//      -----------------------------------------------------------------------------
/*      cmd_mips_dma_perf

        Parameters:
                *cmd ... pointer to input parameters
                *sts ... pointer to output return values

        Description:
                Times operations to range of memory using DMA burst bank 1

        Inputs:
                cmd[1] ... target address (checked for 32-bit alignment)
                cmd[2] ... length of test region (in bytes )
                cmd[3] ... test data (or expected data if doing a read-only test )
                cmd[4] ... test flag ( bit 0 enables phase 0, etc. )
                cmd[5] ... iterations

        Outputs:
                sts[0] ... 0(pass), 1(fail), 9 (timeout), 0xc(DMA error)

                If the test failed:
                sts[1] ... failed address
                sts[2] ... expected data
                sts[3] ... received data
                sts[4] ... failed iteration

                If the test passed:
                sts[1] ... max time of phase 0
                sts[2] ... max time of phase 1
                sts[3] ... max time of phase 2
                sts[4] ... max time of phase 3

        A check is performed to confirm the address is 32-bit aligned,
        the number of bytes is 32-bit aligned, and the range size is
        reasonable, less than or equal to 256MB.
        The function returns with a failure if any of these checks fail.

        Returns:
                0 (command successul), 1 (command not successful)
*/
//      ---------------------------------------------------------------------

#define NUM_BURST_BANKS 8
#define BSIZE 15                // Burst size of 16 words

uint32 cmd_mips_dma_perf( volatile uint32 *cmd, volatile uint32 *sts){
  ((void)cmd); /* unused for now */
  ((void)sts); /* unused for now */
  return 1;
} // close function

//      -----------------------------------------------------------------------------
/*      cmd_pcie_dma_perf

        Parameters:
                *cmd ... pointer to input parameters
                *sts ... pointer to output return values

        Description:
                Times operations to range of memory using pcie DMA engine

        Inputs:
                cmd[1] ... local starting address
                cmd[2] ... pcie starting address
                cmd[3] ... number of bytes to transfer
                cmd[4] ... descriptor starting address
                cmd[5] ... length of each transfer (64B, 512B, 1500B, or random)
                cmd[6] ... number of descripters to burst simultaneously (== 1 for now)
		cmd[7] ... endian mode
                cmd[8] ... iterations
		cmd[9] ... direction ( 1 indicates PCIE --> LOCAL transfer )

        Outputs:
                sts[0] ... 0(pass), 1(fail), 9 (timeout), 0xc(DMA error)

                If the test failed:
                sts[1] ... failed pcie address
		sts[2] ... failed local address
		sts[3] ... failed length of transfer
                sts[4] ... failed iteration

                If the test passed:
                sts[1] ... max time of phase 0
                sts[2] ... total time of test

        A check is performed to confirm the address is 32-bit aligned,
        the number of bytes is 32-bit aligned, and the range size is
        reasonable, less than or equal to 256MB.
        The function returns with a failure if any of these checks fail.

        Returns:
                0 (command successul), 1 (command not successful)
*/
//      ---------------------------------------------------------------------

#define NUM_DESC 64
#define MAX_TRAN_SIZE 1500
#define MIN_TRAN_SIZE 64

uint32 cmd_pcie_dma_perf( volatile uint32 *cmd, volatile uint32 *sts){
uint32 vdma_desc_base;

 volatile DmaDescRegs *pDmaDescRegs;

// parameters
uint32 local_start, pcie_start, nbytes, vdesc_start, trans_len, endian, dir;
uint32 word3, word4;
uint32 local_addr, pcie_addr, bytes_left, cur_desc, transfer_size, cur_dma;
uint32 status, i, time, dma_status, desc_err, pdesc_start;
uint32 temp, last_word4;
uint32 phase_time[3] = {0,0,0};
//char out_buff[256];

uint32 num_checks = 0;

#ifdef DEBUG_PCIE_DMA
uint32 temp_array[14096];
uint32 cur_dma_array[14096];
uint32 x = 0;
#endif

#ifdef DEBUG_PCIE_TO_MEM
uint32 x = 0;
#endif

 vdma_desc_base = cmd[4];
 pDmaDescRegs = ( volatile DmaDescRegs *) vdma_desc_base;

status = 0;

local_start 	= cmd[1];
pcie_start 	= cmd[2];
nbytes 		= cmd[3];
vdesc_start	= cmd[4];
trans_len 	= cmd[5];
//burst_size	= cmd[6];
endian		= cmd[7];
//iter		= cmd[8];
dir		= cmd[9];

// check for 32bit allignment
if( (( pcie_start & 3) != 0 ) ){
	printf("err1\n");
	uart_prompt( 0 );
	return 1;
}
else if( (( local_start & 3) != 0 )){
	printf("err2");
	uart_prompt( 0 );
	return 2;
}

//get physical desc addr from the virtual one
pdesc_start = virt2phys( vdesc_start );
//pdesc_start =  vdesc_start ;
if( pdesc_start == 0xffffffff){
	printf("err3");
	return 3;
}
//UartWriteStr( 0, "start", 1 );
//UartWriteStr( 0, "start2", 1 );
//uart_prompt( 0 );

//sprintf( out_buff, "pdesc_start is &h%x", pdesc_start );
//UartWriteStr( 0, out_buff, 1 );
//sprintf( out_buff, "vdesc_start is &h%x", vdesc_start );
//UartWriteStr( 0, out_buff, 1 );

// Set location status for Descriptors (pcie or local)
if( (vdesc_start >=  0xC0000000) && (vdesc_start<= 0xDFFFFFFF )){				// if the bit is set, it's a pcie addr
        if( dir == 1 ){
                pcie_dma_tx_sw_desc_list_ctrl_sts &= ~(1<<kLocalDesc);
        }else{
                pcie_dma_rx_sw_desc_list_ctrl_sts &= ~(1<<kLocalDesc);
        }

        printf("Remote desc");
} else{
	if( dir == 1 ){
		pcie_dma_tx_sw_desc_list_ctrl_sts |= (1<<kLocalDesc);
        }else{
		pcie_dma_rx_sw_desc_list_ctrl_sts |= (1<<kLocalDesc);
	}
	printf("Local Desc");
	uart_prompt( 0 );
}

if( dir == 1 ){
	pcie_dma_tx_first_desc_l_addr_list0 = pdesc_start;
}else{
	pcie_dma_rx_first_desc_l_addr_list0 = pdesc_start;
}
//sprintf( out_buff, "pcie_start = &h%x", pcie_start);
//UartWriteStr( 0, out_buff, 1 );

//sprintf( out_buff, "local_start = &h%x", local_start);
//UartWriteStr( 0, out_buff, 1 );

//sprintf(out_buff, "vdesc_start = &h%x", vdesc_start);
//UartWriteStr( 0, out_buff, 1 );

//sprintf( out_buff, "pdesc_start = &h%x", pdesc_start);
//UartWriteStr( 0, out_buff, 1 );

word3 = 0;
word4 = 0;
last_word4 = 0;

if( endian != 0 ){
	word4 |= (endian<<kEndian);
}
if( dir == 1){
	word4 |= (1<<kDir);
	printf("Transferring data PCIE --> LOCAL");
}else{
	printf("Transferring data Local --> PCIE");
}
last_word4 = word4;
word4 |= 1<<kCont;

// if the trans_len is static, set it
// BUG ... random transfer size is not properly handled
 transfer_size = 0;
if( trans_len != 0xFF ){
	if( trans_len & 0x3 ){
		printf("Transfer Length must be byte aligned");
	}
	transfer_size = trans_len & ~(0x3);
	word3 = transfer_size;

//	sprintf( out_buff, "Using transfer length of: &h%x", transfer_size );
//	UartWriteStr( 0, out_buff, 1 );
}

for( i = 0; i < NUM_DESC; i++ ){
        pDmaDescRegs[i].fPcieAddrHi = 0;
        pDmaDescRegs[i].fNextDescAddrHi = 0;
	pDmaDescRegs[i].fReserved = 0;

	// point the last descriptor to the first
	if( i == (NUM_DESC-1) ){
		pDmaDescRegs[i].fNextDescAddrLo = pdesc_start;	// must use physical addr
		pDmaDescRegs[i].fWord4 = last_word4;
	}else{
		pDmaDescRegs[i].fWord4 = word4;
	}

	// if the trans_len is static, set it
	if( trans_len != 0xFF ){
		pDmaDescRegs[i].fWord3 = word3;
	}
}
//UartWriteStr( 0, "Done Desc init", 1 );

// **** Main Routine ***

local_addr = local_start;
pcie_addr = virt2phys( pcie_start);
//pcie_addr =  pcie_start;
cur_desc = 0;

//trans_len is the parameter passed; transfer_size is what is set in the descriptor
if( trans_len == 0xFF ){
	srand32( 37 );
//	UartWriteStr( 0, "rnd transfer size", 1 );
}

for( i = 0; i < 1; i++ ){

	// If the test takes too long, WATCHDOG will timeout
	#ifdef DO_WATCHDOG
		// re-start watchdog per iteration
		*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START0;
		*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START1;
	#endif

//	UartWriteStr( 0, "startTimer", 1 );

	// initialize and start timer2
	*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) = TIMER_RESET;
	*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) |= TIMER_START;

	for( bytes_left = nbytes; bytes_left > 0; bytes_left -= transfer_size){

		// get a random number if the transfer length is not fixed
		if( trans_len == 0xFF ){				// 0xFF is the key for a rand value
			transfer_size = ( rand32()%(MAX_TRAN_SIZE-MIN_TRAN_SIZE) + MIN_TRAN_SIZE ) & ~(0x3);
		        //sprintf( out_buff, "transfer_size = &h%x", transfer_size );
			//UartWriteStr( 0, out_buff, 1 );
		}

		// make sure we have enough bytes left to transfer
		if( transfer_size > bytes_left ){
			transfer_size = bytes_left;
		}

		num_checks = 0;

		if( bytes_left != nbytes ){
		// make sure the descripter we're about to write is not in use
		do{
			if( ++num_checks > 1000 ){
				status = 3;
				goto done;
			}
			if( dir == 1 ){
				cur_dma = pcie_dma_tx_cur_desc_l_addr_list0;	// address of descriptor being processed
				dma_status = (pcie_dma_tx_sw_desc_list_ctrl_sts>>kDmaStatus) & 0x3;
			}else{
				cur_dma = pcie_dma_rx_cur_desc_l_addr_list0;
	                               dma_status = (pcie_dma_rx_sw_desc_list_ctrl_sts>>kDmaStatus) & 0x3;
			}

			temp = (cur_desc*32 + pdesc_start);

			#ifdef DEBUG_PCIE_TO_MEM
                                *(volatile uint32 * ) DEBUG_BUFFER_INDEX = x;
                                *(volatile uint32 * ) (DEBUG_BUFFER + (8*x)) = temp;
                                *(volatile uint32 * ) (DEBUG_BUFFER + (8*x)+4) = cur_dma;
				x++;
			#endif

			#ifdef DEBUG_PCIE_DMA
				temp_array[x] = temp;
				cur_dma_array[x] = (cur_dma ); //& 0xfffffff0);
				x++;
			#endif
			if( dma_status == kDmaError ){
				status = 4;
				desc_err = (cur_dma - pdesc_start)>>5;  // number of descriptor with the error

//				sprintf( out_buff, "cur_dma = %x", cur_dma );
//               	                UartWriteStr( 0, out_buff, 1 );
//                                sprintf( out_buff, "dma_status = %x", dma_status );
//                                UartWriteStr( 0, out_buff, 1 );

				local_addr = pDmaDescRegs[ desc_err ].fMemAddr;
				pcie_addr = pDmaDescRegs[ desc_err ].fPcieAddrLo;
//				UartWriteStr( 0, "kDmaError", 1 );
				goto done;
			}

			// poke the dma engine: Wake = bit0, WakeMode = bit1
	                if( dir == 1 ){
				pcie_dma_tx_wake_ctrl = kDmaWake;
			}else{
				pcie_dma_rx_wake_ctrl = kDmaWake;
			}

		}while( ((cur_dma & 0xfffffff0) == temp) );//&& ((cur_dma & 0x4) == 4) ); // while (dma_desc == cpu_desc) AND (dma desc is in progress)
		}
		pDmaDescRegs[ cur_desc ].fMemAddr = local_addr;
		pDmaDescRegs[ cur_desc ].fPcieAddrLo = pcie_addr;

		if( (trans_len == 0xFF) || (transfer_size == bytes_left) ){
			pDmaDescRegs[ cur_desc ].fWord3 = transfer_size;
		}

		// set the 'last' bit
		if( cur_desc == (NUM_DESC - 1) ){
			last_word4 |= 1<<kLast;
			pDmaDescRegs[ cur_desc ].fWord4 = last_word4;
		}else{
			word4 |= 1<<kLast;
			pDmaDescRegs[ cur_desc ].fWord4 = word4;
		}

		// remove the 'last' bit from the previous descriptor
		if( cur_desc == 0 ){
			last_word4 &= ~(1<<kLast);
			pDmaDescRegs[ NUM_DESC - 1 ].fWord4 = last_word4;
		} else{
			word4 &= ~(1<<kLast);
			pDmaDescRegs[ cur_desc-1 ].fWord4 = word4;
		}

		if( bytes_left == nbytes ){	// on the first iteration,
			if( dir == 1 ){
				temp = pcie_dma_tx_sw_desc_list_ctrl_sts;
				temp |= 1<<kRunStop;    // set run bit
				temp |= 1<<kDmaMode;    // mode = wake/reset
				pcie_dma_tx_sw_desc_list_ctrl_sts = temp;
			}else{
				temp = pcie_dma_rx_sw_desc_list_ctrl_sts;
				temp |= 1<<kRunStop;    // set run bit
                                temp |= 1<<kDmaMode;    // mode = wake/reset
                                pcie_dma_rx_sw_desc_list_ctrl_sts = temp;
			}
		}

		// poke the dma engine: Wake = 1, WakeMode = 0
		if( dir == 1 ){
			pcie_dma_tx_wake_ctrl = kDmaWake;
		}else{
			pcie_dma_rx_wake_ctrl = kDmaWake;
		}
		local_addr += transfer_size;
		pcie_addr += transfer_size;
		cur_desc = (cur_desc+1) % NUM_DESC;



	}// end of the whole transfer

	// wait for dma status to be error or idle
	do{
		if( dir == 1 ){
			dma_status = (pcie_dma_tx_sw_desc_list_ctrl_sts>>kDmaStatus) & 0x3;
		}else{
			dma_status = (pcie_dma_rx_sw_desc_list_ctrl_sts>>kDmaStatus) & 0x3;
		}

		if( dma_status  == kDmaError ){
			status = 5;
//			UartWriteStr( 0, "dma err", 1 );
			goto done;
		}
	}while( dma_status & 0x1);	// while( dma_status == busy)

	// read timer value
        time = *((volatile uint32*)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_STAT));
//	sprintf( out_buff, "time: &h%x", time );
//	UartWriteStr( 0, out_buff, 1 );

	// stop timer
        *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_TIMER2_CTRL) &= TIMER_STOP;
	if( phase_time[0] < time ){
		phase_time[0] = time;
	}
	phase_time[1] += time;

	if( dir == 1 ){
		pcie_dma_tx_sw_desc_list_ctrl_sts &= ~(1<<kRunStop);
	}else{
		pcie_dma_rx_sw_desc_list_ctrl_sts &= ~(1<<kRunStop);
	}

} // end of all iterations

done:
sts[0] = status;

if( status ){
        sts[1] = pcie_addr;
        sts[2] = local_addr;
        sts[3] = transfer_size;
        sts[4] = ++i;             //failing iteration
} else if( status == 0 ){
        sts[1] = phase_time[0];
        sts[2] = phase_time[1];
	sts[3] = phase_time[2];
}
//#ifdef DEBUG_PCIE_DMA
//	for( i = 0; i< x; i++ ){
//		sprintf( out_buff, "cpu_desc, dma_desc = &h%x, &h%x", temp_array[i], cur_dma_array[i] );
//		UartWriteStr( 0, out_buff, 1 );
//	}
//#endif

//UartWriteStr( 0, "end", 1 );
//uart_prompt( 0 );

return 0;

}

//-----------------------------------------------------------------------------
// The Command Loop handshakes commands/responses with another entity, optimized
// to minimize such handshakes with the assumption that the other entity is
// a relative slow, BBS script (or equivalent).
//
// 128B of memory are reserved as a mailbox.
//
//    0    ... command/status (bidirectional)
//    4-3F ... up to 15 optional input parameters (not read if not needed)
//   40-7F ... up to 16 optional output results (not written if not applicable)
//
// Upon entering the loop, the MIPS server writes status DONE to a command/status word
// Then it spins reading the location looking for a valid command. The pre-write is
// to make sure there is no garbage value that looks like a command.
// (No consideration is made as to the impact of a relatively tight polling loop -
// potentially it is cached space and only extends externally due to a cache snoop invalidate.)
//
// Upon seeing a valid command, the appropriate function is called. No assumptions
// are made on the number of parameters/results at this level. The command function
// is responsible for retrieving and checking all input parameters, and storing
// any results in the reserved area. The command function only indicates
// successful vs. unsuccessful execution of the command to the cmd_loop.
// The cmd_loop then confirms completion by writing value STS_GOOD or STS_BAD
// to the command/status word. The other entity will be polling for this.
//
// Traditionally the MIPS server has performed read/writes (all sizes), but this
// server handshake should be flexible enough to provide complex services.
//
// MIPS vs. system bus addressing must be considered when passing parameters.
// The init loop currently creates a mapping specifically useful to the PCIe block.
// In the future, a more flexible mapping method could be implemented.
//
// Default mapping:
//    (mips) A000_0000 => (sys) 0000_0000 ' DDR SPACE
//    (mips) B000_0000 => (sys) 1000_0000 ' GISB SPACE
//
// Additional mapping specifically for PCIe:
//    (mips) C000_0000 => (sys) A000_0000 ' PCIE WIN0      (128MB)
//    (mips) C800_0000 => (sys) A800_0000 ' PCIE WIN1      (128MB)
//    (mips) D000_0000 => (sys) B000_0000 ' PCIE WIN2      (128MB)
//    (mips) D800_0000 => (sys) B800_0000 ' PCIE WIN3      (128MB)
//    (mips) F100_0000 => (sys) F100_0000 ' PCIE I/O space
//
// There is a 'quit' command, but it does not appear to do as expected
// (return to the CFE prompt on the UART console used to launch the MIPS server).
//
//-----------------------------------------------------------------------------
void cmd_loop ()
{

  uint32 cmd, cmd_good, cmd_quit, status;
  uint32 ch;
#ifdef DO_UART0
  char uart0_buf[255];
  uint32 uart0_ix;
#endif
#ifdef DO_UART1
  char uart1_buf[255];
  uint32 uart1_ix;
#endif

  *(volatile uint32 * ) (0x200000) = 0x12345678;
  *(MBOX_CMD_0) = STS_DONE;


  #ifdef DO_UART0
	uart0_ix = 0;
	uart_prompt( 0 );
  #endif

  #ifdef DO_UART1
	uart1_ix=0;
	uart_prompt(1);
  #endif

#ifdef DO_WATCHDOG
  *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDTIMEOUT) = WATCHDOG_TIMEOUT;
#endif

  while (1) {

    while (1) {

#ifdef DO_WATCHDOG
      *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START0; // 1st of 2 writes needed to start/restart it
      *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_TIMER_WDCMD) = WATCHDOG_START1; // 2nd of 2 writes
#endif

	cmd = *(MBOX_CMD_0);
	if ((cmd & CMD_MASK) == CMD_VALID){
		break;
	}

	#ifdef DO_UART0
		ch = det_in_char();
		if( ch != 0 ){
			ch &= 0xff;
			uart0_buf[uart0_ix++] = ch;
		}
		if (ch==0xd){                       // Upon seeing CR, execute command
			uart0_buf[uart0_ix-1] = 0;
			execute_uart_cmd(0, &uart0_buf[0]);
			uart_prompt(0);
			uart0_ix=0;
		}
	#endif

	#ifdef DO_UART1
	ch = UartReadChar(1,1);           // Scan UART1 console, echo
	if (ch != 0)
	{
		ch = ch & 0xff;
		uart1_buf[uart1_ix++] = ch;
	}
	if (ch==0xd)                       // Upon seeing CR, execute command
	{
		uart1_buf[uart1_ix-1] = 0;
		execute_uart_cmd(1, &uart1_buf[0]);
		uart_prompt(1);
		uart1_ix=0;
	}
	#endif
  }


    cmd    = cmd & ~CMD_MASK;
    cmd_good = 1;
    cmd_quit = 0;
    status   = 0;

    switch (cmd) {
	case CMD_HELLO     : status = cmd_hello     (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_QUIT      : cmd_quit = 1                                    ; break;
	case CMD_READ32    : status = cmd_read32    (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_READ16    : status = cmd_read16    (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_READ8     : status = cmd_read8     (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_WRITE32   : status = cmd_write32   (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_WRITE16   : status = cmd_write16   (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_WRITE8    : status = cmd_write8    (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_CLR_MEM   : status = cmd_clr_mem   (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_FILL_MEM  : status = cmd_fill_mem  (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_CHECK_MEM : status = cmd_check_mem (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_MARCH_MEM : status = cmd_march_mem (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_ACCESS_SWEEP : status = cmd_access_sweep (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_MIPS_DMA_WR : status = cmd_mips_dma_wr (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_MIPS_DMA_RD : status = cmd_mips_dma_rd (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_MARCH_MEM_DEBUG : status = cmd_march_mem_debug (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_WLAN_ACCESS_SWEEP : status = cmd_wlan_access_sweep (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_PCIE_MEM_PERF : status = cmd_pcie_mem_perf (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_MIPS_DMA_PERF : status = cmd_mips_dma_perf (MBOX_CMD_0, MBOX_STS_0) ; break;
	case CMD_PCIE_DMA_PERF : status = cmd_pcie_dma_perf (MBOX_CMD_0, MBOX_STS_0) ; break;

	default            : cmd_good = 0                                    ; break;
    }
    if (!cmd_good || status)
      *(MBOX_CMD_0) = cmd | STS_BAD;
    else if (cmd_quit)
      *(MBOX_CMD_0) = cmd | STS_QUIT;    // quit command does not appear to work ... restarts code?
    else
      *(MBOX_CMD_0) = cmd | STS_DONE;

    if (cmd_quit)
      break;
  }

}

/*
uint32 virt2phys( uint32 addr )
'
'
'   Description:
'     Translate virtualS address to System address
'
'   Return: system address, or -1 if no mapping found
*/

uint32 virt2phys( uint32 addr ){

  uint32 i;
  uint32 win_base[4];
  uint32 win_lo[4];

  win_base[0] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT);
  win_base[0] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN1_BASE_LIMIT);
  win_base[0] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN2_BASE_LIMIT);
  win_base[0] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN3_BASE_LIMIT);

  win_lo[0] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN0_LO);
  win_lo[1] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN1_LO);
  win_lo[2] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN2_LO);
  win_lo[3] = *(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_PCIE_0_MISC_CPU_2_PCIE_MEM_WIN3_LO);

  for( i = 0; i<4; i++ ) {
    if(    ((addr & 0xfff00000)>= ((win_base[i] & 0xfff0)<<16))
           && ((addr & 0xfff00000)<= (win_base[i] & 0xfff00000))) {
      addr -= ((win_base[i] & 0xfff0 )<<16);
      addr += (win_lo[i] & 0xfff00000);
      break;
    }
  }

  // Could do error detection on system address holes here, but
  // likely if it does not hit the external PCIe memory windows, we
  // should just return the address as-is
  //
  //  if( i == 4 )
  //    return -1;

  return addr;
}



void uart_prompt(uint32 chan)
{
  ((void)chan); /* unused for now */
  char msg[4];
  msg[0]=0xd;
  msg[1]=0xa;
  msg[2]='>';
  msg[3]=0;
  printf("%s\n", msg);

}

uint32 parse_num(char *s, uint32 *ix, uint32 *num)
{
  uint32 ch, n, i, status;

  status = 1;
  n = 0;
  for (i=*ix; (s[i]!=0) && (s[i]!=' ') && (s[i]!=0xd) && (s[i]!=0x9) && (s[i]!=0xa); i++)
    {
    ch = s[i] & 0xff;
    if ((ch>='0')&&(ch<='9'))
      {
        n = (n<<4) + (ch-'0');
        status = 0;
      }
    else if ((ch>='a')&&(ch<='f'))
      {
        n = (n<<4) + (ch-'a'+10);
        status = 0;
      }
    else if ((ch>='A')&&(ch<='F'))
      {
        n = (n<<4) + (ch-'A'+10);
        status = 0;
      }
    else
      {
        status = 1;
        break;
      }
  }
  *num = n;
  *ix = i;
  return status;
}

void parse_space(char *s, uint32 *ix)
{
  uint32 i;

  for (i=*ix; (s[i]==' ') || (s[i]==0xd) || (s[i]==0x9) || (s[i]==0xa); i++)
    ;
  *ix = i;
}

void put_error(uint32 chan)
{
   char msg[20];

  ((void)chan); /* unused for now */
   msg[0]=0xd;  msg[1]=0xa;  msg[2]='I';   msg[3]='n';
   msg[4]='v';  msg[5]='a';  msg[6]='l';   msg[7]='i';
   msg[8]='d';  msg[9]=' ';  msg[10]='c';  msg[11]='m';
   msg[12]='d'; msg[13]=0;
   printf("%s\n", msg);
   // (void)UartWriteStr(chan, "Invalid Command", 0);
}

void put_num(uint32 chan, uint32 num, uint32 sz)
{
  uint32 i, dig;
  char msg[20];

  ((void)chan); /* unused for now */
  for (i=0; i<(sz*2); i++)
    {
      dig = (num>>((sz*2-1-i)*4)) & 0xf;
      if (dig<=9)
        msg[i]='0' + dig;
      else
        msg[i]='a' + dig - 10;
    }
  msg[i]=0;
  printf("%s\n", msg);
}

void execute_uart_cmd(uint32 chan, char *s) {

  uint32 cmd, addr, wdata, rdata, sz, i, status;
  char reset_str[] = "reset";

  // for (i=0; s[i]; i++)
  //  *(volatile uint32 *)(SDRAM_MEM_BASE+0x80+i*4) = s[i];
  // *(volatile uint32 *)(SDRAM_MEM_BASE+0x80+i*4) = s[i];


  // Supported commands:
  //
  // r hexadr [optional size=1/2/4]                ... read address, default 32-bit
  // <space>r<space><addr>[opt <space><size>]
  //
  // w hexadr hexdata [optional size=1/2/4]
  // <space>w<space><addr><space><wdata>[opt <space><size>] ... write address with data, default 32-bit
  //
  // reset	... software master reset

  sz = 4;
  i=0;
  parse_space(&s[0], &i);

  if (s[i]==0)
    return;

  cmd = s[i++] & 0xff;

  if ((cmd != 'r') && (cmd != 'w'))
    {
      put_error(chan);
      return;
    }

  // reset cmd
  for( i=0; i<5; i++ ){
	if( s[i] != reset_str[i] )
		break;
  }

  if( i == 5 ){
	sw_master_reset();
  }

  parse_space(&s[0], &i);
  if (s[i]==0)
    {
	put_error(chan);
	return;
    }

  status = parse_num(&s[0], &i, &addr);
  if (status)
    {
      put_error(chan);
      return;
    }

  if (cmd=='w')
    {
      parse_space(&s[0], &i);
      if (s[i]==0)
        {
          put_error(chan);
          return;
        }


      status = parse_num(&s[0], &i, &wdata);
      if (status)
        {
          put_error(chan);
          return;
        }
    }

  parse_space(&s[0], &i);
  if (s[i])
    {
      status = parse_num(&s[0], &i, &sz);
      if (status)
        {
          put_error(chan);
          return;
        }
    }


  if (cmd=='w')
    {
      switch (sz)
        {
        case 4 : *(volatile uint32 *)addr = wdata;
          break;
        case 2 : *(volatile uint16 *)addr = wdata & 0xffff;
          break;
        case 1 : *(volatile uint8 *)addr = wdata & 0xff;
          break;
        default :
          put_error(chan);
          break;
        }
      return;
    }

  if (cmd=='r')
    {
      switch (sz)
        {
        case 4 : rdata = (uint32) (*(volatile uint32 *)addr);
          break;
        case 2 : rdata = (uint32) (*(volatile uint16 *)addr);
          break;
        case 1 : rdata = (uint32) (*(volatile uint8 *)addr);
          break;
        default :
          put_error(chan);
          return;
          break;
        }
      printf("\n");  // new line
      put_num(chan, rdata, sz);
    }
}

void sw_master_reset ( void ){
	*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_SUN_TOP_CTRL_RESET_SOURCE_ENABLE) |= 1;
	*(volatile uint32 *)(BCHP_PHYSICAL_OFFSET | BCHP_REGISTER_START | BCHP_SUN_TOP_CTRL_SW_MASTER_RESET) = 1;
}

#if 0 //def ARM_TLB

void map_pcie_extended_space (void ) {

  disable_tlb();

  // Each TLB entry is 16MB (0x0100_0000)
  // Note that 0xd0000000 - 0xdfffffff is PCIE1 memory space ... not a good long-term static assignment
  remap_addresses(0xdc000000, 0x06, 0x00000000, TLB_STRONGLY_ORDERED, TLB_MODE_40BIT );   // 40bit address in RBus space
  remap_addresses(0xdd000000, 0x07, 0x40000000, TLB_STRONGLY_ORDERED, TLB_MODE_40BIT );   // 40bit address in RBus space
  remap_addresses(0xde000000, 0x08, 0xc0000000, TLB_STRONGLY_ORDERED, TLB_MODE_40BIT );   // 40bit address in RBus space
  remap_addresses(0xdf000000, 0x08, 0xff000000, TLB_STRONGLY_ORDERED, TLB_MODE_40BIT );   // 40bit address in RBus space

  enable_tlb();

}
#endif
