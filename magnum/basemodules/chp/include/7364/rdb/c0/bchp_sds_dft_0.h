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
 * Date:           Generated on               Mon Feb  8 12:53:16 2016
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

#ifndef BCHP_SDS_DFT_0_H__
#define BCHP_SDS_DFT_0_H__

/***************************************************************************
 *SDS_DFT_0 - SDS DFT Register Set
 ***************************************************************************/
#define BCHP_SDS_DFT_0_CTRL0                     0x01240580 /* [RW] DFT Control 0 */
#define BCHP_SDS_DFT_0_CTRL1                     0x01240584 /* [RW] DFT Control 1 */
#define BCHP_SDS_DFT_0_STATUS                    0x01240588 /* [RO] DFT Status */
#define BCHP_SDS_DFT_0_RANGE_START               0x0124058c /* [RW] DFT Starting Bin */
#define BCHP_SDS_DFT_0_RANGE_END                 0x01240590 /* [RW] DFT Ending Bin */
#define BCHP_SDS_DFT_0_DDFS_FCW                  0x01240594 /* [RW] DFT DDFS Frequency Control Word */
#define BCHP_SDS_DFT_0_PEAK_POW                  0x01240598 /* [RO] DFT Peak Power */
#define BCHP_SDS_DFT_0_PEAK_BIN                  0x0124059c /* [RO] DFT Peak Bin */
#define BCHP_SDS_DFT_0_TOTAL_POW                 0x012405a0 /* [RO] DFT Total Power */
#define BCHP_SDS_DFT_0_MEM_RADDR                 0x012405a4 /* [RW] DFT Bin Memory Read Address */
#define BCHP_SDS_DFT_0_MEM_RDATA                 0x012405a8 /* [RO] DFT Bin Memory Read Data */

/***************************************************************************
 *CTRL0 - DFT Control 0
 ***************************************************************************/
/* SDS_DFT_0 :: CTRL0 :: reserved0 [31:02] */
#define BCHP_SDS_DFT_0_CTRL0_reserved0_MASK                        0xfffffffc
#define BCHP_SDS_DFT_0_CTRL0_reserved0_SHIFT                       2

/* SDS_DFT_0 :: CTRL0 :: dft_srst [01:01] */
#define BCHP_SDS_DFT_0_CTRL0_dft_srst_MASK                         0x00000002
#define BCHP_SDS_DFT_0_CTRL0_dft_srst_SHIFT                        1
#define BCHP_SDS_DFT_0_CTRL0_dft_srst_DEFAULT                      0x00000000

/* SDS_DFT_0 :: CTRL0 :: dft_start [00:00] */
#define BCHP_SDS_DFT_0_CTRL0_dft_start_MASK                        0x00000001
#define BCHP_SDS_DFT_0_CTRL0_dft_start_SHIFT                       0
#define BCHP_SDS_DFT_0_CTRL0_dft_start_DEFAULT                     0x00000000

/***************************************************************************
 *CTRL1 - DFT Control 1
 ***************************************************************************/
/* SDS_DFT_0 :: CTRL1 :: reserved0 [31:12] */
#define BCHP_SDS_DFT_0_CTRL1_reserved0_MASK                        0xfffff000
#define BCHP_SDS_DFT_0_CTRL1_reserved0_SHIFT                       12

