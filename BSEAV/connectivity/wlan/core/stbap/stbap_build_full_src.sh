#!/bin/bash

## A wrapper script to build the set-top box access point (stbap) applications and drivers.

set -x

echo "######################################################################"
echo "## Set environment variables"
echo "######################################################################"

# do this first ===> source ./stbap_build_arml_${LINUXVER}.inc
source ./stbap_config_full_src${BIT}${stbsoc}.inc

echo "######################################################################"
echo "## Copy AP Apps, Driver and FW packages."
echo "######################################################################"

if [[ "${stbsoc}" != *7271* ]]; then
cd linux-stbap-${STBAP_VER}
fi
STBAP_BUILDROOT=`pwd`

echo "######################################################################"
echo "## Build access point applications"
echo "######################################################################"

cd ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/components/opensource/jsonc/
chmod 777 autogen.sh
./autogen.sh

cd ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src
make -C include
make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=${ARCH} STBLINUX=1 -C router clean
cp router/config/defconfig-stbap-dhd router/.config
yes "" | make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=${ARCH} STBLINUX=1 -C router oldconfig
make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=${ARCH} STBLINUX=1 -C router apps apps_install

echo "######################################################################"
echo "## Build access point drivers (emf,igs and dpsta)"
echo "######################################################################"
export CONFIG_STBAP=y
export EMF_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers
export DPSTA_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/Module.symvers
if [[ "${stbsoc}" != *7271* ]]; then
export KBUILD_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/Module.symvers
else
export KBUILD_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wlplat/Module.symvers
fi

make -C router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

make -C router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

make -C router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

cat ${EMF_EXTRA_SYMBOLS} ${DPSTA_EXTRA_SYMBOLS} > ${KBUILD_EXTRA_SYMBOLS}

echo "######################################################################"
echo "## Build full-dongle driver and applications"
echo "######################################################################"

if [[ -z "${stbsoc}" || "${stbsoc}" == "7271p" ]]; then
cd ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}

# WAR to fix the DHD driver undefinbed references to macdbg functions.Need to fix this in DHD TWIG.
sed -i '558i\CFILES += dhd_macdbg.c' src/dhd/linux/Makefile

make -C src/dhd/linux STBLINUX=1 SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 clean
make -C src/dhd/linux STBLINUX=1 SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 ${STBAP_WIFI_FD_DRIVER}
make -C src/dhd/linux STBLINUX=1 SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 ${STBAP_WIFI_FD_SECDMA_DRIVER}

make -C src/dhd/exe CC=${ARCHTOOL}-gcc V=1 clean
make -C src/dhd/exe CC=${ARCHTOOL}-gcc V=1

make -C src/wl/exe CC=${ARCHTOOL}-gcc V=1 clean
make -C src/wl/exe CC=${ARCHTOOL}-gcc V=1
fi

echo "######################################################################"
echo "## Build NIC driver and applications "
echo "######################################################################"

cd ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}
make -C src/include
if [[ "${stbsoc}" == *7271* ]]; then
make -C src/wlplat STBLINUX=1 SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 clean
fi
make -C src/wl/linux STBLINUX=1 SHOWWLCONF=1 DPSTA=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 clean
make -C src/wl/linux STBLINUX=1 SHOWWLCONF=1 DPSTA=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 ${STBAP_WIFI_NIC_DRIVER}
if [[ "${stbsoc}" != *7271* ]]; then
make -C src/wl/linux STBLINUX=1 SHOWWLCONF=1 DPSTA=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 ${STBAP_WIFI_NIC_SECDMA_DRIVER}
fi

echo "######################################################################"
echo "## Copy NIC drivers to STBAP_APPS_DIR/src/wl/linux "
echo "######################################################################"

