#
# Broadcom 802.11 Wireless Network Driver
# RPM spec file
#
# $Copyright (C) 2004 Broadcom Corporation$
#
# $Id: wl.spec,v 1.8 2004-07-16 20:31:52 jsiegel Exp $
#

%{!?LINUXVER: %{expand: %%define LINUXVER %(make --no-print-directory -s -C /lib/modules/`uname -r`/build script 'SCRIPT=@echo $(KERNELRELEASE)')}}

%define name kernel-module-wl-%{LINUXVER}
%define version @EPI_VERSION_STR@
%define release 4

Summary: Broadcom 802.11 Wireless Network Driver
Name: %{name}
Version: %{version}
Release: %{release}
Source0: @OBJDIR@-%{version}.tar.gz
License: Broadcom Corporation Confidential
Group: System/Kernel
Url: http://www.broadcom.com
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(echo $$)

Provides: kernel-module
Provides: kernel-module-wl = %{version}-%{release}

%description
This driver provides Linux 2.4.x kernel support for a wide variety of
Broadcom 802.11 devices.

Broadcom Corporation Confidential. For Evaluation Use Only.

%prep
%setup -q -n src

%build
# Make sure that SRCBASE and RPM are not inherited from an upstream make
make -C wl/linux objdir TARGET=@TARGET@ LINUXVER=%{LINUXVER} SRCBASE=$PWD RPM= 

%install
rm -rf $RPM_BUILD_ROOT
install -D -m 644 wl/linux/@OBJDIR@-%{LINUXVER}/wl.o $RPM_BUILD_ROOT/lib/modules/%{LINUXVER}/kernel/drivers/net/wl.o

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/lib/modules/%{LINUXVER}/kernel/drivers/net/wl.o

%post
if [ "`uname -r`" = "%{LINUXVER}" ] ; then
    depmod -a >/dev/null 2>&1 || :
fi

%postun
if [ "`uname -r`" = "%{LINUXVER}" ] ; then
    depmod -a >/dev/null 2>&1 || :
fi
