=======================================
Instructions for building the WLAN code
=======================================

To build the default configuration, simply do:
    Cleaning:
        make -j8 clean
    Building:
        make -j8
To build stbapd:
    Key exports:
    export B_WLAN_STBAPD={1|0}  // enables stbapd Makefile targets. See below.
    export WLAN_DEFAULT_BUILDCFG={driver target}  // Note. if driver target contains -stbapd-
							will automatically build stbapd_drivers and enable B_WLAN_STBAPD
    export BUILD_BRCM_HOSTAPD={y|n}  // y, builds opensource external authenticator files(libnl,openssl,wpa_supplicant,hostapd,iw etc.).


    Makefile targets:
    make stbapd_build_all  :  builds both stbapd apps and stbapd drivers(emf,igs,dpsta)
    make stbapd_install    :  copies stbapd drivers and apps to common out of source location (i.e obj.97271.B0/BSEAV/bin/stbapd)
				+ will create tarball package (i.e obj.97271.B0/BSEAV/bin/stbapd/ext_auth/target_bin.bz2)
    make stbapd_clean_all  :  cleans both stbapd apps and driver completely
    make stbapd_drv_clean :  cleans just stbapd driver
    make stbadpd_apps_clean:  clean just stbapd apps

    Examples:
    // External Authenticator:Creates complete packages of sta driver, stbapd apps, stbapd driver, and  using opensource.
    plat 97271 B0 64 4.9-1.12 or plat 97271 B0 (takes defaults bit and kernel) //sets platfrom variables,toolchain and kernel
    make -j8 WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-cfg80211-stbapd-multiap-stbsoc // building 7271 driver
    make -j8 stbapd_build_all WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-cfg80211-stbapd-multiap-stbsoc BUILD_BRCM_HOSTAPD=y //stapbd drv+apps+opensrc
    make stbapd_install WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-cfg80211-stbapd-multiap-stbsoc  BUILD_BRCM_HOSTAPD=y  // copy and tarball

    // Internal Authenticator:Creates complete packages of sta driver, stbapd apps, stbapd driver.
    plat 97271 B0 64 4.9-1.12 or plat 97271 B0 (takes defaults bit and kernel)
    make -j8 WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-stbapd-multiap-stbsoc
    make -j8 stbapd_build_all WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-stbapd-multiap-stbsoc 
    make stbapd_install WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-stbapd-multiap-stbsoc 

    //cleans everything
    make clean WLAN_DEFAULT_BUILDCFG=debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-cfg80211-stbapd-multiap-stbsoc BUILD_BRCM_HOSTAPD={y|n}

However, the WLAN code can be built in many different ways, depending on the values of the following "build flags":

     1. B_WLAN_VER specifies the major version of the WLAN source code (e.g., "7271" )
        If not specified, the default is "7271".

     2. NO more brands: 

     3. WLAN_DEFAULT_BUILDCFG specifies a list of "targets" for the build.  

        If not specified, the default is:
            "debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l"		(for 32-bit toolchain) and
            "debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8" 		(for 64-bit toolchain)

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
            -wowl-          (Wake on Wireless Lan)
            -slvradar-      (Slave Radar Detection.  OBSOLETE.  This is part of standard, not an option)

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
        make -j8  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-stbsoc

    Manufacturing 64-bit driver with WET support:
        make -j8  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-wet-stbsoc

    Basic driver with no debug print logs:
        make -j8  WLAN_DEFAULT_BUILDCFG=nodebug-apdef-stadef-extnvm-stbsoc


For cleaning, use the same make command (with the same build flags), but add "clean" at the end of the line.
For example:

        make -j8  WLAN_DEFAULT_BUILDCFG=debug-apdef-stadef-mfgtest-extnvm-wet-stbsoc  clean

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
