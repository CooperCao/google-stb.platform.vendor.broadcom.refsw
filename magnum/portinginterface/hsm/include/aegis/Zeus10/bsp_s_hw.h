/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * Broadcom Security Processor Code
 ***************************************************************************/
#ifndef BSP_S_HW_7400_H__
#define BSP_S_HW_7400_H__


/* multi2 system keys supported by hardware*/
#define BCMD_MULTI2_MAXSYSKEY                     8
/* Total number of pid channels supported */
#define BCMD_TOTAL_PIDCHANNELS                    256
/* RAM User Key size (in bytes) */
#define BCMD_KEYLADDER_KEYRAM_SIZE                32  /* in bytes (256 bits per key)*/

/* Mem2mem key table boundary assigned to key slot*/
#define BCMD_MEM2MEMKEYSLOTSIZE  6
#define BCMD_MEM2MEMKEYTABLE_BASE 0
#define BCMD_MEM2MEMKEYTABLE_TOP  256

#define BCMD_MAX_M2M_KEY_SLOT   ((BCMD_MEM2MEMKEYTABLE_TOP-BCMD_MEM2MEMKEYTABLE_BASE)/BCMD_MEM2MEMKEYSLOTSIZE)

/* Host side */
#define BHSM_IN_BUF1_ADDR		BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE
#define BHSM_IN_BUF2_ADDR  		BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 1)
#define BHSM_OUT_BUF1_ADDR  	BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 2)
#define BHSM_OUT_BUF2_ADDR  	BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 3)

#endif  /* BSP_S_HW_7400_H__ end of header file*/
