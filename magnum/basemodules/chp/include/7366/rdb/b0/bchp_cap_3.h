/***************************************************************************
 *     Copyright (c) 1999-2013, Broadcom Corporation
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
 * Date:           Generated on              Mon Dec 23 13:19:21 2013
 *                 Full Compile MD5 Checksum e5d1378cc1475b750905e70cb70c73d9
 *                   (minus title and desc)  
 *                 MD5 Checksum              aa943f3142a624837db5321711723fcf
 *
 * Compiled with:  RDB Utility               combo_header.pl
 *                 RDB Parser                3.0
 *                 unknown                   unknown
 *                 Perl Interpreter          5.008008
 *                 Operating System          linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_CAP_3_H__
#define BCHP_CAP_3_H__

/***************************************************************************
 *CAP_3 - Capture 3 Registers
 ***************************************************************************/
#define BCHP_CAP_3_REVISION                      0x00680600 /* Capture Engine Revision ID */
#define BCHP_CAP_3_HW_CONFIGURATION              0x00680604 /* CAP Hardware Configuration Status */
#define BCHP_CAP_3_PIC_SIZE                      0x00680608 /* Capture Vertical and Horizontal Size */
#define BCHP_CAP_3_PIC_OFFSET                    0x0068060c /* Capture 2D or 3D Left View Vertical and Horizontal Start Offset */
#define BCHP_CAP_3_PIC_OFFSET_R                  0x00680610 /* Capture 3D Right View Horizontal Start Offset */
#define BCHP_CAP_3_BVB_IN_SIZE                   0x00680614 /* Capture BVB Source Vertical and Horizontal Size */
#define BCHP_CAP_3_MSTART                        0x00680618 /* Capture Memory Buffer Starting Address */
#define BCHP_CAP_3_MSTART_R                      0x0068061c /* Capture Memory Buffer Right Window Starting Address */
#define BCHP_CAP_3_PITCH                         0x00680620 /* Capture Memory Buffer Pitch */
#define BCHP_CAP_3_CTRL                          0x00680624 /* Capture Control */
#define BCHP_CAP_3_MODE                          0x00680628 /* Capture Mode Control */
#define BCHP_CAP_3_COMP_ORDER                    0x0068062c /* Capture Component Order Control */
#define BCHP_CAP_3_RX_CTRL                       0x00680630 /* Capture BVB Receiver Control */
#define BCHP_CAP_3_TRIG_CTRL                     0x00680634 /* Capture RDMA Trigger Control */
#define BCHP_CAP_3_BVB_TRIG_0_CFG                0x00680638 /* Capture BVB Trigger 0 Configuration */
#define BCHP_CAP_3_BVB_TRIG_1_CFG                0x0068063c /* Capture BVB Trigger 1 Configuration */
#define BCHP_CAP_3_STATUS                        0x006806ec /* Capture Engine Status */
#define BCHP_CAP_3_BVB_STATUS                    0x006806f0 /* Capture BVB Status */
#define BCHP_CAP_3_BVB_STATUS_CLEAR              0x006806f4 /* Capture BVB Status Clear */
#define BCHP_CAP_3_FIFO_STATUS                   0x006806f8 /* Capture FIFO Status */
#define BCHP_CAP_3_FIFO_STATUS_CLEAR             0x006806fc /* Capture FIFO Status Clear */
#define BCHP_CAP_3_DEBUG_STATUS                  0x00680700 /* Capture Debug Status Register */
#define BCHP_CAP_3_SCRATCH                       0x0068070c /* Scratch Register */

#endif /* #ifndef BCHP_CAP_3_H__ */

/* End of File */
