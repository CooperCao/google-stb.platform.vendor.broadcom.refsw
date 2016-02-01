/*****************************************************************************

  Copyright (c) 2007  Broadcom Corporation
  All Rights Reserved
  No portions of this material may be reproduced in any form without the
  written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
  All information contained in this document is Broadcom Corporation
  company private, proprietary, and trade secret.

****************************************************************************

  Filename:       dsgcc_clnt_if.h
  Author:         Maurice Turcotte
  Creation Date:  Dec 12, 2004

****************************************************************************
  Description:


****************************************************************************/

#ifndef DSGCCCLNTIF_H
#define DSGCCCLNTIF_H

#include "dsgccClientRegistration_rpc.h"

int 
registerdsgclient(struct dsgClientRegInfo *argp);

int
unregisterdsgclient(struct dsgClientRegInfo *argp);

int
keepalivedsgclient(struct dsgClientRegInfo *argp);

int 
registerdsgclientwithclassifier( struct dsgClientRegInfo *argp, struct dsg_classifier *argp_dsg_classifier);

#endif
