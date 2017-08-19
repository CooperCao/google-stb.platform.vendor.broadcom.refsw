SRCBASE = ../../..


include $(SRCBASE)/Makerules


SIMOBJS= gdb-arm.o
PROGS=gdb-target-armsim


all: $(PROGS)


gdb-target-armsim: $(SIMOBJS)
	$(CC) -o $@ $^


clean:
	-rm -f *.o *.d
	-rm -f $(PROGS)


%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<
