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
//    Filename: BrcmDomain.c
//    Author:   Kevin O'Neal
//    Creation Date: 11/18/2003
//
//**************************************************************************
//    Description:
//
//      		
//    "derived" netsnmp domain for BRCM purposes.
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPDomain.h>

#include "BrcmDomain.h"
#include "BcmV2Glue.h"
#include "SnmpApi.h"

#ifndef INADDR_NONE
#define INADDR_NONE	-1
#endif

static netsnmp_tdomain brcmDomain;

//oid netsnmpBrcmDomain[] = { 1, 3, 6, 1, 4, 1, 4413 };
//size_t netsnmpBrcmDomain_len = OID_LENGTH(netsnmpBrcmDomain);
// Usurp the UDP domain.  BWAH-HAHAHA!
oid netsnmpBrcmDomain[] = { 1, 3, 6, 1, 6, 1, 1 };
size_t netsnmpBrcmDomain_len = OID_LENGTH(netsnmpBrcmDomain);

// Remember the UDP callbacks
static int (*udpRecv) (struct netsnmp_transport_s *, void *, int, void **, int *) = NULL;
static int (*udpSend) (struct netsnmp_transport_s *, void *, int, void **, int *) = NULL;
static int (*udpClose) (struct netsnmp_transport_s *) = NULL;
static char *(*udpFmtaddr) (struct netsnmp_transport_s *, void *, int) = NULL;

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.
 */

static char *
netsnmp_brcm_fmtaddr(netsnmp_transport *t, void *data, int len)
{
    DEBUG ("netsnmp_brcm_fmtaddr\n");
    return udpFmtaddr (t, data, len);
}



/*
 * This is just like netsnmp_udp_recv except that it adds the remote address
 * to the transport.  Also I made the curly braces normal!
 */
static int
netsnmp_brcm_recv(netsnmp_transport *t, void *buf, int size,
		 void **opaque, int *olength)
{
    int             rc = -1;
    socklen_t       fromlen = sizeof(struct sockaddr_in);
    struct sockaddr_in *from;
    
    DEBUG ("netsnmp_brcm_recv\n");

    if (t != NULL && t->sock >= 0) 
    {
        // Who is responsible for freeing this?  Dunno...
        from = (struct sockaddr_in *) malloc (fromlen);
        if (from == NULL) 
        {
            *opaque = NULL;
            *olength = 0;
            return -1;
        } 
        else 
        {
            DEBUG ("  allocated opaque sockaddr_in (%d bytes) @ %p.  Who frees this?\n", fromlen, from);
            memset(from, 0, fromlen);
        }
        
        // 1/28/2004: call the app to read for us.  If he returns an error,
        // then fall back to the old recvfrom...
        rc = AppRecvfromSnmpSocket (t->sock, buf, size, (unsigned long *)&from->sin_addr.s_addr, &from->sin_port);

    	while (rc < 0)                                
        {
    	    rc = recvfrom(t->sock, buf, size, 0, (struct sockaddr *)from, &fromlen);
    	    if (rc < 0 && errno != EINTR) 
            {
                break;
    	    }
    	}

        if (rc >= 0) 
        {
            char *string = netsnmp_brcm_fmtaddr(NULL, from, fromlen);
            DEBUG("  AppReadSnmpSocket fd %d got %d bytes (from %s)\n",
			  t->sock, rc, string);
            free(string);
            
            // Now load the 'from' address into our transport's remote member.
            t->remote[3] = (from->sin_addr.s_addr >> 24) & 0xFF;
            t->remote[2] = (from->sin_addr.s_addr >> 16) & 0xFF;
            t->remote[1] = (from->sin_addr.s_addr >>  8) & 0xFF;
            t->remote[0] = (from->sin_addr.s_addr >>  0) & 0xFF;
            t->remote[5] = (from->sin_port >> 8) & 0xFF;
            t->remote[4] = (from->sin_port >> 0) & 0xFF;
        } 
        else 
        {
            DEBUG("  recvfrom fd %d err %d (\"%s\")\n",
                  t->sock, errno, strerror(errno));
        }
        *opaque = (void *)from;
        *olength = sizeof(struct sockaddr_in);
    }
    return rc;
}



