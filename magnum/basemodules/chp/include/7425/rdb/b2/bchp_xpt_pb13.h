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
 * Date:           Generated on         Tue Jan 17 13:26:48 2012
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

#ifndef BCHP_XPT_PB13_H__
#define BCHP_XPT_PB13_H__

/***************************************************************************
 *XPT_PB13 - Playback 13 Control Registers
 ***************************************************************************/
#define BCHP_XPT_PB13_CTRL1                      0x00908780 /* Playback Control 1 Register */
#define BCHP_XPT_PB13_CTRL2                      0x00908784 /* Playback Control 2 Register */
#define BCHP_XPT_PB13_CTRL3                      0x00908788 /* Playback Control 3 Register */
#define BCHP_XPT_PB13_CTRL4                      0x0090878c /* Playback Control 4 Register */
#define BCHP_XPT_PB13_FIRST_DESC_ADDR            0x00908790 /* Playback First Descriptor Address Register */
#define BCHP_XPT_PB13_CURR_DESC_ADDR             0x00908794 /* Playback Current Descriptor Address Register */
#define BCHP_XPT_PB13_CURR_BUFF_ADDR             0x00908798 /* Playback Current Buffer Address Register */
#define BCHP_XPT_PB13_BLOCKOUT                   0x0090879c /* Data Transport Playback Block Out Control */
#define BCHP_XPT_PB13_PKTZ_CONTEXT0              0x009087a0 /* Data Transport Playback Packetize Mode Context 0 Control */
#define BCHP_XPT_PB13_TS_ERR_BOUND_EARLY         0x009087a4 /* Data Transport Playback Timestamp Error Bound Register */
#define BCHP_XPT_PB13_TS_ERR_BOUND_LATE          0x009087a8 /* Data Transport Playback Timestamp Error Bound Register */
#define BCHP_XPT_PB13_PARSER_CTRL1               0x009087ac /* Data Transport Playback Parser Control Register */
#define BCHP_XPT_PB13_PARSER_CTRL2               0x009087b0 /* Data Transport Playback Parser Control Register 2 */
#define BCHP_XPT_PB13_PARSER_TIMESTAMP           0x009087b4 /* Data Transport Playback Parser Local Timestamp */
#define BCHP_XPT_PB13_INTR                       0x009087b8 /* Playback Processing Error and Status Interrupt Register */
#define BCHP_XPT_PB13_INTR_EN                    0x009087bc /* Playback Processing Error and Status Interrupt Enable Register */
#define BCHP_XPT_PB13_INTR_TAGS                  0x009087c0 /* Playback Interrupt Tag Register */
#define BCHP_XPT_PB13_PACING_PCR_PID             0x009087c4 /* Playback PCR Based Pacing PCR Pid */
#define BCHP_XPT_PB13_PWR_CTRL                   0x009087c8 /* Playback Power Control Register */
#define BCHP_XPT_PB13_MISC_STATUS_REG            0x009087cc /* Miscellaneous Playback Status Registers */
#define BCHP_XPT_PB13_ASF_CTRL                   0x009087d0 /* ASF Control Register */
#define BCHP_XPT_PB13_ASF_DATA_PACKET_LENGTH     0x009087d4 /* ASF_DATA_PACKET_LENGTH */
#define BCHP_XPT_PB13_ASF_SUB_PAYLOAD_PKT_TYPE   0x009087d8 /* ASF_SUB_PAYLOAD_PKT_TYPE */
#define BCHP_XPT_PB13_ASF_PAYLOAD_PKT_TYPE       0x009087dc /* ASF_PAYLOAD_PKT_TYPE */
#define BCHP_XPT_PB13_PLAYBACK_PARSER_BAND_ID    0x009087e0 /* PLAYBACK_PARSER_BAND_ID */
#define BCHP_XPT_PB13_NEXT_PACKET_PACING_TIMESTAMP 0x009087e4 /* Next Packet Pacing Timestamp Value */
#define BCHP_XPT_PB13_PKT2PKT_PACING_TIMESTAMP_DELTA 0x009087e8 /* PKT2PKT Pacing TimeStamp Delta */
#define BCHP_XPT_PB13_PACING_COUNTER             0x009087ec /* Pacing Counter Instantaneous value */
#define BCHP_XPT_PB13_ERROR_VALUE_WATERMARK      0x009087f0 /* WaterMark level of the Pacing Error Bound Reached */
#define BCHP_XPT_PB13_PES_BASED_PACING_CTRL      0x009087f4 /* PES Based Pacing Controls */

#endif /* #ifndef BCHP_XPT_PB13_H__ */

/* End of File */
