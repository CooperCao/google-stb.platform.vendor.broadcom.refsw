/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/include/bbPcRadio.h $
*
* DESCRIPTION:
*   i386 Radio Simulator interface.
*
* $Revision: 4138 $
* $Date: 2014-10-20 10:29:44Z $
*
*****************************************************************************************/


#ifndef _BB_PC_RADIO_H
#define _BB_PC_RADIO_H


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Simulates for the i386 number of channel pages implemented.
 */
#define PC_RADIO_CHANNEL_PAGES_NUMBER               1


/**//**
 * \brief   Simulates for the i386 IEEE-index of the channel page[0].
 */
#define PC_CHANNEL_PAGE_0                           0


/**//**
 * \brief   Simulates for the i386 first channel implemented on the page[0].
 */
#define PC_CHANNEL_MIN_ON_PAGE_0                    11


/**//**
 * \brief   Simulates for the i386 last channel implemented on the page[0].
 */
#define PC_CHANNEL_MAX_ON_PAGE_0                    26


/**//**
 * \brief   Simulates for the i386 channels implemented on the page[0].
 */
#define PC_CHANNELS_ON_PAGE_0                       0x07FFF800


/**//**
 * \brief   Simulates for the i386 default channel page.
 */
#define PC_PAGE_DEFAULT                             0


/**//**
 * \brief   Simulates for the i386 default channel.
 */
#define PC_CHANNEL_DEFAULT                          11


/**//**
 * \brief   Simulates for the i386 minimum Radio Transmitter power value.
 */
#define PC_TX_POWER_MIN                             (-32)


/**//**
 * \brief   Simulates for the i386 maximum Radio Transmitter power value.
 */
#define PC_TX_POWER_MAX                             (+31)


/**//**
 * \brief   Simulates for the i386 default value of the Radio Transmitter power.
 */
#define PC_TX_POWER_DEF                             (+3)


/**//**
 * \brief   Simulates for the i386 tolerance of the Radio Transmitter power value,
 *  absolute value, measured in dBm.
 */
#define PC_TX_POWER_TOL                             1


/**//**
 * \brief   Simulates for the i386 minimum code value of Radio CCA mode.
 */
#define PC_CCA_MODE_MIN                             0


/**//**
 * \brief   Simulates for the i386 maximum code value of Radio CCA mode.
 */
#define PC_CCA_MODE_MAX                             3


/**//**
 * \brief   Simulates for the i386 default code value of Radio CCA mode.
 */
#define PC_CCA_MODE_DEF                             1


/**//**
 * \brief   Simulates for the i386 the RF-INIT.request to the Radio Hardware.
 */
#define PC_RadioInit()                              while(0)


/**//**
 * \brief   Simulates for the i386 the RF-SET-TRX-STATE.request to the Radio Hardware.
 */
#define PC_RadioSetTrxStateReq(cmdId)               do { ((void)(cmdId)); } while(0)


/**//**
 * \brief   Simulates for the i386 the RF-GET-TRX-STATE.request to the Radio Hardware.
 */
#define PC_RadioGetTrxState()                       (HAL_RADIO_STATE_UNDEFINED)


/**//**
 * \brief   Simulates for the i386 locking the Radio Hardware Frame RX-Buffer.
 */
#define PC_RadioRxBufferLock()                      while(0)


/**//**
 * \brief   Simulates for the i386 unlocking the Radio Hardware Frame RX-Buffer.
 */
#define PC_RadioRxBufferUnlock()                    while(0)


/**//**
 * \brief   Simulates for the i386 starting pre-reading of the Radio Hardware Frame
 *  RX-Buffer.
 */
#define PC_RadioRxBufferPreReadStart()              (HAL_RADIO_CRC_BROKEN)


/**//**
 * \brief   Simulates for the i386 pre-reading of the Radio Hardware Frame RX-Buffer.
 */
#define PC_RadioRxBufferPreRead(dst, count)         do { ((void)(dst)); ((void)(count)); } while(0)


