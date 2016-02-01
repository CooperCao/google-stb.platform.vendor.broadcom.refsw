//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//  Filename:       StubInterface.h
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 06, 2004
//
//****************************************************************************
//  Description:
//  C to C++ Interface.
//****************************************************************************
#include "bcmtypes.h"
#include "EnetPkt.h"
#include "dsg_api.h"

#ifdef __cplusplus
extern "C" {
#endif

	// These functions must be provided by the eSTB
	// eCM expects these functions to be present
	void DsgScanSuccess( void );
	void DsgFullScanFail( void );
	void DsgRxDCD( int len, uint8 * dcd );
	void DsgUpstreamChange (DsgEcmUsSettings *pDsgEcmUsSettings);
	void DsgUcid( uint8 uc_id  );
	void DsgTimerEvent( uint8 tm_event );
	bool DsgTunnelPkt( ETHERNET_PACKET *pkt );
	void DsgDccInitType( uint8 tm_event );
	void DsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations, uint8 maxCpe );
	void DsgEcmResetIndication(uint8 eCmResetType);
	void DsgEcmMacReinitEvent(void);

	// Set_docsDevSwFilename(const char *fileName)
	// Get_docsDevSwFilename(char *fileName)
	//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
	//
	// Parameters:
	//      fileName - Pinter to null terminated file name ascii string.
	//
	// Returns:
	//      true - All is well
	//		False - Something didn't work
	//
	bool Set_docsDevSwFilename(const char *fileName);
	bool Get_docsDevSwFilename(char *fileName);
	void DsgDocsisEvent (int eventCode, char *pEventText);

#ifdef __cplusplus
}
#endif

