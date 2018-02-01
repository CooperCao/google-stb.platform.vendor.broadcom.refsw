/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC structured MPDU definitions.
 *
*******************************************************************************/

#ifndef _BB_MAC_MPDU_H
#define _BB_MAC_MPDU_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"                /* MAC-PIB for MAC-SAP definitions. */

#include "bbMacSapTypesData.h"          /* MCPS-DATA service data types. */

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
# include "bbMacSapTypesBeacon.h"       /* MLME-BEACON service data types. */
#endif

#if defined(_MAC_CONTEXT_ZBPRO_)
# include "bbMacSapTypesAssociate.h"    /* MLME-ASSOCIATE service data types. */
#endif


/************************* DEFINITIONS **************************************************/
/* Constants for sizes of particular PPDU fields.
 * See IEEE 802.15.4-2006, subclauses 6.3, 6.3.3, and table 21. */
#define MAC_PPDU_PHR_SIZE     1     /*!< PPDU.PHR field has 1 octet size. */


/* Constants for offsets of particular PPDU fields.
 * See IEEE 802.15.4-2006, subclauses 6.3, 6.3.3, and table 21. */
#define MAC_PPDU_PHR_OFFSET   0     /*!< PPDU.PHR field offset in PPDU is 0 octets. */
#define MAC_PPDU_PSDU_OFFSET  1     /*!< PPDU.PSDU field offset in PPDU is 1 octet. */


/* Constants for sizes of particular MPDU fields.
 * See IEEE 802.15.4-2006, subclauses 7.2, 7.2.1, 7.2.1.1-7.2.1.9, and figure 41. */
#define MAC_MPDU_MHR_FCF_SIZE        2      /*!< MPDU.MHR.FCF field has 2 octets size. */
#define MAC_MPDU_MHR_DSN_SIZE        1      /*!< MPDU.MHR.DSN field has 1 octet size. */
#define MAC_MPDU_MHR_BSN_SIZE        1      /*!< MPDU.MHR.BSN field has 1 octet size. */
#define MAC_MPDU_MHR_PANID_SIZE      2      /*!< MPDU.MHR.Dst(Src)PANId fields have 2 octets size. */
#define MAC_MPDU_MHR_SHORTADDR_SIZE  2      /*!< MPDU.MHR.Dst(Src)ShortAddress fields have 2 octets size. */
#define MAC_MPDU_MHR_EXTADDR_SIZE    8      /*!< MPDU.MHR.Dst(Src)ExtendedAddress fields have 8 octets size. */
#define MAC_MPDU_MFR_FCS_SIZE        2      /*!< MPDU.MFR.FCS field has 2 octets size. */


/* Constants for offsets of particular MPDU fields.
 * See IEEE 802.15.4-2006, subclauses 7.2, 7.2.1, and figure 41. */
#define MAC_MPDU_MHR_FCF_OFFSET      0      /*!< MPDU.MHR.FCF field offset in MPDU is 0 octets. */
#define MAC_MPDU_MHR_DSN_OFFSET      2      /*!< MPDU.MHR.DSN field offset in MPDU is 2 octets. */


/* Constants for sizes of particular MSDU fields of Beacon frame.
 * See IEEE 802.15.4-2006, subclause 7.2.2.1, and figures 44-51. */
#define MAC_MPDU_MSDU_SUPERFRAMESPEC_SIZE    2      /*!< Superframe Specification field has 2 octets. */
#define MAC_MPDU_MSDU_GTSSPEC_SIZE           1      /*!< GTS Specification field has 1 octet. */
#define MAC_MPDU_MSDU_GTSDIRECTIONS_SIZE     1      /*!< GTS Directions field has 1 octet. */
#define MAC_MPDU_MSDU_GTSDESCRIPTOR_SIZE     3      /*!< GTS Descriptor field has 3 octets. */
#define MAC_MPDU_MSDU_PENDINGADDRSPEC_SIZE   1      /*!< Pending Address Specification field has 1 octet. */


/* Constants for sizes of particular MSDU fields of MAC Command frames.
 * See IEEE 802.15.4-2006, subclauses 7.2.2.4, 7.2.2.4.2, and table 82. */
