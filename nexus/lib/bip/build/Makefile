#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
# Module Description:
#
#############################################################################

B_THIS_DIR:=nexus/lib/bip/build

ifdef B_REFSW_REAL_NEXUS_TOP
    NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
# Uncomment the following two lines to enable recipe tracing.
# B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
# B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"

#  Start by defining the help text displayed by "make help".
define BIP_HELP_TEXT
\n  Usage Information:                                                     \
\n                                                                         \
\n      BIP consists of three "component" libraries:                       \
\n      <component> refers to one of the following                         \
\n          client              Client-specific functionality              \
\n          server              Server-specific functionality              \
\n          base                Functionality shared by client and server  \
\n                              or independent of client and server        \
\n                                                                         \
\n      BIP requires the following prerequisite sub-libraries:             \
\n      <sublib> refers to one of the following:                           \
\n          b_os                                                           \
\n          livemedia                                                      \
\n          blive_ext                                                      \
\n          tshdrbuilder                                                   \
\n          psip                                                           \
\n          openssl                                                        \
\n          playback_ip                                                    \
\n          dtcp_ip                                                        \
\n                                                                         \
\n  For Building...                                                                                                         \
\n  ===============                                                                                                         \
\n  make                                Builds all BIP components and prerequisite sub-libraries                            \
\n  make <component>                    Builds only the specified BIP component and its prerequisite sub-libraries          \
\n  make sublibs                        Builds all BIP prerequisite sub-libraries                                           \
\n  make sublibs_<component>            Builds only the sub-libraries needed by the specified BIP component                 \
\n  make <sublib>                       Builds only the specified sub-library                                               \
\n  make docs                           Generate BIP HTML documentation.                                                    \
\n                                                                                                                          \
\n  For Installing...                                                                                                       \
\n  ================                                                                                                        \
\n  make install                        Installs all BIP components and prerequisite sub-libraries                          \
\n  make install_<component>            Installs only the specified BIP component and its prerequisite sub-libraries        \
\n  make install_sublibs                Installs all BIP prerequisite sub-libraries                                         \
\n  make install_sublibs_<component>    Installs only the sub-libraries needed by the specified BIP component               \
\n  make install_<sublib>               Installs only the specified sub-library                                             \
\n                                                                                                                          \
\n  For Cleaning...                                                                                                         \
\n  ===============                                                                                                         \
\n      There are three levels of cleaning:                                                                                 \
\n          minclean:   Cleans BIP components, but no prerequisite sublibraries                                             \
\n          clean:      Cleans BIP components and prerequisite sublibs except for: $(BIP_LIB_DISTCLEAN_SUBLIBS)             \
\n          veryclean:  Cleans BIP components and all prerequisite sublibs.                                                 \
\n                                                                                                                          \
\n  make minclean                       Cleans all BIP components (but does not clean any sub-libraries)                    \
\n  make minclean_<component>           Cleans only the specified BIP component (but not any of its sub-libraries)          \
\n                                                                                                                          \
\n  make clean                          Cleans all BIP components and selective prerequisite sub-libraries                  \
\n  make clean_<component>              Cleans only the specified BIP component and selective prerequisite sub-libraries    \
\n  make clean_sublibs                  Cleans selective BIP prerequisite sub-libraries                                     \
\n  make clean_sublibs_<component>      Cleans some of the sub-libraries needed by the specified BIP component              \
\n  make clean_<sublib>                 Cleans the specified sub-library                                                    \
\n  make clean_docs                     Cleans the BIP HTML documentation generated by "make docs".                         \
\n                                                                                                                          \
\n  make veryclean                       Cleans all BIP components and all prerequisite sub-libraries                       \
\n  make veryclean_<component>           Cleans only the specified BIP component and all of its prerequisite sub-libraries  \
\n  make veryclean_sublibs               Cleans all BIP prerequisite sub-libraries                                          \
\n  make veryclean_sublibs_<component>   Cleans all of the sub-libraries needed by the specified BIP component              \
\n  make veryclean_<sublib>              Cleans the specified sub-library                                                   \
\n
endef

