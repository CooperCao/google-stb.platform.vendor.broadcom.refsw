/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef BCHP_LEAP_CTRL_H__
#define BCHP_LEAP_CTRL_H__

/***************************************************************************
 *LEAP_CTRL - Control registers
 ***************************************************************************/
#define BCHP_LEAP_CTRL_REVID                     0x00100000 /* [RO][32] LEAP Revision ID */
#define BCHP_LEAP_CTRL_CTRL                      0x00100004 /* [RW][32] Main Control Register */
#define BCHP_LEAP_CTRL_CPU_STRAPS                0x00100008 /* [RO][32] CPU Straps Register */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0           0x00100014 /* [RO][32] RBUS Broadcast Acknowledge for 2 cores */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1           0x00100018 /* [RO][32] RBUS Broadcast Acknowledge for 8 cores */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK2           0x0010001c /* [RO][32] RBUS Broadcast Acknowledge for 16 cores */
#define BCHP_LEAP_CTRL_SW_SPARE0                 0x00100070 /* [RW][32] Software Spare Register 0 */
#define BCHP_LEAP_CTRL_SW_SPARE1                 0x00100074 /* [RW][32] Software Spare Register 1 */
#define BCHP_LEAP_CTRL_SW_SPARE2                 0x00100078 /* [RW][32] Software Spare Register 2 */
#define BCHP_LEAP_CTRL_SW_SPARE3                 0x0010007c /* [RW][32] Software Spare Register 3 */
#define BCHP_LEAP_CTRL_RBUS_ERR_ADDR             0x00100080 /* [RO][32] RBUS Error Address */
#define BCHP_LEAP_CTRL_RBUS_ERR_DATA             0x00100084 /* [RO][32] RBUS Error Write Data */
#define BCHP_LEAP_CTRL_RBUS_ERR_XAC              0x00100088 /* [RO][32] RBUS Error Transaction */
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL             0x0010008c /* [RW][32] RBUS Error Control */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN              0x001000a8 /* [RW][32] Byte Swap Enable for UBUS clients */
#define BCHP_LEAP_CTRL_ADDR_TRANS                0x001000ac /* [RW][32] Address Translation window */
#define BCHP_LEAP_CTRL_GP0                       0x00100100 /* [RW][32] CPU General Purpose Register 0 */
#define BCHP_LEAP_CTRL_GP1                       0x00100104 /* [RW][32] CPU General Purpose Register 1 */
#define BCHP_LEAP_CTRL_GP2                       0x00100108 /* [RW][32] CPU General Purpose Register 2 */
#define BCHP_LEAP_CTRL_GP3                       0x0010010c /* [RW][32] CPU General Purpose Register 3 */
#define BCHP_LEAP_CTRL_GP4                       0x00100110 /* [RW][32] CPU General Purpose Register 4 */
#define BCHP_LEAP_CTRL_GP5                       0x00100114 /* [RW][32] CPU General Purpose Register 5 */
#define BCHP_LEAP_CTRL_GP6                       0x00100118 /* [RW][32] CPU General Purpose Register 6 */
#define BCHP_LEAP_CTRL_GP7                       0x0010011c /* [RW][32] CPU General Purpose Register 7 */
#define BCHP_LEAP_CTRL_GP8                       0x00100120 /* [RW][32] CPU General Purpose Register 8 */
#define BCHP_LEAP_CTRL_GP9                       0x00100124 /* [RW][32] CPU General Purpose Register 9 */
#define BCHP_LEAP_CTRL_GP10                      0x00100128 /* [RW][32] CPU General Purpose Register 10 */
#define BCHP_LEAP_CTRL_GP11                      0x0010012c /* [RW][32] CPU General Purpose Register 11 */
#define BCHP_LEAP_CTRL_GP12                      0x00100130 /* [RW][32] CPU General Purpose Register 12 */
#define BCHP_LEAP_CTRL_GP13                      0x00100134 /* [RW][32] CPU General Purpose Register 13 */
#define BCHP_LEAP_CTRL_GP14                      0x00100138 /* [RW][32] CPU General Purpose Register 14 */
#define BCHP_LEAP_CTRL_GP15                      0x0010013c /* [RW][32] CPU General Purpose Register 15 */
#define BCHP_LEAP_CTRL_GP16                      0x00100140 /* [RW][32] CPU General Purpose Register 16 */
#define BCHP_LEAP_CTRL_GP17                      0x00100144 /* [RW][32] CPU General Purpose Register 17 */
#define BCHP_LEAP_CTRL_GP18                      0x00100148 /* [RW][32] CPU General Purpose Register 18 */
#define BCHP_LEAP_CTRL_GP19                      0x0010014c /* [RW][32] CPU General Purpose Register 19 */
#define BCHP_LEAP_CTRL_GP20                      0x00100150 /* [RW][32] CPU General Purpose Register 20 */
#define BCHP_LEAP_CTRL_GP21                      0x00100154 /* [RW][32] CPU General Purpose Register 21 */
#define BCHP_LEAP_CTRL_GP22                      0x00100158 /* [RW][32] CPU General Purpose Register 22 */
#define BCHP_LEAP_CTRL_GP23                      0x0010015c /* [RW][32] CPU General Purpose Register 23 */
#define BCHP_LEAP_CTRL_GP24                      0x00100160 /* [RW][32] CPU General Purpose Register 24 */
#define BCHP_LEAP_CTRL_GP25                      0x00100164 /* [RW][32] CPU General Purpose Register 25 */
#define BCHP_LEAP_CTRL_GP26                      0x00100168 /* [RW][32] CPU General Purpose Register 26 */
#define BCHP_LEAP_CTRL_GP27                      0x0010016c /* [RW][32] CPU General Purpose Register 27 */
#define BCHP_LEAP_CTRL_GP28                      0x00100170 /* [RW][32] CPU General Purpose Register 28 */
#define BCHP_LEAP_CTRL_GP29                      0x00100174 /* [RW][32] CPU General Purpose Register 29 */
#define BCHP_LEAP_CTRL_GP30                      0x00100178 /* [RW][32] CPU General Purpose Register 30 */
#define BCHP_LEAP_CTRL_GP31                      0x0010017c /* [RW][32] CPU General Purpose Register 31 */
#define BCHP_LEAP_CTRL_GP32                      0x00100180 /* [RW][32] CPU General Purpose Register 32 */
#define BCHP_LEAP_CTRL_GP33                      0x00100184 /* [RW][32] CPU General Purpose Register 33 */
#define BCHP_LEAP_CTRL_GP34                      0x00100188 /* [RW][32] CPU General Purpose Register 34 */
#define BCHP_LEAP_CTRL_GP35                      0x0010018c /* [RW][32] CPU General Purpose Register 35 */
#define BCHP_LEAP_CTRL_GP36                      0x00100190 /* [RW][32] CPU General Purpose Register 36 */
#define BCHP_LEAP_CTRL_GP37                      0x00100194 /* [RW][32] CPU General Purpose Register 37 */
#define BCHP_LEAP_CTRL_GP38                      0x00100198 /* [RW][32] CPU General Purpose Register 38 */
#define BCHP_LEAP_CTRL_GP39                      0x0010019c /* [RW][32] CPU General Purpose Register 39 */
#define BCHP_LEAP_CTRL_GP40                      0x001001a0 /* [RW][32] CPU General Purpose Register 40 */
#define BCHP_LEAP_CTRL_GP41                      0x001001a4 /* [RW][32] CPU General Purpose Register 41 */
#define BCHP_LEAP_CTRL_GP42                      0x001001a8 /* [RW][32] CPU General Purpose Register 42 */
#define BCHP_LEAP_CTRL_GP43                      0x001001ac /* [RW][32] CPU General Purpose Register 43 */
#define BCHP_LEAP_CTRL_GP44                      0x001001b0 /* [RW][32] CPU General Purpose Register 44 */
#define BCHP_LEAP_CTRL_GP45                      0x001001b4 /* [RW][32] CPU General Purpose Register 45 */
#define BCHP_LEAP_CTRL_GP46                      0x001001b8 /* [RW][32] CPU General Purpose Register 46 */
#define BCHP_LEAP_CTRL_GP47                      0x001001bc /* [RW][32] CPU General Purpose Register 47 */
#define BCHP_LEAP_CTRL_GP48                      0x001001c0 /* [RW][32] CPU General Purpose Register 48 */
#define BCHP_LEAP_CTRL_GP49                      0x001001c4 /* [RW][32] CPU General Purpose Register 49 */
#define BCHP_LEAP_CTRL_GP50                      0x001001c8 /* [RW][32] CPU General Purpose Register 50 */
#define BCHP_LEAP_CTRL_GP51                      0x001001cc /* [RW][32] CPU General Purpose Register 51 */
#define BCHP_LEAP_CTRL_GP52                      0x001001d0 /* [RW][32] CPU General Purpose Register 52 */
#define BCHP_LEAP_CTRL_GP53                      0x001001d4 /* [RW][32] CPU General Purpose Register 53 */
#define BCHP_LEAP_CTRL_GP54                      0x001001d8 /* [RW][32] CPU General Purpose Register 54 */
#define BCHP_LEAP_CTRL_GP55                      0x001001dc /* [RW][32] CPU General Purpose Register 55 */
#define BCHP_LEAP_CTRL_GP56                      0x001001e0 /* [RW][32] CPU General Purpose Register 56 */
#define BCHP_LEAP_CTRL_GP57                      0x001001e4 /* [RW][32] CPU General Purpose Register 57 */
#define BCHP_LEAP_CTRL_GP58                      0x001001e8 /* [RW][32] CPU General Purpose Register 58 */
#define BCHP_LEAP_CTRL_GP59                      0x001001ec /* [RW][32] CPU General Purpose Register 59 */
#define BCHP_LEAP_CTRL_GP60                      0x001001f0 /* [RW][32] CPU General Purpose Register 60 */
#define BCHP_LEAP_CTRL_GP61                      0x001001f4 /* [RW][32] CPU General Purpose Register 61 */
#define BCHP_LEAP_CTRL_GP62                      0x001001f8 /* [RW][32] CPU General Purpose Register 62 */
#define BCHP_LEAP_CTRL_GP63                      0x001001fc /* [RW][32] CPU General Purpose Register 63 */
#define BCHP_LEAP_CTRL_GP64                      0x00100200 /* [RW][32] CPU General Purpose Register 64 */
#define BCHP_LEAP_CTRL_GP65                      0x00100204 /* [RW][32] CPU General Purpose Register 65 */
#define BCHP_LEAP_CTRL_GP66                      0x00100208 /* [RW][32] CPU General Purpose Register 66 */
#define BCHP_LEAP_CTRL_GP67                      0x0010020c /* [RW][32] CPU General Purpose Register 67 */
#define BCHP_LEAP_CTRL_GP68                      0x00100210 /* [RW][32] CPU General Purpose Register 68 */
#define BCHP_LEAP_CTRL_GP69                      0x00100214 /* [RW][32] CPU General Purpose Register 69 */
#define BCHP_LEAP_CTRL_GP70                      0x00100218 /* [RW][32] CPU General Purpose Register 70 */
#define BCHP_LEAP_CTRL_GP71                      0x0010021c /* [RW][32] CPU General Purpose Register 71 */
#define BCHP_LEAP_CTRL_GP72                      0x00100220 /* [RW][32] CPU General Purpose Register 72 */
#define BCHP_LEAP_CTRL_GP73                      0x00100224 /* [RW][32] CPU General Purpose Register 73 */
#define BCHP_LEAP_CTRL_GP74                      0x00100228 /* [RW][32] CPU General Purpose Register 74 */
#define BCHP_LEAP_CTRL_GP75                      0x0010022c /* [RW][32] CPU General Purpose Register 75 */
#define BCHP_LEAP_CTRL_GP76                      0x00100230 /* [RW][32] CPU General Purpose Register 76 */
#define BCHP_LEAP_CTRL_GP77                      0x00100234 /* [RW][32] CPU General Purpose Register 77 */
#define BCHP_LEAP_CTRL_GP78                      0x00100238 /* [RW][32] CPU General Purpose Register 78 */
#define BCHP_LEAP_CTRL_GP79                      0x0010023c /* [RW][32] CPU General Purpose Register 79 */
#define BCHP_LEAP_CTRL_GP80                      0x00100240 /* [RW][32] CPU General Purpose Register 80 */
#define BCHP_LEAP_CTRL_GP81                      0x00100244 /* [RW][32] CPU General Purpose Register 81 */
#define BCHP_LEAP_CTRL_GP82                      0x00100248 /* [RW][32] CPU General Purpose Register 82 */
#define BCHP_LEAP_CTRL_GP83                      0x0010024c /* [RW][32] CPU General Purpose Register 83 */
#define BCHP_LEAP_CTRL_GP84                      0x00100250 /* [RW][32] CPU General Purpose Register 84 */
#define BCHP_LEAP_CTRL_GP85                      0x00100254 /* [RW][32] CPU General Purpose Register 85 */
#define BCHP_LEAP_CTRL_GP86                      0x00100258 /* [RW][32] CPU General Purpose Register 86 */
#define BCHP_LEAP_CTRL_GP87                      0x0010025c /* [RW][32] CPU General Purpose Register 87 */
#define BCHP_LEAP_CTRL_GP88                      0x00100260 /* [RW][32] CPU General Purpose Register 88 */
#define BCHP_LEAP_CTRL_GP89                      0x00100264 /* [RW][32] CPU General Purpose Register 89 */
#define BCHP_LEAP_CTRL_GP90                      0x00100268 /* [RW][32] CPU General Purpose Register 90 */
#define BCHP_LEAP_CTRL_GP91                      0x0010026c /* [RW][32] CPU General Purpose Register 91 */
#define BCHP_LEAP_CTRL_GP92                      0x00100270 /* [RW][32] CPU General Purpose Register 92 */
#define BCHP_LEAP_CTRL_GP93                      0x00100274 /* [RW][32] CPU General Purpose Register 93 */
#define BCHP_LEAP_CTRL_GP94                      0x00100278 /* [RW][32] CPU General Purpose Register 94 */
#define BCHP_LEAP_CTRL_GP95                      0x0010027c /* [RW][32] CPU General Purpose Register 95 */
#define BCHP_LEAP_CTRL_GP96                      0x00100280 /* [RW][32] CPU General Purpose Register 96 */
#define BCHP_LEAP_CTRL_GP97                      0x00100284 /* [RW][32] CPU General Purpose Register 97 */
#define BCHP_LEAP_CTRL_GP98                      0x00100288 /* [RW][32] CPU General Purpose Register 98 */
#define BCHP_LEAP_CTRL_GP99                      0x0010028c /* [RW][32] CPU General Purpose Register 99 */
#define BCHP_LEAP_CTRL_GP100                     0x00100290 /* [RW][32] CPU General Purpose Register 100 */
#define BCHP_LEAP_CTRL_GP101                     0x00100294 /* [RW][32] CPU General Purpose Register 101 */
#define BCHP_LEAP_CTRL_GP102                     0x00100298 /* [RW][32] CPU General Purpose Register 102 */
#define BCHP_LEAP_CTRL_GP103                     0x0010029c /* [RW][32] CPU General Purpose Register 103 */
#define BCHP_LEAP_CTRL_GP104                     0x001002a0 /* [RW][32] CPU General Purpose Register 104 */
#define BCHP_LEAP_CTRL_GP105                     0x001002a4 /* [RW][32] CPU General Purpose Register 105 */
#define BCHP_LEAP_CTRL_GP106                     0x001002a8 /* [RW][32] CPU General Purpose Register 106 */
#define BCHP_LEAP_CTRL_GP107                     0x001002ac /* [RW][32] CPU General Purpose Register 107 */
#define BCHP_LEAP_CTRL_GP108                     0x001002b0 /* [RW][32] CPU General Purpose Register 108 */
#define BCHP_LEAP_CTRL_GP109                     0x001002b4 /* [RW][32] CPU General Purpose Register 109 */
#define BCHP_LEAP_CTRL_GP110                     0x001002b8 /* [RW][32] CPU General Purpose Register 110 */
#define BCHP_LEAP_CTRL_GP111                     0x001002bc /* [RW][32] CPU General Purpose Register 111 */
#define BCHP_LEAP_CTRL_GP112                     0x001002c0 /* [RW][32] CPU General Purpose Register 112 */
#define BCHP_LEAP_CTRL_GP113                     0x001002c4 /* [RW][32] CPU General Purpose Register 113 */
#define BCHP_LEAP_CTRL_GP114                     0x001002c8 /* [RW][32] CPU General Purpose Register 114 */
#define BCHP_LEAP_CTRL_GP115                     0x001002cc /* [RW][32] CPU General Purpose Register 115 */
#define BCHP_LEAP_CTRL_GP116                     0x001002d0 /* [RW][32] CPU General Purpose Register 116 */
#define BCHP_LEAP_CTRL_GP117                     0x001002d4 /* [RW][32] CPU General Purpose Register 117 */
#define BCHP_LEAP_CTRL_GP118                     0x001002d8 /* [RW][32] CPU General Purpose Register 118 */
#define BCHP_LEAP_CTRL_GP119                     0x001002dc /* [RW][32] CPU General Purpose Register 119 */
#define BCHP_LEAP_CTRL_GP120                     0x001002e0 /* [RW][32] CPU General Purpose Register 120 */
#define BCHP_LEAP_CTRL_GP121                     0x001002e4 /* [RW][32] CPU General Purpose Register 121 */
#define BCHP_LEAP_CTRL_GP122                     0x001002e8 /* [RW][32] CPU General Purpose Register 122 */
#define BCHP_LEAP_CTRL_GP123                     0x001002ec /* [RW][32] CPU General Purpose Register 123 */
#define BCHP_LEAP_CTRL_GP124                     0x001002f0 /* [RW][32] CPU General Purpose Register 124 */
#define BCHP_LEAP_CTRL_GP125                     0x001002f4 /* [RW][32] CPU General Purpose Register 125 */
#define BCHP_LEAP_CTRL_GP126                     0x001002f8 /* [RW][32] CPU General Purpose Register 126 */
#define BCHP_LEAP_CTRL_GP127                     0x001002fc /* [RW][32] CPU General Purpose Register 127 */
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR0         0x00100300 /* [RW][32] Block Range Start Address 0 */
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR1         0x00100304 /* [RW][32] Block Range Start Address 1 */
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR0           0x00100308 /* [RW][32] Block Range End Address 0 */
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR1           0x0010030c /* [RW][32] Block Range End Address 1 */
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL          0x00100310 /* [RW][32] Block Range Control */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL             0x00100320 /* [RW][32] Diagnostic Capture DMA Control Register */
#define BCHP_LEAP_CTRL_DIAG_DEST_ADDR            0x00100324 /* [RW][32] Diagnostic Capture Destination Address */
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE            0x00100328 /* [RW][32] Diagnostic Capture Data Transfer Size */
#define BCHP_LEAP_CTRL_DIAG_POST_TRIG_NUM        0x0010032c /* [RW][32] Number of Samples to Capture Post Trigger */
#define BCHP_LEAP_CTRL_DIAG_STATUS               0x00100330 /* [RO][32] DIAG Status Register */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_START  0x00100334 /* [RO][32] DIAG MEM START Address of Captured Block */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_END    0x00100338 /* [RO][32] DIAG MEM END Address of Captured Block */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_TRIGGER 0x0010033c /* [RO][32] DIAG MEM Address of Trigger Sample */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_START  0x00100340 /* [RO][32] DIAG DDR START Address of Captured Block */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_END    0x00100344 /* [RO][32] DIAG DDR END Address of Captured Block */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_TRIGGER 0x00100348 /* [RO][32] DIAG DDR Address of Trigger Sample */
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_INT_MEM    0x0010034c /* [RW][32] Diagnostic Capture Data Transfer Size */
#define BCHP_LEAP_CTRL_MTSIF_HEADER              0x00100350 /* [RW][32] DIAG Status Register */
#define BCHP_LEAP_CTRL_MTSIF_CTRL                0x00100354 /* [RW][32] DIAG Status Register */
#define BCHP_LEAP_CTRL_MTSIF_STATUS              0x00100358 /* [RO][32] DIAG Status Register */
#define BCHP_LEAP_CTRL_MTSIF_MEM_START_ADDR      0x0010035c /* [RW][32] DIAG Status Register */

