//****************************************************************************
//
// Copyright (c) 2007 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       CmDhcpV6MsgACT.cpp
//  Author:         David Pullen
//  Creation Date:  May 17, 2007
//
//****************************************************************************


//********************** Include Files ***************************************

// My api and definitions...
#include "CmDhcpV6MsgACT.h"

#include "DhcpV6ClientIf.h"

#include "DhcpV6CableLabsVendorSpecificInfoOption.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


BcmCmDhcpV6MsgACT::BcmCmDhcpV6MsgACT() :
    BcmMsgACT(),
    pfDhcpV6ClientIf(NULL)
{
}


BcmCmDhcpV6MsgACT::~BcmCmDhcpV6MsgACT()
{
}

void BcmCmDhcpV6MsgACT::HandleEvent(const BcmCompletionEvent &event_code)
{
    // NOTE:  These events must be processed synchronously!  The DHCP packet
    // object lives on the stack, and the PacketIsNotAcceptable() result code
    // will only be used if it is set in this context.
    switch (event_code)
    {
        case BcmDhcpV6ClientIf::kEventAdvertisePacketReceived:
        case BcmDhcpV6ClientIf::kEventReplyPacketReceived:
            {
                // For the eSTB, 
                // 
                //   - The packet must contain:
                //      - IA_NA option (3) but it is checked in DHCPv6 ClientIf
                // If we need to check for any special options that DHCPv6 does not check
				// then this is the place to do it. Follow the example below.
				/*
                bool somethingIsMissing = false;

                BcmDhcpV6Packet *pDhcpV6Packet = (BcmDhcpV6Packet *) pfMsgParam;

                BcmOctetBuffer clVendorBuffer(128), clVendorOverlay(0);
                BcmDhcpV6OptionsList clVendorOptions;

                BcmDhcpV6CableLabsVendorSpecificInfoOption cableLabsVendorSpecificInfoOption;
                if( pDhcpV6Packet->Options().Get(cableLabsVendorSpecificInfoOption) == true )
                {
                    char * pMissingOption = NULL;

                    if (cableLabsVendorSpecificInfoOption.Options().IsPresent(kDhcpV6OptionTftpServerAddresses) == false)
                    {
                        pMissingOption = "TFTP server (32)";
                        somethingIsMissing = true;
                    }

                    if( pMissingOption != NULL ) 
                    {
                        gLogMessageRaw << "DHCPv6 Advertise or Reply missing required option " << pMissingOption << endl;
                    }
                }
                else
                {
                    somethingIsMissing = true;
                }

                if (somethingIsMissing == true)
                {
                    pfDhcpV6ClientIf->PacketIsNotAcceptable();
                }
				*/
            }
            break;

        case BcmDhcpV6ClientIf::kEventReplyPacketReceived_Renewing:
        case BcmDhcpV6ClientIf::kEventReplyPacketReceived_Rebinding:
           
            break;

        default:
            break;
    }
}


