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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacLeFrameParser.h $
*
* DESCRIPTION:
*   MAC-LE Frame Parser interface.
*
* $Revision: 3455 $
* $Date: 2014-09-04 07:56:49Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_FRAME_PARSER_H
#define _BB_MAC_LE_FRAME_PARSER_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* DEFINITIONS **************************************************/
typedef enum _MacLeFrameIndicate_t
{
    MAC_LE_FRAME_DROP     = 0,      /* Drop the parsed frame due to some reasons. */

    MAC_LE_FRAME_INDICATE = 1,      /* Indicate the parsed frame to the higher layer. */

} MacLeFrameIndicate_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Parses the received MAC Frame.
 * \param[out]  mpduSurr    Pointer to the MPDU Surrogate structure allocated by the
 *  MAC-FE to be filled with the parsed newly received MAC Frame. Must not be NULL.
 *  Allowed not to be initialized (not emptied) prior to calling this function.
 * \param[in]   extBuffer   Pointer to the array of 32-bit words containing the PPDU of a
 *  packet to be parsed; or NULL if the packet shall be read-out from the Radio RX-Buffer
 *  by this function itself. This parameter is not implemented by the RF4CE Controller.
 * \return  INDICATE if the received frame is parsed successfully; DROP if a MAC Frame
 *  format error is encountered (including the case of inappropriate frame length), or if
 *  there is not enough dynamic memory to allocate for the payload received within the
 *  frame, or if the received MAC Frame type or MAC Command is not processed by this
 *  implementation or build configuration of the MAC.
 * \details
 *  This function takes given MPDU Surrogate structure and fills its parameters with
 *  values of corresponding subfields of the received MPDU frame being parsed according to
 *  the MAC Frames formats described in the IEEE 802.15.4-2006, subclauses 7.2, 7.3. An
 *  empty MPDU Surrogate object is provided by the caller and pointed with \p mpduSurr
 *  argument. The length of the received MPDU frame is taken directly from the Radio
 *  hardware. The function returns status of MAC Frame parsing: INDICATE if it was parsed
 *  successfully, and DROP otherwise.
 * \details
 *  The MPDU frame (PSDU) being parsed is obtained by this function directly from the
 *  Radio hardware RX-Buffer; to access the RX-Buffer this function uses the HAL API.
 * \note
 *  This particular implementation takes into account that the first byte in the Radio
 *  hardware RX-Buffer contains value of the PHR field (the total length of the received
 *  MPDU, in octets) of the received PHY packet, and the MPDU (i.e., PSDU) starts from the
 *  second byte of the RX-Buffer.
 * \note
 *  The MFR field of MPDU containing the value of FCS subfield (i.e., the CRC value of the
 *  frame received) is not read-out from the RX-Buffer and is not validated by this
 *  function; the hardware is responsible for validating the MHR value, and the caller is
 *  responsible for discovering the result of such validation. Nevertheless, the value of
 *  MPDU length given with the PHR read from the Radio hardware RX-Buffer includes the
 *  length of this omitted FCS subfield.
 * \details
 *  This function fills all parameters of the MPDU Surrogate object provided by the caller
 *  and pointed with \p mpduSurr from the received MPDU frame; this function does not take
 *  any other variable information itself to fill any parameters of the MPDU Surrogate
 *  (including the Destination Address, etc.). This function does not omit any parameters
 *  of MPDU Surrogate if they are defined with corresponding subfields of received MPDU
 *  frame even if they are to be ignored for some particular types of MAC Frames; it is
 *  the responsibility of the caller (the corresponding MAC-FE Indication Processor) to
 *  decide to use or to ignore the MPDU Surrogate parameters, and this function just
 *  deserializes them according to the Standard. Still, some parameters of the received
 *  MPDU that are not used by higher layers are not included into the MPDU Surrogate (for
 *  example, Security Enabled, Acknowledge Request, PAN ID Compression).
 * \details
 *  If the frame received contains a payload of variable length (in the case of Data frame
 *  or Beacon frame) such payload is saved into the newly allocated chunk of dynamic
 *  memory and linked to the MPDU Surrogate; if the payload has zero size then the
 *  \c payload parameter of the MPDU Surrogate is emptied. The caller (the higher layer)
 *  is responsible for freeing the allocated chunk of dynamic memory after this function
 *  returns, if its returned value is INDICATE.
 * \note
 *  The caller may not initialize the \c payload parameter of the MPDU Surrogate
 *  initially. It is emptied or assigned by this function in any case if the function
 *  returns INDICATE as the result.
 * \details
 *  This function returns INDICATE if the received MPDU frame is parsed successfully and
 *  the payload is saved into the dynamic memory. This function returns DROP if the MPDU
 *  frame has strongly invalid format, if its length according to the value of read PHR
 *  field contradicts with the Standard for particular frame types, or if the dynamic
 *  memory was not allocated for any reason. In the case when this function returns DROP,
 *  the caller shall cancel this MPDU indication.
 * \note
 *  If this function returns DROP, the value of \c payload parameter is undefined, and no
 *  dynamic memory is allocated (or left allocated). In this case the caller shall not try
 *  to free the dynamic memory that is probably pointed with the \c payload parameter,
 *  because such memory was not allocated by this function indeed, and the \c payload
 *  parameter may stay uninitialized from the previous session.
 * \note
 *  The beacon frame is indicated (if it is valid) even in the case of lack of memory to
 *  save the beacon payload received in the frame. In such situation the beacon frame is
 *  indicated without payload (i.e., with empty payload) and used just for PAN Descriptor
 *  assigning within the MLME-SCAN.confirm(ActiveScan) results.
 * \details
 *  This function is called by the RECEIVE task handler (that was scheduled by the
 *  MAC-LE-RECEIVE.indication).
 * \note
 *  The RX-Buffer is not released (reenabled for new reception) by this function itself.
 *  It will be enabled by the MAC-LE-RECEIVE.response called by the MAC-FE after this
 *  function returns.
 * \note
 *  This function assigns also the LQI value of the received MPDU. The starting timestamp,
 *  and the Logical Channel and the Channel Page on which the MPDU was received are not
 *  assigned by this function; they will be assigned by the MAC-LE-RECEIVE.response.
 * \details
 *  The list of MAC Frame types supported depends on the selected build configuration. The
 *  following build configurations are implemented:
 *  - D --- Dual-context MAC for ZigBee PRO and RF4CE-Target;
 *  - Z --- Single-context MAC for ZigBee PRO;
 *  - T --- Single-context MAC for RF4CE-Target;
 *  - C --- Single-context MAC for RF4CE-Controller.
 *
 *  The following MAC Frame types are supported for parsing for these configurations:
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
 *  | Orphan notification           | + | + | - | - |
 *  | Beacon request                | + | + | + | - |
 *  | Coordinator realignment       | - | - | - | - |
 *  | GTS request                   | - | - | - | - |
 * \note
 *  This function is not used for parsing the Acknowledge frame, because it is processed
 *  by the MAC-LE Real-Time Dispatcher internally and is not indicated to the MAC-FE.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2, 7.3.
*****************************************************************************************/
MAC_PRIVATE MacLeFrameIndicate_t macLeFrameParseMpdu(MacMpduSurr_t *const mpduSurr /* , uint32_t *const extBuffer ); */
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
                                                                                      , uint32_t *const extBuffer
#endif
                                                                                                                  );


#endif /* _BB_MAC_LE_FRAME_PARSER_H */