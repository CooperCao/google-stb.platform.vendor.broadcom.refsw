/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: ifBridge.cpp
//    Creation Date: Wednesday, December 04, 2002 at 14:00:30
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.4
//    from input file rfc2233.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "ifBridge.h"

// #include "PacketSwitch.h"
#include "HalIf.h"
#include "InterfaceCounters.h"
#include "StubHalIf.h"
#include "SnmpLog.h"
#include "BcmSnmpAgent.h"

#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

// By default we won't hide IP stacks in the ifTable.  If you want to
// hide them, set this member to true before instantiating any objects!
bool interfacesGroupBridge::fHideIpStackIfEntries = false;


/******************************************************************************\

-- NOTE: Non-SMI text removed for the benefit of the MIB compiler.
-- Kevin O'Neal, 9/7/1999

   IF-MIB DEFINITIONS ::= BEGIN

   IMPORTS
       MODULE-IDENTITY, OBJECT-TYPE, Counter32, Gauge32, Counter64,
       Integer32, TimeTicks, mib-2,
       NOTIFICATION-TYPE                        FROM SNMPv2-SMI
       TEXTUAL-CONVENTION, DisplayString,
       PhysAddress, TruthValue, RowStatus,
       TimeStamp, AutonomousType, TestAndIncr   FROM SNMPv2-TC
       MODULE-COMPLIANCE, OBJECT-GROUP          FROM SNMPv2-CONF
       snmpTraps                                FROM SNMPv2-MIB
       IANAifType                               FROM IANAifType-MIB;

   ifMIB MODULE-IDENTITY
       LAST-UPDATED "9611031355Z"
       ORGANIZATION "IETF Interfaces MIB Working Group"
       CONTACT-INFO
               "   Keith McCloghrie
                   Cisco Systems, Inc.
                   170 West Tasman Drive
                   San Jose, CA  95134-1706
                   US

                   408-526-5260
                   kzm@cisco.com"
       DESCRIPTION
               "The MIB module to describe generic objects for
               network interface sub-layers.  This MIB is an updated
               version of MIB-II's ifTable, and incorporates the
               extensions defined in RFC 1229."
       REVISION      "9602282155Z"
       DESCRIPTION
               "Revisions made by the Interfaces MIB WG."
       REVISION      "9311082155Z"
       DESCRIPTION
               "Initial revision, published as part of RFC 1573."
       ::= { mib-2 31 }


   ifMIBObjects OBJECT IDENTIFIER ::= { ifMIB 1 }

   interfaces   OBJECT IDENTIFIER ::= { mib-2 2 }


   OwnerString ::= TEXTUAL-CONVENTION
       DISPLAY-HINT "255a"
       STATUS       current
       DESCRIPTION
               "This data type is used to model an administratively
               assigned name of the owner of a resource.  This
               information is taken from the NVT ASCII character set.
               It is suggested that this name contain one or more of
               the following: ASCII form of the manager station's
               transport address, management station name (e.g.,
               domain name), network management personnel's name,
               location, or phone number.  In some cases the agent
               itself will be the owner of an entry.  In these cases,
               this string shall be set to a string starting with
               'agent'."
       SYNTAX       OCTET STRING (SIZE(0..255))

   -- InterfaceIndex contains the semantics of ifIndex and
   -- should be used for any objects defined on other mib
   -- modules that need these semantics.

   InterfaceIndex ::= TEXTUAL-CONVENTION
       DISPLAY-HINT "d"
       STATUS       current
       DESCRIPTION
               "A unique value, greater than zero, for each interface
               or interface sub-layer in the managed system.  It is
               recommended that values are assigned contiguously
               starting from 1.  The value for each interface sub-
               layer must remain constant at least from one re-
               initialization of the entity's network management
               system to the next re-initialization."
       SYNTAX       Integer32 (1..2147483647)


   InterfaceIndexOrZero ::= TEXTUAL-CONVENTION
       DISPLAY-HINT "d"
       STATUS       current
       DESCRIPTION
               "This textual convention is an extension of the
               InterfaceIndex convention.  The latter defines a
               greater than zero value used to identify an interface
               or interface sub-layer in the managed system.  This
               extension permits the additional value of zero.  the
               value zero is object-specific and must therefore be
               defined as part of the description of any object which
               uses this syntax.  Examples of the usage of zero might
               include situations where interface was unknown, or
               when none or all interfaces need to be referenced."
       SYNTAX       Integer32 (0..2147483647)



\******************************************************************************/



/*\
 *
 *  interfacesGroupBridge for bridging scalars from the interfacesGroup group 
 *
\*/

/* 
interfaces   OBJECT IDENTIFIER ::= { mib-2 2 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

// interfacesGroupBridge::interfacesGroupBridge (BcmPacketSwitch *pPktSwitch, BcmSnmpAgent *pAgent)
interfacesGroupBridge::interfacesGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.2.1.2", pAgent)
{
    // Used for assigning ifIndex values to entries
    fNextIfIndex = 1;

    // Remember our packet switch
    // fpPktSwitch = pPktSwitch;
    
    fpStubHal = NULL;

    // Find the ifTable
    if (fpAgent)
    {
        fpIfTable = (ifTable *)fpAgent->FindTable (BcmString ("ifTable"));
        fpAgent->fpIfBridge = this;
    }
}

SNMP_STATUS interfacesGroupBridge::Register(CleanupMethod Cleanup, bool SilentFailure)
{
    int Loop;
    BcmHalIf *pHal;


    // Let the base class register.  If that fails then all bets is off.
    SNMP_STATUS Status = ScalarMibBridge::Register (Cleanup, SilentFailure);
    if (Status != SNMP_NO_ERROR)
        return Status;

    linux_Interface_Scan_Init();

    return SNMP_NO_ERROR;
}

interfacesGroupBridge::~interfacesGroupBridge ()
{
  if (fpStubHal)
    delete fpStubHal;
}

/*\ 
 *  GET methods
\*/

/* 
ifNumber  OBJECT-TYPE
       SYNTAX      Integer32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of network interfaces (regardless of their
               current state) present on this system."
       ::= { interfaces 1 }
*/ 
int interfacesGroupBridge::Get_ifNumber ()
{
  if (fpIfTable == NULL)
    return 0;

  #if KB_TODO
  return fpIfTable->NumEntries() - fpIfTable->HiddenEntries();
  #endif

  return fpIfTable->NumEntries();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/*\
 *  ACCESSOR methods to help us deal with the ifTable
\*/

unsigned int 
interfacesGroupBridge::linuxIfSpeed(int fd, struct ifreq ifr)
{
  /* the code is based on mii-diag utility by Donald Becker
   * see ftp://ftp.scyld.com/pub/diag/mii-diag.c
   */
  unsigned int retVal = 10000000;  /* let's default to 10Meg */

#ifdef WE_HAVE_IOCTL_ISSUES
  ushort *data = (ushort *)(&ifr.ifr_data);
  unsigned int *data32 = (unsigned int *)(&ifr.ifr_data);
  unsigned phy_id;
  unsigned char new_ioctl_nums = 0;
  int mii_reg, i;
  ushort mii_val[32];
  ushort bmcr, bmsr, new_bmsr, nway_advert, lkpar;
  const unsigned int media_speeds[] = {10000000, 10000000, 100000000, 100000000, 10000000, 0};
/* It corresponds to "10baseT", "10baseT-FD", "100baseTx", "100baseTx-FD", "100baseT4", "Flow-control", 0, */


  // printf (" linuxIfSpeed called for %s \n", ifr.ifr_name);
  data[0] = 0;

  if (ioctl(fd, 0x8947, &ifr) >= 0) {
    new_ioctl_nums = 1;
  } else if (ioctl(fd, SIOCDEVPRIVATE, &ifr) >= 0) {
    new_ioctl_nums = 0;
  } else {
    // printf ("SIOCGMIIPHY on %s failed\n", ifr.ifr_name);
    return retVal;
  }

  /* Begin getting mii register values */
  phy_id = data[0];
  for (mii_reg = 0; mii_reg < 8; mii_reg++){
    data[0] = phy_id;
    data[1] = mii_reg;
    if (ioctl(fd, new_ioctl_nums ? 0x8948 : SIOCDEVPRIVATE+1, &ifr) <0) {
      // printf ("SIOCGMIIREG on %s failed\n", ifr.ifr_name);
      return retVal;
    }
    mii_val[mii_reg] = data[3];
  }
  
  /*Parsing of mii values*/
  /*Invalid basic mode control register*/
  if (mii_val[0] == 0xffff  ||  mii_val[1] == 0x0000) {
    // printf ("No MII transceiver present!.\n");
    return retVal;
  }

  /* Descriptive rename. */
  bmcr = mii_val[0];        /*basic mode control register*/
  bmsr = mii_val[1];        /* basic mode status register*/
  nway_advert = mii_val[4]; /* autonegotiation advertisement*/
  lkpar = mii_val[5];       /*link partner ability*/

  // printf ("*** mii [0] = %x [1] = %x [4] = 0x%x [5] = 0x%x\n",
           mii_val[0], mii_val[1], mii_val[4], mii_val[5] );
  /*Check for link existence, returns 0 if link is absent*/
  if ((bmsr & 0x0016) != 0x0004){
    // printf ("linuxIfSpeed found no link...\n");
    return 0;
  }

  if (!(bmcr & 0x1000) ) {
    // printf ("linuxIfSpeed: Auto-negotiation disabled.\n");
    retVal = bmcr & 0x2000 ? 100000000 : 10000000;
    return retVal;
  }

  /* Link partner got our advertised abilities */
  if (lkpar & 0x4000) {
    int negotiated = nway_advert & lkpar & 0x3e0;
    int max_capability = 0;
    /* Scan for the highest negotiated capability, highest priority
                   (100baseTx-FDX) to lowest (10baseT-HDX). */
    int media_priority[] = {8, 9, 7, 6, 5};         /* media_names[i-5] */
    for (i = 0; media_priority[i]; i++){
      if (negotiated & (1 << media_priority[i])) {
        max_capability = media_priority[i];
        break;
      }
    }
    if (max_capability)
      retVal = media_speeds[max_capability - 5];
    // else
      // printf ("No common media type was autonegotiated!\n");
  }
#endif

  return retVal;

}