#  Include our own BIP definitions.
include $(NEXUS_TOP)/lib/bip/bip_lib.inc

BSEAV_BIN_DIR=${B_REFSW_OBJ_ROOT}/BSEAV/bin

#   For example:
#       make base           Builds "base" component and its sublibs.
#       make server         Builds "server" component and its sublibs.
#
#
########################################################################################
#  Define high-level build targets for each of the BIP libraries.
#  ("make base", "make client", "make server").
########################################################################################
.PHONY: all  base  client  server
all:         base  client  server

# Client and Server libs both require Base.
# And Server needs Client for the IP source
client: base
server: base client

#  Allow for synonyms: bip_base, bip_client, bip_server.
.PHONY: bip_base bip_client bip_server
bip_base  : base
bip_client: client
bip_server: server

base  client  server:  BIP_LIB_PREFIX_TEXT = $(BIP_LIB_PREFIX_FOR_NICKNAME_$*)
base  client  server:  %: sublibs_%
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... bip_$*]"
	$(Q_)$(MAKE) -f Makefile.bip_lib  BIP_LIB_PREFIX=$(BIP_LIB_PREFIX_TEXT)  BIP_LIB_OUTPUT_NAME=bip_$*
	$(B_TRACE_RECIPE_END)

##############################################################################################
#  Define "clean" targets for each of the BIP libraries.
#  ("make clean_base", "make clean_client", "make clean_server").
##############################################################################################
.PHONY: clean  clean_%
clean:         clean_base  clean_client  clean_server

clean_base  clean_client  clean_server:  BIP_LIB_PREFIX_TEXT = $(BIP_LIB_PREFIX_FOR_NICKNAME_$*)
clean_base  clean_client  clean_server:  clean_%: clean_sublibs_%
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... bip_$*]"
	$(Q_)$(MAKE)  -f Makefile.bip_lib  BIP_LIB_PREFIX=$(BIP_LIB_PREFIX_TEXT)  BIP_LIB_OUTPUT_NAME=bip_$*  clean
	$(B_TRACE_RECIPE_END)

##############################################################################################
#  Define "minclean" targets for each of the BIP libraries.
#  ("make minclean_base", "make minclean_client", "make minclean_server").
#  The "minclean" target cleans only the respective BIP library, but none of the sub-libraries.
##############################################################################################
.PHONY: minclean  minclean_%
minclean:         minclean_base  minclean_client  minclean_server

minclean_base  minclean_client  minclean_server:  BIP_LIB_PREFIX_TEXT = $(BIP_LIB_PREFIX_FOR_NICKNAME_$*)
minclean_base  minclean_client  minclean_server:  minclean_%:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[MinClean... $*]"
	$(Q_)$(MAKE)  -f Makefile.bip_lib  BIP_LIB_PREFIX=$(BIP_LIB_PREFIX_TEXT)  BIP_LIB_OUTPUT_NAME=bip_$*  clean
	$(B_TRACE_RECIPE_END)

##############################################################################################
#  Define "veryclean" targets for each of the BIP libraries.
#  ("make veryclean_base", "make veryclean_client", "make veryclean_server").
##############################################################################################
.PHONY: veryclean  veryclean_%
veryclean:         veryclean_base  veryclean_client  veryclean_server

veryclean_base  veryclean_client  veryclean_server:  BIP_LIB_PREFIX_TEXT = $(BIP_LIB_PREFIX_FOR_NICKNAME_$*)
veryclean_base  veryclean_client  veryclean_server:  veryclean_%: veryclean_sublibs_%
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Distclean... bip_$*]"
	$(Q_)$(MAKE)  -f Makefile.bip_lib  BIP_LIB_PREFIX=$(BIP_LIB_PREFIX_TEXT)  BIP_LIB_OUTPUT_NAME=bip_$*  clean
	$(B_TRACE_RECIPE_END)