/***************************************************************************
 *REVID - LEAP Revision ID
 ***************************************************************************/
/* LEAP_CTRL :: REVID :: reserved0 [31:16] */
#define BCHP_LEAP_CTRL_REVID_reserved0_MASK                        0xffff0000
#define BCHP_LEAP_CTRL_REVID_reserved0_SHIFT                       16

/* LEAP_CTRL :: REVID :: MAJOR [15:08] */
#define BCHP_LEAP_CTRL_REVID_MAJOR_MASK                            0x0000ff00
#define BCHP_LEAP_CTRL_REVID_MAJOR_SHIFT                           8

/* LEAP_CTRL :: REVID :: MINOR [07:00] */
#define BCHP_LEAP_CTRL_REVID_MINOR_MASK                            0x000000ff
#define BCHP_LEAP_CTRL_REVID_MINOR_SHIFT                           0

/***************************************************************************
 *CTRL - Main Control Register
 ***************************************************************************/
/* LEAP_CTRL :: CTRL :: MAESTRO_SEL [31:31] */
#define BCHP_LEAP_CTRL_CTRL_MAESTRO_SEL_MASK                       0x80000000
#define BCHP_LEAP_CTRL_CTRL_MAESTRO_SEL_SHIFT                      31
#define BCHP_LEAP_CTRL_CTRL_MAESTRO_SEL_DEFAULT                    0x00000000

