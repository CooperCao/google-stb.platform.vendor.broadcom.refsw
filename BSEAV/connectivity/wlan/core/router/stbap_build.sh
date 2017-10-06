#!/bin/bash

## A wrapper script to build the set-top box access point (stbap) applications and drivers.

set -x

echo "######################################################################"
echo "## Set environment variables"
echo "######################################################################"

source ./stbap_build.inc

echo "######################################################################"
echo "## Copy AP Apps, Driver and FW packages."
echo "######################################################################"

export STBAP_BUILDROOT=`pwd`

echo "######################################################################"
echo "## Build access point applications"
echo "######################################################################"

cd ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src
make -C include
make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=arm STBLINUX=1 -C router clean
cp router/config/defconfig-stbap-7271 router/.config
yes "" | make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=arm STBLINUX=1 -C router oldconfig
make CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUX_VERSION} PLT=arm STBLINUX=1 -C router apps apps_install

echo "######################################################################"
echo "## Build access point drivers (emf,igs and dpsta)"
echo "######################################################################"
export EMF_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers
export DPSTA_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/Module.symvers
export KBUILD_EXTRA_SYMBOLS=${STBAP_BUILDROOT}/Module.symvers

make -C router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

make -C router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

make -C router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}- clean
make -C router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${TOOLCHAIN}/bin/${ARCHTOOL}-

cat ${EMF_EXTRA_SYMBOLS} ${DPSTA_EXTRA_SYMBOLS} > ${KBUILD_EXTRA_SYMBOLS}

echo "######################################################################"
echo "## Build NIC driver and applications "
echo "######################################################################"

export STBAP_WIFI_NIC_DRIVER=nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l

cd ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}
make -C src/include
make -C src/wl/linux STBLINUX=1 SHOWWLCONF=1 DPSTA=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 clean
make -C src/wl/linux STBLINUX=1 SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- V=1 ${STBAP_WIFI_NIC_DRIVER}

make -C src/wl/exe  CC=${ARCHTOOL}-gcc V=1 clean
make -C src/wl/exe  CC=${ARCHTOOL}-gcc V=1

echo "######################################################################"
echo "## Prepare installation tarball"
echo "######################################################################"

cd ${STBAP_BUILDROOT}

export STBAP_TARGET_DRIVER_TYPE=stb7271-nic-${LINUXVER}
export STBAP_TARGET_DIR=${STBAP_BUILDROOT}/target_${STBAP_TARGET_DRIVER_TYPE}
rm -rf $STBAP_TARGET_DIR

mkdir -p ${STBAP_TARGET_DIR}/etc
mkdir -p ${STBAP_TARGET_DIR}/etc/init.d
mkdir -p ${STBAP_TARGET_DIR}/sbin
mkdir -p ${STBAP_TARGET_DIR}/usr/sbin
mkdir -p ${STBAP_TARGET_DIR}/www
mkdir -p ${STBAP_TARGET_DIR}/bin
mkdir -p ${STBAP_TARGET_DIR}/lib/modules/

cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/shared/nvram/stbap_nvram.txt ${STBAP_TARGET_DIR}/etc/nvrams_ap_default.txt
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/stbap_init_7271.sh ${STBAP_TARGET_DIR}/sbin/stbap_init.sh
chmod 777 ${STBAP_TARGET_DIR}/sbin/stbap_init.sh
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/stbap_rcS ${STBAP_TARGET_DIR}/etc/init.d/rcS
chmod 755 ${STBAP_TARGET_DIR}/etc/init.d/rcS
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/rc/stbhotplug ${STBAP_TARGET_DIR}/sbin
chmod 777 ${STBAP_TARGET_DIR}/sbin/stbhotplug
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/arm-glibc/target/sbin/rc ${STBAP_TARGET_DIR}/sbin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/arm-glibc/target/usr/sbin/* ${STBAP_TARGET_DIR}/usr/sbin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/arm-glibc/target/bin/* ${STBAP_TARGET_DIR}/bin
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/arm-glibc/target/usr/lib/* ${STBAP_TARGET_DIR}/lib
cp -r ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/arm-glibc/target/www ${STBAP_TARGET_DIR}/

cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko ${STBAP_TARGET_DIR}/lib/modules/
cp ${STBAP_BUILDROOT}/${STBAP_APPS_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/dpsta.ko ${STBAP_TARGET_DIR}/lib/modules/

cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/linux/obj-${STBAP_WIFI_NIC_DRIVER}-${LINUXVER}/wl.ko ${STBAP_TARGET_DIR}/lib/modules/

cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/src/wl/exe/wlarm_le ${STBAP_TARGET_DIR}/usr/sbin/wl

cp ${STBAP_BUILDROOT}/${STBAP_NIC_DRIVER_DIR}/components/nvram/bcm97271wlan.txt ${STBAP_TARGET_DIR}/lib/modules/nvram.txt

cd ${STBAP_TARGET_DIR}
tar cvjf target_${STBAP_TARGET_DRIVER_TYPE}.tar.bz2 *

cd ${STBAP_BUILDROOT}
