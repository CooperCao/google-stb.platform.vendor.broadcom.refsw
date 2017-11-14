/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 ***************************************************************************/


/* this file should not be directly included, and it could be included multiple times  */
/* if new entry have to be added please add it into the  bchp_memc_clients_chip_map_all.h and regenerate file */
/* if for whatever reason file has to be manually edited please remove line below */
/* ************** THIS FILE IS AUTOGENERATED. DO NOT EDIT **************************/
/*****
GENERATED by:
perl magnum/basemodules/chp/src/common/bchp_memc_clients_chip_map.pl magnum/basemodules/chp/src/common/bchp_memc_clients_chip_map_all.h magnum/basemodules/chp/include/7563/common/memc/bchp_memc_clients_chip.h magnum/basemodules/chp/include/7563/common/memc/bchp_memc_clients_chip_map.h
*******/


#if defined(BCHP_P_MEMC_DEFINE_CLIENT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_WR_RS,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_WR_XC,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_WR_CDB,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_WR_ITB_MSG,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_RD_RS,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_RD_XC_RMX_MSG,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_RD_XC_RAVE,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(XPT_RD_PB,XPT,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_DBLK,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_ILCPU,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_OLCPU,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_CAB,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_ILSI,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BSP,BSP,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD_DBLK_1,AVD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(T2_TDI_0,T2_TDI,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(T2_TDI_1,T2_TDI,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(CPU,CPU,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(SDIO_CARD,SDIO,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(GENET0_WR,ETHERNET,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(GENET0_RD,ETHERNET,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(USB0_0,USB,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(USB0_1,USB,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(RAAGA,AUD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(VEC_AIO,VEC,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(RAAGA_1,AUD,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_RDC,BVN,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_MFD0,BVN,MFD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_MFD0_1,BVN,MFD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_CAP1,BVN,CAP_1)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_CAP0,BVN,CAP_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_VFD1,BVN,VFD_1)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_VFD0,BVN,VFD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_MAD_PIX_A,BVN,MAD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_MAD_PIX_B,BVN,MAD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_MAD_QUANT,BVN,MAD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_GFD1,BVN,GFD_1)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(BVN_GFD0,BVN,GFD_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(VEC_VBI_ENC0,VEC_VBI,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(M2M_DMA0,M2M,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(M2MC,M2MC,GFX_0)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(MSA,MSA,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(MEMC_DIS,MEMC,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(AVD0_PFRI,PREFETCH,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(CPU_LMB_HIGH,CPU,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(CPU_LMB_LOW,CPU,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(MEMC_DRAM_INIT,MEMC,NOT_MAP)
BCHP_P_MEMC_DEFINE_CLIENT_MAP(REF,MEMC,NOT_MAP)

#else /* #if defined(BCHP_P_MEMC_DEFINE_CLIENT_MAP) */

#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_WR_RS 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_WR_XC 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_WR_CDB 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_WR_ITB_MSG 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_RD_RS 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_RD_XC_RMX_MSG 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_RD_XC_RAVE 1
#define BCHP_P_MEMC_CLIENT_EXISTS_XPT_RD_PB 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_DBLK 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_ILCPU 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_OLCPU 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_CAB 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_ILSI 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BSP 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD_DBLK_1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_T2_TDI_0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_T2_TDI_1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_CPU 1
#define BCHP_P_MEMC_CLIENT_EXISTS_SDIO_CARD 1
#define BCHP_P_MEMC_CLIENT_EXISTS_GENET0_WR 1
#define BCHP_P_MEMC_CLIENT_EXISTS_GENET0_RD 1
#define BCHP_P_MEMC_CLIENT_EXISTS_USB0_0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_USB0_1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_RAAGA 1
#define BCHP_P_MEMC_CLIENT_EXISTS_VEC_AIO 1
#define BCHP_P_MEMC_CLIENT_EXISTS_RAAGA_1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_RDC 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_MFD0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_MFD0_1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_CAP1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_CAP0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_VFD1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_VFD0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_MAD_PIX_A 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_MAD_PIX_B 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_MAD_QUANT 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_GFD1 1
#define BCHP_P_MEMC_CLIENT_EXISTS_BVN_GFD0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_VEC_VBI_ENC0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_M2M_DMA0 1
#define BCHP_P_MEMC_CLIENT_EXISTS_M2MC 1
#define BCHP_P_MEMC_CLIENT_EXISTS_MSA 1
#define BCHP_P_MEMC_CLIENT_EXISTS_MEMC_DIS 1
#define BCHP_P_MEMC_CLIENT_EXISTS_AVD0_PFRI 1
#define BCHP_P_MEMC_CLIENT_EXISTS_CPU_LMB_HIGH 1
#define BCHP_P_MEMC_CLIENT_EXISTS_CPU_LMB_LOW 1
#define BCHP_P_MEMC_CLIENT_EXISTS_MEMC_DRAM_INIT 1
#define BCHP_P_MEMC_CLIENT_EXISTS_REF 1
#endif /* #else #if defined(BCHP_P_MEMC_DEFINE_CLIENT_MAP) */