##############################################################################################
#  Define "install" targets for each of the BIP libraries.
#  ("make install_base", "make install_client", "make install_server").
##############################################################################################
.PHONY: install  install_%
install:         install_base  install_client  install_server

# client and server libs both require base.
install_client: install_base
install_server: install_base

install_base  install_client  install_server:  BIP_LIB_PREFIX_TEXT = $(BIP_LIB_PREFIX_FOR_NICKNAME_$*)
install_base  install_client  install_server:  install_%: install_sublibs_%
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... bip_$*]"
	$(Q_)$(MAKE)  -f Makefile.bip_lib  BIP_LIB_PREFIX=$(BIP_LIB_PREFIX_TEXT)  BIP_LIB_OUTPUT_NAME=bip_$*  $(if $(INSTALL_DIR),B_LIB_INSTALL_DIR?=$(INSTALL_DIR),)  install
	$(B_TRACE_RECIPE_END)

##############################################################################################
#  Now handle the various "sub-libraries" used by BIP, things like OpenSSL, Livemedia, etc.
#  Define build targets for sub-libraries of each BIP library.
#  ("make sublibs_base", "make sublibs_client", "make sublibs_server").
##############################################################################################

.PHONY: sublibs  sublibs_base  sublibs_client  sublibs_server
sublibs:         sublibs_base  sublibs_client  sublibs_server
sublibs_base:    $(foreach   lib,$(BIP_BASE_LIB_SUBLIBS),$(lib))
sublibs_client:  $(foreach   lib,$(BIP_CLIENT_LIB_SUBLIBS),$(lib))
sublibs_server:  $(foreach   lib,$(BIP_SERVER_LIB_SUBLIBS),$(lib))

##############################################################################################
#  Define "clean" targets for sub-libraries of each BIP library.
#  ("make clean_sublibs_base", "make clean_sublibs_client", "make clean_sublibs_server").
#  Normal "clean" skips cleaning of sublibs in the BIP_LIB_DISTCLEAN_SUBLIBS list.
##############################################################################################
.PHONY: clean_sublibs  clean_sublibs_base  clean_sublibs_client  clean_sublibs_server
clean_sublibs:         clean_sublibs_base  clean_sublibs_client  clean_sublibs_server
clean_sublibs_base:    $(foreach   lib,$(filter-out $(BIP_LIB_DISTCLEAN_SUBLIBS),$(BIP_BASE_LIB_SUBLIBS)),clean_$(lib))
clean_sublibs_client:  $(foreach   lib,$(filter-out $(BIP_LIB_DISTCLEAN_SUBLIBS),$(BIP_CLIENT_LIB_SUBLIBS)),clean_$(lib))
clean_sublibs_server:  $(foreach   lib,$(filter-out $(BIP_LIB_DISTCLEAN_SUBLIBS),$(BIP_SERVER_LIB_SUBLIBS)),clean_$(lib))

##############################################################################################
#  Define "veryclean" targets for sub-libraries of each BIP library.
#  ("make veryclean_sublibs_base", "make veryclean_sublibs_client", "make veryclean_sublibs_server").
#  Same as "clean" except that there is no skipping of libs in the BIP_LIB_DISTCLEAN_SUBLIBS list.
##############################################################################################
.PHONY: veryclean_sublibs  veryclean_sublibs_base  veryclean_sublibs_client  veryclean_sublibs_server
veryclean_sublibs:         veryclean_sublibs_base  veryclean_sublibs_client  veryclean_sublibs_server
veryclean_sublibs_base:    $(foreach   lib,$(BIP_BASE_LIB_SUBLIBS),clean_$(lib))
veryclean_sublibs_client:  $(foreach   lib,$(BIP_CLIENT_LIB_SUBLIBS),clean_$(lib))
veryclean_sublibs_server:  $(foreach   lib,$(BIP_SERVER_LIB_SUBLIBS),clean_$(lib))

