include ../Makefile.inc

PROGS = iftest 

all: ${PROGS} 

test_tc: test_tc.o
	${CC} ${CFLAGS} -g -o $@ test_tc.o ${LIBS}

iftest: iftest.o
	${CC} ${CFLAGS} -g -o $@ iftest.o ${LIBS}

clean:
	rm -f ${PROGS} ${CLEANFILES}
