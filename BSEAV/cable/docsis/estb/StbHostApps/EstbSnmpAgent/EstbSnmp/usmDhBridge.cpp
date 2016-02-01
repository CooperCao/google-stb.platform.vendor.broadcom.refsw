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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: usmDhBridge.cpp
//    Creation Date: Monday, May 12, 2003 at 11:24:18
//    Created by Broadcom V2/BFC Mib Compiler BCMIBC.EXE version 1.4.0b
//    from input file rfc2786.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "usmDhBridge.h"

extern unsigned long OAKLEY2_BASE;
extern unsigned long OAKLEY2_PRIME_LEN;
extern unsigned char OAKLEY2_PRIME[];

extern  SNMP_STATUS AgentDhKeyChange(int keytype,SnmpDh* dh);
extern  void RecalculateMyPublicNumbers(BcmSnmpV3Agent *pAgent);

int  SetUsmDhParams (BcmSnmpAgent *pAgent, const BcmString & str, int testonly);
void GetUsmDhParams(BcmString &params);
void AsnEncodeUsmDhParams(BcmString &str, int tag);


/********************************************************************
* Function Name: AsnDecodeUsmDhParamsSet()
*   
* Description: 	Set the usmDHParameters.  These are the inputs for
* the diffie-hellman algorithm.  If the testonly flag is set to true,
* the function will return without actually making the change.  
*********************************************************************/

int SetUsmDhParams (BcmSnmpV3Agent *pAgent, const BcmString & str, int testonly)
{
	int len /*, total_len*/;
	BcmString base,prime;
	int pos = 0;
	SnmpDh dh;
    
	if (str[pos] != 0x30)
		return 1;	        //no sequence tag

	if ((unsigned char)str[++pos] > 127)   // total sequence len
		++pos;

	if (str[++pos] != 0x2) 	//prime tag
        return 1;			
	
	if ((unsigned char)str[++pos] > 127)
		++pos;
	len = str[pos];		    //prime len
	prime.assign(str,++pos,len);


	if (str[pos+=len] != 0x2)//base tag
        return 1;
    
	if (testonly)
		return 0;

	if ((unsigned char)str[++pos] >127)
		++pos;
	len = str[pos];		 //prime len
	base.assign(str,++pos,len);

	dh.Dump((char *) "base = ",base);
	dh.Dump((char *) "prime = ",prime);

	OAKLEY2_BASE = base[0];
	OAKLEY2_PRIME_LEN = prime.length();
	for (int i = 0; i<128; i++)
		OAKLEY2_PRIME[i] = prime[i];

	//Now we have new numbers, need to recalculate public numbers
   	RecalculateMyPublicNumbers(pAgent);

	return 0;
}
/********************************************************************
* Function Name: AsnEncodeUsmDhParams()
*   
* Description: 	Adds Asn.1 encodings to the string passed in.
* Note: tag is the tag value per the BER.
* (integer = 0x2, octet string = 0x5, Sequence = 0x30)
* I probably should make some defines for these; but, this function
* may not stay around. ...so stick the correct number in for now.
*********************************************************************/
void AsnEncodeUsmDhParams(BcmString &str, int tag)
{
	int len = str.length();

	str.insert(0,"0");
	str[0] = len;  //add the length byte

	if (len > 127) {
		str.insert(0,"0"); //this algorithm is simple, but limits us
		str[0] = 0x81;	   //to a total len of 255 bytes for length.   					
	}

	str.insert(0,"0");
	str[0] = tag;  

	//SnmpDh dh;
	//dh.Dump("Asn1Encode value",str);

}

