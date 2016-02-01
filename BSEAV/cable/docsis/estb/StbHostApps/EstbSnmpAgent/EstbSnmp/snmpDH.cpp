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
//    Filename: snmpDH.cpp
//    Author:   Thomas Clack
//    Creation Date: 21-December-2000
//
//**************************************************************************
//    Description: 
//      Cablemodem V2 code
//      Diffie-hellman key exchange for SNMPv3 and DOCSIS 1.1                                                         
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************

#include <stdlib.h>
#include "snmpDH.h"
extern "C" {
#ifdef KERB_SUPPORT
#include "SecLib.h"
#else
#include "SecLiborig.h"
#endif

}

SnmpDh::SnmpDh()
{
	fSecurityName = ""; 
	fMyRandom  = "";
	fMyPublic  = ""; 
	fMgrPublic = "";
} 
   
SnmpDh::SnmpDh(BcmString &SecurityName)
{
	fSecurityName = SecurityName; 
	fMyRandom  = "";
	fMyPublic  = ""; 
	fMgrPublic = "";
}

SnmpDh::~SnmpDh()
{ 
}
//The DH parameters are from OAKLEY group #2 per RFC2786.
unsigned long OAKLEY2_BASE      = 2;
unsigned long OAKLEY2_PRIME_LEN = 128;
unsigned char OAKLEY2_PRIME[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,
	0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,
	0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,0x49,0x28,0x66,0x51,0xEC,0xE6,0x53,0x81,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/********************************************************************
* Function Name: genMyPublic
* Returns:  myPublic - my public key
*
* Description: Uses the Diffie-Hellman algorithm with DOCSIS 1.1 
*  specified parameters to generate a public key. Note: Calling this
*  method will set the fMyPublic private variable. 
*********************************************************************/
int SnmpDh::GenMyPublic(BcmString &mypublic)
{    
	unsigned long  pubKeyLen  = 0;
	unsigned long  privKeyLen = 0;
	unsigned char *pubKey;
	unsigned char *privKey;
    
#ifdef KERB_SUPPORT   
    int ret = SecLibGetCryptoIface()->secCryptoDhGener(
                             OAKLEY2_PRIME,
                             OAKLEY2_PRIME_LEN,
                             OAKLEY2_BASE,
                             &pubKey,
                             (unsigned long *)&pubKeyLen,	 //c
                             &privKey, 
                             (unsigned long *)&privKeyLen); //xa
    
#else    
	int ret = secCryptoDhGener(
							  OAKLEY2_PRIME,
							  OAKLEY2_PRIME_LEN,
							  OAKLEY2_BASE,
							  &pubKey,
							  (unsigned long *)&pubKeyLen,	 //c
							  &privKey, 
							  (unsigned long *)&privKeyLen); //xa
#endif

	mypublic.assign ((char *)pubKey,pubKeyLen);
	fMyPublic.assign((char *)pubKey,pubKeyLen);
	fMyRandom.assign((char *)privKey,privKeyLen);

	//cout<<"RETURN FROM secCryptoDhGener (expect 0) = "<<ret<<endl;
   
   //    cout<<"TEST VECTORS............"<<endl;
   //    Given z=2^x mod p,
   //    Dump("z, mypublic ",fMyPublic);
   //    Dump("x, myrandom ",fMyRandom);
   //    Dump("p, prime    ",OAKLEY2_PRIME, OAKLEY2_PRIME_LEN);
   
	if (ret == 0) {	 //no error 
		free(pubKey);
		free(privKey);
		return 0;
	}
	else
		return 1;
}
/********************************************************************
* Function Name: GenV3Key
* Inputs:  mgrpublic - the other (manager's) public key
* Returns: authKey   - my auth key
*
* Description: Uses the Diffie-Hellman algorithm in the following
*  form:  secretKey = (mgrpublic^fMyRandom mod p).
*  The secretKey is then passed through the PKCS#5 PBKDF2 algorithm to 
*  produce the MD5 authentication password for use by SNMPv3.
*********************************************************************/
int SnmpDh::GenV3Key(int keyType, unsigned char * v3Key)
{   
	GenSecretKey();
	  
	unsigned char  salt[4];
	if (keyType == AUTHKEY) {
		salt[0] = 0x98;
		salt[1] = 0xdf;
		salt[2] = 0xb5;
		salt[3] = 0xac;	//salt for auth key
	} else {
		salt[0] = 0xd1;
		salt[1] = 0x31;
		salt[2] = 0x0b;
		salt[3] = 0xa6;	//salt for priv key
	}
	int saltLen = 4;
	int c       = 500; //iteration count
	int dkLen   = 16;  //intended length in octets of the derived key                    

	int kLen = fSecretKey.length();
	unsigned char k[128];
	memset(k,'n',sizeof(k));

#ifdef KERB_SUPPORT
	SecLibGetCryptoIface()->secCryptoPkcs5KeyGener((const char *)fSecretKey.data(),//const char *pPassword,
									 kLen,							 //int passwordLen,
									 (unsigned char *)&salt,		 //unsigned char *pSalt, 
									 saltLen,						 //int saltLen,
									 c,								 //int iteration,
									 dkLen,							 //int keyBufferLen,
									 v3Key);						 //unsigned char *pKeyBuffer )

#else
	secCryptoPkcs5KeyGener((const char *)fSecretKey.data(),//const char *pPassword,
									 kLen,							 //int passwordLen,
									 (unsigned char *)&salt,		 //unsigned char *pSalt, 
									 saltLen,						 //int saltLen,
									 c,								 //int iteration,
									 dkLen,							 //int keyBufferLen,
									 v3Key);						 //unsigned char *pKeyBuffer )
#endif

	//cout<<"RETURN FROM pkcs5_ (expect 0) = "<<ret<<endl;
  return 0;
}

/********************************************************************
* Function Name: GenV3KeyChange
* Inputs:  
* Returns: 
*
* Description: Uses the Diffie-Hellman algorithm in the following
*  form:  secretKey = (mgrpublic^fMyRandom mod p).
*  The right-most 16 octets of the secretKey are then returned as
*  the new key.  No distinction between Auth or Priv key.  This works
*  for both.
*********************************************************************/
int SnmpDh::GenV3KeyChange( BcmString & v3Key)
{   
    if (GenSecretKey() != 0)
        return 1;  //failed
	int sklen = fSecretKey.length();
	v3Key.assign(fSecretKey,sklen-16,sklen);
	//Dump("v3Key after assign",v3Key);

	return 0;
}

/********************************************************************
* Function Name: GenSecretKey
* Inputs:  mgrpublic - the other (manager's) public key
* Outputs: copies the secretKey to fSecretKey
*
* Description: Uses the Diffie-Hellman algorithm in the following
* form:  secretKey = (mgrpublic^fMyRandom mod p).
* The secretKey is then passed through the PKCS#5 PBKDF2 algorithm to 
* produce the MD5 authentication password for use by SNMPv3.
*********************************************************************/
int SnmpDh::GenSecretKey()
{
	unsigned long  secretKeyLen = 0;
	unsigned char  *secretKey;
	
#ifdef KERB_SUPPORT  
    int ret = SecLibGetCryptoIface()->secCryptoDhComputeSecret(
									   &secretKey,			 //unsigned char **pSecretKey, 
									   (unsigned long *)&secretKeyLen,	 //unsigned long *secretKeyLen, 
									   (unsigned char *)fMyRandom.data(),//unsigned char *pPrivKey,                                  
									   fMyRandom.length(),	 //unsigned long privKeyLen, 
									   (unsigned char *)fMgrPublic.data(),//unsigned char *pPubKeyOther, 
									   fMgrPublic.length(),	 //unsigned long pubKeyOtherLen, mgrpublic                
									   OAKLEY2_PRIME,		 //unsigned char *pPrime, 
									   OAKLEY2_PRIME_LEN,	 //unsigned long primeLen, 
									   OAKLEY2_BASE);		 //unsigned long generator );
#else
	int ret =  secCryptoDhComputeSecret(
									   &secretKey,			 //unsigned char **pSecretKey, 
									   (unsigned long *)&secretKeyLen,	 //unsigned long *secretKeyLen, 
									   (unsigned char *)fMyRandom.data(),//unsigned char *pPrivKey,                                  
									   fMyRandom.length(),	 //unsigned long privKeyLen, 
									   (unsigned char *)fMgrPublic.data(),//unsigned char *pPubKeyOther, 
									   fMgrPublic.length(),	 //unsigned long pubKeyOtherLen, mgrpublic                
									   OAKLEY2_PRIME,		 //unsigned char *pPrime, 
									   OAKLEY2_PRIME_LEN,	 //unsigned long primeLen, 
									   OAKLEY2_BASE);		 //unsigned long generator );
#endif

	fSecretKey.assign((char*)secretKey, secretKeyLen);
	//cout<<"RETURN FROM secCryptoDhComputeSecret (expect 0) = "<<ret<<endl;
    //Dump("secretKey",fSecretKey);
    
    if (ret == 0) {	//no error
		free(secretKey);
		return 0;
	} else
		return 1;     
}

/********************************************************************
* Function Name: GetSecurityName
* 
* Description: Returns the SecurityName associated with this object.
*********************************************************************/
BcmString* SnmpDh::GetSecurityName()
{   
	return &fSecurityName;
};
/********************************************************************
* Function Name: GetMyPublic
* 
* Description: Returns the myPublic number associated with this object.
*********************************************************************/
BcmString* SnmpDh::GetMyPublic()
{   
	return &fMyPublic;
};
/********************************************************************
* Function Name: GetMgrPublic
* 
* Description: Returns the mgrPublic number associated with this object.
*********************************************************************/
BcmString* SnmpDh::GetMgrPublic()
{   
	return &fMgrPublic;
}

/********************************************************************
* Function Name: SetMgrPublic
* 
* Description: Sets the mgrPublic number associated with this object.
*********************************************************************/
void SnmpDh::SetMgrPublic(BcmString &mgrPublic)
{   
	fMgrPublic = mgrPublic;
}

/********************************************************************
* Function Name: Dump
*   Dump(char *comment, unsigned char *str, int len)   
*   Dump(char *comment, char *str, int len)
*   Dump(char *comment, BcmString & bstr)
*
* Description: Dumps the variables to the screen as hex integers.
*********************************************************************/
void SnmpDh::Dump(char * comment,unsigned char * str,int len)
{
	int i; 
	int jus = 0;
	cout << "DUMP (" << len << ") - " << comment << " : \n  " << hex;
	for (i=0; i<len; i++) {
		cout << (unsigned int)str[i] << " ";
		if (jus++ >40) {
			jus = 0;
			cout << "\n  ";
		}
	}
	cout << dec << endl;
}

void SnmpDh::Dump(char * comment,BcmString & bstr)
{
	Dump(comment,(unsigned char *)bstr.data(),bstr.length());
}

void SnmpDh::Dump(char * comment, char * str,int len)
{
	Dump(comment,(unsigned char *)str,len);
}
/********************************************************************
* Function Name: DumpAll
*   
* Description: Dumps all vars in this object.
*********************************************************************/
void SnmpDh::DumpAll(char * comment)
{
	cout<<dec<<"----- DUMP_All SnmpDh ** "<<comment<<" : "<<endl<<"  ";
	cout<<"  fSecurityName - <"<<fSecurityName<<">"<<endl;
	Dump((char *) "  fMgrPublic",fMgrPublic);
	Dump((char *) "  fMyPublic",fMyPublic);
	Dump((char *) "  fSecretKey",fSecretKey);
	//Dump((char *) "  fAuthKey",fAuthKey);
	//Dump((char *) "  fPrivKey",fPrivKey);
	cout<<"---------------------------------"<<endl;
}