##############################################################################################
#  Define "install" targets for sub-libraries of each BIP library.
#  ("make install_sublibs_base", "make install_sublibs_client", "make install_sublibs_server").
##############################################################################################
.PHONY: install_sublibs  install_sublibs_base  install_sublibs_client  install_sublibs_server
install_sublibs:         install_sublibs_base  install_sublibs_client  install_sublibs_server
install_sublibs_base:    $(foreach   lib,$(BIP_BASE_LIB_SUBLIBS),install_$(lib))
install_sublibs_client:  $(foreach   lib,$(BIP_CLIENT_LIB_SUBLIBS),install_$(lib))
install_sublibs_server:  $(foreach   lib,$(BIP_SERVER_LIB_SUBLIBS),install_$(lib))

###############################################################################
#  Define build/clean/install targets for each individual sub-library.
###############################################################################
#  b_os Targets  ("b_os", "clean_b_os", "install_b_os").
###############################################################################
.PHONY: b_os
b_os:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os
	$(B_TRACE_RECIPE_END)

.PHONY: clean_b_os
clean_b_os:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os clean
	$(B_TRACE_RECIPE_END)

.PHONY: install_b_os
install_b_os:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os install
	$(B_TRACE_RECIPE_END)

#########################################################################################
#  livemedia Targets  ("livemedia", "clean_livemedia", "install_livemedia").
#########################################################################################
# Suppress LiveMedia Output for non VERBOSE builds
ifeq ($(VERBOSE),)
    REDIR_LIVEMEDIA := &> /dev/null
else
    REDIR_LIVEMEDIA :=
endif

.PHONY: livemedia
livemedia:
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Build... livemedia]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia         $(REDIR_LIVEMEDIA)
		$(B_TRACE_RECIPE_END)
    endif #LIVEMEDIA_SUPPORT

.PHONY: clean_livemedia
clean_livemedia:
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Clean... livemedia]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia  clean  $(REDIR_LIVEMEDIA)
		$(B_TRACE_RECIPE_END)
    endif #LIVEMEDIA_SUPPORT

.PHONY: install_livemedia
install_livemedia:  livemedia
    #  No "install" target for livemedia

#########################################################################################
#  blive_ext Targets  ("blive_ext", "clean_blive_ext", "install_blive_ext").
#########################################################################################
.PHONY: blive_ext
blive_ext:          livemedia  # Can't build blive_ext until livemedia is build
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Build... blive_ext]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia/blive_ext/build
		$(B_TRACE_RECIPE_END)
    endif #LIVEMEDIA_SUPPORT

.PHONY: clean_blive_ext
clean_blive_ext:
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Clean... blive_ext]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia/blive_ext/build clean
		$(B_TRACE_RECIPE_END)
    endif #LIVEMEDIA_SUPPORT

.PHONY: install_blive_ext
install_blive_ext:      install_livemedia
    #  No "install" target for blive_ext

#################################################################################################
#  tshdrbuilder Targets  ("tshdrbuilder", "clean_tshdrbuilder", "install_tshdrbuilder").
#################################################################################################
.PHONY: tshdrbuilder
tshdrbuilder:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... tshdrbuilder]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/tshdrbuilder
	$(B_TRACE_RECIPE_END)

.PHONY: clean_tshdrbuilder
clean_tshdrbuilder:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... tshdrbuilder]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/tshdrbuilder clean
	$(B_TRACE_RECIPE_END)

.PHONY: install_tshdrbuilder
install_tshdrbuilder:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... tshdrbuilder]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/tshdrbuilder install
	$(B_TRACE_RECIPE_END)

###############################################################################
#  psip Targets  ("psip", "clean_psip", "install_psip").
###############################################################################
.PHONY: psip
psip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... psip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../rockford/lib/psip
	$(B_TRACE_RECIPE_END)

.PHONY: clean_psip
clean_psip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... psip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../rockford/lib/psip clean
	$(B_TRACE_RECIPE_END)

