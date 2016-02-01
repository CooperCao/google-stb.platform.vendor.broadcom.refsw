//****************************************************************************
//
// Copyright (c) 2004-2011 Broadcom Corporation
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
//
//  Filename:       CmDavicApiSocketInterface.h
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  July 14, 2011
//
//****************************************************************************
//  Description:
//  C to C++ Interface.
//****************************************************************************
#include "EstbRemoteInterfaceStub.h"

#include "bcmtypes.h"
#include "EnetPkt.h"
#include "davic_api.h"


#ifdef __cplusplus
extern "C" {
#endif

	int BcmDavicStart(void);
	int BcmDavicStop(void);
	int BcmDavicEnableUpstream(unsigned int twoWayMode);
	int BcmDavicSetMode ( unsigned int mode );
	int BcmDavicOpenTunnel(unsigned int vpivci);
	int BcmDavicCloseTunnel(unsigned int vpivci);
	int BcmDavicSetConfig ( unsigned int param, void *data, int dataLen );
	int BcmDavicGetConfig ( unsigned int param, void *data, int dataLen );

	// Callback functions 
	void BcmDavicSetTunnelPktCallback( PFN_DAVIC_RX_TUNNELDATA_HANDLER rxPktCallback);
	bool BcmDavicRxTunnelPkt(ETHERNET_PACKET* pBuffer);

	void BcmDavicProcessEcmNotification( uint8* pBuf );
	void BcmDavicSetStatusCallback( PFN_DAVIC_STATUS_DATA_HANDLER statusCallback);

	void NtohlDavicConnectionSettings(DavicConnectionSettings *data);
	void NtohlDavicStateInfo(DavicStateInfo *data);
	void NtohlDavicStateChangeInfo(DavicStateChangeMapSets *data);
#ifdef __cplusplus
}
#endif

