#!/bin/bash
# Install prebuilt binaries from src/ to the release/ directory
# This handles the "release prebuild" step of router builds.

#set -e

LINUX_DIR=${LINUX_DIR:-components/opensource/linux/linux-2.6.36}
if [[ ! -d $LINUX_DIR ]]; then
    echo "$0: Error: $LINUX_DIR: no such directory" >&2
    exit 2
fi

RELEASE=${1:-release}

RTR=components/router
REL=$RELEASE/$RTR

mkdir -pv $REL
cp -pv $RTR/.config $REL/

mkdir -pv $RELEASE/$LINUX_DIR/
cp -pv $LINUX_DIR/.config $RELEASE/$LINUX_DIR/

mkdir -pv $REL/httpd/prebuilt/
cp -pv $RTR/httpd/ezc.o $REL/httpd/prebuilt/
cp -pv $RTR/httpd/cgi_iqos.o $REL/httpd/prebuilt/

mkdir -pv $REL/libbcmcrypto/prebuilt/
cp -pv $RTR/libbcmcrypto/*.o $REL/libbcmcrypto/prebuilt/

mkdir -pv $RELEASE/components/et/cfe/
cp -pv components/cfe/build/broadcom/bcm947xx/et*.o $RELEASE/components/et/cfe/

mkdir -pv $RELEASE/components/et/linux/
cp -pv $LINUX_DIR/drivers/net/et/et.o $RELEASE/components/et/linux/
cp -pv $LINUX_DIR/drivers/net/et/et.ko $RELEASE/components/et/linux/

mkdir -pv $REL/dpsta/linux/
cp -pv $LINUX_DIR/drivers/net/dpsta/dpsta.o $REL/dpsta/linux/

mkdir -pv $REL/compressed/
cp -pv $RTR/compressed/Makefile $REL/compressed/

mkdir -pv $RELEASE/src/shared/linux/
cp -pv $LINUX_DIR/drivers/net/hnd/*.o $RELEASE/src/shared/linux/

# This may be obsolete for 2.6+ routers.
cp -pv src/shared/*.o $RELEASE/src/shared/linux/

for h in src/shared/rtecdc_*.h; do
    [[ -f $h ]] || continue
    mkdir -pv $RELEASE/src/shared/
    cp -pv $h $RELEASE/src/shared/
done

mkdir -pv $REL/emf/
cp -pv $LINUX_DIR/drivers/net/emf/emf.o $REL/emf/
cp -pv $LINUX_DIR/drivers/net/emf/emf.ko $REL/emf/
cp -pv $LINUX_DIR/drivers/net/igs/igs.o $REL/emf/
cp -pv $LINUX_DIR/drivers/net/igs/igs.ko $REL/emf/

mkdir -pv $REL/emf/emfconf/
cp -pv $RTR/emf/emfconf/Makefile $REL/emf/emfconf/

mkdir -pv $REL/emf/emfconf/prebuilt/
cp -pv $RTR/emf/emfconf/emf $REL/emf/emfconf/prebuilt/

mkdir -pv $REL/emf/igsconf/
cp -pv $RTR/emf/igsconf/Makefile $REL/emf/igsconf/

mkdir -pv $REL/emf/igsconf/prebuilt/
cp -pv $RTR/emf/igsconf/igs $REL/emf/igsconf/prebuilt/

mkdir -pv $REL/igmp/
cp -pv $RTR/igmp/Makefile $REL/igmp/

mkdir -pv $REL/igmp/prebuilt/
cp -pv $RTR/igmp/igmp $REL/igmp/prebuilt/

mkdir -pv $REL/igd/prebuilt/
cp -pv $RTR/igd/linux/igd $REL/igd/prebuilt/

mkdir -pv $REL/libupnp/prebuilt/
cp -pv $RTR/libupnp/*.o $REL/libupnp/prebuilt/

mkdir -pv $REL/ctf/linux/
cp -pv $LINUX_DIR/drivers/net/ctf/ctf.o $REL/ctf/linux/
cp -pv $LINUX_DIR/drivers/net/ctf/ctf.ko $REL/ctf/linux/

if [[ -f $LINUX_DIR/drivers/net/proxyarp/proxyarp.o ]]; then
    mkdir -pv $REL/proxyarp/
    cp -pv $LINUX_DIR/drivers/net/proxyarp/proxyarp.o $REL/proxyarp/
    cp -pv $LINUX_DIR/drivers/net/proxyarp/proxyarp.ko $REL/proxyarp/
fi

for lib in $RTR/wps/lib/lib*.a; do
    [[ -f $lib ]] || continue
    mkdir -pv $REL/wps/prebuilt/
    cp -pv $lib $REL/wps/prebuilt/
done

for lib in $RTR/nfc/lib*.a; do
    [[ -f $lib ]] || continue
    mkdir -pv $REL/nfc/
    cp -pv $lib $REL/nfc/
done

for hdr in components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/*.h; do
    [[ -f $hdr ]] || continue
    mkdir -pv $RELEASE/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/
    cp -pv $hdr $RELEASE/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/
done

for obj in $LINUX_DIR/drivers/net/bcm57xx/bcm57xx.{o,ko}; do
    [[ -f $obj ]] || continue
    mkdir -pv $RELEASE/src/bcm57xx/linux/
    cp -pv $obj $RELEASE/src/bcm57xx/linux/
done

for obj in $LINUX_DIR/drivers/usb/gadget/*.{o,ko}; do
    [[ -f $obj ]] || continue
    mkdir -pv $RELEASE/src/usbdev/linux/
    cp -pv $obj $RELEASE/src/usbdev/linux/
done

for obj in $LINUX_DIR/drivers/net/il/*.{o,ko}; do
    mkdir -pv $RELEASE/src/il/linux/
    cp -pv $obj $RELEASE/src/il/linux/
done

for obj in $RTR/voipd/voice $RTR/voipd/voipd $RTR/voipd/*.so; do
    [[ -f $obj ]] || continue
    mkdir -pv $REL/voipd/prebuilt/
    cp -pv $obj $REL/voipd/prebuilt/
done

for obj in $(find src/voip -name pcm_bcm47xx.o -o -name spi_bcm47xx.o); do
    [[ -f $obj ]] || continue
    mkdir -pv $RELEASE/src/voip/xChange/prebuilt/
    cp -pv $obj $RELEASE/src/voip/xChange/prebuilt/
done

if [[ -f $LINUX_DIR/drivers/char/endpoint/endpointdd.o ]]; then
    mkdir -pv $RELEASE/src/voip/xChange/prebuilt/
    cp -pv $LINUX_DIR/drivers/char/endpoint/endpointdd.o $RELEASE/src/voip/xChange/prebuilt/endpointdd_linux.o
fi

if [[ -d $RTR/cramfs ]]; then
    mkdir -pv $REL/cramfs/
    cp -pv $RTR/cramfs/GNUmakefile $REL/cramfs/
    cp -pv $RTR/cramfs/cramfsck.c $REL/cramfs/
    cp -pv $RTR/cramfs/mkcramfs.c $REL/cramfs/
    cp -pv $RTR/cramfs/BRCM_IP_TAG.txt $REL/cramfs/
fi

if [[ -d $RTR/squashfs-4.2 ]]; then
    mkdir -pv $REL/squashfs-4.2/
    cp -pv $RTR/squashfs-4.2/Makefile $REL/squashfs-4.2/
    cp -pv $RTR/squashfs-4.2/*.[ch] $REL/squashfs-4.2/
    cp -pv $RTR/squashfs-4.2/BRCM_IP_TAG.txt $REL/squashfs-4.2/
elif [[ -d $RTR/squashfs ]]; then
    mkdir -pv $REL/squashfs/
    cp -pv $RTR/squashfs/Makefile $REL/squashfs/
    cp -pv $RTR/squashfs/*.[ch] $REL/squashfs/
fi

if [[ -e $RTR/bcmupnp/upnp/linux/upnp ]]; then
    mkdir -pv $REL/bcmupnp/prebuilt/
    -cp $RTR/bcmupnp/upnp/linux/upnp $REL/bcmupnp/prebuilt/
fi

HSPOTAP_BASE=sys/components/apps/hspot/router
HSPOTAP_BIN_REL=../../../$HSPOTAP_BASE/hspot_ap
if [[ -f $RTR/$HSPOTAP_BIN_REL/hspotap ]]; then
    mkdir -pv $REL/$HSPOTAP_BIN_REL/prebuilt/
    cp -pv $RTR/$HSPOTAP_BIN_REL/Makefile $REL/$HSPOTAP_BIN_REL/
    cp -pv $RTR/$HSPOTAP_BIN_REL/hspotap $REL/$HSPOTAP_BIN_REL/prebuilt/
fi

if [[ -d components/apps/visualization/installbin ]]; then
    mkdir -pv $RELEASE/components/apps/visualization/installbin/bin/
    cp -pv components/apps/visualization/installbin/bin/* $RELEASE/components/apps/visualization/installbin/bin/
fi

# Install tools
mkdir -pv $RELEASE/tools/
cp -pv src/tools/misc/addhdr $RELEASE/tools/
cp -pv src/tools/misc/trx $RELEASE/tools/
cp -pv src/tools/misc/swap $RELEASE/tools/
cp -pv src/tools/misc/nvserial $RELEASE/tools/
cp -pv src/tools/misc/lzma $RELEASE/tools/
cp -pv src/tools/misc/lzma_4k $RELEASE/tools/
