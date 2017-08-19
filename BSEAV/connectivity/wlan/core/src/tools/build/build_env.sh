#! bash.exe
#
# build_env.sh - sets environment for building all of src under bash
#
# For a 'gmake all' to succeed from the top level of the source tree, the
# environment must be set to include 4 major components: LOCAL, MSDEV, MSSDK
# and DDK. Optinally NTICE can be set too.
#
# LOCAL is the collection of tools we use, including Cygnus's development
# environment for win32. MSDEV is Microsoft's Visual C++ development
# environment.  MSSDK is an additional set of libraries and includes from
# Microsoft, and the DDK is Microsoft's device driver development environment.
# If present NTICE is NuMega's Softice.
#
# This script provides defaults for any of them by looking for the right
# directories in drive c: and then in drive z:.
#
# This shell script requires that all 4 of these environment variables, if
# set;  point to the respective installed directories using a DOS (but
# still SH) style path, e.g. z:/tools/msdev.
#
# Copyright 1998,1999 Epigram, Inc.
#
# $Id$
#

# A function that takes a variable NAME as the first argument, and a
# list of directories for subsequent arguments.  If the environment
# variable named NAME is set, it is left along.  If NAME is not set,
# the directories listed as subsequent arguments are searched and NAME
# is set to the first valid directory found.
#
set_default_dir () {
    var=$1
    shift;
    if eval "[ \"x\${$var}\" = \"x\" ]" ; then
	for dir in $@
	do
	    split_path dir
	    if  [ -d "$dirDIR_SH" ]; then
		eval $var=\"$dir\";
		eval echo "$0: setting $var to default of \$$var";
		eval export $var;
		return;
	    fi
	done
	if eval "[ \"x\${$var}\" = \"x\" ]" ; then
	    echo "Error: ${var} is not set and cannot find an appropriate directory from among $@";
	    exit 1;
        fi
    else
	eval echo "$0: using $var=\$$var";
    fi
}

# for each environment we need both a DOS style path and a SH style
# path so we split the DOS style path into DRIVE and PATH and then
# construct both styles.
#
# This function takes a single variable NAME as an
# argument. If the environment variable named NAME is set, it is split and two
# new environment variables, ${NAME}DIR_DOS and ${NAME}DIR_SH
#
split_path () {
    _var=$1
    if type cygpath >/dev/null 2>&1; then
	eval vpath=\${${_var}}
	eval ${_var}DIR_SH='`cygpath -u ${vpath}`'
	eval ${_var}DIR_DOS='`cygpath -w ${vpath}`'
    else
       eval _DRIVE=\${${_var}%:*}
       eval _PATH="\${$_var##*:}"
       if [ "x$_DRIVE" = "x$_PATH" ]; then
   	eval echo "$0: ${_var}=\$${_var} is not a DOS style path";
   	exit 1;
       fi;
       eval ${_var}DIR_SH=\"//$_DRIVE${_PATH//\\\\//}\";
       eval ${_var}DIR_DOS=\"$_DRIVE:${_PATH//\//\\}\";
    fi;
}

if [ $OS != "Windows_NT" ]; then
    echo "$0: OS is not Windows_NT, try running bash on NT";
    sleep 5;
    exit 1;
fi;

winpfx=${WLAN_WINPFX:-Z:}
HNDTOOLS=$winpfx/projects/hnd/tools
set_default_dir LOCAL ${LOCAL} c:/tools/win32 ${HNDTOOLS}/win32
set_default_dir MSDEV c:/tools/msdev/Studio d:/Studio $winpfx/tools/msdev/Studio
set_default_dir MSSDK ${HNDTOOLS}/win/msdev/PlatformSDK
set_default_dir DDK c:/tools/msdev/2600ddk d:/2600ddk $winpfx/tools/msdev/2600ddk

# set_default_dir exits if the variable can't be set, but we don't want
# to exit if optional NTICE isn't installed
if [ -z "$NTICE" ]; then
	if [ -d c:/tools/SoftIceNt ]; then
		export NTICE=c:/tools/SoftIceNT
	elif [ -d ${HNDTOOLS}/win/numega/SoftICE ]; then
		export NTICE=${HNDTOOLS}/win/numega/SoftICE
	elif [ -d $winpfx/tools/SoftIceNT ]; then
		export NTICE=$winpfx/tools/SoftIceNT
	fi
fi

# for each environment we need both a DOS style path and a SH style path
# so we split the DOS style path into DRIVE and PATH and then construct
# both styles.

split_path LOCAL
split_path MSDEV
split_path MSSDK
split_path DDK

if [ "${NTICE}" != "" ]; then
	split_path NTICE
fi

CPU=i386;
export CPU;
OS_DIR=WINNT;


# To prevent any inconsistency with what may already be in the environment,
# all variables exported by this script are overwritten.  This may seem
# a little heavy handed but the point of this script is to "guarantee" a
# successful build.

