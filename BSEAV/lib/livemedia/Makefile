#############################################################################
#    (c)2010-2016 Broadcom Corporation
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

SHELL := /bin/bash

.PHONY: all clean

B_THIS_DIR:=BSEAV/lib/livemedia

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus


BSEAV ?= $(abspath ${NEXUS_TOP}/../BSEAV)
LIVEMEDIA_SRC_DIR ?= $(abspath ${NEXUS_TOP}/../${B_THIS_DIR})

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

#  More Nexus definitions...
include $(NEXUS_TOP)/build/nexus_defs.inc

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(LIVEMEDIA_SRC_DIR)/livemedia.inc
export B_REFSW_CROSS_COMPILE

$(if ${NEXUS_BIN_DIR},,$(error NEXUS_BIN_DIR is not defined.))
$(if ${B_REFSW_OBJ_ROOT},,$(error B_REFSW_OBJ_ROOT is not defined.))

OBJ_LIVEMEDIA_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/livemedia/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

# DESTDIR is where you want the shared object libraries to end up, default to NEXUS_BIN_DIR
DESTDIR ?= ${NEXUS_BIN_DIR}

# LM_LIBDIR is used if you want the libraries to be in a subdirectory ... like /usr/local/lib
export LM_LIBDIR :=
export LM_PREFIX :=

# Create some filters to clean up the build output, and to hide compiler warnings.
ifeq ($(B_REFSW_VERBOSE),)
	B_LIVEMEDIA_EXTRACT_REDIR := >/dev/null  ; test $${PIPESTATUS[0]}
	B_LIVEMEDIA_PATCH_REDIR :=    |  													\
			egrep --line-buffered  "patching file" |   		                            \
			sed -u 																		\
					-e  "s/^patching file \(.*\)$$/[Patch... \1 (Livemedia)]/ "		    \
			; (exit $${PIPESTATUS[0]})
	B_LIVEMEDIA_COMPILE_REDIR := 2>/dev/null  |  								        \
			egrep --line-buffered  "c\+\+.+\.cpp" |   					                \
			sed -u 																        \
					-e  "s/.* \([[:alnum:]]*\.cpp\)$$/[Compile... \1 (Livemedia)]/ "	\
			; (exit $${PIPESTATUS[0]})
	B_LIVEMEDIA_INSTALL_REDIR := 2>/dev/null  |  									    \
			egrep --line-buffered  "^cd " |   		                                    \
			sed -u 																		\
					-e  "s/^cd \(.*\) ;.*$$/[Staged Install... \1 (Livemedia)]/ "		\
			; (exit $${PIPESTATUS[0]})
endif

