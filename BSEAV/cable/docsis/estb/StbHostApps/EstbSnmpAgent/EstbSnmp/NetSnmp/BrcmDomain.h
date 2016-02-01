#ifndef _BRCMDOMAIN_H
#define _BRCMDOMAIN_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

netsnmp_transport *netsnmp_brcm_transport(struct sockaddr_in *localAddr, struct sockaddr_in *remoteAddr);


/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_brcm_ctor(void);

#ifdef __cplusplus
}
#endif
#endif/*_SNMPUDPDOMAIN_H*/
