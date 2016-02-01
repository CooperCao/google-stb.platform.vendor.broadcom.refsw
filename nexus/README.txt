Nexus Instructions
Broadcom Set-Top Box Software

There are three ways to learn how to use Nexus:

1. Read nexus/docs

    Nexus_Overview.pdf provides a high-level overview.
    Nexus_Usage.pdf tells you how to get started using Nexus.

2. Study and run nexus/examples on a Broadcom reference board

    Build:
        export NEXUS_PLATFORM=97xxx
        cd nexus/examples; make

    Run:
        nexus graphics
        nexus decode
        nexus playback

    Many example apps have hardcoded pids, codecs or clear DVR recordings which can be customized.

    nexus/utils contains more complex utility applications which are driven by command line parameters
    or by scanning DVR recordings.

3. Read Nexus API level documentation found in nexus/modules/*/include/*.h.


Also, see nexus/nxclient/README.txt for multi-process systems.