void
interfacesGroupBridge::linux_Interface_Scan_Init(void)
{

    char line[256], ifname_buf[64], *ifname, *ptr;
    struct ifreq ifrq;
    struct ifnet **ifnetaddr_ptr;
    FILE *devin;
    unsigned long rec_pkt, rec_oct, rec_err, rec_drop;
    unsigned long snd_pkt, snd_oct, snd_err, snd_drop, coll;
    int i, fd;
    // conf_if_list *if_ptr;
    const char *scan_line_2_2 =
        "%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu %*lu %lu";
    const char *scan_line_2_0 =
        "%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
    const char *scan_line_to_use;
    int desiredIndex = 1;




    // auto_nlist(IFNET_SYMBOL, (char *) &ifnetaddr, sizeof(ifnetaddr));



    /*
     * free old list:
    while (ifnetaddr_list) {
        struct ifnet *old = ifnetaddr_list;
        ifnetaddr_list = ifnetaddr_list->if_next;
        free(old->if_name);
        free(old->if_unit);
        free(old);
    }

    ifnetaddr = 0;
    ifnetaddr_ptr = &ifnetaddr_list;
     */

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        SnmpLogRaw << "WARNING: socket open failure in Interface_Scan_Init" << endl;
        return;
    }

    /*
     * build up ifnetaddr list by hand:
     */

    /*
     * at least linux v1.3.53 says EMFILE without reason...
     */
    if (!(devin = fopen("/proc/net/dev", "r"))) {
        close(fd);
        SnmpLogRaw << "WARNING: /proc/net/dev open failure in Interface_Scan_Init" << endl;
        return;
    }

    i = 0;

    /*
     * read the second line (a header) and determine the fields we
     * should read from.  This should be done in a better way by
     * actually looking for the field names we want.  But thats too
     * much work for today.  -- Wes
     */
    fgets(line, sizeof(line), devin);
    fgets(line, sizeof(line), devin);
    if (strstr(line, "compressed")) {
        // printf("Using scan_line_2_2\n");
        scan_line_to_use = scan_line_2_2;
    } else {
        // printf("Using scan_line_2_0\n");
        scan_line_to_use = scan_line_2_0;
    }


    while (fgets(line, sizeof(line), devin)) {
        struct ifnet *nnew;
        char *stats, *ifstart = line;

        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        while (*ifstart && *ifstart == ' ')
            ifstart++;

        if (!*ifstart || ((stats = strrchr(ifstart, ':')) == NULL)) {
            // printf ("/proc/net/dev data format error, line ==|%s|", line);
            continue;
        }
        if ((scan_line_to_use == scan_line_2_2) && ((stats - line) < 6)) {
            // printf ("/proc/net/dev data format error, line ==|%s|", line);
        }

        *stats = 0;
        strncpy(ifname_buf, ifstart, sizeof(ifname_buf));
        ifname_buf[ sizeof(ifname_buf)-1 ] = 0;
        *stats++ = ':';
        while (*stats == ' ')
            stats++;

        if ((scan_line_to_use == scan_line_2_2 &&
             sscanf(stats, scan_line_to_use, &rec_oct, &rec_pkt, &rec_err,
                    &rec_drop, &snd_oct, &snd_pkt, &snd_err, &snd_drop,
                    &coll) != 9) || (scan_line_to_use == scan_line_2_0
                                     && sscanf(stats, scan_line_to_use,
                                               &rec_pkt, &rec_err,
                                               &snd_pkt, &snd_err,
                                               &coll) != 5)) {
            if ((scan_line_to_use == scan_line_2_2)
                && !strstr(line, "No statistics available"))
                // printf ( "/proc/net/dev data format error, line ==|%s|", line);
            continue;
        }

        CleanupMethod cleanupMethod = kDeleteBridge;
        ifEntryBridge *pIfEntryBridge = new ifEntryBridge(NULL, fpAgent, desiredIndex++);

        if (pIfEntryBridge->Register(cleanupMethod) != SNMP_NO_ERROR)
        {
            delete pIfEntryBridge;
        }

        pIfEntryBridge->Assign_ifDescr(ifname_buf);
        pIfEntryBridge->linuxIfTypeGuess();

        strncpy(ifrq.ifr_name, ifname_buf, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        // if (ioctl(fd, SIOCGIFADDR, &ifrq) < 0)
        //     memset((char *) &nnew->if_addr, 0, sizeof(nnew->if_addr));
        // else
        //     nnew->if_addr = ifrq.ifr_addr;
        pIfEntryBridge->Assign_ifSpeed(linuxIfSpeed(fd, ifrq));

        BcmMacAddress macAddr(kAllZerosMac);
        /* only do the kernel dive for ethernet - others are all 0's */
        if (pIfEntryBridge->Get_ifType() == BcmHalIf::kIanaTypeEthernet)  {
          strncpy(ifrq.ifr_name, ifname_buf, sizeof(ifrq.ifr_name));
          ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
          if (ioctl(fd, SIOCGIFHWADDR, &ifrq) >= 0)
            macAddr.Set((uint8 *) &ifrq.ifr_hwaddr.sa_data[0]);
                    // ifrq.ifr_hwaddr.sa_data[1],
                    // ifrq.ifr_hwaddr.sa_data[2],
                    // ifrq.ifr_hwaddr.sa_data[3],
                    // ifrq.ifr_hwaddr.sa_data[4],
                    // ifrq.ifr_hwaddr.sa_data[5]);
        }
        SnmpLogRaw << "Scan_Init calling Assign_ifPhysAddress(" << macAddr << ") " << endl;
        pIfEntryBridge->Assign_ifPhysAddress(macAddr);

 
    } /* while (fgets ... */

#ifdef NOTYET
    ifnetaddr = ifnetaddr_list;

    if (snmp_get_do_debugging()) {
        {
            struct ifnet *x = ifnetaddr;
            DEBUGMSGTL(("mibII/interfaces", "* see: known interfaces:"));
            while (x) {
                DEBUGMSG(("mibII/interfaces", " %s", x->if_name));
                x = x->if_next;
            }
            DEBUGMSG(("mibII/interfaces", "\n"));
        } /* XXX */
    }
#endif

    fclose(devin);
    close(fd);

}

ifEntryBridge *interfacesGroupBridge::FindIfEntry (int ifIndex)
{
    BcmObjectId Oid (ifIndex);
    ifEntry *pEntry;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FindEntry (Oid);

    if (pEntry == NULL)
        return NULL;

    return pEntry->fpBridge;
}

ifEntryBridge *interfacesGroupBridge::FindIfEntry (const BcmHalIf *pHal)
{
    ifEntryBridge *pIF = FirstIfEntry ();

    // Spin through all entries and look for one which has the indicated HAL.
    while (pIF)
    {
        if (pIF->InterfaceCounters()->Interface() == pHal)
            return pIF;

        pIF = NextIfEntry (pIF->Get_ifIndex());
    }

    // If we don't find a direct match, try looking for a HAL that matches the
    // requested HAL's interface number.  This is for support of StubHalIf
    // wrapped HALs.
    pIF = FirstIfEntry ();
    while (pIF)
    {
        if (pIF->InterfaceCounters()->Interface()->InterfaceNumber() == pHal->InterfaceNumber())
            return pIF;

        pIF = NextIfEntry (pIF->Get_ifIndex());
    }

    return NULL;
}

ifEntryBridge *interfacesGroupBridge::FindIfEntry (const char *ifDescr, int len)
{
    ifEntryBridge *pBridge;
    ifEntry *pEntry;
    BcmObjectId Index;
    BcmString descr1(ifDescr, len);
    BcmString descr2;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FirstEntry ();

    while (pEntry)
    {
        pBridge = pEntry->fpBridge;

        pBridge->Get_ifDescr (descr2);
        if (descr1 == descr2) {
            return (pBridge);
        }

        pEntry->GetIndex(Index);
        pEntry = (ifEntry *) fpIfTable->NextEntry (Index);
    }

    return NULL;
}

ifEntryBridge *interfacesGroupBridge::FirstIfEntry ()
{
    ifEntry *pEntry;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FirstEntry ();

    if (pEntry == NULL)
        return NULL;

    return pEntry->fpBridge;
}

ifEntryBridge *interfacesGroupBridge::NextIfEntry (int ifIndex)
{
    BcmObjectId Oid (ifIndex);
    ifEntry *pEntry;

    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->NextEntry (Oid);

    if (pEntry == NULL)
        return NULL;

    return pEntry->fpBridge;
}

ifEntryBridge *interfacesGroupBridge::FirstIfEntryOfType (int Type)
{
    ifEntryBridge *pBridge;
    ifEntry *pEntry;
    BcmObjectId Index;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FirstEntry ();

    while (pEntry)
    {
        pBridge = pEntry->fpBridge;

        if (pBridge->Get_ifType() == Type)
            return pBridge;

        pEntry->GetIndex(Index);
        pEntry = (ifEntry *) fpIfTable->NextEntry (Index);
    }

    return NULL;
}

ifEntryBridge *interfacesGroupBridge::NextIfEntryOfType (int ifIndex, int Type)
{
    ifEntryBridge *pBridge;
    ifEntry *pEntry;
    BcmObjectId IxOid;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->NextEntry (BcmObjectId(ifIndex));

    while (pEntry)
    {
        pBridge = pEntry->fpBridge;

        if (pBridge->Get_ifType() == Type)
            return pBridge;

        pEntry->GetIndex(IxOid);
        pEntry = (ifEntry *) fpIfTable->NextEntry (IxOid);
    }

    return NULL;
}

ifEntryBridge *interfacesGroupBridge::FirstIfEntryOfIfNum (int ifNum)
{
    ifEntryBridge *pBridge;
    ifEntry *pEntry;
    BcmObjectId Index;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FirstEntry ();

    while (pEntry)
    {
        pBridge = pEntry->fpBridge;

        if (pBridge->Get_ifNumber() == ifNum)
            return pBridge;

        pEntry->GetIndex (Index);
        pEntry = (ifEntry *) fpIfTable->NextEntry (Index);
    }

    return NULL;
}

ifEntryBridge *interfacesGroupBridge::NextIfEntryOfIfNum (int ifNum, int Type)
{
    ifEntryBridge *pBridge;
    ifEntry *pEntry;
    BcmObjectId IxOid;


    if (fpIfTable == NULL)
        return NULL;

    pEntry = (ifEntry *) fpIfTable->FirstEntry();

    while (pEntry)
    {
        pBridge = pEntry->fpBridge;

        if (pBridge->Get_ifNumber() == ifNum)
            return pBridge;

        pEntry->GetIndex(IxOid);
        pEntry = (ifEntry *) fpIfTable->NextEntry (IxOid);
    }

    return NULL;
}

/*\
 *
 *  ifEntryBridge for bridging elements in the ifTable
 *
\*/

