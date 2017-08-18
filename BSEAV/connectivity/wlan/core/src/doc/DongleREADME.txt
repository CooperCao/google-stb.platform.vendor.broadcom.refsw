
This distribution includes binary files in the following subdirectories:

dongle_images/	  -- binary images for download to the dongle device
modules/	  -- linux kernel modules: host drivers for the dongle
tools/		  -- tools to generate a .trx from a .bin if necessary
apps/		  -- tools for dongle download and runtime WLAN commands

A brief overview of how to use these files is found in ReleaseNotes.html.

Many of the binaries can be reproduced using the source provided in the 
src directory.  An overview of the src structure is:

src/include	-- various header files
src/shared	-- source files which may be shared by several binaries
src/dongle	-- dongle-specific header
src/usbdev	-- USB-specific host code
src/usbdev/hostdrv -- original host driver module for USB only
src/usbdev/usbdl   -- USB download application code
src/wl/exe	-- code for wl application (WLAN control commands)
src/dhd		-- newer dongle host driver (dhd) module, both USB and SDIO
src/dhd/exe	-- code for dhd application (DHD/SDIO control and download)
src/dhd/linux	-- build directory for dhd module
src/dhd/sys	-- source code for dhd kernel module

To rebuild the dhd kernel module:
   % cd src/dhd/linux
   % make dhd-cdc-sdstd (for SDIO)
   % make dhd-cdc-usb (for USB)

To rebuild the dhd application:
   % cd src/dhd/exe
   % make

To rebuild the USB-only kernel module (usb-cdc):
For a 2.4 linux kernel:
   % cd src/usbdev/hostdrv
   % make
For a 2.6 linux kernel:
   % cd src/usbdev/hostdrv/linux-2.6
   % make

To rebuild the USB download application (bcmdl):
   % cd src/usbdev/usbdl
   % make

To rebuild the wl application:
   % cd src/wl/exe
   % make

------------------------------------------------------------------------

List of files in this distribution:

