#
# SES Linux Builds Makefile
#
# Copyright (C) 2017, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile 271576 2011-07-12 20:31:22Z $
#
# targets/builds
BUILDS = ses

# default build
.PHONY: all
all : ${BUILDS}

.PHONY: ${BUILDS}
${BUILDS} ::
	+$(MAKE) -C $@

# clean up
BUILDS-CLEAN = ${BUILDS:=-clean}

.PHONY: clean
clean : ${BUILDS-CLEAN}

.PHONY: ${BUILDS-CLEAN}
${BUILDS-CLEAN} ::
	+$(MAKE) -C ${@:-clean=} clean

# install
BUILDS-INSTALL = ${BUILDS:=-install}

.PHONY: install
install : ${BUILDS-INSTALL}

.PHONY: ${BUILDS-INSTALL}
${BUILDS-INSTALL} ::
	+$(MAKE) -C ${@:-install=} install