/* 
ifTable OBJECT-TYPE
       SYNTAX      SEQUENCE OF IfEntry
       MAX-ACCESS  not-accessible
       STATUS      current
       DESCRIPTION
               "A list of interface entries.  The number of entries
               is given by the value of ifNumber."
       ::= { interfaces 2 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifEntryBridge::ifEntryBridge (BcmInterfaceCounters *pInterfaceCounters,
                              BcmSnmpAgent *pAgent, int ifIndex, int SubIface,
                              int ifType)
  : TabularMibBridge ("1.3.6.1.2.1.2.2",
                      BcmObjectId(ifIndex),
                      pAgent)
{
  fpInterfaceCounters = pInterfaceCounters;
  fpIfXBridge = NULL;
  fpIfStackTable = NULL;
  fLastChange = 0;

  fSubIface = SubIface;
  fIfIndex = 0;

  if (ifType)
    fIfType = ifType;

  AssignIfIndex (ifIndex);

  // If our agent doesn't like us (que triste!) then don't bother creating
  // an XBridge.
  if (fIfIndex == -1)
    return;

  // Find the ifStack table.
  // fpIfStackTable = (ifStackTable *)fpAgent->FindTable (BcmString("ifStackTable"));

  /* Create a corresponding ifXTableBridge */

  fpIfXBridge = new ifXEntryBridge (pInterfaceCounters, this, pAgent);

  // All should have link up/down traps disabled by default.
  fpIfXBridge->Set_ifLinkUpDownTrapEnable (CONSTVAL_IFLINKUPDOWNTRAPENABLE_DISABLED);
}

ifEntryBridge::~ifEntryBridge ()
{
  if (fpIfXBridge)
  {
    delete fpIfXBridge;
    fpIfXBridge = NULL;
  }

  // Just zero out my pointer; I don't delete it.
  fpInterfaceCounters = NULL;
}

/*\ 
 *  GET methods
\*/


/* Get_ifIndex: moved inline */


/* 
ifDescr OBJECT-TYPE
       SYNTAX      DisplayString (SIZE (0..255))
       MAX-ACCESS  read-only


       STATUS      current
       DESCRIPTION
               "A textual string containing information about the
               interface.  This string should include the name of the
               manufacturer, the product name and the version of the
               interface hardware/software."
       ::= { ifEntry 2 }
*/ 
int ifEntryBridge::Get_ifDescr (BcmString &ifDescr)
{
  ifDescr = fIfDescr;

  return ifDescr.length();
}


/* 
ifType OBJECT-TYPE
       SYNTAX      IANAifType
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The type of interface.  Additional values for ifType
               are assigned by the Internet Assigned Numbers
               Authority (IANA), through updating the syntax of the
               IANAifType textual convention."
       ::= { ifEntry 3 }
*/ 
int ifEntryBridge::Get_ifTypeImpl ()
{
  // This method returns the MIB query ifType.  That is, if it is an
  // unassigned type, we return Other(1).  The inline version of this
  // method, used internally, will return the real type even if it is
  // unassigned.
  if (fIfType >= BcmHalIf::kIanaTypeUnassignedBase)
    return BcmHalIf::kIanaTypeOther;

  return fIfType;
}


/* 
ifMtu OBJECT-TYPE
       SYNTAX      Integer32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The size of the largest packet which can be
               sent/received on the interface, specified in octets.
               For interfaces that are used for transmitting network
               datagrams, this is the size of the largest network
               datagram that can be sent on the interface."
       ::= { ifEntry 4 }
*/ 
int ifEntryBridge::Get_ifMtu ()
{
  linuxInterfaceScan(); 

  return fIfMtu;
}


/* 
ifSpeed OBJECT-TYPE
       SYNTAX      Gauge32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "An estimate of the interface's current bandwidth in
               bits per second.  For interfaces which do not vary in
               bandwidth or for those where no accurate estimation
               can be made, this object should contain the nominal
               bandwidth.  If the bandwidth of the interface is
               greater than the maximum value reportable by this
               object then this object should report its maximum
               value (4,294,967,295) and ifHighSpeed must be used to
               report the interace's speed.  For a sub-layer which
               has no concept of bandwidth, this object should be
               zero."
       ::= { ifEntry 5 }
*/ 
unsigned int ifEntryBridge::Get_ifSpeed ()
{
    return fIfSpeed;
}


/* 
ifPhysAddress OBJECT-TYPE
       SYNTAX      PhysAddress
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The interface's address at its protocol sub-layer.
               For example, for an 802.x interface, this object
               normally contains a MAC address.  The interface's
               media-specific MIB must define the bit and byte
               ordering and the format of the value of this object.
               For interfaces which do not have such an address
               (e.g., a serial line), this object should contain an
               octet string of zero length."
       ::= { ifEntry 6 }
*/ 
void ifEntryBridge::Get_ifPhysAddress (BcmMacAddress &ifPhysAddress)
{
  // linuxInterfaceScan();

  ifPhysAddress = fIfPhysAddress;
}


/* 
ifAdminStatus OBJECT-TYPE
       SYNTAX  INTEGER {
                   up(1),       -- ready to pass packets
                   down(2),
                   testing(3)   -- in some test mode
               }
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "The desired state of the interface.  The testing(3)
               state indicates that no operational packets can be
               passed.  When a managed system initializes, all
               interfaces start with ifAdminStatus in the down(2)
               state.  As a result of either explicit management
               action or per configuration information retained by
               the managed system, ifAdminStatus is then changed to
               either the up(1) or testing(3) states (or remains in
               the down(2) state)."
       ::= { ifEntry 7 }
*/ 
int ifEntryBridge::Get_ifAdminStatus ()
{
  return 1;
}


/* 
ifOperStatus OBJECT-TYPE
       SYNTAX  INTEGER {
                   up(1),        -- ready to pass packets
                   down(2),
                   testing(3),   -- in some test mode
                   unknown(4),   -- status can not be determined
                                 -- for some reason.
                   dormant(5),
                   notPresent(6),    -- some component is missing
                   lowerLayerDown(7) -- down due to state of
                                     -- lower-layer interface(s)
               }

       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The current operational state of the interface.  The
               testing(3) state indicates that no operational packets
               can be passed.  If ifAdminStatus is down(2) then
               ifOperStatus should be down(2).  If ifAdminStatus is
               changed to up(1) then ifOperStatus should change to
               up(1) if the interface is ready to transmit and
               receive network traffic; it should change to
               dormant(5) if the interface is waiting for external
               actions (such as a serial line waiting for an incoming
               connection); it should remain in the down(2) state if
               and only if there is a fault that prevents it from
               going to the up(1) state; it should remain in the
               notPresent(6) state if the interface has missing
               (typically, hardware) components."
       ::= { ifEntry 8 }
*/ 
int ifEntryBridge::Get_ifOperStatus ()
{
  linuxInterfaceScan();

  return 1;
}


/* 
ifLastChange OBJECT-TYPE
       SYNTAX      TimeTicks
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The value of sysUpTime at the time the interface
               entered its current operational state.  If the current
               state was entered prior to the last re-initialization
               of the local network management subsystem, then this
               object contains a zero value."
       ::= { ifEntry 9 }
*/ 
time_t ifEntryBridge::Get_ifLastChange ()
{
  linuxInterfaceScan();

  return fLastChange;
}


/* 
ifInOctets OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of octets received on the interface,
               including framing characters.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 10 }
*/ 
unsigned int ifEntryBridge::Get_ifInOctets ()
{
  linuxInterfaceScan();

  return fReceivedOctets.Low32();
}


/* 
ifInUcastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were not addressed to a
               multicast or broadcast address at this sub-layer.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 11 }
*/ 
unsigned int ifEntryBridge::Get_ifInUcastPkts ()
{
  linuxInterfaceScan();

  return fReceivedUnicastPackets.Low32();
}


/* 
ifInNUcastPkts OBJECT-TYPE
       SYNTAX  Counter32
       MAX-ACCESS  read-only
       STATUS      deprecated
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were addressed to a
               multicast or broadcast address at this sub-layer.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime.

               This object is deprecated in favour of
               ifInMulticastPkts and ifInBroadcastPkts."
       ::= { ifEntry 12 }
*/ 
unsigned int ifEntryBridge::Get_ifInNUcastPkts ()
{
  // Deprecated: MUST return 0.
  return 0;
}


/* 
ifInDiscards OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of inbound packets which were chosen to be
               discarded even though no errors had been detected to
               prevent their being deliverable to a higher-layer
               protocol.  One possible reason for discarding such a
               packet could be to free up buffer space.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 13 }
*/ 
unsigned int ifEntryBridge::Get_ifInDiscards ()
{
  linuxInterfaceScan();

  return fReceivedPacketsNotForwarded;
}


/* 
ifInErrors OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "For packet-oriented interfaces, the number of inbound
               packets that contained errors preventing them from
               being deliverable to a higher-layer protocol.  For
               character-oriented or fixed-length interfaces, the
               number of inbound transmission units that contained
               errors preventing them from being deliverable to a
               higher-layer protocol.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 14 }
*/ 
unsigned int ifEntryBridge::Get_ifInErrors ()
{
  // linuxInterfaceScan();

  return 0;
}


/* 
ifInUnknownProtos OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "For packet-oriented interfaces, the number of packets
               received via the interface which were discarded
               because of an unknown or unsupported protocol.  For
               character-oriented or fixed-length interfaces that
               support protocol multiplexing the number of
               transmission units received via the interface which
               were discarded because of an unknown or unsupported
               protocol.  For any interface that does not support
               protocol multiplexing, this counter will always be 0.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 15 }
*/ 
unsigned int ifEntryBridge::Get_ifInUnknownProtos ()
{
  // linuxInterfaceScan();

  return 0;
}


/* 
ifOutOctets OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of octets transmitted out of the
               interface, including framing characters.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 16 }
*/ 
unsigned int ifEntryBridge::Get_ifOutOctets ()
{
  linuxInterfaceScan();

  return fTransmittedOctets.Low32();
}


/* 
ifOutUcastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were not
               addressed to a multicast or broadcast address at this
               sub-layer, including those that were discarded or not
               sent.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 17 }
*/ 
unsigned int ifEntryBridge::Get_ifOutUcastPkts ()
{
  linuxInterfaceScan();

  return fTransmittedUnicastPackets.Low32();
}


/* 
ifOutNUcastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      deprecated
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were
               addressed to a multicast or broadcast address at this
               sub-layer, including those that were discarded or not
               sent.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime.
               This object is deprecated in favour of
               ifOutMulticastPkts and ifOutBroadcastPkts."
       ::= { ifEntry 18 }
*/ 
unsigned int ifEntryBridge::Get_ifOutNUcastPkts ()
{
  // Deprecated: MUST return 0.
  return 0;
}


/* 
ifOutDiscards OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of outbound packets which were chosen to
               be discarded even though no errors had been detected
               to prevent their being transmitted.  One possible
               reason for discarding such a packet could be to free
               up buffer space.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 19 }
*/ 
unsigned int ifEntryBridge::Get_ifOutDiscards ()
{
  linuxInterfaceScan();

  return fTransmitPacketsDiscarded;
}


