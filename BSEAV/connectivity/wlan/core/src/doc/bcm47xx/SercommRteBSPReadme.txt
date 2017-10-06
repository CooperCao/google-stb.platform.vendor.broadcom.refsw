		** RTE BSP Demo code package for Sercomm **

This file describes the contents of the package and how to build, load the 
demo code on BCM95352E platform.
1. Toolchain requirement
TBD

2. Content of the package
------------------
src/sercomm/           - Contains the basic support package 
src/sercomm/sercomm.c  - A demo application that interface with the
                            Broadcom code
src/sercomm/rtebsp.h   - Interface with Broadcom code
src/sercomm/iob.h      - IOB API mapped to Broadcom buffer routines
src/sercomm/debug-sercomm   - Target directory for debug version of sercomm.trx.  Also contains
                              binary code from Broadcom necessary to build the trx.
src/sercomm/nodebug-sercomm  - Target directory for non-debug version of sercomm.trx.  Also contains
                              binary code from Broadcom necessary to build the trx.
src/include               - Contains header files necessary for compilation
src/include/flashutl.h    - API for raw flash access
src/include/bcmnvram.h    - API for accessing Broadcom NVRAM configuration
src/image/debug-sercomm - Prebuilt image file with debug support
src/image/nodebug-sercomm - Prebuilt image file with debug support


3. Compiling and Loading:
---------------------
cd src/sercomm
make -f sercomm-build.mk

This will create sercomm.trx. This trx can be loaded on any 5352E with CFE.


4. BSP Description:
---------------
* Supports 5352E only.
* Acts as AP only (no support for TCP/IP/PING/ARP or other protocols).
* Configuration changes are made through CFE


5. Using CFE to Change Configuration:
---------------------------------
The BSP does not include a management interface.  The CFE prompt can be used
to get and set configuration during system testing.  It is expected that the
customer will add their own management interface that calls the Broadcom
NVRAM API.

To get to the CFE command prompt, when the device is booting repeatedly hit
Ctrl-C until you receive the CFE> prompt.  At the cfe prompt you can issue
the following commands:
CFE> nvram show            - Show all parameters and their current values
CFE> nvram get <XXX>       - Get a value, e.g., nvram get wl0_ssid
CFE> nvram set <XXX>=<YYY> - Set a value, e.g., nvram set wl0_ssid="CustTest"
CFE> nvram commit          - Commit "nvram set" changes to flash
