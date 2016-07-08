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
 * Date:           Generated on               Mon Aug 24 11:29:31 2015
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

#ifndef BCHP_SAGE_UART_H__
#define BCHP_SAGE_UART_H__

/***************************************************************************
 *SAGE_UART - Sage UART in the sage UART clock domain
 ***************************************************************************/
#define BCHP_SAGE_UART_RBR                       0x20312200 /* [RO] Receive Buffer Register */
#define BCHP_SAGE_UART_THR                       0x20312200 /* [WO] Transmit Holding Register */
#define BCHP_SAGE_UART_DLL                       0x20312200 /* [RW] Divisor Latch Low */
#define BCHP_SAGE_UART_DLH                       0x20312204 /* [RW] Divisor Latch High */
#define BCHP_SAGE_UART_IER                       0x20312204 /* [RW] Interrupt Enable Register */
#define BCHP_SAGE_UART_IIR                       0x20312208 /* [RO] Interrupt Identity Register */
#define BCHP_SAGE_UART_FCR                       0x20312208 /* [WO] FIFO Control Register */
#define BCHP_SAGE_UART_LCR                       0x2031220c /* [RW] Line Control Register */
#define BCHP_SAGE_UART_MCR                       0x20312210 /* [RW] Modem Control Register */
#define BCHP_SAGE_UART_LSR                       0x20312214 /* [RO] Line Status Register */
#define BCHP_SAGE_UART_MSR                       0x20312218 /* [RO] Modem Status Register */
#define BCHP_SAGE_UART_SCR                       0x2031221c /* [RW] Scratchpad Register */

/***************************************************************************
 *RBR - Receive Buffer Register
 ***************************************************************************/
/* SAGE_UART :: RBR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_RBR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_RBR_reserved0_SHIFT                         8

/* SAGE_UART :: RBR :: RBR [07:00] */
#define BCHP_SAGE_UART_RBR_RBR_MASK                                0x000000ff
#define BCHP_SAGE_UART_RBR_RBR_SHIFT                               0
#define BCHP_SAGE_UART_RBR_RBR_DEFAULT                             0x00000000

/***************************************************************************
 *THR - Transmit Holding Register
 ***************************************************************************/
/* SAGE_UART :: THR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_THR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_THR_reserved0_SHIFT                         8

/* SAGE_UART :: THR :: THR [07:00] */
#define BCHP_SAGE_UART_THR_THR_MASK                                0x000000ff
#define BCHP_SAGE_UART_THR_THR_SHIFT                               0
#define BCHP_SAGE_UART_THR_THR_DEFAULT                             0x00000000

/***************************************************************************
 *DLL - Divisor Latch Low
 ***************************************************************************/
/* SAGE_UART :: DLL :: reserved0 [31:08] */
#define BCHP_SAGE_UART_DLL_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_DLL_reserved0_SHIFT                         8

/* SAGE_UART :: DLL :: DLL [07:00] */
#define BCHP_SAGE_UART_DLL_DLL_MASK                                0x000000ff
#define BCHP_SAGE_UART_DLL_DLL_SHIFT                               0
#define BCHP_SAGE_UART_DLL_DLL_DEFAULT                             0x00000000

/***************************************************************************
 *DLH - Divisor Latch High
 ***************************************************************************/
/* SAGE_UART :: DLH :: reserved0 [31:08] */
#define BCHP_SAGE_UART_DLH_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_DLH_reserved0_SHIFT                         8

/* SAGE_UART :: DLH :: DLH [07:00] */
#define BCHP_SAGE_UART_DLH_DLH_MASK                                0x000000ff
#define BCHP_SAGE_UART_DLH_DLH_SHIFT                               0
#define BCHP_SAGE_UART_DLH_DLH_DEFAULT                             0x00000000

/***************************************************************************
 *IER - Interrupt Enable Register
 ***************************************************************************/
/* SAGE_UART :: IER :: reserved0 [31:08] */
#define BCHP_SAGE_UART_IER_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_IER_reserved0_SHIFT                         8

