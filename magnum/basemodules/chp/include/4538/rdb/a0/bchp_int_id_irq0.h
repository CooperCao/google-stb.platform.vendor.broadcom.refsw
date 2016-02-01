/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCHP_INT_ID_IRQ0_H__
#define BCHP_INT_ID_IRQ0_H__


#if (BCHP_CHIP==4538)  /* use this file only when building 4538 firmware */

#include "bchp_leap_l1.h"
#include "bchp_leap_l2.h"

#define BCHP_INT_ID_spi ((BCHP_LEAP_L1_INTR_W0_STATUS_LEAP_INTR_SHIFT << 8) | BCHP_LEAP_L2_CPU_STATUS_MSPI_DONE_INTR_SHIFT)
#define BCHP_INT_ID_uart_rx ((BCHP_LEAP_L1_INTR_W0_STATUS_LEAP_INTR_SHIFT << 8) | BCHP_LEAP_L2_CPU_STATUS_UART_RX_INTR_SHIFT)

#endif
#endif
