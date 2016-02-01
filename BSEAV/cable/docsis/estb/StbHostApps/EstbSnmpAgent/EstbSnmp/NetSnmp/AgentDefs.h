//**************************************************************************
//
//    Copyright 2003  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: AgentDefs.h
//    Author:   Thomas Clack
//    Creation Date: 8-April-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//      Definitions and prototypes for snmp.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef AGENTDEFS_H
#define AGENTDEFS_H


#if defined (__cplusplus)
extern "C"
{
#endif
//#define StorageType_other        1    //values for storage type objects
//#define StorageType_nonVolatile  3
//#define StorageType_permanent    4
//#define StorageType_readOnly     5

#define LEN_MD5_KEY       16	//length of a MD5 key
#define LEN_SHA_KEY       20    //length of a SHA key
#define LEN_DES_KEY       16    //length of a DES key
#define MAX_ENGINE_ID_LEN 32	//length of our engine id

#define AUTH_MD5           1
#define AUTH_SHA           2           
 
//extern int LEN_ENGINE_ID;

    //int  Install_v1_view();  
//    int  GetMyAuthKeyFromPassword(char * pword,int pword_len, unsigned char* key);
//    void DumpBuf(char * text, unsigned char * ucBuf, int BufLen, char format);
    //void CleanupEnvoy();


    /*struct V3USER_T 
     {
     char   securityName[25];//user-   The name for this user
     int    userEnable;      //user-   Set user to Enable/disable state.
     int    securityLevel;   //access- ETC_SEC_LEVEL_NONE,ETC_SEC_LEVEL_AUTH,ETC_SEC_LEVEL_PRIV
     OIDC_T read_subtree;    //views-  MIB subtree enabled for read       
     OIDC_T write_subtree;   //views-  MIB subtree enabled for write 
     OIDC_T notify_subtree;  //views-  MIB subtree enabled for notify      
     } v3user_T; 
     */
#if defined (__cplusplus)
}
#endif

#endif

