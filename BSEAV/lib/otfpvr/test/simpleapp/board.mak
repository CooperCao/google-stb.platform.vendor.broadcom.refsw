############################################################
#   Make common definitions
############################################################
#
#-----------------------------------------------------------
#       Input variables
#-----------------------------------------------------------
# ROM_CODE      : YES = build reset code
# CACHE_ON      : YES use cache
#
#-----------------------------------------------------------
#       Directives
#-----------------------------------------------------------
# CFLAGS        = Compiler Directives
# AFLAGS        = Assembler Directives
#
#-----------------------------------------------------------
#       Tools Defined
#-----------------------------------------------------------
# RM            = remove file program
# CP            = copy file program
# CC            = cross compiler
# AS            = assembler
# LD            = Linker
# AR            = Archiver
#
#-----------------------------------------------------------
#       Directories
#-----------------------------------------------------------
# DBGR_SRC      = relative location of Debugger Source code
# OBJDIR        = location of object files
# SRCDIR        = location of the source files
# TARGETDIR     = location of the target file output

#-----------------------------------------------------------
#       File Lists
#-----------------------------------------------------------
# INCDIR        = directories to search for headers
# LIBS          = defines linker library
# OBJS          = Objects used in dependencies
#
#-----------------------------------------------------------
#  Directives used by CFLAGS and AFLAGS
#-----------------------------------------------------------
#
# CACHE_FLAG    : defines Stack Address and Cache state
# ENDIANDIR     : defines directory of Endian Library
# ENDIAN_FLAG   : Directive for Endian mode selection
# INCDIR        : directories to search for headers
# NO_CACHE      : do not complile in cache init or run in cache
#                 must also set text in SDRAM_LFLAGS=0xa0000000
# ROM_EXE       : make code that runs in ROM otherwise run in SDRAM
# ROM2M         : Do not compile code to try to fit in Rom
# _STACK_ADDR_  : SDRAM address to use as stack
# _TARGET_DEBUG : enable debugger without file system
# _TEXT_FLAG_   : defines address of text section
#


#
############################################################
# Build configuration flags
############################################################
#
BCM_DEBUG     ?= y

NOISY_BUILD   ?= n

#
############################################################
# Hardware related directories
############################################################
#

# for magnum
ifeq "$(findstring environment,$(origin REVISION))" ""
REVISION         = A0
endif

PLATFORM=$(BCM_BOARD)

BCHP_VER	 = $(REVISION)
BCHP_REV         = $(REVISION)
PLATFORM_PATH	 = $(REVISION)
BSTD_CPU_ENDIAN ?= BSTD_ENDIAN_BIG
BHDMLIB_HDCP_SUPPORT ?= n

#STACK_ADRS      = 0x82000000
#ROM_TEXT_ADRS   = 0x80001000

#
# BCM7401 portinginterface build CFLAGS
#
# These flags were buried in the settop makefile, making them
# applicable only to the PI files. Set them here so all makefiles
# can take advantage of them
#
EXTRA_CFLAGS  = -DBCHP_CHIP=$(BCHP_CHIP)
EXTRA_CFLAGS += -DBSTD_CPU_ENDIAN=$(BSTD_CPU_ENDIAN)
EXTRA_CFLAGS += -DBCHP_VER=BCHP_VER_$(BCHP_VER)
EXTRA_CFLAGS += -DBSCD_DSS_ICAM
###EXTRA_CFLAGS += -DBMEM_REENTRANT_CONFIG=BMEM_NOT_REENTRANT
EXTRA_CFLAGS += -DBMEM_REENTRANT_CONFIG=BMEM_REENTRANT
###EXTRA_CFLAGS += -D__DEBUG_WIN_PROFILE__
###EXTRA_CFLAGS += -D__RESET_ON_RAM_IMAGE_CORRUPT__
EXTRA_FLAGS += -DBRCM_OUTPUT_BANDS_NOT_SUPPORTED
#EXTRA_CFLAGS += -DBHSM_AEGIS_SUPPORT
CFLAGS += $(EXTRA_CFLAGS)

CFLAGS+=-DHARM_UART=2

#############################################################################
# Turn the build configuration flags above into defines for
# our build flags
#
ifeq ($(BCM_DEBUG), y)
DEBUG_BUILD_FLAG = -DBDBG_DEBUG_BUILD=1
DEBUG_BUILD_FLAG += -g
DBGFLAG = dbg
else
DEBUG_BUILD_FLAG = -DBDBG_DEBUG_BUILD=0
DBGFLAG = rel
endif

#############################################################################
# Now add all our configuration flags to the standard build flags
#
CFLAGS += $(DEBUG_BUILD_FLAG)

CFLAGS += $(ENV_CFLAGS)

AFLAGS += $(DEBUG_BUILD_FLAG)

AFLAGS += $(ENV_AFLAGS)



CHIP            = $(BCHP_CHIP)
BCHP_XVD_CHIP   = $(BCHP_CHIP)
BCMCHIP         = $(BCM_BOARD)


OS              = vxworks
TARGET_CPU      = MIPS32
BRCMFLASH       = 1
RCDECODE        = HWDECODE
BCMTUNER        = BCM3416

#JUMP_FROM_ROM means boot from BOOT ROM, copy code & data to RAM, run
#otherwise means there is no boot rom, boot code is integrated in.
#SHMOO_TEST should not be defined, if code size is larger than 4MB. At BOOT
#up time only 4MB can be accessed (0xbfc00000-0xbffffff)
#BRD_DEF     = -DSHMOO_TEST -DJUMP_FROM_ROM


