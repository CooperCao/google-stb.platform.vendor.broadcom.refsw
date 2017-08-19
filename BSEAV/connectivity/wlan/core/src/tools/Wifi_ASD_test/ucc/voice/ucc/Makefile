include ../Makefile.inc

PROGS = wfa_ucc

all:${PROGS}

wfa_ucc : wfa_ucc.o 
	${CC} ${CFLAGS} -o $@ wfa_ucc.o

clean:
	rm -f ${PROGS} ${CLEANFILES}
