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
 * Date:           Generated on               Mon Aug 24 11:29:33 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_DECODE_MAIN_0_H__
#define BCHP_DECODE_MAIN_0_H__

/***************************************************************************
 *DECODE_MAIN_0
 ***************************************************************************/
#define BCHP_DECODE_MAIN_0_REG_MAINCTL           0x20020100 /* [RW] Decoder Control */
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE         0x20020104 /* [RW] Size of the picture being decoded */
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION       0x20020108 /* [RO] Version of the decoder core */
#define BCHP_DECODE_MAIN_0_REG_STATUS            0x20020110 /* [RO] Provides back-end decoder processing status */
#define BCHP_DECODE_MAIN_0_REG_PMONCTL           0x20020120 /* [RW] Performance Monitoring */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0          0x20020124 /* [RO] REG_PMONCNT0 */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1          0x20020128 /* [RO] REG_PMONCNT1 */
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL        0x2002012c /* [RW] REG_PMON_MBCTL */
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL     0x20020130 /* [RW] DBLK_BUFF_CONTROL */
#define BCHP_DECODE_MAIN_0_CRC_CONTROL           0x20020134 /* [RW] DBLK CRC CONTROL register */
#define BCHP_DECODE_MAIN_0_CRC_SEED              0x20020138 /* [RW] DBLK CRC SEED register */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_Y          0x2002013c /* [RO] DBLK Luma CRC/Checksum result register */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CB         0x20020140 /* [RO] DBLK Chroma (Cb) CRC/Checksum result register */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CR         0x20020144 /* [RO] DBLK Chroma (Cr) CRC/Checksum result register */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL  0x20020150 /* [RW] VP6 Mocomp Control */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_AUTO     0x20020154 /* [RW] VP6 Mocomp Auto Filter Selection */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_ALPHA    0x20020158 /* [RW] VP6 Mocomp Alpha Filter Selection */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_FLIMIT   0x2002015c /* [RW] VP6 Mocomp Flimit Control */
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG     0x20020160 /* [RW] Backend debug Select */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG      0x20020164 /* [RO] VC1 Mocomp Debug */
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG   0x20020168 /* [RO] Qpel FIFO Debug */
#define BCHP_DECODE_MAIN_0_REG_MAIN_END          0x200201fc /* [RW] REG_MAIN_END */

/***************************************************************************
 *REG_MAINCTL - Decoder Control
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_MAINCTL :: USE_2_OFF [31:31] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_USE_2_OFF_MASK              0x80000000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_USE_2_OFF_SHIFT             31
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_USE_2_OFF_DEFAULT           0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: reserved0 [30:30] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved0_MASK              0x40000000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved0_SHIFT             30

/* DECODE_MAIN_0 :: REG_MAINCTL :: QPC_OFFSET2 [29:24] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QPC_OFFSET2_MASK            0x3f000000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QPC_OFFSET2_SHIFT           24
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QPC_OFFSET2_DEFAULT         0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: reserved1 [23:22] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved1_MASK              0x00c00000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved1_SHIFT             22

/* DECODE_MAIN_0 :: REG_MAINCTL :: QpC_Offset [21:16] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QpC_Offset_MASK             0x003f0000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QpC_Offset_SHIFT            16
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_QpC_Offset_DEFAULT          0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: reserved2 [15:13] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved2_MASK              0x0000e000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved2_SHIFT             13

/* DECODE_MAIN_0 :: REG_MAINCTL :: use_alt_mocomp [12:12] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_mocomp_MASK         0x00001000
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_mocomp_SHIFT        12
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_mocomp_DEFAULT      0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: use_alt_xform [11:11] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_xform_MASK          0x00000800
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_xform_SHIFT         11
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_use_alt_xform_DEFAULT       0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: Standard [10:07] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Standard_MASK               0x00000780
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Standard_SHIFT              7
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Standard_DEFAULT            0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: Profile [06:04] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Profile_MASK                0x00000070
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Profile_SHIFT               4
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Profile_DEFAULT             0x00000000

/* DECODE_MAIN_0 :: REG_MAINCTL :: reserved3 [03:01] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved3_MASK              0x0000000e
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_reserved3_SHIFT             1

/* DECODE_MAIN_0 :: REG_MAINCTL :: Rst [00:00] */
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK                    0x00000001
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_SHIFT                   0
#define BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_DEFAULT                 0x00000000