/********************************************************************
* Function Name: GetUsmDhParams
*   
* Description: Gets the usmDHParamters in ASN1 encoded form.
*********************************************************************/
void GetUsmDhParams(BcmString &params)
{   
    unsigned int i;

	BcmString base("a");
	base[0] = OAKLEY2_BASE;
	AsnEncodeUsmDhParams(base, 2);

	BcmString prime;
	for (i=0; i<OAKLEY2_PRIME_LEN; i++) {
		prime += "0";
		prime[i] = OAKLEY2_PRIME[i];
	}
	AsnEncodeUsmDhParams(prime, 2);
	 
	params = prime;
	params += base; 
    
    AsnEncodeUsmDhParams(params, 48);	//48 = 0x30 - the sequence tag

	//SnmpDh dh;
	//dh.Dump("Encoded parms ",params);
	//cout<<"CALLING decoder"<<endl;
	//AsnDecodeUsmDhParamsSet (params);
}


/******************************************************************************\

-- Network Working Group                                        M. St. Johns
-- Request for Comments: 2786                                    Excite@Home
-- Category: Experimental                                         March 2000

SNMP-USM-DH-OBJECTS-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE,
    -- OBJECT-IDENTITY,
    experimental, Integer32
        FROM SNMPv2-SMI
    TEXTUAL-CONVENTION
        FROM SNMPv2-TC
    MODULE-COMPLIANCE, OBJECT-GROUP
        FROM SNMPv2-CONF
    usmUserEntry
        FROM SNMP-USER-BASED-SM-MIB
    SnmpAdminString
        FROM SNMP-FRAMEWORK-MIB;

snmpUsmDHObjectsMIB MODULE-IDENTITY
    -- LAST-UPDATED "200003060000Z"  - - 6 March 2000, Midnight
    -- KO Mod: Epilogue mib compiler doesn't like 2000, change to 00
    LAST-UPDATED "0003060000Z"  -- 6 March 2000, Midnight
    ORGANIZATION "Excite@Home"
    CONTACT-INFO "Author: Mike StJohns
                  Postal: Excite@Home
                          450 Broadway
                          Redwood City, CA 94063
                  Email:  stjohns@corp.home.net
                  Phone:  +1-650-556-5368"

    DESCRIPTION
        "The management information definitions for providing forward
    secrecy for key changes for the usmUserTable, and for providing a
    method for 'kickstarting' access to the agent via a Diffie-Helman
    key agreement."

    -- REVISION     "200003060000Z"
    -- KO Mod: Epilogue mib compiler doesn't like 2000, change to 00
    REVISION     "0003060000Z"
    DESCRIPTION
       "Initial version published as RFC 2786."


    ::= { experimental 101 }  -- IANA DHKEY-CHANGE 101

-- Administrative assignments

usmDHKeyObjects OBJECT IDENTIFIER ::= { snmpUsmDHObjectsMIB 1 }
usmDHKeyConformance OBJECT IDENTIFIER ::= { snmpUsmDHObjectsMIB 2 }

-- Textual conventions


DHKeyChange ::=         TEXTUAL-CONVENTION
    STATUS              current
    DESCRIPTION
        "Upon initialization, or upon creation of a row containing an
    object of this type, and after any successful SET of this value, a
    GET of this value returns 'y' where y = g^xa MOD p, and where g is
    the base from usmDHParameters, p is the prime from
    usmDHParameters, and xa is a new random integer selected by the
    agent in the interval 2^(l-1) <= xa < 2^l < p-1.  'l' is the
    optional privateValueLength from usmDHParameters in bits.  If 'l'
    is omitted, then xa (and xr below) is selected in the interval 0
    <= xa < p-1.  y is expressed as an OCTET STRING 'PV' of length 'k'
    which satisfies

              k
        y =  SUM   2^(8(k-i)) PV'i
             i=1

        where PV1,...,PVk are the octets of PV from first to last, and
        where PV1 <> 0.

    A successful SET consists of the value 'y' expressed as an OCTET
    STRING as above concatenated with the value 'z'(expressed as an
    OCTET STRING in the same manner as y) where z = g^xr MOD p, where
    g, p and l are as above, and where xr is a new random integer
    selected by the manager in the interval 2^(l-1) <= xr < 2^l <
    p-1. A SET to an object of this type will fail with the error
    wrongValue if the current 'y' does not match the 'y' portion of
    the value of the varbind for the object. (E.g. GET yout, SET
    concat(yin, z), yout <> yin).

    Note that the private values xa and xr are never transmitted from
    manager to device or vice versa, only the values y and z.
    Obviously, these values must be retained until a successful SET on
    the associated object.

    The shared secret 'sk' is calculated at the agent as sk = z^xa MOD
    p, and at the manager as sk = y^xr MOD p.

    Each object definition of this type MUST describe how to map from
    the shared secret 'sk' to the operational key value used by the
    protocols and operations related to the object.  In general, if n
    bits of key are required, the author suggests using the n
    right-most bits of the shared secret as the operational key value."
    REFERENCE
        "-- Diffie-Hellman Key-Agreement Standard, PKCS #3;
            RSA Laboratories, November 1993"
    SYNTAX              OCTET STRING


-- Diffie Hellman public values

usmDHPublicObjects      OBJECT IDENTIFIER ::= { usmDHKeyObjects 1 }


\******************************************************************************/




