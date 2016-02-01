#############################################################################
#     Copyright (c) 2002-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: Makefile $
# $brcm_Revision: Hydra_Software_Devel/4 $
# $brcm_Date: 10/29/13 4:45p $
#
# Module Description:
#
# Makefile for xvd_save_image program.
#
# Revision History:
#
# $brcm_Log: /rockford/applications/xvd_save_FW_image/Makefile $
#
# Hydra_Software_Devel/4   10/29/13 4:45p davidp
# SW7425-5316: Change makefile to use new PI directory structure, also
# add support for Hercules FW.
#
# Hydra_Software_Devel/3   2/23/11 6:11p davidp
# SW7422-22: Use environment variable XVD_CHIP to select the magnum PI
# XVD chip directory.
#
# Hydra_Software_Devel/2   2/22/11 2:17p davidp
# SW7422-22: Use makefile comment prefix.
#
# Hydra_Software_Devel/1   2/11/11 3:34p davidp
# SW7422-22: Initial checkin for xvd_save_image FW signing tool.
#
#############################################################################

ifeq ("$(XVD_CHIP)","")
$(warning WARNING: XVD_CHIP environment variable needs to be set, export XVD_CHIP='target_chip')
$(warning WARNING: Defaulting to XVD_CHIP=7425)
XVD_CHIP = 7425
endif

DEBUG = -O0 -ggdb -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

XVD_SRCDIR = ../../../magnum/portinginterface/xvd/src/
XVD_INCDIR = ../../../magnum/portinginterface/xvd/include/
AFL_DIR = ../../../magnum/commonutils/afl/

xvd_save_image: xvd_save_image.o bafl.o bkni.o bdbg.o
	gcc $(DEBUG) -o xvd_save_image xvd_save_image.o bafl.o bkni.o bdbg.o

xvd_save_image.o: xvd_save_image.c
	gcc $(DEBUG) -DXVD_CHIP=$(XVD_CHIP) -I$(XVD_SRCDIR) -I$(XVD_INCDIR) -I$(AFL_DIR) -I./ -c -o xvd_save_image.o xvd_save_image.c

bafl.o: $(AFL_DIR)bafl.c
	gcc $(DEBUG) -I$(AFL_DIR) -I./ -c -o bafl.o $(AFL_DIR)bafl.c


bkni.o: bkni.c
	gcc $(DEBUG)  -I./ -c -o bkni.o bkni.c


bdbg.o: bdbg.c
	gcc $(DEBUG)  -I./ -c -o bdbg.o bdbg.c

all:	xvd_save_image

clean:
	rm -f *.o xvd_save_image

cleanall:
	rm -f *.o xvd_save_image
