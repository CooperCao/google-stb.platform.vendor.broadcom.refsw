/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:53:14 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_V3D_CLE_H__
#define BCHP_V3D_CLE_H__

/***************************************************************************
 *V3D_CLE - V3D Control List Executor Registers
 ***************************************************************************/
#define BCHP_V3D_CLE_CT0CS                       0x00bea100 /* [RW] Control Processor Thread 0 Control and Status */
#define BCHP_V3D_CLE_CT1CS                       0x00bea104 /* [RW] Control Processor Thread 1 Control and Status */
#define BCHP_V3D_CLE_CT0EA                       0x00bea108 /* [RW] Control Processor Thread 0 End Address */
#define BCHP_V3D_CLE_CT1EA                       0x00bea10c /* [RW] Control Processor Thread 1 End Address */
#define BCHP_V3D_CLE_CT0CA                       0x00bea110 /* [RW] Control Processor Thread 0 Current Address */
#define BCHP_V3D_CLE_CT1CA                       0x00bea114 /* [RW] Control Processor Thread 1 Current Address */
#define BCHP_V3D_CLE_CT00RA0                     0x00bea118 /* [RO] Control Processor Thread 0 Return Address 0 */
#define BCHP_V3D_CLE_CT01RA0                     0x00bea11c /* [RO] Control Processor Thread 1 Return Address 0 */
#define BCHP_V3D_CLE_CT0LC                       0x00bea120 /* [RW] Control Processor Thread 0 List Counter */
#define BCHP_V3D_CLE_CT1LC                       0x00bea124 /* [RW] Control Processor Thread 1 List Counter */
#define BCHP_V3D_CLE_CT0PC                       0x00bea128 /* [RW] Control Processor Thread 0 Primitive List Counter */
#define BCHP_V3D_CLE_CT1PC                       0x00bea12c /* [RW] Control Processor Thread 1 Primitive List Counter */
#define BCHP_V3D_CLE_PCS                         0x00bea130 /* [RW] V3D Pipeline Control and Status */
#define BCHP_V3D_CLE_BFC                         0x00bea134 /* [RW] Binning Mode Flush Count */
#define BCHP_V3D_CLE_RFC                         0x00bea138 /* [RW] Rendering Mode Flush Count */

/***************************************************************************
 *CT0CS - Control Processor Thread 0 Control and Status
 ***************************************************************************/
/* V3D_CLE :: CT0CS :: CTMCT [31:16] */
#define BCHP_V3D_CLE_CT0CS_CTMCT_MASK                              0xffff0000
#define BCHP_V3D_CLE_CT0CS_CTMCT_SHIFT                             16
#define BCHP_V3D_CLE_CT0CS_CTMCT_DEFAULT                           0x00000000

/* V3D_CLE :: CT0CS :: CTRSTA [15:15] */
#define BCHP_V3D_CLE_CT0CS_CTRSTA_MASK                             0x00008000
#define BCHP_V3D_CLE_CT0CS_CTRSTA_SHIFT                            15
#define BCHP_V3D_CLE_CT0CS_CTRSTA_DEFAULT                          0x00000000

/* V3D_CLE :: CT0CS :: reserved0 [14:09] */
#define BCHP_V3D_CLE_CT0CS_reserved0_MASK                          0x00007e00
#define BCHP_V3D_CLE_CT0CS_reserved0_SHIFT                         9

/* V3D_CLE :: CT0CS :: CTRTSD [08:08] */
#define BCHP_V3D_CLE_CT0CS_CTRTSD_MASK                             0x00000100
#define BCHP_V3D_CLE_CT0CS_CTRTSD_SHIFT                            8
#define BCHP_V3D_CLE_CT0CS_CTRTSD_DEFAULT                          0x00000000

/* V3D_CLE :: CT0CS :: reserved1 [07:06] */
#define BCHP_V3D_CLE_CT0CS_reserved1_MASK                          0x000000c0
#define BCHP_V3D_CLE_CT0CS_reserved1_SHIFT                         6

/* V3D_CLE :: CT0CS :: CTRUN [05:05] */
#define BCHP_V3D_CLE_CT0CS_CTRUN_MASK                              0x00000020
#define BCHP_V3D_CLE_CT0CS_CTRUN_SHIFT                             5
#define BCHP_V3D_CLE_CT0CS_CTRUN_DEFAULT                           0x00000000

/* V3D_CLE :: CT0CS :: CTSUBS [04:04] */
#define BCHP_V3D_CLE_CT0CS_CTSUBS_MASK                             0x00000010
#define BCHP_V3D_CLE_CT0CS_CTSUBS_SHIFT                            4
#define BCHP_V3D_CLE_CT0CS_CTSUBS_DEFAULT                          0x00000000

