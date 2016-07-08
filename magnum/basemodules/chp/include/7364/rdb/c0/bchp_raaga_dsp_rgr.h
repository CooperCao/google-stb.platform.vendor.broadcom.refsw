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
 * Date:           Generated on               Mon Feb  8 12:53:13 2016
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

#ifndef BCHP_RAAGA_DSP_RGR_H__
#define BCHP_RAAGA_DSP_RGR_H__

/***************************************************************************
 *RAAGA_DSP_RGR - Raaga DSP RGR Bridge Registers
 ***************************************************************************/
#define BCHP_RAAGA_DSP_RGR_REVISION              0x00c00000 /* [RO] RGR Bridge Revision */
#define BCHP_RAAGA_DSP_RGR_CTRL                  0x00c00004 /* [RW] RGR Bridge Control Register */
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER            0x00c00008 /* [RW] RGR Bridge RBUS Timer Register */

/***************************************************************************
 *REVISION - RGR Bridge Revision
 ***************************************************************************/
/* RAAGA_DSP_RGR :: REVISION :: reserved0 [31:16] */
#define BCHP_RAAGA_DSP_RGR_REVISION_reserved0_MASK                 0xffff0000
#define BCHP_RAAGA_DSP_RGR_REVISION_reserved0_SHIFT                16

/* RAAGA_DSP_RGR :: REVISION :: MAJOR [15:08] */
#define BCHP_RAAGA_DSP_RGR_REVISION_MAJOR_MASK                     0x0000ff00
#define BCHP_RAAGA_DSP_RGR_REVISION_MAJOR_SHIFT                    8
#define BCHP_RAAGA_DSP_RGR_REVISION_MAJOR_DEFAULT                  0x00000002

/* RAAGA_DSP_RGR :: REVISION :: MINOR [07:00] */
#define BCHP_RAAGA_DSP_RGR_REVISION_MINOR_MASK                     0x000000ff
#define BCHP_RAAGA_DSP_RGR_REVISION_MINOR_SHIFT                    0
#define BCHP_RAAGA_DSP_RGR_REVISION_MINOR_DEFAULT                  0x00000000

/***************************************************************************
 *CTRL - RGR Bridge Control Register
 ***************************************************************************/
/* RAAGA_DSP_RGR :: CTRL :: reserved0 [31:02] */
#define BCHP_RAAGA_DSP_RGR_CTRL_reserved0_MASK                     0xfffffffc
#define BCHP_RAAGA_DSP_RGR_CTRL_reserved0_SHIFT                    2

/* RAAGA_DSP_RGR :: CTRL :: rbus_error_intr [01:01] */
#define BCHP_RAAGA_DSP_RGR_CTRL_rbus_error_intr_MASK               0x00000002
#define BCHP_RAAGA_DSP_RGR_CTRL_rbus_error_intr_SHIFT              1
#define BCHP_RAAGA_DSP_RGR_CTRL_rbus_error_intr_DEFAULT            0x00000000
#define BCHP_RAAGA_DSP_RGR_CTRL_rbus_error_intr_INTR_DISABLE       0
#define BCHP_RAAGA_DSP_RGR_CTRL_rbus_error_intr_INTR_ENABLE        1

/* RAAGA_DSP_RGR :: CTRL :: gisb_error_intr [00:00] */
#define BCHP_RAAGA_DSP_RGR_CTRL_gisb_error_intr_MASK               0x00000001
#define BCHP_RAAGA_DSP_RGR_CTRL_gisb_error_intr_SHIFT              0
#define BCHP_RAAGA_DSP_RGR_CTRL_gisb_error_intr_DEFAULT            0x00000000
#define BCHP_RAAGA_DSP_RGR_CTRL_gisb_error_intr_INTR_DISABLE       0
#define BCHP_RAAGA_DSP_RGR_CTRL_gisb_error_intr_INTR_ENABLE        1

/***************************************************************************
 *RBUS_TIMER - RGR Bridge RBUS Timer Register
 ***************************************************************************/
/* RAAGA_DSP_RGR :: RBUS_TIMER :: reserved0 [31:16] */
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER_reserved0_MASK               0xffff0000
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER_reserved0_SHIFT              16

/* RAAGA_DSP_RGR :: RBUS_TIMER :: timer_value [15:00] */
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER_timer_value_MASK             0x0000ffff
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER_timer_value_SHIFT            0
#define BCHP_RAAGA_DSP_RGR_RBUS_TIMER_timer_value_DEFAULT          0x0000ffff

#endif /* #ifndef BCHP_RAAGA_DSP_RGR_H__ */

/* End of File */
