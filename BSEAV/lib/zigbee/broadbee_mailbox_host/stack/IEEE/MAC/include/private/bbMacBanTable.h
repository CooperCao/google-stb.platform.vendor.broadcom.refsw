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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacBanTable.h $
*
* DESCRIPTION:
*   MAC-SAP ban table private definitions.
*
* $Revision: 3816 $
* $Date: 2014-10-02 07:46:11Z $
*
*****************************************************************************************/
#ifndef _BB_MAC_BAN_TABLE_H
#define _BB_MAC_BAN_TABLE_H

/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesBanTable.h"
#include "bbPhySapForMAC.h"

/************************* DEFINITIONS **************************************************/
#ifdef _MAC_BAN_TABLE_
/**//**
 * \brief Ban table entry type.
 */
typedef struct _MAC_BanTableEntry_t
{
    MAC_Addr64bit_t extAddr;
    MAC_Addr16bit_t shortAddr;
    linkCost_t      cost;
} MAC_BanTableEntry_t;

/**//**
 * \brief Ban table descriptor type.
 */
typedef struct _MAC_BanDescr_t
{
    MAC_BanTableEntry_t table[_MAC_BAN_TABLE_SIZE_];
    bool                defaultAction;
} MAC_BanDescr_t;

/************************************************************************************//**
  \brief Sets ban table into default state.
****************************************************************************************/
void macBanTableReset(void);

#define MAC_BAN_TABLE_RESET() \
    macBanTableReset();

/************************************************************************************//**
  \brief Finds link into ban table by given address and correct frame lqi if needed.
  \param[in] mode - address mode.
  \param[in] addr - mac address.
  \param[out] lqi - lqi pointer to be corrected.
  \result true if given address is banned and false otherwise.
****************************************************************************************/
bool macBanTableIsAddrBanned(const MAC_AddrMode_t mode, const MAC_Address_t *const addr, PHY_Lqi_t *const lqi);

#define MAC_BAN_TABLE_IF_FRAME_BANNED(addrMode, addr, lqiStorage) \
    (MAC_MPDU_NO_ADDRESS != (addrMode) \
        && macBanTableIsAddrBanned((addrMode), (addr), (lqiStorage)))
#else
/* Stubs for mac ban functionality if it's disabled */
# define MAC_BAN_TABLE_RESET()
# define MAC_BAN_TABLE_IF_FRAME_BANNED(addrMode, addr, lqiStorage)  (false)
#endif /* _MAC_BAN_TABLE_ */
#endif /* _BB_MAC_BAN_TABLE_H */