# setup for Cygwin
LOCAL="${LOCALDIR_DOS//\\\\//}"
LOCALDIR_SH_BACK="${LOCALDIR_SH//\//\\}"
GCC_EXEC_PREFIX=$LOCALDIR_SH_BACK\\lib\\gcc-lib\\;
MAKE_MODE=unix;
export GCC_EXEC_PREFIX;
export MAKE_MODE;

# we construct a new path and then prepend it to the existing path
# when we're all done.
NEW_PATH=$LOCALDIR_SH/bin;


# setup for VC++
MSDEVDIR=$MSDEVDIR_DOS
if [ -d "$MSDEVDIR_SH/Common" ]
then
   # The following is largely taken from the NT5beta2 DDK in
   # z:\tools\msdev\nt50ddk\bin\ddkvars.bat
   #

    NEW_PATH=$NEW_PATH:$MSDEVDIR_SH/Common/msdev98/bin
    NEW_PATH=$NEW_PATH:$MSDEVDIR_SH/VC98/bin
    NEW_PATH=$NEW_PATH:$MSDEVDIR_SH/Common/TOOLS/${OS_DIR}
    NEW_PATH=$NEW_PATH:$MSDEVDIR_SH/Common/TOOLS

    NEW_INCLUDE=$MSDEVDIR/VC98/ATL/INCLUDE
    NEW_INCLUDE=${NEW_INCLUDE}\;$MSDEVDIR/VC98/INCLUDE
    NEW_INCLUDE=${NEW_INCLUDE}\;$MSDEVDIR/VC98/MFC/INCLUDE

    NEW_LIB=$MSDEVDIR/VC98/LIB
    NEW_LIB=${NEW_LIB}\;$MSDEVDIR/VC98/MFC/LIB
else
    NEW_PATH=$NEW_PATH:$MSDEVDIR_SH/bin:$MSDEVDIR_SH/bin/${OS_DIR}
    NEW_INCLUDE=$MSDEVDIR/include\;$MSDEVDIR/mfc/include\;
    NEW_LIB=$MSDEVDIR/lib\;$MSDEVDIR/mfc/lib\;
fi

export MSDEVDIR;

# setup for sdk (ddk depends on sdk)
MSSDK=$MSSDKDIR_DOS
MSTOOLS=$MSSDK
NEW_PATH=$NEW_PATH:$MSSDKDIR_SH/bin:$MSSDKDIR_SH/bin/$OS_DIR
NEW_INCLUDE=$MSSDK/include\;$NEW_INCLUDE
NEW_LIB=$MSSDK/lib\;$NEW_LIB

export MSSDK;
export MSTOOLS;
export INCLUDE;
export LIB;

# setup for ddk builds
BASEDIR=$DDKDIR_DOS;
NEW_PATH=$NEW_PATH:$DDKDIR_SH/bin;
# sdk include before ddk include
NEW_INCLUDE=$NEW_INCLUDE\;$BASEDIR/inc;
NTMAKEENV=$BASEDIR/inc;
BUILD_MAKE_PROGRAM=nmake.exe;
BUILD_DEFAULT="-ei -nmake -i";
BUILD_DEFAULT_TARGETS=-386;

export BASEDIR;
export NTMAKEENV;
export BUILD_MAKE_PROGRAM;
export BUILD_DEFAULT;
export BUILD_DEFAULT_TARGETS;

# setup for checked build (default)
DDKBUILDENV=checked;
C_DEFINES="-D_IDWBUILD -DRDRDBG -DSRVDBG";
NTDBGFILES=;
NTDEBUG=ntsd;
NTDEBUGTYPE=both;
MSC_OPTIMIZATION="/Od /Oi";

export DDKBUILDENV;
export C_DEFINES;
export NTDBGFILES;
export NTDEBUG;
export NTDEBUGTYPE;
export MSC_OPTIMIZATION;

_OBJ_DIR=obj;
NEW_CRTS=1;
_NTROOT=$BASEDIR;

export _OBJ_DIR;
export NEW_CRTS;
export _NTROOT;

NEW_PATH=${NEW_PATH//\/\///}
echo NEW_PATH=${NEW_PATH}

# finally, prepend NEW_PATH to PATH
PATH=$NEW_PATH:$PATH;
INCLUDE=$NEW_INCLUDE\;$INCLUDE
LIB=$NEW_LIB\;$LIB

export PATH;
export INCLUDE;
export LIB;

# to build everything, TARGETENV must be win32
export TARGETENV=win32;

# To run the simulator, set the tcl library
if [ "${TCL_LIBRARY}" == "" ]; then
	export TCL_LIBRARY=$winpfx\\projects\\hnd\\tools\\win32\\usr\\share\\tcl8.0
fi
