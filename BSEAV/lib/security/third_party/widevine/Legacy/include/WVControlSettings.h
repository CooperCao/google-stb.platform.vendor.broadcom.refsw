/****************************************************************************************************
 * WVControlSettings.h
 *
 * (c) Copyright 2011-2012 Google, Inc.
 *
 * Widevine API types
 ***************************************************************************************************/

#ifndef __WVCONTROLSETTINGS_H__
#define __WVCONTROLSETTINGS_H__

#include <string>

/* WVCredentials is used in both WVPlaybackAPI and WVStreamControlAPI. */
struct WVCredentials {
    std::string deviceID;        // unique player device ID from CinemaNow
    std::string streamID;        // unique streamID from CinemaNow
    std::string clientIP;        // IP address of client
    std::string drmServerURL;    // URL for DRM server
    std::string userData;        // Additional optional user data, TBD
    std::string portal;          // Identifies the operator
    std::string storefront;      // Identifies store run by operator
    std::string drmAckServerURL; // URL for server that receives
                                 //     entitlement confirmations
    std::string heartbeatURL;    // URL to receive client heartbeats
    unsigned int heartbeatPeriod;// Duration between consecutive heartbeats in
                                 // seconds. 0 indicates no heatbeats requested
    unsigned int cnDeviceType;   // device type identifier defined by CinemaNow
};

/* WVProxySettings is used in both WVPlaybackAPI and WVStreamControlAPI. */
struct WVProxySettings {
    bool enable;              // If true, proxy use is enable, otherwise disabled
    std::string ipAddr;       // IP address of proxy server, e.g. "1.2.3.4" or host name
    unsigned short ipPort;    // IP port number
    std::string userId;       // User ID if authentication is needed, otherwise "" to disable authentication
    std::string password;     // Password if userID is not ""
};


#endif // __WVCONTROLSETTINGS_H__
