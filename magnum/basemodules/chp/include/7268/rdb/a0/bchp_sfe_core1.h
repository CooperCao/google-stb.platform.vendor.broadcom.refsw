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
 * Date:           Generated on               Mon Nov 30 09:49:00 2015
 *                 Full Compile MD5 Checksum  1e9e6fafcad3e4be7081dbf62ac498b1
 *                     (minus title and desc)
 *                 MD5 Checksum               e2d1ec86648badd2d3ecac8333ba3ddb
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     535
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
 ***************************************************************************/

#ifndef BCHP_SFE_CORE1_H__
#define BCHP_SFE_CORE1_H__

/***************************************************************************
 *SFE_CORE1 - SFE Core1 Registers
 ***************************************************************************/
#define BCHP_SFE_CORE1_CL_FCW                    0x21220400 /* [RW] Carrier Loop Frequency Control Word */
#define BCHP_SFE_CORE1_CL_OFFSET                 0x21220404 /* [RW] Carrier Loop offset */
#define BCHP_SFE_CORE1_CL_PA                     0x21220408 /* [RW] Carrier Loop Phase Accumulator */
#define BCHP_SFE_CORE1_SPINV                     0x2122040c /* [RW] SPINV for mixer */
#define BCHP_SFE_CORE1_TL_NCO                    0x21220410 /* [RW] timing loop NCO */
#define BCHP_SFE_CORE1_TL_FCW                    0x21220414 /* [RW] timing loop NCO FCW */
#define BCHP_SFE_CORE1_TL_OFFSET                 0x21220418 /* [RW] timing loop OFFSET */
#define BCHP_SFE_CORE1_TL_NCO_CTL                0x2122041c /* [RW] timing loop NCO CTL */
#define BCHP_SFE_CORE1_IRF_H0                    0x21220420 /* [RW] IRF filter programmble coeff 0 */
#define BCHP_SFE_CORE1_IRF_H1                    0x21220424 /* [RW] IRF filter programmble coeff 1 */
#define BCHP_SFE_CORE1_IRF_H2                    0x21220428 /* [RW] IRF filter programmble coeff 2 */
#define BCHP_SFE_CORE1_IRF_H3                    0x2122042c /* [RW] IRF filter programmble coeff 3 */
#define BCHP_SFE_CORE1_IRF_H4                    0x21220430 /* [RW] IRF filter programmble coeff 4 */
#define BCHP_SFE_CORE1_IRF_H5                    0x21220434 /* [RW] IRF filter programmble coeff 5 */
#define BCHP_SFE_CORE1_IRF_H6                    0x21220438 /* [RW] IRF filter programmble coeff 6 */
#define BCHP_SFE_CORE1_IRF_H7                    0x2122043c /* [RW] IRF filter programmble coeff 7 */
#define BCHP_SFE_CORE1_IRF_H8                    0x21220440 /* [RW] IRF filter programmble coeff 8 */
#define BCHP_SFE_CORE1_IRF_H9                    0x21220444 /* [RW] IRF filter programmble coeff 9 */
#define BCHP_SFE_CORE1_IRF_H10                   0x21220448 /* [RW] IRF filter programmble coeff 10 */
#define BCHP_SFE_CORE1_IRF_H11                   0x2122044c /* [RW] IRF filter programmble coeff 11 */
#define BCHP_SFE_CORE1_IRF_H12                   0x21220450 /* [RW] IRF filter programmble coeff 12 */
#define BCHP_SFE_CORE1_IRF_H13                   0x21220454 /* [RW] IRF filter programmble coeff 13 */
#define BCHP_SFE_CORE1_IRF_H14                   0x21220458 /* [RW] IRF filter programmble coeff 14 */
#define BCHP_SFE_CORE1_IRF_H15                   0x2122045c /* [RW] IRF filter programmble coeff 15 */
#define BCHP_SFE_CORE1_IRF_H16                   0x21220460 /* [RW] IRF filter programmble coeff 16 */
#define BCHP_SFE_CORE1_IRF_H17                   0x21220464 /* [RW] IRF filter programmble coeff 17 */
#define BCHP_SFE_CORE1_IRF_H18                   0x21220468 /* [RW] IRF filter programmble coeff 18 */
#define BCHP_SFE_CORE1_IRF_H19                   0x2122046c /* [RW] IRF filter programmble coeff 19 */
#define BCHP_SFE_CORE1_IRF_H20                   0x21220470 /* [RW] IRF filter programmble coeff 20 */
#define BCHP_SFE_CORE1_NYQ_H0                    0x21220480 /* [RW] NYQ filter programmble coeff 0 */
#define BCHP_SFE_CORE1_NYQ_H1                    0x21220484 /* [RW] NYQ filter programmble coeff 1 */
#define BCHP_SFE_CORE1_NYQ_H2                    0x21220488 /* [RW] NYQ filter programmble coeff 2 */
#define BCHP_SFE_CORE1_NYQ_H3                    0x2122048c /* [RW] NYQ filter programmble coeff 3 */
#define BCHP_SFE_CORE1_NYQ_H4                    0x21220490 /* [RW] NYQ filter programmble coeff 4 */
#define BCHP_SFE_CORE1_NYQ_H5                    0x21220494 /* [RW] NYQ filter programmble coeff 5 */
#define BCHP_SFE_CORE1_NYQ_H6                    0x21220498 /* [RW] NYQ filter programmble coeff 6 */
#define BCHP_SFE_CORE1_NYQ_H7                    0x2122049c /* [RW] NYQ filter programmble coeff 7 */
#define BCHP_SFE_CORE1_NYQ_H8                    0x212204a0 /* [RW] NYQ filter programmble coeff 8 */
#define BCHP_SFE_CORE1_NYQ_H9                    0x212204a4 /* [RW] NYQ filter programmble coeff 9 */
#define BCHP_SFE_CORE1_NYQ_H10                   0x212204a8 /* [RW] NYQ filter programmble coeff 10 */
#define BCHP_SFE_CORE1_NYQ_H11                   0x212204ac /* [RW] NYQ filter programmble coeff 11 */
#define BCHP_SFE_CORE1_NYQ_H12                   0x212204b0 /* [RW] NYQ filter programmble coeff 12 */
#define BCHP_SFE_CORE1_NYQ_H13                   0x212204b4 /* [RW] NYQ filter programmble coeff 13 */
#define BCHP_SFE_CORE1_NYQ_H14                   0x212204b8 /* [RW] NYQ filter programmble coeff 14 */
#define BCHP_SFE_CORE1_NYQ_H15                   0x212204bc /* [RW] NYQ filter programmble coeff 15 */
#define BCHP_SFE_CORE1_NYQ_H16                   0x212204c0 /* [RW] NYQ filter programmble coeff 16 */
#define BCHP_SFE_CORE1_NYQ_H17                   0x212204c4 /* [RW] NYQ filter programmble coeff 17 */
#define BCHP_SFE_CORE1_NYQ_H18                   0x212204c8 /* [RW] NYQ filter programmble coeff 18 */
#define BCHP_SFE_CORE1_NYQ_H19                   0x212204cc /* [RW] NYQ filter programmble coeff 19 */
#define BCHP_SFE_CORE1_NYQ_H20                   0x212204d0 /* [RW] NYQ filter programmble coeff 20 */
#define BCHP_SFE_CORE1_NYQ_H21                   0x212204d4 /* [RW] NYQ filter programmble coeff 21 */
#define BCHP_SFE_CORE1_NYQ_H22                   0x212204d8 /* [RW] NYQ filter programmble coeff 22 */
#define BCHP_SFE_CORE1_NYQ_H23                   0x212204dc /* [RW] NYQ filter programmble coeff 23 */
#define BCHP_SFE_CORE1_NYQ_H24                   0x212204e0 /* [RW] NYQ filter programmble coeff 24 */
#define BCHP_SFE_CORE1_NYQ_H25                   0x212204e4 /* [RW] NYQ filter programmble coeff 25 */
#define BCHP_SFE_CORE1_NYQ_H26                   0x212204e8 /* [RW] NYQ filter programmble coeff 26 */
#define BCHP_SFE_CORE1_NYQ_H27                   0x212204ec /* [RW] NYQ filter programmble coeff 27 */
#define BCHP_SFE_CORE1_NYQ_H28                   0x212204f0 /* [RW] NYQ filter programmble coeff 28 */
#define BCHP_SFE_CORE1_NYQ_H29                   0x212204f4 /* [RW] NYQ filter programmble coeff 29 */
#define BCHP_SFE_CORE1_NYQ_H30                   0x212204f8 /* [RW] NYQ filter programmble coeff 30 */
#define BCHP_SFE_CORE1_NYQ_H31                   0x212204fc /* [RW] NYQ filter programmble coeff 31 */
#define BCHP_SFE_CORE1_NYQ_H32                   0x21220500 /* [RW] NYQ filter programmble coeff 32 */
#define BCHP_SFE_CORE1_NYQ_H33                   0x21220504 /* [RW] NYQ filter programmble coeff 33 */
#define BCHP_SFE_CORE1_NYQ_H34                   0x21220508 /* [RW] NYQ filter programmble coeff 34 */
#define BCHP_SFE_CORE1_DAGC                      0x21220510 /* [RW] Digital AGC2 Loop Control */
#define BCHP_SFE_CORE1_DAGC_INT                  0x21220514 /* [RW] Digital AGC2 Loop Filter Output */
#define BCHP_SFE_CORE1_DAGC_UPDATE               0x21220518 /* [RW] Digital AGC2 Loop Update Control */
#define BCHP_SFE_CORE1_OUT_SEL                   0x2122051c /* [RW] OUTput select to SCB IF */
#define BCHP_SFE_CORE1_AAGC                      0x21220520 /* [RW] Analog AGC Loop Control */
#define BCHP_SFE_CORE1_AAGC_COEF                 0x21220524 /* [RW] Analog AGC Loop Coefficients */
#define BCHP_SFE_CORE1_AAGC_THRESH               0x21220528 /* [RW] Analog AGC Loop Thresholds */
#define BCHP_SFE_CORE1_AAGC_ILIMIT               0x2122052c /* [RW] Analog AGC IF Loop Limits */
#define BCHP_SFE_CORE1_AAGC_IFILT                0x21220530 /* [RW] Analog AGC Loop IF Filter Output */
#define BCHP_SFE_CORE1_AAGC_UPDATE               0x21220534 /* [RW] Analog AGC Loop Update Control */
#define BCHP_SFE_CORE1_AAGC_OV                   0x21220538 /* [RW] Analog AGC Loop Override */
#define BCHP_SFE_CORE1_AAGC_IPWR                 0x2122053c /* [RO] Analog AGC IF Leaky Averager */
#define BCHP_SFE_CORE1_AAGC_IF_DELSIG_FCNT       0x21220540 /* [RW] Delta-Sigma Clock Rate Control */
#define BCHP_SFE_CORE1_TIMERCTL0                 0x21220550 /* [RW] TIMERCTL0 for Interrupt generation */
#define BCHP_SFE_CORE1_TIMERCTL1                 0x21220554 /* [RW] TIMERCTL1 for Interrupt generation */
#define BCHP_SFE_CORE1_SCB_CTL                   0x21220560 /* [RW] SCB Interface Control Register */
#define BCHP_SFE_CORE1_SCB_BUF_INT_THRESH        0x21220564 /* [RW] SCB Buffer Interrupt Trigger Threshold */
#define BCHP_SFE_CORE1_SCB_BASE_ADDR_LO          0x21220570 /* [RW] SCB Circular Buffer Base address (Lower 32-bit) */
#define BCHP_SFE_CORE1_SCB_BASE_ADDR_HI          0x21220574 /* [RW] SCB Circular Buffer Base address (Upper 8-bit) */
#define BCHP_SFE_CORE1_SCB_END_ADDR_LO           0x21220578 /* [RW] SCB Circular Buffer End address (Lower 32-bit) */
#define BCHP_SFE_CORE1_SCB_END_ADDR_HI           0x2122057c /* [RW] SCB Circular Buffer End address (Upper 8-bit) */
#define BCHP_SFE_CORE1_SCB_WRITE_ADDR_LO         0x21220580 /* [RO] SCB Circular Buffer Write address (Lower 32-bit) */
#define BCHP_SFE_CORE1_SCB_WRITE_ADDR_HI         0x21220584 /* [RO] SCB Circular Buffer Write address (Upper 8-bit) */
#define BCHP_SFE_CORE1_SCB_READ_ADDR_LO          0x21220588 /* [RW] SCB Circular Buffer Read address (Lower 32-bit) */
#define BCHP_SFE_CORE1_SCB_READ_ADDR_HI          0x2122058c /* [RW] SCB Circular Buffer Read address (Upper 8-bit) */
#define BCHP_SFE_CORE1_SCB_STATUS                0x21220590 /* [RO] SCB Interface Status */
#define BCHP_SFE_CORE1_SCB_BUF_SIZE              0x21220594 /* [RO] SCB Circular Buffer Size */
#define BCHP_SFE_CORE1_TPIN                      0x21220598 /* [RW] Reg Overwrite for mixer input */
#define BCHP_SFE_CORE1_TPOUT_SEL                 0x2122059c /* [RW] Select tpout readback input */
#define BCHP_SFE_CORE1_TPOUT                     0x212205a0 /* [RO] tpout readback */
#define BCHP_SFE_CORE1_HDOFFCTL                  0x212205a4 /* [RW] HDOFFCTL */
#define BCHP_SFE_CORE1_HDOFFSTS                  0x212205a8 /* [RO] HDOFFSTS */
#define BCHP_SFE_CORE1_COUNTER_ENA               0x212205b0 /* [RW] COUNTER_ENA */
#define BCHP_SFE_CORE1_FS_COUNTER                0x212205b4 /* [RO] FS_COUNTER */
#define BCHP_SFE_CORE1_F1B_COUNTER               0x212205b8 /* [RO] F1B_COUNTER */
#define BCHP_SFE_CORE1_F2B_COUNTER               0x212205bc /* [RO] F2B_COUNTER */
#define BCHP_SFE_CORE1_RESERVED0                 0x212205c0 /* [RW] Reserved Register0 */
#define BCHP_SFE_CORE1_RESERVED1                 0x212205c4 /* [RW] Reserved Register1 */
#define BCHP_SFE_CORE1_SW_SPARE0                 0x212205c8 /* [RW] SW Spare Register0 */
#define BCHP_SFE_CORE1_SW_SPARE1                 0x212205cc /* [RW] SW Spare Register1 */

#endif /* #ifndef BCHP_SFE_CORE1_H__ */

/* End of File */