/* V3D_CLE :: CT0CS :: reserved2 [03:02] */
#define BCHP_V3D_CLE_CT0CS_reserved2_MASK                          0x0000000c
#define BCHP_V3D_CLE_CT0CS_reserved2_SHIFT                         2

/* V3D_CLE :: CT0CS :: CTMODE [01:00] */
#define BCHP_V3D_CLE_CT0CS_CTMODE_MASK                             0x00000003
#define BCHP_V3D_CLE_CT0CS_CTMODE_SHIFT                            0
#define BCHP_V3D_CLE_CT0CS_CTMODE_DEFAULT                          0x00000000

/***************************************************************************
 *CT1CS - Control Processor Thread 1 Control and Status
 ***************************************************************************/
/* V3D_CLE :: CT1CS :: CTMCT [31:16] */
#define BCHP_V3D_CLE_CT1CS_CTMCT_MASK                              0xffff0000
#define BCHP_V3D_CLE_CT1CS_CTMCT_SHIFT                             16
#define BCHP_V3D_CLE_CT1CS_CTMCT_DEFAULT                           0x00000000

/* V3D_CLE :: CT1CS :: CTRSTA [15:15] */
#define BCHP_V3D_CLE_CT1CS_CTRSTA_MASK                             0x00008000
#define BCHP_V3D_CLE_CT1CS_CTRSTA_SHIFT                            15
#define BCHP_V3D_CLE_CT1CS_CTRSTA_DEFAULT                          0x00000000

/* V3D_CLE :: CT1CS :: reserved0 [14:09] */
#define BCHP_V3D_CLE_CT1CS_reserved0_MASK                          0x00007e00
#define BCHP_V3D_CLE_CT1CS_reserved0_SHIFT                         9

/* V3D_CLE :: CT1CS :: CTRTSD [08:08] */
#define BCHP_V3D_CLE_CT1CS_CTRTSD_MASK                             0x00000100
#define BCHP_V3D_CLE_CT1CS_CTRTSD_SHIFT                            8
#define BCHP_V3D_CLE_CT1CS_CTRTSD_DEFAULT                          0x00000000

/* V3D_CLE :: CT1CS :: reserved1 [07:06] */
#define BCHP_V3D_CLE_CT1CS_reserved1_MASK                          0x000000c0
#define BCHP_V3D_CLE_CT1CS_reserved1_SHIFT                         6

/* V3D_CLE :: CT1CS :: CTRUN [05:05] */
#define BCHP_V3D_CLE_CT1CS_CTRUN_MASK                              0x00000020
#define BCHP_V3D_CLE_CT1CS_CTRUN_SHIFT                             5
#define BCHP_V3D_CLE_CT1CS_CTRUN_DEFAULT                           0x00000000

/* V3D_CLE :: CT1CS :: CTSUBS [04:04] */
#define BCHP_V3D_CLE_CT1CS_CTSUBS_MASK                             0x00000010
#define BCHP_V3D_CLE_CT1CS_CTSUBS_SHIFT                            4
#define BCHP_V3D_CLE_CT1CS_CTSUBS_DEFAULT                          0x00000000

/* V3D_CLE :: CT1CS :: reserved2 [03:02] */
#define BCHP_V3D_CLE_CT1CS_reserved2_MASK                          0x0000000c
#define BCHP_V3D_CLE_CT1CS_reserved2_SHIFT                         2

/* V3D_CLE :: CT1CS :: CTMODE [01:00] */
#define BCHP_V3D_CLE_CT1CS_CTMODE_MASK                             0x00000003
#define BCHP_V3D_CLE_CT1CS_CTMODE_SHIFT                            0
#define BCHP_V3D_CLE_CT1CS_CTMODE_DEFAULT                          0x00000000

/***************************************************************************
 *CT0EA - Control Processor Thread 0 End Address
 ***************************************************************************/
/* V3D_CLE :: CT0EA :: CTLEA [31:00] */
#define BCHP_V3D_CLE_CT0EA_CTLEA_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT0EA_CTLEA_SHIFT                             0
#define BCHP_V3D_CLE_CT0EA_CTLEA_DEFAULT                           0x00000000

/***************************************************************************
 *CT1EA - Control Processor Thread 1 End Address
 ***************************************************************************/
/* V3D_CLE :: CT1EA :: CTLEA [31:00] */
#define BCHP_V3D_CLE_CT1EA_CTLEA_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT1EA_CTLEA_SHIFT                             0
#define BCHP_V3D_CLE_CT1EA_CTLEA_DEFAULT                           0x00000000

