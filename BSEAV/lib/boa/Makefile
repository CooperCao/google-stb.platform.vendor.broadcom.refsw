#############################################################################
#    (c)2010-2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTLIBCURLLAR PURPOSE,
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
#############################################################################
.PHONY: all clean

B_THIS_DIR:=BSEAV/lib/boa
SHELL:=/bin/bash

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus


BSEAV ?= $(abspath ${NEXUS_TOP}/../BSEAV)
BOA_SRC_DIR ?= $(abspath ${NEXUS_TOP}/../${B_THIS_DIR})
BOA_SRC_VER ?= boa-0.94.14rc21

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
export B_REFSW_ARCH
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

ifeq ($(NEXUS_PLATFORM),)
NEXUS_PLATFORM=$(PLATFORM)
endif

$(if ${NEXUS_PLATFORM},,$(error NEXUS_PLATFORM is not defined.))

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if ${NEXUS_BIN_DIR},,$(error NEXUS_BIN_DIR is not defined.))
$(if ${B_REFSW_OBJ_ROOT},,$(error B_REFSW_OBJ_ROOT is not defined.))

OBJ_BOA_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/boa/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
COMPILER := ${B_REFSW_ARCH}-gcc

# DESTDIR is where you want the shared object libraries to end up, default to NEXUS_BIN_DIR
DESTDIR ?= ${NEXUS_BIN_DIR}

# Create some filters to clean up the build output, and to hide compiler warnings.
ifeq ($(B_REFSW_VERBOSE),)
	B_BOA_EXTRACT_REDIR := >/dev/null  && test $${PIPESTATUS[0]}
	B_BOA_PATCH_REDIR :=    |  													\
			egrep --line-buffered  "patching file" |   		                            \
			sed -u 																		\
					-e  "s/^patching file \(.*\)$$/[Patch... \1 (boa)]/ "		    \
			&& test $${PIPESTATUS[0]}
	B_BOA_COMPILE_REDIR := 2>/dev/null  |  								        \
			egrep --line-buffered  "c\+\+.+\.cpp" |   					                \
			sed -u 																        \
					-e  "s/.* \([[:alnum:]]*\.cpp\)$$/[Compile... \1 (boa)]/ "	\
			&& test $${PIPESTATUS[0]}
	B_BOA_INSTALL_REDIR := 2>/dev/null  |  									    \
			egrep --line-buffered  "^cd " |   		                                    \
			sed -u 																		\
					-e  "s/^cd \(.*\) ;.*$$/[Staged Install... \1 (boa)]/ "		\
			&& test $${PIPESTATUS[0]}
endif

WEB_FILES = index.c index.js index.html
all: $(WEB_FILES) ${DESTDIR} ${DESTDIR}/index.cgi
	$(Q_)if [ ! -d $(OBJ_BOA_DIR) ]; then \
		mkdir -p $(OBJ_BOA_DIR); \
		echo "[Extract... (Boa)]" ; \
		(cd $(OBJ_BOA_DIR) && tar xvzf $(BOA_SRC_DIR)/$(BOA_SRC_VER).tar.gz $(B_BOA_EXTRACT_REDIR) ); \
		(cd $(OBJ_BOA_DIR) && chmod 777 $(BOA_SRC_VER) ); \
		echo "[Configure ... (Boa) for ${B_REFSW_ARCH} ]" ; \
		(cd $(OBJ_BOA_DIR)/$(BOA_SRC_VER) && ./configure --host=${B_REFSW_ARCH} $(B_BOA_EXTRACT_REDIR) );  \
		echo "[Patching ... (Boa)]" ; \
		(cd $(OBJ_BOA_DIR)/$(BOA_SRC_VER) && patch -p1 < $(BOA_SRC_DIR)/$(BOA_SRC_VER).patch   $(B_LIVEMEDIA_PATCH_REDIR) ); \
	fi;

    # First do the boa make with the -q (question) option to see if there is anything for it to do.
    # This way, we can avoid the boa staged install if boa is up to date.  This is necessary because
    # the staged install updates the timestamps on the header files and makes everything that includes them to be rebuilt.
	$(Q_)if ! $(MAKE) -q -C $(OBJ_BOA_DIR)     DESTDIR=$(B_BOA_STAGED_INSTALL_DIR) LIBDIR=/lib      >/dev/null 2>&1; then \
		echo "[Build... (Boa) ... COMPILER ${COMPILER} ... B_BOA_STAGED_INSTALL_DIR $(B_BOA_STAGED_INSTALL_DIR) ] "; \
		# The boa makefile seems to have a problem when the "install" target is run with  \
		# parallel execution (e.g., "-j4"), so first, just do a plain make (without "install") allowing parallel execution... \
		CC=${COMPILER} $(MAKE) -C $(OBJ_BOA_DIR)/$(BOA_SRC_VER)    DESTDIR=$(B_BOA_STAGED_INSTALL_DIR) LIBDIR=/lib   $(B_BOA_COMPILE_REDIR) ; \
		#  Now do the boa "install", but disable parallel execution by specifying "-j1".   \
		echo "[Staged Install... (Boa)]" ; \
		CC=${COMPILER} $(MAKE) -j1 -C $(OBJ_BOA_DIR)/$(BOA_SRC_VER)   DESTDIR=$(B_BOA_STAGED_INSTALL_DIR) LIBDIR=/lib  install  $(B_BOA_COMPILE_REDIR) ; \
	fi
	$(Q_)if [ ! -d $(DESTDIR) ]; then mkdir -p $(DESTDIR); fi;

    #  Copy the libraries from the staged install dir to the destination dir (probably NEXUS_BIN_DIR).
    ifneq ($(DESTDIR),)
		$(Q_)echo "[Install... (Boa)]"
		$(Q_)$(MKDIR) -p $(DESTDIR)
		$(Q_)$(CP) -dpf $(OBJ_BOA_DIR)/$(BOA_SRC_VER)/src/boa  $(DESTDIR)/boa_server
