#!groovy

// Run a coverity analysis for the specified build string
def coverityAnalyze(String name, String repoDir, String platformString, String buildCommand, String covPath, String covAnalysisFlags, String covStream, String commitServer, String commitPort, String commitDesc) {
    return node {
        def objDir = "objects."+name.replaceAll(" ","_"); // B_REFSW_OBJ_DIR is relative to top of tree, not absolute
        def covDir = "${repoDir}/coverity."+name.replaceAll(" ","_");;
        stage("build ${name}") {
            // Execute coverity build
            def shellString;
            // Important: Setting B_REFSW_OBJ_ROOT must come after plat now
            shellString = "export PATH=${covPath}:\$PATH;" +
                          "cd ${repoDir};" +
                          "rm -fr ${objDir};" +
                          "rm -fr ${covDir};" +
                          "source BSEAV/tools/build/plat ${platformString};" +
                          "export B_REFSW_OBJ_DIR=${objDir};" +
                          "cov-configure --gcc;" +
                          "cov-build --dir ${covDir} ${buildCommand}"
           sh("bash -c \'${shellString}\'")
           //echo "${shellString}"
        }
        stage("analyze ${name}") {
            // Run coverity analysis
            def shellString;
            shellString = "export PATH=${covPath}:\$PATH;" +
                          "cov-analyze --dir ${covDir} --strip-path ${repoDir} ${covAnalysisFlags}"
            sh("bash -c \'${shellString}\'")
            //echo "${shellString}"
        }
        stage("commit ${name}") {
            // Commit results to connect server
            def shellString;
            shellString = "export PATH=${covPath}:\$PATH;" +
                          "cov-commit-defects --dir ${covDir} --host ${commitServer} --dataport ${commitPort} --stream ${covStream} --user admin --password coverity --description ${commitDesc}"
            sh("bash -c \'${shellString}\'")
            //echo "${shellString}"
        }
    }
}

// Generate a node for a driver build
def driverBuildStep(String platformString, Boolean kernelMode, String repoDir, String covPath, String cgitUrl, String commitServer, String commitPort) {
    // The return from this must be a {} block to defer execution until the 'parallel' operation runs.  The node/stages are defined in coverityAnalyze.
    return {
        def buildString;
        def buildName;
        def analysisOptions;
        def streamName;
        def chipName = platformString.getAt(1..4)

        // Define kernel-mode vs. user-mode build commands
        if ( kernelMode ) {
            buildString = "make -C nexus/utils playback NEXUS_MODE=proxy"
            buildName = "${platformString} driver linuxkernel"
            streamName = "${chipName}NexusDriverKernel"
        }
        else {
            buildString = "make -C nexus/utils playback"
            buildName = "${platformString} driver linuxuser"
            streamName = "${chipName}NexusDriverUser"
        }

        // Setup 64-bit vs. 32-bit analysis options
        def aarch64 = (platformString.getAt((platformString.length()-2)..-1) == "64");
        if ( aarch64 ) {
            analysisOptions = "--jobs max2 --all -en STACK_USE -co STACK_USE:max_single_base_use_bytes:4096 -co STACK_USE:max_total_use_bytes:14744"
            streamName = "${streamName}64"
        }
        else {
            analysisOptions = "--jobs max2 --all -en STACK_USE -co STACK_USE:max_single_base_use_bytes:2048 -co STACK_USE:max_total_use_bytes:7372"
        }

        coverityAnalyze(buildName, repoDir, platformString, buildString, covPath, analysisOptions, streamName, commitServer, commitPort, cgitUrl)
    }
}

// Generate a node for an app build
def appBuildStep(String platformString, Boolean nxclientMode, String repoDir, String covPath, String cgitUrl, String commitServer, String commitPort) {
    // The return from this must be a {} block to defer execution until the 'parallel' operation runs.  The node/stages are defined in coverityAnalyze.
    return {
        def buildString;
        def buildName;
        def analysisOptions = "--all --jobs max4";
        def streamName;
        def chipName = platformString.getAt(1..4)

        // Define kernel-mode vs. user-mode build commands
        if ( nxclientMode ) {
            buildString = "make -C BSEAV/app/atlas/build NXCLIENT_SUPPORT=y"
            buildName = "${platformString} app nxclient"
            streamName = "${chipName}AppNxClient"
        }
        else {
            buildString = "make -C BSEAV/app/atlas/build"
            buildName = "${platformString} app linuxuser"
            streamName = "${chipName}AppUser"
        }

        // Setup 64-bit vs. 32-bit analysis options
        def aarch64 = (platformString.getAt((platformString.length()-2)..-1) == "64");
        if ( aarch64 ) {
            streamName = "${streamName}64"
        }

        coverityAnalyze(buildName, repoDir, platformString, buildString, covPath, analysisOptions, streamName, commitServer, commitPort, cgitUrl)
    }
}

// Entry point called from Jenkins
def call(String repoDir, String gitRemote, String gitRef, String covPath, String commitServer, String commitPort) {
    def driverPlatformStrings = ["97439 b0 sff 32", "97271 b0 sff 64", "97278 a0 sv 32", "97260 a0 sff 32"]
    def appPlatformStrings = ["97271 b0 sff 64", "97271 b0 sff 32"]

    def buildMap = [:]
    def cgitUrl;
    def gitHash;

    // Define node to print hello world and form cgit url
    node {
        echo "Hello, stb_coverity.groovy is running in ${repoDir}"
        gitHash = sh(returnStdout: true, script: "cd ${repoDir}; git rev-parse HEAD")
        cgitUrl = "http://git-irv-03.lvn.broadcom.net/cgit/${gitRemote}.git?h=${gitRef},id=${gitHash}"
        echo "cgit url - ${cgitUrl}"
    }

    // Generate map of driver builds
    for ( int i = 0; i < driverPlatformStrings.size(); i++ ) {
        def str = driverPlatformStrings.get(i)

        def userStepName = "${str} driver linuxuser"
        buildMap[userStepName] = driverBuildStep(str, false, repoDir, covPath, cgitUrl, commitServer, commitPort)

        def kernelStepName = "${str} driver linuxkernel"
        buildMap[kernelStepName] = driverBuildStep(str, true, repoDir, covPath, cgitUrl, commitServer, commitPort)
    }

    // Generate map of app builds
    for ( int i = 0; i < appPlatformStrings.size(); i++ ) {
        def str = appPlatformStrings.get(i)

        def appStepName = "${str} app linuxuser"
        buildMap[appStepName] = appBuildStep(str, false, repoDir, covPath, cgitUrl, commitServer, commitPort)

        def nxStepName = "${str} app nxclient"
        buildMap[nxStepName] = appBuildStep(str, true, repoDir, covPath, cgitUrl, commitServer, commitPort)
    }

    // Allow builds to execute in parallel - make -j is not very effective with coverity
    parallel buildMap
}

return this
