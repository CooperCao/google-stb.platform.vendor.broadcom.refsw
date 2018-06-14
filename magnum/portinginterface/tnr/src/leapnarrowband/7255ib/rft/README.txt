Rafael Tuner Build Instructions:
-------------------------------

1. To build Rafael Tuner for BCM97255 platform, Rafael Tuner source code should be obtained from Rafael Micro.
   The build will fail, if the original Rafael Tuner Source Code is not supplied
2. Convert R642_API_VERSION_Multi.rar file to R642_API_Multi.tgz
3. Place it at the top level of BRCM source tree, same level as the nexus directory
4. Broadcom Nexus build system will take care of extracting, patching and building Rafael Tuner Code