/*\
 *
 *  usmDHPublicObjectsGroupBridge for bridging scalars from the usmDHPublicObjectsGroup group 
 *
\*/

/* 
usmDHPublicObjects      OBJECT IDENTIFIER ::= { usmDHKeyObjects 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

usmDHPublicObjectsGroupBridge::usmDHPublicObjectsGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.3.101.1.1", pAgent)
{
}

usmDHPublicObjectsGroupBridge::~usmDHPublicObjectsGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
usmDHParameters OBJECT-TYPE
    SYNTAX  OCTET STRING
    MAX-ACCESS read-write
    STATUS  current
    DESCRIPTION
        "The public Diffie-Hellman parameters for doing a Diffie-Hellman
    key agreement for this device.  This is encoded as an ASN.1
    DHParameter per PKCS #3, section 9.  E.g.

        DHParameter ::= SEQUENCE {
           prime   INTEGER,   -- p
           base    INTEGER,   -- g
           privateValueLength  INTEGER OPTIONAL }


    Implementors are encouraged to use either the values from
    Oakley Group 1  or the values of from Oakley Group 2 as specified
    in RFC-2409, The Internet Key Exchange, Section 6.1, 6.2 as the
    default for this object.  Other values may be used, but the
    security properties of those values MUST be well understood and
    MUST meet the requirements of PKCS #3 for the selection of
    Diffie-Hellman primes.

        In addition, any time usmDHParameters changes, all values of
    type DHKeyChange will change and new random numbers MUST be
    generated by the agent for each DHKeyChange object."
    REFERENCE
        "-- Diffie-Hellman Key-Agreement Standard, PKCS #3,
            RSA Laboratories, November 1993
         -- The Internet Key Exchange, RFC 2409, November 1998,
            Sec 6.1, 6.2"
    ::= { usmDHPublicObjects 1 }
*/ 
int usmDHPublicObjectsGroupBridge::Get_usmDHParameters (BcmString &usmDHParameters)
{
	GetUsmDhParams(usmDHParameters);
	return usmDHParameters.length();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
usmDHParameters OBJECT-TYPE
    SYNTAX  OCTET STRING
    MAX-ACCESS read-write
    STATUS  current
    DESCRIPTION
        "The public Diffie-Hellman parameters for doing a Diffie-Hellman
    key agreement for this device.  This is encoded as an ASN.1
    DHParameter per PKCS #3, section 9.  E.g.

        DHParameter ::= SEQUENCE {
           prime   INTEGER,   -- p
           base    INTEGER,   -- g
           privateValueLength  INTEGER OPTIONAL }


    Implementors are encouraged to use either the values from
    Oakley Group 1  or the values of from Oakley Group 2 as specified
    in RFC-2409, The Internet Key Exchange, Section 6.1, 6.2 as the
    default for this object.  Other values may be used, but the
    security properties of those values MUST be well understood and
    MUST meet the requirements of PKCS #3 for the selection of
    Diffie-Hellman primes.

        In addition, any time usmDHParameters changes, all values of
    type DHKeyChange will change and new random numbers MUST be
    generated by the agent for each DHKeyChange object."
    REFERENCE
        "-- Diffie-Hellman Key-Agreement Standard, PKCS #3,
            RSA Laboratories, November 1993
         -- The Internet Key Exchange, RFC 2409, November 1998,
            Sec 6.1, 6.2"
    ::= { usmDHPublicObjects 1 }
*/ 
SNMP_STATUS usmDHPublicObjectsGroupBridge::Set_usmDHParameters (const BcmString &usmDHParameters)
{
	//NOTE: This SET causes a new myPubublic number to be calculated 
	// for each user.  This may take 5-6 seconds per user.  Hence,
	// if a SNMP manager has its timeout set too low, it will 
	// time-out and possibly send the set again.  The set works, 
	// it just does not respond back in time to make the manager happy.
	// If this becomes a problem, we may need to fork here.
    
    //lets do a check to see if we are setting what we already have
    //if so, don't recalculate.
    BcmString CurrentParams;
    GetUsmDhParams(CurrentParams);
    if (CurrentParams == usmDHParameters)
        return SNMP_NO_ERROR; //trying to set params to same val we already have.
    
    BcmSnmpV3Agent *pV3Agent = BcmSnmpThread::Singleton().FindV3Agent (fpAgent);
    if (pV3Agent == NULL)
      return SNMP_RESOURCE_UNAVAILABLE;
      
    int ret = SetUsmDhParams (pV3Agent, usmDHParameters,0); 
	
	if (ret == 0)
		return SNMP_NO_ERROR;
	else
		return SNMP_WRONG_VALUE;
}






/*\
 *
 *  usmDHUserKeyEntryBridge for bridging elements in the usmDHUserKeyTable
 *
\*/

/* 
usmDHUserKeyTable OBJECT-TYPE
    SYNTAX  SEQUENCE OF UsmDHUserKeyEntry
    MAX-ACCESS not-accessible
    STATUS  current
    DESCRIPTION
        "This table augments and extends the usmUserTable and provides
    4 objects which exactly mirror the objects in that table with the
    textual convention of 'KeyChange'.  This extension allows key
    changes to be done in a manner where the knowledge of the current
    secret plus knowledge of the key change data exchanges (e.g. via
    wiretapping)  will not reveal the new key."
    ::= { usmDHPublicObjects 2 }

INDEX { usmUserEngineID, usmUserName }
*/ 

/*\ 
 *  construction / destruction
\*/ 

usmDHUserKeyEntryBridge::usmDHUserKeyEntryBridge (BcmString &usmUserIndex, SnmpDh *pDH, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.3.101.1.1.2",
                      0,
                      pAgent)
{
	unsigned int loop;
	BcmString *indexName;
	fDhObject = pDH;
    BcmObjectId Index;

	//index needs to be len of engineId + engineId +len of name + name
	Index = usmUserIndex.length();
	for (loop = 0; loop<usmUserIndex.length(); loop++)
		Index+= (unsigned char) usmUserIndex[loop];

	indexName = pDH->GetSecurityName();

	Index +=indexName->length();
	for (loop = 0; loop<indexName->length(); loop++)
		Index+= (unsigned char) (*indexName)[loop];
        
    SetIndex (Index);
}

usmDHUserKeyEntryBridge::~usmDHUserKeyEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
usmDHUserAuthKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change any given user's Authentication Key
    using a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserAuthProtocol, are installed as the operational
    authentication key for this row after a successful SET."
    ::= { usmDHUserKeyEntry 1 }
*/ 
int usmDHUserKeyEntryBridge::Get_usmDHUserAuthKeyChange (BcmString &usmDHUserAuthKeyChange)
{
	usmDHUserAuthKeyChange = *(fDhObject->GetMyPublic());
	return usmDHUserAuthKeyChange.length();
}


/* 
usmDHUserOwnAuthKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change the agents own Authentication Key
    using a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserAuthProtocol, are installed as the operational
    authentication key for this row after a successful SET."
    ::= { usmDHUserKeyEntry 2 }
*/ 
int usmDHUserKeyEntryBridge::Get_usmDHUserOwnAuthKeyChange (BcmString &usmDHUserOwnAuthKeyChange)
{
	usmDHUserOwnAuthKeyChange =  *(fDhObject->GetMyPublic());
	return usmDHUserOwnAuthKeyChange.length();
}


/* 
usmDHUserPrivKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change any given user's Privacy Key using
    a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserPrivProtocol, are installed as the operational privacy key
    for this row after a successful SET."
    ::= { usmDHUserKeyEntry 3 }
*/ 
int usmDHUserKeyEntryBridge::Get_usmDHUserPrivKeyChange (BcmString &usmDHUserPrivKeyChange)
{
	usmDHUserPrivKeyChange = *(fDhObject->GetMyPublic());
	return usmDHUserPrivKeyChange.length();
}


/* 
usmDHUserOwnPrivKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change the agent's own Privacy Key using a
    Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserPrivProtocol, are installed as the operational privacy key
    for this row after a successful SET."
    ::= { usmDHUserKeyEntry 4 }
*/ 
int usmDHUserKeyEntryBridge::Get_usmDHUserOwnPrivKeyChange (BcmString &usmDHUserOwnPrivKeyChange)
{
	usmDHUserOwnPrivKeyChange = *(fDhObject->GetMyPublic());
	return usmDHUserOwnPrivKeyChange.length();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
usmDHUserAuthKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change any given user's Authentication Key
    using a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserAuthProtocol, are installed as the operational
    authentication key for this row after a successful SET."
    ::= { usmDHUserKeyEntry 1 }
*/ 
SNMP_STATUS usmDHUserKeyEntryBridge::Set_usmDHUserAuthKeyChange (const BcmString &usmDHUserAuthKeyChange)
{
    BcmString newMgrPublic;
	int mypub_len,ret;

	mypub_len = (fDhObject->GetMyPublic())->length();   

	ret = usmDHUserAuthKeyChange.find(*(fDhObject->GetMyPublic()),0);
    if (ret != 0)
    {
		//The correct public value of this agent is not part of the set value.
		//Return wrong value error per rfc2786 DHKeyChange definition.
		//cout<<"Test_usmDHUserAuthKeyChange failed - mypublic not part of set val"<<endl;
		return SNMP_WRONG_VALUE;
	}
      
    newMgrPublic.assign(usmDHUserAuthKeyChange,mypub_len,usmDHUserAuthKeyChange.length());
    if (newMgrPublic.length() < 1)
    {
        //cout<<"Set_usmDHUserAuthKeyChange: bad set - newMgrPublic val len < 1"<<endl;
        return SNMP_WRONG_VALUE;
    }
             
	fDhObject->SetMgrPublic(newMgrPublic);
	ret = AgentDhKeyChange(AUTHKEY,fDhObject);

	return SNMP_NO_ERROR;
}


/* 
usmDHUserOwnAuthKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change the agents own Authentication Key
    using a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserAuthProtocol, are installed as the operational
    authentication key for this row after a successful SET."
    ::= { usmDHUserKeyEntry 2 }
*/ 
SNMP_STATUS usmDHUserKeyEntryBridge::Set_usmDHUserOwnAuthKeyChange (const BcmString &usmDHUserOwnAuthKeyChange)
{
    //need some work here
	//check to see if WE are the user - do this in test_ before we get here

	//The set consists of the concatenation of the agent's derived DH 
	//public value and the manager's derived DH public value.
    BcmString newMgrPublic;
	int mypub_len,ret;
    
    mypub_len = (fDhObject->GetMyPublic())->length();   
    
	ret = usmDHUserOwnAuthKeyChange.find(*(fDhObject->GetMyPublic()),0);
	if (ret !=0) {
		//The correct public value of this agent is not part of the set value.
		//Return wrong value error per rfc2786 DHKeyChange definition.
		//cout<<"Test_usmDHUserOwnAuthKeyChange failed - mypublic not part of set val"<<endl;
		return SNMP_WRONG_VALUE;
	}
	
    newMgrPublic.assign(usmDHUserOwnAuthKeyChange,mypub_len,usmDHUserOwnAuthKeyChange.length());
    if (newMgrPublic.length() < 1)
    {
        //cout<<"Set_usmDHUserOwnAuthKeyChange: bad set - newMgrPublic val len < 1"<<endl;
        return SNMP_WRONG_VALUE;
    }
    
	fDhObject->SetMgrPublic(newMgrPublic);
    ret = AgentDhKeyChange(AUTHKEY,fDhObject);

    return SNMP_NO_ERROR;
}


/* 
usmDHUserPrivKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change any given user's Privacy Key using
    a Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserPrivProtocol, are installed as the operational privacy key
    for this row after a successful SET."
    ::= { usmDHUserKeyEntry 3 }
*/ 
SNMP_STATUS usmDHUserKeyEntryBridge::Set_usmDHUserPrivKeyChange (const BcmString &usmDHUserPrivKeyChange)
{
    BcmString newMgrPublic;
	int mypub_len,ret;

	mypub_len = (fDhObject->GetMyPublic())->length();   

	ret = usmDHUserPrivKeyChange.find(*(fDhObject->GetMyPublic()),0);
	
    if (ret !=0) 
    {
		//The correct public value of this agent is not part of the set value.
		//Return wrong value error per rfc2786 DHKeyChange definition.
		//cout<<"Test_usmDHUserPrivKeyChange failed - mypublic not part of set val"<<endl;
		return SNMP_WRONG_VALUE;
	}

	newMgrPublic.assign(usmDHUserPrivKeyChange,mypub_len,usmDHUserPrivKeyChange.length());
    if (newMgrPublic.length() < 1)
    {
        //cout<<"Set_usmDHUserPrivKeyChange: bad set - newMgrPublic val len < 1"<<endl;
        return SNMP_WRONG_VALUE;
    }
    
	fDhObject->SetMgrPublic(newMgrPublic);
    ret = AgentDhKeyChange(PRIVKEY,fDhObject);

	return SNMP_NO_ERROR;
}


/* 
usmDHUserOwnPrivKeyChange OBJECT-TYPE
    SYNTAX  DHKeyChange
    MAX-ACCESS read-create
    STATUS  current
    DESCRIPTION
        "The object used to change the agent's own Privacy Key using a
    Diffie-Hellman key exchange.

    The right-most n bits of the shared secret 'sk', where 'n' is the
    number of bits required for the protocol defined by
    usmUserPrivProtocol, are installed as the operational privacy key
    for this row after a successful SET."
    ::= { usmDHUserKeyEntry 4 }
*/ 
SNMP_STATUS usmDHUserKeyEntryBridge::Set_usmDHUserOwnPrivKeyChange (const BcmString &usmDHUserOwnPrivKeyChange)
{
  //The set consists of the concatenation of the agent's derived DH 
    //public value and the manager's derived DH public value.
    
    //unfinished- need to make sure that the user name matches 
    // this is "own"   see if we can do this in test_

      
    BcmString newMgrPublic;
    int mypub_len,ret;

    mypub_len = (fDhObject->GetMyPublic())->length();   

    ret = usmDHUserOwnPrivKeyChange.find(*(fDhObject->GetMyPublic()),0);
    if (ret !=0) 
    {
        //The correct public value of this agent is not part of the set value.
        //Return wrong value error per rfc2786 DHKeyChange definition.
        //cout<<"Test_usmDHUserPrivKeyChange failed - mypublic not part of set val"<<endl;
        return SNMP_WRONG_VALUE;
    }

    newMgrPublic.assign(usmDHUserOwnPrivKeyChange,mypub_len,usmDHUserOwnPrivKeyChange.length());
    if (newMgrPublic.length() < 1)
    {
        //cout<<"Set_usmDHUserOwnPrivKeyChange: bad set - newMgrPublic val len < 1"<<endl;
        return SNMP_WRONG_VALUE;
    }
    
    fDhObject->SetMgrPublic(newMgrPublic);
    //fDhObject->DumpAll("D. After Set of newMgrPublic");
    ret = AgentDhKeyChange(PRIVKEY,fDhObject);


    return SNMP_NO_ERROR;
}






/*\
 *
 *  usmDHKickstartEntryBridge for bridging elements in the usmDHKickstartTable
 *
\*/

/* 
usmDHKickstartTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF UsmDHKickstartEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "A table of mappings between zero or more Diffie-Helman key
    agreement values and entries in the usmUserTable.  Entries in this
    table are created by providing the associated device with a
    Diffie-Helman public value and a usmUserName/usmUserSecurityName
    pair during initialization. How these values are provided is
    outside the scope of this MIB, but could be provided manually, or
    through a configuration file.  Valid public value/name pairs
    result in the creation of a row in this table as well as the
    creation of an associated row (with keys derived as indicated) in
    the usmUserTable.  The actual access the related usmSecurityName
    has is dependent on the entries in the VACM tables.  In general,
    an implementor will specify one or more standard security names
    and will provide entries in the VACM tables granting various
    levels of access to those names.  The actual content of the VACM
    table is beyond the scope of this MIB.

    Note: This table is expected to be readable without authentication
    using the usmUserSecurityName 'dhKickstart'.  See the conformance
    statements for details."
    ::= { usmDHKickstartGroup 1 }

INDEX { usmDHKickstartIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

usmDHKickstartEntryBridge::usmDHKickstartEntryBridge (int usmDHKickstartIndex, SnmpDh *pDH, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.3.101.1.2.1",
                      BcmObjectId(usmDHKickstartIndex),
                      pAgent)
{
  fDhObject = pDH;
}

usmDHKickstartEntryBridge::~usmDHKickstartEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
usmDHKickstartIndex OBJECT-TYPE
    SYNTAX      Integer32  (1..2147483647)
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "Index value for this row."
    ::= { usmDHKickstartEntry 1 }
*/ 
int usmDHKickstartEntryBridge::Get_usmDHKickstartIndex ()
{
    BcmObjectId Index;
    GetIndex (Index);
    return Index[0];
}


/* 
usmDHKickstartMyPublic OBJECT-TYPE
    SYNTAX      OCTET STRING
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The agent's Diffie-Hellman public value for this row.  At
    initialization, the agent generates a random number and derives
    its public value from that number.  This public value is published
    here.  This public value 'y' equals g^r MOD p where g is the from
    the set of Diffie-Hellman parameters, p is the prime from those
    parameters, and r is a random integer selected by the agent in the
    interval 2^(l-1) <= r < p-1 < 2^l.  If l is unspecified, then r is
    a random integer selected in the interval 0 <= r < p-1

    The public value is expressed as an OCTET STRING 'PV' of length
    'k' which satisfies

              k
        y =  SUM   2^(8(k-i)) PV'i
             i = 1

        where PV1,...,PVk are the octets of PV from first to last, and
        where PV1 != 0.


    The following DH parameters (Oakley group #2, RFC 2409, sec 6.1,
    6.2) are used for this object:

    g = 2
    p = FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1
        29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD
        EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245
        E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED
        EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE65381
        FFFFFFFF FFFFFFFF
    l=1024
    "
    REFERENCE
        "-- Diffie-Hellman Key-Agreement Standard, PKCS#3v1.4;
            RSA Laboratories, November 1993
         -- The Internet Key Exchange, RFC2409;
            Harkins, D., Carrel, D.; November 1998"
    ::= { usmDHKickstartEntry 2 }
*/ 
int usmDHKickstartEntryBridge::Get_usmDHKickstartMyPublic (BcmString &usmDHKickstartMyPublic)
{
    usmDHKickstartMyPublic = *(fDhObject->GetMyPublic());
	return usmDHKickstartMyPublic.length();
}


/* 
usmDHKickstartMgrPublic OBJECT-TYPE
    SYNTAX      OCTET STRING
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION

        "The manager's Diffie-Hellman public value for this row.  Note
    that this value is not set via the SNMP agent, but may be set via
    some out of band method, such as the device's configuration file.
    The manager calculates this value in the same manner and using the
    same parameter set as the agent does.  E.g. it selects a random
    number 'r', calculates y = g^r mod p and provides 'y' as the
    public number expressed as an OCTET STRING.  See
    usmDHKickstartMyPublic for details.

    When this object is set with a valid value during initialization,
    a row is created in the usmUserTable with the following values:

    usmUserEngineID             localEngineID
    usmUserName                 [value of usmDHKickstartSecurityName]
    usmUserSecurityName         [value of usmDHKickstartSecurityName]
    usmUserCloneFrom            ZeroDotZero
    usmUserAuthProtocol         usmHMACMD5AuthProtocol
    usmUserAuthKeyChange        -- derived from set value
    usmUserOwnAuthKeyChange     -- derived from set value
    usmUserPrivProtocol         usmDESPrivProtocol
    usmUserPrivKeyChange        -- derived from set value
    usmUserOwnPrivKeyChange     -- derived from set value
    usmUserPublic               ''
    usmUserStorageType          permanent
    usmUserStatus               active

    A shared secret 'sk' is calculated at the agent as sk =
    mgrPublic^r mod p where r is the agents random number and p is the
    DH prime from the common parameters.  The underlying privacy key
    for this row is derived from sk by applying the key derivation
    function PBKDF2 defined in PKCS#5v2.0 with a salt of 0xd1310ba6,
    and iterationCount of 500, a keyLength of 16 (for
    usmDESPrivProtocol), and a prf (pseudo random function) of
    'id-hmacWithSHA1'.  The underlying authentication key for this row
    is derived from sk by applying the key derivation function PBKDF2
    with a salt of 0x98dfb5ac , an interation count of 500, a
    keyLength of 16 (for usmHMAC5AuthProtocol), and a prf of
    'id-hmacWithSHA1'.  Note: The salts are the first two words in the
    ks0 [key schedule 0] of the BLOWFISH cipher from 'Applied
    Cryptography' by Bruce Schnier - they could be any relatively
    random string of bits.

    The manager can use its knowledge of its own random number and the
    agent's public value to kickstart its access to the agent in a
    secure manner.  Note that the security of this approach is
    directly related to the strength of the authorization security of
    the out of band provisioning of the managers public value
    (e.g. the configuration file), but is not dependent at all on the
    strength of the confidentiality of the out of band provisioning
    data."
    REFERENCE
        "-- Password-Based Cryptography Standard, PKCS#5v2.0;
            RSA Laboratories, March 1999
         -- Applied Cryptography, 2nd Ed.; B. Schneier,
            Counterpane Systems; John Wiley & Sons, 1996"
    ::= { usmDHKickstartEntry 3 }
*/ 
int usmDHKickstartEntryBridge::Get_usmDHKickstartMgrPublic (BcmString &usmDHKickstartMgrPublic)
{
    usmDHKickstartMgrPublic = *(fDhObject->GetMgrPublic());
	return usmDHKickstartMgrPublic.length();
}


/* 
usmDHKickstartSecurityName OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The usmUserName and usmUserSecurityName in the usmUserTable
    associated with this row.  This is provided in the same manner and
    at the same time as the usmDHKickstartMgrPublic value -
    e.g. possibly manually, or via the device's configuration file."
    ::= { usmDHKickstartEntry 4 }
*/ 
int usmDHKickstartEntryBridge::Get_usmDHKickstartSecurityName (BcmString &usmDHKickstartSecurityName)
{
    usmDHKickstartSecurityName = *(fDhObject->GetSecurityName());
	return usmDHKickstartSecurityName.length();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/



