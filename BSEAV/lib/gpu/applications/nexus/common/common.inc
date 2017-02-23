###############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
# All rights reserved.
###############################################################################

BASEPATH ?= BSEAV
V3D_DIR ?= $(shell cd $(NEXUS_TOP)/../$(BASEPATH)/lib/gpu/$(V3D_PREFIX)/driver; pwd)
V3D_PLATFORM_DIR ?= $(shell cd $(NEXUS_TOP)/../$(BASEPATH)/lib/gpu/$(V3D_PREFIX)/platform; pwd)
