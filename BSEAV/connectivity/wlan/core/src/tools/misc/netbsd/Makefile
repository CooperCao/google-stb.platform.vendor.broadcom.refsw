# $NetBSD: Id $

# Hack to trick NetBSD (bsd.own.mk) into thinking that we're compiling within the
# NetBSD source tree.  We set _SRC_TOP_ here to get the behavior we want for the "install".
_SRC_TOP_= ${NETBSDSRCDIR}/..

.include <bsd.own.mk>

SRCBASE= ${NETBSDSRCDIR}/..
.PATH: ../

PROG=	$(PROG_TARG)
SRCS=	$(PROG_TARG).c
MAN=	$(PROG_TARG).1
LDADD+= -lkvm
DPADD+= ${LIBKVM}

BINDIR=	/usr/sbin

.include <bsd.prog.mk>
# $NetBSD: Id $
	