/***************************************************************************
 *REG_FRAMESIZE - Size of the picture being decoded
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_FRAMESIZE :: reserved0 [31:27] */
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_reserved0_MASK            0xf8000000
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_reserved0_SHIFT           27

/* DECODE_MAIN_0 :: REG_FRAMESIZE :: Lines [26:16] */
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_Lines_MASK                0x07ff0000
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_Lines_SHIFT               16

/* DECODE_MAIN_0 :: REG_FRAMESIZE :: reserved1 [15:11] */
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_reserved1_MASK            0x0000f800
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_reserved1_SHIFT           11

/* DECODE_MAIN_0 :: REG_FRAMESIZE :: Pixels [10:00] */
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_Pixels_MASK               0x000007ff
#define BCHP_DECODE_MAIN_0_REG_FRAMESIZE_Pixels_SHIFT              0

/***************************************************************************
 *REG_DEC_VERSION - Version of the decoder core
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_DEC_VERSION :: Major [31:16] */
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Major_MASK              0xffff0000
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Major_SHIFT             16
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Major_DEFAULT           0x00000013

/* DECODE_MAIN_0 :: REG_DEC_VERSION :: Minor [15:08] */
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Minor_MASK              0x0000ff00
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Minor_SHIFT             8
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_Minor_DEFAULT           0x00000000

/* DECODE_MAIN_0 :: REG_DEC_VERSION :: FixID [07:00] */
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_FixID_MASK              0x000000ff
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_FixID_SHIFT             0
#define BCHP_DECODE_MAIN_0_REG_DEC_VERSION_FixID_DEFAULT           0x00000000

/***************************************************************************
 *REG_STATUS - Provides back-end decoder processing status
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_STATUS :: Ixfm [31:30] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Ixfm_MASK                    0xc0000000
#define BCHP_DECODE_MAIN_0_REG_STATUS_Ixfm_SHIFT                   30

/* DECODE_MAIN_0 :: REG_STATUS :: Spre [29:28] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Spre_MASK                    0x30000000
#define BCHP_DECODE_MAIN_0_REG_STATUS_Spre_SHIFT                   28

/* DECODE_MAIN_0 :: REG_STATUS :: Mcom [27:26] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Mcom_MASK                    0x0c000000
#define BCHP_DECODE_MAIN_0_REG_STATUS_Mcom_SHIFT                   26

/* DECODE_MAIN_0 :: REG_STATUS :: reserved0 [25:23] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_reserved0_MASK               0x03800000
#define BCHP_DECODE_MAIN_0_REG_STATUS_reserved0_SHIFT              23

/* DECODE_MAIN_0 :: REG_STATUS :: vp8_recon_clamp_error [22:22] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_vp8_recon_clamp_error_MASK   0x00400000
#define BCHP_DECODE_MAIN_0_REG_STATUS_vp8_recon_clamp_error_SHIFT  22

/* DECODE_MAIN_0 :: REG_STATUS :: InpBuf_Overflow [21:16] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_InpBuf_Overflow_MASK         0x003f0000
#define BCHP_DECODE_MAIN_0_REG_STATUS_InpBuf_Overflow_SHIFT        16

/* DECODE_MAIN_0 :: REG_STATUS :: mocomp_data_avail [15:15] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_mocomp_data_avail_MASK       0x00008000
#define BCHP_DECODE_MAIN_0_REG_STATUS_mocomp_data_avail_SHIFT      15

/* DECODE_MAIN_0 :: REG_STATUS :: xform_data_avail [14:14] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_xform_data_avail_MASK        0x00004000
#define BCHP_DECODE_MAIN_0_REG_STATUS_xform_data_avail_SHIFT       14

/* DECODE_MAIN_0 :: REG_STATUS :: Output [13:12] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Output_MASK                  0x00003000
#define BCHP_DECODE_MAIN_0_REG_STATUS_Output_SHIFT                 12

/* DECODE_MAIN_0 :: REG_STATUS :: Dblk [11:10] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Dblk_MASK                    0x00000c00
#define BCHP_DECODE_MAIN_0_REG_STATUS_Dblk_SHIFT                   10

/* DECODE_MAIN_0 :: REG_STATUS :: Recon [09:08] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_Recon_MASK                   0x00000300
#define BCHP_DECODE_MAIN_0_REG_STATUS_Recon_SHIFT                  8

/* DECODE_MAIN_0 :: REG_STATUS :: reserved1 [07:00] */
#define BCHP_DECODE_MAIN_0_REG_STATUS_reserved1_MASK               0x000000ff
#define BCHP_DECODE_MAIN_0_REG_STATUS_reserved1_SHIFT              0