if [[ "${stbsoc}" != *7271* ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/linux/obj-${STBAP_WIFI_NIC_SECDMA_DRIVER}-${LINUXVER}/wl.ko ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/wl_secdma.ko
fi
cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/linux/obj-${STBAP_WIFI_NIC_DRIVER}-${LINUXVER}/wl.ko ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/
if [[ "${stbsoc}" == *7271* ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/linux/obj-${STBAP_WIFI_NIC_DRIVER}-${LINUXVER}/wlplat.ko ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/
cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/components/nvram/bcm97271wlan.txt ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/nvram.txt
fi

make -C src/wl/exe  CC=${ARCHTOOL}-gcc V=1 clean
make -C src/wl/exe  CC=${ARCHTOOL}-gcc V=1

echo "######################################################################"
echo "## Build firmware and copy to DHD sources"
echo "######################################################################"

if [[ -z "${stbsoc}" || "${stbsoc}" == "7271p" ]]; then
cd ${STBAP_BUILDROOT}/${STBAP_FW_DIR}
make -C main/src/router obj-pciefd=43602a1
cd ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/43602a1-roml
cp ${STBAP_BUILDROOT}/${STBAP_FW_DIR}/43602/src/dongle/rte/wl/builds/43602a1-roml/${STBAP_FW_43602A1_NAME}/rtecdc.bin bcm43602a1-firmware.bin
strings bcm43602a1-firmware.bin | grep Version >> ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/firmware_ver.txt

cd ${STBAP_BUILDROOT}/${STBAP_FW_DIR}
make -C main/src/router obj-pciefd=43602a3
cd ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/43602a3-roml
cp ${STBAP_BUILDROOT}/${STBAP_FW_DIR}/43602/src/dongle/rte/wl/builds/43602a3-roml/${STBAP_FW_43602A3_NAME}/rtecdc.bin bcm43602a3-firmware.bin
strings bcm43602a3-firmware.bin | grep Version >> ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/firmware_ver.txt

cd ${STBAP_BUILDROOT}/${STBAP_FW_DIR}
STB_FW_TRAGET_PATH=4365/build/dongle/4366c0-roml/$STBAP_FW_4366C0_NAME
mkdir -p ${STB_FW_TRAGET_PATH}
cp -rpf 4365/build/dongle/4366c0-roml/.prebuilt/* 4365/build/dongle/4366c0-roml/${STBAP_FW_4366C0_NAME}/
make -C 4365/src/dongle/make/wl 4366c0-roml/${STBAP_FW_4366C0_NAME}
cd ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/4366c0-roml
cp ${STBAP_BUILDROOT}/${STBAP_FW_DIR}/4365/build/dongle/4366c0-roml/${STBAP_FW_4366C0_NAME}/rtecdc.bin bcm4366c0-firmware.bin
strings bcm4366c0-firmware.bin | grep Version >> ${STBAP_RELROOT}/${STBAP_FD_DRIVER_DIR}/firmware/firmware_ver.txt
fi

echo "######################################################################"
echo "## Prepare installation tarball"
echo "######################################################################"

cd ${STBAP_BUILDROOT}

if [[ -z "${stbsoc}" ]]; then
STBAP_TARGET_DRIVER_TYPE=fdnic-2ifs-${LINUXVER}
elif [[ "${stbsoc}" == "7271p" ]]; then
STBAP_TARGET_DRIVER_TYPE=stb7271-fdnic-2ifs-${LINUXVER}
else
STBAP_TARGET_DRIVER_TYPE=stb7271-nic-${LINUXVER}
fi
STBAP_TARGET_DIR=${STBAP_BUILDROOT}/target_${STBAP_TARGET_DRIVER_TYPE}
rm -rf ${STBAP_TARGET_DIR}

mkdir -p ${STBAP_TARGET_DIR}/etc
mkdir -p ${STBAP_TARGET_DIR}/root
mkdir -p ${STBAP_TARGET_DIR}/etc/init.d
mkdir -p ${STBAP_TARGET_DIR}/sbin
mkdir -p ${STBAP_TARGET_DIR}/usr/sbin
mkdir -p ${STBAP_TARGET_DIR}/www
mkdir -p ${STBAP_TARGET_DIR}/bin
mkdir -p ${STBAP_TARGET_DIR}/lib/modules/
mkdir -p ${STBAP_TARGET_DIR}/lib/firmware/brcm
mkdir -p ${STBAP_TARGET_DIR}/lib64

cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/shared/nvram/stbap_nvram.txt ${STBAP_TARGET_DIR}/etc/nvrams_ap_default.txt
if [[ "${stbsoc}" != *7271* ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/stbap_init.sh ${STBAP_TARGET_DIR}/sbin/stbap_init.sh
else
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/stbap_init_7271.sh ${STBAP_TARGET_DIR}/sbin/stbap_init.sh
fi
chmod 777 ${STBAP_TARGET_DIR}/sbin/stbap_init.sh
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/rc.user ${STBAP_TARGET_DIR}/root/
chmod 777 ${STBAP_TARGET_DIR}/root/rc.user
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/sbin/rc ${STBAP_TARGET_DIR}/sbin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/usr/sbin/* ${STBAP_TARGET_DIR}/usr/sbin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/bin/* ${STBAP_TARGET_DIR}/bin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/wps/bin/* ${STBAP_TARGET_DIR}/bin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/usr/lib/* ${STBAP_TARGET_DIR}/lib
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/usr/lib/* ${STBAP_TARGET_DIR}/lib64
cp -r ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/${ARCH}-glibc/target/www ${STBAP_TARGET_DIR}/

if [[ "${stbsoc}" != *7271* ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/apps/* ${STBAP_TARGET_DIR}/usr/sbin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/lib/* ${STBAP_TARGET_DIR}/lib
else
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/exe/* ${STBAP_TARGET_DIR}/usr/sbin
fi

cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/dpsta.ko ${STBAP_TARGET_DIR}/lib/modules/

if [[ -z "${stbsoc}" || "${stbsoc}" == "7271p" ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/src/dhd/linux/${STBAP_WIFI_FD_DRIVER}-${LINUXVER}/dhd.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/src/dhd/linux/${STBAP_WIFI_FD_SECDMA_DRIVER}-${LINUXVER}/dhd.ko ${STBAP_TARGET_DIR}/lib/modules/dhd_secdma.ko

## 43602a1 WiFi chip firmware
cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/firmware/43602a1-roml/bcm43602a1-firmware.bin ${STBAP_TARGET_DIR}/lib/firmware/brcm/bcm43602a1-firmware.bin

## 43602a3 WiFi chip firmware
cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/firmware/43602a3-roml/bcm43602a3-firmware.bin ${STBAP_TARGET_DIR}/lib/firmware/brcm/bcm43602a3-firmware.bin

## 4366c0 WiFi chip firmware
cp ${STBAP_BUILDROOT}/${STBAP_FW_DIR}/bcm4366c0-firmware.bin ${STBAP_TARGET_DIR}/lib/firmware/brcm/bcm4366c0-firmware.bin
fi

if [[ "${stbsoc}" != *7271* ]]; then
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/wl.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/wl_secdma.ko ${STBAP_TARGET_DIR}/lib/modules/wl_secdma.ko

cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/src/dhd/exe/dhd ${STBAP_TARGET_DIR}/usr/sbin/dhd
cp ${STBAP_BUILDROOT}/${STBAP_FD_DRIVER_DIR}/src/wl/exe/wl ${STBAP_TARGET_DIR}/usr/sbin/wl
else
## 7271 WiFi NIC driver
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/wlplat.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/wl.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/wl/linux/nvram.txt ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/exe/wl ${STBAP_TARGET_DIR}/usr/sbin/wl
fi

cd ${STBAP_TARGET_DIR}
if [[ "${stbsoc}" != *7271* ]]; then
tar cvzf target_${STBAP_TARGET_DRIVER_TYPE}.tar.gz *
else
tar cvjf target_${STBAP_TARGET_DRIVER_TYPE}.tar.bz2 *
fi

cd ${STBAP_BUILDROOT}