/* LEAP_CTRL :: CTRL :: reserved0 [30:26] */
#define BCHP_LEAP_CTRL_CTRL_reserved0_MASK                         0x7c000000
#define BCHP_LEAP_CTRL_CTRL_reserved0_SHIFT                        26

/* LEAP_CTRL :: CTRL :: HOSTIF_SEL [25:25] */
#define BCHP_LEAP_CTRL_CTRL_HOSTIF_SEL_MASK                        0x02000000
#define BCHP_LEAP_CTRL_CTRL_HOSTIF_SEL_SHIFT                       25
#define BCHP_LEAP_CTRL_CTRL_HOSTIF_SEL_DEFAULT                     0x00000000

/* LEAP_CTRL :: CTRL :: START_ARC [24:24] */
#define BCHP_LEAP_CTRL_CTRL_START_ARC_MASK                         0x01000000
#define BCHP_LEAP_CTRL_CTRL_START_ARC_SHIFT                        24
#define BCHP_LEAP_CTRL_CTRL_START_ARC_DEFAULT                      0x00000000

/* LEAP_CTRL :: CTRL :: reserved1 [23:22] */
#define BCHP_LEAP_CTRL_CTRL_reserved1_MASK                         0x00c00000
#define BCHP_LEAP_CTRL_CTRL_reserved1_SHIFT                        22

/* LEAP_CTRL :: CTRL :: RBUS_BCAST_GRP2_0_WR_EN [21:21] */
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_0_WR_EN_MASK           0x00200000
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_0_WR_EN_SHIFT          21
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_0_WR_EN_DEFAULT        0x00000000

/* LEAP_CTRL :: CTRL :: RBUS_BCAST_GRP2_1_WR_EN [20:20] */
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_1_WR_EN_MASK           0x00100000
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_1_WR_EN_SHIFT          20
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_1_WR_EN_DEFAULT        0x00000000

/* LEAP_CTRL :: CTRL :: RBUS_BCAST_GRP2_2_WR_EN [19:19] */
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_2_WR_EN_MASK           0x00080000
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_2_WR_EN_SHIFT          19
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_2_WR_EN_DEFAULT        0x00000000

/* LEAP_CTRL :: CTRL :: RBUS_BCAST_GRP2_3_WR_EN [18:18] */
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_3_WR_EN_MASK           0x00040000
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_3_WR_EN_SHIFT          18
#define BCHP_LEAP_CTRL_CTRL_RBUS_BCAST_GRP2_3_WR_EN_DEFAULT        0x00000000

/* LEAP_CTRL :: CTRL :: reserved2 [17:16] */
#define BCHP_LEAP_CTRL_CTRL_reserved2_MASK                         0x00030000
#define BCHP_LEAP_CTRL_CTRL_reserved2_SHIFT                        16

/* LEAP_CTRL :: CTRL :: RBUS_TIMEOUT_SEL [15:12] */
#define BCHP_LEAP_CTRL_CTRL_RBUS_TIMEOUT_SEL_MASK                  0x0000f000
#define BCHP_LEAP_CTRL_CTRL_RBUS_TIMEOUT_SEL_SHIFT                 12
#define BCHP_LEAP_CTRL_CTRL_RBUS_TIMEOUT_SEL_DEFAULT               0x0000000c

/* LEAP_CTRL :: CTRL :: WDOG_EN [11:11] */
#define BCHP_LEAP_CTRL_CTRL_WDOG_EN_MASK                           0x00000800
#define BCHP_LEAP_CTRL_CTRL_WDOG_EN_SHIFT                          11
#define BCHP_LEAP_CTRL_CTRL_WDOG_EN_DEFAULT                        0x00000000

/* LEAP_CTRL :: CTRL :: reserved3 [10:10] */
#define BCHP_LEAP_CTRL_CTRL_reserved3_MASK                         0x00000400
#define BCHP_LEAP_CTRL_CTRL_reserved3_SHIFT                        10

/* LEAP_CTRL :: CTRL :: SPARE_STRAP_OVERRIDE_EN [09:09] */
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_EN_MASK           0x00000200
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_EN_SHIFT          9
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_EN_DEFAULT        0x00000000

/* LEAP_CTRL :: CTRL :: SPARE_STRAP_OVERRIDE [08:07] */
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_MASK              0x00000180
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_SHIFT             7
#define BCHP_LEAP_CTRL_CTRL_SPARE_STRAP_OVERRIDE_DEFAULT           0x00000000

/* LEAP_CTRL :: CTRL :: BOOT_STRAP_OVERRIDE_EN [06:06] */
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_EN_MASK            0x00000040
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_EN_SHIFT           6
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_EN_DEFAULT         0x00000000

/* LEAP_CTRL :: CTRL :: BOOT_STRAP_OVERRIDE [05:04] */
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_MASK               0x00000030
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_SHIFT              4
#define BCHP_LEAP_CTRL_CTRL_BOOT_STRAP_OVERRIDE_DEFAULT            0x00000000

/* LEAP_CTRL :: CTRL :: UART_RST [03:03] */
#define BCHP_LEAP_CTRL_CTRL_UART_RST_MASK                          0x00000008
#define BCHP_LEAP_CTRL_CTRL_UART_RST_SHIFT                         3
#define BCHP_LEAP_CTRL_CTRL_UART_RST_DEFAULT                       0x00000000

/* LEAP_CTRL :: CTRL :: reserved4 [02:02] */
#define BCHP_LEAP_CTRL_CTRL_reserved4_MASK                         0x00000004
#define BCHP_LEAP_CTRL_CTRL_reserved4_SHIFT                        2

/* LEAP_CTRL :: CTRL :: CPU_RST [01:01] */
#define BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK                           0x00000002
#define BCHP_LEAP_CTRL_CTRL_CPU_RST_SHIFT                          1
#define BCHP_LEAP_CTRL_CTRL_CPU_RST_DEFAULT                        0x00000001

/* LEAP_CTRL :: CTRL :: LEAP_RST [00:00] */
#define BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK                          0x00000001
#define BCHP_LEAP_CTRL_CTRL_LEAP_RST_SHIFT                         0
#define BCHP_LEAP_CTRL_CTRL_LEAP_RST_DEFAULT                       0x00000000

