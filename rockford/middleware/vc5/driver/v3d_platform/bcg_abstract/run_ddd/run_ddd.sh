#!/bin/bash

# Run run_ddd.sh help to have more info on how to use this script
# This script needs to be run from the build bin dir where the binary of the app is.

# Requirements to use this script on your own linux machine (in contrast to the build server):
# - you should have access to fs-bri-01.bri.broadcom.com:/vol/vol02904/stbopt_p from your linux machine
#   To have access to this share you either need to log into your linux machine using NIS or
#   to be logged with a user that has the same uid than your user on the build server
# - fs-bri-01.bri.broadcom.com:/vol/vol02904/stbopt_p should be mounted into /opt_p
# - ln -s /opt_p/brcm /opt/brcm
# - ln -s /opt_p/toolchains_303 /opt/toochains (this might have to change sometime?)

OK=1

case $0 in
*bash)
    # you're sourcing it. you're ok
    ;;
*)
  if [[ $1 != 'help' ]] && [[ $1 ]]; then
    echo =======================================================================
    echo The arguments of this script will only be kept as environment variables
    echo in the current terminal if the file is sourced:
    echo source run_ddd.sh [App name] [IP address of the board] [gdbserver port]
    echo After sourcing the script the first time, you can run it without parameter:
    echo run_ddd.sh
    echo =======================================================================
  fi
esac

function help()
{
    echo =========================================================================
    echo usage: source run_ddd.sh [App name] [IP address of the board] [gdbserver port]
    echo usage: run_ddd.sh [App name] [IP address of the board] [gdbserver port]
    echo
    echo This script needs to be run from the build bin dir where the binary of the app is.
    echo
    echo The arguments of this script will only be kept as environment variables
    echo in the current terminal if the file is sourced:
    echo source run_ddd.sh [App name] [IP address of the board] [gdbserver port]
    echo After sourcing the script the first time, you can run it without parameters:
    echo run_ddd.sh
    echo
    echo You can change the name of the app with:
    echo source run_ddd.sh APP_NAME
    echo
    echo run_ddd.sh help: print this message
    echo
    echo This script modifies .gdbinit in the current directory as well as the .gdbinit
    echo in the users home directory. The original .gdbinit files are saved
    echo "(as .gdbinit_saved) and restored when the script finished."
    echo =========================================================================
    OK=0
}

if [[ $1 == 'help' ]] ; then
  help
fi

if [[ $1 ]] ; then
  export V3D_DDD_APP=$1
fi

if [[ $2 ]] ; then
  export V3D_DDD_BOARD_IP=$2
fi

if [[ $3 ]] ; then
  export V3D_DDD_GDBSERVER_PORT=$3
fi

if [[ $V3D_DDD_APP == '' ]] || [[ $V3D_DDD_BOARD_IP == '' ]] || [[ $V3D_DDD_GDBSERVER_PORT == '' ]] ; then
  if [[ $OK == 1 ]] ; then
    help
  fi
fi

if [[ :$PATH: != *:"${TOOLCHAIN_PATH}":* ]] ; then
  export PATH=${TOOLCHAIN_PATH}:$PATH
fi

bin_gdbinit_moved=0
home_gdbinit_moved=0

if [[ $OK == 1 ]] ; then

  # It will backup and restore the .gdbinit in the current directory as well
  # as the .gdbinit in the user's home directory

  if [[ -f ".gdbinit" ]] ; then
     bin_gdbinit_moved=1
     echo Copy .gdbinit in current dir to .gdbinit_saved
     cp -f .gdbinit .gdbinit_saved
  fi
  # Copy the .gdbinit that will be used by ddd
  cp -f .gdbinit_ddd .gdbinit

  # eval echo replace ~ by the real path
  if [[ -f "`eval echo ~/.gdbinit`" ]] ; then
     home_gdbinit_moved=1
     echo Copy .gdbinit in your home directory into .gdbinit_saved
     cp -f ~/.gdbinit ~/.gdbinit_saved
  fi

  # Copy the .gdbinit that will be used by ddd
  # This .gdbinit is used to enable the loading of .gdbinit from other locations
  cp -f .gdbinit_home_ddd ~/.gdbinit

  # Run ddd
  ddd --debugger ${gdb_name} --eval-command="target remote $V3D_DDD_BOARD_IP:$V3D_DDD_GDBSERVER_PORT" --eval-command="break main" $V3D_DDD_APP

  # Restore all the original gdbinit files
  if [[ $bin_gdbinit_moved == 1 ]] ; then
     echo Restore .gdbinit in the current dir
     cp -f .gdbinit_saved .gdbinit
  else
     rm -f .gdbinit
  fi
  if [[ $home_gdbinit_moved == 1 ]] ; then
     echo Restore .gdbinit in your home directory
     cp -f ~/.gdbinit_saved ~/.gdbinit
  else
     rm -f ~/.gdbinit
  fi
fi