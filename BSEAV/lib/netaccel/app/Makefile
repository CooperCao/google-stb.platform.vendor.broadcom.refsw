############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
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
############################################################

include ../../../api/build/tools.mak
all: $(TARGETS)
TOP_DIR := $(shell cd ".." && ${PWD})
OS:=linuxuser
SYSTEM:=linux
BSEAV:= $(shell cd "../../.." && ${PWD})
MAGNUM:=${BSEAV}/../magnum
include ${TOP_DIR}/include.mak


#
# Driver Test Targets
#

UDPSTREAMER	= udpstreamer
HTTP_CLIENT			= http_client
HTTP_TEST_CLIENT	= http_test_client
UDP_TEST_CLIENT		= udp_test_client
UDP_TEST_SERVER		= udp_test_server
UDP_CLIENT		= udp_client
UDP_SERVER		= udp_server
HTTP_TEST_SERVER = http_test_server
HTTP_SERVER = http_server
STREAMER	= streamer
NETRECORD	= netrecord
HTTPSTREAMER	= httpstreamer


# SGIL Test Targets
#

BOUNCE		= bounce


#
# include definitions for building libraries and applications for the settop
#

LDFLAGS += -L$(TOP_DIR)/lib/obj -L$(TOP_DIR)/bin/test -L$(BSEAV)/bin

#
# Program objects
#

STREAMER_OBJS := streamer.o
UDPSTREAMER_OBJS		= udpstreamer.o
HTTP_TEST_CLIENT_OBJS		= http_test_client.o
HTTP_CLIENT_OBJS			= http_client.o
UDP_TEST_CLIENT_OBJS		= udp_test_client.o
UDP_TEST_SERVER_OBJS		= udp_test_server.o
UDP_CLIENT_OBJS		= udp_client.o
UDP_SERVER_OBJS		= udp_server.o
HTTP_TEST_SERVER_OBJS = http_test_server.o
HTTP_TEST_SERVER_OBJS = http_test_server.o
HTTP_SERVER_OBJS 	  = http_server.o
HTTP_TEST_SERVER_OBJS = http_test_server.o
NETRECORD_OBJS		= netrecord.o
HTTPSTREAMER_OBJS		= httpstreamer.o


#
# Set up VPATH(s)
#

vpath %.h $(BCM_APP_INC)
vpath %.c $(BCM_APP_SRC)

NETACCELLIB                 = netaccel
BCMINDEXER		= bcmindexer
BCMPLAYER		= bcmplayer
SETTOP			= settop
#DRIVERS is what is copied over to bin dir 
DRIVERS:=httpstreamer udpstreamer http_test_client http_client http_test_server udp_test_client udp_client udp_test_server udp_server http_server
#
# Targets
#

CFLAGS+= -DBDBG_DEBUG_BUILD ${B_REFSW_MAGNUM_INCLUDE_DIRS} ${B_REFSW_CFLAGS} -I../module -DCMDLINE_TEST -I../lib
CFLAGS += -I${TOP_DIR}/include -I${TOP_DIR}/module/bne -I${TOP_DIR}/module/core -I${TOP_DIR}/module/drv -I${BSEAV}/api/include -I${BSEAV}/lib/bfile


TARGETS = \
	$(HTTPSTREAMER)\
	$(HTTP_TEST_CLIENT)\
	$(HTTP_CLIENT)\
	$(HTTP_TEST_SERVER)\
	$(HTTP_SERVER)\
	$(UDP_TEST_CLIENT)\
	$(UDP_CLIENT)\
	$(UDP_TEST_SERVER)\
	$(UDP_SERVER)\
	$(UDPSTREAMER) 
#      $(STREAMER) \

all: $(TARGETS)
	
$(NETRECORD): $(NETRECORD_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic -l$(NETACCELLIB) -lpthread

$(HTTPSTREAMER): $(HTTPSTREAMER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic -l$(NETACCELLIB) -lpthread

$(HTTP_TEST_SERVER): $(HTTP_TEST_SERVER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic

$(HTTP_SERVER): $(HTTP_SERVER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic
	
$(HTTP_TEST_CLIENT): $(HTTP_TEST_CLIENT_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 
	
$(HTTP_CLIENT): $(HTTP_CLIENT_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 

$(UDP_TEST_CLIENT): $(UDP_TEST_CLIENT_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 

$(UDP_TEST_SERVER): $(UDP_TEST_SERVER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 	
	
$(UDP_CLIENT): $(UDP_CLIENT_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 	

$(UDP_SERVER): $(UDP_SERVER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic 

$(UDPSTREAMER): $(UDPSTREAMER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -l$(NETACCELLIB) -lpthread 

$(STREAMER): $(STREAMER_OBJS)
	$(Q_)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Bstatic -l$(NETACCELLIB) \
		-lpthread


clean:
	${Q_}$(RM) $(TARGETS) -f *.o *.d *.map *~ core

%.o: %.c
	@echo "[Compiling... $<]"
	$(Q_)$(CC) $(CFLAGS) -MMD -c $< -o $@

# include individual depend files
-include *.d