/***************************************************************************
 *CPU_STRAPS - CPU Straps Register
 ***************************************************************************/
/* LEAP_CTRL :: CPU_STRAPS :: reserved0 [31:04] */
#define BCHP_LEAP_CTRL_CPU_STRAPS_reserved0_MASK                   0xfffffff0
#define BCHP_LEAP_CTRL_CPU_STRAPS_reserved0_SHIFT                  4

/* LEAP_CTRL :: CPU_STRAPS :: SPARE [03:02] */
#define BCHP_LEAP_CTRL_CPU_STRAPS_SPARE_MASK                       0x0000000c
#define BCHP_LEAP_CTRL_CPU_STRAPS_SPARE_SHIFT                      2

/* LEAP_CTRL :: CPU_STRAPS :: BOOT_MODE [01:00] */
#define BCHP_LEAP_CTRL_CPU_STRAPS_BOOT_MODE_MASK                   0x00000003
#define BCHP_LEAP_CTRL_CPU_STRAPS_BOOT_MODE_SHIFT                  0

/***************************************************************************
 *RBUS_BCAST_ACK0 - RBUS Broadcast Acknowledge for 2 cores
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_BCAST_ACK0 :: reserved0 [31:08] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_reserved0_MASK              0xffffff00
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_reserved0_SHIFT             8

/* LEAP_CTRL :: RBUS_BCAST_ACK0 :: GRP2_3_ACK [07:06] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_3_ACK_MASK             0x000000c0
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_3_ACK_SHIFT            6

/* LEAP_CTRL :: RBUS_BCAST_ACK0 :: GRP2_2_ACK [05:04] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_2_ACK_MASK             0x00000030
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_2_ACK_SHIFT            4

/* LEAP_CTRL :: RBUS_BCAST_ACK0 :: GRP2_1_ACK [03:02] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_1_ACK_MASK             0x0000000c
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_1_ACK_SHIFT            2

/* LEAP_CTRL :: RBUS_BCAST_ACK0 :: GRP2_0_ACK [01:00] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_0_ACK_MASK             0x00000003
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK0_GRP2_0_ACK_SHIFT            0

/***************************************************************************
 *RBUS_BCAST_ACK1 - RBUS Broadcast Acknowledge for 8 cores
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_BCAST_ACK1 :: GRP8_2_ACK [31:16] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_2_ACK_MASK             0xffff0000
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_2_ACK_SHIFT            16

/* LEAP_CTRL :: RBUS_BCAST_ACK1 :: GRP8_1_ACK [15:08] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_1_ACK_MASK             0x0000ff00
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_1_ACK_SHIFT            8

/* LEAP_CTRL :: RBUS_BCAST_ACK1 :: GRP8_0_ACK [07:00] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_0_ACK_MASK             0x000000ff
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK1_GRP8_0_ACK_SHIFT            0

/***************************************************************************
 *RBUS_BCAST_ACK2 - RBUS Broadcast Acknowledge for 16 cores
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_BCAST_ACK2 :: reserved0 [31:16] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK2_reserved0_MASK              0xffff0000
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK2_reserved0_SHIFT             16

/* LEAP_CTRL :: RBUS_BCAST_ACK2 :: GRP16_0_ACK [15:00] */
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK2_GRP16_0_ACK_MASK            0x0000ffff
#define BCHP_LEAP_CTRL_RBUS_BCAST_ACK2_GRP16_0_ACK_SHIFT           0

/***************************************************************************
 *SW_SPARE0 - Software Spare Register 0
 ***************************************************************************/
/* LEAP_CTRL :: SW_SPARE0 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_SW_SPARE0_SPARE_MASK                        0xffffffff
#define BCHP_LEAP_CTRL_SW_SPARE0_SPARE_SHIFT                       0
#define BCHP_LEAP_CTRL_SW_SPARE0_SPARE_DEFAULT                     0x00000000

/***************************************************************************
 *SW_SPARE1 - Software Spare Register 1
 ***************************************************************************/
/* LEAP_CTRL :: SW_SPARE1 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_SW_SPARE1_SPARE_MASK                        0xffffffff
#define BCHP_LEAP_CTRL_SW_SPARE1_SPARE_SHIFT                       0
#define BCHP_LEAP_CTRL_SW_SPARE1_SPARE_DEFAULT                     0x00000000

/***************************************************************************
 *SW_SPARE2 - Software Spare Register 2
 ***************************************************************************/
/* LEAP_CTRL :: SW_SPARE2 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_SW_SPARE2_SPARE_MASK                        0xffffffff
#define BCHP_LEAP_CTRL_SW_SPARE2_SPARE_SHIFT                       0
#define BCHP_LEAP_CTRL_SW_SPARE2_SPARE_DEFAULT                     0x00000000

/***************************************************************************
 *SW_SPARE3 - Software Spare Register 3
 ***************************************************************************/
/* LEAP_CTRL :: SW_SPARE3 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_SW_SPARE3_SPARE_MASK                        0xffffffff
#define BCHP_LEAP_CTRL_SW_SPARE3_SPARE_SHIFT                       0
#define BCHP_LEAP_CTRL_SW_SPARE3_SPARE_DEFAULT                     0x00000000

/***************************************************************************
 *RBUS_ERR_ADDR - RBUS Error Address
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_ERR_ADDR :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_RBUS_ERR_ADDR_ADDR_MASK                     0xffffffff
#define BCHP_LEAP_CTRL_RBUS_ERR_ADDR_ADDR_SHIFT                    0

/***************************************************************************
 *RBUS_ERR_DATA - RBUS Error Write Data
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_ERR_DATA :: DATA [31:00] */
#define BCHP_LEAP_CTRL_RBUS_ERR_DATA_DATA_MASK                     0xffffffff
#define BCHP_LEAP_CTRL_RBUS_ERR_DATA_DATA_SHIFT                    0

/***************************************************************************
 *RBUS_ERR_XAC - RBUS Error Transaction
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_ERR_XAC :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_RBUS_ERR_XAC_reserved0_MASK                 0xfffffffe
#define BCHP_LEAP_CTRL_RBUS_ERR_XAC_reserved0_SHIFT                1

/* LEAP_CTRL :: RBUS_ERR_XAC :: XAC [00:00] */
#define BCHP_LEAP_CTRL_RBUS_ERR_XAC_XAC_MASK                       0x00000001
#define BCHP_LEAP_CTRL_RBUS_ERR_XAC_XAC_SHIFT                      0

/***************************************************************************
 *RBUS_ERR_CTRL - RBUS Error Control
 ***************************************************************************/
/* LEAP_CTRL :: RBUS_ERR_CTRL :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL_reserved0_MASK                0xfffffffe
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL_reserved0_SHIFT               1

/* LEAP_CTRL :: RBUS_ERR_CTRL :: CLR [00:00] */
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL_CLR_MASK                      0x00000001
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL_CLR_SHIFT                     0
#define BCHP_LEAP_CTRL_RBUS_ERR_CTRL_CLR_DEFAULT                   0x00000000

/***************************************************************************
 *BYTE_SWAP_EN - Byte Swap Enable for UBUS clients
 ***************************************************************************/
/* LEAP_CTRL :: BYTE_SWAP_EN :: reserved0 [31:28] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved0_MASK                 0xf0000000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved0_SHIFT                28

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_MST_EN3 [27:27] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN3_MASK      0x08000000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN3_SHIFT     27
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN3_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_MST_EN3 [26:26] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN3_MASK      0x04000000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN3_SHIFT     26
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN3_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_SLV_EN3 [25:25] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN3_MASK      0x02000000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN3_SHIFT     25
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN3_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_SLV_EN3 [24:24] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN3_MASK      0x01000000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN3_SHIFT     24
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN3_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: reserved1 [23:20] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved1_MASK                 0x00f00000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved1_SHIFT                20

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_MST_EN2 [19:19] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN2_MASK      0x00080000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN2_SHIFT     19
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN2_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_MST_EN2 [18:18] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN2_MASK      0x00040000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN2_SHIFT     18
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN2_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_SLV_EN2 [17:17] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN2_MASK      0x00020000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN2_SHIFT     17
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN2_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_SLV_EN2 [16:16] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN2_MASK      0x00010000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN2_SHIFT     16
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN2_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: reserved2 [15:12] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved2_MASK                 0x0000f000
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved2_SHIFT                12

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_MST_EN1 [11:11] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN1_MASK      0x00000800
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN1_SHIFT     11
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN1_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_MST_EN1 [10:10] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN1_MASK      0x00000400
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN1_SHIFT     10
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN1_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_SLV_EN1 [09:09] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN1_MASK      0x00000200
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN1_SHIFT     9
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN1_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_SLV_EN1 [08:08] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN1_MASK      0x00000100
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN1_SHIFT     8
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN1_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: reserved3 [07:04] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved3_MASK                 0x000000f0
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_reserved3_SHIFT                4

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_MST_EN0 [03:03] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN0_MASK      0x00000008
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN0_SHIFT     3
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_MST_EN0_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_MST_EN0 [02:02] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN0_MASK      0x00000004
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN0_SHIFT     2
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_MST_EN0_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_RD_SLV_EN0 [01:01] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN0_MASK      0x00000002
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN0_SHIFT     1
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_RD_SLV_EN0_DEFAULT   0x00000000

/* LEAP_CTRL :: BYTE_SWAP_EN :: BYTE_SWAP_WR_SLV_EN0 [00:00] */
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN0_MASK      0x00000001
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN0_SHIFT     0
#define BCHP_LEAP_CTRL_BYTE_SWAP_EN_BYTE_SWAP_WR_SLV_EN0_DEFAULT   0x00000000

