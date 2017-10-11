#
#  WFA Test Engine Source Code (DUT and Control PC) Revision 2.30 

# Release History:
# Initial Release: Version 1.0, July 10 2006, Qiumin Hu, Wi-Fi Alliance
#
# Revision: 01.05, September 28 2006   bug fixes, maintenance release
# Revision: 01.06, October 26 2006     bug fixes (environment variables), maintenance release
# Revision: 01.10, January 11 2007     bug fixes, maintenance release
# Revision: 01.40, March 30 2007       bug fixed, enhancement and official WMM Beta
# Revision: 02.00, April 20 2007       WPA2 maintenance and official WMM 
# Revision: 02.10, August 17, 2007     WMM Power Save and IBSS support
# Revision: 02.30, November 7, 2007    Voice - Personal support
#
# REFER TO THE ReleaseNote-230.txt FOR THE SET OF UPDATES THAT 
# DOCUMENT THE SPECIFIC CHANGES BETWEEN THIS VERSION AND
# THE PREVIOUS Released VERSION
#
# The revisions are done in the following files:
#     wfa_dut.c     -- Voice - Personal 
#     wfa_cmdproc.c -- Voice - Personal 
#     wfa_tg.c      -- Voice - Personal 
#     wfa_tg.h      -- Voice - Personal 
#     wfa_cs.c      -- Voice - Personal 
#     wfa_misc.c    -- Voice - Personal
#     wfa_dut_init.c -- newly added support.
#
           
0. Introduction
   This source code package contains a sample Linux implementation of the DUT code for 
   the Wi-Fi Test Engine.  This DUT code includes the Wi-Fi Traffic Generator.  
   The DUT code is written to be portable to other platforms.
  
   Also included here is Linux source code for the Control PC component of the Wi-Fi Test Engine.
   The sample code uses TCP over USB to provide connectivity between the DUT and the Control PC. 
   This code will need to be modified to support alternate connectivity methods. 
   
   The third component included is called the Test Console.  This component is a peer traffic 
   Generator to the DUT's traffic generator that can run on the Control PC 

Please refer to the WFA Porting Guidelines document for detailed instructions on the porting process. 

The following instructions pertain to the direct use of this sample code in a Linux environment.

************************************************ IMPORTANT****************************************************
Before building the software, if you plan to support WPA2 as default, you can 
simply go to the step 1 "Building the software". 

If the DUT is preparing for WMM support, the lines in the file "Makefile.inc" 
for WMM is needed to be uncommented and the default setting for WPA2 should 
be commented out as well.

For WMM-PS, make sure to uncomment the line to build WMM-PS and comment out 
original build-line.

To support Voice, the build in the Makefile.inc must include WFA_WMM_EXT and 
WFA_WMM_VOICE

The current code supports WPA2, WMM, WMM-PS, and Voice.

1. Building the software
   a. build all
      all you need is to simply type "make all".

   b. build individual modules
      cd into the appropriate directory to edit and type "make"

   c. build a specific agent 
      there are three agents in this package. They are "dut", "ca" and "tc", 
      standing for "Device Under Test", "Control Agent" and "Test Console" 
      respectively.

      To build one of them only, you need to edit the file "Makefile.inc" to
      comment rest of modules out

   d. clean modules and object files
      simply type "make clean" or cd into the module directories, 
      type "make clean"

2. Installing the software
   a. copy dut/wfa_dut to /usr/sbin (or other location) on the 
      linux DUT

   b. MUST copy scripts/getipconfig.sh to /usr/local/sbin on the Linux DUT. 

