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
 * Date:           Generated on               Fri Aug 21 14:43:23 2015
 *                 Full Compile MD5 Checksum  6f40c93fa7adf1b7b596c84d59590a10
 *                     (minus title and desc)
 *                 MD5 Checksum               b1b8c76af39c441b8e9ab1ae2930543d
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     88
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_WDMA_PM_RESULTS_H__
#define BCHP_XPT_WDMA_PM_RESULTS_H__

/***************************************************************************
 *XPT_WDMA_PM_RESULTS - PerfMeter Status Registers
 ***************************************************************************/
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_MOVED 0x20a68600 /* [RO] Perfmeter: Number of accepted input data words. */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_READY 0x20a68604 /* [RO] Perfmeter: Number of cycles with READY=0 at the input */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_ACCEPTED 0x20a68608 /* [RO] Perfmeter: Number of cycles with READY=1, ACCEPT=0 at the input */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_HEADER_MOVED 0x20a6860c /* [RO] Perfmeter: Total number of X-header words accepted from XPT Security */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_PAYLD_MOVED 0x20a68610 /* [RO] Perfmeter: Total number of X-packet payload words accepted from XPT Security */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_INPUT_PACKETS_TOTAL 0x20a68614 /* [RO] Perfmeter: Total number of packets accepted from XPT Security */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_MOVED 0x20a68618 /* [RO] Perfmeter: Number of accepted output data words */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_READY 0x20a6861c /* [RO] Perfmeter: Number of cycles with READY=0 at the output */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_ACCEPTED 0x20a68620 /* [RO] Perfmeter: Number of cycles with READY=1, ACCEPT=0 at the output */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_HEADER_MOVED 0x20a68624 /* [RO] Perfmeter: Total number of X-header words sent to XMEMIF */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_PAYLD_MOVED 0x20a68628 /* [RO] Perfmeter: Total number of X-packet payload sent to XMEMIF */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_OUTPUT_PACKETS_TOTAL 0x20a6862c /* [RO] Perfmeter: Total number of packets sent to XMEMIF Security */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_DESCRIPTORS 0x20a68630 /* [RO] Perfmeter: Number of Descriptors read */
#define BCHP_XPT_WDMA_PM_RESULTS_DESCR_READS     0x20a68634 /* [RO] Perfmeter: Number of Descriptor read Transactions */
#define BCHP_XPT_WDMA_PM_RESULTS_DATA_STALL_CLOCKS 0x20a68638 /* [RO] Perfmeter: Number of clocks when the data transfer was stalled */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT     0x20a6863c /* [RW] Perfmeter: Specify the channel number for per_channel statistics. */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_BYTES_TXFERED 0x20a68640 /* [RO] Perfmeter: Number of bytes transferred by the channel */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_DATA_STALL_CLOCKS 0x20a68644 /* [RO] Perfmeter: Number of clocks when the data transfer was stalled for the channel */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_ACCEPTED 0x20a68648 /* [RO] Perfmeter: Number of Packets accepted by the channel */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_OVERFLOW 0x20a6864c /* [RO] Perfmeter: Number of Packets dropped by the channel due to Overflow */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_RUN_VERSION 0x20a68650 /* [RO] Perfmeter: Number of Packets dropped by the channel due to RUN_VERSION mismatch */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_READING_DESCRIPTORS 0x20a68654 /* [RO] Perfmeter: Number of clocks spent by the channel reading descriptors */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SLEEP      0x20a68658 /* [RO] Perfmeter: Number of clocks spent by the channel in SLEEP state */

