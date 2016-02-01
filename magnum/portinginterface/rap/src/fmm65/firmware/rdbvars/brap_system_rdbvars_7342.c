
/***************************************************************************
*     Copyright (c) 1999-2011, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*
* THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
* AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
* EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
***************************************************************************/
#include "bchp.h"
const uint32_t BRAP_IMG_system_rdbvars_array1[] = 
{
	0x00000000,	/* Version */
	0x00000000,	/* Version */
	0x00000000,	/* Version */
	0x20100804,	/* Version Date */
	0x00000200,	/* Dummy Clock */
	0x1038304c,	/* AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 */
	0x10383050,	/* AUD_DSP_CFG0_CONTROL_REGISTER1_CXT0 */
	0x10382904,	/* AUD_DSP_ESR_SO00_INT_SET */
	0x10382984,	/* AUD_DSP_ESR_SO10_INT_SET */
	0x10382a04,	/* AUD_DSP_ESR_SO20_INT_SET */
	0x10382058,	/* AUD_DSP_CTRL0_WATCHDOG_TIMER */
	0x10388480,	/* AUD_FMM_MISC_STC_UPPER0 */
	0x10327DF8,	/* BCHP_BSP_SECURITY_MEM_REGION_START_ADDR */
	0x10327DFC,	/* BCHP_BSP_SECURITY_MEM_REGION_END_ADDR */
	0x10327E04,	/* BCHP_BSP_SOFTIP_CONTROL_START_ADDR */
	0x1038207c,	/* AUD_DSP_CTRL0_UART_CTRL */
	0x10382078,	/* AUD_DSP_CTRL0_UART_XMIT_DATA */
	0x10382070,	/* AUD_DSP_CTRL0_UART_STATUS */
	0x10382180,	/* AUD_DSP_CTRL0_DMA_SRC_ADDR0 */
	0x10382184,	/* AUD_DSP_CTRL0_DMA_DST_ADDR0 */
	0x10382188,	/* AUD_DSP_CTRL0_DMA_TRANSFER0 */
	0x1038218c,	/* AUD_DSP_CTRL0_DMA_ABORT0 */
	0x10382190,	/* AUD_DSP_CTRL0_DMA_SRC_ADDR1 */
	0x10382194,	/* AUD_DSP_CTRL0_DMA_DST_ADDR1 */
	0x10382198,	/* AUD_DSP_CTRL0_DMA_TRANSFER1 */
	0x1038219c,	/* AUD_DSP_CTRL0_DMA_ABORT1 */
	0x103821a0,	/* AUD_DSP_CTRL0_DMA_STATUS */
	0x103821a4,	/* AUD_DSP_CTRL0_DMA_TEST_STATUS */
	0x103834f0,	/* AUD_DSP_CFG0_BASE_ADDR_FIFO_17 */
	0x10382030,	/* AUD_DSP_CTRL0_MAILBOX0 */
	0x10383500,	/* AUD_DSP_CFG0_SW_UNDEFINED_SPARE0 */
	0x10404000,	/* BCHP_SUN_TOP_CTRL_PROD_REVISION */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */
	0x00000000,	/* Reserved */

};
const uint32_t BRAP_IMG_system_rdbvars_header [2] = {sizeof(BRAP_IMG_system_rdbvars_array1), 1};
const void * const BRAP_IMG_system_rdbvars [2] = {BRAP_IMG_system_rdbvars_header, BRAP_IMG_system_rdbvars_array1};