README.txt
ReleaseNotesDongle.html
dongle_images/
dongle_images/usb-arm-thumb-sb-run-noretail-g-cdc-reclaim-ccx.trx
dongle_images/usb-arm-thumb-sb-run-noretail-ag-cdc-reclaim-ccx.trx
dongle_images/usb-arm-thumb-sb-run-noretail-g-cdc-reclaim-idsup.trx
dongle_images/usb-arm-thumb-sb-run-noretail-ag-cdc-reclaim-idsup.trx
dongle_images/usb-arm-thumb-sb-run-noretail-g-cdc-noreclaim-idsup.trx
dongle_images/usb-arm-thumb-sb-run-mfgtest-g-cdc-noreclaim-noccx.trx
dongle_images/sdio-arm-thumb-sb-run-noretail-g-cdc-reclaim-ccx.bin
dongle_images/sdio-arm-thumb-sb-run-noretail-ag-cdc-reclaim-ccx.bin
dongle_images/sdio-arm-thumb-sb-run-noretail-g-cdc-reclaim-idsup.bin
dongle_images/sdio-arm-thumb-sb-run-noretail-ag-cdc-reclaim-idsup.bin
modules/
modules/usb-cdc-2.4.20-8/
modules/usb-cdc-2.4.20-8/usb-cdc.o
modules/usb-cdc-2.6.11-1.1369_FC4/
modules/usb-cdc-2.6.11-1.1369_FC4/usb-cdc.ko
modules/dhd-cdc-sdstd-2.4.20-8/
modules/dhd-cdc-sdstd-2.4.20-8/dhd.o
modules/dhd-cdc-sdstd-2.6.11-1.1369_FC4/
modules/dhd-cdc-sdstd-2.6.11-1.1369_FC4/dhd.ko
modules/dhd-cdc-usb-2.4.20-8/
modules/dhd-cdc-usb-2.4.20-8/dhd.o
modules/dhd-cdc-usb-2.6.11-1.1369_FC4/
modules/dhd-cdc-usb-2.6.11-1.1369_FC4/dhd.ko
tools/
tools/trx
tools/nvserial
apps/
apps/wl
apps/dhd
apps/bcmdl
src/
src/Makerules
src/Makerules.env
src/branding.inc
src/include/
src/include/bcmcrypto/
src/include/bcmcrypto/aes.h
src/include/bcmcrypto/aeskeywrap.h
src/include/bcmcrypto/rijndael-alg-fst.h
src/include/bcmcdc.h
src/include/bcmdefs.h
src/include/bcmdevs.h
src/include/bcmendian.h
src/include/bcmnvram.h
src/include/bcmsdbus.h
src/include/bcmsdh.h
src/include/bcmsdstd.h
src/include/bcmsrom.h
src/include/bcmutils.h
src/include/rte_ioctl.h
src/shared/bcmwifi/include/bcmwifi_channels.h
src/include/dhdioctl.h
src/include/linux_osl.h
src/include/linux_pkt.h
src/include/linuxver.h
src/include/osl.h
src/include/pcicfg.h
src/include/sbchipc.h
src/include/sbconfig.h
src/include/sbextif.h
src/include/sbhnddma.h
src/include/pci_core.h
src/include/pcie_core.h
src/include/sbpcmcia.h
src/include/sbsdio.h
src/include/sbsdpcmdev.h
src/include/sbsocram.h
src/include/siutils.h
src/include/sdio.h
src/include/sdioh.h
src/include/sdiovar.h
src/include/trxhdr.h
src/include/typedefs.h
src/include/usbrdl.h
src/include/wlioctl.h
src/include/proto/
src/include/proto/802.11.h
src/include/proto/802.11e.h
src/include/proto/802.1d.h
src/include/proto/bcmeth.h
src/include/proto/bcmevent.h
src/include/proto/bcmip.h
src/include/proto/eapol.h
src/include/proto/ethernet.h
src/include/proto/vlan.h
src/include/proto/wpa.h
src/include/epivers.h
src/shared/
src/shared/bcmsrom.c
src/shared/bcmutils.c
src/shared/bcmwifi/src/bcmwifi_channels.c
src/shared/linux_osl.c
src/shared/linux_pkt.c
src/shared/linux_osl_priv.h
src/shared/nvramstubs.c
src/shared/siutils.c
src/shared/nvram/
src/shared/nvram/bcm94320r.txt
src/shared/zlib/
src/shared/zlib/ChangeLog
src/shared/zlib/FAQ
src/shared/zlib/INDEX
src/shared/zlib/Make_vms.com
src/shared/zlib/Makefile
src/shared/zlib/Makefile.in
src/shared/zlib/Makefile.riscos
src/shared/zlib/README
src/shared/zlib/adler32.c
src/shared/zlib/algorithm.txt
src/shared/zlib/compress.c
src/shared/zlib/configure
src/shared/zlib/crc32.c
src/shared/zlib/deflate.c
src/shared/zlib/deflate.h
src/shared/zlib/descrip.mms
src/shared/zlib/example.c
src/shared/zlib/gzio.c
src/shared/zlib/infblock.c
src/shared/zlib/infblock.h
src/shared/zlib/infcodes.c
src/shared/zlib/infcodes.h
src/shared/zlib/inffast.c
src/shared/zlib/inffast.h
src/shared/zlib/inffixed.h
src/shared/zlib/inflate.c
src/shared/zlib/inftrees.c
src/shared/zlib/inftrees.h
src/shared/zlib/infutil.c
src/shared/zlib/infutil.h
src/shared/zlib/maketree.c
src/shared/zlib/minigzip.c
src/shared/zlib/trees.c
src/shared/zlib/trees.h
src/shared/zlib/uncompr.c
src/shared/zlib/zconf.h
src/shared/zlib/zlib.3
src/shared/zlib/zlib.h
src/shared/zlib/zutil.c
src/shared/zlib/zutil.h
src/dongle/
src/dongle/dngl_stats.h
src/usbdev/
src/usbdev/dongle/
src/usbdev/dongle/usb.h
src/usbdev/hostdrv/
src/usbdev/hostdrv/linux-2.6/
src/usbdev/hostdrv/linux-2.6/Makefile
src/usbdev/hostdrv/Makefile
src/usbdev/hostdrv/usb-cdc.c
src/usbdev/usbdl/
src/usbdev/usbdl/Makefile
src/usbdev/usbdl/bcmdl.c
src/usbdev/usbdl/bcmdl.o
src/usbdev/usbdl/adler32.o
src/usbdev/usbdl/inffast.o
src/usbdev/usbdl/inflate.o
src/usbdev/usbdl/infcodes.o
src/usbdev/usbdl/infblock.o
src/usbdev/usbdl/inftrees.o
src/usbdev/usbdl/infutil.o
src/usbdev/usbdl/zutil.o
src/usbdev/usbdl/crc32.o
src/usbdev/usbdl/bcmdl
src/wl/
src/wl/exe/
src/wl/exe/wlu.o
src/wl/exe/GNUmakefile
src/wl/exe/wlu.c
src/wl/exe/wlu.h
src/wl/exe/wlu_cmd.h
src/wl/exe/wlu_linux.c
src/wl/exe/wlu_linux.o
src/wl/exe/wl
src/wl/exe/bcmutils.o
src/wl/exe/bcmwifi_channels.o
src/dhd/
src/dhd/exe/
src/dhd/exe/dhdu.o
src/dhd/exe/GNUmakefile
src/dhd/exe/dhdu.c
src/dhd/exe/dhdu.h
src/dhd/exe/dhdu_cmd.h
src/dhd/exe/dhdu_linux.c
src/dhd/exe/dhdu_linux.o
src/dhd/exe/dhd
src/dhd/exe/bcmutils.o
src/dhd/linux/
src/dhd/linux/Makefile
src/dhd/linux/makefile.26
src/dhd/sys/
src/dhd/sys/dhd.h
src/dhd/sys/dhd_cdc.c
src/dhd/sys/dhd_dbg.h
src/dhd/sys/dhd_linux.c
src/dhd/sys/dhd_sdio.c
src/dhd/sys/dhd_usb_linux.c
src/bcmsdio/
src/bcmsdio/sys/
src/bcmsdio/sys/bcmsdh.c
src/bcmsdio/sys/bcmsdh_linux.c
src/bcmsdio/sys/bcmsdstd.c
src/bcmsdio/sys/bcmsdstd_linux.c
