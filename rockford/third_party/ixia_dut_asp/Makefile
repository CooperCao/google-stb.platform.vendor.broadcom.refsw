#############################################################################

#VPATH = ..
ifeq ($(B_REFSW_ARCH),)
	CC=gcc
else
	CC=$(B_REFSW_ARCH)-gcc
endif

CFLAGS=-I../ -Wall -Wformat -g -std=gnu99

all: ixia_dut_asp install

ixia_dut_asp: mntcpapp.o mntcplnx.o ../../asp/emulation/dut/asp_manager.o ../../asp/emulation/dut/asp_connx_migration.o ../../asp/emulation/cmodel/asp_proxy_server_messages.o
	$(CC) -o $@ mntcpapp.o mntcplnx.o ../../asp/emulation/dut/asp_manager.o ../../asp/emulation/dut/asp_connx_migration.o ../../asp/emulation/cmodel/asp_proxy_server_messages.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

DEPS := $(patsubst %.o,%.d,$(OBJS))

clean:
	rm -rf *.o; rm -f *.d; rm -rf ../*.o; rm -f ixia_dut_asp

install:
	cp -f ixia_dut_asp ../../../obj.$(NEXUS_PLATFORM)/nexus/bin
PHONY: all clean install