/***************************************************************************
 *REG_PMONCTL - Performance Monitoring
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_PMONCTL :: reserved0 [31:12] */
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_reserved0_MASK              0xfffff000
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_reserved0_SHIFT             12

/* DECODE_MAIN_0 :: REG_PMONCTL :: CNT1_SEL [11:08] */
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_CNT1_SEL_MASK               0x00000f00
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_CNT1_SEL_SHIFT              8

/* DECODE_MAIN_0 :: REG_PMONCTL :: reserved1 [07:04] */
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_reserved1_MASK              0x000000f0
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_reserved1_SHIFT             4

/* DECODE_MAIN_0 :: REG_PMONCTL :: CNT0_SEL [03:00] */
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_CNT0_SEL_MASK               0x0000000f
#define BCHP_DECODE_MAIN_0_REG_PMONCTL_CNT0_SEL_SHIFT              0

/***************************************************************************
 *REG_PMONCNT0 - REG_PMONCNT0
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_PMONCNT0 :: DATA [31:16] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_DATA_MASK                  0xffff0000
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_DATA_SHIFT                 16

/* DECODE_MAIN_0 :: REG_PMONCNT0 :: reserved0 [15:12] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_reserved0_MASK             0x0000f000
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_reserved0_SHIFT            12

/* DECODE_MAIN_0 :: REG_PMONCNT0 :: COUNT [11:00] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_COUNT_MASK                 0x00000fff
#define BCHP_DECODE_MAIN_0_REG_PMONCNT0_COUNT_SHIFT                0

/***************************************************************************
 *REG_PMONCNT1 - REG_PMONCNT1
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_PMONCNT1 :: DATA [31:16] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_DATA_MASK                  0xffff0000
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_DATA_SHIFT                 16

/* DECODE_MAIN_0 :: REG_PMONCNT1 :: reserved0 [15:12] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_reserved0_MASK             0x0000f000
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_reserved0_SHIFT            12

/* DECODE_MAIN_0 :: REG_PMONCNT1 :: COUNT [11:00] */
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_COUNT_MASK                 0x00000fff
#define BCHP_DECODE_MAIN_0_REG_PMONCNT1_COUNT_SHIFT                0

/***************************************************************************
 *REG_PMON_MBCTL - REG_PMON_MBCTL
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_PMON_MBCTL :: reserved0 [31:02] */
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_reserved0_MASK           0xfffffffc
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_reserved0_SHIFT          2

/* DECODE_MAIN_0 :: REG_PMON_MBCTL :: SW_Pmon [01:01] */
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_SW_Pmon_MASK             0x00000002
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_SW_Pmon_SHIFT            1

/* DECODE_MAIN_0 :: REG_PMON_MBCTL :: MBCtlEna [00:00] */
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_MBCtlEna_MASK            0x00000001
#define BCHP_DECODE_MAIN_0_REG_PMON_MBCTL_MBCtlEna_SHIFT           0

/***************************************************************************
 *DBLK_BUFF_CONTROL - DBLK_BUFF_CONTROL
 ***************************************************************************/
/* DECODE_MAIN_0 :: DBLK_BUFF_CONTROL :: reserved0 [31:02] */
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_reserved0_MASK        0xfffffffc
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_reserved0_SHIFT       2

/* DECODE_MAIN_0 :: DBLK_BUFF_CONTROL :: Disable_Block32 [01:01] */
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Disable_Block32_MASK  0x00000002
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Disable_Block32_SHIFT 1
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Disable_Block32_DEFAULT 0x00000000

/* DECODE_MAIN_0 :: DBLK_BUFF_CONTROL :: Enable [00:00] */
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Enable_MASK           0x00000001
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Enable_SHIFT          0
#define BCHP_DECODE_MAIN_0_DBLK_BUFF_CONTROL_Enable_DEFAULT        0x00000001

/***************************************************************************
 *CRC_CONTROL - DBLK CRC CONTROL register
 ***************************************************************************/
