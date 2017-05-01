===============================================================================
===============================================================================
Release Notes for the example PlayReady CDMi Implementation

This document details the contents of this package, system requirements,
installation steps, new behavior, and known issues.


===============================================================================
Notice
===============================================================================

© 2014 Microsoft Corporation.  All rights reserved.
Your evaluation and/or use of this software and related materials are subject
to the terms and conditions of the agreement under which Microsoft provided the
software and related materials to you.  If an authorized employee of your
company did not sign and return the agreement to Microsoft you are not
authorized to install, copy or otherwise evaluate and/or use the software and
you must uninstall and delete this entire software package and its materials
from your computer.

Unless required by applicable law or under the terms and conditions of the
agreement under which Microsoft provided the software and related materials to
you, the software is licensed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either expressed or implied.


===============================================================================
Contact Information
===============================================================================

To provide feedback or request support, email askdrm@Microsoft.com.


===============================================================================
System Requirements
===============================================================================

The Example PlayReady CDMi Implementation can be built/run under the following
assumptions.

- Official PlayReady PK 2.5 is installed & built - both DRM_BUILD_TYPE=CHECKED
  and DRM_BUILD_TYPE=FREE - under the directory:
  C:\PlayReady\Device_PK_2.5.0\ (default location)

- DRM environment has been setup according the PK 2.5 documentation.

- The following official PlayReady PK 2.5 release files are located under the
  directory c:\wmdrmpd\
  o bgroupcert.dat
  o devcerttemplate.dat
  o priv.dat
  o zgpriv.dat

- Visual Studio 2012 or later is installed.

===============================================================================
Contents of this package
===============================================================================

cdmi.h, cdmi_imp.h, drmlibs.h, otadla.h, otadla.cpp, main.cpp,
mediaenginesession.cpp, mediakeys.cpp, mediakeysession.cpp, and
cdmitest.vcxproj: source and project files for building an example PlayReady
CDMi Implementation and a test program for exercising that implementation.

mediaengine_certchain.dat, mediaengine_privkey.dat, pssh.dat: sample data files
for exercising the PlayReady CDMi Implementation.

Note: The CDM interface (CDMi) is described in: "Content Decryption Module
Interface Specification", http://www.microsoft.com/playready/documents/


===============================================================================
How to build the CDMi Implementation and cdmitest program
===============================================================================

To build the example CDMi Implementation and cdmitest program, unzip the
contents to a new folder and double click cdmitest.vcxproj.

Build/run after Visual Studio is launched by pressing F5.


===============================================================================
Notes
===============================================================================

A future version of the PlayReady Porting Kit will incorporate a fully
integrated Content Decryption Module interface Implementation. The current
release is an example CDMi Implementation, only. It is missing several key
features which will be provided in that future PK release. Specifically,
the precise mechanism for processing the media engine certificate chain,
which has not been finalized.

We are providing this example implementation to our licensees at this time
because of the importance we place in an open interface for accessing platform
DRM technology from a browser. Such an interface has just been specified in
the "Content Decryption Module Interface Specification". The importance of this
capability is described in the white paper "Interoperability, Digital Rights
Management and the Web".

Both the specification and the white paper can be found on the PlayReady
documents webpage - http://www.microsoft.com/playready/documents/.

# end