3. Running the software
   3.1 DUT -- dut/wfa_dut
   a. The current implementation is based on the open source wpa_supplicant
      (hostap.epitest.fi/wpa_supplicant). The DUT must have it installed.

   b. The supplicant can be either auto started during the system boot or 
      manually started by typing "wpa_supplicant -Dmadwifi -iath0 -c "any 
      configuration file". Before doing this, you need to have a console 
      terminal to access/login the DUT.
      Please refer the wpa_supplicant document/README and Linux setup document 
      for setting auto-start.

   c. The IP address for the control link interface (can be USB, ethernet, or 
      others with TCP/IP/aLink) needs to be set, e.g. 10.10.1.100 . This should be set for a differnt
      subnet from the traffic agent interface (Wi-Fi interface).

   d. Once you have already logged in the DUT, type:
      wfa_dut <control link interface, etc usb0> <local tcp port, e.g. 8000>  

   3.2 Control Agent (LINUX PC) -- ca/wfa_ca
   a. Before starting the control agent, first you need to make sure or set 
      the IP address for the control link interface (e.g. 10.10.1.110) that 
      matches the subnet set in the DUT. Second, you need to set two 
      environment variables. Type:
      a.1: export WFA_ENV_AGENT_IPADDR=<IP address of the DUT's control 
           interface, e.g. 10.10.1.100>
      a.2: export WFA_ENV_AGENT_PORT=<Port number of the DUT control TCP sits
           on, e.g. 8000>  

   b. Start the control agent:
      wfa_ca <local ethernet interface to communicate to others such as Test 
              Manager, CLI commander or Console with Python scripts e.g eth0> 
	      <local port to listen on, e.g. 9000> 

   c. Now you are ready.

   3.3 Control Agent CLI (LINUX PC) -- cli/ca_cli
   a. Before using this CLI commander, you need to have two environment 
      variables set.
      a.1 export WFA_ENV_CA_IPADDR=<IP address of the control agent, e.g.
               192.168.1.201, the local IP address>
      a.2 export WFA_ENV_CA_PORT=<The local port of control agent listening,
               e.g. 9000>

   b. type: ca_cli <full command string, e.g. ca_get_version>
      for more commands, please refer WFA CAPI document.

   3.4 Console using Python Scripts.
       Read the instructions in the directory "ucc" for how to setup python if 
       it is not installed on your PC. It also has instructions for the script
       file modification.

   3.5 PC-WTG (Linux) -- tc/wfa_tc tc/wfa_pctg 
       The current Linux implementation is similar to DUT that is consist of
       two programs, pc traffic generator and test console. 
       Please take this as a reference peer only for validating your porting, 
       and note that for actual certification testing a different peer traffic generator
       will be used.  

   a. Starts PC TG:
      wfa_pctg <local interface, e.g. eth0> <traffic generator control port, e.g. 9800> 

      a.1: export WFA_ENV_PCTG_IPADDR=<IP address of the DUT's control 
           interface, e.g. 192.168.1.201>
      a.2: export WFA_ENV_PCTG_PORT=<Port number of the DUT control TCP sits
           on, e.g. 9800>  
      wfa_tc <local ethernet interface to listen commands from CLI, e.g. eth0>
             <local port number, e.g. 9850>

   3.6 Test Console CLI (LINUX PC) -- cli/tc_cli
   a. Before using this CLI commander, you need to have two environment 
      variables set.
      a.1 export WFA_ENV_TC_IPADDR=<IP address of the control agent, e.g.
               192.168.1.201, the local IP address on eth0>
      a.2 export WFA_ENV_TC_PORT=<The local port of control agent listening,
               e.g. 9850>

   b. type: tc_cli <full command string, e.g. ca_get_version>
      for more commands, please refer WFA CAPI document for traffic generator
      commands.

   3.7 Test Console with Python Script
       see 3.4

4. Debug utility
   There is a CLI command implemented to enable debug output. The debug levels
   include
   a. ERROR -- Default and not Changeable  (level 1)
   b. INFO  -- Can be turned on/off        (level 2)
   c. WARNING -- Can be turned on/off      (level 4)

   e.g. The command:
   ca_cli sta_debug_set,level,2,enable,1 
             will enable to debug INFO
   ca_cli sta_debug_set,level,2,enable,0 
             will disable to debug INFO

For questions, send emails to qhu@wi-fi.org

