/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Wed Jun 13 14:32:13 2012
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_IRQ0_AON_H__
#define BCHP_IRQ0_AON_H__

/***************************************************************************
 *IRQ0_AON - Level 2 CPU Interrupt Enable/Status
 ***************************************************************************/
#define BCHP_IRQ0_AON_IRQEN                      0x00408b80 /* Interrupt Enable */
#define BCHP_IRQ0_AON_IRQSTAT                    0x00408b84 /* Interrupt Status */

/***************************************************************************
 *IRQEN - Interrupt Enable
 ***************************************************************************/
/* IRQ0_AON :: IRQEN :: reserved0 [31:28] */
#define BCHP_IRQ0_AON_IRQEN_reserved0_MASK                         0xf0000000
#define BCHP_IRQ0_AON_IRQEN_reserved0_SHIFT                        28

/* IRQ0_AON :: IRQEN :: iicd_irqen [27:27] */
#define BCHP_IRQ0_AON_IRQEN_iicd_irqen_MASK                        0x08000000
#define BCHP_IRQ0_AON_IRQEN_iicd_irqen_SHIFT                       27
#define BCHP_IRQ0_AON_IRQEN_iicd_irqen_DEFAULT                     0x00000000

/* IRQ0_AON :: IRQEN :: reserved1 [26:21] */
#define BCHP_IRQ0_AON_IRQEN_reserved1_MASK                         0x07e00000
#define BCHP_IRQ0_AON_IRQEN_reserved1_SHIFT                        21

/* IRQ0_AON :: IRQEN :: spi_irqen [20:20] */
#define BCHP_IRQ0_AON_IRQEN_spi_irqen_MASK                         0x00100000
#define BCHP_IRQ0_AON_IRQEN_spi_irqen_SHIFT                        20
#define BCHP_IRQ0_AON_IRQEN_spi_irqen_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQEN :: reserved2 [19:09] */
#define BCHP_IRQ0_AON_IRQEN_reserved2_MASK                         0x000ffe00
#define BCHP_IRQ0_AON_IRQEN_reserved2_SHIFT                        9

/* IRQ0_AON :: IRQEN :: kbd3_irqen [08:08] */
#define BCHP_IRQ0_AON_IRQEN_kbd3_irqen_MASK                        0x00000100
#define BCHP_IRQ0_AON_IRQEN_kbd3_irqen_SHIFT                       8
#define BCHP_IRQ0_AON_IRQEN_kbd3_irqen_DEFAULT                     0x00000000

/* IRQ0_AON :: IRQEN :: icap_irqen [07:07] */
#define BCHP_IRQ0_AON_IRQEN_icap_irqen_MASK                        0x00000080
#define BCHP_IRQ0_AON_IRQEN_icap_irqen_SHIFT                       7
#define BCHP_IRQ0_AON_IRQEN_icap_irqen_DEFAULT                     0x00000000

/* IRQ0_AON :: IRQEN :: gio_irqen [06:06] */
#define BCHP_IRQ0_AON_IRQEN_gio_irqen_MASK                         0x00000040
#define BCHP_IRQ0_AON_IRQEN_gio_irqen_SHIFT                        6
#define BCHP_IRQ0_AON_IRQEN_gio_irqen_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQEN :: kbd2_irqen [05:05] */
#define BCHP_IRQ0_AON_IRQEN_kbd2_irqen_MASK                        0x00000020
#define BCHP_IRQ0_AON_IRQEN_kbd2_irqen_SHIFT                       5
#define BCHP_IRQ0_AON_IRQEN_kbd2_irqen_DEFAULT                     0x00000000

/* IRQ0_AON :: IRQEN :: reserved_for_eco3 [04:02] */
#define BCHP_IRQ0_AON_IRQEN_reserved_for_eco3_MASK                 0x0000001c
#define BCHP_IRQ0_AON_IRQEN_reserved_for_eco3_SHIFT                2
#define BCHP_IRQ0_AON_IRQEN_reserved_for_eco3_DEFAULT              0x00000000

/* IRQ0_AON :: IRQEN :: ldk_irqen [01:01] */
#define BCHP_IRQ0_AON_IRQEN_ldk_irqen_MASK                         0x00000002
#define BCHP_IRQ0_AON_IRQEN_ldk_irqen_SHIFT                        1
#define BCHP_IRQ0_AON_IRQEN_ldk_irqen_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQEN :: kbd1_irqen [00:00] */
#define BCHP_IRQ0_AON_IRQEN_kbd1_irqen_MASK                        0x00000001
#define BCHP_IRQ0_AON_IRQEN_kbd1_irqen_SHIFT                       0
#define BCHP_IRQ0_AON_IRQEN_kbd1_irqen_DEFAULT                     0x00000000