/***************************************************************************
 *CT0CA - Control Processor Thread 0 Current Address
 ***************************************************************************/
/* V3D_CLE :: CT0CA :: CTLCA [31:00] */
#define BCHP_V3D_CLE_CT0CA_CTLCA_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT0CA_CTLCA_SHIFT                             0
#define BCHP_V3D_CLE_CT0CA_CTLCA_DEFAULT                           0x00000000

/***************************************************************************
 *CT1CA - Control Processor Thread 1 Current Address
 ***************************************************************************/
/* V3D_CLE :: CT1CA :: CTLCA [31:00] */
#define BCHP_V3D_CLE_CT1CA_CTLCA_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT1CA_CTLCA_SHIFT                             0
#define BCHP_V3D_CLE_CT1CA_CTLCA_DEFAULT                           0x00000000

/***************************************************************************
 *CT00RA0 - Control Processor Thread 0 Return Address 0
 ***************************************************************************/
/* V3D_CLE :: CT00RA0 :: CTLRA [31:00] */
#define BCHP_V3D_CLE_CT00RA0_CTLRA_MASK                            0xffffffff
#define BCHP_V3D_CLE_CT00RA0_CTLRA_SHIFT                           0
#define BCHP_V3D_CLE_CT00RA0_CTLRA_DEFAULT                         0x00000000

/***************************************************************************
 *CT01RA0 - Control Processor Thread 1 Return Address 0
 ***************************************************************************/
/* V3D_CLE :: CT01RA0 :: CTLRA [31:00] */
#define BCHP_V3D_CLE_CT01RA0_CTLRA_MASK                            0xffffffff
#define BCHP_V3D_CLE_CT01RA0_CTLRA_SHIFT                           0
#define BCHP_V3D_CLE_CT01RA0_CTLRA_DEFAULT                         0x00000000

/***************************************************************************
 *CT0LC - Control Processor Thread 0 List Counter
 ***************************************************************************/
/* V3D_CLE :: CT0LC :: CTLLCM [31:16] */
#define BCHP_V3D_CLE_CT0LC_CTLLCM_MASK                             0xffff0000
#define BCHP_V3D_CLE_CT0LC_CTLLCM_SHIFT                            16
#define BCHP_V3D_CLE_CT0LC_CTLLCM_DEFAULT                          0x00000000

/* V3D_CLE :: CT0LC :: CTLSLCS [15:00] */
#define BCHP_V3D_CLE_CT0LC_CTLSLCS_MASK                            0x0000ffff
#define BCHP_V3D_CLE_CT0LC_CTLSLCS_SHIFT                           0
#define BCHP_V3D_CLE_CT0LC_CTLSLCS_DEFAULT                         0x00000000

/***************************************************************************
 *CT1LC - Control Processor Thread 1 List Counter
 ***************************************************************************/
/* V3D_CLE :: CT1LC :: CTLLCM [31:16] */
#define BCHP_V3D_CLE_CT1LC_CTLLCM_MASK                             0xffff0000
#define BCHP_V3D_CLE_CT1LC_CTLLCM_SHIFT                            16
#define BCHP_V3D_CLE_CT1LC_CTLLCM_DEFAULT                          0x00000000

/* V3D_CLE :: CT1LC :: CTLSLCS [15:00] */
#define BCHP_V3D_CLE_CT1LC_CTLSLCS_MASK                            0x0000ffff
#define BCHP_V3D_CLE_CT1LC_CTLSLCS_SHIFT                           0
#define BCHP_V3D_CLE_CT1LC_CTLSLCS_DEFAULT                         0x00000000

/***************************************************************************
 *CT0PC - Control Processor Thread 0 Primitive List Counter
 ***************************************************************************/
/* V3D_CLE :: CT0PC :: CTLPC [31:00] */
#define BCHP_V3D_CLE_CT0PC_CTLPC_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT0PC_CTLPC_SHIFT                             0
#define BCHP_V3D_CLE_CT0PC_CTLPC_DEFAULT                           0x00000000

/***************************************************************************
 *CT1PC - Control Processor Thread 1 Primitive List Counter
 ***************************************************************************/
/* V3D_CLE :: CT1PC :: CTLPC [31:00] */
#define BCHP_V3D_CLE_CT1PC_CTLPC_MASK                              0xffffffff
#define BCHP_V3D_CLE_CT1PC_CTLPC_SHIFT                             0
#define BCHP_V3D_CLE_CT1PC_CTLPC_DEFAULT                           0x00000000

