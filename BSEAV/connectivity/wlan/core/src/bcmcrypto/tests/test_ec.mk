#
#  makefile for ecp  tests
#
INC=../../include
BCM=$(INC)/bcmcrypto
CC=gcc -I$(INC) -I$(BCM)
#CC=gcc -g -I$(INC) -I$(BCM)
CFLAGS=-c -Wall

OBJS	:= ../bcm_ec.o ../bcm_bn.o ../bcm_rand.o

all: test_ec clean

test_ec: test_ec.c $(OBJS)
	$(CC) test_ec.c bn_utils.c $(OBJS) -o $@

bcm_ec.o: bcm_ec.c
	$(CC) $(CFLAGS) bcm_ec.c -o $@

bcm_bn.o: bcm_bn.c
	$(CC) $(CFLAGS) bcm_bn.c -o $@

bcm_rand.o: bcm_rand.c
	$(CC) $(CFLAGS) bcm_rand.c -o $@

clean:
	rm -rf $(OBJS)
