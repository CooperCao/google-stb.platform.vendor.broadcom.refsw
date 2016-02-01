############################################################
#	  (c)2012 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################
ROOT_DIR=$(TOPDIR)
ROOT = $(subst \,/,$(ROOT_DIR))
TOPDIR := $(ROOT)

REFSWROOT_UNIX = $(subst \,/,$(REFSWROOT))
REFSWROOT := $(REFSWROOT_UNIX)

MAGNUM	 	    = $(REFSWROOT)/magnum
ROCKFORD 	    = $(REFSWROOT)/rockford

include board.mak

OBJDIR=objs

SRCDIRS= ./simpleapp
include libs.mak

# Make vpath from SRCDIRS
vpath %.c $(SRCDIRS)
vpath %.s $(SRCDIRS)
vpath %.S $(SRCDIRS)
vpath %.cpp $(SRCDIRS)

# Generate list of targets for this makefile
TESTOBJS = \
	$(OBJDIR)/simpleinit.o \
	$(OBJDIR)/vxworks_isr.o \

OTFOBJS = \
	$(OBJDIR)/botf_test.o \

MAGNUMOBJS=$(patsubst %, $(OBJDIR)/%.o, $(notdir $(basename $(OBJ) $(R_OBJ))))
OTFOBJS=$(patsubst %, $(OBJDIR)/%.o, $(notdir $(basename $(BOTF_OBJ))))

otftest : $(OBJDIR)/simpleapp.out
	$(LINK.o) -o $@ $(OBJDIR)/simpleapp.out	

#otftest : $(VXWORKSOBJ) $(OBJDIR)/simpleapp.out
#	ldmips -t -y BREG_Open -EB -X -N -e sysInit -Ttext 80010000      \
#	-o vxWorks dataSegPad.o vxWorks.tmp ctdt.o $(OBJDIR)/simpleapp.out -defsym _VX_DATA_ALIGN=1 \
#	-T C:/WindRiver/vxworks-6.1/target/h/tool/gnu/ldscripts/link.MIPSRAM
#	objcopymips --extract-symbol vxWorks vxWorks.sym


$(OBJDIR)/simpleapp.out : $(OBJDIR)/magnum.a $(OBJDIR)/otf.o $(TESTOBJS)
	$(LINK.o) -o $@ $(TESTOBJS) $(OBJDIR)/otf.o $(OBJDIR)/magnum.a

magnum:	$(OBJDIR)/magnum.a

clean_magnum:
	rm -rf $(OBJDIR)/magnum.a $(MAGNUMOBJS)

$(OBJDIR)/magnum.a: $(MAGNUMOBJS)
	$(LIBAR) $@ $(MAGNUMOBJS)

otf: $(OBJDIR)/otf.o

clean_otf:
	rm -rf $(OBJDIR)/otf.o

$(OBJDIR)/otf.o : $(OTFOBJS)
	$(LINK.o) -o $@ $(OTFOBJS)

# standard targets
# include here so that one of them is not the default target

include make.rul
