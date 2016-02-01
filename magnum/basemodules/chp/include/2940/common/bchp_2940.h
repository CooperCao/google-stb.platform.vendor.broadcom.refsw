/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
 ***************************************************************************/

#ifndef BCHP_2940_H__
#define BCHP_2940_H__

/* I2C host registers */
#define BCM2940_DMA_START_ADDR0 0x80
#define BCM2940_DMA_START_ADDR1 0x81
#define BCM2940_DMA_START_ADDR2 0x82
#define BCM2940_DMA_START_ADDR3 0x83
#define BCM2940_DMA_LEN_LO 0x84
#define BCM2940_DMA_LEN_HI 0x85
#define BCM2940_DMA_CONTROL 0x86
#define BCM2940_WFIFO_DATA 0x92
#define BCM2940_RFIFO_DEPTH 0xA1
#define BCM2940_RFIFO_DATA 0xA2
#define BCM2940_I2C_STATUS 0x89

#define BCHP_DMA_CONTROL_EN 1
#define BCHP_DMA_CONTROL_WRITE 2
#define BCHP_DMA_CONTROL_READ 0

#define BCHP_STATUS_I2C_COMPLETE (1<<0)
#define BCHP_STATUS_WFIFO_OVERWRITE (1<<1)
#define BCHP_STATUS_WFIFO_UNDERRUN (1<<2)
#define BCHP_STATUS_RFIFO_OVERFLOW (1<<3)
#define BCHP_STATUS_RFIFO_UNDERRUN (1<<4)
#define BCHP_STATUS_DMA_ERROR (1<<5)


/* Common block */
#define BCM2940_COMMON_CHIPID 0x840000
#define BCM2940_COMMON_SR 0x840004
#define BCM2940_COMMON_REMAP 0x840008
#define BCM2940_COMMON_CE 0x84000C
#define BCM2940_COMMON_SPARE 0x8400A8

#define BCHP_REMAP_BOOT_DONE (1<<1)
#define BCHP_SR_CHIP_SOFT_RST (1<<0)
#define BCHP_SR_ARM_SOFT_RST_HOLD (1<<20)

/* INTC block */
#define BCM2940_INTC_ICINTENCLEAR 0xD00014


#endif /* BCHP_2940_H__ */