/* 
ifOutErrors OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "For packet-oriented interfaces, the number of
               outbound packets that could not be transmitted because
               of errors.  For character-oriented or fixed-length
               interfaces, the number of outbound transmission units
               that could not be transmitted because of errors.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifEntry 20 }
*/ 
unsigned int ifEntryBridge::Get_ifOutErrors ()
{
  // linuxInterfaceScan();

    return 0;
}


/* 
ifOutQLen OBJECT-TYPE
       SYNTAX      Gauge32
       MAX-ACCESS  read-only
       STATUS      deprecated
       DESCRIPTION
               "The length of the output packet queue (in packets)."
       ::= { ifEntry 21 }
*/ 
unsigned int ifEntryBridge::Get_ifOutQLen ()
{
    // Deprecated: MUST return 0.
    return 0;
}


/* 
ifSpecific OBJECT-TYPE
       SYNTAX      OBJECT IDENTIFIER
       MAX-ACCESS  read-only
       STATUS      deprecated
       DESCRIPTION
               "A reference to MIB definitions specific to the
               particular media being used to realize the interface.
               It is recommended that this value point to an instance
               of a MIB object in the media-specific MIB, i.e., that
               this object have the semantics associated with the
               InstancePointer textual convention defined in RFC
               1903.  In fact, it is recommended that the media-
               specific MIB specify what value ifSpecific should/can
               take for values of ifType.  If no MIB definitions
               specific to the particular media are available, the
               value should be set to the OBJECT IDENTIFIER { 0 0 }."
       ::= { ifEntry 22 }
*/ 
int ifEntryBridge::Get_ifSpecific (BcmObjectId &ifSpecific)
{
    return ifSpecific.Length();
}

void ifEntryBridge::linuxInterfaceScan(void)
{
    char line[256], ifname_buf[64], *ifname, *ptr;
    struct ifreq ifrq;
    struct ifnet **ifnetaddr_ptr;
    FILE *devin;
    unsigned long rec_pkt, rec_oct, rec_err, rec_drop;
    unsigned long snd_pkt, snd_oct, snd_err, snd_drop, coll;
    int i, fd;
    // conf_if_list *if_ptr;
    const char *scan_line_2_2 =
        "%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu %*lu %lu";
    const char *scan_line_2_0 =
        "%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
    const char *scan_line_to_use;

    fIfMtu = 1;




    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        SnmpLogRaw << "WARNING: socket open failure in Interface_Scan_Init" << endl;
        return;
    }

    /*
     * at least linux v1.3.53 says EMFILE without reason...
     */
    if (!(devin = fopen("/proc/net/dev", "r"))) {
        close(fd);
        SnmpLogRaw << "WARNING: /proc/net/dev open failure in Interface_Scan_Init" << endl;
        return;
    }

    i = 0;

    /*
     * read the second line (a header) and determine the fields we
     * should read from.  This should be done in a better way by
     * actually looking for the field names we want.  But thats too
     * much work for today.  -- Wes
     */
    fgets(line, sizeof(line), devin);
    fgets(line, sizeof(line), devin);
    if (strstr(line, "compressed")) {
        scan_line_to_use = scan_line_2_2;
    } else {
        scan_line_to_use = scan_line_2_0;
    }


    while (fgets(line, sizeof(line), devin)) {
        struct ifnet *nnew;
        char *stats, *ifstart = line;

        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        while (*ifstart && *ifstart == ' ')
            ifstart++;

        if (!*ifstart || ((stats = strrchr(ifstart, ':')) == NULL)) {
            // printf ("/proc/net/dev data format error, line ==|%s|", line);
            continue;
        }
        if ((scan_line_to_use == scan_line_2_2) && ((stats - line) < 6)) {
            // printf ("/proc/net/dev data format error, line ==|%s|", line);
        }

        *stats = 0;
        strncpy(ifname_buf, ifstart, sizeof(ifname_buf));
        ifname_buf[ sizeof(ifname_buf)-1 ] = 0;

        /* if name doesn't match ours, go get next one */
        BcmString tmpIfDescr(ifname_buf);
        if (fIfDescr != tmpIfDescr)
          continue;

        *stats++ = ':';
        while (*stats == ' ')
            stats++;

        if ((scan_line_to_use == scan_line_2_2 &&
             sscanf(stats, scan_line_to_use, &rec_oct, &rec_pkt, &rec_err,
                    &rec_drop, &snd_oct, &snd_pkt, &snd_err, &snd_drop,
                    &coll) != 9) || (scan_line_to_use == scan_line_2_0
                                     && sscanf(stats, scan_line_to_use,
                                               &rec_pkt, &rec_err,
                                               &snd_pkt, &snd_err,
                                               &coll) != 5)) {
            if ((scan_line_to_use == scan_line_2_2)
                && !strstr(line, "No statistics available"))
                // printf ( "/proc/net/dev data format error, line ==|%s|", line);
            continue;
        }

        fReceivedOctets = (uint32) rec_oct;
        fReceivedUnicastPackets = (uint32) rec_pkt;
        // ??? fInErrors = rec_err;
        fReceivedPacketsNotForwarded = rec_drop;
        fTransmittedOctets = (uint32) snd_oct;
        fTransmittedUnicastPackets = snd_pkt;
        // ??? fOutErrors = snd_err;
        fTransmitPacketsDiscarded = snd_drop;
        // ??? f = coll;

    } /* while (fgets ... */

  fclose(devin);
  close(fd);

}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
ifAdminStatus OBJECT-TYPE
       SYNTAX  INTEGER {
                   up(1),       -- ready to pass packets
                   down(2),
                   testing(3)   -- in some test mode
               }
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "The desired state of the interface.  The testing(3)
               state indicates that no operational packets can be
               passed.  When a managed system initializes, all
               interfaces start with ifAdminStatus in the down(2)
               state.  As a result of either explicit management
               action or per configuration information retained by
               the managed system, ifAdminStatus is then changed to
               either the up(1) or testing(3) states (or remains in
               the down(2) state)."
       ::= { ifEntry 7 }
*/ 
SNMP_STATUS ifEntryBridge::Set_ifAdminStatus (int ifAdminStatus)
{
    SNMP_STATUS Status = Int32MibObject::TestDefaults (ifAdminStatus);

    if (Status == SNMP_NO_ERROR)
    {
        if ((ifAdminStatus != CONSTVAL_IFADMINSTATUS_UP)
        &&  (ifAdminStatus != CONSTVAL_IFADMINSTATUS_DOWN))
        //&&  (ifAdminStatus != CONSTVAL_IFADMINSTATUS_TESTING)) not supported
          return SNMP_WRONG_VALUE;

        bool setStatus = true;

        // If this fails, it means that the HAL won't let us change his state.
        // Since we know we already got either UP or DOWN, we should now return
        // INCONSISTENT VALUE
        if (setStatus == true)
        {
            time_t Now;

            // If the call succeeds, update ifLastChange.  Note that we must 
            // essentially store sysUpTime here, because if we just store the current
            // time then sometime later the time might change via TOD or 
            // docsDevDateTime, which would render the time incorrect.
            if (time (&Now) == -1)
                fLastChange =  fpAgent->sysUpTime();
        }
        else
            Status = SNMP_INCONSISTENT_VALUE;
    }

    return Status;
}

int ifEntryBridge::Get_ifNumber ()
{
        return 1;
}


bool ifEntryBridge::IsDefaultInterface ()
{
        return false;

}


int ifEntryBridge::AssignIfIndex (int RequestedIfIndex)
{
    int IfIndex = 0, AgentIfIndex = 0;

    if (fpAgent == NULL)
    {
        SnmpLogRaw << "WARNING: ifTable has no agent, can't assign indices!" << endl;
        return 0;
    }

    AgentIfIndex = fpAgent->AssignIfIndex (this);

    if (AgentIfIndex != 0)
        IfIndex = AgentIfIndex;
    else if (RequestedIfIndex != 0)
        IfIndex = RequestedIfIndex;

    // -1 is a special value that means don't add this interface.
    if (IfIndex == -1)
    {
        fIfIndex = -1;
        return -1;
    }

    fpAgent->LockMib();

    // Try to honor the requested ifIndex if one is specified.
    if (IfIndex)
    {
        if (fpAgent->IfBridge().FindIfEntry(IfIndex) == NULL)
            fIfIndex = IfIndex;
        else
            fIfIndex = fpAgent->IfBridge().fNextIfIndex;
    }

    else
        fIfIndex = fpAgent->IfBridge().fNextIfIndex;

    // If we just got the next ifIndex, then increment it.  If the ifIndex
    // just assigned is lower than or higher than the 'next' one, then that
    // means we've honored a requested ifIndex and we should leave the 'next'
    // one alone.
    if (fIfIndex == fpAgent->IfBridge().fNextIfIndex)
    {
        int NextIfIndex = fpAgent->IfBridge().fNextIfIndex + 1;

        // Make sure the ifIndex we're about to assign as 'next' is available.
        // If it's not, then keep incrementing until we find an available one.
        while (fpAgent->IfBridge().FindIfEntry (NextIfIndex) != NULL)
            NextIfIndex++;

        fpAgent->IfBridge().fNextIfIndex = NextIfIndex;
    }

    SetIndex (BcmObjectId (fIfIndex));

    fpAgent->UnlockMib();

    return fIfIndex;
}

SNMP_STATUS ifEntryBridge::Register(CleanupMethod Cleanup, bool SilentFailure)
{
    SNMP_STATUS Status;


    // If our agent has decided he doesn't like this kind of ifEntry for
    // some reason (indicated by fIfIdex == -1) then don't register.
    if (fIfIndex == -1)
        return SNMP_GEN_ERR;

    Status = TabularMibBridge::Register (Cleanup, SilentFailure);
    if (Status != SNMP_NO_ERROR)
        return Status;

    // Register corresponding ifXEntry.  We're going to delete
    // the bridge ourselves, so don't call Register w/ true.
    if (fpIfXBridge)
        fpIfXBridge->Register(kDeleteBridge);

    // Don't do this if we're hiding the IP stack entries and this is an IP stack.
    if ((fpAgent->IfBridge().HideIpStackIfEntries() == false)
        ||  (fIfType != BcmHalIf::kIanaTypeIpStack))
        AddToIfStack ();

    return SNMP_NO_ERROR;
}

SNMP_STATUS ifEntryBridge::DeRegister()
{
    SnmpLogMem << "IfTableBridge::DeRegister enter" <<endl;

    if (fpIfXBridge)
    {
        SnmpLogMem << "  IfTableBridge::DeRegister - DeRegistering fpIfXBridge" << endl;
        fpIfXBridge->DeRegister ();
    }

    // Update the ifStack
    RemoveFromIfStack ();

    SnmpLogMem << "IfTableBridge::DeRegister exit" <<endl;

    return TabularMibBridge::DeRegister ();
}

