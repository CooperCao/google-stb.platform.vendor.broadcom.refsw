#! bash
# This is not a script to be exec-ed. It is intended for
# use as a bash startup file, to set up an initial Cygwin
# build environment. We cause it to be read at shell startup
# with BASH_ENV or similar.

# If triggered via BASH_ENV, make sure that's a one-time thing.
unset BASH_ENV

# Be unforgiving of errors at this startup stage.
set -o errexit

# Allow scripts with CRLF line endings to work correctly.
set -o igncr

# Ensure path funcs will work with case-insensitive filesystems.
shopt -s nocaseglob nocasematch

# Make sure the above get through to child shells. This also means
# any subsequent settings will be silently passed down, so beware.
export BASHOPTS

# We used to use P:\ as HOME but now that /home is mounted
# in fstab we can use it directly. Note that the /home mount
# will only be visible to Cygwin processes but fortunately
# the $HOME variable only has meaning to Cygwin as well.
export HOME=/home/$LOGNAME
cd $HOME

# Assert that this variable has a value.
: ${CYGWIN_DIRECTORY?}

# This is important because the following may set something
# like TEMP=C:\Temp. Cygwin/Unix-like utilities should use /tmp,
# native Windows programs should use C:\Temp. Fortunately TMPDIR
# is ignored by native programs so if we set TMPDIR to a unix
# path and TEMP to a DOS path, everybody's happy.
# Specifically, Cygwin svn fails when its temp dir is a DOS path.
export TMPDIR=/tmp

# Here we update the C:/tools/build scripts dir.
# Each build will always have the latest scripts, and scripts are
# less likely to change during a build, and fewer updates mean fewer
# opportunities for things to break. And it doesn't require maintaining
# a scheduled task on each box. However, the update could occasionally
# fail if two builds start at the same time so we continue past
# update errors.
if [[ -w /cygdrive/c/tools/build ]]; then
    if [[ -f /cygdrive/c/tools/build/.svn/wc.db ]]; then
	# Use Cygwin svn for 1.7+
	(set -x; /usr/bin/svn update /cygdrive/c/tools/build) ||:
    else
	# Otherwise assume svn 1.6 and use old native version
	# TODO this clause can go away once native svn is gone.
	(set -x; C:/tools/Subversion/svn.exe update C:/tools/build) ||:
    fi
fi

# Export a set of environment variables derived from set_buildenv.bat.
source /cygdrive/c/tools/build/set_buildenv.sh

# There can be many different versions of HOME in Cygwin;
# the UNC path, P:\, the Unix /home, the Cygwin /home, etc.,
# all multiplied by / vs \. Use this one to mount P:.
declare home_unc=//corp.ad.broadcom.com/sjca$HOME

# Bring in some shell functions for manipulating PATH.
# We borrow hwnbuild's copy regardless of actual user name.
# TODO This is all backward compatibility work intended to
# remove bogus old stuff from PATH that was historically set
# in the registry.
# TODO Once we're not running builds on those older machines
# any more this path-removal code should be obsolete.
declare pathfuncs=${home_unc%/*}/hwnbuild/BASHRC/func/pathfuncs
if [[ -r $pathfuncs ]]; then
    source $pathfuncs

    # Remove paths to native tools which were needed with 1.5 but
    # are replaced with the bundled Cygwin versions now.
    offpath /cygdrive/c/tools/Python /cygdrive/c/tools/Subversion
    offpath /cygdrive/c/tools/vim/vim72 /cygdrive/c/tools/vim/vim73

    # Remove path entries for Cygwin 1.5 binaries ...
    offpath /cygdrive/c/Tools/win32/bin /cygdrive/c/Tools/Win32/usr/local/bin

    # ... and replace them with the modern equivalent.
    onpath /usr/bin

    # Remove redundant path entries.
    cleanpath
fi

# This function will mount a UNC path on a drive letter.
function winmount {
    declare drive=$1 dir=$2
    declare pswd=${PASSWD_CACHE:-$home_unc/.restricted/passwd}
    test -e "$drive/." || {
	net use "$drive" /delete &>/dev/null ||:
	# Password isn't always required so use it only if it's available.
	if [[ -r "$pswd" ]]; then
	    net use "$drive" "$(cygpath -w $dir)" /persistent:yes "$(<$pswd)" >/dev/null
	else
	    net use "$drive" "$(cygpath -w $dir)" /persistent:yes >/dev/null
	fi
    }
}

# Ensure the home directory is mounted as P:. This is no longer the
# value of $HOME as used by Cygwin but is still the method of home dir
# access for native Windows processes. However, since we've seen some
# instances of "System Error: (1311) There are currently no logon
# servers available to service the logon request" and P: may not be
# required any more, a failure here is ignored.
winmount P: $home_unc ||:

# Make sure other required drives are mounted.
# Specs must be given as "X://unc/path" (forward slashes).
for spec in ${WIN_MOUNT_LIST:-Z://brcm-sj/dfs}; do
    winmount ${spec%%/*} ${spec#*:}
done

# Downstream shell code often cannot handle -e (errexit) mode.
set +o errexit

# This sets the registry key allowing cmd.exe to cd into a UNC path.
# If we got here at all we probably don't need it but some later process
# might. At the very least it documents the key and a way of setting it.
# UPDATE: turned off because (a) unneeded and (b) fails on some test/dev
# systems (corporate laptops), but retained for doc purposes.
# declare regpath="/HKLM/SOFTWARE/Microsoft/Command Processor/DisableUNCCheck"
# /usr/bin/regtool -w set $regpath 1
# /usr/bin/regtool -W set $regpath 1

# Our older Cygwin shells may be vulnerable to the "ShellShock" exploit.
# This code would alert and fail if such an attempt was made. HIGHLY unlikely
# to occur, but checking is the responsible thing (until bash shells are fixed).
typeset shock="$(env | grep '=() {')"
if [[ -n "$shock" ]]; then
    typeset vars="$(echo $(env | grep '=() {' | cut -f1 -d=))"
    printf "Error: shellshock content found in exported variables: '$vars'" >&2
    exit 9
fi

# This is to make sure igncr etc get through to all the shells we care about.
export SHELLOPTS

# If an original BASH_ENV was temporarily hijacked for this use, restore it.
if [[ -n "$BASH_ENV_ORIG" ]]; then
    export BASH_ENV=$BASH_ENV_ORIG
    unset BASH_ENV_ORIG
    source $BASH_ENV
fi