/***************************************************************************
 *PCS - V3D Pipeline Control and Status
 ***************************************************************************/
/* V3D_CLE :: PCS :: reserved0 [31:09] */
#define BCHP_V3D_CLE_PCS_reserved0_MASK                            0xfffffe00
#define BCHP_V3D_CLE_PCS_reserved0_SHIFT                           9

/* V3D_CLE :: PCS :: BMOOM [08:08] */
#define BCHP_V3D_CLE_PCS_BMOOM_MASK                                0x00000100
#define BCHP_V3D_CLE_PCS_BMOOM_SHIFT                               8
#define BCHP_V3D_CLE_PCS_BMOOM_DEFAULT                             0x00000001

/* V3D_CLE :: PCS :: reserved1 [07:06] */
#define BCHP_V3D_CLE_PCS_reserved1_MASK                            0x000000c0
#define BCHP_V3D_CLE_PCS_reserved1_SHIFT                           6

/* V3D_CLE :: PCS :: RMRST [05:05] */
#define BCHP_V3D_CLE_PCS_RMRST_MASK                                0x00000020
#define BCHP_V3D_CLE_PCS_RMRST_SHIFT                               5
#define BCHP_V3D_CLE_PCS_RMRST_DEFAULT                             0x00000000

/* V3D_CLE :: PCS :: BMRST [04:04] */
#define BCHP_V3D_CLE_PCS_BMRST_MASK                                0x00000010
#define BCHP_V3D_CLE_PCS_BMRST_SHIFT                               4
#define BCHP_V3D_CLE_PCS_BMRST_DEFAULT                             0x00000000

/* V3D_CLE :: PCS :: RMBUSY [03:03] */
#define BCHP_V3D_CLE_PCS_RMBUSY_MASK                               0x00000008
#define BCHP_V3D_CLE_PCS_RMBUSY_SHIFT                              3
#define BCHP_V3D_CLE_PCS_RMBUSY_DEFAULT                            0x00000000

/* V3D_CLE :: PCS :: RMACTIVE [02:02] */
#define BCHP_V3D_CLE_PCS_RMACTIVE_MASK                             0x00000004
#define BCHP_V3D_CLE_PCS_RMACTIVE_SHIFT                            2
#define BCHP_V3D_CLE_PCS_RMACTIVE_DEFAULT                          0x00000000

/* V3D_CLE :: PCS :: BMBUSY [01:01] */
#define BCHP_V3D_CLE_PCS_BMBUSY_MASK                               0x00000002
#define BCHP_V3D_CLE_PCS_BMBUSY_SHIFT                              1
#define BCHP_V3D_CLE_PCS_BMBUSY_DEFAULT                            0x00000000

/* V3D_CLE :: PCS :: BMACTIVE [00:00] */
#define BCHP_V3D_CLE_PCS_BMACTIVE_MASK                             0x00000001
#define BCHP_V3D_CLE_PCS_BMACTIVE_SHIFT                            0
#define BCHP_V3D_CLE_PCS_BMACTIVE_DEFAULT                          0x00000000

/***************************************************************************
 *BFC - Binning Mode Flush Count
 ***************************************************************************/
/* V3D_CLE :: BFC :: reserved0 [31:08] */
#define BCHP_V3D_CLE_BFC_reserved0_MASK                            0xffffff00
#define BCHP_V3D_CLE_BFC_reserved0_SHIFT                           8

/* V3D_CLE :: BFC :: BMFCT [07:00] */
#define BCHP_V3D_CLE_BFC_BMFCT_MASK                                0x000000ff
#define BCHP_V3D_CLE_BFC_BMFCT_SHIFT                               0
#define BCHP_V3D_CLE_BFC_BMFCT_DEFAULT                             0x00000000

/***************************************************************************
 *RFC - Rendering Mode Flush Count
 ***************************************************************************/
/* V3D_CLE :: RFC :: reserved0 [31:08] */
#define BCHP_V3D_CLE_RFC_reserved0_MASK                            0xffffff00
#define BCHP_V3D_CLE_RFC_reserved0_SHIFT                           8

/* V3D_CLE :: RFC :: RMFCT [07:00] */
#define BCHP_V3D_CLE_RFC_RMFCT_MASK                                0x000000ff
#define BCHP_V3D_CLE_RFC_RMFCT_SHIFT                               0
#define BCHP_V3D_CLE_RFC_RMFCT_DEFAULT                             0x00000000

#endif /* #ifndef BCHP_V3D_CLE_H__ */

/* End of File */