/***************************************************************************
 *INPUT_DATA_MOVED - Perfmeter: Number of accepted input data words.
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: INPUT_DATA_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_MOVED_NUM_WORDS_MASK   0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_MOVED_NUM_WORDS_SHIFT  0

/***************************************************************************
 *INPUT_DATA_NOT_READY - Perfmeter: Number of cycles with READY=0 at the input
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: INPUT_DATA_NOT_READY :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_READY_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_READY_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *INPUT_DATA_NOT_ACCEPTED - Perfmeter: Number of cycles with READY=1, ACCEPT=0 at the input
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: INPUT_DATA_NOT_ACCEPTED :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_ACCEPTED_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_ACCEPTED_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *INPUT_HEADER_MOVED - Perfmeter: Total number of X-header words accepted from XPT Security
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: INPUT_HEADER_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_HEADER_MOVED_NUM_WORDS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_HEADER_MOVED_NUM_WORDS_SHIFT 0

/***************************************************************************
 *INPUT_PAYLD_MOVED - Perfmeter: Total number of X-packet payload words accepted from XPT Security
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: INPUT_PAYLD_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_PAYLD_MOVED_NUM_WORDS_MASK  0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_INPUT_PAYLD_MOVED_NUM_WORDS_SHIFT 0

/***************************************************************************
 *NUM_INPUT_PACKETS_TOTAL - Perfmeter: Total number of packets accepted from XPT Security
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: NUM_INPUT_PACKETS_TOTAL :: NUM_PACKETS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_INPUT_PACKETS_TOTAL_NUM_PACKETS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_INPUT_PACKETS_TOTAL_NUM_PACKETS_SHIFT 0

/***************************************************************************
 *OUTPUT_DATA_MOVED - Perfmeter: Number of accepted output data words
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: OUTPUT_DATA_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_MOVED_NUM_WORDS_MASK  0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_MOVED_NUM_WORDS_SHIFT 0

/***************************************************************************
 *OUTPUT_DATA_NOT_READY - Perfmeter: Number of cycles with READY=0 at the output
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: OUTPUT_DATA_NOT_READY :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_READY_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_READY_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *OUTPUT_DATA_NOT_ACCEPTED - Perfmeter: Number of cycles with READY=1, ACCEPT=0 at the output
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: OUTPUT_DATA_NOT_ACCEPTED :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_ACCEPTED_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_ACCEPTED_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *OUTPUT_HEADER_MOVED - Perfmeter: Total number of X-header words sent to XMEMIF
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: OUTPUT_HEADER_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_HEADER_MOVED_NUM_WORDS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_HEADER_MOVED_NUM_WORDS_SHIFT 0

/***************************************************************************
 *OUTPUT_PAYLD_MOVED - Perfmeter: Total number of X-packet payload sent to XMEMIF
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: OUTPUT_PAYLD_MOVED :: NUM_WORDS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_PAYLD_MOVED_NUM_WORDS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_PAYLD_MOVED_NUM_WORDS_SHIFT 0

/***************************************************************************
 *NUM_OUTPUT_PACKETS_TOTAL - Perfmeter: Total number of packets sent to XMEMIF Security
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: NUM_OUTPUT_PACKETS_TOTAL :: NUM_PACKETS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_OUTPUT_PACKETS_TOTAL_NUM_PACKETS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_OUTPUT_PACKETS_TOTAL_NUM_PACKETS_SHIFT 0

/***************************************************************************
 *NUM_DESCRIPTORS - Perfmeter: Number of Descriptors read
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: NUM_DESCRIPTORS :: NUM_DESCRIPTORS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_DESCRIPTORS_NUM_DESCRIPTORS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_NUM_DESCRIPTORS_NUM_DESCRIPTORS_SHIFT 0

/***************************************************************************
 *DESCR_READS - Perfmeter: Number of Descriptor read Transactions
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: DESCR_READS :: NUM_READS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_DESCR_READS_NUM_READS_MASK        0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_DESCR_READS_NUM_READS_SHIFT       0

/***************************************************************************
 *DATA_STALL_CLOCKS - Perfmeter: Number of clocks when the data transfer was stalled
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: DATA_STALL_CLOCKS :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_DATA_STALL_CLOCKS_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_DATA_STALL_CLOCKS_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *CHAN_SELECT - Perfmeter: Specify the channel number for per_channel statistics.
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_SELECT :: reserved0 [31:08] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT_reserved0_MASK        0xffffff00
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT_reserved0_SHIFT       8

/* XPT_WDMA_PM_RESULTS :: CHAN_SELECT :: CHAN_NUM [07:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT_CHAN_NUM_MASK         0x000000ff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT_CHAN_NUM_SHIFT        0
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SELECT_CHAN_NUM_DEFAULT      0x00000000

/***************************************************************************
 *CHAN_BYTES_TXFERED - Perfmeter: Number of bytes transferred by the channel
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_BYTES_TXFERED :: NUM_BYTES [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_BYTES_TXFERED_NUM_BYTES_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_BYTES_TXFERED_NUM_BYTES_SHIFT 0

/***************************************************************************
 *CHAN_DATA_STALL_CLOCKS - Perfmeter: Number of clocks when the data transfer was stalled for the channel
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_DATA_STALL_CLOCKS :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_DATA_STALL_CLOCKS_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_DATA_STALL_CLOCKS_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *CHAN_PACKETS_ACCEPTED - Perfmeter: Number of Packets accepted by the channel
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_PACKETS_ACCEPTED :: NUM_PACKETS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_ACCEPTED_NUM_PACKETS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_ACCEPTED_NUM_PACKETS_SHIFT 0

/***************************************************************************
 *CHAN_PACKETS_REJECTED_OVERFLOW - Perfmeter: Number of Packets dropped by the channel due to Overflow
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_PACKETS_REJECTED_OVERFLOW :: NUM_PACKETS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_OVERFLOW_NUM_PACKETS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_OVERFLOW_NUM_PACKETS_SHIFT 0

/***************************************************************************
 *CHAN_PACKETS_REJECTED_RUN_VERSION - Perfmeter: Number of Packets dropped by the channel due to RUN_VERSION mismatch
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_PACKETS_REJECTED_RUN_VERSION :: NUM_PACKETS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_RUN_VERSION_NUM_PACKETS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_PACKETS_REJECTED_RUN_VERSION_NUM_PACKETS_SHIFT 0

/***************************************************************************
 *CHAN_READING_DESCRIPTORS - Perfmeter: Number of clocks spent by the channel reading descriptors
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_READING_DESCRIPTORS :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_READING_DESCRIPTORS_NUM_CLOCKS_MASK 0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_READING_DESCRIPTORS_NUM_CLOCKS_SHIFT 0

/***************************************************************************
 *CHAN_SLEEP - Perfmeter: Number of clocks spent by the channel in SLEEP state
 ***************************************************************************/
/* XPT_WDMA_PM_RESULTS :: CHAN_SLEEP :: NUM_CLOCKS [31:00] */
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SLEEP_NUM_CLOCKS_MASK        0xffffffff
#define BCHP_XPT_WDMA_PM_RESULTS_CHAN_SLEEP_NUM_CLOCKS_SHIFT       0

#endif /* #ifndef BCHP_XPT_WDMA_PM_RESULTS_H__ */

/* End of File */