#
##########################################################
# TARGET
##########################################################
#
TARGET      = rom

#
##########################################################
# Determine Cache configuration flags
##########################################################
#
ROM_CODE        = YES
CACHE_ON        = YES
ROM_CODE_FLAG   = -DROM_CODE

TEXT_FLAG       = -Ttext $(ROM_TEXT_ADRS)
ADDR_FLAG       = -DROM_TEXT_ADRS=$(ROM_TEXT_ADRS) -DSTACK_ADRS=$(STACK_ADRS)

#
##########################################################
# Determine ENDIAN Mode
##########################################################
#
ifeq ($(BSTD_CPU_ENDIAN), BSTD_ENDIAN_BIG)
ENDIAN_FLAG     = -EB
MIPSE_FLAG	= -DMIPSEB
ENDIAN          = be
ENDIANNESSFLAG  = eb
else
ENDIAN_FLAG     = -EL
MIPSE_FLAG	= -DMIPSEL
ENDIAN		= le
ENDIANNESSFLAG  = el
endif

#
##########################################################
# Tools
##########################################################
#
CC  = ccmips
LD  = ldmips
AR  = armips
NM  = nmmips

#AFLAGS   += -msoft-float
#CFLAGS   += -msoft-float
FLOAT    += ieee
LFLAGS   += $(ENV_LFLAGS)

#
############################################################
# Objects
############################################################
#
LIBAS   = $(CC) -x
LIBCC   = $(CC)
LIBAR   = $(AR) crs

#
############################################################
# Include libraries
############################################################

#
############################################################
# Include directory
############################################################
# The directories that store must-have header files.
# INCDIR = 
#
############################################################
# Header files
############################################################
#
HDR     =
#
############################################################
# Objects
############################################################
#
OBJS    =
#
#
############################################################
# Make file names
############################################################
#
#MAKEFILE = makefile
#
############################################################
# Compiler Flags
#
# -EB   : Big Endian format output
# -S    : generate Assembler
# -G 32 : use gp references for variables smaller than 32 bytes
# -fverbose-asm :
# -ansi : invoke strict ANSI checking
# -dp   : add comment indicating pattern & alternative
# -dA   : add misc debug info
#
# -membedded-pic : All calls are made using PC relative address
#                  and all data address using the $gp register
#       FAILS: relocation truncated to fit: GPREL
# -mlong-calls : force use of JALR instruction
#
# _C_STRUCT_    : enable conditional compile of C struct in header
#               : Assembler can not handle these
############################################################
#
#ifeq ($(OS),vxworks)
CFLAGS += \
	-Wall                       \
	-DVXWORKS                   \
	$(ENDIAN_FLAG)              \
	$(MIPSE_FLAGS)              \
	-G 0                        \
	-DBCM_BOARD=$(BCM_BOARD)    \
	-D$(RCDECODE)               \
	-DLANGUAGE_C                \
	-DCPU=$(TARGET_CPU)         \
	-mips32                     \
	-msoft-float                \
	-DSOFT_FLOAT                \
	-DTOOL_FAMILY=gnu           \
	-DTOOL=sfgnu                \
	-mno-branch-likely          \
	-fno-builtin                \
	-DR4000                     \
	-D_CC3                      \
	$(INCDIR)                   \
	-D_C_STRUCT_                \
	-DATA_DEBUG		    \
	-DQPSKDSS                   \
	-c
ifeq ($(WIND_BASE),)
$(error WIND_BASE is required)
endif
ifeq ($(VXWORKS_6),1)
CFLAGS += \
	-fdollars-in-identifiers \
	-I$(WIND_BASE)/target/h/wrn/coreip/\
	-I$(WIND_BASE)/target/h\
	-r -nostdlib \
	-DVXWORKS_6

ifneq ($(BCM_DEBUG),y)
CFLAGS += -O2
endif
endif

#
############################################################
# Assembler Flags
############################################################
#
AFLAGS += \
	-g                              \
	-Wall                           \
	-x assembler-with-cpp           \
	-DCPU=$(TARGET_CPU)             \
        -DMIPS_SDE                      \
        -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'  \
	-D_ASMLANGUAGE                  \
	-DR4000                         \
	$(ENDIAN_FLAG)                  \
	-G 0                            \
	-DBCM_BOARD=$(BCM_BOARD)     \
	-DBRCM_FLASH=$(BRCMFLASH)       \
	-DTLBREGISTERS -DENVOFF_BASE=10 \
	$(ROM_CODE_FLAG)                \
	$(CACHE_FLAG)                   \
	$(INCDIR)                       \
	$(ADDR_FLAG)                    \
	-c
#
############################################################
#
# Linker Flags
#
# --architecture : Set architecture
# --oformat TARGET : Specify target of output file
# --cref        : Output cross reference table
# --print-map   : Print map file on standard output
#  -EL                         Link little-endian $(OBJDIR)ects
#  -Tbss ADDRESS               Set address of .bss section
#  -Tdata ADDRESS              Set address of .data section
#  -Ttext ADDRESS              Set address of .text section
#
############################################################
#

LDFLAGS  += -g -Wl,-Map,$(OBJDIR)/$(basename $(@F)).map   \
	$(ENDIAN_FLAG)          \
	-msoft-float

