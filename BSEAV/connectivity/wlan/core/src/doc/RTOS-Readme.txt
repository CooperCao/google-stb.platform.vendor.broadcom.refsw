This distribution is sample code for porting the WLAN driver to Nucleus and 
other similar Real-Time Operating Systems (RTOS). Ports to new operating systems should 
be based on the supported official release, typically the linux build. Using 
this sample code as a guide, however, may help you get started, and may make it 
easier for you to merge in future updated releases from Broadcom.

This sample code does not come with makefiles, a toolchain, or a target 
environment. We can only advise that we have built it for Nucleus with 
ARM RVDS 2.2 and run it on our Broadcom 2153 TorpedoRay development platform. 
Testing on this platform has been limited.

There are several categories of files involved in an RTOS port:


1. Changes that are typical of any RTOS.

This release includes the following new files that are intended to be used as-is 
with any RTOS port:

    src/include/osl_ext.h
    src/include/generic_osl.h
    src/shared/generic_osl.c
    src/wl/exe/wlu_generic.c
    src/wl/exe/wlu_server_generic.c
    src/include/bcmsdh_generic.h
    src/bcmsdio/sys/bcmsdh_generic.c

The above files reflect common characteristics of RTOS ports, such as 
the lack of a kernel/user mode split, the lack of a file system to store 
the dongle image, and the linking of all files including the wl and dhd 
command line utilities into a single image.

Additional files support an optional OS-independent lbuf packet structure:

    src/include/lbuf.h
    src/shared/lbuf.c
    src/include/pkt_lbuf.h
    src/shared/pkt_lbuf.c
    src/shared/pkt_lbuf_generic.c


2. Changes required to port to the specific operating system.

The sample code includes a port to the Nucleus RTOS kernel. This is 
directly useful if your target is Nucleus. For another operating system
-- let's call the target OS "xxx" -- create new files to implement the 
OS-specific code. Specifically:

    Create /include/xxx_osl.h and xxx_osl_ext.h with necessary macros and types 
following the example of /include/nucleus_osl.h and nucleus_osl_ext.h

    Update /include/osl.h to conditionally include xxx_osl.h, based upon 
a makefile definition.

    Create /shared/xxx_osl.c and xxx_osl_ext.c based on /shared/nucleus_osl.c 
and nucleus_osl_ext.c.


3. Changes required to port to a new TCP/IP stack.

Implement callbacks defined by "dhd_drv_netif_callbacks_t" in 
/include/wl_drv.h.


4. Changes required to interface to a new SDIO host controller.

Replace bcmsdstd.c and bcmsdstd_linux.c with equivalent source files 
that interface to the new SDIO host controller. New source files should 
implement the API specified by /include/bcmsdbus.h.


The remaining files are common with the official linux release. The RTOS,
Linux, and Windows releases have all been updated with changes to make the code 
more easily ported to an RTOS, avoiding possible structure packing problems, 
eliminating duplicate symbol errors, and eliminating warnings from the 
ARM RVDS 2.2 compiler.

Please also refer to README.txt and ReleaseNotes.htm.
