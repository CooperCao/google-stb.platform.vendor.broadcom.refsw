############################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
############################################################

# Makefile.app is called by nexus/platforms/$(NEXUS_PLATFORM)/build/Makefile to generate the static nexus/bin/include/platform_app.inc
# The static platform_app.inc can be delivered with a binary libnexus.so
# This Makefile assumes that NEXUS_TOP and NEXUS_PLATFORM are defined.
#
# Example usage:
#
# cd nexus/examples
# make clean
#
# # build nexus binary
# make api
#
# # fill nexus/bin/include w/ headers and platform_app.inc
# make nexus_headers
#
# # build using nexus/bin/include
# make NEXUS_PREBUILT_BINARY=y
#

ifndef NEXUS_TOP
$(error NEXUS_TOP is not defined)
endif

# include the dynamic platform_app.inc
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

NEXUS_CFLAGS := $(NEXUS_CFLAGS) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I\$${NEXUS_BIN_DIR}/include
PLATFORM_APP_INC_FILE := ${NEXUS_BIN_DIR}/include/platform_app.inc
CONFIG_H_FILE := ${NEXUS_BIN_DIR}/include/nexus_config.h
NEXUS_HEADER := $(NEXUS_BIN_DIR)/include/nexus.h
NEXUS_CPP_HEADER := $(NEXUS_BIN_DIR)/include/nexus.cpp.h

.PHONY: copy_headers

all: ${PLATFORM_APP_INC_FILE} $(CONFIG_H_FILE) $(NEXUS_HEADER)

# override BCHP_INCLUDES so we don't copy all rdb header files. apps only need bchp.h.
BCHP_INCLUDES = $(MAGNUM)/basemodules/chp