/* DECODE_MAIN_0 :: CRC_CONTROL :: reserved0 [31:03] */
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_reserved0_MASK              0xfffffff8
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_reserved0_SHIFT             3

/* DECODE_MAIN_0 :: CRC_CONTROL :: CRC_DONE [02:02] */
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_DONE_MASK               0x00000004
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_DONE_SHIFT              2
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_DONE_DEFAULT            0x00000000

/* DECODE_MAIN_0 :: CRC_CONTROL :: CRC_ENABLE [01:01] */
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_ENABLE_MASK             0x00000002
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_ENABLE_SHIFT            1
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_ENABLE_DEFAULT          0x00000000

/* DECODE_MAIN_0 :: CRC_CONTROL :: CRC_CHKSUM_MODE [00:00] */
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_CHKSUM_MODE_MASK        0x00000001
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_CHKSUM_MODE_SHIFT       0
#define BCHP_DECODE_MAIN_0_CRC_CONTROL_CRC_CHKSUM_MODE_DEFAULT     0x00000000

/***************************************************************************
 *CRC_SEED - DBLK CRC SEED register
 ***************************************************************************/
/* DECODE_MAIN_0 :: CRC_SEED :: VALUE [31:00] */
#define BCHP_DECODE_MAIN_0_CRC_SEED_VALUE_MASK                     0xffffffff
#define BCHP_DECODE_MAIN_0_CRC_SEED_VALUE_SHIFT                    0
#define BCHP_DECODE_MAIN_0_CRC_SEED_VALUE_DEFAULT                  0xffffffff

/***************************************************************************
 *CRC_CHKSUM_Y - DBLK Luma CRC/Checksum result register
 ***************************************************************************/
/* DECODE_MAIN_0 :: CRC_CHKSUM_Y :: VALUE [31:00] */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_Y_VALUE_MASK                 0xffffffff
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_Y_VALUE_SHIFT                0
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_Y_VALUE_DEFAULT              0x00000000

/***************************************************************************
 *CRC_CHKSUM_CB - DBLK Chroma (Cb) CRC/Checksum result register
 ***************************************************************************/
/* DECODE_MAIN_0 :: CRC_CHKSUM_CB :: VALUE [31:00] */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CB_VALUE_MASK                0xffffffff
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CB_VALUE_SHIFT               0
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CB_VALUE_DEFAULT             0x00000000

/***************************************************************************
 *CRC_CHKSUM_CR - DBLK Chroma (Cr) CRC/Checksum result register
 ***************************************************************************/
/* DECODE_MAIN_0 :: CRC_CHKSUM_CR :: VALUE [31:00] */
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CR_VALUE_MASK                0xffffffff
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CR_VALUE_SHIFT               0
#define BCHP_DECODE_MAIN_0_CRC_CHKSUM_CR_VALUE_DEFAULT             0x00000000

/***************************************************************************
 *REG_VP6_MCOM_CONTROL - VP6 Mocomp Control
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_VP6_MCOM_CONTROL :: reserved0 [31:03] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_reserved0_MASK     0xfffffff8
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_reserved0_SHIFT    3

/* DECODE_MAIN_0 :: REG_VP6_MCOM_CONTROL :: tap2_or_tap4 [02:02] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_tap2_or_tap4_MASK  0x00000004
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_tap2_or_tap4_SHIFT 2

/* DECODE_MAIN_0 :: REG_VP6_MCOM_CONTROL :: auto_select [01:01] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_auto_select_MASK   0x00000002
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_auto_select_SHIFT  1

/* DECODE_MAIN_0 :: REG_VP6_MCOM_CONTROL :: use_loop_filter [00:00] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_use_loop_filter_MASK 0x00000001
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_CONTROL_use_loop_filter_SHIFT 0

/***************************************************************************
 *REG_VP6_MCOM_AUTO - VP6 Mocomp Auto Filter Selection
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_VP6_MCOM_AUTO :: variance_threshold [31:16] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_AUTO_variance_threshold_MASK 0xffff0000
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_AUTO_variance_threshold_SHIFT 16

/* DECODE_MAIN_0 :: REG_VP6_MCOM_AUTO :: mv_threshold [15:00] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_AUTO_mv_threshold_MASK     0x0000ffff
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_AUTO_mv_threshold_SHIFT    0

/***************************************************************************
 *REG_VP6_MCOM_ALPHA - VP6 Mocomp Alpha Filter Selection
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_VP6_MCOM_ALPHA :: reserved0 [31:05] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_ALPHA_reserved0_MASK       0xffffffe0
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_ALPHA_reserved0_SHIFT      5

/* DECODE_MAIN_0 :: REG_VP6_MCOM_ALPHA :: filter_alpha [04:00] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_ALPHA_filter_alpha_MASK    0x0000001f
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_ALPHA_filter_alpha_SHIFT   0

/***************************************************************************
 *REG_VP6_MCOM_FLIMIT - VP6 Mocomp Flimit Control
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_VP6_MCOM_FLIMIT :: reserved0 [31:05] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_FLIMIT_reserved0_MASK      0xffffffe0
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_FLIMIT_reserved0_SHIFT     5

/* DECODE_MAIN_0 :: REG_VP6_MCOM_FLIMIT :: loop_filter_flimit [04:00] */
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_FLIMIT_loop_filter_flimit_MASK 0x0000001f
#define BCHP_DECODE_MAIN_0_REG_VP6_MCOM_FLIMIT_loop_filter_flimit_SHIFT 0

