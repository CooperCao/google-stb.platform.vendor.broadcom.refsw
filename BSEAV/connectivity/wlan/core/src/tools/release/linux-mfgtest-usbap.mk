include linux-usbap.mk
include linux-mfgtest-router.mk

KERNELCFG := defconfig-bcm947xx-mfgrouter-usbap
ROUTERCFG := defconfig-mfgtest-usbap

CONFIG_GLIBC = false

UNDEFS := $(filter-out _RTE_ BCMROMOFFLOAD,$(UNDEFS))

%.mk: OVFILE=$(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
%.mk:
	cvs co $(if $(TAG),-r $(TAG)) -p src/tools/release/$@ > $@
ifneq ($(OVERRIDE),)
	-[ -f "$(OVFILE)" ] && cp $(OVFILE) $@
endif