.PHONY: install_psip
install_psip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... psip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../rockford/lib/psip install
	$(B_TRACE_RECIPE_END)

###############################################################################
#  openssl Targets  ("openssl", "clean_openssl", "install_openssl").
#
#  We'll take some shortcuts here, because the OpenSSL makefile takes quite
#  a long time (several seconds) when everything is up to date.  So as long
#  as the OpenSSL libraries exist, we'll assume that they're up to date.
#
#  Of course, this means that if somebody is changing something (like endianess)
#  that requires that OpenSSL gets rebuild, then they will need to do a
#  "make openssl_veryclean" that will force it to be rebuilt.
###############################################################################

# Build a list of places that we need to put the test CA certificate "host.cert".
SSL_CERT_FILE := host.cert
SSL_CERT_FILE_DEST_LIST :=
ifneq ($(BIP_OPENSSL_LIB_MAYBE),)

    SSL_CERT_FILE_DEST_LIST += $(abspath $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE))

    ifeq ($(shell test $(BSEAV_BIN_DIR) && test -d $(BSEAV_BIN_DIR) && echo y),y)
        SSL_CERT_FILE_DEST_LIST += $(abspath $(BSEAV_BIN_DIR)/$(SSL_CERT_FILE))
    endif
    ifeq ($(shell test $(INSTALL_DIR) && test -d $(INSTALL_DIR) && echo y),y)
        SSL_CERT_FILE_DEST_LIST += $(abspath $(INSTALL_DIR)/$(SSL_CERT_FILE))
    endif
    # Sort to eliminate duplicates.
    SSL_CERT_FILE_DEST_LIST := $(sort $(SSL_CERT_FILE_DEST_LIST))
endif

#  Here's the rule that will copy the host.cert file.
ifneq ($(SSL_CERT_FILE_DEST_LIST),)
    $(SSL_CERT_FILE_DEST_LIST): $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE)
		$(B_TRACE_RECIPE_BEGIN)
		${Q_}$(MKDIR) -p $(dir $@)
		${Q_}$(CP) -dfp  $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) $@
		$(B_TRACE_RECIPE_END)
endif


.PHONY: openssl

ifneq ($(BIP_OPENSSL_LIB_MAYBE),)
    # Don't bother with the OpenSSL make if the its ".so" files exist.
    openssl:  $(NEXUS_BIN_DIR)/libcrypto.so  $(NEXUS_BIN_DIR)/libssl.so \
              $(SSL_CERT_FILE_DEST_LIST)
else
    openssl:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Skipping... OpenSSL]"
	$(B_TRACE_RECIPE_END)
endif   #  ifneq ($(BIP_OPENSSL_LIB_MAYBE),)

# If the OpenSSL libraries are missing from NEXUS_BIN_DIR, then copy them from the OpenSSL install dir.
# Note the use of a "pattern rule" so that the recipe will only run once to generate both targets.
$(NEXUS_BIN_DIR)/%crypto.so  $(NEXUS_BIN_DIR)/%ssl.so:   $(B_OPENSSL_LIB_INSTALL_DIR)/%crypto.so  $(B_OPENSSL_LIB_INSTALL_DIR)/%ssl.so
    ifneq ($(BIP_OPENSSL_LIB_MAYBE),)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Install... OpenSSL]"
		${Q_}$(MKDIR) -p $(NEXUS_BIN_DIR)
		${Q_}$(CP) -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so*   $(NEXUS_BIN_DIR)
		${Q_}$(CP) -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so*      $(NEXUS_BIN_DIR)
        # export B_OPENSSL_BUILD_COMPLETED=y To prevent submakes from rebuilding OpenSSL
		$(eval export B_OPENSSL_BUILD_COMPLETED=y)
		$(B_TRACE_RECIPE_END)
    endif   #  ifneq ($(BIP_OPENSSL_LIB_MAYBE),)

