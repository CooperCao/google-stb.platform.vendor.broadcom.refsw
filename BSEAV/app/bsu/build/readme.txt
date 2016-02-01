URSR 15.4 Notes/Instructions.

BOLT version 1.08 is required
CFE version 15.4 is required

To install BOLT.
1. Copy BRCM_BOLT_Generic_0_1_08_E1.tgz to a temporary directory.
2. bash-4.2$ tar zxvf BRCM_BOLT_Generic_0_1_08_E1.tgz
3. bash-4.2$ cd release_bolt_v1.08/src
4. bash-4.2$ tar zxvf BRCM_Src_BOLT_Generic_0_1_08_E1.tgz
5. Copy bolt_v1.08 to the same top-level directory containing BSEAV, magnum, etc.,
6. Build the appropriate BOLT for your board, or just enter "make bolts" to build all.

To install CFE
1. Copy BRCM_CFE_Generic_15.4_E1.zip to a temporary directory.
2. bash-4.2$ unzip BRCM_CFE_Generic_15.4_E1.zip
3. bash-4.2$ cd release_unified_cfe_v15.4/src
4. base-4.2$ unzip unified_cfe_v15.4.zip
5. Copy v15.4/CFE_15_4 to the same top-level directory containing BSEAV, magnum, etc.,
