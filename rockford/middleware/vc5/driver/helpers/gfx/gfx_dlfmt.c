/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/gfx/gfx_lfmt.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   if (argc != 2) {
      printf("usage: %s <lfmt>\n", argv[0]);
      return 1;
   }

   GFX_LFMT_SPRINT(desc, (GFX_LFMT_T)strtoul(argv[1], NULL, 0));
   printf("%s\n", desc);
   return 0;
}
