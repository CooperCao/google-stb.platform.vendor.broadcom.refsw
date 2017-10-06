override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := \
	src/tools/release/module_list.mk \
	src/tools/release/bomrules.mk \
	src/tools/release/linux-router-bom.mk \
	src/tools/release/linux26-router-bom.mk \
	src/tools/release/linux-vistapremium-bom.mk \
	src/tools/release/linux-usbap-bom.mk \
	src/tools/release/linux26-usbap-bom.mk \
	src/tools/release/hnd-drivers-bom.mk \
	src/tools/release/modules-list-bom.mk \
	src/tools/release/linux-postbuild.sh src/tools/release/install_linux.sh \
	src/tools/release/vx-ap.mk \
	src/tools/release/linux26-usbap.mk \
	src/tools/release/linux-usbap.mk \
	src/tools/release/linux-external-usbap.mk \
	src/tools/release/linux-internal-usbap.mk \
	src/tools/release/mkversion.sh src/tools/release/linux-router.mk \
	src/tools/release/release_linux.sh  src/tools/release/swrls.mk \
	src/tools/release/linux-external-router-sdio-std.mk \
	src/tools/release/linux-external-router-combo.mk \
	src/tools/release/linux-external-router-full-src.mk \
	src/tools/release/linux-external-router.mk \
	src/tools/release/linux26-external-router.mk \
	src/tools/release/linux-external-router-partial-src.mk \
	src/tools/release/linux-external-router-partial-src-with-ses.mk \
	src/tools/release/linux-external-slimrtr.mk \
	src/tools/release/linux-external-vista-router-full-src.mk \
	src/tools/release/linux-external-vista-router.mk \
	src/tools/release/linux-external-vista-router-partial-src.mk \
	src/tools/release/linux-external-vista-router-partial-src-with-ses.mk \
	src/tools/release/linux-internal-ap.mk \
	src/tools/release/linux-internal-router.mk \
	src/tools/release/linux26-internal-router.mk \
	src/tools/release/linux26-external-router-combo.mk \
	src/tools/release/linux26-external-router-full-src.mk \
	src/tools/release/linux26-external-router.mk \
	src/tools/release/linux26-external-router-partial-src.mk \
	src/tools/release/linux26-external-router-sdiio-std.mk \
	src/tools/release/linux-internal-vista-router.mk \
	src/tools/release/linux-internal-wet.mk \
	src/tools/release/linux-mfgtest-router.mk \
	src/tools/release/linux-mfgtest-router-noramdisk.mk \
	src/tools/release/linux-perfprof-router.mk \
	src/tools/release/linux-perfprof-tools.mk \
	src/tools/release/vx-external-ap-ccx-full-src.mk \
	src/tools/release/vx-external-ap-ccx.mk \
	src/tools/release/vx-external-ap-full-src.mk \
	src/tools/release/vx-external-ap.mk \
	src/tools/release/vx-external-router.mk \
	src/tools/release/vx-external-router-wlsrc.mk \
	src/tools/release/vx-internal-ap.mk \
	src/tools/release/vx-internal-router.mk \
	src/tools/release/vx-internal-wl.mk \
	src/tools/release/vx-mfgtest-ap.mk \
	src/tools/release/vx-router-bom.mk \
	src/tools/release/vx-router.mk \
	src/doc \
	src/tools/release/unreleased-chiplist.mk

include src/tools/release/components/makerules.mk
