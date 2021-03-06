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
 * Date:           Generated on               Tue Aug 25 10:46:55 2015
 *                 Full Compile MD5 Checksum  add20cb7888302c2ee8be1277223a4e4
 *                     (minus title and desc)
 *                 MD5 Checksum               f64b4ec86ad9ad7523e5d75b1dc732c5
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

#ifndef BCHP_SDS_AGC_1_H__
#define BCHP_SDS_AGC_1_H__

/***************************************************************************
 *SDS_AGC_1 - SDS AGC Register Set
 ***************************************************************************/
#define BCHP_SDS_AGC_1_AGCCTL                    0x00701100 /* [RW] AGC Control (Formerly, AGCMISC, AGTCTL, AGICTL) */
#define BCHP_SDS_AGC_1_IAGCTH                    0x00701104 /* [RW] Internal tuner AGC threshold */
#define BCHP_SDS_AGC_1_DSGIN                     0x00701108 /* [RW] DSGIN Register */
#define BCHP_SDS_AGC_1_ATHR                      0x0070110c /* [RW] IF AGC loop threshold */
#define BCHP_SDS_AGC_1_ABW                       0x00701110 /* [RW] IF and RF AGC loop bandwidths */
#define BCHP_SDS_AGC_1_AII                       0x00701114 /* [RW] IF AGC integrator */
#define BCHP_SDS_AGC_1_AGI                       0x00701118 /* [RW] IF AGC gain threshold */
#define BCHP_SDS_AGC_1_AIT                       0x0070111c /* [RW] RF AGC integrator */
#define BCHP_SDS_AGC_1_AGT                       0x00701120 /* [RW] RF AGC gain threshold */
#define BCHP_SDS_AGC_1_AGCLI                     0x00701124 /* [RW] IF AGC delta-sigma fixed gain value and input select. */

#endif /* #ifndef BCHP_SDS_AGC_1_H__ */

/* End of File */