/***************************************************************************
 *ADDR_TRANS - Address Translation window
 ***************************************************************************/
/* LEAP_CTRL :: ADDR_TRANS :: ADDR_TRANS [31:20] */
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_MASK                  0xfff00000
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_SHIFT                 20
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_DEFAULT               0x00000000

/* LEAP_CTRL :: ADDR_TRANS :: reserved0 [19:01] */
#define BCHP_LEAP_CTRL_ADDR_TRANS_reserved0_MASK                   0x000ffffe
#define BCHP_LEAP_CTRL_ADDR_TRANS_reserved0_SHIFT                  1

/* LEAP_CTRL :: ADDR_TRANS :: ADDR_TRANS_EN [00:00] */
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_EN_MASK               0x00000001
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_EN_SHIFT              0
#define BCHP_LEAP_CTRL_ADDR_TRANS_ADDR_TRANS_EN_DEFAULT            0x00000000

/***************************************************************************
 *GP0 - CPU General Purpose Register 0
 ***************************************************************************/
/* LEAP_CTRL :: GP0 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP0_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP0_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP0_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP1 - CPU General Purpose Register 1
 ***************************************************************************/
/* LEAP_CTRL :: GP1 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP1_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP1_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP1_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP2 - CPU General Purpose Register 2
 ***************************************************************************/
/* LEAP_CTRL :: GP2 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP2_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP2_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP2_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP3 - CPU General Purpose Register 3
 ***************************************************************************/
/* LEAP_CTRL :: GP3 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP3_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP3_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP3_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP4 - CPU General Purpose Register 4
 ***************************************************************************/
/* LEAP_CTRL :: GP4 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP4_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP4_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP4_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP5 - CPU General Purpose Register 5
 ***************************************************************************/
/* LEAP_CTRL :: GP5 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP5_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP5_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP5_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP6 - CPU General Purpose Register 6
 ***************************************************************************/
/* LEAP_CTRL :: GP6 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP6_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP6_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP6_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP7 - CPU General Purpose Register 7
 ***************************************************************************/
/* LEAP_CTRL :: GP7 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP7_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP7_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP7_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP8 - CPU General Purpose Register 8
 ***************************************************************************/
/* LEAP_CTRL :: GP8 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP8_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP8_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP8_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP9 - CPU General Purpose Register 9
 ***************************************************************************/
/* LEAP_CTRL :: GP9 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP9_SPARE_MASK                              0xffffffff
#define BCHP_LEAP_CTRL_GP9_SPARE_SHIFT                             0
#define BCHP_LEAP_CTRL_GP9_SPARE_DEFAULT                           0x00000000

/***************************************************************************
 *GP10 - CPU General Purpose Register 10
 ***************************************************************************/
/* LEAP_CTRL :: GP10 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP10_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP10_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP10_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP11 - CPU General Purpose Register 11
 ***************************************************************************/
/* LEAP_CTRL :: GP11 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP11_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP11_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP11_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP12 - CPU General Purpose Register 12
 ***************************************************************************/
/* LEAP_CTRL :: GP12 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP12_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP12_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP12_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP13 - CPU General Purpose Register 13
 ***************************************************************************/
/* LEAP_CTRL :: GP13 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP13_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP13_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP13_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP14 - CPU General Purpose Register 14
 ***************************************************************************/
/* LEAP_CTRL :: GP14 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP14_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP14_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP14_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP15 - CPU General Purpose Register 15
 ***************************************************************************/
/* LEAP_CTRL :: GP15 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP15_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP15_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP15_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP16 - CPU General Purpose Register 16
 ***************************************************************************/
/* LEAP_CTRL :: GP16 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP16_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP16_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP16_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP17 - CPU General Purpose Register 17
 ***************************************************************************/
/* LEAP_CTRL :: GP17 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP17_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP17_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP17_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP18 - CPU General Purpose Register 18
 ***************************************************************************/
/* LEAP_CTRL :: GP18 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP18_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP18_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP18_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP19 - CPU General Purpose Register 19
 ***************************************************************************/
/* LEAP_CTRL :: GP19 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP19_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP19_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP19_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP20 - CPU General Purpose Register 20
 ***************************************************************************/
/* LEAP_CTRL :: GP20 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP20_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP20_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP20_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP21 - CPU General Purpose Register 21
 ***************************************************************************/
/* LEAP_CTRL :: GP21 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP21_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP21_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP21_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP22 - CPU General Purpose Register 22
 ***************************************************************************/
/* LEAP_CTRL :: GP22 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP22_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP22_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP22_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP23 - CPU General Purpose Register 23
 ***************************************************************************/
/* LEAP_CTRL :: GP23 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP23_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP23_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP23_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP24 - CPU General Purpose Register 24
 ***************************************************************************/
/* LEAP_CTRL :: GP24 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP24_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP24_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP24_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP25 - CPU General Purpose Register 25
 ***************************************************************************/
/* LEAP_CTRL :: GP25 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP25_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP25_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP25_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP26 - CPU General Purpose Register 26
 ***************************************************************************/
/* LEAP_CTRL :: GP26 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP26_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP26_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP26_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP27 - CPU General Purpose Register 27
 ***************************************************************************/
/* LEAP_CTRL :: GP27 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP27_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP27_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP27_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP28 - CPU General Purpose Register 28
 ***************************************************************************/
/* LEAP_CTRL :: GP28 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP28_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP28_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP28_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP29 - CPU General Purpose Register 29
 ***************************************************************************/
/* LEAP_CTRL :: GP29 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP29_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP29_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP29_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP30 - CPU General Purpose Register 30
 ***************************************************************************/
/* LEAP_CTRL :: GP30 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP30_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP30_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP30_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP31 - CPU General Purpose Register 31
 ***************************************************************************/
/* LEAP_CTRL :: GP31 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP31_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP31_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP31_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP32 - CPU General Purpose Register 32
 ***************************************************************************/
/* LEAP_CTRL :: GP32 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP32_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP32_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP32_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP33 - CPU General Purpose Register 33
 ***************************************************************************/
/* LEAP_CTRL :: GP33 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP33_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP33_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP33_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP34 - CPU General Purpose Register 34
 ***************************************************************************/
/* LEAP_CTRL :: GP34 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP34_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP34_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP34_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP35 - CPU General Purpose Register 35
 ***************************************************************************/
/* LEAP_CTRL :: GP35 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP35_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP35_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP35_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP36 - CPU General Purpose Register 36
 ***************************************************************************/
/* LEAP_CTRL :: GP36 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP36_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP36_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP36_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP37 - CPU General Purpose Register 37
 ***************************************************************************/
/* LEAP_CTRL :: GP37 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP37_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP37_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP37_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP38 - CPU General Purpose Register 38
 ***************************************************************************/
/* LEAP_CTRL :: GP38 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP38_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP38_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP38_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP39 - CPU General Purpose Register 39
 ***************************************************************************/
/* LEAP_CTRL :: GP39 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP39_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP39_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP39_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP40 - CPU General Purpose Register 40
 ***************************************************************************/
/* LEAP_CTRL :: GP40 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP40_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP40_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP40_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP41 - CPU General Purpose Register 41
 ***************************************************************************/
/* LEAP_CTRL :: GP41 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP41_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP41_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP41_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP42 - CPU General Purpose Register 42
 ***************************************************************************/
/* LEAP_CTRL :: GP42 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP42_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP42_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP42_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP43 - CPU General Purpose Register 43
 ***************************************************************************/
/* LEAP_CTRL :: GP43 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP43_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP43_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP43_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP44 - CPU General Purpose Register 44
 ***************************************************************************/
/* LEAP_CTRL :: GP44 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP44_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP44_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP44_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP45 - CPU General Purpose Register 45
 ***************************************************************************/
/* LEAP_CTRL :: GP45 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP45_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP45_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP45_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP46 - CPU General Purpose Register 46
 ***************************************************************************/
/* LEAP_CTRL :: GP46 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP46_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP46_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP46_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP47 - CPU General Purpose Register 47
 ***************************************************************************/
/* LEAP_CTRL :: GP47 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP47_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP47_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP47_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP48 - CPU General Purpose Register 48
 ***************************************************************************/
/* LEAP_CTRL :: GP48 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP48_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP48_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP48_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP49 - CPU General Purpose Register 49
 ***************************************************************************/
/* LEAP_CTRL :: GP49 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP49_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP49_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP49_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP50 - CPU General Purpose Register 50
 ***************************************************************************/
/* LEAP_CTRL :: GP50 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP50_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP50_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP50_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP51 - CPU General Purpose Register 51
 ***************************************************************************/
/* LEAP_CTRL :: GP51 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP51_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP51_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP51_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP52 - CPU General Purpose Register 52
 ***************************************************************************/
/* LEAP_CTRL :: GP52 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP52_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP52_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP52_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP53 - CPU General Purpose Register 53
 ***************************************************************************/
/* LEAP_CTRL :: GP53 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP53_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP53_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP53_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP54 - CPU General Purpose Register 54
 ***************************************************************************/