SNMP_STATUS ifEntryBridge::AddToIfStack ()
{
    ifStackEntryBridge *pBridge;


    /*
      Logic: if we have no sub-iface, create 2 stack entries:
      0.me and me.0
      
      If we have a sub-iface, create
      fpHal->Get_ifIndex().me and me.0
      then find fpHal->Get_ifIndex().0 and remove it
      
      Note that this won't work if the sub-iface objects are
      created first.  Additional logic might search the table
      to see if there is an anything.me entry anywhere and if there
      is don't create me.0.
    */
    if (fpIfStackTable == NULL)
        return SNMP_RESOURCE_UNAVAILABLE;

    // Create me.0 regardless.
    pBridge = new ifStackEntryBridge (Get_ifIndex(), 0, fpAgent);
    if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
        delete (pBridge);

    if (fSubIface == 0)
    {
        pBridge = new ifStackEntryBridge (0, Get_ifIndex(), fpAgent);
        if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
            delete (pBridge);
    }
    else
    {
        BcmMibEntry *pEntry;
        char Index[8];


        pBridge = new ifStackEntryBridge (fpInterfaceCounters->Interface()->IfTableIndex(fpAgent), Get_ifIndex(), fpAgent);
        if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
            delete (pBridge);

        sprintf(Index, "%d.%d", fpInterfaceCounters->Interface()->IfTableIndex(fpAgent), 0);

        pEntry = fpIfStackTable->FindEntry(BcmObjectId(Index));
        if (pEntry)
            delete (pEntry);
    }

    return SNMP_NO_ERROR;
}

SNMP_STATUS ifEntryBridge::RemoveFromIfStack ()
{
    /*
    Logic here is more complicated than AddToIfStack.  
    
    Search for any entry that has my ifIndex in it anywhere.  If it's
    0.me or me.0 you can just ditch it.  
    
    If it's something.me or me.something, then we have to ditch that
    entry and replace it with something.0 or 0.something.
  */
    if (fpIfStackTable == NULL)
        return SNMP_RESOURCE_UNAVAILABLE;

    return SNMP_NO_ERROR;
}


// This Method is needed to allow a Halif to override the default Interface Counter methods
SNMP_STATUS ifEntryBridge::Set_ifInterfaceCounter (BcmInterfaceCounters * pinterfaceCounters)
{
    // setup a new counters interface
    fpInterfaceCounters = pinterfaceCounters;
    fpIfXBridge->Set_ifInterfaceCounter(pinterfaceCounters);

    return SNMP_NO_ERROR;
}


/*\
 *
 *  ifMIBObjectsGroupBridge for bridging scalars from the ifMIBObjectsGroup group 
 *
\*/

/* 
ifMIBObjects OBJECT IDENTIFIER ::= { ifMIB 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifMIBObjectsGroupBridge::ifMIBObjectsGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.2.1.31.1", pAgent)
{
}

ifMIBObjectsGroupBridge::~ifMIBObjectsGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ifTableLastChange  OBJECT-TYPE
       SYNTAX      TimeTicks
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The value of sysUpTime at the time of the last
               creation or deletion of an entry in the ifTable.  If
               the number of entries has been unchanged since the
               last re-initialization of the local network management
               subsystem, then this object contains a zero value."
       ::= { ifMIBObjects 5 }
*/ 
unsigned long ifMIBObjectsGroupBridge::Get_ifTableLastChange ()
{
    return 0;
}


/* 
ifStackLastChange OBJECT-TYPE
       SYNTAX         TimeTicks
       MAX-ACCESS     read-only
       STATUS         current
       DESCRIPTION
               "The value of sysUpTime at the time of the last change
               of the (whole) interface stack.  A change of the
               interface stack is defined to be any creation,
               deletion, or change in value of any instance of
               ifStackStatus.  If the interface stack has been
               unchanged since the last re-initialization of the
               local network management subsystem, then this object
               contains a zero value."
       ::= { ifMIBObjects 6 }
*/ 
unsigned long ifMIBObjectsGroupBridge::Get_ifStackLastChange ()
{
    return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ifXEntryBridge for bridging elements in the ifXTable
 *
\*/

/* 
ifXTable        OBJECT-TYPE
       SYNTAX      SEQUENCE OF IfXEntry
       MAX-ACCESS  not-accessible
       STATUS      current
       DESCRIPTION
               "A list of interface entries.  The number of entries
               is given by the value of ifNumber.  This table
               contains additional objects for the interface table."
       ::= { ifMIBObjects 1 }

INDEX {  }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifXEntryBridge::ifXEntryBridge (BcmInterfaceCounters *pInterfaceCounters,
                                ifEntryBridge *pIfEntry, BcmSnmpAgent *pAgent)
: TabularMibBridge ("1.3.6.1.2.1.31.1.1", 0, pAgent)
{
    fpInterfaceCounters = pInterfaceCounters;
    fpIfEntry = pIfEntry;
    fLinkUpDownTrapEnabled = true;

    SetIndex (BcmObjectId(fpIfEntry->Get_ifIndex ()));
}

ifXEntryBridge::~ifXEntryBridge ()
{
}

// This Method is needed to allow a Halif to override the default Interface Counter methods
SNMP_STATUS ifXEntryBridge::Set_ifInterfaceCounter (BcmInterfaceCounters * pinterfaceCounters)
{
    // setup a new counters interface
    fpInterfaceCounters = pinterfaceCounters;

    return SNMP_NO_ERROR;
}


/*\ 
 *  GET methods
\*/

/* 
ifName OBJECT-TYPE
       SYNTAX      DisplayString
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The textual name of the interface.  The value of this
               object should be the name of the interface as assigned
               by the local device and should be suitable for use in
               commands entered at the device's `console'.  This
               might be a text name, such as `le0' or a simple port
               number, such as `1', depending on the interface naming
               syntax of the device.  If several entries in the
               ifTable together represent a single interface as named
               by the device, then each will have the same value of
               ifName.  Note that for an agent which responds to SNMP
               queries concerning an interface on some other
               (proxied) device, then the value of ifName for such an
               interface is the proxied device's local name for it.

               If there is no local name, or this object is otherwise
               not applicable, then this object contains a zero-
               length string."
       ::= { ifXEntry 1 }
*/ 
int ifXEntryBridge::Get_ifName (BcmString &ifName)
{
    // The interfaces have been assigned short names, which are 32-bit
    // numbers derived from ASCII characters (not \0 terminated).  We will
    // use this short name value as the ifName.
    char name[8];

    memset(name, 0, sizeof(name));

    if (fpInterfaceCounters) {
      name[0] = (char) ((fpInterfaceCounters->Interface()->ShortName() >> 24) & 0xff);
      name[1] = (char) ((fpInterfaceCounters->Interface()->ShortName() >> 16) & 0xff);
      name[2] = (char) ((fpInterfaceCounters->Interface()->ShortName() >>  8) & 0xff);
      name[3] = (char) ((fpInterfaceCounters->Interface()->ShortName() >>  0) & 0xff);    
    }

    ifName = name;

    return ifName.length();
}


/* 
ifInMulticastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were addressed to a
               multicast address at this sub-layer.  For a MAC layer
               protocol, this includes both Group and Functional
               addresses.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 2 }
*/ 
unsigned int ifXEntryBridge::Get_ifInMulticastPkts ()
{
    return 0;
}


/* 
ifInBroadcastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were addressed to a
               broadcast address at this sub-layer.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 3 }
*/ 
unsigned int ifXEntryBridge::Get_ifInBroadcastPkts ()
{
    return 0;
}


/* 
ifOutMulticastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were
               addressed to a multicast address at this sub-layer,
               including those that were discarded or not sent.  For
               a MAC layer protocol, this includes both Group and
               Functional addresses.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 4 }
*/ 
unsigned int ifXEntryBridge::Get_ifOutMulticastPkts ()
{

    return 0;
}


/* 
ifOutBroadcastPkts OBJECT-TYPE
       SYNTAX      Counter32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were
               addressed to a broadcast address at this sub-layer,
               including those that were discarded or not sent.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 5 }
*/ 
unsigned int ifXEntryBridge::Get_ifOutBroadcastPkts ()
{
    return 0;
}


/* 
ifHCInOctets OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current

       DESCRIPTION
               "The total number of octets received on the interface,
               including framing characters.  This object is a 64-bit
               version of ifInOctets.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 6 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCInOctets (BcmCounter64 &ifHCInOctets)
{
    return ifHCInOctets.Low32();
}


/* 
ifHCInUcastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were not addressed to a
               multicast or broadcast address at this sub-layer.
               This object is a 64-bit version of ifInUcastPkts.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 7 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCInUcastPkts (BcmCounter64 &ifHCInUcastPkts)
{
    return ifHCInUcastPkts.Low32();
}


/* 
ifHCInMulticastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were addressed to a
               multicast address at this sub-layer.  For a MAC layer
               protocol, this includes both Group and Functional
               addresses.  This object is a 64-bit version of
               ifInMulticastPkts.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 8 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCInMulticastPkts (BcmCounter64 &ifHCInMulticastPkts)
{
    return ifHCInMulticastPkts.Low32();
}


/* 
ifHCInBroadcastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The number of packets, delivered by this sub-layer to
               a higher (sub-)layer, which were addressed to a
               broadcast address at this sub-layer.  This object is a
               64-bit version of ifInBroadcastPkts.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 9 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCInBroadcastPkts (BcmCounter64 &ifHCInBroadcastPkts)
{
    return ifHCInBroadcastPkts.Low32();
}


/* 
ifHCOutOctets OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of octets transmitted out of the
               interface, including framing characters.  This object
               is a 64-bit version of ifOutOctets.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 10 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCOutOctets (BcmCounter64 &ifHCOutOctets)
{
    return ifHCOutOctets.Low32();
}


/* 
ifHCOutUcastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were not
               addressed to a multicast or broadcast address at this
               sub-layer, including those that were discarded or not
               sent.  This object is a 64-bit version of
               ifOutUcastPkts.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 11 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCOutUcastPkts (BcmCounter64 &ifHCOutUcastPkts)
{
    return ifHCOutUcastPkts.Low32();
}


/* 
ifHCOutMulticastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were
               addressed to a multicast address at this sub-layer,
               including those that were discarded or not sent.  For
               a MAC layer protocol, this includes both Group and
               Functional addresses.  This object is a 64-bit version
               of ifOutMulticastPkts.
               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 12 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCOutMulticastPkts (BcmCounter64 &ifHCOutMulticastPkts)
{
    return ifHCOutMulticastPkts.Low32();
}


/* 
ifHCOutBroadcastPkts OBJECT-TYPE
       SYNTAX      Counter64
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The total number of packets that higher-level
               protocols requested be transmitted, and which were
               addressed to a broadcast address at this sub-layer,
               including those that were discarded or not sent.  This
               object is a 64-bit version of ifOutBroadcastPkts.

               Discontinuities in the value of this counter can occur
               at re-initialization of the management system, and at
               other times as indicated by the value of
               ifCounterDiscontinuityTime."
       ::= { ifXEntry 13 }
*/ 
unsigned int ifXEntryBridge::Get_ifHCOutBroadcastPkts (BcmCounter64 &ifHCOutBroadcastPkts)
{
    return ifHCOutBroadcastPkts.Low32();
}


/* 
ifLinkUpDownTrapEnable  OBJECT-TYPE
       SYNTAX      INTEGER { enabled(1), disabled(2) }
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION

               "Indicates whether linkUp/linkDown traps should be
               generated for this interface.

               By default, this object should have the value
               enabled(1) for interfaces which do not operate on
               'top' of any other interface (as defined in the
               ifStackTable), and disabled(2) otherwise."
       ::= { ifXEntry 14 }
*/ 
int ifXEntryBridge::Get_ifLinkUpDownTrapEnable ()
{
    if (fLinkUpDownTrapEnabled == true)
        return CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED;
    else
        return CONSTVAL_IFLINKUPDOWNTRAPENABLE_DISABLED;
}


/* 
ifHighSpeed OBJECT-TYPE
       SYNTAX      Gauge32
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "An estimate of the interface's current bandwidth in
               units of 1,000,000 bits per second.  If this object
               reports a value of `n' then the speed of the interface
               is somewhere in the range of `n-500,000' to
               `n+499,999'.  For interfaces which do not vary in
               bandwidth or for those where no accurate estimation
               can be made, this object should contain the nominal
               bandwidth.  For a sub-layer which has no concept of
               bandwidth, this object should be zero."
       ::= { ifXEntry 15 }
*/ 
unsigned int ifXEntryBridge::Get_ifHighSpeed ()
{
    unsigned long value = 0;

    // Just use the ifSpeed and divide by 1,000,000.  NOTE:  This is only possible
    // because our interfaces never go faster than 4billion bits per second.
    if (fpIfEntry)
    {
        // Add 500,000 so that we round up for values of X.5 MHz and greater.
        value = (fpIfEntry->Get_ifSpeed() + 500000) / 1000000;
    }

    return value;
}


/* 
ifPromiscuousMode  OBJECT-TYPE
       SYNTAX      TruthValue
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "This object has a value of false(2) if this interface
               only accepts packets/frames that are addressed to this
               station.  This object has a value of true(1) when the
               station accepts all packets/frames transmitted on the
               media.  The value true(1) is only legal on certain
               types of media.  If legal, setting this object to a
               value of true(1) may require the interface to be reset
               before becoming effective.

               The value of ifPromiscuousMode does not affect the
               reception of broadcast and multicast packets/frames by
               the interface."
       ::= { ifXEntry 16 }
*/ 
bool ifXEntryBridge::Get_ifPromiscuousMode ()
{
    bool value = false;

    return value;
}


/* 
ifConnectorPresent   OBJECT-TYPE
       SYNTAX      TruthValue
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "This object has the value 'true(1)' if the interface
               sublayer has a physical connector and the value
               'false(2)' otherwise."
       ::= { ifXEntry 17 }
*/ 
bool ifXEntryBridge::Get_ifConnectorPresent ()
{
    bool value = false;

    return value;
}


/* 
ifAlias   OBJECT-TYPE
       SYNTAX      DisplayString (SIZE(0..64))
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "This object is an 'alias' name for the interface as
               specified by a network manager, and provides a non-
               volatile 'handle' for the interface.

               On the first instantiation of an interface, the value
               of ifAlias associated with that interface is the
               zero-length string.  As and when a value is written
               into an instance of ifAlias through a network
               management set operation, then the agent must retain
               the supplied value in the ifAlias instance associated
               with the same interface for as long as that interface
               remains instantiated, including across all re-
               initializations/reboots of the network management
               system, including those which result in a change of
               the interface's ifIndex value.

               An example of the value which a network manager might
               store in this object for a WAN interface is the
               (Telco's) circuit number/identifier of the interface.

               Some agents may support write-access only for
               interfaces having particular values of ifType.  An
               agent which supports write access to this object is
               required to keep the value in non-volatile storage,
               but it may limit the length of new values depending on
               how much storage is already occupied by the current
               values for other interfaces."
       ::= { ifXEntry 18 }
*/ 
int ifXEntryBridge::Get_ifAlias (BcmString &ifAlias)
{
    ifAlias = "";
    return 0;
}


/* 
ifCounterDiscontinuityTime OBJECT-TYPE
       SYNTAX      TimeStamp
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The value of sysUpTime on the most recent occasion at
               which any one or more of this interface's counters
               suffered a discontinuity.  The relevant counters are
               the specific instances associated with this interface
               of any Counter32 or Counter64 object contained in the
               ifTable or ifXTable.  If no such discontinuities have
               occurred since the last re-initialization of the local
               management subsystem, then this object contains a zero
               value."
       ::= { ifXEntry 19 }
*/ 
time_t ifXEntryBridge::Get_ifCounterDiscontinuityTime ()
{
    time_t Time = 0;

    return Time;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
ifLinkUpDownTrapEnable  OBJECT-TYPE
       SYNTAX      INTEGER { enabled(1), disabled(2) }
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION

               "Indicates whether linkUp/linkDown traps should be
               generated for this interface.

               By default, this object should have the value
               enabled(1) for interfaces which do not operate on
               'top' of any other interface (as defined in the
               ifStackTable), and disabled(2) otherwise."
       ::= { ifXEntry 14 }
*/ 
SNMP_STATUS ifXEntryBridge::Set_ifLinkUpDownTrapEnable (int ifLinkUpDownTrapEnable)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ifLinkUpDownTrapEnable);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ifLinkUpDownTrapEnable != CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED)
    &&  (ifLinkUpDownTrapEnable != CONSTVAL_IFLINKUPDOWNTRAPENABLE_DISABLED))
      return SNMP_WRONG_VALUE;

    if (ifLinkUpDownTrapEnable == CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED)
      fLinkUpDownTrapEnabled = true;
    else
      fLinkUpDownTrapEnabled = false;
  }

  return Status;
}


