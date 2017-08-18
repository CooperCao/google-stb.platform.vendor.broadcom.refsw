I) Information on contents of any linux-*-wl build:
===================================================

  UTILS:
  ------
  exe/wl: wl util
  
  RPMS:
  -----
  obj-[no]debug-<wltarget>-<os>:
     Inside release subfolder you may find many directory for each 
     <wltarget> and <os> we support. Each of these directory contains
     a mix of binary built objects and source package as shown below
     (These are created by rpm build process automatically):
  
  obj-[no]debug-<wltarget>-<os>/BUILD:
     Copies of built object to be released for given <wltarget>-<os>
     (e.g. wl.o or wl.ko)
     Example: BUILD/src/wl/linux/obj-debug-native-apdef-2.4.20-8/wl.o
      
  obj-[no]debug-<wltarget>-<os>/RPMS:
     When used by any user, the rpm installs the binary objects
     (i.e. driver) at standard os path.
     Example: RPMS/i386/kernel-module-wl-2.4.20-8-2006.3.14.0-4.i386.rpm
  
  obj-[no]debug-<wltarget>-<os>/SOURCES:
     .tar.gz of mogrified sources used to build <wltarget>-<os> binary 
     Example: SOURCES/obj-debug-native-apdef-2006.3.14.0.tar.gz
  
  obj-[no]debug-<wltarget>-<os>/SRPMS:
     Same source .tar.gz packaged in an rpm. This can be built with
     rpmbuild utility. Note the .src.rpm suffix to file name.
     Example: SRPMS/kernel-module-wl-2.4.20-8-2006.3.14.0-4.src.rpm
  
  NOTE: If you release obj-[no]debug-<wltarget>-<os>/BUILD, then 
        obj-[no]debug-<wltarget>-<os>/RPMS is redundant

  NOTE: If you release obj-[no]debug-<wltarget>-<os>/SOURCES, then
        obj-[no]debug-<wltarget>-<os>/SRPMS is redundant