/* LEAP_CTRL :: GP54 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP54_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP54_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP54_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP55 - CPU General Purpose Register 55
 ***************************************************************************/
/* LEAP_CTRL :: GP55 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP55_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP55_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP55_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP56 - CPU General Purpose Register 56
 ***************************************************************************/
/* LEAP_CTRL :: GP56 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP56_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP56_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP56_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP57 - CPU General Purpose Register 57
 ***************************************************************************/
/* LEAP_CTRL :: GP57 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP57_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP57_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP57_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP58 - CPU General Purpose Register 58
 ***************************************************************************/
/* LEAP_CTRL :: GP58 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP58_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP58_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP58_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP59 - CPU General Purpose Register 59
 ***************************************************************************/
/* LEAP_CTRL :: GP59 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP59_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP59_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP59_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP60 - CPU General Purpose Register 60
 ***************************************************************************/
/* LEAP_CTRL :: GP60 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP60_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP60_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP60_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP61 - CPU General Purpose Register 61
 ***************************************************************************/
/* LEAP_CTRL :: GP61 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP61_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP61_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP61_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP62 - CPU General Purpose Register 62
 ***************************************************************************/
/* LEAP_CTRL :: GP62 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP62_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP62_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP62_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP63 - CPU General Purpose Register 63
 ***************************************************************************/
/* LEAP_CTRL :: GP63 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP63_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP63_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP63_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP64 - CPU General Purpose Register 64
 ***************************************************************************/
/* LEAP_CTRL :: GP64 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP64_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP64_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP64_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP65 - CPU General Purpose Register 65
 ***************************************************************************/
/* LEAP_CTRL :: GP65 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP65_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP65_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP65_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP66 - CPU General Purpose Register 66
 ***************************************************************************/
/* LEAP_CTRL :: GP66 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP66_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP66_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP66_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP67 - CPU General Purpose Register 67
 ***************************************************************************/
/* LEAP_CTRL :: GP67 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP67_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP67_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP67_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP68 - CPU General Purpose Register 68
 ***************************************************************************/
/* LEAP_CTRL :: GP68 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP68_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP68_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP68_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP69 - CPU General Purpose Register 69
 ***************************************************************************/
/* LEAP_CTRL :: GP69 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP69_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP69_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP69_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP70 - CPU General Purpose Register 70
 ***************************************************************************/
/* LEAP_CTRL :: GP70 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP70_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP70_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP70_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP71 - CPU General Purpose Register 71
 ***************************************************************************/
/* LEAP_CTRL :: GP71 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP71_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP71_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP71_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP72 - CPU General Purpose Register 72
 ***************************************************************************/
/* LEAP_CTRL :: GP72 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP72_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP72_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP72_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP73 - CPU General Purpose Register 73
 ***************************************************************************/
/* LEAP_CTRL :: GP73 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP73_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP73_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP73_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP74 - CPU General Purpose Register 74
 ***************************************************************************/
/* LEAP_CTRL :: GP74 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP74_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP74_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP74_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP75 - CPU General Purpose Register 75
 ***************************************************************************/
/* LEAP_CTRL :: GP75 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP75_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP75_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP75_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP76 - CPU General Purpose Register 76
 ***************************************************************************/
/* LEAP_CTRL :: GP76 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP76_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP76_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP76_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP77 - CPU General Purpose Register 77
 ***************************************************************************/
/* LEAP_CTRL :: GP77 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP77_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP77_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP77_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP78 - CPU General Purpose Register 78
 ***************************************************************************/
/* LEAP_CTRL :: GP78 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP78_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP78_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP78_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP79 - CPU General Purpose Register 79
 ***************************************************************************/
/* LEAP_CTRL :: GP79 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP79_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP79_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP79_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP80 - CPU General Purpose Register 80
 ***************************************************************************/
/* LEAP_CTRL :: GP80 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP80_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP80_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP80_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP81 - CPU General Purpose Register 81
 ***************************************************************************/
/* LEAP_CTRL :: GP81 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP81_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP81_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP81_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP82 - CPU General Purpose Register 82
 ***************************************************************************/
/* LEAP_CTRL :: GP82 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP82_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP82_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP82_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP83 - CPU General Purpose Register 83
 ***************************************************************************/
/* LEAP_CTRL :: GP83 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP83_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP83_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP83_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP84 - CPU General Purpose Register 84
 ***************************************************************************/
/* LEAP_CTRL :: GP84 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP84_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP84_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP84_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP85 - CPU General Purpose Register 85
 ***************************************************************************/
/* LEAP_CTRL :: GP85 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP85_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP85_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP85_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP86 - CPU General Purpose Register 86
 ***************************************************************************/
/* LEAP_CTRL :: GP86 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP86_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP86_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP86_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP87 - CPU General Purpose Register 87
 ***************************************************************************/
/* LEAP_CTRL :: GP87 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP87_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP87_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP87_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP88 - CPU General Purpose Register 88
 ***************************************************************************/
/* LEAP_CTRL :: GP88 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP88_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP88_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP88_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP89 - CPU General Purpose Register 89
 ***************************************************************************/
/* LEAP_CTRL :: GP89 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP89_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP89_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP89_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP90 - CPU General Purpose Register 90
 ***************************************************************************/
/* LEAP_CTRL :: GP90 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP90_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP90_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP90_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP91 - CPU General Purpose Register 91
 ***************************************************************************/
/* LEAP_CTRL :: GP91 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP91_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP91_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP91_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP92 - CPU General Purpose Register 92
 ***************************************************************************/
/* LEAP_CTRL :: GP92 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP92_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP92_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP92_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP93 - CPU General Purpose Register 93
 ***************************************************************************/
/* LEAP_CTRL :: GP93 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP93_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP93_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP93_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP94 - CPU General Purpose Register 94
 ***************************************************************************/
/* LEAP_CTRL :: GP94 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP94_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP94_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP94_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP95 - CPU General Purpose Register 95
 ***************************************************************************/
/* LEAP_CTRL :: GP95 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP95_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP95_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP95_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP96 - CPU General Purpose Register 96
 ***************************************************************************/
/* LEAP_CTRL :: GP96 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP96_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP96_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP96_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP97 - CPU General Purpose Register 97
 ***************************************************************************/
/* LEAP_CTRL :: GP97 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP97_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP97_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP97_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP98 - CPU General Purpose Register 98
 ***************************************************************************/
/* LEAP_CTRL :: GP98 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP98_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP98_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP98_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP99 - CPU General Purpose Register 99
 ***************************************************************************/
/* LEAP_CTRL :: GP99 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP99_SPARE_MASK                             0xffffffff
#define BCHP_LEAP_CTRL_GP99_SPARE_SHIFT                            0
#define BCHP_LEAP_CTRL_GP99_SPARE_DEFAULT                          0x00000000

/***************************************************************************
 *GP100 - CPU General Purpose Register 100
 ***************************************************************************/
/* LEAP_CTRL :: GP100 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP100_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP100_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP100_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP101 - CPU General Purpose Register 101
 ***************************************************************************/
/* LEAP_CTRL :: GP101 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP101_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP101_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP101_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP102 - CPU General Purpose Register 102
 ***************************************************************************/
/* LEAP_CTRL :: GP102 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP102_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP102_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP102_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP103 - CPU General Purpose Register 103
 ***************************************************************************/
/* LEAP_CTRL :: GP103 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP103_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP103_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP103_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP104 - CPU General Purpose Register 104
 ***************************************************************************/
/* LEAP_CTRL :: GP104 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP104_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP104_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP104_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP105 - CPU General Purpose Register 105
 ***************************************************************************/
/* LEAP_CTRL :: GP105 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP105_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP105_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP105_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP106 - CPU General Purpose Register 106
 ***************************************************************************/
/* LEAP_CTRL :: GP106 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP106_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP106_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP106_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP107 - CPU General Purpose Register 107
 ***************************************************************************/
/* LEAP_CTRL :: GP107 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP107_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP107_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP107_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP108 - CPU General Purpose Register 108
 ***************************************************************************/
/* LEAP_CTRL :: GP108 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP108_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP108_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP108_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP109 - CPU General Purpose Register 109
 ***************************************************************************/
/* LEAP_CTRL :: GP109 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP109_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP109_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP109_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP110 - CPU General Purpose Register 110
 ***************************************************************************/
/* LEAP_CTRL :: GP110 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP110_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP110_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP110_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP111 - CPU General Purpose Register 111
 ***************************************************************************/
/* LEAP_CTRL :: GP111 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP111_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP111_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP111_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP112 - CPU General Purpose Register 112
 ***************************************************************************/
/* LEAP_CTRL :: GP112 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP112_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP112_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP112_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP113 - CPU General Purpose Register 113
 ***************************************************************************/
/* LEAP_CTRL :: GP113 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP113_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP113_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP113_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP114 - CPU General Purpose Register 114
 ***************************************************************************/
/* LEAP_CTRL :: GP114 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP114_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP114_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP114_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP115 - CPU General Purpose Register 115
 ***************************************************************************/
/* LEAP_CTRL :: GP115 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP115_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP115_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP115_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP116 - CPU General Purpose Register 116
 ***************************************************************************/
/* LEAP_CTRL :: GP116 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP116_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP116_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP116_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP117 - CPU General Purpose Register 117
 ***************************************************************************/
