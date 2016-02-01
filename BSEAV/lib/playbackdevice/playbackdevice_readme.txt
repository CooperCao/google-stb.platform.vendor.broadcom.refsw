Instructions to Build playbackdevice:
------------------------------------

1. export the following variables as per you platform

export PLATFORM=97425
export BCHP_VER=B1
export TOOLCHAIN_ROOT=/opt/toolchains/stbgcc-4.5.3-1.3
export SC_PLATFORM=bcm97425nexus
export LINUX=/opt/brcm/linux/stblinux-2.6.37-2.8/stblinux-2.6.37
export TOOLCHAIN=/opt/toolchains/stbgcc-4.5.3-1.3
export PATH=$TOOLCHAIN/bin:$PATH

export PLAYBACKDEVICE_STAND_ALONE_APPLICATION=y
CC_PATH=refsw_7425
export NEXUS_MGR_DIR=~/$CC_PATH/BSEAV/lib/playbackdevice
export NEXUS=~/$CC_PATH/nexus
export NETFLIX=~/$CC_PATH/AppLibs/thirdparty/netflix/2.x
export DIRECTFB=~/$CC_PATH/AppLibs/opensource/directfb/build
export STAGECRAFT=~/$CC_PATH/AppLibs/thirdparty/adobe/stagecraft/1.x/public/broadcom
export STAGECRAFT2=~/$CC_PATH/AppLibs/thirdparty/adobe/stagecraft/2.x/internal
export APPLIBS=~/$CC_PATH/AppLibs/common

export APPLIBS_TOOLCHAIN_ARCH=mipsel-linux
export ARCH=mipsel-linux

2.go to <your working directory>/AppLibs/broadcom/playbackdevice
and run make. it should build nexus manger and playbackdevice librraies and place them in the <your working directory>/AppLibs/broadcom/playbackdevice/bin folder.

Instructions to build the Playbackdevice example:
-------------------------------------------------
1. 
make sure export PLAYBACKDEVICE_STAND_ALONE_APPLICATION=y is set.

2. This will build libnexusMng.so, libPlaybackDevice.so, libcurl.so, libssl.so, libcrypto.so, libnexus.so, libb_os.so, bcmdriver.ko and app_playback_device application, and will put it under <your_working_path>/vobs/AppLibs/broadcom/playbackdevice/bin directory. Hence make sure you have the refsw and AppLibs/opensource sources.

3.run build_playbackdevice_app script;

Running the Playbackdevice example:
-----------------------------------

1. boot the target board with linux.
2. mount -o nolock 10.21.81.184:/home/gpantula/refsw_7425/AppLibs/broadcom/playbackdevice/bin  /mnt/nfs
3. cd /mnt/nfs
4. copy ts or pes files to the directory where the binaries are there
5. find out the pids of the streams
6../nexus app_card_playback_device -f <filename.pes> -vpesid 0xe0 -apesid 0xc0 
7. type ./nexus app_card_playback_device to get help