static int
netsnmp_brcm_send(netsnmp_transport *t, void *buf, int size,
		 void **opaque, int *olength)
{
    int rc = 0;
    struct sockaddr_in *to = NULL;
    

    DEBUG ("netsnmp_brcm_send\n");
    
    // 2/12/2004: let the app send for us, too.  If it fails, fall back to
    // the ol' udpSend.  Note that opaque and olength are where the address
    // and port are hidden, unless they are in t->data.  So dig 'em out.  
    // The code below is directly stolen from snmpUDPDomain.c.
    if (opaque != NULL && *opaque != NULL &&
        *olength == sizeof(struct sockaddr_in)) {
        to = (struct sockaddr_in *) (*opaque);
    } else if (t != NULL && t->data != NULL &&
               t->data_length == sizeof(struct sockaddr_in)) {
        to = (struct sockaddr_in *) (t->data);
    }
    
    if (to)
      rc = AppSendtoSnmpSocket (t->sock, buf, size, to->sin_addr.s_addr, to->sin_port);
    
    if (rc == -1)
      rc = udpSend (t, buf, size, opaque, olength);
    
    DEBUG ("  udpSend returned %d (errno=%d)\n", rc, errno);
    
    if (*opaque)
    {
      DEBUG ("  Opaque pointer @ %p, FYI.  Not freeing.\n", *opaque);
    }
    
    return rc;
}



static int
netsnmp_brcm_close(netsnmp_transport *t)
{
    int rc;

    DEBUG ("netsnmp_brcm_close\n");
    
    // Call the app and have him close our socket.  Check the return value
    // here just in case the app didn't create this socket to begin with.
    rc = AppCloseSnmpSocket (t->sock);
    
    if (rc == 0)
      t->sock = -1;
    else
    {
      // This socket must not have come from the app.  Let UDP close it.
      DEBUG ("  calling udpClose.\n");
      rc = udpClose (t);
    }
    
    return rc;
}



/*
 * Open a UDP-based transport for SNMP.  Local is TRUE if addr is the local
 * address to bind to (i.e. this is a server-type session); otherwise addr is 
 * the remote address to send things to.  
 */

netsnmp_transport *
netsnmp_brcm_transport(struct sockaddr_in *localAddr, struct sockaddr_in *remoteAddr)
{
    netsnmp_transport *t = NULL;
    unsigned long AppIp = 0;
    unsigned short AppPort = 0;
    

    DEBUG ("netsnmp_brcm_transport\n");
    
    if ((localAddr == NULL) && (remoteAddr == NULL))
    {
      DEBUG ("WARNING: netsnmp_brcm_transport called with neither local nor remote address!\n");
      return NULL;
    }
    
    /* Call 'base class' UDP function */
    
    // if remote is NULL, this is a local socket.
    if (remoteAddr == NULL)
    {
      DEBUG ("  local.  Calling netsnmp_udp_transport with %p, 1\n", localAddr);
      t = netsnmp_udp_transport (localAddr, 1);
    }
    // Otherwise, this is a remote socket.
    else
    {
      DEBUG ("  remote.  Calling netsnmp_udp_transport with %p, 0\n", remoteAddr);
      t = netsnmp_udp_transport (remoteAddr, 0);
    }
    
    DEBUG ("  got transport %p.\n", t);
    
    if (t == NULL)
    {
      DEBUG ("WARNING: netsnmp_brcm_transport got NULL from netsnmp_udp_transport!\n");
      return NULL;
    }
    
    // Usurp callbacks for our own purposes.  Mua-hahaha!
    t->msgMaxSize = 1400;
    t->f_recv     = netsnmp_brcm_recv;
    t->f_send     = netsnmp_brcm_send;
    t->f_close    = netsnmp_brcm_close;
    t->f_accept   = NULL;
    t->f_fmtaddr  = netsnmp_brcm_fmtaddr;
    
    // Now, the UDP call will have opened a socket for us.  Close it and
    // get one from the app.
    if (t->sock >= 0)
    {
      DEBUG ("  closing socket %d\n", t->sock);
      close (t->sock);
    }
    else
      DEBUG ("  no socket opened by UDP.\n");
   
      
    DEBUG ("  calling app for new socket\n");
    
     // Local socket.  Specify IP and port.
    if (remoteAddr == NULL)
    {
      AppIp = localAddr->sin_addr.s_addr;
      AppPort = localAddr->sin_port;
    }
    
    // Remote socket.  Specify IP, possibly (but not necessarily) port.
    else 
    {
      if (localAddr)
      {
        AppIp = localAddr->sin_addr.s_addr;
        AppPort = localAddr->sin_port;
      }
      else
      {
        AppIp = 0;
        AppPort = 0;
      }
    }
      
      
    t->sock = AppOpenSnmpSocket(AppIp, AppPort);
    DEBUG ("  got socket %d\n", t->sock);
    
    return t;
}


