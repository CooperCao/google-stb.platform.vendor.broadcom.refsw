//**************************************************************************
//                                                                          
//    Copyright 2003 Broadcom Corporation                                   
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: docsDevTrapBridge.h
//    Creation Date: Thursday, December 18, 2003 at 13:58:58
//    Created by Broadcom V2/BFC Mib Compiler BCMIBC.EXE version 1.4.0d
//    from input file docsDevTrap.mib
//
//**************************************************************************

#ifndef DOCSDEVTRAPBRIDGE_H
#define DOCSDEVTRAPBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "docsDevTrapMib.h"
#include "EventLog.h"


/*\
 *  docsDevTrapControlGroup 
\*/

#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMINITTLVUNKNOWNTRAP  0
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVREQFAILTRAP  1
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVRSPFAILTRAP  2
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVACKFAILTRAP  3
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPIINITTRAP  4
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPKMTRAP  5
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNAMICSATRAP  6
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDHCPFAILTRAP  7
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEINITTRAP  8
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEFAILTRAP  9
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADESUCCESSTRAP  10
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADECVCTRAP  11
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMTODFAILTRAP  12
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCREQFAILTRAP  13
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCRSPFAILTRAP  14
#define CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCACKFAILTRAP  15
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSINITREGREQFAILTRAP  0
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSINITREGRSPFAILTRAP  1
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSINITREGACKFAILTRAP  2
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDYNSERVREQFAILTRAP  3
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDYNSERVRSPFAILTRAP  4
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDYNSERVACKFAILTRAP  5
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSBPIINITTRAP  6
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSBPKMTRAP  7
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDYNAMICSATRAP  8
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDCCREQFAILTRAP  9
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDCCRSPFAILTRAP  10
#define CONSTVAL_DOCSDEVCMTSTRAPCONTROL_CMTSDCCACKFAILTRAP  11

class docsDevTrapControlGroup;
class docsDevTrapControlGroupBridge : public ScalarMibBridge
{
  friend class docsDevTrapControlGroup;

  public:
    docsDevTrapControlGroupBridge (BcmSnmpAgent *pAgent, EventLog *pLog);
   ~docsDevTrapControlGroupBridge ();

    /* GET methods */

    unsigned long Get_docsDevCmTrapControl ();
#if SUPPORT_CMTS_ONLY_MIBS
    unsigned long Get_docsDevCmtsTrapControl ();
#endif

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_docsDevCmTrapControl (unsigned long docsDevCmTrapControl);
#if SUPPORT_CMTS_ONLY_MIBS
    SNMP_STATUS Set_docsDevCmtsTrapControl (unsigned long docsDevCmtsTrapControl);
#endif

  protected:
    EventLog *fpLog;
};



#endif