/**//**
 * \brief   Simulates for the i386 starting final-reading of the Radio Hardware Frame
 *  RX-Buffer.
 */
#define PC_RadioRxBufferFinalReadStart()            while(0)


/**//**
 * \brief   Simulates for the i386 final-reading of the Radio Hardware Frame RX-Buffer.
 */
#define PC_RadioRxBufferFinalRead(ppdu)             (((void)(ppdu)), 0x0)


/**//**
 * \brief   Simulates for the i386 finishing of the pre- or final-read of the Radio
 *  Hardware Frame RX-Buffer.
 */
#define PC_RadioRxBufferReadFinish()                while(0)


/**//**
 * \brief   Simulates for the i386 writing PPDU into the Radio Hardware Frame TX-Buffer.
 */
#define PC_RadioTxBufferWrite(phr, psdu)            do { ((void)(phr)); ((void)(psdu)); } while(0)


/**//**
 * \brief   Simulates for the i386 the RF-TRANSMIT.request to the Radio Hardware.
 */
#define PC_RadioTransmitReq(timed, startTime)       do { ((void)(timed)); ((void)(startTime)); } while(0)


/**//**
 * \brief   Simulates for the i386 the RF-CCA.request to the Radio Hardware.
 */
#define PC_RadioCcaReq()                            while(0)


#if defined(_ZBPRO_) || defined(RF4CE_TARGET)
/**//**
 * \brief   Simulates for the i386 the RF-ED.request to the Radio Hardware.
 */
# define PC_RadioEdReq()                            while(0)
#endif /* _ZBPRO_ || RF4CE_TARGET */


/**//**
 * \brief   Simulates for the i386 the RF-SET-CHANNEL.request to the Radio Hardware.
 */
#define PC_RadioSetChannelReq(channelOnPage)        do { ((void)(channelOnPage)); } while(0)


/**//**
 * \brief   Simulates for the i386 switching of the Radio Hardware transceiver nominal
 *  transmit power value.
 */
#define PC_RadioSetTransmitPower(txPower)           do { ((void)(txPower)); } while(0)


/**//**
 * \brief   Simulates for the i386 switching of the Radio Hardware transceiver CCA mode.
 */
#define PC_RadioSetCcaMode(ccaMode)                 do { ((void)(ccaMode)); } while(0)


/**//**
 * \brief   Simulates for the i386 return of the RSSI value, in dBm.
 */
#define PC_RadioGetRssi()                           0


#if defined(_ZBPRO_) && defined(_RF4CE_)

/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter PAN Coordinator register.
 */
# define PC_RadioFrameFilterSetPanCoord(context, panCoord)        do { ((void)(context)); ((void)(panCoord)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter PAN ID register.
 */
# define PC_RadioFrameFilterSetPanId(context, panId)              do { ((void)(context)); ((void)(panId)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter Short Address register.
 */
# define PC_RadioFrameFilterSetShortAddr(context, shortAddr)      do { ((void)(context)); ((void)(shortAddr)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter Extended Address register.
 */
# define PC_RadioFrameFilterSetExtAddr(context, extAddr)          do { ((void)(context)); ((void)(extAddr)); } while(0)


#else /* ! ( _ZBPRO_ && _RF4CE_ ) */

/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter PAN Coordinator register.
 */
# define PC_RadioFrameFilterSetPanCoord(panCoord)                 do { ((void)(panCoord)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter PAN ID register.
 */
# define PC_RadioFrameFilterSetPanId(panId)                       do { ((void)(panId)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter Short Address register.
 */
# define PC_RadioFrameFilterSetShortAddr(shortAddr)               do { ((void)(shortAddr)); } while(0)


/**//**
 * \brief   Simulates for the i386 Radio Hardware Frame Filter Extended Address register.
 */
# define PC_RadioFrameFilterSetExtAddr(extAddr)                   do { ((void)(extAddr)); } while(0)

#endif /* ! ( _ZBPRO_ && _RF4CE_ ) */


#endif /* _BB_PC_RADIO_H */