/* SAGE_UART :: IER :: PTIME [07:07] */
#define BCHP_SAGE_UART_IER_PTIME_MASK                              0x00000080
#define BCHP_SAGE_UART_IER_PTIME_SHIFT                             7
#define BCHP_SAGE_UART_IER_PTIME_DEFAULT                           0x00000000

/* SAGE_UART :: IER :: reserved1 [06:04] */
#define BCHP_SAGE_UART_IER_reserved1_MASK                          0x00000070
#define BCHP_SAGE_UART_IER_reserved1_SHIFT                         4

/* SAGE_UART :: IER :: EDSSI [03:03] */
#define BCHP_SAGE_UART_IER_EDSSI_MASK                              0x00000008
#define BCHP_SAGE_UART_IER_EDSSI_SHIFT                             3
#define BCHP_SAGE_UART_IER_EDSSI_DEFAULT                           0x00000000

/* SAGE_UART :: IER :: ELSI [02:02] */
#define BCHP_SAGE_UART_IER_ELSI_MASK                               0x00000004
#define BCHP_SAGE_UART_IER_ELSI_SHIFT                              2
#define BCHP_SAGE_UART_IER_ELSI_DEFAULT                            0x00000000

/* SAGE_UART :: IER :: ETBEI [01:01] */
#define BCHP_SAGE_UART_IER_ETBEI_MASK                              0x00000002
#define BCHP_SAGE_UART_IER_ETBEI_SHIFT                             1
#define BCHP_SAGE_UART_IER_ETBEI_DEFAULT                           0x00000000

/* SAGE_UART :: IER :: ERBFI [00:00] */
#define BCHP_SAGE_UART_IER_ERBFI_MASK                              0x00000001
#define BCHP_SAGE_UART_IER_ERBFI_SHIFT                             0
#define BCHP_SAGE_UART_IER_ERBFI_DEFAULT                           0x00000000

/***************************************************************************
 *IIR - Interrupt Identity Register
 ***************************************************************************/
/* SAGE_UART :: IIR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_IIR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_IIR_reserved0_SHIFT                         8

/* SAGE_UART :: IIR :: FIFOSE [07:06] */
#define BCHP_SAGE_UART_IIR_FIFOSE_MASK                             0x000000c0
#define BCHP_SAGE_UART_IIR_FIFOSE_SHIFT                            6
#define BCHP_SAGE_UART_IIR_FIFOSE_DEFAULT                          0x00000000

/* SAGE_UART :: IIR :: reserved1 [05:04] */
#define BCHP_SAGE_UART_IIR_reserved1_MASK                          0x00000030
#define BCHP_SAGE_UART_IIR_reserved1_SHIFT                         4

/* SAGE_UART :: IIR :: IID [03:00] */
#define BCHP_SAGE_UART_IIR_IID_MASK                                0x0000000f
#define BCHP_SAGE_UART_IIR_IID_SHIFT                               0
#define BCHP_SAGE_UART_IIR_IID_DEFAULT                             0x00000001

/***************************************************************************
 *FCR - FIFO Control Register
 ***************************************************************************/
/* SAGE_UART :: FCR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_FCR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_FCR_reserved0_SHIFT                         8

/* SAGE_UART :: FCR :: RT [07:06] */
#define BCHP_SAGE_UART_FCR_RT_MASK                                 0x000000c0
#define BCHP_SAGE_UART_FCR_RT_SHIFT                                6
#define BCHP_SAGE_UART_FCR_RT_DEFAULT                              0x00000000

/* SAGE_UART :: FCR :: TET [05:04] */
#define BCHP_SAGE_UART_FCR_TET_MASK                                0x00000030
#define BCHP_SAGE_UART_FCR_TET_SHIFT                               4
#define BCHP_SAGE_UART_FCR_TET_DEFAULT                             0x00000000

/* SAGE_UART :: FCR :: DMAM [03:03] */
#define BCHP_SAGE_UART_FCR_DMAM_MASK                               0x00000008
#define BCHP_SAGE_UART_FCR_DMAM_SHIFT                              3
#define BCHP_SAGE_UART_FCR_DMAM_DEFAULT                            0x00000000