/* 
ifPromiscuousMode  OBJECT-TYPE
       SYNTAX      TruthValue
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "This object has a value of false(2) if this interface
               only accepts packets/frames that are addressed to this
               station.  This object has a value of true(1) when the
               station accepts all packets/frames transmitted on the
               media.  The value true(1) is only legal on certain
               types of media.  If legal, setting this object to a
               value of true(1) may require the interface to be reset
               before becoming effective.

               The value of ifPromiscuousMode does not affect the
               reception of broadcast and multicast packets/frames by
               the interface."
       ::= { ifXEntry 16 }
*/ 
SNMP_STATUS ifXEntryBridge::Set_ifPromiscuousMode (bool ifPromiscuousMode)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

    if ((ifPromiscuousMode != true) && (ifPromiscuousMode != false))
      return SNMP_WRONG_VALUE;

        Status = SNMP_RESOURCE_UNAVAILABLE;

  return Status;
}


/* 
ifAlias   OBJECT-TYPE
       SYNTAX      DisplayString (SIZE(0..64))
       MAX-ACCESS  read-write
       STATUS      current
       DESCRIPTION
               "This object is an 'alias' name for the interface as
               specified by a network manager, and provides a non-
               volatile 'handle' for the interface.

               On the first instantiation of an interface, the value
               of ifAlias associated with that interface is the
               zero-length string.  As and when a value is written
               into an instance of ifAlias through a network
               management set operation, then the agent must retain
               the supplied value in the ifAlias instance associated
               with the same interface for as long as that interface
               remains instantiated, including across all re-
               initializations/reboots of the network management
               system, including those which result in a change of
               the interface's ifIndex value.

               An example of the value which a network manager might
               store in this object for a WAN interface is the
               (Telco's) circuit number/identifier of the interface.

               Some agents may support write-access only for
               interfaces having particular values of ifType.  An
               agent which supports write access to this object is
               required to keep the value in non-volatile storage,
               but it may limit the length of new values depending on
               how much storage is already occupied by the current
               values for other interfaces."
       ::= { ifXEntry 18 }
*/ 
SNMP_STATUS ifXEntryBridge::Set_ifAlias (const BcmString &ifAlias)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (ifAlias);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ifAlias.size() < MINLEN_IFALIAS)
    ||  (ifAlias.size() > MAXLEN_IFALIAS))
      return SNMP_WRONG_LENGTH;

    return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}






/*\
 *
 *  ifStackEntryBridge for bridging elements in the ifStackTable
 *
\*/

/* 
ifStackTable  OBJECT-TYPE
        SYNTAX        SEQUENCE OF IfStackEntry
        MAX-ACCESS    not-accessible
        STATUS        current
        DESCRIPTION
               "The table containing information on the relationships
               between the multiple sub-layers of network interfaces.
               In particular, it contains information on which sub-
               layers run 'on top of' which other sub-layers, where
               each sub-layer corresponds to a conceptual row in the
               ifTable.  For example, when the sub-layer with ifIndex
               value x runs over the sub-layer with ifIndex value y,
               then this table contains:

                 ifStackStatus.x.y=active

               For each ifIndex value, I, which identifies an active
               interface, there are always at least two instantiated
               rows in this table associated with I.  For one of
               these rows, I is the value of ifStackHigherLayer; for
               the other, I is the value of ifStackLowerLayer.  (If I
               is not involved in multiplexing, then these are the
               only two rows associated with I.)

               For example, two rows exist even for an interface
               which has no others stacked on top or below it:

                 ifStackStatus.0.x=active
                 ifStackStatus.x.0=active "
        ::= { ifMIBObjects 2 }

INDEX { ifStackHigherLayer, ifStackLowerLayer }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifStackEntryBridge::ifStackEntryBridge (int ifStackHigherLayer, int ifStackLowerLayer, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.31.1.2",
                      BcmObjectId(ifStackHigherLayer) + BcmObjectId(ifStackLowerLayer),
                      pAgent)
{
  fHi = ifStackHigherLayer;
  fLo = ifStackLowerLayer;
  
  fRowStatus = SNMP_STATUS_ACTIVE;
}

ifStackEntryBridge::~ifStackEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ifStackHigherLayer  OBJECT-TYPE
        SYNTAX        Integer32
        MAX-ACCESS    not-accessible
        STATUS        current
        DESCRIPTION
               "The value of ifIndex corresponding to the higher
               sub-layer of the relationship, i.e., the sub-layer
               which runs on 'top' of the sub-layer identified by the
               corresponding instance of ifStackLowerLayer.  If there
               is no higher sub-layer (below the internetwork layer),
               then this object has the value 0."
        ::= { ifStackEntry 1 }
*/ 
int ifStackEntryBridge::Get_ifStackHigherLayer ()
{
  return fHi;
}


