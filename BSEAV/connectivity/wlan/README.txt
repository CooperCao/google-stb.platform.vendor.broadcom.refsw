=======================================
Instructions for building the WLAN code
=======================================

To build the default configuration, simply do:
    Cleaning:
        make -j8 clean
    Building:
        make -j8


However, the WLAN code can be built in many different ways, depending on the values of the following "build flags":

     1. B_WLAN_VER specifies the major version of the WLAN source code (e.g., "STB7271_BRANCH_15_10" or "EAGLE_10_10_84")
        If not specified, the default is "STB7271_BRANCH_15_10".

     2. B_WLAN_BRAND specifies which set of WLAN source code will be used for the build.  If not specified
        the default is "linux-external-stbsoc".

        B_WLAN_BRAND=linux-external-stbsoc       # (Default) Builds using standard WLAN code base
        B_WLAN_BRAND=linux-mfgtest-stbsoc        # Builds using special "manufacturing test" code base

     3. WLAN_DEFAULT_BUILDCFG specifies a list of "targets" for the build.  

        If not specified, the default is:
            "debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-stbsoc-armv7l"		(for 32-bit toolchain) and
            "debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-stbsoc-armv8" 		(for 64-bit toolchain)

        For non-default builds, choose the appropriate targets from the following list.  Their 
        order does not matter, but targets must be separated by a single dash ("-").
        (Credit goes to Binh Vo for making this list.)

            Required targets:
            -apdef-    (AP features to support softAP)
            -stadef-   (STA features)
            -extnvm-   (NVRAM file feature. nvram.txt needs to be in same dir as driver wl.ko)
			-stbsoc-   (BCM7271 WIFI features)
			
            Required target for 32-bit or 64-bit driver. Automatically added by Makefile based on B_REFSW_ARCH.
            -armv7l-   (32-bit ARM arch target.)
			-armv8-    (64-bit ARM arch target.)

            Required target for debug prints:
            -nodebug-       (Less debug prints)
            -debug-         (More debug prints. Use "wl msglevel" to show current prints. "wl msglevel -h" to list all options )

            Optional features:
            -mfgtest-       (Manufacturing driver. Should use -debug- when using this target,
                             also use the  "B_WLAN_BRAND=linux-mfgtest-media"  build flag)
            -wet-           (Wireless Ethernet bridging feature)
            -mfp-           (Protected Frame Management)
            -cfg80211-      (Config 802.11 feature)
            -p2p-mchan-     (Enables p2p and multichannel)
            -tdls-          (Enables Tunneled Direct Link Setup(TDLS))
			-pspretend-     (Enable PS_PRETEND. This is an AP/softAP feature only)

        Some examples:
            Basic 32-bit driver with no debug print logs:
                WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc

            Basic 64-bit driver with no debug print logs:
                WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc

            32-bit driver with WET feature and debug print logs:
                WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-extnvm-wet-stbsoc
			
			32-bit driver with more STA features and no debug print logs:
				WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-p2p-mchan-mfp-tdls-stbsoc
			
			32-bit driver with softAP features and no debug print logs:
				WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc

            Manufacturing 32-bit driver:
                WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-stbsoc

            Manufacturing 64-bit driver with WET support:
                WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-wet-stbsoc


For the custom build configurations, the build flags can be exported environment variables, or 
can be passed on the command line like this:

    Basic 32-bit driver with no debug print logs:
        make -j8  WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc

    Basic 64-bit driver with no debug print logs:
        make -j8  WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc

    32-bit driver with WET feature and debug print logs:
        make -j8  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-extnvm-wet-stbsoc

    Manufacturing 32-bit driver:
        make -j8  B_WLAN_BRAND=linux-mfgtest-stbsoc  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-stbsoc

    Manufacturing 64-bit driver with WET support:
        make -j8  B_WLAN_BRAND=linux-mfgtest-stbsoc  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-wet-stbsoc

    Basic driver from "BCM7271_BRANCH_1_1" with no debug print logs:
        make -j8  B_WLAN_VER=BCM7271_BRANCH_1_1  WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc


For cleaning, use the same make command (with the same build flags), but add "clean" at the end of the line.
For example:

        make -j8  B_WLAN_BRAND=linux-mfgtest-stbsoc  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-wet-stbsoc  clean

Finally, at the end of the build, you should find these files:

        bcm97*.txt      NVRAM files
        wl.ko           WLAN Driver
        wl              "wl" Utility
        wlinstall       Prashant's installation utility

        Which have been copied to:
            .../BSEAV/bin/ (always)
            .../nexus/bin  (if it exists)
            $(INSTALL_DIR) (if it exists)

Additional Notes:
1. How to test TDLS:
Test can be done thru wl cmds
 - Bring up two clients and associate with the same AP.
 - Enable tdls on both clients using the command
 wl tdls_enable 1
 - Run following command from one client to establish TDLS connection between the two clients (mac address should be the
other client):
 wl tdls_endpoint create xx:xx:xx:xx:xx:xx
 - To delete TDLS link run the following command
 wl tdls_endpoint delete xx:xx:xx:xx:xx:xx
