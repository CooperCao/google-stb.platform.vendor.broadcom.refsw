CFLAGS += -Wall -g -I../include
#CC = gcc
CC = mipsel-linux-gcc
APP = msutil

all: msapp.o msutil.o
	$(CC) -o $(APP) msapp.o msutil.o
	cp $(APP) ../../../../../../../nexus/bin

clean:
	rm *.o $(APP)
