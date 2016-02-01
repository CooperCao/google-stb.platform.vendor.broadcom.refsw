/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacLeFrameComposer.h $
*
* DESCRIPTION:
*   MAC-LE Frame Composer interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_FRAME_COMPOSER_H
#define _BB_MAC_LE_FRAME_COMPOSER_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Composes the MAC Frame to be transmitted.
 * \param[in]   mpduSurr    Pointer to the MPDU Surrogate structure containing the type
 *  and parameters of a MAC Frame to be composed and transmitted. Must not be NULL.
 * \return  The size of composed MPDU (PSDU) frame, in octets.
 * \details
 *  This function takes given MPDU Surrogate structure and composes the MPDU frame to be
 *  transmitted according to the MAC Frames formats described in the IEEE 802.15.4-2006,
 *  subclauses 7.2, 7.3. The MPDU Surrogate object is provided by the caller and pointed
 *  with \p mpduSurr argument. The function returns the length of the constructed MPDU, in
 *  octets; this value is used by the caller as the PHR field of the PHY packet to be
 *  transmitted.
 * \details
 *  The resulting MPDU frame (PSDU) is constructed by this function directly in the Radio
 *  hardware TX-Buffer starting from its origin; the PPDU.PHR value is also assigned to
 *  the Radio hardware. To access the TX-Buffer this function uses the Radio HAL.
 * \note
 *  The value of FCS subfield of MPDU.MFR field (i.e., the CRC value of the frame to be
 *  transmitted) is not calculated by this function and is not written into the hardware
 *  TX-Buffer; the hardware is responsible for calculating the MPDU.MFR.FCS and appending
 *  it at the end of the PHY packet. Nevertheless, the returned value of MPDU length
 *  (i.e., PHR - the length of PSDU) includes the length of this omitted FCS subfield.
 * \details
 *  This function takes all necessary parameters of the constructed MPDU from the MPDU
 *  Surrogate object provided by the caller and pointed with \p mpduSurr; this function
 *  does not take any other variable information itself to construct the final MPDU
 *  (including the Source Address, the frame Sequence Number, etc.). Still, all subfields
 *  of the MPDU being composed that are constants or may be unambiguously calculated from
 *  other fields are assigned by this function itself according to the IEEE 802.15.4-2006,
 *  subclauses 7.2, 7.3 (all such fields are not given with MPDU Surrogate parameters; for
 *  example, Security Enabled, Frame Pending, PAN ID Compression). This function never
 *  overrides parameters that are strictly defined in the MPDU Surrogate even if they are
 *  constants for some particular types of MAC Frames; it is the responsibility of the
 *  caller (the corresponding MAC-FE Request/Response Processor) to define the correct
 *  values for these parameters, and this function just serializes them according to the
 *  Standard.
 * \details
 *  If the frame to be transmitted involves a payload of variable length (in the case of
 *  Data frame or Beacon frame) such payload must be provided by the caller in the dynamic
 *  memory and linked to the MPDU Surrogate with \p payload parameter; if the payload has
 *  zero size then the \c payload parameter of the MPDU Surrogate must be empty. If the
 *  frame has a payload, then the caller (the higher layer) is responsible for freeing the
 *  allocated chunk of dynamic memory after this function returns. Instead of truly
 *  dynamically allocated payload the statically allocated payload may be used.
 * \details
 *  This function is called by the READY_TX task handler (that was scheduled by the
 *  MAC-LE-READY-TX.indication).
 * \details
 *  The list of MAC Frame types supported depends on the selected build configuration. The
 *  following build configurations are implemented:
 *  - D --- Dual-context MAC for ZigBee PRO and RF4CE-Target;
 *  - Z --- Single-context MAC for ZigBee PRO;
 *  - T --- Single-context MAC for RF4CE-Target;
 *  - C --- Single-context MAC for RF4CE-Controller.
 *
 *  The following MAC Frame types are supported for composing for these configurations:
 *  | MAC Frame                     | D | Z | T | C |
 *  |:----------------------------- |:-:|:-:|:-:|:-:|
 *  | Beacon frame                  | + | + | + | - |
 *  | Data frame                    | + | + | + | + |
 *  | Acknowledge frame             | - | - | - | - |
 *  | MAC Commands:                 |   |   |   |   |
 *  | Association request           | + | + | - | - |
 *  | Association response          | + | + | - | - |
 *  | Disassociation notification   | - | - | - | - |
 *  | Data request                  | + | + | - | - |
 *  | PAN ID conflict notification  | - | - | - | - |
 *  | Orphan notification           | - | - | - | - |
 *  | Beacon request                | + | + | + | - |
 *  | Coordinator realignment       | + | + | - | - |
 *  | GTS request                   | - | - | - | - |
 * \note
 *  This function is not used for composing the Acknowledge frame which is composed
 *  directly by the MAC-LE Real-Time Dispatcher.
 * \note
 *  For the case of platform with the single Frame RX/TX-Buffer shared by receiver and
 *  transmitter the PPDU.PHR and PPDU.PSDU are stored in the MAC Static Memory in order to
 *  provide ability to restore the content of the TX-Buffer when in need to retransmit the
 *  composed frame.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2, 7.3.
*****************************************************************************************/
MAC_PRIVATE PHY_PsduLength_t macLeFrameComposeMpdu(const MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_LE_FRAME_COMPOSER_H */