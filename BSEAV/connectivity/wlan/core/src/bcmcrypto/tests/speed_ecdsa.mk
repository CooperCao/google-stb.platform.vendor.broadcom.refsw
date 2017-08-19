#
#  makefile for ecp  tests
#
INC=/home/shalomb/crypto/include
BCM=$(INC)/bcm_ec
CC=gcc -I$(INC) -I$(BCM)
#CC=gcc -g -I$(INC) -I$(BCM)
CFLAGS=-c -Wall

OBJS	:= ../bcm_ecdsa.o ../bcm_ec.o ../bcm_bn.o ../sha2.o ../bcm_rand.o

all: speed_ecdsa clean

speed_ecdsa: speed_ecdsa.c $(OBJS)
	$(CC) speed_ecdsa.c bn_utils.c $(OBJS) -o $@

bcm_ecdsa.o: bcm_ecdsa.c
	$(CC) $(CFLAGS) bcm_ecdsa.c -o $@

bcm_ec.o: bcm_ec.c
	$(CC) $(CFLAGS) bcm_ec.c -o $@

bcm_bn.o: bcm_bn.c
	$(CC) $(CFLAGS) bcm_bn.c -o $@

sha2.o: sha2.c
	$(CC) $(CFLAGS) sha2.c -o $@

bcm_rand.o: bcm_rand.c
	$(CC) $(CFLAGS) bcm_rand.c -o $@

clean:
	rm -rf $(OBJS)