#define MAC_MPDU_MSDU_CMDID_SIZE             1      /*!< MPDU.MSDU.CommandId field has 1 octet size. */


/* Constants for sizes of particular MSDU fields of Association Request MAC Command frame.
 * See IEEE 802.15.4-2006, subclause 7.3.1, and figure 55. */
#define MAC_MPDU_MSDU_CAPABILITY_INFO_SIZE   1      /*!< Capability Info field has 1 octet size. */


/* Constants for sizes of particular MSDU fields of Association Response MAC Command frame.
 * See IEEE 802.15.4-2006, subclause 7.3.2, and figure 57. */
#define MAC_MPDU_MSDU_ASSOC_SHORTADDR_SIZE   2      /*!< Associated Short Address field has 2 octets size. */
#define MAC_MPDU_MSDU_ASSOC_STATUS_SIZE      1      /*!< Association Status field has 1 octet size. */


/* Constants for sizes of particular MSDU fields of Coordinator Realignment MAC Command frame.
 * See IEEE 802.15.4-2006, subclause 7.3.8, and figure 63.
 * Note that this implementation of MAC does not issue Coordinator Realignment MAC Command
 * with the Channel Page field that was introduces just in IEEE 802.15.4-2006, because
 * such version of this MAC Command is to be issued only by the MLME-START.request
 * primitive while the PAN ID is being changed, but this functionality of
 * MLME-START.request is not implemented. Consequently, this type of MAC Command frame is
 * always generated without the Channel Page field. And when this command is received by
 * this implementation of MAC it is ignored, because this MAC does not implement neither
 * MLME-SCAN.request(Orphan), no MLME-SYNC-LOSS.indication. */
#define MAC_MPDU_MSDU_COORD_SHORTADDR_SIZE   2      /*!< Coordinator Short Address field has 2 octets size. */
#define MAC_MPDU_MSDU_LOGICAL_CHANNEL_SIZE   1      /*!< Logical Channel field has 1 octet size. */
#define MAC_MPDU_MSDU_ORPHAN_SHORTADDR_SIZE  2      /*!< Orphan Short Address field has 2 octets size. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.FrameType subfield.
 * \note    The RF4CE-Controller implements only the Data frame.
 * \note    For compatibility with this implementation of MAC the maximum value of a MAC
 *  frame type is allowed to be 0x7 in order to fit into 3 bits \c frameType subfield of
 *  the MPDU Surrogate identifier.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1.1.1, and table 79.
 */
typedef enum _MacMpduFrameType_t
{
    MAC_MPDU_FRAME_TYPE_BEACON  = 0x0,          /*!< Beacon frame. */
    MAC_MPDU_FRAME_TYPE_DATA    = 0x1,          /*!< Data frame. */
    MAC_MPDU_FRAME_TYPE_ACK     = 0x2,          /*!< Acknowledgment frame. */
    MAC_MPDU_FRAME_TYPE_COMMAND = 0x3,          /*!< MAC Command frame. */
    MAC_MPDU_FRAME_TYPE_MAX_ALLOWED = 0x3,      /*!< Maximum allowed MAC Frame Type value. */
} MacMpduFrameType_t;

#define MAC_MPDU_FRAME_TYPE_START  0        /*!< Frame Type subfield starts at bit #0. */
#define MAC_MPDU_FRAME_TYPE_WIDTH  3        /*!< Frame Type subfield has 3 bits width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.SecurityEnabled subfield.
 * \note    The MAC security is not implemented.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1.1.2.
 */
typedef enum _MacMpduSecurityEnabled_t
{
    MAC_MPDU_SECURITY_DISABLED = 0,                 /*!< MAC Security disabled. */
    MAC_MPDU_SECURITY_ENABLED  = 1,                 /*!< MAC Security enabled. */
} MacMpduSecurityEnabled_t;