/* SDS_DFT_0 :: CTRL1 :: dft_mod_sel [11:10] */
#define BCHP_SDS_DFT_0_CTRL1_dft_mod_sel_MASK                      0x00000c00
#define BCHP_SDS_DFT_0_CTRL1_dft_mod_sel_SHIFT                     10
#define BCHP_SDS_DFT_0_CTRL1_dft_mod_sel_DEFAULT                   0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_front_insel [09:09] */
#define BCHP_SDS_DFT_0_CTRL1_dft_front_insel_MASK                  0x00000200
#define BCHP_SDS_DFT_0_CTRL1_dft_front_insel_SHIFT                 9
#define BCHP_SDS_DFT_0_CTRL1_dft_front_insel_DEFAULT               0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_pwr_fmt [08:08] */
#define BCHP_SDS_DFT_0_CTRL1_dft_pwr_fmt_MASK                      0x00000100
#define BCHP_SDS_DFT_0_CTRL1_dft_pwr_fmt_SHIFT                     8
#define BCHP_SDS_DFT_0_CTRL1_dft_pwr_fmt_DEFAULT                   0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_scale [07:05] */
#define BCHP_SDS_DFT_0_CTRL1_dft_scale_MASK                        0x000000e0
#define BCHP_SDS_DFT_0_CTRL1_dft_scale_SHIFT                       5
#define BCHP_SDS_DFT_0_CTRL1_dft_scale_DEFAULT                     0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_range_sel [04:04] */
#define BCHP_SDS_DFT_0_CTRL1_dft_range_sel_MASK                    0x00000010
#define BCHP_SDS_DFT_0_CTRL1_dft_range_sel_SHIFT                   4
#define BCHP_SDS_DFT_0_CTRL1_dft_range_sel_DEFAULT                 0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_in_sel [03:03] */
#define BCHP_SDS_DFT_0_CTRL1_dft_in_sel_MASK                       0x00000008
#define BCHP_SDS_DFT_0_CTRL1_dft_in_sel_SHIFT                      3
#define BCHP_SDS_DFT_0_CTRL1_dft_in_sel_DEFAULT                    0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_mode [02:02] */
#define BCHP_SDS_DFT_0_CTRL1_dft_mode_MASK                         0x00000004
#define BCHP_SDS_DFT_0_CTRL1_dft_mode_SHIFT                        2
#define BCHP_SDS_DFT_0_CTRL1_dft_mode_DEFAULT                      0x00000000

/* SDS_DFT_0 :: CTRL1 :: dft_size [01:00] */
#define BCHP_SDS_DFT_0_CTRL1_dft_size_MASK                         0x00000003
#define BCHP_SDS_DFT_0_CTRL1_dft_size_SHIFT                        0
#define BCHP_SDS_DFT_0_CTRL1_dft_size_DEFAULT                      0x00000000

/***************************************************************************
 *STATUS - DFT Status
 ***************************************************************************/
/* SDS_DFT_0 :: STATUS :: reserved0 [31:03] */
#define BCHP_SDS_DFT_0_STATUS_reserved0_MASK                       0xfffffff8
#define BCHP_SDS_DFT_0_STATUS_reserved0_SHIFT                      3

/* SDS_DFT_0 :: STATUS :: dft_done [02:02] */
#define BCHP_SDS_DFT_0_STATUS_dft_done_MASK                        0x00000004
#define BCHP_SDS_DFT_0_STATUS_dft_done_SHIFT                       2
#define BCHP_SDS_DFT_0_STATUS_dft_done_DEFAULT                     0x00000000

/* SDS_DFT_0 :: STATUS :: dft_done_iter [01:01] */
#define BCHP_SDS_DFT_0_STATUS_dft_done_iter_MASK                   0x00000002
#define BCHP_SDS_DFT_0_STATUS_dft_done_iter_SHIFT                  1
#define BCHP_SDS_DFT_0_STATUS_dft_done_iter_DEFAULT                0x00000000

/* SDS_DFT_0 :: STATUS :: dft_busy [00:00] */
#define BCHP_SDS_DFT_0_STATUS_dft_busy_MASK                        0x00000001
#define BCHP_SDS_DFT_0_STATUS_dft_busy_SHIFT                       0
#define BCHP_SDS_DFT_0_STATUS_dft_busy_DEFAULT                     0x00000000

/***************************************************************************
 *RANGE_START - DFT Starting Bin
 ***************************************************************************/
/* SDS_DFT_0 :: RANGE_START :: reserved0 [31:11] */
#define BCHP_SDS_DFT_0_RANGE_START_reserved0_MASK                  0xfffff800
#define BCHP_SDS_DFT_0_RANGE_START_reserved0_SHIFT                 11

/* SDS_DFT_0 :: RANGE_START :: dft_range_start [10:00] */
#define BCHP_SDS_DFT_0_RANGE_START_dft_range_start_MASK            0x000007ff
#define BCHP_SDS_DFT_0_RANGE_START_dft_range_start_SHIFT           0
#define BCHP_SDS_DFT_0_RANGE_START_dft_range_start_DEFAULT         0x00000000

/***************************************************************************
 *RANGE_END - DFT Ending Bin
 ***************************************************************************/
/* SDS_DFT_0 :: RANGE_END :: reserved0 [31:11] */
#define BCHP_SDS_DFT_0_RANGE_END_reserved0_MASK                    0xfffff800
#define BCHP_SDS_DFT_0_RANGE_END_reserved0_SHIFT                   11

/* SDS_DFT_0 :: RANGE_END :: dft_range_end [10:00] */
#define BCHP_SDS_DFT_0_RANGE_END_dft_range_end_MASK                0x000007ff
#define BCHP_SDS_DFT_0_RANGE_END_dft_range_end_SHIFT               0
#define BCHP_SDS_DFT_0_RANGE_END_dft_range_end_DEFAULT             0x00000000