/***************************************************************************
 *REG_BACKEND_DEBUG - Backend debug Select
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_BACKEND_DEBUG :: reserved0 [31:04] */
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_reserved0_MASK        0xfffffff0
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_reserved0_SHIFT       4

/* DECODE_MAIN_0 :: REG_BACKEND_DEBUG :: debug_datapath_sel [03:01] */
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_debug_datapath_sel_MASK 0x0000000e
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_debug_datapath_sel_SHIFT 1

/* DECODE_MAIN_0 :: REG_BACKEND_DEBUG :: debug_mode [00:00] */
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_debug_mode_MASK       0x00000001
#define BCHP_DECODE_MAIN_0_REG_BACKEND_DEBUG_debug_mode_SHIFT      0

/***************************************************************************
 *REG_VC1_MC_DEBUG - VC1 Mocomp Debug
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: reserved0 [31:27] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_reserved0_MASK         0xf8000000
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_reserved0_SHIFT        27

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: fetch_size [26:20] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_size_MASK        0x07f00000
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_size_SHIFT       20

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: fetch_fifo_req [19:19] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_req_MASK    0x00080000
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_req_SHIFT   19

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: fetch_fifo_level_ok [18:18] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_level_ok_MASK 0x00040000
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_level_ok_SHIFT 18

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: fetch_fifo_depth [17:08] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_depth_MASK  0x0003ff00
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_fifo_depth_SHIFT 8

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: filter_sm [07:04] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_filter_sm_MASK         0x000000f0
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_filter_sm_SHIFT        4

/* DECODE_MAIN_0 :: REG_VC1_MC_DEBUG :: fetch_sm [03:00] */
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_sm_MASK          0x0000000f
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_sm_SHIFT         0
#define BCHP_DECODE_MAIN_0_REG_VC1_MC_DEBUG_fetch_sm_DEFAULT       0x00000000

/***************************************************************************
 *REG_QPEL_FIFO_DEBUG - Qpel FIFO Debug
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_QPEL_FIFO_DEBUG :: reserved0 [31:10] */
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG_reserved0_MASK      0xfffffc00
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG_reserved0_SHIFT     10

/* DECODE_MAIN_0 :: REG_QPEL_FIFO_DEBUG :: qpel_fifo_depth [09:00] */
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG_qpel_fifo_depth_MASK 0x000003ff
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG_qpel_fifo_depth_SHIFT 0
#define BCHP_DECODE_MAIN_0_REG_QPEL_FIFO_DEBUG_qpel_fifo_depth_DEFAULT 0x00000000

/***************************************************************************
 *REG_MAIN_END - REG_MAIN_END
 ***************************************************************************/
/* DECODE_MAIN_0 :: REG_MAIN_END :: reserved0 [31:00] */
#define BCHP_DECODE_MAIN_0_REG_MAIN_END_reserved0_MASK             0xffffffff
#define BCHP_DECODE_MAIN_0_REG_MAIN_END_reserved0_SHIFT            0

#endif /* #ifndef BCHP_DECODE_MAIN_0_H__ */

/* End of File */
