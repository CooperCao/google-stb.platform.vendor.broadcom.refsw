/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Aug 14 09:27:09 2015
 *                 Full Compile MD5 Checksum  498077c0ce1e95e6ab4307854b2612d7
 *                     (minus title and desc)
 *                 MD5 Checksum               d5991b102b67d0c014966f49ee5cb996
 *
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     16421
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/generate_int_id.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#include "bchp.h"
#include "bchp_raaga_dsp_inth.h"

#ifndef BCHP_INT_ID_RAAGA_DSP_INTH_H__
#define BCHP_INT_ID_RAAGA_DSP_INTH_H__

#define BCHP_INT_ID_DAISY_CHAIN_TIMER_INT     BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DAISY_CHAIN_TIMER_INT_SHIFT)
#define BCHP_INT_ID_DMA_Q0                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q0_SHIFT)
#define BCHP_INT_ID_DMA_Q1                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q1_SHIFT)
#define BCHP_INT_ID_DMA_Q2                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q2_SHIFT)
#define BCHP_INT_ID_DMA_Q3                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q3_SHIFT)
#define BCHP_INT_ID_DMA_Q4                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q4_SHIFT)
#define BCHP_INT_ID_DMA_Q5                    BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q5_SHIFT)
#define BCHP_INT_ID_DMA_SUB_ERROR0            BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_SUB_ERROR0_SHIFT)
#define BCHP_INT_ID_DSP_CLK_TIMER_INT         BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_DSP_CLK_TIMER_INT_SHIFT)
#define BCHP_INT_ID_GISB_ERROR                BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_GISB_ERROR_SHIFT)
#define BCHP_INT_ID_MDMEM_MISS_ERROR          BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_MDMEM_MISS_ERROR_SHIFT)
#define BCHP_INT_ID_MEM_PEEK_POKE             BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_SHIFT)
#define BCHP_INT_ID_MEM_SUB_ERROR             BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_SUB_ERROR_SHIFT)
#define BCHP_INT_ID_PMEM_BRIDGE_ERROR         BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_PMEM_BRIDGE_ERROR_SHIFT)
#define BCHP_INT_ID_RICH                      BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_RICH_SHIFT)
#define BCHP_INT_ID_SEC_PROT_VIOL             BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_SEC_PROT_VIOL_SHIFT)
#define BCHP_INT_ID_SYS_CLK_TIMER_INT0        BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT0_SHIFT)
#define BCHP_INT_ID_SYS_CLK_TIMER_INT1        BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT1_SHIFT)
#define BCHP_INT_ID_SYS_CLK_TIMER_INT2        BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT2_SHIFT)
#define BCHP_INT_ID_SYS_CLK_TIMER_INT3        BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT3_SHIFT)
#define BCHP_INT_ID_TSM_TIMER_INT             BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_TSM_TIMER_INT_SHIFT)
#define BCHP_INT_ID_UART_ERROR                BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_ERROR_SHIFT)
#define BCHP_INT_ID_UART_RX                   BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_RX_SHIFT)
#define BCHP_INT_ID_UART_TX                   BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_TX_SHIFT)
#define BCHP_INT_ID_VOM_MISS_INT              BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_VOM_MISS_INT_SHIFT)
#define BCHP_INT_ID_WATCHDOG_TIMER_ATTN       BCHP_INT_ID_CREATE(BCHP_RAAGA_DSP_INTH_HOST_STATUS, BCHP_RAAGA_DSP_INTH_HOST_STATUS_WATCHDOG_TIMER_ATTN_SHIFT)

#endif /* #ifndef BCHP_INT_ID_RAAGA_DSP_INTH_H__ */

/* End of File */
