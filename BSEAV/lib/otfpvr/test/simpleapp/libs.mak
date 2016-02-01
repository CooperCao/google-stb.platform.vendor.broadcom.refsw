# This file defines all the library includes (and sources)
# that might be used by any build for this board.

INC :=

include $(MAGNUM)/basemodules/chp/bchp.inc
include $(MAGNUM)/basemodules/reg/breg.inc
include $(MAGNUM)/basemodules/int/bint.inc
include $(MAGNUM)/basemodules/mem/bmem.inc
include $(MAGNUM)/basemodules/tmr/btmr.inc

#include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/err/berr.inc
#include $(MAGNUM)/basemodules/kni/bkni.inc

include $(MAGNUM)/portinginterface/xvd/bxvd.inc
include $(MAGNUM)/portinginterface/i2c/bi2c.inc
include $(MAGNUM)/portinginterface/xpt/bxpt.inc
CUST=G
ifeq ($(BCHP_CHIP), 7401)
include $(MAGNUM)/portinginterface/hsm/bhsm.inc
endif
include $(MAGNUM)/portinginterface/vdc/bvdc.inc
include $(MAGNUM)/portinginterface/hdm/bhdm.inc
include $(MAGNUM)/portinginterface/grc/bgrc.inc
include $(MAGNUM)/portinginterface/kpd/bkpd.inc
include $(MAGNUM)/portinginterface/led/bled.inc
include $(MAGNUM)/portinginterface/kir/bkir.inc
include $(MAGNUM)/portinginterface/rap/brap.inc
#include $(MAGNUM)/portinginterface/vbi/bvbi.inc
#include $(MAGNUM)/portinginterface/urt/burt.inc
include $(MAGNUM)/portinginterface/spi/bspi.inc
include $(MAGNUM)/portinginterface/irb/birb.inc
include $(MAGNUM)/portinginterface/icp/bicp.inc

include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/commonutils/mth/bmth.inc
include $(MAGNUM)/commonutils/avc/bavc.inc
include $(MAGNUM)/commonutils/sur/bsur.inc
include $(MAGNUM)/commonutils/fmt/bfmt.inc
include $(MAGNUM)/commonutils/pxl/bpxl.inc
include $(MAGNUM)/commonutils/rdc/brdc.inc

#include $(MAGNUM)/syslib/vbilib/bvbilib.inc
include $(MAGNUM)/syslib/pcrlib/bpcrlib.inc
#include $(MAGNUM)/syslib/pvrlib/bpvrlib.inc

include $(ROCKFORD)/modules/transition.inc


INC+= \
	basemodules/dbg \
	commonutils/lst \
	basemodules/std \
	basemodules/err 

INC +=  commonutils/img

OBJ+= \
	basemodules/dbg/bdbg

INC+= \
	basemodules/kni/vxworks\
	basemodules/std/types/vxworks 

OBJ+= \
	basemodules/kni/vxworks/bkni \
	basemodules/dbg/vxworks/bdbg_output \
	basemodules/dbg/vxworks/bdbg_os_priv

INC += syslib/pvrlib

OBJ += \
	syslib/pvrlib/bpvrlib_play \
	syslib/pvrlib/bpvrlib_p_ringmgr

# include $(ROCKFORD)/modules/pwm.inc
# gio - missing from rockford/modules
INC += portinginterface/gio/${BCHP_CHIP}

OBJ += \
	portinginterface/gio/${BCHP_CHIP}/bgio \
	portinginterface/gio/${BCHP_CHIP}/bgio_pin \
	portinginterface/gio/${BCHP_CHIP}/bgio_pin_priv \
	portinginterface/gio/${BCHP_CHIP}/bgio_priv \

INC += commonutils/mth/

INC += syslib/pvrlib

# others not convered by rockford modules
INC += basemodules/std/config

# Bits of Rockford required
R_OBJ = bsp/bcm$(BCM_BOARD)/common/bcm4500ap.c \
	bsp/bcm$(BCM_BOARD)/common/bcmtm.c

# BSP
R_INC =	\
	bsp/bcm$(BCM_BOARD)/common \


# FPGA
R_OBJ += commondrivers/fpga/$(BCM_BOARD)/bfpga \
	 commondrivers/fpga/$(BCM_BOARD)/bfpga_name

R_INC += commondrivers/fpga/$(BCM_BOARD)

OBJ += \
	$(PATCH_DIR)/vxworks_isr

include $(BOTF_DIR)/botf.inc

SRCDIRS += $(BOARDDIR)/bsp
SRCDIRS += $(patsubst %, $(MAGNUM)/%, $(sort $(dir $(OBJ))))
SRCDIRS += $(patsubst %, $(ROCKFORD)/%, $(sort $(dir $(R_OBJ))))
# patches
SRCDIRS += $(P_INC)
INCDIRS += $(P_INC)

#otf
SRCDIRS += $(BOTF_INC)
INCDIRS += $(BOTF_INC)

INCDIR += $(patsubst %,-I$(MAGNUM)/%, $(INC))
INCDIR += $(patsubst %,-I$(ROCKFORD)/%, $(R_INC))
INCDIR += $(patsubst %,-I%, $(INCDIRS))


