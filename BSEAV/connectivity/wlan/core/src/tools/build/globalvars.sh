if [ "$OS" == "Windows_NT" ]; then
   winpfx=${WLAN_WINPFX:-Z:}
   projectsdrive="$winpfx"; TEMP="c:/temp"; FMT1="fmt -w 1";
   blatcmd="$winpfx/projects/hnd/tools/win/Blat240/full/blat.exe"
   blatopts="-server mailhost"
   blatopts="${blatopts} -f ${USERNAME}@broadcom.com"
else
   projectsdrive="";   TEMP="/tmp";    FMT1="fmt -1";
fi

build_linux="${projectsdrive}/projects/hnd/swbuild/build_linux"
build_window="${projectsdrive}/projects/hnd/swbuild/build_window"
build_netbsd="${projectsdrive}/projects/hnd/swbuild/build_netbsd"
build_macos="${projectsdrive}/projects/hnd/swbuild/build_macos"
nightly_disks="${projectsdrive}/projects/hnd_swbuild_ext7_scratch \
               ${projectsdrive}/projects/hnd_swbuild_ext9_scratch \
               ${projectsdrive}/projects/hnd_swbuild_ext57 \
               ${projectsdrive}/projects/hnd_swbuild_ext58 \
               ${projectsdrive}/projects/hnd_swbuild_ext59 \
               ${projectsdrive}/projects/hnd_swbuild_ext60"
