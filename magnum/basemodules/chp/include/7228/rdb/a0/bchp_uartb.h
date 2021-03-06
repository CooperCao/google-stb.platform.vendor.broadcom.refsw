/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
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
 * Date:           Generated on               Fri Jul 11 11:35:56 2014
 *                 Full Compile MD5 Checksum  74317a30eab2c0396bb2d7c2dd9c02f7
 *                     (minus title and desc)
 *                 MD5 Checksum               2e09daa633013a10114c0860c809d8eb
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14441
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_UARTB_H__
#define BCHP_UARTB_H__

/***************************************************************************
 *UARTB - UART B
 ***************************************************************************/
#define BCHP_UARTB_RBR                           0x00406840 /* Receive Buffer Register */
#define BCHP_UARTB_THR                           0x00406840 /* Transmit Holding Register */
#define BCHP_UARTB_DLH                           0x00406844 /* Divisor Latch High */
#define BCHP_UARTB_DLL                           0x00406840 /* Divisor Latch Low */
#define BCHP_UARTB_IER                           0x00406844 /* Interrupt Enable Register */
#define BCHP_UARTB_IIR                           0x00406848 /* Interrupt Identity Register */
#define BCHP_UARTB_FCR                           0x00406848 /* FIFO Control Register */
#define BCHP_UARTB_LCR                           0x0040684c /* Line Control Register */
#define BCHP_UARTB_MCR                           0x00406850 /* Modem Control Register */
#define BCHP_UARTB_LSR                           0x00406854 /* Line Status Register */
#define BCHP_UARTB_MSR                           0x00406858 /* Modem Status Register */
#define BCHP_UARTB_SCR                           0x0040685c /* Scratchpad Register */

#endif /* #ifndef BCHP_UARTB_H__ */

/* End of File */