# If the OpenSSL ".so" files are missing from the OpenSSL install dir, then we'll run the OpenSSL make.
# Again, use a "pattern rule" because running the recipe once will generate all (both) of the targets.
$(B_OPENSSL_LIB_INSTALL_DIR)/%crypto.so $(B_OPENSSL_LIB_INSTALL_DIR)/%ssl.so:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... OpenSSL]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/openssl
	$(B_TRACE_RECIPE_END)

.PHONY: clean_openssl
clean_openssl:
ifneq ($(BIP_OPENSSL_LIB_MAYBE),)
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... OpenSSL]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/openssl clean
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/openssl target-clean
	$(Q_)rm -f $(SSL_CERT_FILE_DEST_LIST)
	$(B_TRACE_RECIPE_END)
endif   #  ifneq ($(BIP_OPENSSL_LIB_MAYBE),)

.PHONY: install_openssl
# Don't bother with the OpenSSL make if its ".so" files exist (we'll just assume that they're up to date
# since supposedly "OpenSSL never changes".
ifneq ($(BIP_OPENSSL_LIB_MAYBE),)
    install_openssl: $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so  $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so \
                     $(SSL_CERT_FILE_DEST_LIST)
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Install... OpenSSL]"
		${Q_}$(MKDIR) -p $(NEXUS_BIN_DIR)
		${Q_}$(CP) -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so*   $(NEXUS_BIN_DIR)
		${Q_}$(CP) -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so*      $(NEXUS_BIN_DIR)
        # export B_OPENSSL_BUILD_COMPLETED=y To prevent submakes from rebuilding OpenSSL
		$(eval export B_OPENSSL_BUILD_COMPLETED=y)
		$(B_TRACE_RECIPE_END)
else
    install_openssl:
		$(B_TRACE_RECIPE_BEGIN)
		@echo "[Skipping... OpenSSL]"
		$(B_TRACE_RECIPE_END)
endif   #  ifneq ($(BIP_OPENSSL_LIB_MAYBE),)

###########################################################################################
#  playback_ip Targets  ("playback_ip", "clean_playback_ip", "install_playback_ip").
###########################################################################################
.PHONY: playback_ip
playback_ip:   b_os livemedia blive_ext tshdrbuilder psip openssl
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... playback_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/playback_ip
	$(B_TRACE_RECIPE_END)

.PHONY: clean_playback_ip
clean_playback_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... playback_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/playback_ip clean
	$(B_TRACE_RECIPE_END)

.PHONY: install_playback_ip
install_playback_ip:        install_b_os install_livemedia install_blive_ext install_tshdrbuilder install_psip install_openssl
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... playback_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/playback_ip install
	$(B_TRACE_RECIPE_END)


###########################################################################################
#  dtcp_ip Targets  ("dtcp_ip", "clean_dtcp_ip", "install_dtcp_ip").
###########################################################################################

#  The following variables were set appropriately in bip_lib.inc.
#    BIP_DTCP_IP_LIB_DIR_NAME:  either "dtcp_ip" or "dtcp_ip_sage"
#    BIP_OPENSSL_LIB_MAYBE:  either "" or "openssl"

.PHONY: dtcp_ip
dtcp_ip:   $(if $(BIP_OPENSSL_LIB_MAYBE),openssl,)
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... dtcp_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/$(BIP_DTCP_IP_LIB_DIR_NAME)
	$(B_TRACE_RECIPE_END)

.PHONY: clean_dtcp_ip
clean_dtcp_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... dtcp_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/$(BIP_DTCP_IP_LIB_DIR_NAME) clean
	$(B_TRACE_RECIPE_END)

.PHONY: install_dtcp_ip
install_dtcp_ip:   $(if $(BIP_OPENSSL_LIB_MAYBE),install_openssl,)
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... dtcp_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/$(BIP_DTCP_IP_LIB_DIR_NAME) install
	$(B_TRACE_RECIPE_END)