/* SAGE_UART :: FCR :: XFIFOR [02:02] */
#define BCHP_SAGE_UART_FCR_XFIFOR_MASK                             0x00000004
#define BCHP_SAGE_UART_FCR_XFIFOR_SHIFT                            2
#define BCHP_SAGE_UART_FCR_XFIFOR_DEFAULT                          0x00000000

/* SAGE_UART :: FCR :: RFIFOR [01:01] */
#define BCHP_SAGE_UART_FCR_RFIFOR_MASK                             0x00000002
#define BCHP_SAGE_UART_FCR_RFIFOR_SHIFT                            1
#define BCHP_SAGE_UART_FCR_RFIFOR_DEFAULT                          0x00000000

/* SAGE_UART :: FCR :: FIFOE [00:00] */
#define BCHP_SAGE_UART_FCR_FIFOE_MASK                              0x00000001
#define BCHP_SAGE_UART_FCR_FIFOE_SHIFT                             0
#define BCHP_SAGE_UART_FCR_FIFOE_DEFAULT                           0x00000000

/***************************************************************************
 *LCR - Line Control Register
 ***************************************************************************/
/* SAGE_UART :: LCR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_LCR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_LCR_reserved0_SHIFT                         8

/* SAGE_UART :: LCR :: DLAB [07:07] */
#define BCHP_SAGE_UART_LCR_DLAB_MASK                               0x00000080
#define BCHP_SAGE_UART_LCR_DLAB_SHIFT                              7
#define BCHP_SAGE_UART_LCR_DLAB_DEFAULT                            0x00000000

/* SAGE_UART :: LCR :: BC [06:06] */
#define BCHP_SAGE_UART_LCR_BC_MASK                                 0x00000040
#define BCHP_SAGE_UART_LCR_BC_SHIFT                                6
#define BCHP_SAGE_UART_LCR_BC_DEFAULT                              0x00000000

/* SAGE_UART :: LCR :: reserved1 [05:05] */
#define BCHP_SAGE_UART_LCR_reserved1_MASK                          0x00000020
#define BCHP_SAGE_UART_LCR_reserved1_SHIFT                         5

/* SAGE_UART :: LCR :: EPS [04:04] */
#define BCHP_SAGE_UART_LCR_EPS_MASK                                0x00000010
#define BCHP_SAGE_UART_LCR_EPS_SHIFT                               4
#define BCHP_SAGE_UART_LCR_EPS_DEFAULT                             0x00000000

/* SAGE_UART :: LCR :: PEN [03:03] */
#define BCHP_SAGE_UART_LCR_PEN_MASK                                0x00000008
#define BCHP_SAGE_UART_LCR_PEN_SHIFT                               3
#define BCHP_SAGE_UART_LCR_PEN_DEFAULT                             0x00000000

/* SAGE_UART :: LCR :: STOP [02:02] */
#define BCHP_SAGE_UART_LCR_STOP_MASK                               0x00000004
#define BCHP_SAGE_UART_LCR_STOP_SHIFT                              2
#define BCHP_SAGE_UART_LCR_STOP_DEFAULT                            0x00000000

/* SAGE_UART :: LCR :: DLS [01:00] */
#define BCHP_SAGE_UART_LCR_DLS_MASK                                0x00000003
#define BCHP_SAGE_UART_LCR_DLS_SHIFT                               0
#define BCHP_SAGE_UART_LCR_DLS_DEFAULT                             0x00000000

/***************************************************************************
 *MCR - Modem Control Register
 ***************************************************************************/
/* SAGE_UART :: MCR :: reserved0 [31:07] */
#define BCHP_SAGE_UART_MCR_reserved0_MASK                          0xffffff80
#define BCHP_SAGE_UART_MCR_reserved0_SHIFT                         7

/* SAGE_UART :: MCR :: SIRE [06:06] */
#define BCHP_SAGE_UART_MCR_SIRE_MASK                               0x00000040
#define BCHP_SAGE_UART_MCR_SIRE_SHIFT                              6
#define BCHP_SAGE_UART_MCR_SIRE_DEFAULT                            0x00000000