/***************************************************************************
 *IRQSTAT - Interrupt Status
 ***************************************************************************/
/* IRQ0_AON :: IRQSTAT :: reserved0 [31:28] */
#define BCHP_IRQ0_AON_IRQSTAT_reserved0_MASK                       0xf0000000
#define BCHP_IRQ0_AON_IRQSTAT_reserved0_SHIFT                      28

/* IRQ0_AON :: IRQSTAT :: iicdirq [27:27] */
#define BCHP_IRQ0_AON_IRQSTAT_iicdirq_MASK                         0x08000000
#define BCHP_IRQ0_AON_IRQSTAT_iicdirq_SHIFT                        27
#define BCHP_IRQ0_AON_IRQSTAT_iicdirq_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQSTAT :: reserved1 [26:21] */
#define BCHP_IRQ0_AON_IRQSTAT_reserved1_MASK                       0x07e00000
#define BCHP_IRQ0_AON_IRQSTAT_reserved1_SHIFT                      21

/* IRQ0_AON :: IRQSTAT :: spiirq [20:20] */
#define BCHP_IRQ0_AON_IRQSTAT_spiirq_MASK                          0x00100000
#define BCHP_IRQ0_AON_IRQSTAT_spiirq_SHIFT                         20
#define BCHP_IRQ0_AON_IRQSTAT_spiirq_DEFAULT                       0x00000000

/* IRQ0_AON :: IRQSTAT :: reserved2 [19:09] */
#define BCHP_IRQ0_AON_IRQSTAT_reserved2_MASK                       0x000ffe00
#define BCHP_IRQ0_AON_IRQSTAT_reserved2_SHIFT                      9

/* IRQ0_AON :: IRQSTAT :: kbd3irq [08:08] */
#define BCHP_IRQ0_AON_IRQSTAT_kbd3irq_MASK                         0x00000100
#define BCHP_IRQ0_AON_IRQSTAT_kbd3irq_SHIFT                        8
#define BCHP_IRQ0_AON_IRQSTAT_kbd3irq_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQSTAT :: icapirq [07:07] */
#define BCHP_IRQ0_AON_IRQSTAT_icapirq_MASK                         0x00000080
#define BCHP_IRQ0_AON_IRQSTAT_icapirq_SHIFT                        7
#define BCHP_IRQ0_AON_IRQSTAT_icapirq_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQSTAT :: gioirq [06:06] */
#define BCHP_IRQ0_AON_IRQSTAT_gioirq_MASK                          0x00000040
#define BCHP_IRQ0_AON_IRQSTAT_gioirq_SHIFT                         6
#define BCHP_IRQ0_AON_IRQSTAT_gioirq_DEFAULT                       0x00000000

/* IRQ0_AON :: IRQSTAT :: kbd2irq [05:05] */
#define BCHP_IRQ0_AON_IRQSTAT_kbd2irq_MASK                         0x00000020
#define BCHP_IRQ0_AON_IRQSTAT_kbd2irq_SHIFT                        5
#define BCHP_IRQ0_AON_IRQSTAT_kbd2irq_DEFAULT                      0x00000000

/* IRQ0_AON :: IRQSTAT :: reserved3 [04:02] */
#define BCHP_IRQ0_AON_IRQSTAT_reserved3_MASK                       0x0000001c
#define BCHP_IRQ0_AON_IRQSTAT_reserved3_SHIFT                      2

/* IRQ0_AON :: IRQSTAT :: ldkirq [01:01] */
#define BCHP_IRQ0_AON_IRQSTAT_ldkirq_MASK                          0x00000002
#define BCHP_IRQ0_AON_IRQSTAT_ldkirq_SHIFT                         1
#define BCHP_IRQ0_AON_IRQSTAT_ldkirq_DEFAULT                       0x00000000

/* IRQ0_AON :: IRQSTAT :: kbd1irq [00:00] */
#define BCHP_IRQ0_AON_IRQSTAT_kbd1irq_MASK                         0x00000001
#define BCHP_IRQ0_AON_IRQSTAT_kbd1irq_SHIFT                        0
#define BCHP_IRQ0_AON_IRQSTAT_kbd1irq_DEFAULT                      0x00000000

#endif /* #ifndef BCHP_IRQ0_AON_H__ */

/* End of File */
