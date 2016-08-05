#!/bin/bash

# this script builds all of the libnexus_security.a
# source make_security_libraries.sh

echo ${PWD}


export   AVKEYLADDER_SUPPORT=n
export   SECURITY_MSIPTV_SUPPORT=n
export   BHSM_VERBOSE_BSP_TRACE=n
export   GENROOTKEY_SUPPORT=n
export   SECUREPKL_SUPPORT=n
export   KEYLADDER_SUPPORT=y
export   OTPMSP_SUPPORT=y
export   OTPID_SUPPORT=y
export   AEGIS_USERCMD_SUPPORT=y
export   SECUREACCESS_SUPPORT=n
export   SECURERSA_SUPPORT=n
export   BSECKCMD_SUPPORT=y
export   SECURERAWCMD_SUPPORT=y
export   IPLICENSING_SUPPORT=n
export   NEXUS_COMMON_CRYPTO_SUPPORT=n
export   NEXUS_SECURITY_IRDETO_SUPPORT=n
export   NEXUS_SECURITY_ALPINE_SUPPORT=n
export   NEXUS_SECURITY_VISTA_CWC_SUPPORT=n
export   NEXUS_SERVER_SUPPORT=n

unset ZEUS_DIR_SUPPLEMENTARY_TAG

function build_platform {
    unset NEXUS_MODE

    source ../../../../BSEAV/tools/build/plat $1 $2 linuxkernel $4

    echo NEXUS_PLATFORM $NEXUS_PLATFORM
    echo BUILD $3 ... $1 $2 linuxkernel debug $4
    echo
    make clean
    rm -rf ../../../../obj.$1
    export B_REFSW_DEBUG=y
    make sec_install -j 3

    echo
    echo BUILD $3 ... $1 $2 linuxkernel release $4
    echo
    make clean
    rm -rf ../../../../obj.$1
    export B_REFSW_DEBUG=n
    make sec_install -j 3

    source ../../../../BSEAV/tools/build/plat $1 $2 linuxuser $4

    echo
    echo BUILD $3 ... $1 $2 linuxuser release $4
    echo
    make clean
    rm -rf ../../../../obj.$1
    export B_REFSW_DEBUG=n
    make sec_install -j 3

    echo
    echo BUILD $3 ... $1 $2 linuxuser debug $4
    echo
    make clean
    rm -rf ../../../../obj.$1
    export B_REFSW_DEBUG=y
    make sec_install -j 3

}

# **** LIBRARIES ARE NO LONGER RELEASED BY DEFAULT FOR ZEUS PLATFORMS ****
# Zeus 10
# build_platform 97358 A1 Zeus10

# Zeus 20
# build_platform 97425 B2 Zeus20

# Zeus 30
# build_platform 97435 B0 Zeus30

# Zeus 41
# build_platform 97439 A0 Zeus41

# Zeus 42
build_platform 97445 E0 Zeus42

unset  AVKEYLADDER_SUPPORT
unset  SECURITY_MSIPTV_SUPPORT
unset  BHSM_VERBOSE_BSP_TRACE
unset  GENROOTKEY_SUPPORT
unset  SECUREPKL_SUPPORT
unset  KEYLADDER_SUPPORT
unset  OTPMSP_SUPPORT
unset  OTPID_SUPPORT
unset  AEGIS_USERCMD_SUPPORT
unset  SECUREACCESS_SUPPORT
unset  SECURERSA_SUPPORT
unset  BSECKCMD_SUPPORT
unset  SECURERAWCMD_SUPPORT
unset  IPLICENSING_SUPPORT
unset  NEXUS_COMMON_CRYPTO_SUPPORT
unset  NEXUS_SECURITY_IRDETO_SUPPORT
unset  NEXUS_SECURITY_ALPINE_SUPPORT
unset  NEXUS_SECURITY_VISTA_CWC_SUPPORT
unset  NEXUS_SERVER_SUPPORT

# List all that files that have been build (in the last five minutes.)
find Zeus* -name libnexus_security.a  -mmin -5 -exec chmod -x {} \; -exec ls -lh {} \;