all:
	$(Q_)if [[ ! -a $(OBJ_LIVEMEDIA_DIR)/live ]]; then \
		mkdir -p $(OBJ_LIVEMEDIA_DIR); \
		echo "[Extract... (Livemedia)]" ; \
		(cd $(OBJ_LIVEMEDIA_DIR) && tar xvzf $(LIVEMEDIA_SRC_DIR)/live.2013.12.05.tar.gz   $(B_LIVEMEDIA_EXTRACT_REDIR) ); \
		(cd $(OBJ_LIVEMEDIA_DIR)/live && cp -f $(LIVEMEDIA_SRC_DIR)/config.mipslinux-with-shared-libraries . ); \
		(cd $(OBJ_LIVEMEDIA_DIR)/live && cp -f $(LIVEMEDIA_SRC_DIR)/config.android-with-shared-libraries . ); \
		echo "[GenMakefiles... (Livemedia)]" ; \
		(cd $(OBJ_LIVEMEDIA_DIR)/live && ./genMakefiles mipslinux-with-shared-libraries );  \
		(cd $(OBJ_LIVEMEDIA_DIR)/live && patch -p1 < $(LIVEMEDIA_SRC_DIR)/live.2013.12.05.patch   $(B_LIVEMEDIA_PATCH_REDIR) ); \
	fi;

    # First do the livemedia make with the -q (question) option to see if there is anything for it to do.
    # This way, we can avoid the livemedia staged install if livemedia is up to date.  This is necessary because
    # the staged install updates the timestamps on the header files and makes everything that includes them to be rebuilt.
	$(Q_)if ! $(MAKE) -q -C $(OBJ_LIVEMEDIA_DIR)/live     DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib      >/dev/null 2>&1; then \
		echo "[Build... (Livemedia)]" &&  \
		# The livemedia makefile seems to have a problem when the "install" target is run with  \
		# parallel execution (e.g., "-j4"), so first, just do a plain make (without "install") allowing parallel execution... \
		$(MAKE) -C $(OBJ_LIVEMEDIA_DIR)/live    DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib   $(B_LIVEMEDIA_COMPILE_REDIR) && \
		#  Now do the livemedia "install", but disable parallel execution by specifying "-j1".   \
		echo "[Staged Install ... (Livemedia)]" && \
		$(MAKE) -j1 -C $(OBJ_LIVEMEDIA_DIR)/live    DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib  install  $(B_LIVEMEDIA_COMPILE_REDIR) ; \
        # For LGPL reasons, we cannot include our livemedia extensions in a static .a library and link with our apps. We have to include them \
        # in a shared object. We chose to include lm.cpp and lm_lgpl.cpp in liblivemedia.so \
        # This next section of code patches the livemedia/Makefile to include the two lm files. \
        # Once the lm*.o files are created, we have to force the re-creation of the liblivemedia.so shared object. \
		$(MAKE) -j1 -C blive_ext/build DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib  $(B_LIVEMEDIA_COMPILE_REDIR) ; \
		if [ ! -e $(OBJ_LIVEMEDIA_DIR)/live/liveMedia/lm_blive.patch ]; then echo [Creating... lm_blive.patch] ; \
				awk --assign B_REFSW_ARCH=$(B_REFSW_ARCH) --assign DEBUG_SUFFIX=$(DEBUG_SUFFIX) -f $(LIVEMEDIA_SRC_DIR)/blive_ext/build/lm_blive.awk $(LIVEMEDIA_SRC_DIR)/blive_ext/build/lm_blive.patch > $(OBJ_LIVEMEDIA_DIR)/live/liveMedia/lm_blive.patch ; \
				if [ -e $(OBJ_LIVEMEDIA_DIR)/live/liveMedia/lm_blive.patch ]; then echo [Patch... livemedia/Makefile with lm_blive.patch] ; \
						(cd $(OBJ_LIVEMEDIA_DIR)/live/liveMedia/ && patch -p0 < lm_blive.patch ) $(B_LIVEMEDIA_PATCH_REDIR) ; \
				fi ; \
		fi ; \
		rm -f $(OBJ_LIVEMEDIA_DIR)/live/liveMedia/libliveMedia.so* $(B_LIVEMEDIA_PATCH_REDIR) ; \
		$(MAKE)     -C $(OBJ_LIVEMEDIA_DIR)/live/    DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib           $(B_LIVEMEDIA_COMPILE_REDIR) ; \
		$(MAKE) -j1 -C $(OBJ_LIVEMEDIA_DIR)/live/    DESTDIR=$(B_LIVEMEDIA_STAGED_INSTALL_DIR) LIBDIR=/lib  install  $(B_LIVEMEDIA_COMPILE_REDIR) ; \
	fi

    #  Copy the libraries from the staged install dir to the destination dir (probably NEXUS_BIN_DIR).
    ifneq ($(DESTDIR),)
		$(Q_)echo "[Install... (Livemedia)]"
		$(Q_)$(MKDIR) -p $(DESTDIR)
		$(Q_)$(CP) -dpf $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/lib/*  $(DESTDIR)
    endif

clean:
	$(Q_)if [ -e $(OBJ_LIVEMEDIA_DIR)/live ]; then (cd $(OBJ_LIVEMEDIA_DIR)/ && cd ../.. && rm -rf livemedia ); fi;
	$(Q_)if [ -e $(DESTDIR) ]; then (cd $(DESTDIR) && rm -f libUsage* liblive*  libBasic*  libgroup* ); fi;