#define MAC_MPDU_SECURITY_ENABLED_START  3          /*!< Security Enabled subfield starts at bit #3. */
#define MAC_MPDU_SECURITY_ENABLED_WIDTH  1          /*!< Security Enabled subfield has 1 bit width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.FramePending subfield.
 * \note    This implementation of MAC is able to issue/process MAC frames with Frame
 *  Pending flag set to one only in the case of the Acknowledgment frames
 *  transmitted/received in answer on the Data Request MAC Command frames. This flag is
 *  set to zero and ignored on reception in all other frames and cases.
 * \note    The RF4CE Stack does not implement indirect transmission.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1.1.3.
 */
typedef enum _MacMpduFramePending_t
{
    MAC_MPDU_NO_FRAME_PENDING = MAC_TX_OPTION_DIRECT,       /*!< The sender has no pending data for the recipient. */
    MAC_MPDU_FRAME_PENDING    = MAC_TX_OPTION_INDIRECT,     /*!< The sender has more data for the recipient. */
} MacMpduFramePending_t;

#define MAC_MPDU_FRAME_PENDING_START  4                     /*!< Frame Pending subfield starts at bit #4. */
#define MAC_MPDU_FRAME_PENDING_WIDTH  1                     /*!< Frame Pending subfield has 1 bit width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.AckRequest subfield.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1.1.4.
 */
typedef enum _MacMpduAckRequest_t
{
    MAC_MPDU_NO_ACK_REQUEST = MAC_TX_OPTION_UNACK,      /*!< Acknowledgment is not requested. */
    MAC_MPDU_ACK_REQUEST    = MAC_TX_OPTION_ACK,        /*!< Acknowledgment is requested. */
} MacMpduAckRequest_t;

#define MAC_MPDU_ACK_REQUEST_START  5                   /*!< Ack. Request subfield starts at bit #5. */
#define MAC_MPDU_ACK_REQUEST_WIDTH  1                   /*!< Ack. Request subfield has 1 bit width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.PANIDCompression subfield.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1.1.5.
 */
typedef enum _MacMpduPanIdCompress_t
{
    MAC_MPDU_NO_PAN_ID_COMPRESS = 0,            /*!< PAN IDs differ, or one or both addresses are missed. */
    MAC_MPDU_PAN_ID_COMPRESS    = 1,            /*!< Consider the Dst. PAN ID as the Src. PAN ID also. */
} MacMpduPanIdCompress_t;

#define MAC_MPDU_PAN_ID_COMPRESS_START  6       /*!< PAN ID Compression subfield starts at bit #6. */
#define MAC_MPDU_PAN_ID_COMPRESS_WIDTH  1       /*!< PAN ID Compression subfield has 1 bit width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.Dst/SrcAddressingMode subfield.
 * \note    The code for Reserved addressing mode with value 0x1 is used only in
 *  conditions for MAC frame format validation; it is not used for communication.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2.1.1.6, 7.2.1.1.8, and table 80.
 */
typedef enum _MacMpduAddrMode_t
{
    MAC_MPDU_NO_ADDRESS         = MAC_NO_ADDRESS,               /*!< No address field present. */
    MAC_MPDU_RESERVED_ADDR_MODE = MAC_RESERVED_ADDR_MODE,       /*!< Reserved addressing mode. */
    MAC_MPDU_16BIT_ADDRESS      = MAC_16BIT_ADDRESS,            /*!< 16-bit short address present. */
    MAC_MPDU_64BIT_ADDRESS      = MAC_64BIT_ADDRESS,            /*!< 64-bit extended address present. */
} MacMpduAddrMode_t;

#define MAC_MPDU_DST_ADDR_MODE_START  10        /*!< Dst. Addressing Mode subfield starts at bit #10. */
#define MAC_MPDU_DST_ADDR_MODE_WIDTH   2        /*!< Dst. Addressing Mode subfield has 2 bits width. */
#define MAC_MPDU_SRC_ADDR_MODE_START  14        /*!< Src. Addressing Mode subfield starts at bit #14. */
#define MAC_MPDU_SRC_ADDR_MODE_WIDTH   2        /*!< Src. Addressing Mode subfield has 2 bits width. */


