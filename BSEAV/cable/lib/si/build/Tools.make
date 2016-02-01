#TOOLCHAIN	?= uclibc

MAKE	=	make
CP		=	/bin/cp
RM		=	/bin/rm
CP		=	/bin/cp
RM		=	/bin/rm
CC		=	$(CROSS_COMPILE)gcc
CXX		=	$(CROSS_COMPILE)g++
LD		=	$(CROSS_COMPILE)ld -G 0
AR		=	$(CROSS_COMPILE)ar
AS      =   $(CROSS_COMPILE)as
RANLIB	=	$(CROSS_COMPILE)ranlib

#STD_INC=$(TOOLCHAIN_DIR)/mipsel-linux-uclibc/include/
#GCC_BASE=$(TOOLCHAIN_DIR)/lib/gcc/mipsel-linux-uclibc
#GCC_VER=$(shell (ls $(GCC_BASE)))
#GCC_INC=$(GCC_BASE)/$(GCC_VER)/include
#LINUX_SUBVER_GE_18 ?= $(shell (grep "SUBLEVEL =" ${LINUX}/Makefile | awk '{print ($$3 >= 18)? "y" : "n"}'))