ifeq ($(ANDROID_BUILD),y)
		$(Q_)echo "[Configuring boa.conf for ANDROID]"
		$(Q_)$(AWK) -f $(BOA_SRC_DIR)/boa.conf.awk $(BOA_SRC_DIR)/boa.conf  > $(DESTDIR)/boa.conf
else # end of ANDROID_BUILD=y
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/boa.conf  $(DESTDIR)
endif # end of ANDROID_BUILD=n
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/boa  $(DESTDIR)
		$(Q_)chmod a+x  $(DESTDIR)/boa
		$(Q_)echo "[Install... (web files)]"
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/index.html $(DESTDIR)
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/index.js $(DESTDIR)
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/favicon.ico $(DESTDIR)
		$(Q_)$(CP) -dpf $(BOA_SRC_DIR)/brcm-logo-white.jpg $(DESTDIR)
    endif

${DESTDIR}:
	@mkdir -p $@

${DESTDIR}/index.cgi: $(BOA_SRC_DIR)/index.c
	@${COMPILER} -o ${DESTDIR}/index.cgi index.c

clean:
	$(Q_)if [ -e $(OBJ_BOA_DIR) ]; then (cd $(OBJ_BOA_DIR)/ && cd .. && pwd && rm -rf $(B_REFSW_ARCH).$(DEBUG_SUFFIX) ); fi;
	$(Q_)if [ -e $(DESTDIR)/boa_server  ]; then rm -f $(DESTDIR)/boa_server ; fi;
	$(Q_)if [ -e $(DESTDIR)/boa.conf  ]; then rm -f $(DESTDIR)/boa.conf ; fi;
	$(Q_)if [ -e $(DESTDIR)/boa ]; then rm -f $(DESTDIR)/boa ; fi;
	$(Q_)if [ -e $(DESTDIR)/index.html ]; then rm -f $(DESTDIR)/index.html ; fi;
	$(Q_)if [ -e $(DESTDIR)/index.js ]; then rm -f $(DESTDIR)/index.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/index.cgi ]; then rm -f $(DESTDIR)/index.cgi ; fi;
	$(Q_)if [ -e $(DESTDIR)/favicon.ico ]; then rm -f $(DESTDIR)/favicon.ico ; fi;
	$(Q_)if [ -e $(DESTDIR)/brcm-logo-white.jpg ]; then rm -f $(DESTDIR)/brcm-logo-white.jpg ; fi;
	$(Q_)if [ -e $(DESTDIR)/boa_error.log ]; then rm -f $(DESTDIR)/boa_error.log ; fi;
	$(Q_)if [ -e $(DESTDIR)/boa_access.log ]; then rm -f $(DESTDIR)/boa_access.log ; fi;
