
#include <rpc/rpc.h>
#include <stdio.h>
#include "dsgccClientRegistration_rpc.h"



int 
registerdsgclient(struct dsgClientRegInfo *argp)
{
  return (*registerdsgclient_1(argp, (CLIENT*)argp->handle));
}

int
unregisterdsgclient(struct dsgClientRegInfo *argp)
{
  return (*unregisterdsgclient_1(argp, (CLIENT*)argp->handle));
}

int
keepalivedsgclient(struct dsgClientRegInfo *argp)
{
  return (*keepalivedsgclient_1(argp, (CLIENT*)argp->handle));
}

int
registerdsgclientwithclassifier( struct dsgClientRegInfo *argp, struct dsg_classifier *argp_dsg_classifier)
{
   dsgClientClassifierRegInfo regInfo;
   memcpy(&regInfo.dsgClient_RegInfo, argp, sizeof(dsgClientRegInfo));
   memcpy(&regInfo.dsgClassifier, argp_dsg_classifier, sizeof(dsg_classifier));

   return (*registerdsgclientwithclassifier_1(&regInfo, (CLIENT*)argp->handle));
}