/**//**
 * \brief   Enumeration of values for MPDU.MHR.FCF.FrameVersion subfield.
 * \note    This implementation of MAC:
 *  - allows incoming frames with version codes '2003' as well as '2006' in spite of their
 *    other fields values. The MAC security is not implemented;
 *  - issues Data frames with payload size from zero and up to 118 octets with version
 *    code '2003'. In order to stay compatible with ZigBee PRO Certification Test
 *    TP/154/MAC/DATA-01.h, the MAC does not use code '2006' when issuing Data frames with
 *    payload size longer then 102 octets (aMaxMACSafePayloadSize);
 *  - does not issue Coordinator Realignment MAC Command with the Channel Page field that
 *    was introduces just in IEEE 802.15.4-2006, because such version of this MAC Command
 *    is to be issued only by the MLME-START.request primitive while the PAN ID is being
 *    changed, but this functionality of MLME-START.request is not implemented.
 *    Consequently, this type of MAC Command frame is always generated with version code
 *    '2003'. And when this command is received by this implementation of MAC it is
 *    ignored, because this MAC does not implement neither MLME-SCAN.request(Orphan), no
 *    MLME-SYNC-LOSS.indication.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2.1.1.7, 7.2.3, and table 81.<br>
 *  See ZigBee Document 095436r19 ZB_CSG-ZigBee-IP IEEE 802.15.4 Level Test Specification,
 *  Draft Revision 3.5, April 4, 2011
 */
typedef enum _MacMpduFrameVersion_t
{
    MAC_MPDU_FRAME_VERSION_2003 = 0x0,              /*!< Frame is assembled according to IEEE 802.15.4-2003. */
    MAC_MPDU_FRAME_VERSION_2006 = 0x1,              /*!< Frame is assembled according to IEEE 802.15.4-2006. */
    MAC_MPDU_FRAME_VERSION_MAX_ALLOWED = 0x1,       /*!< Maximum allowed MAC Frame Version value. */
} MacMpduFrameVersion_t;

#define MAC_MPDU_FRAME_VERSION_START  12        /*!< Frame Version subfield starts at bit #12. */
#define MAC_MPDU_FRAME_VERSION_WIDTH   2        /*!< Frame Version subfield has 2 bits width. */


/**//**
 * \brief   Structure for MPDU.MHR.FCF (FrameControl) field.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.1, and figure 42.
 */
typedef union _MacMpduFrameControl_t
{
    BitField16_t                  plain;                    /*!< Plain data. */

    struct
    {
        MacMpduFrameType_t        frameType       : 3;      /*!< Frame Type. */
        MacMpduSecurityEnabled_t  securityEnabled : 1;      /*!< Security Enabled. */
        MacMpduFramePending_t     framePending    : 1;      /*!< Frame Pending. */
        MacMpduAckRequest_t       ackRequest      : 1;      /*!< Ack. Request. */
        MacMpduPanIdCompress_t    panIdCompress   : 1;      /*!< PAN ID Compression. */
        BitField16_t                              : 3;      /*!< Reserved. */
        MacMpduAddrMode_t         dstAddrMode     : 2;      /*!< Dest. Addressing Mode. */
        MacMpduFrameVersion_t     frameVersion    : 2;      /*!< Frame Version. */
        MacMpduAddrMode_t         srcAddrMode     : 2;      /*!< Source Addressing Mode. */
    };
} MacMpduFrameControl_t;

#define MAC_MPDU_FRAME_CONTROL_MASK  0xFC7F     /*!< Mask for used (not reserved) bits
                                                    in the Frame Control Field of MHR. */


/**//**
 * \brief   Macro to assemble MPDU.MHR.FrameControl field from the set of subfields.
 */
#define MAC_MPDU_MAKE_FRAME_CONTROL(                                    \
                frameType,                                              \
                securityEnabled,                                        \
                framePending,                                           \
                ackRequest,                                             \
                panIdCompress,                                          \
                dstAddrMode,                                            \
                frameVersion,                                           \
                srcAddrMode)                                            \
        (((frameType)       << MAC_MPDU_FRAME_TYPE_START)       |       \
         ((securityEnabled) << MAC_MPDU_SECURITY_ENABLED_START) |       \
         ((framePending)    << MAC_MPDU_FRAME_PENDING_START)    |       \
         ((ackRequest)      << MAC_MPDU_ACK_REQUEST_START)      |       \
         ((panIdCompress)   << MAC_MPDU_PAN_ID_COMPRESS_START)  |       \
         ((dstAddrMode)     << MAC_MPDU_DST_ADDR_MODE_START)    |       \
         ((frameVersion)    << MAC_MPDU_FRAME_VERSION_START)    |       \
         ((srcAddrMode)     << MAC_MPDU_SRC_ADDR_MODE_START))


