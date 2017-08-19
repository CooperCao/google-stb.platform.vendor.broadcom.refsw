#
# larq/sys/makefile.inc
#
# Copyright 1999, Epigram, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Epigram, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of Epigram, Inc.
#
# $Id: makefile.inc,v 1.2 1999-08-13 22:25:54 $
#

$(TARGETPATH)\$(TARGET_DIRECTORY)\$(DDKBUILDENV)\nul :
	mkdir $(TARGETPATH)\$(TARGET_DIRECTORY)\$(DDKBUILDENV)

always : 
	rem echo always build this target


.SUFFIXES: .sys .nms
!IFDEF NOPASS0
{$(TARGETPATH)\$(TARGET_DIRECTORY)\checked}.sys{$(TARGETPATH)\$(TARGET_DIRECTORY)\checked}.nms:
!IFDEF NTICE
	$(NTICE:/=\)\nmsym.exe -translate:always,source,package \
		-prompt -source:../../.. -source:../../../../../include \
		$(@R).SYS
!ELSE
	@echo do nothing to build nms files because NTICE is not defined.
!ENDIF

!ELSE
{$(TARGETPATH)\$(TARGET_DIRECTORY)\checked}.sys{$(TARGETPATH)\$(TARGET_DIRECTORY)\checked}.nms:
	@echo do nothing for checked builds in PASS0

!ENDIF

# Do nothing for the free nms file.
{$(TARGETPATH)\$(TARGET_DIRECTORY)\free}.sys{$(TARGETPATH)\$(TARGET_DIRECTORY)\free}.nms:
	@echo do nothing for free nms files