/* 
ifStackLowerLayer  OBJECT-TYPE
        SYNTAX        Integer32
        MAX-ACCESS    not-accessible
        STATUS        current
        DESCRIPTION
               "The value of ifIndex corresponding to the lower sub-
               layer of the relationship, i.e., the sub-layer which
               runs 'below' the sub-layer identified by the
               corresponding instance of ifStackHigherLayer.  If
               there is no lower sub-layer, then this object has the
               value 0."
        ::= { ifStackEntry 2 }
*/ 
int ifStackEntryBridge::Get_ifStackLowerLayer ()
{
  return fLo;
}


/* 
ifStackStatus  OBJECT-TYPE
       SYNTAX         RowStatus
       MAX-ACCESS     read-create
       STATUS         current
       DESCRIPTION
               "The status of the relationship between two sub-
               layers.

               Changing the value of this object from 'active' to
               'notInService' or 'destroy' will likely have
               consequences up and down the interface stack.  Thus,
               write access to this object is likely to be
               inappropriate for some types of interfaces, and many
               implementations will choose not to support write-
               access for any type of interface."
       ::= { ifStackEntry 3 }
*/ 
int ifStackEntryBridge::Get_ifStackStatus ()
{
  return fRowStatus;
}



#if 0 // Write access not required or supported

/*\ 
 *  TEST methods
\*/

/* 
ifStackStatus  OBJECT-TYPE
       SYNTAX         RowStatus
       MAX-ACCESS     read-create
       STATUS         current
       DESCRIPTION
               "The status of the relationship between two sub-
               layers.

               Changing the value of this object from 'active' to
               'notInService' or 'destroy' will likely have
               consequences up and down the interface stack.  Thus,
               write access to this object is likely to be
               inappropriate for some types of interfaces, and many
               implementations will choose not to support write-
               access for any type of interface."
       ::= { ifStackEntry 3 }
*/ 
SNMP_STATUS ifStackEntryBridge::Test_ifStackStatus (int ifStackStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ifStackStatus, Get_ifStackStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: check any non-default constraints here and
    // return the appropriate error code in case of failure.
    SnmpLogRaw << "UNFINISHED: Test_ifStackStatus" << endl;

    fObjectMask |= kBIT_ifStackStatus;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
ifStackStatus  OBJECT-TYPE
       SYNTAX         RowStatus
       MAX-ACCESS     read-create
       STATUS         current
       DESCRIPTION
               "The status of the relationship between two sub-
               layers.

               Changing the value of this object from 'active' to
               'notInService' or 'destroy' will likely have
               consequences up and down the interface stack.  Thus,
               write access to this object is likely to be
               inappropriate for some types of interfaces, and many
               implementations will choose not to support write-
               access for any type of interface."
       ::= { ifStackEntry 3 }
*/ 
SNMP_STATUS ifStackEntryBridge::Set_ifStackStatus (int &ifStackStatus)
{
  SNMP_STATUS Status = SetRowStatus (ifStackStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ifStackStatus;
    fRowStatus = ifStackStatus;

    // UNFINISHED: really set something here!
    SnmpLogRaw << "UNFINISHED: Set_ifStackStatus" << endl;
  }

  return Status;
}

#endif

#if 0 // NOT REQUIRED OR SUPPORTED

/*\
 *
 *  ifRcvAddressEntryBridge for bridging elements in the ifRcvAddressTable
 *
\*/

/* 
ifRcvAddressTable  OBJECT-TYPE
       SYNTAX      SEQUENCE OF IfRcvAddressEntry
       MAX-ACCESS  not-accessible
       STATUS      current
       DESCRIPTION
               "This table contains an entry for each address
               (broadcast, multicast, or uni-cast) for which the
               system will receive packets/frames on a particular
               interface, except as follows:

               - for an interface operating in promiscuous mode,
               entries are only required for those addresses for
               which the system would receive frames were it not
               operating in promiscuous mode.

               - for 802.5 functional addresses, only one entry is
               required, for the address which has the functional
               address bit ANDed with the bit mask of all functional
               addresses for which the interface will accept frames.

               A system is normally able to use any unicast address
               which corresponds to an entry in this table as a
               source address."
       ::= { ifMIBObjects 4 }

INDEX { ifIndex, ifRcvAddressAddress }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifRcvAddressEntryBridge::ifRcvAddressEntryBridge (int ifIndex, const BcmMacAddress & ifRcvAddressAddress, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.31.1.4",
                      BcmObjectId(ifIndex) + BcmObjectId(ifRcvAddressAddress),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_ifRcvAddressStatus (Status);
}

/* Alternate form of constructor for use by ifRcvAddressTable::NewDefaultEntry */
ifRcvAddressEntryBridge::ifRcvAddressEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.31.1.4", Index, pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
}

ifRcvAddressEntryBridge::~ifRcvAddressEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ifRcvAddressAddress OBJECT-TYPE
       SYNTAX      PhysAddress
       MAX-ACCESS  not-accessible
       STATUS      current
       DESCRIPTION
               "An address for which the system will accept
               packets/frames on this entry's interface."

       ::= { ifRcvAddressEntry 1 }
*/ 
void ifRcvAddressEntryBridge::Get_ifRcvAddressAddress (BcmMacAddress &ifRcvAddressAddress)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifRcvAddressAddress" << endl;
  ifRcvAddressAddress.Set (0,0,0,0,0,0);
}


/* 
ifRcvAddressStatus OBJECT-TYPE
       SYNTAX      RowStatus
       MAX-ACCESS  read-create
       STATUS      current
       DESCRIPTION
               "This object is used to create and delete rows in the
               ifRcvAddressTable."

       ::= { ifRcvAddressEntry 2 }
*/ 
int ifRcvAddressEntryBridge::Get_ifRcvAddressStatus ()
{
  return fRowStatus;
}


/* 
ifRcvAddressType OBJECT-TYPE
       SYNTAX      INTEGER {
                       other(1),
                       volatile(2),
                       nonVolatile(3)
                   }

       MAX-ACCESS  read-create
       STATUS      current
       DESCRIPTION
               "This object has the value nonVolatile(3) for those
               entries in the table which are valid and will not be
               deleted by the next restart of the managed system.
               Entries having the value volatile(2) are valid and
               exist, but have not been saved, so that will not exist
               after the next restart of the managed system.  Entries
               having the value other(1) are valid and exist but are
               not classified as to whether they will continue to
               exist after the next restart."

       DEFVAL  { volatile }
       ::= { ifRcvAddressEntry 3 }
*/ 
int ifRcvAddressEntryBridge::Get_ifRcvAddressType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifRcvAddressType" << endl;
  return 0;
}


/*\ 
 *  TEST methods
\*/

/* 
ifRcvAddressStatus OBJECT-TYPE
       SYNTAX      RowStatus
       MAX-ACCESS  read-create
       STATUS      current
       DESCRIPTION
               "This object is used to create and delete rows in the
               ifRcvAddressTable."

       ::= { ifRcvAddressEntry 2 }
*/ 
SNMP_STATUS ifRcvAddressEntryBridge::Test_ifRcvAddressStatus (int ifRcvAddressStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ifRcvAddressStatus, Get_ifRcvAddressStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: check any non-default constraints here and
    // return the appropriate error code in case of failure.
    SnmpLogRaw << "UNFINISHED: Test_ifRcvAddressStatus" << endl;

    fObjectMask |= kBIT_ifRcvAddressStatus;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
ifRcvAddressStatus OBJECT-TYPE
       SYNTAX      RowStatus
       MAX-ACCESS  read-create
       STATUS      current
       DESCRIPTION
               "This object is used to create and delete rows in the
               ifRcvAddressTable."

       ::= { ifRcvAddressEntry 2 }
*/ 
SNMP_STATUS ifRcvAddressEntryBridge::Set_ifRcvAddressStatus (int &ifRcvAddressStatus)
{
  SNMP_STATUS Status = SetRowStatus (ifRcvAddressStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ifRcvAddressStatus;
    fRowStatus = ifRcvAddressStatus;

    // UNFINISHED: really set something here!
    SnmpLogRaw << "UNFINISHED: Set_ifRcvAddressStatus" << endl;
  }

  return Status;
}


/* 
ifRcvAddressType OBJECT-TYPE
       SYNTAX      INTEGER {
                       other(1),
                       volatile(2),
                       nonVolatile(3)
                   }

       MAX-ACCESS  read-create
       STATUS      current
       DESCRIPTION
               "This object has the value nonVolatile(3) for those
               entries in the table which are valid and will not be
               deleted by the next restart of the managed system.
               Entries having the value volatile(2) are valid and
               exist, but have not been saved, so that will not exist
               after the next restart of the managed system.  Entries
               having the value other(1) are valid and exist but are
               not classified as to whether they will continue to
               exist after the next restart."

       DEFVAL  { volatile }
       ::= { ifRcvAddressEntry 3 }
*/ 
SNMP_STATUS ifRcvAddressEntryBridge::Set_ifRcvAddressType (int ifRcvAddressType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ifRcvAddressType);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ifRcvAddressType != CONSTVAL_IFRCVADDRESSTYPE_OTHER)
    &&  (ifRcvAddressType != CONSTVAL_IFRCVADDRESSTYPE_VOLATILE)
    &&  (ifRcvAddressType != CONSTVAL_IFRCVADDRESSTYPE_NONVOLATILE))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ifRcvAddressType" << endl;
  }

  return Status;
}






/*\
 *
 *  ifTestEntryBridge for bridging elements in the ifTestTable
 *
\*/