# copy to bin/include
# because of common symlinks, we must do a separate cp per nexus module
# single cp is sufficient for magnum modules
copy_headers:
	${Q_}$(MKDIR) ${NEXUS_BIN_DIR}/include
	${Q_}$(foreach module, $(NEXUS_MODULES), $(CP) $(addsuffix /*.h, $(NEXUS_$(module)_PUBLIC_INCLUDES)) ${NEXUS_BIN_DIR}/include 2>/dev/null;)
	${Q_}$(RM) ${NEXUS_BIN_DIR}/include/nexus_core_init.h
# these bfile & utils header files are needed for file module public API
# TODO: nexus modules needs method for specifying their external header files. NEXUS_<MODULE>_MAGNUM_MODULES is a mix of internal & external
	${Q_}(cd $(NEXUS_TOP)/../BSEAV/lib/bfile; $(CP) bfile_types.h bfile_io.h bfile_buffer.h ${NEXUS_BIN_DIR}/include)
	${Q_}(cd $(NEXUS_TOP)/../BSEAV/lib/utils; $(CP) bioatom.h balloc.h bpool.h ${NEXUS_BIN_DIR}/include)
# bstd is required for nexus api
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/std/types/$(B_REFSW_OS)/bstd_defs.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/std/bstd.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/std/bstd_file.h ${NEXUS_BIN_DIR}/include
# bdbg and bkni is convenient for apps
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/dbg/bdbg.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/dbg/bdbg_app.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/dbg/bdbg_priv.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/kni/generic/bkni.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/kni/generic/bkni_multi.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/err/berr.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/err/berr_ids.h ${NEXUS_BIN_DIR}/include
# overwrite nexus redirects
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/portinginterface/grc/include/bm2mc_packet.h ${NEXUS_BIN_DIR}/include
	${Q_}cd ${NEXUS_BIN_DIR}/include; perl -n -e 'if (/\#include \"(.+\/nexus_platform_features.h)\"/) {system("cp -f ${NEXUS_TOP}/platforms/$$1 .");}' nexus_platform_features.h
	${Q_}cd ${NEXUS_BIN_DIR}/include; perl -n -e 'if (/\#include \"(.+\/nexus_platform_version.h)\"/) {system("cp -f ${NEXUS_TOP}/platforms/$$1 .");}' nexus_platform_version.h
	${Q_}cd ${NEXUS_BIN_DIR}/include; if [ -f nexus_platform_frontend_power_management.h ]; then perl -n -e 'if (/\#include \"(.+\/nexus_platform_frontend_power_management.h)\"/) {system("cp -f ${NEXUS_TOP}/platforms/$$1 .");}' nexus_platform_frontend_power_management.h; fi
# blst is required for bdbg
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/commonutils/lst/blst_*.h ${NEXUS_BIN_DIR}/include
# do not allow the required headers to bleed into internal, chip-specific headers
	${Q_}echo "/* stub file */" >${NEXUS_BIN_DIR}/include/bstd_cfg.h
	${Q_}echo "#include \"bchp_ver_types.h\"" >${NEXUS_BIN_DIR}/include/bchp.h
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/chp/include/common/bchp_ver_types.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/$(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')/bchp_common.h ${NEXUS_BIN_DIR}/include
	${Q_}$(CP) -f $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/$(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')/bchp_cmp_0.h ${NEXUS_BIN_DIR}/include

# NEXUS_TOP may be relative. convert to absolute path.
${PLATFORM_APP_INC_FILE}: copy_headers
	${Q_}${RM} ${PLATFORM_APP_INC_FILE}
	${Q_}echo "#######################################" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "# Nexus platform_app.inc" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "# This file is generated with 'make nexus_headers' and placed into nexus/bin/include" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "#######################################" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "# Location of headers and binaries. Override if they are moved." >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_BIN_DIR ?= ${NEXUS_BIN_DIR}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "# Build environment" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "B_REFSW_OS  = ${B_REFSW_OS}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_PLATFORM  = ${NEXUS_PLATFORM}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_MODE  = ${NEXUS_MODE}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "BCHP_CHIP  = ${BCHP_CHIP}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "BCHP_VER  = ${BCHP_VER}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_CFLAGS = ${NEXUS_CFLAGS}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_LDFLAGS = ${NEXUS_LDFLAGS}" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_LD_LIBRARIES = $(subst $(NEXUS_BIN_DIR),\$${NEXUS_BIN_DIR},${NEXUS_LD_LIBRARIES})" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NEXUS_CLIENT_LD_LIBRARIES = $(subst $(NEXUS_BIN_DIR),\$${NEXUS_BIN_DIR},${NEXUS_CLIENT_LD_LIBRARIES})" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "# Standard toolchain" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(AS),as)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "AS      = ${B_REFSW_CROSS_COMPILE}as" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(LD),ld)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "LD      = ${B_REFSW_CROSS_COMPILE}ld" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(CC),cc)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "CC      = ${B_REFSW_CROSS_COMPILE}gcc" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(CXX),g++)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "CXX     = ${B_REFSW_CROSS_COMPILE}c++" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(AR),ar)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "AR      = ${B_REFSW_CROSS_COMPILE}ar" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "ifeq (\$$(NM),nm)" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "NM      = ${B_REFSW_CROSS_COMPILE}nm" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "endif" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "STRIP   = ${B_REFSW_CROSS_COMPILE}strip" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "OBJCOPY = ${B_REFSW_CROSS_COMPILE}objcopy" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "OBJDUMP = ${B_REFSW_CROSS_COMPILE}objdump" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "RANLIB  = ${B_REFSW_CROSS_COMPILE}ranlib" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "MKDIR   = mkdir -p" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "PERL    = perl" >>${PLATFORM_APP_INC_FILE}
	${Q_}echo "CP      = cp -f" >>${PLATFORM_APP_INC_FILE}
	${Q_}grep -v "#" platform_version.inc | grep "NEXUS_PLATFORM_VERSION" >>${PLATFORM_APP_INC_FILE}

$(CONFIG_H_FILE): ${PLATFORM_APP_INC_FILE}
	${Q_}$(PERL) $(NEXUS_TOP)/build/generate_nexus_config.pl ${PLATFORM_APP_INC_FILE} >${CONFIG_H_FILE}

CPP=$(B_REFSW_CROSS_COMPILE)cpp
$(NEXUS_HEADER) $(NEXUS_CPP_HEADER): copy_headers
	${Q_}$(RM) $(NEXUS_CPP_HEADER)
	${Q_}cd $(NEXUS_BIN_DIR)/include;\
	    echo "#ifndef NEXUS_H__" >$(NEXUS_HEADER); \
	    echo "#define NEXUS_H__" >>$(NEXUS_HEADER); \
	    echo "#include \"bstd.h\"" >>$(NEXUS_HEADER); \
	    echo "#include \"nexus_platform_features.h\"" >>$(NEXUS_HEADER); \
	    LC_ALL=C ls *.h|grep -v "nexus_config\.h"|grep -v "nexus\.h"|grep -v "bstd\.h"|awk '{print "#include \"" $$1 "\""}' >>$(NEXUS_HEADER); \
	    echo "#endif" >>$(NEXUS_HEADER)
	${Q_}$(CPP) -P $(NEXUS_HEADER) $(NEXUS_CFLAGS) $(NEXUS_API_CPP_CFLAGS) >$(NEXUS_CPP_HEADER)