/* SAGE_UART :: MCR :: AFCE [05:05] */
#define BCHP_SAGE_UART_MCR_AFCE_MASK                               0x00000020
#define BCHP_SAGE_UART_MCR_AFCE_SHIFT                              5
#define BCHP_SAGE_UART_MCR_AFCE_DEFAULT                            0x00000000

/* SAGE_UART :: MCR :: LB [04:04] */
#define BCHP_SAGE_UART_MCR_LB_MASK                                 0x00000010
#define BCHP_SAGE_UART_MCR_LB_SHIFT                                4
#define BCHP_SAGE_UART_MCR_LB_DEFAULT                              0x00000000

/* SAGE_UART :: MCR :: OUT2 [03:03] */
#define BCHP_SAGE_UART_MCR_OUT2_MASK                               0x00000008
#define BCHP_SAGE_UART_MCR_OUT2_SHIFT                              3
#define BCHP_SAGE_UART_MCR_OUT2_DEFAULT                            0x00000000

/* SAGE_UART :: MCR :: OUT1 [02:02] */
#define BCHP_SAGE_UART_MCR_OUT1_MASK                               0x00000004
#define BCHP_SAGE_UART_MCR_OUT1_SHIFT                              2
#define BCHP_SAGE_UART_MCR_OUT1_DEFAULT                            0x00000000

/* SAGE_UART :: MCR :: RTS [01:01] */
#define BCHP_SAGE_UART_MCR_RTS_MASK                                0x00000002
#define BCHP_SAGE_UART_MCR_RTS_SHIFT                               1
#define BCHP_SAGE_UART_MCR_RTS_DEFAULT                             0x00000000

/* SAGE_UART :: MCR :: DTR [00:00] */
#define BCHP_SAGE_UART_MCR_DTR_MASK                                0x00000001
#define BCHP_SAGE_UART_MCR_DTR_SHIFT                               0
#define BCHP_SAGE_UART_MCR_DTR_DEFAULT                             0x00000000

/***************************************************************************
 *LSR - Line Status Register
 ***************************************************************************/
/* SAGE_UART :: LSR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_LSR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_LSR_reserved0_SHIFT                         8

/* SAGE_UART :: LSR :: RFE [07:07] */
#define BCHP_SAGE_UART_LSR_RFE_MASK                                0x00000080
#define BCHP_SAGE_UART_LSR_RFE_SHIFT                               7
#define BCHP_SAGE_UART_LSR_RFE_DEFAULT                             0x00000000

/* SAGE_UART :: LSR :: TEMT [06:06] */
#define BCHP_SAGE_UART_LSR_TEMT_MASK                               0x00000040
#define BCHP_SAGE_UART_LSR_TEMT_SHIFT                              6
#define BCHP_SAGE_UART_LSR_TEMT_DEFAULT                            0x00000001

/* SAGE_UART :: LSR :: THRE [05:05] */
#define BCHP_SAGE_UART_LSR_THRE_MASK                               0x00000020
#define BCHP_SAGE_UART_LSR_THRE_SHIFT                              5
#define BCHP_SAGE_UART_LSR_THRE_DEFAULT                            0x00000001

/* SAGE_UART :: LSR :: BI [04:04] */
#define BCHP_SAGE_UART_LSR_BI_MASK                                 0x00000010
#define BCHP_SAGE_UART_LSR_BI_SHIFT                                4
#define BCHP_SAGE_UART_LSR_BI_DEFAULT                              0x00000000

/* SAGE_UART :: LSR :: FE [03:03] */
#define BCHP_SAGE_UART_LSR_FE_MASK                                 0x00000008
#define BCHP_SAGE_UART_LSR_FE_SHIFT                                3
#define BCHP_SAGE_UART_LSR_FE_DEFAULT                              0x00000000

/* SAGE_UART :: LSR :: PE [02:02] */
#define BCHP_SAGE_UART_LSR_PE_MASK                                 0x00000004
#define BCHP_SAGE_UART_LSR_PE_SHIFT                                2
#define BCHP_SAGE_UART_LSR_PE_DEFAULT                              0x00000000

