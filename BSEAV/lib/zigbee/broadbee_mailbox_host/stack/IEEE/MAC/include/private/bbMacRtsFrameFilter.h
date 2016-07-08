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
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacRtsFrameFilter.h $
*
* DESCRIPTION:
*   MAC Real Time Service Frame Filter implementation.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/

#ifndef _BB_MAC_RTS_FRAME_FILTER_H
#define _BB_MAC_RTS_FRAME_FILTER_H

/************************* INCLUDES ***********************************************************************************/
#include "bbHalRadio.h"

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \name    Set of variables used for Beacon, Data, and Command frames filtering and preprocessing.
 */
/**@{*/
/**//**
 * \brief   Actual value of the Sequence Number (SN) field of the received frame.
 * \note    This value must be ignored if the Frame Filter rejected the received frame.
 * \details This value is set after the SN field of the received frame and used for issuing ACK frame if its requested.
 */
extern Octet_t  MRTS_FF__BDC_SN;

/**//**
 * \brief   Actual modified value of the Ack. Request (AR) subfield of the Frame Control field of the received frame.
 * \note    This flag must be ignored if the Frame Filter rejected the received frame.
 * \details This flag is set after the AR subfield of the received frame, but it is forced to zero in the following
 *  cases:
 *  - When the received frame has broadcast destination address.
 *  - When the received frame is a Beacon frame.
 *
 * \details Data and Command frames are considered broadcast in following cases:
 *  - The frame contains Short Dst. Address field and it is set to 0xFFFF.
 *  - The frame contains Short Dst. Address field, it is less than 0xFFFE, and the Dst. PAN Id equals to 0xFFFF.
 *
 * \note    Frames containing Short Dst. Address equal to 0xFFFE (unassigned short address) are rejected.
 * \note    Frames containing Extended Dst. Address are considered unicast even if Dst. PAN Id equals 0xFFFF.
 * \note    Data and Command frames having no destination address considered unicast for the PAN Coordinator.
 */
extern Bool8_t  MRTS_FF__BDC_AR;

/**//**
 * \brief   Flag indicating reception of valid Data Request command and existence of pending data.
 * \note    This flag must be ignored if the Frame Filter rejected the received frame.
 * \details This flag is set if the following conditions are met:
 *  - A valid Data Request command was received.
 *  - The received command is unicast and directed to this node.
 *  - The received command contains Ack. Request subfield set to one.
 *  - The received command explicitly contains valid unicast Src. Address.
 *  - There is data or command frame in the indirect transactions queue on this node for the remote node that sent this
 *      command.
 *
 * \details The explicitly specified source address is considered unicast in the following cases:
 *  - The frame contains Extended Src. Address (even if the Src. PAN Id or Dst. PAN Id equals 0xFFFF).
 *  - The frame contains Short Src. Address that is less than 0xFFFE and explicit Src. PAN Id that is less than 0xFFFF.
 *  - The frame contains Short Src. Address that is less than 0xFFFE and does not contain explicit Src. PAN Id. But the
 *      frame contains explicit Dst. PAN Id and it less than 0xFFFF.
 *  - The frame contains Short Src. Address that is less than 0xFFFE and does not contain neither Src. PAN Id, nor
 *      Dst. PAN Id (and Dst. Address). And this node is the PAN Coordinator.
 */
extern Bool8_t  MRTS_FF__BDC_PD;

/**//**
 * \brief   Hash value of the Src. Address of the received Data Request command.
 */
extern uint8_t  MRTS_FF__DataReq_SrcAddrHash;

/**//**
 * \brief   Mask of MAC contexts to which to deliver the received frame.
 * \note    This value must be ignored if the Frame Filter rejected the received frame.
 * \details If a received frame successfully passed the Frame Filter, it shall be delivered to one or more MAC
 *  higher-level layer Contexts for processing. Each single bit in the mask correspond to one of MAC Contexts with the
 *  same number as the bit position. Up to eight contexts supported.
 */