/* 
ifTestTable   OBJECT-TYPE
       SYNTAX      SEQUENCE OF IfTestEntry
       MAX-ACCESS  not-accessible
       STATUS      deprecated
       DESCRIPTION
               "This table contains one entry per interface.  It
               defines objects which allow a network manager to
               instruct an agent to test an interface for various
               faults.  Tests for an interface are defined in the
               media-specific MIB for that interface.  After invoking
               a test, the object ifTestResult can be read to
               determine the outcome.  If an agent can not perform
               the test, ifTestResult is set to so indicate.  The
               object ifTestCode can be used to provide further
               test-specific or interface-specific (or even
               enterprise-specific) information concerning the
               outcome of the test.  Only one test can be in progress
               on each interface at any one time.  If one test is in
               progress when another test is invoked, the second test
               is rejected.  Some agents may reject a test when a
               prior test is active on another interface.

               Before starting a test, a manager-station must first
               obtain 'ownership' of the entry in the ifTestTable for
               the interface to be tested.  This is accomplished with
               the ifTestId and ifTestStatus objects as follows:

            try_again:
                get (ifTestId, ifTestStatus)
                while (ifTestStatus != notInUse)
                   / *
                     * Loop while a test is running or some other
                     * manager is configuring a test.
                     * /
                    short delay
                    get (ifTestId, ifTestStatus)
                }

               / *
                 * Is not being used right now -- let's compete
                 * to see who gets it.
                 * /
                lock_value = ifTestId

                if ( set(ifTestId = lock_value, ifTestStatus = inUse,
                         ifTestOwner = 'my-IP-address') == FAILURE)
                   / *
                     * Another manager got the ifTestEntry -- go
                     * try again
                     * /
                    goto try_again;

               / *
                 * I have the lock
                 * /
                set up any test parameters.

               / *
                 * This starts the test
                 * /
                set(ifTestType = test_to_run);

                wait for test completion by polling ifTestResult

                when test completes, agent sets ifTestResult
                     agent also sets ifTestStatus = 'notInUse'

                retrieve any additional test results, and ifTestId

                if (ifTestId == lock_value+1) results are valid

              A manager station first retrieves the value of the
              appropriate ifTestId and ifTestStatus objects,
              periodically repeating the retrieval if necessary,
              until the value of ifTestStatus is 'notInUse'.  The
              manager station then tries to set the same ifTestId
              object to the value it just retrieved, the same
              ifTestStatus object to 'inUse', and the corresponding
              ifTestOwner object to a value indicating itself.  If
              the set operation succeeds then the manager has
              obtained ownership of the ifTestEntry, and the value of
              the ifTestId object is incremented by the agent (per
              the semantics of TestAndIncr).  Failure of the set
              operation indicates that some other manager has
              obtained ownership of the ifTestEntry.

              Once ownership is obtained, any test parameters can be
              setup, and then the test is initiated by setting
              ifTestType.  On completion of the test, the agent sets
              ifTestStatus to 'notInUse'.  Once this occurs, the
              manager can retrieve the results.  In the (rare) event
              that the invocation of tests by two network managers
              were to overlap, then there would be a possibility that
              the first test's results might be overwritten by the
              second test's results prior to the first results being
              read.  This unlikely circumstance can be detected by a
              network manager retrieving ifTestId at the same time as
              retrieving the test results, and ensuring that the
              results are for the desired request.

              If ifTestType is not set within an abnormally long
              period of time after ownership is obtained, the agent
              should time-out the manager, and reset the value of the
              ifTestStatus object back to 'notInUse'.  It is
              suggested that this time-out period be 5 minutes.

              In general, a management station must not retransmit a
              request to invoke a test for which it does not receive
              a response; instead, it properly inspects an agent's
              MIB to determine if the invocation was successful.
              Only if the invocation was unsuccessful, is the
              invocation request retransmitted.

              Some tests may require the interface to be taken off-
              line in order to execute them, or may even require the
              agent to reboot after completion of the test.  In these
              circumstances, communication with the management
              station invoking the test may be lost until after
              completion of the test.  An agent is not required to
              support such tests.  However, if such tests are
              supported, then the agent should make every effort to
              transmit a response to the request which invoked the
              test prior to losing communication.  When the agent is
              restored to normal service, the results of the test are
              properly made available in the appropriate objects.
              Note that this requires that the ifIndex value assigned
              to an interface must be unchanged even if the test
              causes a reboot.  An agent must reject any test for
              which it cannot, perhaps due to resource constraints,
              make available at least the minimum amount of
              information after that test completes."
       ::= { ifMIBObjects 3 }

INDEX {  }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ifTestEntryBridge::ifTestEntryBridge unsigned long Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.31.1.3",
                      ,
                      pAgent)
{
}

ifTestEntryBridge::~ifTestEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ifTestId         OBJECT-TYPE
       SYNTAX       TestAndIncr
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "This object identifies the current invocation of the
               interface's test."
       ::= { ifTestEntry 1 }
*/ 
/*
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
int ifTestEntryBridge::Get_ifTestId ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestId" << endl;
  return 0;
}
*/


/* 
ifTestStatus     OBJECT-TYPE
       SYNTAX       INTEGER { notInUse(1), inUse(2) }
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "This object indicates whether or not some manager
               currently has the necessary 'ownership' required to
               invoke a test on this interface.  A write to this
               object is only successful when it changes its value
               from 'notInUse(1)' to 'inUse(2)'.  After completion of
               a test, the agent resets the value back to
               'notInUse(1)'."
       ::= { ifTestEntry 2 }
*/ 
int ifTestEntryBridge::Get_ifTestStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestStatus" << endl;
  return 0;
}


/* 
ifTestType       OBJECT-TYPE
       SYNTAX       AutonomousType
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "A control variable used to start and stop operator-
               initiated interface tests.  Most OBJECT IDENTIFIER
               values assigned to tests are defined elsewhere, in
               association with specific types of interface.
               However, this document assigns a value for a full-
               duplex loopback test, and defines the special meanings
               of the subject identifier:

                   noTest  OBJECT IDENTIFIER ::= { 0 0 }

               When the value noTest is written to this object, no
               action is taken unless a test is in progress, in which
               case the test is aborted.  Writing any other value to
               this object is only valid when no test is currently in
               progress, in which case the indicated test is
               initiated.

               When read, this object always returns the most recent
               value that ifTestType was set to.  If it has not been
               set since the last initialization of the network
               management subsystem on the agent, a value of noTest
               is returned."
       ::= { ifTestEntry 3 }
*/ 
int ifTestEntryBridge::Get_ifTestType (BcmObjectId &ifTestType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestType" << endl;
  ifTestType = "0.0";

  return ifTestType.Length();
}


/* 
ifTestResult  OBJECT-TYPE
       SYNTAX       INTEGER {
                        none(1),          -- no test yet requested
                        success(2),
                        inProgress(3),
                        notSupported(4),
                        unAbleToRun(5),   -- due to state of system
                        aborted(6),
                        failed(7)
                    }
       MAX-ACCESS   read-only
       STATUS       deprecated
       DESCRIPTION
               "This object contains the result of the most recently
               requested test, or the value none(1) if no tests have
               been requested since the last reset.  Note that this
               facility provides no provision for saving the results
               of one test when starting another, as could be
               required if used by multiple managers concurrently."
       ::= { ifTestEntry 4 }
*/ 
int ifTestEntryBridge::Get_ifTestResult ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestResult" << endl;
  return 0;
}


/* 
ifTestCode  OBJECT-TYPE
       SYNTAX       OBJECT IDENTIFIER
       MAX-ACCESS   read-only
       STATUS       deprecated
       DESCRIPTION
               "This object contains a code which contains more
               specific information on the test result, for example
               an error-code after a failed test.  Error codes and
               other values this object may take are specific to the
               type of interface and/or test.  The value may have the
               semantics of either the AutonomousType or
               InstancePointer textual conventions as defined in RFC
               1903.  The identifier:

                   testCodeUnknown  OBJECT IDENTIFIER ::= { 0 0 }

               is defined for use if no additional result code is
               available."
       ::= { ifTestEntry 5 }
*/ 
int ifTestEntryBridge::Get_ifTestCode (BcmObjectId &ifTestCode)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestCode" << endl;
  ifTestCode = "0.0";

  return ifTestCode.Length();
}


/* 
ifTestOwner      OBJECT-TYPE
       SYNTAX       OwnerString
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "The entity which currently has the 'ownership'
               required to invoke a test on this interface."
       ::= { ifTestEntry 6 }
*/ 
int ifTestEntryBridge::Get_ifTestOwner (BcmString &ifTestOwner)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ifTestOwner" << endl;
  ifTestOwner = "(unfinished)";

  return ifTestOwner.length();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
ifTestId         OBJECT-TYPE
       SYNTAX       TestAndIncr
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "This object identifies the current invocation of the
               interface's test."
       ::= { ifTestEntry 1 }
*/ 
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
/*
SNMP_STATUS ifTestEntryBridge::Set_ifTestId (int ifTestId)
{
  SNMP_STATUS Status = TestAndIncrMibObject::TestDefaults (ifTestId);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ifTestId. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ifTestId" << endl;
  }

  return Status;
}
*/


/* 
ifTestStatus     OBJECT-TYPE
       SYNTAX       INTEGER { notInUse(1), inUse(2) }
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "This object indicates whether or not some manager
               currently has the necessary 'ownership' required to
               invoke a test on this interface.  A write to this
               object is only successful when it changes its value
               from 'notInUse(1)' to 'inUse(2)'.  After completion of
               a test, the agent resets the value back to
               'notInUse(1)'."
       ::= { ifTestEntry 2 }
*/ 
SNMP_STATUS ifTestEntryBridge::Set_ifTestStatus (int ifTestStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ifTestStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ifTestStatus != CONSTVAL_IFTESTSTATUS_NOTINUSE)
    &&  (ifTestStatus != CONSTVAL_IFTESTSTATUS_INUSE))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ifTestStatus" << endl;
  }

  return Status;
}


/* 
ifTestType       OBJECT-TYPE
       SYNTAX       AutonomousType
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "A control variable used to start and stop operator-
               initiated interface tests.  Most OBJECT IDENTIFIER
               values assigned to tests are defined elsewhere, in
               association with specific types of interface.
               However, this document assigns a value for a full-
               duplex loopback test, and defines the special meanings
               of the subject identifier:

                   noTest  OBJECT IDENTIFIER ::= { 0 0 }

               When the value noTest is written to this object, no
               action is taken unless a test is in progress, in which
               case the test is aborted.  Writing any other value to
               this object is only valid when no test is currently in
               progress, in which case the indicated test is
               initiated.

               When read, this object always returns the most recent
               value that ifTestType was set to.  If it has not been
               set since the last initialization of the network
               management subsystem on the agent, a value of noTest
               is returned."
       ::= { ifTestEntry 3 }
*/ 
SNMP_STATUS ifTestEntryBridge::Set_ifTestType (const BcmObjectId &ifTestType)
{
  SNMP_STATUS Status = OidMibObject::TestDefaults (ifTestType);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ifTestType. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ifTestType" << endl;
  }

  return Status;
}


/* 
ifTestOwner      OBJECT-TYPE
       SYNTAX       OwnerString
       MAX-ACCESS   read-write
       STATUS       deprecated
       DESCRIPTION
               "The entity which currently has the 'ownership'
               required to invoke a test on this interface."
       ::= { ifTestEntry 6 }
*/ 
SNMP_STATUS ifTestEntryBridge::Set_ifTestOwner (const BcmString &ifTestOwner)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (ifTestOwner);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ifTestOwner.size() < MINLEN_IFTESTOWNER)
    ||  (ifTestOwner.size() > MAXLEN_IFTESTOWNER))
      return SNMP_WRONG_LENGTH;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ifTestOwner" << endl;
  }

  return Status;
}

#endif





