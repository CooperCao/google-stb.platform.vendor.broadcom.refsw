URSR 16.1 Notes/Instructions.

BOLT version 1.10 is required
CFE version 16.1 is required

To install and build BOLT.
Note that building BOLT is necessary to building BSU, because building BOLT generates a config.h file, needed by BSU.
1. Copy BRCM_BOLT_Generic_0_1_10_E1.tgz to a temporary directory.
2. Change directory to this location.
3. bash-4.2$ tar zxvf BRCM_BOLT_Generic_0_1_10_E1.tgz
4. bash-4.2$ cd release_bolt_v1.10/src
5. bash-4.2$ tar zxvf BRCM_Src_BOLT_Generic_0_1_10_E1.tgz
6. Copy bolt_v1.10 to the same top-level directory containing BSEAV, magnum, etc.,
7. Change directory to this bolt_v1.10 location.
8. bash-4.2$ export PATH=/opt/toolchains/stbgcc-4.8-1.1/bin:$PATH
9. Build the appropriate BOLT for your board, or just enter "make bolts" to build all.

To install CFE
1. Copy BRCM_CFE_Generic_16.1_E1.zip to a temporary directory.
2. bash-4.2$ unzip BRCM_CFE_Generic_16.1_E1.zip
3. bash-4.2$ cd release_unified_cfe_v16.1/src/
4. bash-4.2$ unzip unified_cfe_v16.1.zip
5. Copy v16.1/CFE_16_1 to the same top-level directory containing BSEAV, magnum, etc.,