extern uint8_t  MRTS_FF__BDC_Contexts;
/**@}*/

/**//**
 * \name    Set of variables used for ACK frames filtering and preprocessing.
 */
/**@{*/
/**//**
 * \brief   Reference value of the Sequence Number (SN) field of the expected ACK frame.
 * \details When Frame Filter is in the waiting for ACK state it passes only valid ACK frame having matching SN. All
 *  other frames are rejected.
 */
extern Octet_t  MRTS_FF__ACK_SN;

/**//**
 * \brief   Actual value of the Frame Pending (FP) subfield of the Frame Control field of the received ACK frame.
 * \note    This flag must be ignored if the Frame Filter rejected the received ACK frame.
 * \details If the received ACK frame is passed by the Frame Filter, its FP subfield is saved in this variable.
 * \details The FP field of other frame types is not put here, it is delivered to the higher-level layer within the
 *  Frame Control field of the received frame - the higher-level layer makes its own decision how to process this flag.
 * \note    Processing of the FP subfield of received ACK frames and frames of different types is performed in different
 *  ways:
 *  - When an ACK frame is received with FP set to one, and it's the ACK frame on the Data Request command sent by this
 *      node, the Radio is left in the RX_ON state for specified period waiting for incoming Data or Command frame.
 *  - When a Beacon frame is received with FP set to one, this fact is ignored. The FP field of Beacon frames is used
 *      only in beacon-enabled PAN (it means that the coordinator has broadcast pending frame).
 *  - When a Data or a Command frame is received with FP set to one, it instructs this node to perform automatic polling
 *      (if such polling is enabled with macAutoRequest).
 */
extern Bool8_t  MRTS_FF__ACK_FP;
/**@}*/

/**//**
 * \name    Set of the Frame Filtering functions.
 */
/**@{*/
/**//**
 * \brief   Performs MAC frame partial parsing and filtering while not waiting for an ACK frame.
 * \details This function has implicit parameter which must be assigned by the caller prior to call it:
 *  - PHY_FrmBuf__RX_PPDU   - array of bytes with PPDU of the received packet. Contains PHR field at PPDU[0] and from 0
 *      to 127 bytes of PSDU (MPDU) field starting from PPDU[1]. Value of the PHR field is from 0 to 127.
 *
 * \return  TRUE if a different type of frame is received except the ACK frame (i.e., either Beacon, Data, or Command
 *  frame) and all of the filtering requirements are satisfied; FALSE otherwise.
 * \details This function parses the received frame MHR (Frame Control, Sequence Number and Addressing Fields) and
 *  applies frame filtering. If it's either Beacon or Data or Command frame and the frame filter was passed successfully
 *  this function saves results of MHR parsing and returns TRUE. The following properties of the received frame are
 *  saved:
 *  - Frame Control field as it was received within the frame
 *  - Sequence Number field of the frame
 *  - Decompressed Dst. and Src. PAN Id
 *  - Normalized Dst. and Src. Addresses
 *  - Ack. Request subfield, modified after analysis of the Frame Control and Addressing fields
 *  - Set of MAC contexts to which this frame is to be delivered
 */
Bool8_t MRTS_FF__Filter_BDC(void);

/**//**
 * \brief   Performs MAC frame filtering while waiting for an ACK frame with specific Sequence Number.
 * \details This function has implicit parameter which must be assigned by the caller prior to call it:
 *  - PHY_FrmBuf__RX_PPDU   - array of bytes with PPDU of the received packet. Contains PHR field at PPDU[0] and from 0
 *      to 127 bytes of PSDU (MPDU) field starting from PPDU[1]. Value of the PHR field is from 0 to 127.
 *
 * \return  TRUE if an ACK frame is received and all of the filtering requirements are satisfied; FALSE otherwise.
 */
Bool8_t MRTS_FF__Filter_ACK(void);
/**@}*/

#endif /* _BB_MAC_RTS_FRAME_FILTER_H */
