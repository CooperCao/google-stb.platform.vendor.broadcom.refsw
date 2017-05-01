/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ***************************************************************************/
/* this file should never be included, it holds all known mappings from the Chip Design (HW) IO client names:
 *   (1) to HW blocks used by SW
 * content of this file used by the bchp_memc_clients_chip_map.pl script
 * script supports M:1 mapping (different HW names for different chips correspond to the same HW Block)
 * and it doesn't support 1:M mapping (the same HW name on different chips correspond to different HW Blocks)
 *
 * #define BCHP_P_IO_DEFINE_CLIENT_MAP(HW_CLIENT_NAME,HW_BLOCK_NAME)
 */
BCHP_P_IO_DEFINE_CLIENT_MAP(ASP_0,ASP)
BCHP_P_IO_DEFINE_CLIENT_MAP(AVD_0,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(AVD_1,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(AVD,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(AVS_0,AVS)
BCHP_P_IO_DEFINE_CLIENT_MAP(BBSI_SPI,BBSI)
BCHP_P_IO_DEFINE_CLIENT_MAP(BSP_0,BSP)
BCHP_P_IO_DEFINE_CLIENT_MAP(BSP,BSP)
BCHP_P_IO_DEFINE_CLIENT_MAP(CPU_0,CPU)
BCHP_P_IO_DEFINE_CLIENT_MAP(CPU,CPU)
BCHP_P_IO_DEFINE_CLIENT_MAP(CPU_S_0,CPU_S)
BCHP_P_IO_DEFINE_CLIENT_MAP(HVD_0,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(HVD_1,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(HVD_2,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(JTAG_0,JTAG)
BCHP_P_IO_DEFINE_CLIENT_MAP(JTAG,JTAG)
BCHP_P_IO_DEFINE_CLIENT_MAP(LEAP_0,FRONTEND)
BCHP_P_IO_DEFINE_CLIENT_MAP(LEAP,FRONTEND)
BCHP_P_IO_DEFINE_CLIENT_MAP(PCIE_0,PCIE)
BCHP_P_IO_DEFINE_CLIENT_MAP(PCIE_1,PCIE)
BCHP_P_IO_DEFINE_CLIENT_MAP(PCIE,PCIE)
BCHP_P_IO_DEFINE_CLIENT_MAP(RAAGA_0,AUD)
BCHP_P_IO_DEFINE_CLIENT_MAP(RAAGA_1,AUD)
BCHP_P_IO_DEFINE_CLIENT_MAP(RAAGA,AUD)
BCHP_P_IO_DEFINE_CLIENT_MAP(RDC_0,RDC)
BCHP_P_IO_DEFINE_CLIENT_MAP(RDC,RDC)
BCHP_P_IO_DEFINE_CLIENT_MAP(SCPU_0,SCPU
BCHP_P_IO_DEFINE_CLIENT_MAP(SCPU_0,SCPU)
BCHP_P_IO_DEFINE_CLIENT_MAP(SCPU,SCPU)
BCHP_P_IO_DEFINE_CLIENT_MAP(SSP_0,SSP)
BCHP_P_IO_DEFINE_CLIENT_MAP(SSP,SSP)
BCHP_P_IO_DEFINE_CLIENT_MAP(SVD_0,AVD)
BCHP_P_IO_DEFINE_CLIENT_MAP(VICE_0,VICE)
BCHP_P_IO_DEFINE_CLIENT_MAP(VICE_1,VICE)
BCHP_P_IO_DEFINE_CLIENT_MAP(WEBCPU_0,WEBCPU)
BCHP_P_IO_DEFINE_CLIENT_MAP(WEBCPU,WEBCPU)
