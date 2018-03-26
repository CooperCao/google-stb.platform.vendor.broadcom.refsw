/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef _BFSK_45402_H_
#define _BFSK_45402_H_


#ifdef __cplusplus
extern "C" {
#endif


/* device configuration parameters */
enum {
   /* TBD: add new device configuration parameters here... */
   BFSK_45402_CONFIG_MAX
};


/* channel configuration parameters */
enum {
   BFSK_45402_CHAN_CONFIG_TX_FREQ_HZ = 0,   /* Tx frequency in Hz */
   BFSK_45402_CHAN_CONFIG_RX_FREQ_HZ,       /* Rx frequency in Hz */
   BFSK_45402_CHAN_CONFIG_TX_DEV_HZ,        /* Tx deviation in Hz */
   BFSK_45402_CHAN_CONFIG_PRECARRIER_US,    /* pre-carrier in us */
   BFSK_45402_CHAN_CONFIG_POSTCARRIER_US,   /* post-carrier in us */
   BFSK_45402_CHAN_CONFIG_TX_POWER,         /* config param for transmit power */
   BFSK_45402_CHAN_CONFIG_FSK_PREAMBLE,     /* preamble bytes */
   BFSK_45402_CHAN_CONFIG_CRC_POLY,         /* polynomial used to calculate the CRC */
   BFSK_45402_CHAN_CONFIG_INSERT_TX_CRC,    /* insert crc during transmit */
   BFSK_45402_CHAN_CONFIG_CHECK_RX_CRC,     /* validate crc during receive */
   BFSK_45402_CHAN_CONFIG_UART_BAUD_RATE,   /* uart baud rate in bits/sec */
   BFSK_45402_CHAN_CONFIG_RX_GAIN,          /* receiver pga gain */
   BFSK_45402_CHAN_CONFIG_RX_THRESH,        /* receiver threshold to control sensitivity */
   BFSK_45402_CHAN_CONFIG_MAX
};


/* chip-specific functions */
BERR_Code BFSK_45402_GetDefaultSettings(BFSK_Settings *pDefSettings);
BERR_Code BFSK_45402_GetChannelDefaultSettings(BFSK_Handle h, uint32_t chnNo, BFSK_ChannelSettings *pChnDefSettings);

#ifdef __cplusplus
}
#endif

#endif /* _BFSK_45402_H_ */
