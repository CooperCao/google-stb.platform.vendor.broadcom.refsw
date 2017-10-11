#
#  makefile for ecp  tests
#
INC=../../include
BCM=$(INC)/bcmcrypto
CC=gcc -I$(INC) -I$(BCM)
CFLAGS=-c -Wall

OBJS	:= ../bcm_bn.o ../bcm_rand.o

all: test_bn clean

test_bn: test_bn.c $(OBJS)
	$(CC) test_bn.c bn_utils.c $(OBJS) -o $@

bcm_bn.o: bcm_bn.c
	$(CC) $(CFLAGS) bcm_bn.c -o $@

bcm_rand.o: bcm_rand.c
	$(CC) $(CFLAGS) bcm_rand.c -o $@

clean:
	rm -rf $(OBJS)
