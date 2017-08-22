URSR 17.1 Notes/Instructions.

BOLT version 1.31 is required
CFE version 17.1 is required

To install and build BOLT.
Note that building BOLT is necessary to building BSU, because building BOLT generates a config.h file, needed by BSU.
1. Copy BRCM_BOLT_Generic_0_1_31_E1.tgz to a temporary directory.
2. Change directory to this location.
3. bash-4.2$ tar zxvf BRCM_BOLT_Generic_0_1_31_E1.tgz
4. bash-4.2$ cd release_bolt_v1.31/src
5. bash-4.2$ tar zxvf BRCM_Src_BOLT_Generic_0_1_31_E1.tgz
6. Copy bolt_v1.31 to the directory two levels above the directory containing BSEAV, magnum, etc.,
7. Change directory to this bolt_v1.31 location.
8. bash-4.2$ export PATH=/opt/toolchains/stbgcc-4.8-1.5/bin:$PATH
9. Build the appropriate BOLT for your board, or just enter "make bolts" to build all.

To install CFE
1. Copy BRCM_CFE_Generic_17.1.2_E1.zip to a temporary directory.
2. bash-4.2$ unzip BRCM_CFE_Generic_17.1.2_E1.zip
3. bash-4.2$ cd cd release_unified_cfe_v17.3/src/
4. bash-4.2$ unzip unified_cfe_v17.3.zip
5. Copy v17.3/CFE_17_3 to the directory two levels above the directory containing BSEAV, magnum, etc.,

If building for 73465 or 74295, you must change CFE_17_3\bsp\mips_config.h as follows:

From:

#if (BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 7435)

To:

#if (BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 73465 || BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 74295 || BCHP_CHIP == 7435)
