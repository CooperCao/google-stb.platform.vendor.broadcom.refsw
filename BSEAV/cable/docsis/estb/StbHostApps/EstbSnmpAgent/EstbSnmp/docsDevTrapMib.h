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
//    Filename: docsDevTrapMib.h
//    Creation Date: Monday, December 29, 2003 at 11:35:19
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0a
//    from input file docsDevTrap.mib
//
//**************************************************************************

#ifndef DOCSDEVTRAPMIB_H
#define DOCSDEVTRAPMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class docsDevTrapControlGroupBridge;


/*\
 *  docsDevTrapsGroup group
\*/

class docsDevTrapsGroup : public BcmMibGroup
{
  public:
    docsDevTrapsGroup ();

    bool Populate ();
};



/*\
 *  docsDevTrapControlGroup group
\*/

class docsDevTrapControlGroup : public BcmMibGroup
{
  friend class docsDevTrapControlGroupBridge;

  public:
    docsDevTrapControlGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    docsDevTrapControlGroupBridge *fpBridge;

    enum
    {
      kLEAF_docsDevCmTrapControl = 1,
      kLEAF_docsDevCmtsTrapControl = 2
    };
};



#endif
