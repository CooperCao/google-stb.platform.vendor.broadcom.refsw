############################################################
#     (c)2003-2010 Broadcom Corporation
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
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#   This file defines the OS-specific rules for building in vxworks.
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

nexus_install: nexus_all
	@echo [Install... libraries]
	$(Q_)$(MKDIR) ${NEXUS_BIN_DIR}
ifdef NEXUS_SHARED_LIB
	@$(CP) $(NEXUS_SHARED_LIB) ${NEXUS_BIN_DIR}
endif
	@$(CP) $(NEXUS_STATIC_LIB) ${NEXUS_BIN_DIR}
	@$(CP) $(MAGNUM_LIB) ${NEXUS_BIN_DIR}

ifdef NEXUS_SHARED_LIB
nexus_all: $(MAGNUM_LIB) ${EXTRA_ALL} $(NEXUS_STATIC_LIB) $(NEXUS_SHARED_LIB)
else
nexus_all: $(MAGNUM_LIB) ${EXTRA_ALL} $(NEXUS_STATIC_LIB)
endif

# Rule for magnum sources
$(MAGNUM_OBJECTS): $(NEXUS_OBJ_DIR)/MAGNUM/%.$(NEXUS_OBJ_SUFFIX): %.c
	-$(Q_)$(MKDIR) $(NEXUS_OBJ_DIR)/MAGNUM >/dev/null 2>&1
	@echo "[Compile... $(notdir $<) (magnum)]"
	$(Q_)$(CC) $(CDEP_FLAG) $(NEXUS_CFLAGS) -c $< -o $@

## Rule for magnum library
#$(MAGNUM_LIB): $(MAGNUM_OBJECTS)
#	@echo "[Archive... $(notdir $@)]"
#	-$(Q_)$(AR) rcs $@ $(notdir $^)

## Rule for nexus library
#$(NEXUS_STATIC_LIB): $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS)
#	@echo "[Archive... $(notdir $@)]"
#	-$(Q_)$(AR) rcs $@ $(notdir $^)

# We cannot build the static libraries by placing all the objects on a single command line (as in the above) -- command line is too long.
# Instead, create a script with the commands necessary to load all the objects (below).
# If both the nexus and magnum objects were in the same directory then we could have 'chdir' into that directory before using the 'ar' command.
# Note: the (old) vxworks 'ar' app will not update the libray like its supposed to do on a SAVE command.
# Instead it creates a "tmp-xxx.a" and never updates the original.  So we have to delete the original each time.

# Rule for magnum library
$(MAGNUM_LIB): $(MAGNUM_OBJECTS)
#	@echo "[Deleting old... $@]"
	${Q_}$(RM) $@
	@echo "[Archive script... $(notdir $@)]"
	@echo "CREATE $@" > $*.ars
	@echo "ADDMOD $^" >> $*.ars
	@echo "SAVE" >> $*.ars
	@echo "END" >> $*.ars
	@echo "[Archive... $(notdir $@)]"
	${Q_}$(AR) -M < $*.ars
	${Q_}$(RANLIB) $@

# Rule for nexus library
$(NEXUS_STATIC_LIB): $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS)
#	@echo "[Deleting old... $@]"
	${Q_}$(RM) $@
	@echo "[Archive script... $(notdir $@)]"
	@echo "CREATE $@" > $*.ars
	@echo "ADDMOD $^" >> $*.ars
	@echo "SAVE" >> $*.ars
	@echo "END" >> $*.ars
	@echo "[Archive... $(notdir $@)]"
	${Q_}$(AR) -M < $*.ars
	${Q_}$(RANLIB) $@

# We cannot build the nexus shared library by combining all the nexus objects with all the magnum objects -- it creates a command line that is just too long.
# In order to fix this we'll build two libraries and then combine them into a single object.

# Rule for shared nexus library
ifdef NEXUS_SHARED_LIB
$(NEXUS_SHARED_LIB): $(NEXUS_OBJ_DIR)/libmagnum.so $(NEXUS_OBJ_DIR)/libnexus.so
	@echo [Link...... libnexus.out]
	${Q_}$(LD) -r -o $(NEXUS_OBJ_DIR)/libnexus.out $(NEXUS_OBJ_DIR)/libmagnum.so $(NEXUS_OBJ_DIR)/libnexus.so

$(NEXUS_OBJ_DIR)/libmagnum.so: $(MAGNUM_OBJECTS)
	@echo [Link...... libmagnum.so]
# Each file in the target includes the full path that creates a command line that is too big for DOS.
# Change directory to where the objects are and strip off the path (note: the map file will now end up in the object directory).
	${Q_}cd $(dir $(firstword $(MAGNUM_OBJECTS))); \
	$(CC) ${LDFLAGS} -nostdlib -Wl,-Map,libmagnum.map -Wl,--cref -Wl,-X -o $@ -Wl,-soname,$@ $(NEXUS_EXTRA_LDFLAGS) $(notdir $^)

$(NEXUS_OBJ_DIR)/libnexus.so: $(NEXUS_OBJECTS) $(NEXUS_EXTRALIBS)
	@echo [Link...... libnexus.so]
	${Q_}$(CC) ${LDFLAGS} -nostdlib -Wl,-Map,libnexus.map -Wl,--cref -Wl,-X -o $@ -Wl,-soname,$@ $(NEXUS_EXTRA_LDFLAGS) $^
endif

nexus_clean:
	-$(Q_)$(RM) -rf $(NEXUS_TOP)/libmagnum.a $(NEXUS_TOP)/libnexus.* $(NEXUS_OBJ_DIR)/* $(NEXUS_SYNCTHUNK_DIR)/*

# Include dependencies
NEXUS_DEP_FILES := $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
ifneq ($(NEXUS_DEP_FILES),)
-include $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
endif