netsnmp_transport *
netsnmp_brcm_create_tstring(const char *string, int local)
{
    char String1[32], String2[32];
    int Substrings;
    struct sockaddr_in addr1;
    struct sockaddr_in addr2;
    struct sockaddr_in *pLocalAddr = NULL, *pRemoteAddr = NULL;
    
    DEBUG ("netsnmp_brcm_create_tstring %s\n", string);
    
    memset (&addr1, 0, sizeof (struct sockaddr_in));
    memset (&addr2, 0, sizeof (struct sockaddr_in));
    
    // Seperate out local and remote strings.
    // They're formatted like this:
    // localAddr:localPort remoteAddr:remotePort
    // example 10.24.5.22:0 10.24.65.57:162
    Substrings = sscanf (string, "%s %s", String1, String2);
    
    if (Substrings > 0)
    {
      DEBUG ("  getting sockaddr_in for string 1: %s\n", String1);
      netsnmp_sockaddr_in(&addr1, String1, 0);
    }
    if (Substrings > 1)
    {
      DEBUG ("  getting sockaddr_in for string 2: %s\n", String2);
      netsnmp_sockaddr_in(&addr2, String2, 0);
    }
    else
      DEBUG ("WARNING: netsnmp_brcm_create_tstring called with no address!\n");
      
    // local = 1: this is a server.  Doesn't make much sense to
    // specify more than one address in this case.  Use only the first even
    // if a second is specified.
    if (local)
    {
      pLocalAddr = &addr1;
      pRemoteAddr = NULL;
      
      if (Substrings > 1)
        DEBUG ("WARNING: netsnmp_brcm_create_tstring called for local transport with local and\n"
                "remote addresses specified!  Ignoring remote.\n");

    }
    
    // local = 0: this is a client.  If only one address is specified, then
    // use it as remote and take potluck on which stack it comes from.  If
    // both addresses are specified, use addr1 = local, addr2 = remote.
    else
    {
      if (Substrings == 2)
      {
        pLocalAddr = &addr1;
        pRemoteAddr = &addr2;
      }
      else
      {
        DEBUG ("WARNING: netsnmp_brcm_create_tstring called for remote transport with only\n"
                "remote address specified.  We will be taking pot-luck on the source address.\n");
        pRemoteAddr = &addr1;
        pLocalAddr = NULL;
      }
    }

    return netsnmp_brcm_transport (pLocalAddr, pRemoteAddr);
}


