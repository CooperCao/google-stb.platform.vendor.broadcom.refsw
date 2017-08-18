::See this twiki for other commands :
::http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/SIGPrivateBuilds#NIC_driver_for_Windows

::Win10 NIC:
make -C src/wl/sys/wdm BUILD_TYPES="free checked" BUILD_ARCHS="x86 x64" BCMCCX=0 WINOS=WIN10 VCTHREADS=ON WIN10_WDK_VER=10240 VS_VER=2015 build_win10_driver

::Win8x NIC:
make -C src/wl/sys/wdm BUILD_TYPES="free checked" BUILD_ARCHS="x86 x64" BCMCCX=0 WINOS=WIN8X VCTHREADS=ON WIN8_WDK_VER=9600 VS_VER=2013 build_win8x_driver

::Win7 NIC:
make -C src/wl/sys/wdm BUILD_TYPES=checked BUILD_ARCHS=x86 BCMCCX=0 build_win7_driver

::XP NIC:
make -C src/wl/sys/wdm BUILD_TYPES=checked BUILD_ARCHS=x86 build_xp_driver

