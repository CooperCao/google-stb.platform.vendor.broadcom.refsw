WPS SDK and Sample Utility Program for Windows XP and Vista

Requirements:
* Windows XP SP2 or Vista
* You need a Broadcom WiFi adapter with a recent version of its driver
* You need to install the VS2005 runtime DLLs (included in this package) if you do not have VS2005 installed
* You need to install WinpCap (included in this package)

Installation:
* extract all files to a folder
* run vcredist_x86.exe to install the VS2005 runtime DLLs (included in this package)
* run WinPcap_4_0_2.exe to install WinPcap (included in this package)
* run wps_api_test.exe to try out our WPS client engine. 
* run wps_test_gui.exe to try out our WPS client engine using a GUI. 

Release notes:
This version 5.10.40 (and later) has the following enhancements upon 5.10.27.5:
* Added new APIs including wps_hwbutton_supported, wps_get_hwbutton_state, wps_generate_pin and wps_configureAP to enable the enhancements
* Added support that STA ER (External Registrar) configures an "unconfigured" AP
* Added support that STA ER gets networking settings from "configured" AP 
* Added API to generate a valid PIN code
* Added API to query WPS GPIO PIN button status ("Pused"/"Released") to support implementation to triggering WPS PBC mode via hardware push button. A dongle with WPS GPIO enabled button is required.
* Updated wps GUI test program to demo newly added enhancements 