/**//**
 * \brief   Structure for GTS Specification field of the Beacon frame.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.2.1.3, and figure 48.
 */
typedef union _MacMpduGtsSpec_t
{
    BitField8_t      plain;                         /*!< Plain data. */

    struct
    {
        uint8_t      gtsDescriptorCount : 3;        /*!< The number of 3-octet GTS descriptors contained in the GTS List
                                                        field of the beacon frame. If the value of this subfield is
                                                        zero, the GTS Directions field and GTS List field of the beacon
                                                        frame are not present. */

        BitField8_t                     : 4;        /*!< Reserved. */

        Bool8_t      gtsPermit          : 1;        /*!< Equal to the value of macGTSPermit attribute of the originator
                                                        of the beacon frame. */
    };
} MacMpduGtsSpec_t;


/**//**
 * \brief   Structure for Pending Address Specification field of the Beacon frame.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.2.2.1.6, and figure 51.
 */
typedef union _MacMpduPendAddrSpec_t
{
    BitField8_t      plain;                                     /*!< Plain data. */

    struct
    {
        uint8_t      numberOfShortAddressesPending    : 3;      /*!< The number of 16-bit short addresses contained in
                                                                    the Address List field of the beacon frame. */

        BitField8_t                                   : 1;      /*!< Reserved. */

        uint8_t      numberOfExtendedAddressesPending : 3;      /*!< The number of 64-bit extended addresses contained
                                                                    in the Address List field of the beacon frame. */

        BitField8_t                                   : 1;      /*!< Reserved. */
    };
} MacMpduPendAddrSpec_t;


/**//**
 * \brief   Enumeration of MAC Commands identifiers.
 * \note    For compatibility with this implementation of MAC the maximum value of a MAC
 *  Command identifier is allowed to be 0x1F in order to fit into 5 bits \c commandId
 *  subfield of the MPDU Surrogate identifier.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.3, and table 82.
 */
typedef enum _MacMpduCommandId_t
{
    MAC_COMMAND_NOT_COMMAND_FRAME     = 0x00,   /*!< Zero is used instead the Command Id for different frame type. */
    MAC_COMMAND_ASSOCIATION_REQ       = 0x01,   /*!< Association request. */
    MAC_COMMAND_ASSOCIATION_RESP      = 0x02,   /*!< Association response. */
    MAC_COMMAND_DISASSOCIATION_NOTIF  = 0x03,   /*!< Disassociation notification. */
    MAC_COMMAND_DATA_REQ              = 0x04,   /*!< Data request. */
    MAC_COMMAND_PAN_ID_CONFLICT_NOTIF = 0x05,   /*!< PAN ID conflict notification. */
    MAC_COMMAND_ORPHAN_NOTIF          = 0x06,   /*!< Orphan notification. */
    MAC_COMMAND_BEACON_REQ            = 0x07,   /*!< Beacon request. */
    MAC_COMMAND_COORD_REALIGNMENT     = 0x08,   /*!< Coordinator realignment. */
    MAC_COMMAND_GTS_REQ               = 0x09,   /*!< GTS request. */
    MAC_COMMAND_MAX_ID_GUARD,                   /*!< Maximum value of MAC Command Id plus 1. */
} MacMpduCommandId_t;

/* Validate the maximum value of MAC Command Id. For compatibility with this
 * implementation of MAC the maximum value of a MAC Command identifier is allowed to be
 * 0x1F in order to fit into 5 bits \c commandId subfield of the MPDU Surrogate
 * identifier. */
SYS_DbgAssertStatic(MAC_COMMAND_MAX_ID_GUARD - 1 <= 0x1F);


/**//**
 * \brief   Macro to assemble FrameType and CommandId into SurrogateId.
 */
