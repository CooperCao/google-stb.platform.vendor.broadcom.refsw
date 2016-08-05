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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   PD-DATA and PD-DATA-ACK services data types definition.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_PHY_SAP_TYPES_DATA_H
#define _BB_PHY_SAP_TYPES_DATA_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhyBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Data types for the PHY packet format.
 */
/**@{*/
/**//**
 * \brief   Data type for single Octet (8-bit byte).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, table 16.
 */
typedef HAL_Radio__Octet_t  PHY_Octet_t;
SYS_DbgAssertStatic(sizeof(PHY_Octet_t) == 1);

/**//**
 * \brief   Data type for PHY header (PHR) of a packet.
 * \details The PHR includes the following fields:
 *  - bits 6..0 - FrameLength[6..0] - specifies the total number of octets contained in the PSDU.
 *  - bit 7 - Reserved[0] - must be set to zero on transmission and ignored on reception.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, figure 16.
 */
typedef HAL_Radio__PHR_t  PHY_PHR_t;
SYS_DbgAssertStatic(sizeof(PHY_PHR_t) == 1);

/**//**
 * \brief   Data type for the Frame Length field of the PHY header (PHR).
 * \details The Frame Length field is 7 bits in length and specifies the total number of octets contained in the PSDU
 *  (i.e., PHY payload). It is a value between 0 and aMaxPHYPacketSize (127).
 * \details For the MAC mode of operation of PHY the Frame Length value must belong to one of allowed ranges:
 *  - 0 to 4 - reserved. Must not be used for transmission. Rejected on reception.
 *  - 5 - PSDU is expected to contain MSDU of an Acknowledgment frame.
 *  - 6 to 8 - reserved. Must not be used for transmission. Rejected on reception.
 *  - 9 to aMaxPHYPacketSize (127) - PSDU is expected to contain MSDU of a Beacon, Data, or Command frame (i.e., except
 *      the Acknowledgment frame).
 *
 * \details For the pure PHY mode of operation of PHY the Frame Length value is allowed to have arbitrary value in the
 *  range from 0 to aMaxPHYPacketSize (127).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, 6.3.3, table 21.
 */
typedef HAL_Radio__FrameLen_t  PHY_FrameLen_t;
SYS_DbgAssertStatic(sizeof(PHY_FrameLen_t) == 1);
/**@}*/

#endif /* _BB_PHY_SAP_TYPES_DATA_H */
