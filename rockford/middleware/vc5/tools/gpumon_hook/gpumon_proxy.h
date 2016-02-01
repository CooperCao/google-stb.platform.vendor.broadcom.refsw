/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  SpyHook
Module   :  Proxy server for Android

FILE DESCRIPTION

Provides a local proxy server than can be run with root privileges to allow
applications which have been built without INTERNET permissions to be
monitored.

=============================================================================*/

#ifndef __SPYPROXY_H__
#define __SPYPROXY_H__

#define FIFO_CTRL           "/data/app/spyhook/fifo_ctrl"
#define FIFO_ACK            "/data/app/spyhook/fifo_ack"
#define FIFO_TX             "/data/app/spyhook/fifo_tx"
#define FIFO_RX             "/data/app/spyhook/fifo_rx"

#endif /* __SPYPROXY_H__ */
