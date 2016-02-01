/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_3510_H__
#define BCHP_3510_H__

/* BCM3510 registers */
#define BCM3510_HCTL1   0xA0
#define BCM3510_APSTAT1 0xA2
#define BCM3510_APMSK1  0xA3
#define BCM3510_APSTAT2 0xA4
#define BCM3510_APMSK2  0xA5
#define BCM3510_HABADR  0xA6
#define BCM3510_HABDATA 0xA7
#define BCM3510_HABSTAT 0xA8
#define BCM3510_MADRH   0xA9
#define BCM3510_MADRL   0xAA
#define BCM3510_MDATA   0xAB
#define BCM3510_REVID   0xE0
#define BCM3510_JDEC    0xCA
#define BCM3510_TSTCTL  0x2E

/* HCTL1 */
#define BCM3510_HCTL1_CLEAR_RESET 0x00
#define BCM3510_HCTL1_RESET       0x01
#define BCM3510_HCTL1_IDLE        0x02
#define BCM3510_HCTL1_STOP        0x04
#define BCM3510_HCTL1_CLEAR_IDLE  0x20
#define BCM3510_HCTL1_SSIN        0x80

/* APSTAT1/APMSK1 */
#define BCM3510_APSTAT1_RESET  0x01
#define BCM3510_APSTAT1_IDLE   0x02
#define BCM3510_APSTAT1_STOP   0x04
#define BCM3510_APSTAT1_RUN    0x08
#define BCM3510_APSTAT1_HABAV  0x10
#define BCM3510_APSTAT1_MEMAV  0x20
#define BCM3510_APSTAT1_ALDONE 0x40
#define BCM3510_APSTAT1_REIRQ  0x80

/* APSTAT2/APMSK2 */
#define BCM3510_APSTAT2_HABERR    0x01
#define BCM3510_APSTAT2_SCRERR    0x02
#define BCM3510_APSTAT2_NOTLOCK   0x10
#define BCM3510_APSTAT2_LOCK      0x20
#define BCM3510_APSTAT2_INIT_DONE 0x80

/* HABSTAT */
#define BCM3510_HABSTAT_HABR     0x01
#define BCM3510_HABSTAT_LDHABR   0x02
#define BCM3510_HABSTAT_SET_HABR 0x03
#define BCM3510_HABSTAT_MCMSK    0x04
#define BCM3510_HABSTAT_HMSK     0x08
#define BCM3510_HABSTAT_LDMSK    0x10

#endif /* #ifndef BCHP_3510_H__ */

