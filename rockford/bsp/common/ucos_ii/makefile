################################################################################
#
#       Copyright (c) 2005-2011, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
# 
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
################################################################################
#
################################################################################
#       Makefile
#
#   please use the following style to build the UCOS library.
# 
#    	make
#
################################################################################
#

#
################################################################################
# Endian
################################################################################
#
ifeq ($(BE), 1)
	BUILD_BIG_ENDIAN = 1
	ENDIAN_SUFFIX = be
else
	BUILD_BIG_ENDIAN = 0
	ENDIAN_SUFFIX = le
endif

#
################################################################################
# TARGET
################################################################################
#
TARGET = bcm$(BOARD_NUM)_diags_${ENDIAN_SUFFIX}

#
#-----------------------------------------------------------
#       Directives
#-----------------------------------------------------------
# CFLAGS        = Compiler Directives
# AFLAGS        = Assembler Directives
# LFLAGS        = Linker Directives
#
#-----------------------------------------------------------
#       Tools Defined
#-----------------------------------------------------------
# RM            = remove file program
# CP            = copy file program
# CC            = cross compiler
# AS            = assembler
# LD            = Linker
#

#
################################################################################
# Determine Configuration flags
################################################################################
#

STACK_ADRS      = 0x84000000
ROM_TEXT_ADRS   = 0x80010000
TEXT_FLAG = -Ttext $(ROM_TEXT_ADRS)
ADDR_FLAG = -DROM_TEXT_ADRS=$(ROM_TEXT_ADRS) -DSTACK_ADRS=$(STACK_ADRS)

#
################################################################################
# Determine ENDIAN Mode
################################################################################
#
ifeq ($(BUILD_BIG_ENDIAN), 1)
	ENDIAN_FLAG = -EB
else
	ENDIAN_FLAG = -EL
endif

LIBS += $(UCOS_LIB_DIR)/$(UCOS_LIB)

#
################################################################################
# Tools
################################################################################
#
RM=rm
CP=cp

#
# Algorithmics Tools
#
CC = sde-gcc
AS = sde-gcc
LD = sde-ld
AFLAGS= -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'
CFLAGS= -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'

ifneq ($(OS), Windows_NT)
	ifeq ($(BUILD_BIG_ENDIAN), 1)
		LIBPATHS = /opt/toolchains/sde_5.03/sde/lib/mips32/sof
	else
		LIBPATHS = /opt/toolchains/sde_5.03/sde/lib/el/mips32/sof
	endif
else
	ifeq ($(BUILD_BIG_ENDIAN), 1)
		LIBPATHS = $(LIBRARY_PATH)/mips32/sof
	else
		LIBPATHS = $(LIBRARY_PATH)/el/mips32/sof
	endif
endif

CFLAGS+= \
    $(ENDIAN_FLAG) \
	-g -G 0 -O2	\
    -DMIPS_SDE \
    -DMIPS_SDE5 \
   	-std=c89 -Wall -pedantic -W -Wstrict-prototypes -Wno-unused-value \
    -c

AFLAGS+= \
	$(ENDIAN_FLAG)			\
	-G 0 -O				\
	-DMIPS_SDE				\
	-DMIPS_SDE5				\
	-x assembler-with-cpp	\
	-D_ASMLANGUAGE			\
	$(ADDR_FLAG)
	
ROCKFORD_DIR  = ../../..

DIAGS_VPATH += \
	$(ROCKFORD_DIR)/bsp/common/ucos_ii/include \
	$(ROCKFORD_DIR)/bsp/common/ucos_ii/src

# don't delete, for debug   
#        $(warning  .... CFLAGS= $(CFLAGS))

VPATH = $(DIAGS_VPATH)
OBJDIR = $(ENDIAN_SUFFIX).obj

#
################################################################################
# Make Target Library
################################################################################
#

all : makefile $(OBJDIR) ucoslib
   
$(OBJDIR):
	mkdir $(OBJDIR)

AR = sde-ar
RANLIB = sde-ranlib

#
################################################################################
# Make UCOS Library
################################################################################
#

UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/sde
UCOS_LIB = libucos_$(ENDIAN_SUFFIX).a
UCOS_SRC = ucos_ii.c
UCOS_OBJS = $(UCOS_SRC:.c=.o)

ucoslib:  $(OBJDIR) $(UCOS_OBJS) 
	$(AR) cr $(UCOS_LIB_DIR)/$(UCOS_LIB) $(OBJDIR)/$(UCOS_OBJS)
	$(RANLIB) $(UCOS_LIB_DIR)/$(UCOS_LIB)

#
################################################################################
#  Rules for making
################################################################################
#

vpath %.h $(DIAGS_VPATH)
vpath %.c $(DIAGS_VPATH)
vpath %.o $(OBJDIR)

DIAGS_INCLUDE=$(patsubst %,-I%,$(subst :, ,$(DIAGS_VPATH)))

CFLAGS += $(DIAGS_INCLUDE)
AFLAGS += $(DIAGS_INCLUDE)

%.o: %.c
	@echo '$(CC) $(notdir $<)'
	@$(CC) $(CFLAGS) -MMD -c $< -o $(OBJDIR)/$@
	@mv $(@:.o=.d) $(OBJDIR)

%.o: %.cpp
	@echo '$(CC) $(notdir $<)'
	@$(CC) $(CFLAGS) -MMD -c $< -o $(OBJDIR)/$@
	@mv $(@:.o=.d) $(OBJDIR)
	
%.o: %.s
	@echo '$(AS) $(notdir $<)'
	@$(AS) $(AFLAGS) -MMD -c $< -o $(OBJDIR)/$@
	@mv $(@:.o=.d) $(OBJDIR)

# include individual depend files
-include $(OBJDIR)/*.d

#
################################################################################
#  Rules for clean 
################################################################################
#

clean :
	$(RM) -rf $(OBJDIR)
#	rmdir $(OBJDIR)

