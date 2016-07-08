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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
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

#ifndef BCHP_SDS_CWC_1_H__
#define BCHP_SDS_CWC_1_H__

/***************************************************************************
 *SDS_CWC_1 - SDS CW Canceller Register Set
 ***************************************************************************/
#define BCHP_SDS_CWC_1_CTRL1                     0x01241600 /* [RW] CWC Control 1 */
#define BCHP_SDS_CWC_1_CTRL2                     0x01241604 /* [RW] CWC Control 2 */
#define BCHP_SDS_CWC_1_LEAK                      0x01241608 /* [RW] CWC Leak Control */
#define BCHP_SDS_CWC_1_SPUR_FCW1                 0x01241610 /* [RW] CWC1 NCO frequency control word */
#define BCHP_SDS_CWC_1_SPUR_FCW2                 0x01241614 /* [RW] CWC2 NCO frequency control word */
#define BCHP_SDS_CWC_1_SPUR_FCW3                 0x01241618 /* [RW] CWC3 NCO frequency control word */
#define BCHP_SDS_CWC_1_SPUR_FCW4                 0x0124161c /* [RW] CWC4 NCO frequency control word */
#define BCHP_SDS_CWC_1_SPUR_FCW5                 0x01241620 /* [RW] CWC5 NCO frequency control word */
#define BCHP_SDS_CWC_1_SPUR_FCW6                 0x01241624 /* [RW] CWC6 NCO frequency control word */
#define BCHP_SDS_CWC_1_LFC1                      0x01241630 /* [RW] CWC1 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_LFC2                      0x01241634 /* [RW] CWC2 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_LFC3                      0x01241638 /* [RW] CWC3 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_LFC4                      0x0124163c /* [RW] CWC4 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_LFC5                      0x01241640 /* [RW] CWC5 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_LFC6                      0x01241644 /* [RW] CWC6 Loop Filter Coefficients */
#define BCHP_SDS_CWC_1_INT1                      0x01241650 /* [RO] CWC1 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_INT2                      0x01241654 /* [RO] CWC2 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_INT3                      0x01241658 /* [RO] CWC3 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_INT4                      0x0124165c /* [RO] CWC4 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_INT5                      0x01241660 /* [RO] CWC5 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_INT6                      0x01241664 /* [RO] CWC6 Loop Filter INtegrator */
#define BCHP_SDS_CWC_1_COEFF_RWCTRL              0x01241670 /* [RW] CWC Coefficient Read/Write Control */
#define BCHP_SDS_CWC_1_COEFF                     0x01241674 /* [RW] CWC Coefficients */
#define BCHP_SDS_CWC_1_FOFS1                     0x01241680 /* [RW] Frequency Offset for CWC1 */
#define BCHP_SDS_CWC_1_FOFS2                     0x01241684 /* [RW] Frequency Offset for CWC2 */
#define BCHP_SDS_CWC_1_FOFS3                     0x01241688 /* [RW] Frequency Offset for CWC3 */
#define BCHP_SDS_CWC_1_FOFS4                     0x0124168c /* [RW] Frequency Offset for CWC4 */
#define BCHP_SDS_CWC_1_FOFS5                     0x01241690 /* [RW] Frequency Offset for CWC5 */
#define BCHP_SDS_CWC_1_FOFS6                     0x01241694 /* [RW] Frequency Offset for CWC6 */

#endif /* #ifndef BCHP_SDS_CWC_1_H__ */

/* End of File */