/* LEAP_CTRL :: GP117 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP117_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP117_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP117_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP118 - CPU General Purpose Register 118
 ***************************************************************************/
/* LEAP_CTRL :: GP118 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP118_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP118_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP118_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP119 - CPU General Purpose Register 119
 ***************************************************************************/
/* LEAP_CTRL :: GP119 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP119_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP119_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP119_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP120 - CPU General Purpose Register 120
 ***************************************************************************/
/* LEAP_CTRL :: GP120 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP120_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP120_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP120_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP121 - CPU General Purpose Register 121
 ***************************************************************************/
/* LEAP_CTRL :: GP121 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP121_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP121_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP121_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP122 - CPU General Purpose Register 122
 ***************************************************************************/
/* LEAP_CTRL :: GP122 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP122_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP122_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP122_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP123 - CPU General Purpose Register 123
 ***************************************************************************/
/* LEAP_CTRL :: GP123 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP123_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP123_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP123_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP124 - CPU General Purpose Register 124
 ***************************************************************************/
/* LEAP_CTRL :: GP124 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP124_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP124_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP124_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP125 - CPU General Purpose Register 125
 ***************************************************************************/
/* LEAP_CTRL :: GP125 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP125_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP125_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP125_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP126 - CPU General Purpose Register 126
 ***************************************************************************/
/* LEAP_CTRL :: GP126 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP126_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP126_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP126_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *GP127 - CPU General Purpose Register 127
 ***************************************************************************/
/* LEAP_CTRL :: GP127 :: SPARE [31:00] */
#define BCHP_LEAP_CTRL_GP127_SPARE_MASK                            0xffffffff
#define BCHP_LEAP_CTRL_GP127_SPARE_SHIFT                           0
#define BCHP_LEAP_CTRL_GP127_SPARE_DEFAULT                         0x00000000

/***************************************************************************
 *BLOCK_START_ADDR0 - Block Range Start Address 0
 ***************************************************************************/
/* LEAP_CTRL :: BLOCK_START_ADDR0 :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR0_ADDR_MASK                 0xffffffff
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR0_ADDR_SHIFT                0
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR0_ADDR_DEFAULT              0x00000000

/***************************************************************************
 *BLOCK_START_ADDR1 - Block Range Start Address 1
 ***************************************************************************/
/* LEAP_CTRL :: BLOCK_START_ADDR1 :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR1_ADDR_MASK                 0xffffffff
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR1_ADDR_SHIFT                0
#define BCHP_LEAP_CTRL_BLOCK_START_ADDR1_ADDR_DEFAULT              0x00000000

/***************************************************************************
 *BLOCK_END_ADDR0 - Block Range End Address 0
 ***************************************************************************/
/* LEAP_CTRL :: BLOCK_END_ADDR0 :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR0_ADDR_MASK                   0xffffffff
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR0_ADDR_SHIFT                  0
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR0_ADDR_DEFAULT                0x00000000

/***************************************************************************
 *BLOCK_END_ADDR1 - Block Range End Address 1
 ***************************************************************************/
/* LEAP_CTRL :: BLOCK_END_ADDR1 :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR1_ADDR_MASK                   0xffffffff
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR1_ADDR_SHIFT                  0
#define BCHP_LEAP_CTRL_BLOCK_END_ADDR1_ADDR_DEFAULT                0x00000000

/***************************************************************************
 *BLOCK_RANGE_CTRL - Block Range Control
 ***************************************************************************/
/* LEAP_CTRL :: BLOCK_RANGE_CTRL :: reserved0 [31:02] */
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_reserved0_MASK             0xfffffffc
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_reserved0_SHIFT            2

/* LEAP_CTRL :: BLOCK_RANGE_CTRL :: BLOCK_EN1 [01:01] */
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN1_MASK             0x00000002
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN1_SHIFT            1
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN1_DEFAULT          0x00000000

/* LEAP_CTRL :: BLOCK_RANGE_CTRL :: BLOCK_EN0 [00:00] */
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN0_MASK             0x00000001
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN0_SHIFT            0
#define BCHP_LEAP_CTRL_BLOCK_RANGE_CTRL_BLOCK_EN0_DEFAULT          0x00000000

/***************************************************************************
 *DIAG_DMA_CTRL - Diagnostic Capture DMA Control Register
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_DMA_CTRL :: reserved0 [31:06] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_reserved0_MASK                0xffffffc0
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_reserved0_SHIFT               6

/* LEAP_CTRL :: DIAG_DMA_CTRL :: DIS_UBUS_MUX [05:05] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_DIS_UBUS_MUX_MASK             0x00000020
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_DIS_UBUS_MUX_SHIFT            5
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_DIS_UBUS_MUX_DEFAULT          0x00000000

/* LEAP_CTRL :: DIAG_DMA_CTRL :: reserved1 [04:04] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_reserved1_MASK                0x00000010
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_reserved1_SHIFT               4

/* LEAP_CTRL :: DIAG_DMA_CTRL :: WORD_SWAP [03:03] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_WORD_SWAP_MASK                0x00000008
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_WORD_SWAP_SHIFT               3
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_WORD_SWAP_DEFAULT             0x00000000

/* LEAP_CTRL :: DIAG_DMA_CTRL :: INT_MEM_MODE [02:01] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_INT_MEM_MODE_MASK             0x00000006
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_INT_MEM_MODE_SHIFT            1
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_INT_MEM_MODE_DEFAULT          0x00000000

/* LEAP_CTRL :: DIAG_DMA_CTRL :: EXT_MEM_MODE [00:00] */
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_EXT_MEM_MODE_MASK             0x00000001
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_EXT_MEM_MODE_SHIFT            0
#define BCHP_LEAP_CTRL_DIAG_DMA_CTRL_EXT_MEM_MODE_DEFAULT          0x00000000

/***************************************************************************
 *DIAG_DEST_ADDR - Diagnostic Capture Destination Address
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_DEST_ADDR :: DEST_ADDR [31:00] */
#define BCHP_LEAP_CTRL_DIAG_DEST_ADDR_DEST_ADDR_MASK               0xffffffff
#define BCHP_LEAP_CTRL_DIAG_DEST_ADDR_DEST_ADDR_SHIFT              0
#define BCHP_LEAP_CTRL_DIAG_DEST_ADDR_DEST_ADDR_DEFAULT            0x00000000

/***************************************************************************
 *DIAG_XFER_SIZE - Diagnostic Capture Data Transfer Size
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_XFER_SIZE :: XFER_SIZE [31:00] */
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_XFER_SIZE_MASK               0xffffffff
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_XFER_SIZE_SHIFT              0
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_XFER_SIZE_DEFAULT            0x00000000

/***************************************************************************
 *DIAG_POST_TRIG_NUM - Number of Samples to Capture Post Trigger
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_POST_TRIG_NUM :: SIZE [31:00] */
#define BCHP_LEAP_CTRL_DIAG_POST_TRIG_NUM_SIZE_MASK                0xffffffff
#define BCHP_LEAP_CTRL_DIAG_POST_TRIG_NUM_SIZE_SHIFT               0
#define BCHP_LEAP_CTRL_DIAG_POST_TRIG_NUM_SIZE_DEFAULT             0x00000001

/***************************************************************************
 *DIAG_STATUS - DIAG Status Register
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STATUS :: reserved0 [31:06] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_reserved0_MASK                  0xffffffc0
#define BCHP_LEAP_CTRL_DIAG_STATUS_reserved0_SHIFT                 6

/* LEAP_CTRL :: DIAG_STATUS :: DIAG_CAP_MEM_DONE [05:05] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_CAP_MEM_DONE_MASK          0x00000020
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_CAP_MEM_DONE_SHIFT         5

/* LEAP_CTRL :: DIAG_STATUS :: DIAG_CAP_DDR_DONE [04:04] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_CAP_DDR_DONE_MASK          0x00000010
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_CAP_DDR_DONE_SHIFT         4

/* LEAP_CTRL :: DIAG_STATUS :: reserved1 [03:02] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_reserved1_MASK                  0x0000000c
#define BCHP_LEAP_CTRL_DIAG_STATUS_reserved1_SHIFT                 2

/* LEAP_CTRL :: DIAG_STATUS :: SCB_FIFO_ERR [01:01] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_SCB_FIFO_ERR_MASK               0x00000002
#define BCHP_LEAP_CTRL_DIAG_STATUS_SCB_FIFO_ERR_SHIFT              1

/* LEAP_CTRL :: DIAG_STATUS :: DIAG_FIFO_ERR [00:00] */
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_FIFO_ERR_MASK              0x00000001
#define BCHP_LEAP_CTRL_DIAG_STATUS_DIAG_FIFO_ERR_SHIFT             0

/***************************************************************************
 *DIAG_STAT_MEM_ADDR_START - DIAG MEM START Address of Captured Block
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_MEM_ADDR_START :: BANK [31:25] */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_START_BANK_MASK          0xfe000000
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_START_BANK_SHIFT         25

/* LEAP_CTRL :: DIAG_STAT_MEM_ADDR_START :: ADDR [24:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_START_ADDR_MASK          0x01ffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_START_ADDR_SHIFT         0

/***************************************************************************
 *DIAG_STAT_MEM_ADDR_END - DIAG MEM END Address of Captured Block
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_MEM_ADDR_END :: BANK [31:25] */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_END_BANK_MASK            0xfe000000
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_END_BANK_SHIFT           25