#define MAC_MPDU_MAKE_SURROGATE_ID(frameType, commandId)    \
        (BIT_FIELD_VALUE(commandId, MAC_MPDU_FRAME_TYPE_WIDTH, 8) | BIT_FIELD_VALUE(frameType, 0, 8))


/**//**
 * \brief   Macro to extract FrameType from SurrogateId.
 */
#define MAC_MPDU_GET_FRAME_TYPE(surrogateId)    \
        (GET_BITFIELD_VALUE(surrogateId, 0, MAC_MPDU_FRAME_TYPE_WIDTH))


/**//**
 * \brief   Macro to extract CommandId from SurrogateId.
 */
#define MAC_MPDU_GET_COMMAND_ID(surrogateId)    \
        (GET_BITFIELD_VALUE(surrogateId, MAC_MPDU_FRAME_TYPE_WIDTH, 8 - MAC_MPDU_FRAME_TYPE_WIDTH))


/**//**
 * \brief   Enumeration of MPDU Surrogate identifiers.
 * \details SurrogateId is used to uniquely identify the frame type and the MAC Command by
 *  a single integer code. It helps to eliminate nested switch-case constructions.
 * \note    Acknowledge frame is not delivered to MAC-FE, it is fully internal for MAC-LE.
 * \note    Not all MAC Commands are implemented, only those are implemented that are
 *  supported by an FFD used as PAN coordinator or coordinator in nonbeacon-enabled PAN.
 * \note    RF4CE-Controller does not use SurrogateId, it supports only Data frames.
 */
typedef enum _MacMpduSurrId_t
{
    MAC_SURR_BEACON            = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_BEACON, MAC_COMMAND_NOT_COMMAND_FRAME),
    MAC_SURR_DATA              = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_DATA, MAC_COMMAND_NOT_COMMAND_FRAME),
    MAC_SURR_ASSOCIATION_REQ   = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_ASSOCIATION_REQ),
    MAC_SURR_ASSOCIATION_RESP  = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_ASSOCIATION_RESP),
    MAC_SURR_DATA_REQ          = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_DATA_REQ),
    MAC_SURR_ORPHAN_NOTIF      = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_ORPHAN_NOTIF),
    MAC_SURR_BEACON_REQ        = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_BEACON_REQ),
    MAC_SURR_COORD_REALIGNMENT = MAC_MPDU_MAKE_SURROGATE_ID(MAC_MPDU_FRAME_TYPE_COMMAND, MAC_COMMAND_COORD_REALIGNMENT)
} PACKED MacMpduSurrId_t;


/**//**
 * \brief   Structure for MPDU surrogate object.
 * \details This structure is used to exchange MAC frames between MAC-FE and MAC-LE. It
 *  contains only data fields that may not be ascertained by the second side of
 *  communication between MAC-FE and MAC-LE itself.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2, 7.3.
 */
