#ifndef NETSNMPAGNT_H
#define NETSNMPAGNT_H

#if defined (__cplusplus)
extern "C"{
#endif

// For 'raw' API communications with extension apps that know how to
// talk to Net-SNMP.
typedef struct _NetSnmpRawApiRec
{
  netsnmp_mib_handler *handler;
  netsnmp_handler_registration *reginfo;
  netsnmp_agent_request_info *reqinfo;
  netsnmp_request_info *requests; // first in the chain
  netsnmp_request_info *request; // currently being processed
} NetSnmpRawApiRec;

#if defined (__cplusplus)
}
#endif

#endif