/* SAGE_UART :: LSR :: OE [01:01] */
#define BCHP_SAGE_UART_LSR_OE_MASK                                 0x00000002
#define BCHP_SAGE_UART_LSR_OE_SHIFT                                1
#define BCHP_SAGE_UART_LSR_OE_DEFAULT                              0x00000000

/* SAGE_UART :: LSR :: DR [00:00] */
#define BCHP_SAGE_UART_LSR_DR_MASK                                 0x00000001
#define BCHP_SAGE_UART_LSR_DR_SHIFT                                0
#define BCHP_SAGE_UART_LSR_DR_DEFAULT                              0x00000000

/***************************************************************************
 *MSR - Modem Status Register
 ***************************************************************************/
/* SAGE_UART :: MSR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_MSR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_MSR_reserved0_SHIFT                         8

/* SAGE_UART :: MSR :: DCD [07:07] */
#define BCHP_SAGE_UART_MSR_DCD_MASK                                0x00000080
#define BCHP_SAGE_UART_MSR_DCD_SHIFT                               7
#define BCHP_SAGE_UART_MSR_DCD_DEFAULT                             0x00000000

/* SAGE_UART :: MSR :: RI [06:06] */
#define BCHP_SAGE_UART_MSR_RI_MASK                                 0x00000040
#define BCHP_SAGE_UART_MSR_RI_SHIFT                                6
#define BCHP_SAGE_UART_MSR_RI_DEFAULT                              0x00000000

/* SAGE_UART :: MSR :: DSR [05:05] */
#define BCHP_SAGE_UART_MSR_DSR_MASK                                0x00000020
#define BCHP_SAGE_UART_MSR_DSR_SHIFT                               5
#define BCHP_SAGE_UART_MSR_DSR_DEFAULT                             0x00000000

/* SAGE_UART :: MSR :: CTS [04:04] */
#define BCHP_SAGE_UART_MSR_CTS_MASK                                0x00000010
#define BCHP_SAGE_UART_MSR_CTS_SHIFT                               4
#define BCHP_SAGE_UART_MSR_CTS_DEFAULT                             0x00000000

/* SAGE_UART :: MSR :: DDCD [03:03] */
#define BCHP_SAGE_UART_MSR_DDCD_MASK                               0x00000008
#define BCHP_SAGE_UART_MSR_DDCD_SHIFT                              3
#define BCHP_SAGE_UART_MSR_DDCD_DEFAULT                            0x00000000

/* SAGE_UART :: MSR :: TERI [02:02] */
#define BCHP_SAGE_UART_MSR_TERI_MASK                               0x00000004
#define BCHP_SAGE_UART_MSR_TERI_SHIFT                              2
#define BCHP_SAGE_UART_MSR_TERI_DEFAULT                            0x00000000

/* SAGE_UART :: MSR :: DDSR [01:01] */
#define BCHP_SAGE_UART_MSR_DDSR_MASK                               0x00000002
#define BCHP_SAGE_UART_MSR_DDSR_SHIFT                              1
#define BCHP_SAGE_UART_MSR_DDSR_DEFAULT                            0x00000000

/* SAGE_UART :: MSR :: DCTS [00:00] */
#define BCHP_SAGE_UART_MSR_DCTS_MASK                               0x00000001
#define BCHP_SAGE_UART_MSR_DCTS_SHIFT                              0
#define BCHP_SAGE_UART_MSR_DCTS_DEFAULT                            0x00000000

/***************************************************************************
 *SCR - Scratchpad Register
 ***************************************************************************/
/* SAGE_UART :: SCR :: reserved0 [31:08] */
#define BCHP_SAGE_UART_SCR_reserved0_MASK                          0xffffff00
#define BCHP_SAGE_UART_SCR_reserved0_SHIFT                         8

/* SAGE_UART :: SCR :: SCR [07:00] */
#define BCHP_SAGE_UART_SCR_SCR_MASK                                0x000000ff
#define BCHP_SAGE_UART_SCR_SCR_SHIFT                               0
#define BCHP_SAGE_UART_SCR_SCR_DEFAULT                             0x00000000

#endif /* #ifndef BCHP_SAGE_UART_H__ */

/* End of File */