/* LEAP_CTRL :: DIAG_STAT_MEM_ADDR_END :: ADDR [24:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_END_ADDR_MASK            0x01ffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_END_ADDR_SHIFT           0

/***************************************************************************
 *DIAG_STAT_MEM_ADDR_TRIGGER - DIAG MEM Address of Trigger Sample
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_MEM_ADDR_TRIGGER :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_TRIGGER_ADDR_MASK        0xffffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_MEM_ADDR_TRIGGER_ADDR_SHIFT       0

/***************************************************************************
 *DIAG_STAT_DDR_ADDR_START - DIAG DDR START Address of Captured Block
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_DDR_ADDR_START :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_START_ADDR_MASK          0xffffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_START_ADDR_SHIFT         0

/***************************************************************************
 *DIAG_STAT_DDR_ADDR_END - DIAG DDR END Address of Captured Block
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_DDR_ADDR_END :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_END_ADDR_MASK            0xffffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_END_ADDR_SHIFT           0

/***************************************************************************
 *DIAG_STAT_DDR_ADDR_TRIGGER - DIAG DDR Address of Trigger Sample
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_STAT_DDR_ADDR_TRIGGER :: ADDR [31:00] */
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_TRIGGER_ADDR_MASK        0xffffffff
#define BCHP_LEAP_CTRL_DIAG_STAT_DDR_ADDR_TRIGGER_ADDR_SHIFT       0

/***************************************************************************
 *DIAG_XFER_SIZE_INT_MEM - Diagnostic Capture Data Transfer Size
 ***************************************************************************/
/* LEAP_CTRL :: DIAG_XFER_SIZE_INT_MEM :: XFER_SIZE [31:00] */
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_INT_MEM_XFER_SIZE_MASK       0xffffffff
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_INT_MEM_XFER_SIZE_SHIFT      0
#define BCHP_LEAP_CTRL_DIAG_XFER_SIZE_INT_MEM_XFER_SIZE_DEFAULT    0x00003fff

/***************************************************************************
 *MTSIF_HEADER - DIAG Status Register
 ***************************************************************************/
/* LEAP_CTRL :: MTSIF_HEADER :: SYNC_BYTE [31:24] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SYNC_BYTE_MASK                 0xff000000
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SYNC_BYTE_SHIFT                24
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SYNC_BYTE_DEFAULT              0x00000047

/* LEAP_CTRL :: MTSIF_HEADER :: TRANSPORT_ERROR_INDICATOR [23:23] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_ERROR_INDICATOR_MASK 0x00800000
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_ERROR_INDICATOR_SHIFT 23
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_ERROR_INDICATOR_DEFAULT 0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: PAYLOAD_UNIT_START_INDICATOR [22:22] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_UNIT_START_INDICATOR_MASK 0x00400000
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_UNIT_START_INDICATOR_SHIFT 22
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_UNIT_START_INDICATOR_DEFAULT 0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: TRANSPORT_PRIORITY [21:21] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_PRIORITY_MASK        0x00200000
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_PRIORITY_SHIFT       21
#define BCHP_LEAP_CTRL_MTSIF_HEADER_TRANSPORT_PRIORITY_DEFAULT     0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: PID [20:08] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PID_MASK                       0x001fff00
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PID_SHIFT                      8
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PID_DEFAULT                    0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: SCRAMBLING_CONTROL [07:06] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SCRAMBLING_CONTROL_MASK        0x000000c0
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SCRAMBLING_CONTROL_SHIFT       6
#define BCHP_LEAP_CTRL_MTSIF_HEADER_SCRAMBLING_CONTROL_DEFAULT     0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: ADAPTATION_FIELD_FLAG [05:05] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_ADAPTATION_FIELD_FLAG_MASK     0x00000020
#define BCHP_LEAP_CTRL_MTSIF_HEADER_ADAPTATION_FIELD_FLAG_SHIFT    5
#define BCHP_LEAP_CTRL_MTSIF_HEADER_ADAPTATION_FIELD_FLAG_DEFAULT  0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: PAYLOAD_FLAG [04:04] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_FLAG_MASK              0x00000010
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_FLAG_SHIFT             4
#define BCHP_LEAP_CTRL_MTSIF_HEADER_PAYLOAD_FLAG_DEFAULT           0x00000000

/* LEAP_CTRL :: MTSIF_HEADER :: reserved0 [03:00] */
#define BCHP_LEAP_CTRL_MTSIF_HEADER_reserved0_MASK                 0x0000000f
#define BCHP_LEAP_CTRL_MTSIF_HEADER_reserved0_SHIFT                0

/***************************************************************************
 *MTSIF_CTRL - DIAG Status Register
 ***************************************************************************/
/* LEAP_CTRL :: MTSIF_CTRL :: reserved0 [31:18] */
#define BCHP_LEAP_CTRL_MTSIF_CTRL_reserved0_MASK                   0xfffc0000
#define BCHP_LEAP_CTRL_MTSIF_CTRL_reserved0_SHIFT                  18

/* LEAP_CTRL :: MTSIF_CTRL :: MODE [17:17] */
#define BCHP_LEAP_CTRL_MTSIF_CTRL_MODE_MASK                        0x00020000
#define BCHP_LEAP_CTRL_MTSIF_CTRL_MODE_SHIFT                       17
#define BCHP_LEAP_CTRL_MTSIF_CTRL_MODE_DEFAULT                     0x00000001

/* LEAP_CTRL :: MTSIF_CTRL :: TIMER [16:01] */
#define BCHP_LEAP_CTRL_MTSIF_CTRL_TIMER_MASK                       0x0001fffe
#define BCHP_LEAP_CTRL_MTSIF_CTRL_TIMER_SHIFT                      1
#define BCHP_LEAP_CTRL_MTSIF_CTRL_TIMER_DEFAULT                    0x000000ff

/* LEAP_CTRL :: MTSIF_CTRL :: ENABLE_TRANSFER [00:00] */
#define BCHP_LEAP_CTRL_MTSIF_CTRL_ENABLE_TRANSFER_MASK             0x00000001
#define BCHP_LEAP_CTRL_MTSIF_CTRL_ENABLE_TRANSFER_SHIFT            0
#define BCHP_LEAP_CTRL_MTSIF_CTRL_ENABLE_TRANSFER_DEFAULT          0x00000000

/***************************************************************************
 *MTSIF_STATUS - DIAG Status Register
 ***************************************************************************/
/* LEAP_CTRL :: MTSIF_STATUS :: reserved0 [31:05] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_reserved0_MASK                 0xffffffe0
#define BCHP_LEAP_CTRL_MTSIF_STATUS_reserved0_SHIFT                5

/* LEAP_CTRL :: MTSIF_STATUS :: MEM_TRANSFER_COMPLETE [04:04] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MEM_TRANSFER_COMPLETE_MASK     0x00000010
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MEM_TRANSFER_COMPLETE_SHIFT    4
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MEM_TRANSFER_COMPLETE_DEFAULT  0x00000000

/* LEAP_CTRL :: MTSIF_STATUS :: FIFO_OVERFLOW [03:03] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_FIFO_OVERFLOW_MASK             0x00000008
#define BCHP_LEAP_CTRL_MTSIF_STATUS_FIFO_OVERFLOW_SHIFT            3
#define BCHP_LEAP_CTRL_MTSIF_STATUS_FIFO_OVERFLOW_DEFAULT          0x00000000

/* LEAP_CTRL :: MTSIF_STATUS :: TIMEOUT_ERR [02:02] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_TIMEOUT_ERR_MASK               0x00000004
#define BCHP_LEAP_CTRL_MTSIF_STATUS_TIMEOUT_ERR_SHIFT              2
#define BCHP_LEAP_CTRL_MTSIF_STATUS_TIMEOUT_ERR_DEFAULT            0x00000000

/* LEAP_CTRL :: MTSIF_STATUS :: INCOMPLETE_PCKT [01:01] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_INCOMPLETE_PCKT_MASK           0x00000002
#define BCHP_LEAP_CTRL_MTSIF_STATUS_INCOMPLETE_PCKT_SHIFT          1
#define BCHP_LEAP_CTRL_MTSIF_STATUS_INCOMPLETE_PCKT_DEFAULT        0x00000000

/* LEAP_CTRL :: MTSIF_STATUS :: MTSIF_TRANSFER_DONE [00:00] */
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MTSIF_TRANSFER_DONE_MASK       0x00000001
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MTSIF_TRANSFER_DONE_SHIFT      0
#define BCHP_LEAP_CTRL_MTSIF_STATUS_MTSIF_TRANSFER_DONE_DEFAULT    0x00000000

/***************************************************************************
 *MTSIF_MEM_START_ADDR - DIAG Status Register
 ***************************************************************************/
/* LEAP_CTRL :: MTSIF_MEM_START_ADDR :: START_ADDRESS [31:00] */
#define BCHP_LEAP_CTRL_MTSIF_MEM_START_ADDR_START_ADDRESS_MASK     0xffffffff
#define BCHP_LEAP_CTRL_MTSIF_MEM_START_ADDR_START_ADDRESS_SHIFT    0
#define BCHP_LEAP_CTRL_MTSIF_MEM_START_ADDR_START_ADDRESS_DEFAULT  0x00000000

#endif /* #ifndef BCHP_LEAP_CTRL_H__ */

/* End of File */