netsnmp_transport *
netsnmp_brcm_create_ostring(const u_char * o, size_t o_len, int local)
{
    struct sockaddr_in addr1;
    struct sockaddr_in addr2;
    struct sockaddr_in *pLocalAddr = NULL, *pRemoteAddr = NULL;
    
    DEBUG ("netsnmp_brcm_create_ostring - %d bytes\n", o_len);
    
    memset (&addr1, 0, sizeof (struct sockaddr_in));
    memset (&addr2, 0, sizeof (struct sockaddr_in));

    if (o_len >= 6) 
    {
        addr1.sin_family = AF_INET;
        memcpy((u_char *) & (addr1.sin_addr.s_addr), o, 4);
        addr1.sin_port = ntohs((o[4] << 8) + o[5]);
    }
    
    if (o_len >= 12)
    {
        addr2.sin_family = AF_INET;
        memcpy((u_char *) & (addr2.sin_addr.s_addr), o+6, 4);
        addr2.sin_port = ntohs((o[10] << 8) + o[11]);
    }
    
    // local = 1: this is a server.  Doesn't make much sense to
    // specify more than one address in this case.
    if (local)
    {
      if (o_len > 6)
        DEBUG ("WARNING: netsnmp_brcm_create_ostring called for local transport with local and\n"
                "remote addresses specified!  Ignoring remote.\n");
                
      pLocalAddr = &addr1;
    }
    
    // local = 0: this is a client.  If only one address is specified, then
    // use it as remote and take potluck on which stack it comes from.  If
    // both addresses are specified, use addr1 = local, addr2 = remote.
    else
    {
      // Both specified:
      if (o_len >= 12)
      {
        pLocalAddr = &addr1;
        pRemoteAddr = &addr2;
      }
      
      // Only one specified:
      else if (o_len >= 6)
      {
        DEBUG ("WARNING: netsnmp_brcm_create_ostring called for remote transport with only\n"
                "remote address specified.  We will be taking pot-luck on the source address.\n");
      
        pLocalAddr = NULL;
        pRemoteAddr = &addr1;
      }
    }
    
    return netsnmp_brcm_transport (pLocalAddr, pRemoteAddr);
}


// Register our BRCM domain
void
netsnmp_brcm_ctor(void)
{
    netsnmp_transport *t;
    netsnmp_tdomain *pUdpDomain;
    
    
    DEBUG ("netsnmp_brcm_ctor:  creating custom BRCM transport domain.\n");

    // Get a standard UDP domain just so we can remember his callbacks.
    t = netsnmp_tdomain_transport("udp:420", 1, "udp");
    
    if (t == NULL)
    {
      DEBUG ("netsnmp_brcm_ctor: couldn't get UDP transport to create BRCM domain!\n");
      return;
    }
    
    // Remember callbacks
    udpRecv = t->f_recv;
    udpSend = t->f_send;
    udpClose = t->f_close;
    udpFmtaddr = t->f_fmtaddr;
    
    // Close the socket
    if (t->sock >= 0)
    {
        close (t->sock);
        t->sock = -1;
    }
    
    // Free the transport
    netsnmp_transport_free (t);
    
    // De-register the UDP domain.  We shall take it over!
    pUdpDomain = netsnmp_tdomain_find (netsnmpBrcmDomain, netsnmpBrcmDomain_len);
    if (pUdpDomain)
    {
      DEBUG ("  Found existing UDP domain @ %p, unregistering.\n", pUdpDomain);
      
      if (netsnmp_tdomain_unregister(pUdpDomain) == 0)
        DEBUG ("netsnmp_brcm_ctor: netsnmp_tdomain_unregister for UDP failed!\n");
      else
        DEBUG ("  UDP domain un-registered OK.\n");
    }

    // Now set up our custom domain.  Basically it's just a UDP domain
    // but we hook the callbacks.  Now if somebody calls one of the netsnmp
    // functions where you specify "udp" you get one of our domains!
    brcmDomain.name = netsnmpBrcmDomain;
    brcmDomain.name_length = netsnmpBrcmDomain_len;
    brcmDomain.prefix = calloc(2, sizeof(char *));
    brcmDomain.prefix[0] = "udp";

    brcmDomain.f_create_from_tstring = netsnmp_brcm_create_tstring;
    brcmDomain.f_create_from_ostring = netsnmp_brcm_create_ostring;
    
    DEBUG ("  Domain created, registering.\n");
    
    if (netsnmp_tdomain_register(&brcmDomain) == 0)
      DEBUG ("netsnmp_brcm_ctor: netsnmp_tdomain_register failed!\n");
    else
      DEBUG ("  Domain registered OK.\n");
}
