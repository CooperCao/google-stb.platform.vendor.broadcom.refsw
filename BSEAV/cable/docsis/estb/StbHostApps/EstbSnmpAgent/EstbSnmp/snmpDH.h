//**************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//
//
//**************************************************************************
//    Filename: SnmpDH.h
//    Author:   Thomas Clack
//    Creation Date: 21-December-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Diffie-Hellman key exchange for SNMPv3 and DOCSIS 1.1
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPDH_H
#define SNMPDH_H

#include "BcmString.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <list>
#else
#include <list.h>
#endif


//defines for keyType
#define AUTHKEY 1
#define PRIVKEY 2

class SnmpDh 
{
  public:
    SnmpDh ();
	SnmpDh (BcmString &securityName);
   ~SnmpDh ();
          
    int        GenMyPublic(BcmString &mypublic);
	int        GenV3Key(int keyType, unsigned char *authKey);
	int        GenV3KeyChange( BcmString & v3Key);
	int        GenSecretKey();
    BcmString* GetSecurityName();
	BcmString* GetMyPublic();
	BcmString* GetMgrPublic();

	void       SetMgrPublic(BcmString &mgrpublic);

	void Dump(char *comment,BcmString & bstr);
    void Dump(char *comment,unsigned char *str,int len);
    void Dump(char *comment, char *str,int len);
    void DumpAll(char * comment);


  private: 
	BcmString fSecurityName;
    BcmString fMgrPublic;
	BcmString fMyRandom;
    BcmString fMyPublic; 
	BcmString fSecretKey;
	//BcmString fAuthKey;
	//BcmString fPrivKey;
    
	};

typedef list<SnmpDh*> DhUserContainer;

#endif


