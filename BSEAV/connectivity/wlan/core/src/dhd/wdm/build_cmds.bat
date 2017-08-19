::See this twiki for more commands.
::http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/SIGPrivateBuilds#Dongle_host_on_WinXP

::Win10 PCIe :
make WINOS=WIN10 BUILD_TYPES="checked free" VS_VER=2015 BUILD_ARCHS="x86 x64" WDK_VER=10240 BUS=pcie -f win8xdriver.mk build_pcie_win10_driver

::Win10 SDIO :
make WINOS=WIN10 BUILD_TYPES="checked free" VS_VER=2015 BUILD_ARCHS="x86 x64" WDK_VER=10240 BUS=sdio -f win8xdriver.mk build_sdio_win10_driver

::Win8x PCIe :
make WINOS=WIN8X BUILD_TYPES="checked free" VS_VER=2013 BUILD_ARCHS="x86 x64 arm" WDK_VER=9600 BUS=pcie  -f win8xdriver.mk build_pcie_win8x_driver

::Win8x SDIO :
make WINOS=WIN8X BUILD_TYPES="checked free" VS_VER=2013 BUILD_ARCHS="x86 x64 arm" WDK_VER=9600 BUS=sdio  -f win8xdriver.mk build_sdio_win8x_driver

::XP SDIO :
make -C src/wl/sys/wdm -f sdio.mk DNGL_IMAGE_NAME=""
::or
make -C src/wl/sys/wdm -f sdio.mk DNGL_IMAGE_NAME=FakeDongle DNGL_IMAGE_PATH=../../include BUILD_ARCHS=x86 :: Fake Image

