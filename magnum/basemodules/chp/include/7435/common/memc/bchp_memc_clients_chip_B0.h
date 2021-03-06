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
/* this file should not be directly included, and it could be included multiple times  */
/* ************** THIS FILE IS AUTOGENERATED. DO NOT EDIT **************************/
/*****
GENERATED by:
perl magnum/basemodules/chp/src/common/bchp_memc_clients_chip.pl magnum/basemodules/chp/include/7435/common/memc/bchp_memc_clients_chip_B0.txt magnum/basemodules/chp/include/7435/common/memc/bchp_memc_clients_chip_B0.h
*******/
/* # entries copied (as 'select' -> copy -> 'paste into this file')  from http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Bussing/Released/BCM7435/scb_clients_B0.xls */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_RS,0,INVALID)              /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_XC,1,INVALID)              /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_CDB,2,INVALID)             /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_WR_ITB_MSG,3,INVALID)         /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_RS,4,INVALID)              /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_XC_RMX_MSG,5,INVALID)      /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_XC_RAVE,6,INVALID)         /*              */
BCHP_P_MEMC_DEFINE_CLIENT(XPT_RD_PB,7,INVALID)              /*              */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_DBLK,8,8)                     /*         AVD Deblock Writes */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_ILCPU,9,INVALID)              /*             AVD Inner Loop ARC  */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_OLCPU,10,INVALID)             /*             AVD Outer Loop ARC */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_CAB,11,INVALID)               /*             AVD CABAC  */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_ILSI,12,INVALID)              /*             AVD IL Symbol read */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_DBLK,13,13)                   /*         SVD Deblock Writes */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_ILCPU,14,INVALID)             /*             SVD Inner Loop ARC  */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_OLCPU,15,INVALID)             /*             SVD Outer Loop ARC */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_CAB,16,INVALID)               /*             SVD CABAC  */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_ILSI,17,INVALID)              /*             SVD IL Symbol read */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_BLCPU,18,INVALID)             /*             SVD base layer CPU */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_BLSI,19,INVALID)              /*             SVD BL Symbol Interpreter */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD3_PIX_CAP,20,20)           /*         New in BCM7435, to support fourth simultaneous transcode [Was SVD_MVSCL. Removed in RevM: SVD base layer MV scaler] */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD3_PIX_FD,21,21)            /*         New in BCM7435, to support fourth simultaneous transcode [Was SVD_SPIXSTR. Removed in RevM: SVD base layer SPIXSTORE / DBLK] */
BCHP_P_MEMC_DEFINE_CLIENT(BSP,22,22)                        /*         BSP (AEGIS) must remain at client 22. BSP requires special MEMC hardware. */
BCHP_P_MEMC_DEFINE_CLIENT(SCPU,23,INVALID)                  /*             SAGE CPU (BMIPS3300, single-SCB interface); only on SCB0. Was added in BCM7435B0. */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_DBLK_1,24,24)                 /*         SVD Deblock Writes, alternate blockout */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD3,25,25)                   /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(SATA,26,26)                       /*           */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_DBLK_1,27,27)                 /*         AVD Deblock Writes, alternate blockout (moved here to make room for SAGE at 23) */
BCHP_P_MEMC_DEFINE_CLIENT(CPU,28,28)                        /*         BMIPS5200 has 1 SCB and 1 LMB client per MEMC. (This is the SCB client, carrying requests from both cores) */
BCHP_P_MEMC_DEFINE_CLIENT(EDU,29,29)                        /*         EBI DMA Unit (Was FGT in older chips) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD3_QUANT,30,30)             /*         New in BCM7435, to support fourth simultaneous transcode [Was PCI/HIF_PCI. No PCI in BCM7435.] */
BCHP_P_MEMC_DEFINE_CLIENT(PCIE,31,31)                       /*         PCI Express */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP5,32,32)                   /*         New in BCM7435, to support fourth simultaneous transcode [Was IEEE_1394. No 1394/FireWire in BCM7435] */
BCHP_P_MEMC_DEFINE_CLIENT(MCIF_RD,33,INVALID)               /*             Smart Card; New in BCM7435: UART_DMA (for Bluetooth interaction), traffic multiplexed. */
BCHP_P_MEMC_DEFINE_CLIENT(MCIF_WR,34,INVALID)               /*             Smart Card */
BCHP_P_MEMC_DEFINE_CLIENT(SDIO_EMMC,35,35)                  /*          */
BCHP_P_MEMC_DEFINE_CLIENT(SDIO_CARD,36,36)                  /*          */
BCHP_P_MEMC_DEFINE_CLIENT(GENET0_WR,37,37)                  /*         Internal 10/100 EPHY, external 10/100/1000 GPHY, HPNA, HomePlug or similar. */
BCHP_P_MEMC_DEFINE_CLIENT(GENET0_RD,38,38)                  /*         Internal 10/100 EPHY, external 10/100/1000 GPHY, HPNA, HomePlug or similar. */
BCHP_P_MEMC_DEFINE_CLIENT(FLASH_DMA,39,39)                  /*         New client in BCM7435, accelerates NAND and Parallel-NOR transfers */
BCHP_P_MEMC_DEFINE_CLIENT(GENET1_WR,40,40)                  /*         Internal MoCA, external 10/100/1000 GPHY, HPNA, HomePlug or similar. */
BCHP_P_MEMC_DEFINE_CLIENT(GENET1_RD,41,41)                  /*         Internal MoCA, external 10/100/1000 GPHY, HPNA, HomePlug or similar. */
BCHP_P_MEMC_DEFINE_CLIENT(MOCA_MIPS,42,42)                  /*         This client is MOCA DMA which is used to load IMEM during boot time only */
BCHP_P_MEMC_DEFINE_CLIENT(USB_HI_0,43,43)                   /*         Also called "USB_20", carries UHCI traffic for first USB controller. Each controller drives 2 PHYs. */
BCHP_P_MEMC_DEFINE_CLIENT(USB_LO_0,44,44)                   /*         Also called "USB_11", carries OHCI traffic for first USB controller. Each controller drives 2 PHYs. */
BCHP_P_MEMC_DEFINE_CLIENT(USB_HI_1,45,45)                   /*         Second USB controller */
BCHP_P_MEMC_DEFINE_CLIENT(USB_LO_1,46,46)                   /*         Second USB controller */
BCHP_P_MEMC_DEFINE_CLIENT(RAAGA1,47,47)                     /*         Second instance in BCM7435, to support more Audio transcodes */
BCHP_P_MEMC_DEFINE_CLIENT(RAAGA1_1,48,48)                   /*         Second instance in BCM7435, to support more Audio transcodes */
BCHP_P_MEMC_DEFINE_CLIENT(RAAGA,49,49)                      /*         RAAGA audio engine */
BCHP_P_MEMC_DEFINE_CLIENT(AUD_AIO,50,50)                    /*         "AIO" is the new name for "FMM"; BCM7435 has AIO on both SCBs (bugfix) */
BCHP_P_MEMC_DEFINE_CLIENT(RAAGA_1,51,51)                    /*         RAAGA_1 is a "selective RTS" client for video apps. */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD2_PIX_CAP,52,52)           /*         MAD_R pixel capture (write) -- second MAD_R for dual transcode */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_RDC,53,53)                    /*         Register DMA controller */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD2_PIX_FD,54,54)            /*         MAD_R pixel feed (read) -- second MAD_R for dual transcode */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD2_QUANT,55,55)             /*         MAD_R quant motion history (R/W)  -- second MAD_R for dual transcode */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD2,56,56)                   /*         VICE BVN */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD2_1,57,57)                 /*         VICE BVN */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD1,58,58)                   /*         MPEG feeders have access to all DRAMs */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD1_1,59,59)                 /*         MFD Alternate blockout */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD0,60,60)                   /*         MPEG feeders have access to all DRAMs */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD0_1,61,61)                 /*         MFD Alternate blockout */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP4,62,62)                   /*         VICE BVN */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP3,63,63)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP2,64,64)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP1,65,65)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_CAP0,66,66)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD4,67,67)                   /*         VICE BVN */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD3,68,68)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD2,69,69)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD1,70,70)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD0,71,71)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MCVP0,72,72)                  /*         AKA "client 0" and "rwc0" -- 16JW R/W (MCVP contains MCDI) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MCVP1,73,73)                  /*         AKA "client 1" and "rrc1" -- 16JW R */
/* # TPCAP    74    74    BVN_MAD_PIX_CAP        Test Port Capture; MAD_R pixel capture (write) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_PIX_CAP,74,74)            /* Test Port Capture; MAD_R pixel capture (write) */
/* #        75    BVN_MAD_PIX_FD        MAD_R pixel feed (read) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_PIX_FD,75,75)             /* MAD_R pixel feed (read) */
/* #        76    BVN_MAD_QUANT        MAD_R quant motion history (R/W) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD_QUANT,76,76)              /* MAD_R quant motion history (R/W) */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD3,77,77)                   /*         VICE BVN */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD2,78,78)                   /*         Third output "Echostar" GFD */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD1,79,79)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD0,80,80)                   /*          */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_VFD5,81,81)                   /*         New in BCM7435, to support fourth simultaneous transcode [Was VEC_VBI_ENC2. No analog tertiary VEC in BCM7435] */
BCHP_P_MEMC_DEFINE_CLIENT(VEC_VBI_ENC1,82,INVALID)          /*             TTX1 -- assigned to VBI path 1 (typically IT_1, SD/CVBS display) and arbitrated with ITU656 path 1 */
BCHP_P_MEMC_DEFINE_CLIENT(VEC_VBI_ENC0,83,INVALID)          /*             TTX0 -- assigned to VBI path 0 (typically IT_0, HD/component display) and arbitrated with ITU656 path 0 */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MCVP2,84,84)                  /*         AKA "client 2" and "rwc2" -- QM client for MCDI */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_PIX_CAP,85,85)           /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(M2M_DMA1,86,86)                   /*         Second instance in BCM7435, something not done since 7400 */
BCHP_P_MEMC_DEFINE_CLIENT(M2M_DMA0,87,87)                   /*         Note that M2M DMA has simultaneous access to all SCBs */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_0,88,88)                      /*         VC4 3D graphics core (replaces Pix3D/pirhana) */
BCHP_P_MEMC_DEFINE_CLIENT(V3D_1,89,89)                      /*         VC4 3D graphics core -- Tile Buffer writes */
BCHP_P_MEMC_DEFINE_CLIENT(M2MC0_GFX,90,90)                  /*         M2MC has access to all SCBs (Called "GFX" for BCM7038) */
BCHP_P_MEMC_DEFINE_CLIENT(M2MC1,91,91)                      /*         Second instance in BCM7435, potentially for WebApp use */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_PIX_FD,92,92)            /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MAD1_QUANT,93,93)             /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(SID,94,94)                        /*         Still Image decoder */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP0_INST0,95,95)            /* VICE1_VIP0_INST0        "VICE Clients, all VICE clients have access to SCB0. */
/* (module: o_scb_420_req. vice2: o_vip0_scb1_req) write only. Luma and 420 chroma" */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP1_INST0,96,96)            /* VICE1_VIP1_INST0        (module: o_scb_dec_y_req. vice2: o_vip1_scb1_req) read/write. Write: 2h2v_luma, 2h1v_luma. Read: pcc_luma, hist_luma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP2_INST0,97,97)            /* VICE1_VIP2_INST0        (module: o_scb_shift_c_req. vice2: o_vip2_scb1_req) write only. Shift_chroma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_CME0,98,98)                  /* VICE1_CME0        (module: o_scb_xmb_req. vice2: o_cme0_scb1_req) read only. Input and reference picture luma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_CME1,99,99)                  /* VICE1_CME1        (module: o_scb_csc_req. vice2: o_cme1_scb1_req) write only. CMV, SAD, Confidence values */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_FME0,100,100)                /* VICE1_FME0        (module: o_scb_csc_req. vice2: o_fme0_scb1_req) read only. CMV, SAD, Confidence values */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_FME1,101,101)                /* VICE1_FME1        (module: o_scb_cmb_y_req. vice2: o_fme1_scb1_req) read only. CMB luma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_FME2,102,102)                /* VICE1_FME2        (module: o_scb_cmb_c_req. vice2: o_fme2_scb1_req) read only. CMB chroma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_SG,103,INVALID)              /*             (module: o_scb_sg_req. vice2: o_sg_scb0_req) write only. Coded bits / bins */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_DBLK,104,104)                /* VICE1_DBLK        (module: o_scb_dblk_req. vice2: o_dblk_scb1_req) write only. Deblocked, reconstructed MBs */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_CABAC0,105,INVALID)          /*             "(module: o_scb_cabac0_req. vice2: o_cabac0_scb0_req) write only.  */
/* CABAC0 writes final bitstream to SCB0. (All clear compressed data must be on SCB0)" */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_CABAC1,106,INVALID)          /*             (module: o_scb_cabac1_req. vice2: o_cabac1_scb0_req) read only. Bits / bins from SG */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_ARCSS0,107,107)              /* VICE1_ARCSS0        (module: o_arcss_scb0_req. vice2: o_arcss0_scb1_req) read/write. Firmware code and data */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP0_INST1,108,108)          /* VICE1_VIP0_INST1        Second VIP instance for dual transcode. write only. Luma and 420 chroma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP1_INST1,109,109)          /* VICE1_VIP1_INST1        Second VIP instance for dual transcode. write only. read/write. Write: 2h2v_luma, 2h1v_luma. Read: pcc_luma, hist_luma */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_VIP2_INST1,110,110)          /* VICE1_VIP2_INST1        Second VIP instance for dual transcode. write only. write only. Shift_chroma */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD4,111,111)                 /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(VICE1_SG,112,INVALID)             /*             Was DOCSIS. New in BCM7435: second ViCE. All clear compressed data must be on SCB0 */
BCHP_P_MEMC_DEFINE_CLIENT(VICE1_CABAC0,113,INVALID)         /*             Was DOCSIS. New in BCM7435: second ViCE. All clear compressed data must be on SCB0 */
BCHP_P_MEMC_DEFINE_CLIENT(VICE1_CABAC1,114,INVALID)         /*             Was DOCSIS. New in BCM7435: second ViCE. All clear compressed data must be on SCB0 */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_MFD3_1,115,115)               /*         New in BCM7435, to support third transcode */
BCHP_P_MEMC_DEFINE_CLIENT(BVN_GFD5,116,116)                 /*         New in BCM7435, to support fourth simultaneous transcode  */
BCHP_P_MEMC_DEFINE_CLIENT(DIS2,117,117)                     /*         Internal data stress client (test) */
BCHP_P_MEMC_DEFINE_CLIENT(MSA,118,118)                      /*         MSA soft access */
BCHP_P_MEMC_DEFINE_CLIENT(DIS,119,119)                      /*         Internal data stress client (test) */
BCHP_P_MEMC_DEFINE_CLIENT(AVD_MOCOMP,120,120)               /*         AVD MOCOMP/DBLK PFRI */
BCHP_P_MEMC_DEFINE_CLIENT(SVD_MOCOMP,121,121)               /*         SVD MOCOMP/DBLK PFRI */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_OPT,122,122)                 /* VICE1_OPT        VICE optional PFRI fetches. [Was on 116 in 7425B0] [Client 122 was SVD_SPIXSCALE. Removed in RevM: SVD SpixScale PFRI] */
BCHP_P_MEMC_DEFINE_CLIENT(VICE_REQ,123,123)                 /* VICE1_REQ        VICE required PFRI fetches */
BCHP_P_MEMC_DEFINE_CLIENT(CPU_LMB_HI,124,124)               /*         High priority LMB client  */
BCHP_P_MEMC_DEFINE_CLIENT(CPU_LMB_LO,125,125)               /*         1-LMB bus fans into 2 SCB clients inside MEMC. */
BCHP_P_MEMC_DEFINE_CLIENT(DRAM_INIT,126,126)                /*         MEMC internal client */
BCHP_P_MEMC_DEFINE_CLIENT(REF,127,127)                      /*         Internal Refresh */
/*  */