/***************************************************************************
 *DDFS_FCW - DFT DDFS Frequency Control Word
 ***************************************************************************/
/* SDS_DFT_0 :: DDFS_FCW :: reserved0 [31:16] */
#define BCHP_SDS_DFT_0_DDFS_FCW_reserved0_MASK                     0xffff0000
#define BCHP_SDS_DFT_0_DDFS_FCW_reserved0_SHIFT                    16

/* SDS_DFT_0 :: DDFS_FCW :: dft_ddfs_fcw [15:00] */
#define BCHP_SDS_DFT_0_DDFS_FCW_dft_ddfs_fcw_MASK                  0x0000ffff
#define BCHP_SDS_DFT_0_DDFS_FCW_dft_ddfs_fcw_SHIFT                 0
#define BCHP_SDS_DFT_0_DDFS_FCW_dft_ddfs_fcw_DEFAULT               0x00000000

/***************************************************************************
 *PEAK_POW - DFT Peak Power
 ***************************************************************************/
/* SDS_DFT_0 :: PEAK_POW :: dft_peak_pow [31:00] */
#define BCHP_SDS_DFT_0_PEAK_POW_dft_peak_pow_MASK                  0xffffffff
#define BCHP_SDS_DFT_0_PEAK_POW_dft_peak_pow_SHIFT                 0
#define BCHP_SDS_DFT_0_PEAK_POW_dft_peak_pow_DEFAULT               0x00000000

/***************************************************************************
 *PEAK_BIN - DFT Peak Bin
 ***************************************************************************/
/* SDS_DFT_0 :: PEAK_BIN :: reserved0 [31:12] */
#define BCHP_SDS_DFT_0_PEAK_BIN_reserved0_MASK                     0xfffff000
#define BCHP_SDS_DFT_0_PEAK_BIN_reserved0_SHIFT                    12

/* SDS_DFT_0 :: PEAK_BIN :: dft_peak_bin [11:00] */
#define BCHP_SDS_DFT_0_PEAK_BIN_dft_peak_bin_MASK                  0x00000fff
#define BCHP_SDS_DFT_0_PEAK_BIN_dft_peak_bin_SHIFT                 0
#define BCHP_SDS_DFT_0_PEAK_BIN_dft_peak_bin_DEFAULT               0x00000000

/***************************************************************************
 *TOTAL_POW - DFT Total Power
 ***************************************************************************/
/* SDS_DFT_0 :: TOTAL_POW :: dft_total_pow [31:00] */
#define BCHP_SDS_DFT_0_TOTAL_POW_dft_total_pow_MASK                0xffffffff
#define BCHP_SDS_DFT_0_TOTAL_POW_dft_total_pow_SHIFT               0
#define BCHP_SDS_DFT_0_TOTAL_POW_dft_total_pow_DEFAULT             0x00000000

/***************************************************************************
 *MEM_RADDR - DFT Bin Memory Read Address
 ***************************************************************************/
/* SDS_DFT_0 :: MEM_RADDR :: reserved0 [31:06] */
#define BCHP_SDS_DFT_0_MEM_RADDR_reserved0_MASK                    0xffffffc0
#define BCHP_SDS_DFT_0_MEM_RADDR_reserved0_SHIFT                   6

/* SDS_DFT_0 :: MEM_RADDR :: dft_mem_raddr [05:00] */
#define BCHP_SDS_DFT_0_MEM_RADDR_dft_mem_raddr_MASK                0x0000003f
#define BCHP_SDS_DFT_0_MEM_RADDR_dft_mem_raddr_SHIFT               0
#define BCHP_SDS_DFT_0_MEM_RADDR_dft_mem_raddr_DEFAULT             0x00000000

/***************************************************************************
 *MEM_RDATA - DFT Bin Memory Read Data
 ***************************************************************************/
/* SDS_DFT_0 :: MEM_RDATA :: dft_mem_rdata [31:00] */
#define BCHP_SDS_DFT_0_MEM_RDATA_dft_mem_rdata_MASK                0xffffffff
#define BCHP_SDS_DFT_0_MEM_RDATA_dft_mem_rdata_SHIFT               0
#define BCHP_SDS_DFT_0_MEM_RDATA_dft_mem_rdata_DEFAULT             0x00000000

#endif /* #ifndef BCHP_SDS_DFT_0_H__ */

/* End of File */