###############################################################################
#  dtcp_ip_ext Targets  ("dtcp_ip_ext", "clean_dtcp_ip_ext").
###############################################################################
.PHONY: dtcp_ip_ext
dtcp_ip_ext: openssl  dtcp_ip
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... dtcp_ip_ext]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/security/third_party/dtcp_ip install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_dtcp_ip_ext
clean_dtcp_ip_ext:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... dtcp_ip_ext]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/security/third_party/dtcp_ip clean
	$(B_TRACE_RECIPE_END)

########################################################################################
#  Define build targets to generate BIP's HTML documentation.
#  ("make docs", "make clean_docs").
########################################################################################
BIP_DOXYGEN_BLD_DIR=$(abspath $(B_REFSW_OBJ_ROOT)/doxygenBuild)
BIP_DOXYGEN_OUT_DIR=$(BIP_DOXYGEN_BLD_DIR)/output
BIP_DOXYGEN_CONFIG_DIR=$(abspath $(NEXUS_TOP)/../BSEAV/tools/doxygen)

BIP_DOXYGEN_REDIR := $(if $(VERBOSE)$(B_REFSW_VERBOSE) ,,&>/dev/null)

docs: $(BIP_DOXYGEN_OUT_DIR) docs_config_files  docs_to_doxygen run_doxygen

$(BIP_DOXYGEN_BLD_DIR)  $(BIP_DOXYGEN_OUT_DIR):
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Creating... $(BIP_DOXYGEN_OUT_DIR)]"
	$(Q_)$(MKDIR) -p $(BIP_DOXYGEN_OUT_DIR)
	$(B_TRACE_RECIPE_END)

.PHONY: docs_config_files
docs_config_files:  $(BIP_DOXYGEN_BLD_DIR)
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Copying... $(BIP_DOXYGEN_CONFIG_DIR)/*]"
	$(Q_)$(CP) -r -u $(BIP_DOXYGEN_CONFIG_DIR)/* $(BIP_DOXYGEN_BLD_DIR)
	$(B_TRACE_RECIPE_END)

.PHONY: docs_to_doxygen
docs_to_doxygen:  docs_config_files
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Converting from Docjet to Doxygen...]"
	$(Q_)$(RM) -f $(BIP_DOXYGEN_BLD_DIR)/nexus
	$(Q_)ln -sf $(NEXUS_TOP)  $(BIP_DOXYGEN_BLD_DIR)/nexus
	$(Q_)pushd  $(BIP_DOXYGEN_CONFIG_DIR) $(BIP_DOXYGEN_REDIR) && perl doxy_xlate.pl BIP  $(BIP_DOXYGEN_BLD_DIR)/.   $(BIP_DOXYGEN_REDIR)  ; popd $(BIP_DOXYGEN_REDIR)
	$(Q_)$(RM) -f $(BIP_DOXYGEN_BLD_DIR)/nexus
	$(B_TRACE_RECIPE_END)

.PHONY: run_doxygen
run_doxygen: docs_to_doxygen
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Running Doxygen...]"
	$(Q_)pushd $(BIP_DOXYGEN_BLD_DIR)/source/BIP/nexus $(BIP_DOXYGEN_REDIR)  && CHIP=BIP DOXYGENVER=1.7.1   doxygen    ../../../nexus.doxyfile  $(BIP_DOXYGEN_REDIR)   ; popd $(BIP_DOXYGEN_REDIR)
	@echo "[Done. Docs are here: file://$(BIP_DOXYGEN_OUT_DIR)/BIP/nexus/group__bip.html]"
	$(B_TRACE_RECIPE_END)

.PHONY: clean_docs
clean_docs:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Cleaning...]"
	$(Q_)$(RM) -rf $(BIP_DOXYGEN_BLD_DIR)
	$(B_TRACE_RECIPE_END)

##   examples:
##   install_examples:
##   clean_examples:
##
##   utils:
##   install_utils:
##   clean_utils:
##

#  Put "help" rule last.
help:
	@echo -e "$(BIP_HELP_TEXT)"
#
#  The End.