typedef struct _MacMpduSurr_t
{
    /* 64-bit data. */
    MAC_Address_t          thisAddress;     /*!< (RX/TX) Address of 'this' device. */
    MAC_Address_t          thatAddress;     /*!< (RX/TX) Address of 'that' device. */

    /* 2x16-bit data. */
    MAC_PanId_t            thisPanId;       /*!< (RX/TX) PAN ID of 'this' device. */
    MAC_PanId_t            thatPanId;       /*!< (RX/TX) PAN ID of 'that' device. */

    /* Structured data. */
//    union
//    {
        struct  /* (RX/TX) Parameters of Data frame and Beacon frame. */
        {
            /* 32-bit data. */
            HAL_Symbol__Tstamp_t    timestamp;              /*!< (RX) Frame reception timestamp, in symbol quotients. */

            /* 32-bit data. */
            SYS_DataPointer_t       payload;                /*!< (RX/TX) Data frame or Beacon frame payload. */

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
            /* 16-bit data. */
            MAC_SuperframeSpec_t    superframeSpec;         /*!< (RX/TX) Superframe Specification. */

            /* 16-bit data. */
            PHY_PageChannel_t       channelOnPage;          /*!< (RX) The logical channel and channel page
                                                                on which the beacon was received. */
#endif
        };

#if defined(_MAC_CONTEXT_ZBPRO_)
        struct  /* (RX/TX) Parameters of Association Request command.  */
        {
            /* 8-bit data. */
            MAC_CapabilityInfo_t    capabilityInfo;         /*!< (RX/TX) Capability Info. */
        };

        struct  /* (RX/TX) Parameters of Association Response command. */
        {
            /* 16-bit data. */
            MAC_Addr16bit_t         assocShortAddress;      /*!< (RX/TX) Associated Short Address. */

            /* 8-bit data. */
            MAC_Status_t            associationStatus;      /*!< (RX/TX) Association Status. */
        };

        struct  /* (RX) Parameters of Data Request command. */
        {
            /* 8-bit data. */
            MacMpduFramePending_t   txSendAckWithFp;        /*!< (RX) Value for the FramePending subfield of the ACK
                                                                Response frame that was sent on this Data Request. */

            MacAddrHash_t           thatAddrHash;           /*!< (RX) Hash value of the requesting device Address and
                                                                PAN Id. */
        };

        struct  /* (TX) Parameters of Coordinator Realignment command (except ChannelPage). */
        {
            /* 16-bit data. */
            MAC_PanId_t             panIdentifier;          /*!< (TX) PAN Identifier field. */
            MAC_Addr16bit_t         coordShortAddress;      /*!< (TX) Coordinator Short Address. */
            MAC_Addr16bit_t         orphanShortAddress;     /*!< (TX) Orphan Short Address. */

            /* 8-bit data. */
            PHY_Channel_t           logicalChannel;         /*!< (TX) Logical Channel. */
        };
#endif /* _MAC_CONTEXT_ZBPRO_ */
//    };

    /* 8-bit data. */
    MAC_AddrMode_t                thisAddrMode;     /*!< (RX/TX) Addressing mode of 'this' device. */
    MAC_AddrMode_t                thatAddrMode;     /*!< (RX/TX) Addressing mode of 'that' device. */

    union
    {
        MacSeqNum_t               seqNumber;        /*!< (RX/TX) MHR Sequence Number field. */
        MAC_Dsn_t                 dsn;              /*!< (RX/TX) Data, MAC Command, or Acknowledge frame SeqNum. */
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
        MAC_Bsn_t                 bsn;              /*!< (RX/TX) Beacon frame SeqNum. */
#endif
    };

    MAC_DECLARE_CONTEXTS_SET( contexts );           /*!< (RX) Destination MAC Contexts set. */
    MacMpduAckRequest_t           ackRequest;       /*!< (RX/TX) Ack. Request subfield value. */

    union
    {
        PHY_LQI_t                 lqi;              /*!< (RX) Frame LQI value from radio. */
        MAC_MaxFrameRetries_t     retries;          /*!< (TX) Number of retransmission attempts. */
    };

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    MacMpduSurrId_t               surrogateId;      /*!< (RX/TX) MPDU Surrogate identifier. */
#endif

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    MacMpduSecurityEnabled_t      securityEnabled;  /*!< (RX) SecurityEnabled bit of the Frame Control. */
    MacMpduFrameVersion_t         frameVersion;     /*!< (RX) FrameVersion bits of the Frame Control. */
    MAC_Status_t                  status;           /*!< (RX) Security processing status. */
#endif
#if defined(_MAC_CONTEXT_ZBPRO_)
    MAC_SecurityLevel_t           securityLevel;    /*!< (RX/TX) The security level to be used. */

    MAC_SecurityParams_t          securityParams;   /*!< (RX/TX) Security parameters. They are ignored if the
                                                        SecurityLevel is set to zero. */
    MAC_FrameCounter_t            frameCounter;     /*!< (RX/TX) Security Frame Counter. */
#endif

} MacMpduSurr_t;


/**//**
 * \brief   Data type for entry point to the MPDU Constructor callback function.
 * \param[in]   mpduSurr    Pointer to an empty MPDU Surrogate object.
 * \details Functions of this type must be provided by all the MAC-FE Request Processors;
 *  these functions are used to construct MPDU Surrogate objects to be transmitted.
 */
typedef void (*MacMpduConstructor_t)(MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_MPDU_H */

/* eof bbMacMpdu.h */