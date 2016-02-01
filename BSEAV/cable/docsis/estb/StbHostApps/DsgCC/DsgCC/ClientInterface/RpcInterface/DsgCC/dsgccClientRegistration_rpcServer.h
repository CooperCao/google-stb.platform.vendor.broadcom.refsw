//****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       dsgcc_svc_if.h
//  Author:         Maurice Turcotte
//  Creation Date:  Nov 25, 2004
//
//****************************************************************************
//  Description:
//  C to C++ mapping for the RPC interface to the DSG Client Registrar.
//****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif


int *
registerdsgclient_1_svc(struct dsgClientRegInfo *regInfo,
                        struct svc_req * svcReqInfo);

int *
unregisterdsgclient_1_svc(struct dsgClientRegInfo *regInfo, 
                          struct svc_req * svcReqInfo);

int *
keepalivedsgclient_1_svc(struct dsgClientRegInfo *regInfo,
                         struct svc_req * svcReqInfo);

int
initializeRPCinterface(void);

#ifdef __cplusplus
}
#endif

