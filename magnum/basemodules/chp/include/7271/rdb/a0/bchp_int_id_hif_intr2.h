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
 * Date:           Generated on               Thu Jun 18 10:59:35 2015
 *                 Full Compile MD5 Checksum  32b78c1804e11666b824f2b9450a6228
 *                     (minus title and desc)
 *                 MD5 Checksum               3452ff65b8043c1c458e059705af3b49
 *
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     16265
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/generate_int_id.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#include "bchp.h"
#include "bchp_hif_intr2.h"

#ifndef BCHP_INT_ID_HIF_INTR2_H__
#define BCHP_INT_ID_HIF_INTR2_H__

#define BCHP_INT_ID_FLASH_DMA_DONE_INTR       BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_FLASH_DMA_DONE_INTR_SHIFT)
#define BCHP_INT_ID_FLASH_DMA_ERR_INTR        BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_FLASH_DMA_ERR_INTR_SHIFT)
#define BCHP_INT_ID_HIF_RGR2_BRIDGE_INTR      BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_HIF_RGR2_BRIDGE_INTR_SHIFT)
#define BCHP_INT_ID_ITCH0_RD_INTR             BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_ITCH0_RD_INTR_SHIFT)
#define BCHP_INT_ID_ITCH1_RD_INTR             BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_ITCH1_RD_INTR_SHIFT)
#define BCHP_INT_ID_NAND_BLKERA_INTR          BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_BLKERA_INTR_SHIFT)
#define BCHP_INT_ID_NAND_CORR_INTR            BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_CORR_INTR_SHIFT)
#define BCHP_INT_ID_NAND_CPYBK_INTR           BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_CPYBK_INTR_SHIFT)
#define BCHP_INT_ID_NAND_CTLRDY_INTR          BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_SHIFT)
#define BCHP_INT_ID_NAND_NP_READ_INTR         BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_NP_READ_INTR_SHIFT)
#define BCHP_INT_ID_NAND_PGMPG_INTR           BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_PGMPG_INTR_SHIFT)
#define BCHP_INT_ID_NAND_RBPIN_INTR           BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_RBPIN_INTR_SHIFT)
#define BCHP_INT_ID_NAND_UNC_INTR             BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_NAND_UNC_INTR_SHIFT)
#define BCHP_INT_ID_PCIE_0_LINKDOWN_INTR      BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_PCIE_0_LINKDOWN_INTR_SHIFT)
#define BCHP_INT_ID_PCIE_0_LINKUP_INTR        BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_PCIE_0_LINKUP_INTR_SHIFT)
#define BCHP_INT_ID_PCIE_0_RGR_BRIDGE_INTR    BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_PCIE_0_RGR_BRIDGE_INTR_SHIFT)
#define BCHP_INT_ID_PCIE_0_RG_BRIDGE_INTR     BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_PCIE_0_RG_BRIDGE_INTR_SHIFT)
#define BCHP_INT_ID_WEBHIF_WD_TIMEOUT_INTR    BCHP_INT_ID_CREATE(BCHP_HIF_INTR2_CPU_STATUS, BCHP_HIF_INTR2_CPU_STATUS_WEBHIF_WD_TIMEOUT_INTR_SHIFT)

#endif /* #ifndef BCHP_INT_ID_HIF_INTR2_H__ */

/* End of File */
