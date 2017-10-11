This distribution includes binary files in the following subdirectories:

free/     -- Release copy of wince driver installer setup.exe, driver library
             and wl.exe tool
checked/  -- Debug copy of wince driver installer setup.exe, driver library
             and wl.exe tool

If you got a source release package(*src.tar.gz), then it contains following 
structure.  You can use instructions below to build your own CE wl driver:

Source distribution layout:
---------------------------
  src/
  |-- bcmcrypto
  |-- include
  |   |-- bcmcrypto
  |   `-- proto
  |-- makefiles
  |-- shared
  `-- wl
      |-- ce
      |   |-- <device_name>_<device_type>
      |   `-- install
      |       `-- <device_name>
      |           `-- <device_type>
      |-- config
      `-- sys

Where <device_name> is acronym for your win-ce device and <device_type> is 
your device type (either pcmcia or sdio).

Brief overview of above source layout:
--------------------------------------
  src/bcmcrypto   : These three dirs contain all driver source-code
  src/shared      :
  src/wl/sys      :

  src/include     : These two folders contain driver header
  src/wl/sys      : files

  src/makefiles   : These two directories contain global makefiles
  src/wl/config   :

  src/wl/ce/<device_name>_<device_type> : Makefiles to launch driver library 
                     build

  src/wl/ce/install/<device_name>/<device_type> : Makefile to launch driver 
                     installer package build (optional)

Rebuild (compilation) instructions:
-----------------------------------
NOTE: Replace contents within '<' '>' blocks appropriate to your environment

  1. To rebuild WinCE wl driver library (bcmwl5.dll):
  ---------------------------------------------------
  NOTE: You need to have access to these two tools (either installed locally
        or copied from other installed instance)
        1. WINCE 500 toolkit.
        2. Cygwin (any version released in 2005 or above)

  % Extract the *src.tar.gz to a <build-folder> on a windows 2k or xp host
  % cd <build-folder>
  % set _WINCEROOT=<path_where_WinCE500_is_available>
  % set PATH=<path_where_cygwin_is_available>\bin;%PATH%
  % make -C src/wl/ce/<device_name>_<device_type>

    Find built new wl driver library at:
    - src/wl/ce/<device_name>_<device_type>/obj/sta/free/bcmwl5.dll

  2. To rebuild installer ( bcm<device_type>_setup.exe ) [optional]
  -----------------------------------------------------------------
  NOTE: You need to have access to these WIN-CE installer utils 
        (from WinCE 420 or any other WinCE version)
        1. Cabwiz.exe (and Cabwiz.ddf)
        2. Makecab.exe
        3. ezsetup.exe
  NOTE: Copy above tools to a <install_tools_dir>

  % Build the new wl driver as per instruction above (in #1)
  % cd <build-folder>
  % set _WINCE_BUILD_TOOL_DIR=<install_tools_dir>
  % set PATH=<path_where_cygwin_is_available>\bin;%PATH%
  % make -C src/wl/ce/install/<device_name>/<device_type>

    Find built new wl driver library at:
    - src/wl/ce/install/<device_name>/<device_type>/sta/free/bcm<device_type>_setup.exe

   Copy over built bcm<device_type>_setup.exe and bcmwl5.dll to private folder.
   And you can install the new driver from this new folder

