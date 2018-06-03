URSR 18.2 Notes/Instructions.

BOLT version 1.42 is required
CFE version 18.2 is required

To install and build BOLT.
Note that building BOLT is necessary to building BSU, because building BOLT generates a config.h file, needed by BSU.
1. Copy BRCM_BOLT_Generic_0_1_42_E1.tgz to a temporary directory.
2. Change directory to this location.
3. bash-4.2$ tar zxvf BRCM_BOLT_Generic_0_1_42_E1.tgz
4. bash-4.2$ cd release_bolt_v1.42/src
5. bash-4.2$ tar zxvf BRCM_Src_BOLT_Generic_0_1_42_E1.tgz
6. Copy bolt_v1.42 to the directory two levels above the directory containing BSEAV, magnum, etc.,
7. Change directory to this bolt_v1.42 location.
8. bash-4.2$ export PATH=/opt/toolchains/stbgcc-6.3-1.2/bin:$PATH
9. Build the appropriate BOLT for your board, or just enter "make bolts" to build all.

To install CFE
1. Copy BRCM_CFE_Generic_18.2_E1.zip to a temporary directory.
2. bash-4.2$ unzip BRCM_CFE_Generic_18.2_E1.zip
3. bash-4.2$ cd release_unified_cfe_v17.4/src/
4. bash-4.2$ unzip unified_cfe_v18.2.zip
5. Copy v18.2/CFE_18_2 to the directory two levels above the directory containing BSEAV, magnum, etc.,

If building for 73465 or 74295, you must change CFE_17_4\bsp\mips_config.h as follows:

From:

#if (BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 7435)

To:

#if (BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 73465 || BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 74295 || BCHP_CHIP == 7435)
