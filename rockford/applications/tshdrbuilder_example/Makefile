BSEAV = ../../../BSEAV
MAGNUM = ../../../magnum
B_REFSW_OS = linuxuser

include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc

LDFLAGS += -lpthread
CFLAGS += $(addprefix -I,$(BSTD_INCLUDES) $(BKNI_INCLUDES) $(BDBG_INCLUDES))
CFLAGS = \
       -I$(MAGNUM)/commonutils/lst \
       -I$(MAGNUM)/basemodules/std \
       -I$(MAGNUM)/basemodules/chp \
       -I$(MAGNUM)/basemodules/err \
       -I$(MAGNUM)/basemodules/reg \
       -I$(MAGNUM)/basemodules/dbg \
       -I$(MAGNUM)/basemodules/kni/$(B_REFSW_OS) \
       -I$(MAGNUM)/basemodules/std \
       -I$(MAGNUM)/basemodules/std/types/$(B_REFSW_OS) \
       -I$(MAGNUM)/basemodules/std/config \
       -DBCHP_CHIP=1000 -DBCHP_VER=BCHP_VER_A0 -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE \
       -I$(BSEAV)/lib/tshdrbuilder -I$(BSEAV)/lib/mpeg2_ts_parse

OBJS = $(BSEAV)/lib/mpeg2_ts_parse/ts_pat.c \
       $(BSEAV)/lib/mpeg2_ts_parse/ts_priv.c \
       $(BSEAV)/lib/mpeg2_ts_parse/ts_pmt.c \
       $(BSEAV)/lib/mpeg2_ts_parse/ts_psi.c \
       $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.c 

MAGNUM_SOURCES = $(BSTD_SOURCES) $(BKNI_SOURCES) $(BDBG_SOURCES)

tshdrbuilder_test: tshdrbuilder_test.c $(OBJS) $(MAGNUM_SOURCES)

clean:
	$(RM) tshdrbuilder_test
	$(RM) tshdrbuilder_test.o
