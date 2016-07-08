/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/


/* this file should not be directly included by unprepared code, but properly constructed code included it multiple times  */
/* ************** THIS FILE IS AUTOGENERATED. DO NOT EDIT **************************/
/*****
GENERATED by:
perl magnum/basemodules/chp/src/common/bchp_memc_clients_chip.pl magnum/basemodules/chp/include/7268/common/memc/bchp_memc_clients_chip_A0.txt magnum/basemodules/chp/include/7268/common/memc/bchp_memc_clients_chip_A0.h
*******/
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_RS,0)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_XC,1)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_CDB,2)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_ITB_MSG,3)                 /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_RS,4)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_XC_RMX_MSG,5)              /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_XC_RAVE,6)                 /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_PB,7)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_MEMDMA,8)                  /*      */
/*             Reserved for XPT */
BCHP_P_MEMC_DEFINE_CLIENT(GENET0_WR,10)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(GENET0_RD,11)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(GENET1_WR,12)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(GENET1_RD,13)                     /*      */



BCHP_P_MEMC_DEFINE_CLIENT(SATA,17)                          /*      */




BCHP_P_MEMC_DEFINE_CLIENT(BSP,22)                           /*     dedicated port */
BCHP_P_MEMC_DEFINE_CLIENT(SCPU,23)                          /*     dedicated port */
BCHP_P_MEMC_DEFINE_CLIENT(FLASH_DMA,24)                     /*      */

BCHP_P_MEMC_DEFINE_CLIENT(SDIO_EMMC,26)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(SDIO_CARD,27)                     /*      */

BCHP_P_MEMC_DEFINE_CLIENT(MCIF_RD,29)                       /*     M-Card, 1st port (read), (cablecard) */
BCHP_P_MEMC_DEFINE_CLIENT(MCIF_WR,30)                       /*     M-Card, 1st port (write), (cablecard) */
BCHP_P_MEMC_DEFINE_CLIENT(UART_DMA_RD,31)                   /*      */
BCHP_P_MEMC_DEFINE_CLIENT(UART_DMA_WR,32)                   /*      */
BCHP_P_MEMC_DEFINE_CLIENT(USB_HI_0,33)                      /*     Port 0 (STB owned) of dual port USB interface */
BCHP_P_MEMC_DEFINE_CLIENT(USB_LO_0,34)                      /*     Port 0 (STB owned) of dual port USB interface */
BCHP_P_MEMC_DEFINE_CLIENT(USB_X_WRITE_0,35)                 /*     Port 0 (STB owned) of dual port USB interface */
BCHP_P_MEMC_DEFINE_CLIENT(USB_X_READ_0,36)                  /*     Port 0 (STB owned) of dual port USB interface */
BCHP_P_MEMC_DEFINE_CLIENT(USB_X_CTRL_0,37)                  /*     Port 0 (STB owned) of dual port USB interface */
BCHP_P_MEMC_DEFINE_CLIENT(USB_X_BDC_0,38)                   /*     Port 0 (STB owned) of dual port USB interface */

BCHP_P_MEMC_DEFINE_CLIENT(RAAGA,40)                         /*      */
BCHP_P_MEMC_DEFINE_CLIENT(RAAGA_1,41)                       /*      */


BCHP_P_MEMC_DEFINE_CLIENT(AUD_AIO,44)                       /*      */
BCHP_P_MEMC_DEFINE_CLIENT(VEC_VIP0,55)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(VEC_VIP1,56)                      /*      */


BCHP_P_MEMC_DEFINE_CLIENT(HVD0_DBLK,73)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_DBLK_1,74)                   /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_ILCPU,75)                    /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_OLCPU,76)                    /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_CAB,77)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_ILSI,78)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_ILCPU_p2,79)                 /*     HVD0 Inner Loop ARC, 2nd pipeline */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_ILSI_p2,80)                  /*     HVD0 IL Symbol read, 2nd pipeline */
BCHP_P_MEMC_DEFINE_CLIENT(SID,87)                           /*     HVD0 SID */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_PIX_FD,94)                /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_QUANT,95)                 /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_PIX_CAP,96)               /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_PIX_FD,97)               /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_QUANT,98)                /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_PIX_CAP,99)              /*      */

BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD0,106)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD0_1,107)                   /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD1,108)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD1_1,109)                   /*      */

BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD0,118)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD1,119)                     /*      */

BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP0,126)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP1,127)                     /*      */

BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD0,134)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD1,135)                     /*      */

BCHP_P_MEMC_DEFINE_CLIENT(BVN_RDC,144)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(VEC_VBI_ENC0,145)                 /*      */

BCHP_P_MEMC_DEFINE_CLIENT(M2MC_0,147)                       /*     2D graphics - instance 0 - client "0" */
BCHP_P_MEMC_DEFINE_CLIENT(M2MC_1,148)                       /*     2D graphics - instance 0 - client "1" */
BCHP_P_MEMC_DEFINE_CLIENT(M2MC_2,149)                       /*     2D graphics - instance 0 - client "2" */

BCHP_P_MEMC_DEFINE_CLIENT(PCIE_0,151)                       /*      */



BCHP_P_MEMC_DEFINE_CLIENT(HVD0_DBLK_p2,155)                 /*     HVD0 Deblock Writes, 2nd pipeline */
BCHP_P_MEMC_DEFINE_CLIENT(HVD0_DBLK1_p2,156)                /*     HVD0 Deblock Writes / alternate blockout, 2nd pipeline */





/*BCHP_P_MEMC_DEFINE_CLIENT(SFE0,170)*/                         /*     SFE   */
/*BCHP_P_MEMC_DEFINE_CLIENT(SFE1,171)*/                         /*     SFE 1 */



/*             Ports 200 and up have special purposes */
BCHP_P_MEMC_DEFINE_CLIENT(HOST_CPU_MCP_R_HI,200)            /*     Host CPU MCP read client - high priority */
BCHP_P_MEMC_DEFINE_CLIENT(HOST_CPU_MCP_R_LO,201)            /*     Host CPU MCP read client - low priority */
BCHP_P_MEMC_DEFINE_CLIENT(HOST_CPU_MCP_W_HI,202)            /*     Host CPU MCP write client - high priority */
BCHP_P_MEMC_DEFINE_CLIENT(HOST_CPU_MCP_W_LO,203)            /*     Host CPU MCP write client - low priority */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_MCP_R_HI,204)                 /*     V3D (VC4 graphics core) MCP read client - high priority */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_MCP_R_LO,205)                 /*     V3D (VC4 graphics core) MCP read client - low priority */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_MCP_W_HI,206)                 /*     V3D (VC4 graphics core) MCP write client (tile buffer writes) - high priority */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_MCP_W_LO,207)                 /*     V3D (VC4 graphics core) MCP write client (tile buffer writes) - low priority */
BCHP_P_MEMC_DEFINE_CLIENT(UBUS_RD,208)                      /*      */
BCHP_P_MEMC_DEFINE_CLIENT(UBUS_WR,209)                      /*      */
/*             210:215 Reserved for UBUS clients */





BCHP_P_MEMC_DEFINE_CLIENT(HVD0_MOCOMP,216)                  /*     HVD0 PFRI / MOCOMP */
BCHP_P_MEMC_DEFINE_CLIENT(HVD1_MOCOMP,217)                  /*     HVD1 PFRI / MOCOMP */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_PFRI,218)                    /*     VICE2v2 instance 0 PFRI (required + optional) */

/*             219:231 Reserved for additional PFRI clients */











/*             232:247 Reserved for additional LMB ports */















BCHP_P_MEMC_DEFINE_CLIENT(TRACELOG,248)                     /*      */
BCHP_P_MEMC_DEFINE_CLIENT(MEMC_RESERVED_0,249)              /*     Reserved */
BCHP_P_MEMC_DEFINE_CLIENT(ZCQS_ENGINE,250)                  /*     DRAM ZQ Calibration Short client */
BCHP_P_MEMC_DEFINE_CLIENT(MSA,251)                          /*     MSA (Memory Soft Access) */
BCHP_P_MEMC_DEFINE_CLIENT(DIS0,252)                         /*     DIS (DRAM Interface Stress) #0 */
BCHP_P_MEMC_DEFINE_CLIENT(DIS1,253)                         /*     DIS (DRAM Interface Stress) #1 */
BCHP_P_MEMC_DEFINE_CLIENT(DRAM_INIT_ZQCS,254)               /*     DRAM Init and Low Power Mode Engine */
BCHP_P_MEMC_DEFINE_CLIENT(REFRESH,255)                      /*     Refresh */
/*  */
