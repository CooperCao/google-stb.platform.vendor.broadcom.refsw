Rafael Tuner Build Instructions:
-------------------------------

1. To build Rafael Tuner for BCM97255 platform, Rafael Tuner source code should be obtained from Rafael Micro.
   The build will fail, if the original Rafael Tuner Source Code(for e.g. R642_API_v1.2J8_Multi.rar file) is not provided
2. Place it at the top level of Broadcom source tree, same level as the nexus directory
3. Broadcom Nexus build system will take care of extracting, patching and building Rafael Tuner Code. This process
   requires 7z utility installed on the build server, as 7z is used to extract the .rar file