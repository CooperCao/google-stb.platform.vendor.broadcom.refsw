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
//  Filename:       dsgcc_clnt_if.h
//  Author:         Jean Cyr
//  Creation Date:  Dec 12, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

#ifndef DSGCCCLNTIF_H
#define DSGCCCLNTIF_H

#ifdef __cplusplus
extern "C" {
#endif

int registerdsgclient(struct dsgClientRegInfo *argp);

int unregisterdsgclient(struct dsgClientRegInfo *argp);

int keepalivedsgclient(struct dsgClientRegInfo *argp);

#ifdef __cplusplus
}
#endif

#endif
