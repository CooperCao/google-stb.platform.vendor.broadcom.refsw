include ../Makefile.inc

PROGS = wfa_tc wfa_pctg 

all: ${PROGS}

wfa_tc: wfa_tc.o ../lib/libwfa_ca.a 
	${CC} ${CFLAGS} -o $@ wfa_tc.o ${CALIBS}

wfa_pctg: wfa_pctg.o ../lib/libwfa_dut.a
	${CC} ${CFLAGS} -o $@ wfa_pctg.o ${DUTLIBS}

clean:
	rm -f ${PROGS} ${CLEANFILES}
