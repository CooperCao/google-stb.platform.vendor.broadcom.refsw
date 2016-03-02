/******************************************************************************
 * (c) 2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bip_priv.h"
#include "bip_http_server_impl.h"
#include "bip_udp_streamer_impl.h"
#include "bip_http_socket_impl.h"
#include "bip_rtsp_socket_impl.h"
#include "bip_http_streamer_impl.h"
#include "bip_transcode_impl.h"

BDBG_MODULE( bip_to_str );


/* Case-sensitive name lookup. */
static unsigned BIP_FromStr_LookupWithDefault(const namevalue_t *table, const char *name, unsigned defaultValue)
{
    unsigned i;
    for (i=0;table[i].name;i++) {
        if (!strcmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    return defaultValue;
}

#if 0 /* Enable this when needed. */
/* Case-insensitive name lookup. */
static unsigned BIP_FromStr_LookupNoCaseWithDefault(const namevalue_t *table, const char *name, unsigned defaultValue)
{
    unsigned i;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    return defaultValue;
}
#endif

/* Please note: The following functions are arranged in alphabetical order of BIP data type. */


const char * BIP_ToStr_BIP_Arb_State( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized" , BIP_ArbState_eUninitialized },
        { "Idle"          , BIP_ArbState_eIdle          },
        { "Acquired"      , BIP_ArbState_eAcquired      },
        { "Submitted"     , BIP_ArbState_eSubmitted     },
        { "Accepted"      , BIP_ArbState_eAccepted      },
        { "Completed"     , BIP_ArbState_eCompleted     },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_Arb_ThreadOrigin( int value)
{
    const namevalue_t myStrings[] = {
        { "Arb"                , BIP_Arb_ThreadOrigin_eArb                  },
        { "Unknown"            , BIP_Arb_ThreadOrigin_eUnknown              },
        { "IoChecker"          , BIP_Arb_ThreadOrigin_eIoChecker            },
        { "Timer"              , BIP_Arb_ThreadOrigin_eTimer                },
        { "BipCallback"        , BIP_Arb_ThreadOrigin_eBipCallback          },
        { "MaybeUpstreamLocks" , BIP_Arb_ThreadOrigin_eMaybeUpstreamLocks   },
        { "NoUpstreamLocks"    , BIP_Arb_ThreadOrigin_eNoUpstreamLocks      },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

static const namevalue_t BIP_HttpRequestMethod_Strings[] = {
    { "HEAD"    , BIP_HttpRequestMethod_eHead     },
    { "GET"     , BIP_HttpRequestMethod_eGet      },
    { "POST"    , BIP_HttpRequestMethod_ePost     },
    { "PUT"     , BIP_HttpRequestMethod_ePut      },
    { "DELETE"  , BIP_HttpRequestMethod_eDelete   },
    { "CONNECT" , BIP_HttpRequestMethod_eConnect  },
    { "OPTIONS" , BIP_HttpRequestMethod_eOptions  },
    { "TRACE"   , BIP_HttpRequestMethod_eTrace    },
    {NULL, 0}
};

const char * BIP_ToStr_BIP_HttpRequestMethod( int value)
{
    const char * pName = lookup_name(BIP_HttpRequestMethod_Strings, (value));
    if (pName == NULL)
    {
        pName = "Unknown";
    }
    return (pName);
}

unsigned BIP_FromStr_BIP_HttpRequestMethod( const char * name)
{
    return (BIP_FromStr_LookupWithDefault(BIP_HttpRequestMethod_Strings, name, BIP_HttpRequestMethod_eUnknown));
}

/* Returns NULL for unrecognized enum values. */
const char * BIP_ToStr_BIP_HttpResponseStatus( int value)
{
    const namevalue_t BIP_HttpResponseStatus_ToStrings[] = {
        { "Continue"                             , BIP_HttpResponseStatus_e100_Continue                       },
        { "Switching Protocols"                  , BIP_HttpResponseStatus_e101_SwitchingProtocols             },
        { "Processing"                           , BIP_HttpResponseStatus_e102_Processing                     },
        { "OK"                                   , BIP_HttpResponseStatus_e200_OK                             },
        { "Created"                              , BIP_HttpResponseStatus_e201_Created                        },
        { "Accepted"                             , BIP_HttpResponseStatus_e202_Accepted                       },
        { "Non-Authoritative Information"        , BIP_HttpResponseStatus_e203_NonAuthoritativeInformation    },
        { "No Content"                           , BIP_HttpResponseStatus_e204_NoContent                      },
        { "Reset Content"                        , BIP_HttpResponseStatus_e205_ResetContent                   },
        { "Partial Content"                      , BIP_HttpResponseStatus_e206_PartialContent                 },
        { "Multi-Status"                         , BIP_HttpResponseStatus_e207_MultiStatus                    },
        { "Already Reported"                     , BIP_HttpResponseStatus_e208_AlreadyReported                },
        { "IM Used"                              , BIP_HttpResponseStatus_e226_IMUsed                         },
        { "Multiple Choices"                     , BIP_HttpResponseStatus_e300_MultipleChoices                },
        { "Moved Permanently"                    , BIP_HttpResponseStatus_e301_MovedPermanently               },
        { "Found"                                , BIP_HttpResponseStatus_e302_Found                          },
        { "See Other"                            , BIP_HttpResponseStatus_e303_SeeOther                       },
        { "Not Modified"                         , BIP_HttpResponseStatus_e304_NotModified                    },
        { "Use Proxy"                            , BIP_HttpResponseStatus_e305_UseProxy                       },
        { "(Unused)"                             , BIP_HttpResponseStatus_e306_Unused                         },
        { "Temporary Redirect"                   , BIP_HttpResponseStatus_e307_TemporaryRedirect              },
        { "Permanent Redirect"                   , BIP_HttpResponseStatus_e308_PermanentRedirect              },
        { "Bad Request"                          , BIP_HttpResponseStatus_e400_BadRequest                     },
        { "Unauthorized"                         , BIP_HttpResponseStatus_e401_Unauthorized                   },
        { "Payment Required"                     , BIP_HttpResponseStatus_e402_PaymentRequired                },
        { "Forbidden"                            , BIP_HttpResponseStatus_e403_Forbidden                      },
        { "Not Found"                            , BIP_HttpResponseStatus_e404_NotFound                       },
        { "Method Not Allowed"                   , BIP_HttpResponseStatus_e405_MethodNotAllowed               },
        { "Not Acceptable"                       , BIP_HttpResponseStatus_e406_NotAcceptable                  },
        { "Proxy Authentication Required"        , BIP_HttpResponseStatus_e407_ProxyAuthenticationRequired    },
        { "Request Timeout"                      , BIP_HttpResponseStatus_e408_RequestTimeout                 },
        { "Conflict"                             , BIP_HttpResponseStatus_e409_Conflict                       },
        { "Gone"                                 , BIP_HttpResponseStatus_e410_Gone                           },
        { "Length Required"                      , BIP_HttpResponseStatus_e411_LengthRequired                 },
        { "Precondition Failed"                  , BIP_HttpResponseStatus_e412_PreconditionFailed             },
        { "Payload Too Large"                    , BIP_HttpResponseStatus_e413_PayloadTooLarge                },
        { "URI Too Long"                         , BIP_HttpResponseStatus_e414_URITooLong                     },
        { "Unsupported Media Type"               , BIP_HttpResponseStatus_e415_UnsupportedMediaType           },
        { "Range Not Satisfiable"                , BIP_HttpResponseStatus_e416_RangeNotSatisfiable            },
        { "Expectation Failed"                   , BIP_HttpResponseStatus_e417_ExpectationFailed              },
        { "Misdirected Request"                  , BIP_HttpResponseStatus_e421_MisdirectedRequest             },
        { "Unprocessable Entity"                 , BIP_HttpResponseStatus_e422_UnprocessableEntity            },
        { "Locked"                               , BIP_HttpResponseStatus_e423_Locked                         },
        { "Failed Dependency"                    , BIP_HttpResponseStatus_e424_FailedDependency               },
        { "Unassigned"                           , BIP_HttpResponseStatus_e425_Unassigned                     },
        { "Upgrade Required"                     , BIP_HttpResponseStatus_e426_UpgradeRequired                },
        { "Unassigned"                           , BIP_HttpResponseStatus_e427_Unassigned                     },
        { "Precondition Required"                , BIP_HttpResponseStatus_e428_PreconditionRequired           },
        { "Too Many Requests"                    , BIP_HttpResponseStatus_e429_TooManyRequests                },
        { "Unassigned"                           , BIP_HttpResponseStatus_e430_Unassigned                     },
        { "Request Header Fields Too Large"      , BIP_HttpResponseStatus_e431_RequestHeaderFieldsTooLarge    },
        { "Internal Server Error"                , BIP_HttpResponseStatus_e500_InternalServerError            },
        { "Not Implemented"                      , BIP_HttpResponseStatus_e501_NotImplemented                 },
        { "Bad Gateway"                          , BIP_HttpResponseStatus_e502_BadGateway                     },
        { "Service Unavailable"                  , BIP_HttpResponseStatus_e503_ServiceUnavailable             },
        { "Gateway Timeout"                      , BIP_HttpResponseStatus_e504_GatewayTimeout                 },
        { "HTTP Version Not Supported"           , BIP_HttpResponseStatus_e505_HTTPVersionNotSupported        },
        { "Variant Also Negotiates"              , BIP_HttpResponseStatus_e506_VariantAlsoNegotiates          },
        { "Insufficient Storage"                 , BIP_HttpResponseStatus_e507_InsufficientStorage            },
        { "Loop Detected"                        , BIP_HttpResponseStatus_e508_LoopDetected                   },
        { "Unassigned"                           , BIP_HttpResponseStatus_e509_Unassigned                     },
        { "Not Extended"                         , BIP_HttpResponseStatus_e510_NotExtended                    },
        { "Network Authentication Required"      , BIP_HttpResponseStatus_e511_NetworkAuthenticationRequired  },
        {NULL, 0}
    };

    const char * pName = lookup_name(BIP_HttpResponseStatus_ToStrings, (value));
    return (pName);
}

/* Returns BIP_HttpResponseStatus_e9999_Unknown if string not recognized. */
unsigned BIP_FromStr_BIP_HttpResponseStatus( const char * name)
{
    const namevalue_t BIP_HttpResponseStatus_FromStrings[] = {
        { "100" , BIP_HttpResponseStatus_e100_Continue                        },
        { "101" , BIP_HttpResponseStatus_e101_SwitchingProtocols              },
        { "102" , BIP_HttpResponseStatus_e102_Processing                      },
        { "200" , BIP_HttpResponseStatus_e200_OK                              },
        { "201" , BIP_HttpResponseStatus_e201_Created                         },
        { "202" , BIP_HttpResponseStatus_e202_Accepted                        },
        { "203" , BIP_HttpResponseStatus_e203_NonAuthoritativeInformation     },
        { "204" , BIP_HttpResponseStatus_e204_NoContent                       },
        { "205" , BIP_HttpResponseStatus_e205_ResetContent                    },
        { "206" , BIP_HttpResponseStatus_e206_PartialContent                  },
        { "207" , BIP_HttpResponseStatus_e207_MultiStatus                     },
        { "208" , BIP_HttpResponseStatus_e208_AlreadyReported                 },
        { "226" , BIP_HttpResponseStatus_e226_IMUsed                          },
        { "300" , BIP_HttpResponseStatus_e300_MultipleChoices                 },
        { "301" , BIP_HttpResponseStatus_e301_MovedPermanently                },
        { "302" , BIP_HttpResponseStatus_e302_Found                           },
        { "303" , BIP_HttpResponseStatus_e303_SeeOther                        },
        { "304" , BIP_HttpResponseStatus_e304_NotModified                     },
        { "305" , BIP_HttpResponseStatus_e305_UseProxy                        },
        { "306" , BIP_HttpResponseStatus_e306_Unused                          },
        { "307" , BIP_HttpResponseStatus_e307_TemporaryRedirect               },
        { "308" , BIP_HttpResponseStatus_e308_PermanentRedirect               },
        { "400" , BIP_HttpResponseStatus_e400_BadRequest                      },
        { "401" , BIP_HttpResponseStatus_e401_Unauthorized                    },
        { "402" , BIP_HttpResponseStatus_e402_PaymentRequired                 },
        { "403" , BIP_HttpResponseStatus_e403_Forbidden                       },
        { "404" , BIP_HttpResponseStatus_e404_NotFound                        },
        { "405" , BIP_HttpResponseStatus_e405_MethodNotAllowed                },
        { "406" , BIP_HttpResponseStatus_e406_NotAcceptable                   },
        { "407" , BIP_HttpResponseStatus_e407_ProxyAuthenticationRequired     },
        { "408" , BIP_HttpResponseStatus_e408_RequestTimeout                  },
        { "409" , BIP_HttpResponseStatus_e409_Conflict                        },
        { "410" , BIP_HttpResponseStatus_e410_Gone                            },
        { "411" , BIP_HttpResponseStatus_e411_LengthRequired                  },
        { "412" , BIP_HttpResponseStatus_e412_PreconditionFailed              },
        { "413" , BIP_HttpResponseStatus_e413_PayloadTooLarge                 },
        { "414" , BIP_HttpResponseStatus_e414_URITooLong                      },
        { "415" , BIP_HttpResponseStatus_e415_UnsupportedMediaType            },
        { "416" , BIP_HttpResponseStatus_e416_RangeNotSatisfiable             },
        { "417" , BIP_HttpResponseStatus_e417_ExpectationFailed               },
        { "421" , BIP_HttpResponseStatus_e421_MisdirectedRequest              },
        { "422" , BIP_HttpResponseStatus_e422_UnprocessableEntity             },
        { "423" , BIP_HttpResponseStatus_e423_Locked                          },
        { "424" , BIP_HttpResponseStatus_e424_FailedDependency                },
        { "425" , BIP_HttpResponseStatus_e425_Unassigned                      },
        { "426" , BIP_HttpResponseStatus_e426_UpgradeRequired                 },
        { "427" , BIP_HttpResponseStatus_e427_Unassigned                      },
        { "428" , BIP_HttpResponseStatus_e428_PreconditionRequired            },
        { "429" , BIP_HttpResponseStatus_e429_TooManyRequests                 },
        { "430" , BIP_HttpResponseStatus_e430_Unassigned                      },
        { "431" , BIP_HttpResponseStatus_e431_RequestHeaderFieldsTooLarge     },
        { "500" , BIP_HttpResponseStatus_e500_InternalServerError             },
        { "501" , BIP_HttpResponseStatus_e501_NotImplemented                  },
        { "502" , BIP_HttpResponseStatus_e502_BadGateway                      },
        { "503" , BIP_HttpResponseStatus_e503_ServiceUnavailable              },
        { "504" , BIP_HttpResponseStatus_e504_GatewayTimeout                  },
        { "505" , BIP_HttpResponseStatus_e505_HTTPVersionNotSupported         },
        { "506" , BIP_HttpResponseStatus_e506_VariantAlsoNegotiates           },
        { "507" , BIP_HttpResponseStatus_e507_InsufficientStorage             },
        { "508" , BIP_HttpResponseStatus_e508_LoopDetected                    },
        { "509" , BIP_HttpResponseStatus_e509_Unassigned                      },
        { "510" , BIP_HttpResponseStatus_e510_NotExtended                     },
        { "511" , BIP_HttpResponseStatus_e511_NetworkAuthenticationRequired   },
        {NULL, 0}
    };

    return (BIP_FromStr_LookupWithDefault(BIP_HttpResponseStatus_FromStrings, name, BIP_HttpResponseStatus_e9999_Unknown));
}

const char * BIP_ToStr_BIP_HttpServerListenerState( int value)
{
    const namevalue_t myStrings[] = {
        { "Idle"             ,     BIP_HttpServerListenerState_eIdle                },
        { "New"              ,     BIP_HttpServerListenerState_eNew                 },
        { "Listening"        ,     BIP_HttpServerListenerState_eListening           },
        { "RequestQueueFull" ,     BIP_HttpServerListenerState_eRequestQueueFull    },
        { "Max"              ,     BIP_HttpServerListenerState_eMax                 },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpServerSocketState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized"              , BIP_HttpServerSocketState_eUninitialized               },
        { "Idle"                       , BIP_HttpServerSocketState_eIdle                        },
        { "WaitingForRequestArrival"   , BIP_HttpServerSocketState_eWaitingForRequestArrival    },
        { "WaitingForRecvRequestApi"   , BIP_HttpServerSocketState_eWaitingForRecvRequestApi    },
        { "WaitingForStartStreamerApi" , BIP_HttpServerSocketState_eWaitingForStartStreamerApi  },
        { "ProcessingRequest"          , BIP_HttpServerSocketState_eProcessingRequest           },
        { "Destroying"                 , BIP_HttpServerSocketState_eDestroying                  },
        { "Max"                        , BIP_HttpServerSocketState_eMax                         },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpServerStartState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized" , BIP_HttpServerStartState_eUninitialized },
        { "Idle"          , BIP_HttpServerStartState_eIdle          },
        { "ReadyToStart"  , BIP_HttpServerStartState_eReadyToStart  },
        { "Started"       , BIP_HttpServerStartState_eStarted       },
        { "Stopped"       , BIP_HttpServerStartState_eStopped       },
        { "Max"           , BIP_HttpServerStartState_eMax           },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpSocketCallbackState( int value)
{
    const namevalue_t myStrings[] = {
        { "Disabled" , BIP_HttpSocketCallbackState_eDisabled },
        { "Enabled"  , BIP_HttpSocketCallbackState_eEnabled  },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpSocketConsumerCallbackState( int value)
{
    const namevalue_t myStrings[] = {
        { "Disabled"  , BIP_HttpSocketConsumerCallbackState_eDisabled   },
        { "Armed"     , BIP_HttpSocketConsumerCallbackState_eArmed      },
        { "Triggered" , BIP_HttpSocketConsumerCallbackState_eTriggered  },
        { "Max"       , BIP_HttpSocketConsumerCallbackState_eMax        },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpSocketShutdownState( int value)
{
    const namevalue_t myStrings[] = {
        { "Normal"        , BIP_HttpSocketShutdownState_eNormal         },
        { "StartShutdown" , BIP_HttpSocketShutdownState_eStartShutdown  },
        { "FinishingArbs" , BIP_HttpSocketShutdownState_eFinishingArbs  },
        { "ShutdownDone"  , BIP_HttpSocketShutdownState_eShutdownDone   },
        { "Max"           , BIP_HttpSocketShutdownState_eMax            },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpSocketState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized"          , BIP_HttpSocketState_eUninitialized             },
        { "Idle"                   , BIP_HttpSocketState_eIdle                      },
        { "ReceiveNewRequest"      , BIP_HttpSocketState_eReceiveNewRequest         },
        { "ReceivingRequest"       , BIP_HttpSocketState_eReceivingRequest          },
        { "ReceivingRequestDone"   , BIP_HttpSocketState_eReceivingRequestDone      },
        { "ReceivedRequest"        , BIP_HttpSocketState_eReceivedRequest           },
        { "ReceiveRequestTimedout" , BIP_HttpSocketState_eReceiveRequestTimedout    },
        { "ReadyToSendResponse"    , BIP_HttpSocketState_eReadyToSendResponse       },
        { "SendNewResponse"        , BIP_HttpSocketState_eSendNewResponse           },
        { "SendingResponse"        , BIP_HttpSocketState_eSendingResponse           },
        { "SendingResponseDone"    , BIP_HttpSocketState_eSendingResponseDone       },
        { "SentResponse"           , BIP_HttpSocketState_eSentResponse              },
        { "ReadyToSendPayload"     , BIP_HttpSocketState_eReadyToSendPayload        },
        { "SendResponseTimedout"   , BIP_HttpSocketState_eSendResponseTimedout      },
        { "SendNewPayload"         , BIP_HttpSocketState_eSendNewPayload            },
        { "SendingPayload"         , BIP_HttpSocketState_eSendingPayload            },
        { "SendingPayloadDone"     , BIP_HttpSocketState_eSendingPayloadDone        },
        { "SentPayload"            , BIP_HttpSocketState_eSentPayload               },
        { "SendPayloadTimedout"    , BIP_HttpSocketState_eSendPayloadTimedout       },
        { "SendingRequest"         , BIP_HttpSocketState_eSendingRequest            },
        { "SentRequest"            , BIP_HttpSocketState_eSentRequest               },
        { "ReceiveNewResponse"     , BIP_HttpSocketState_eReceiveNewResponse        },
        { "ReceivingResponse"      , BIP_HttpSocketState_eReceivingResponse         },
        { "ReceivedResponse"       , BIP_HttpSocketState_eReceivedResponse          },
        { "ReadyToReceivePayload"  , BIP_HttpSocketState_eReadyToReceivePayload     },
        { "ReceiveNewPayload"      , BIP_HttpSocketState_eReceiveNewPayload         },
        { "ReceivingPayload"       , BIP_HttpSocketState_eReceivingPayload          },
        { "ReceivedPayload"        , BIP_HttpSocketState_eReceivedPayload           },
        { "Error"                  , BIP_HttpSocketState_eError                     },
        { "Destroying"             , BIP_HttpSocketState_eDestroying                },
        { "Max"                    , BIP_HttpSocketState_eMax                       },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpStreamerOutputState( int value)
{
    const namevalue_t myStrings[] = {
        { "NotSet" , BIP_HttpStreamerOutputState_eNotSet},
        { "Set"    , BIP_HttpStreamerOutputState_eSet   },
        { "Max"    , BIP_HttpStreamerOutputState_eMax   },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpStreamerProtocol( int value)
{
    const namevalue_t myStrings[] = {
        { "Direct"    , BIP_HttpStreamerProtocol_eDirect    },
        { "Hls"       , BIP_HttpStreamerProtocol_eHls       },
        { "MpegDash"  , BIP_HttpStreamerProtocol_eMpegDash  },
        { "Max"       , BIP_HttpStreamerProtocol_eMax       },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpStreamerResponseHeadersState( int value)
{
    const namevalue_t myStrings[] = {
        { "NotSet" , BIP_HttpStreamerResponseHeadersState_eNotSet},
        { "Set"    , BIP_HttpStreamerResponseHeadersState_eSet   },
        { "Max"    , BIP_HttpStreamerResponseHeadersState_eMax   },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_HttpStreamerState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized"               , BIP_HttpStreamerState_eUninitialized              },
        { "Idle"                        , BIP_HttpStreamerState_eIdle                       },
        { "SetupComplete"               , BIP_HttpStreamerState_eSetupComplete              },
        { "WaitingForProcessRequestApi" , BIP_HttpStreamerState_eWaitingForProcessRequestApi},
        { "Streaming"                   , BIP_HttpStreamerState_eStreaming                  },
        { "StreamingDone"               , BIP_HttpStreamerState_eStreamingDone              },
        { "WaitingForStopApi"           , BIP_HttpStreamerState_eWaitingForStopApi          },
        { "Max"                         , BIP_HttpStreamerState_eMax                        },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_IgmpListener_MembershipReportType( int value)
{
    const namevalue_t myStrings[] = {

        { "Unknown"  , BIP_IgmpListener_MembershipReportType_eUnknown },
        { "Join"     , BIP_IgmpListener_MembershipReportType_eJoin    },
        { "Leave"    , BIP_IgmpListener_MembershipReportType_eLeave   },
        { "Max"      , BIP_IgmpListener_MembershipReportType_eMax     },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_IoCheckerEvent( int value)
{
    const namevalue_t myStrings[] = {
        { "PollIn"    , BIP_IoCheckerEvent_ePollIn    },
        { "PollPri"   , BIP_IoCheckerEvent_ePollPri   },
        { "PollOut"   , BIP_IoCheckerEvent_ePollOut   },
        { "PollRdHup" , BIP_IoCheckerEvent_ePollRdHup },
        { "PollError" , BIP_IoCheckerEvent_ePollError },
        { "PollHup"   , BIP_IoCheckerEvent_ePollHup   },
        { "PollNvall" , BIP_IoCheckerEvent_ePollNvall },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_ListenerCallbackState( int value)
{
    const namevalue_t myStrings[] = {
        { "Disabled" , BIP_ListenerCallbackState_eDisabled },
        { "Enabled"  , BIP_ListenerCallbackState_eEnabled  },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_MediaInfoNavCreated( int value)
{
    const namevalue_t myStrings[] = {
        { "Unknown" , BIP_MediaInfoNavCreated_eUnknown},
        { "True"    , BIP_MediaInfoNavCreated_eTrue},
        { "False"   , BIP_MediaInfoNavCreated_eFalse},
        { "Max"     , BIP_MediaInfoNavCreated_eMax},
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_MediaInfoTrackType( int value)
{
    const namevalue_t myStrings[] = {
        {"Video", BIP_MediaInfoTrackType_eVideo},
        {"Audio", BIP_MediaInfoTrackType_eAudio},
        {"Pcr"  , BIP_MediaInfoTrackType_ePcr},
        {"Pmt"  , BIP_MediaInfoTrackType_ePmt},
        {"Other", BIP_MediaInfoTrackType_eOther},
        {"Max"  , BIP_MediaInfoTrackType_eMax},
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_NetworkAddressType( int value)
{
    const namevalue_t myStrings[] = {
        { "IpV4"           , BIP_NetworkAddressType_eIpV4           },
        { "IpV6"           , BIP_NetworkAddressType_eIpV6           },
        { "IpV6_and_IpV4"  , BIP_NetworkAddressType_eIpV6_and_IpV4  },
        { "IpV6_over_IpV4" , BIP_NetworkAddressType_eIpV6_over_IpV4 },
        { "Max"            , BIP_NetworkAddressType_eMax            },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_RtspIgmpMemRepStatus( int value)
{
    const namevalue_t myStrings[] = {
        { "None"    , BIP_RtspIgmpMemRepStatus_eNone    },
        { "Join"    , BIP_RtspIgmpMemRepStatus_eJoin    },
        { "Leave"   , BIP_RtspIgmpMemRepStatus_eLeave   },
        { "Invalid" , BIP_RtspIgmpMemRepStatus_eInvalid },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_RtspLiveMediaStreamingMode( int value)
{
    const namevalue_t myStrings[] = {
            { "RTP_UDP" , BIP_StreamingMode_eRTP_UDP  },
            { "RTP_TCP" , BIP_StreamingMode_eRTP_TCP  },
            { "RAW_UDP" , BIP_StreamingMode_eRAW_UDP  },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_RtspRequestMethod( int value)
{
    const namevalue_t myStrings[] = {
        { "Options"      , BIP_RtspRequestMethod_eOptions       },
        { "Describe"     , BIP_RtspRequestMethod_eDescribe      },
        { "Setup"        , BIP_RtspRequestMethod_eSetup         },
        { "Play"         , BIP_RtspRequestMethod_ePlay          },
        { "PlayWithUrl"  , BIP_RtspRequestMethod_ePlayWithUrl   },
        { "Teardown"     , BIP_RtspRequestMethod_eTeardown      },
        { "Join"         , BIP_RtspRequestMethod_eJoin          },
        { "Seek"         , BIP_RtspRequestMethod_eSeek          },
        { "GetParameter" , BIP_RtspRequestMethod_eGetParameter  },
        { "Max"          , BIP_RtspRequestMethod_eMax           },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_RtspResponseStatus( int value)
{
    const namevalue_t myStrings[] = {
        { "Success"      , BIP_RtspResponseStatus_eSuccess      },
        { "ClientError"  , BIP_RtspResponseStatus_eClientError  },
        { "ServerError"  , BIP_RtspResponseStatus_eServerError  },
        { "Invalid"      , BIP_RtspResponseStatus_eInvalid      },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_RtspSocketState( int value)
{
    const namevalue_t myStrings[] = {
        { "Idle"           , BIP_RtspSocketState_eIdle          },
        { "MessagePending" , BIP_RtspSocketState_eMessagePending},
        { "Error"          , BIP_RtspSocketState_eError         },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_SocketCallbackState( int value)
{
    const namevalue_t myStrings[] = {
        { "Disabled" , BIP_SocketCallbackState_eDisabled  },
        { "Enabled"  , BIP_SocketCallbackState_eEnabled   },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_SocketType( int value)
{
    const namevalue_t myStrings[] = {
        { "Tcp"     , BIP_SocketType_eTcp},
        { "UdpTx"   , BIP_SocketType_eUdpTx},
        { "UdpRx"   , BIP_SocketType_eUdpRx},
        { "Max"     , BIP_SocketType_eMax},
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerInputState( int value)
{
    const namevalue_t myStrings[] = {
        { "NotSet" , BIP_StreamerInputState_eNotSet },
        { "Set"    , BIP_StreamerInputState_eSet    },
        { "Max"    , BIP_StreamerInputState_eMax    },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerInputType( int value)
{
    const namevalue_t myStrings[] = {
        { "File"    , BIP_StreamerInputType_eFile    },
        { "Tuner"   , BIP_StreamerInputType_eTuner   },
        { "Bfile"   , BIP_StreamerInputType_eBfile   },
        { "Ip"      , BIP_StreamerInputType_eIp      },
        { "Recpump" , BIP_StreamerInputType_eRecpump },
        { "Max"     , BIP_StreamerInputType_eMax     },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerMpeg2TsPatPmtMode( int value)
{
    const namevalue_t myStrings[] = {
        { "Auto"             , BIP_StreamerMpeg2TsPatPmtMode_eAuto              },
        { "PassThruAsTracks" , BIP_StreamerMpeg2TsPatPmtMode_ePassThruAsTracks  },
        { "InsertCustom"     , BIP_StreamerMpeg2TsPatPmtMode_eInsertCustom      },
        { "Disable"          , BIP_StreamerMpeg2TsPatPmtMode_eDisable           },
        { "Max"              , BIP_StreamerMpeg2TsPatPmtMode_eMax               },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerOutputState( int value)
{
    const namevalue_t myStrings[] = {
        { "NotSet" , BIP_StreamerOutputState_eNotSet},
        { "Set"    , BIP_StreamerOutputState_eSet   },
        { "Max"    , BIP_StreamerOutputState_eMax   },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerProtocol( int value)
{
    const namevalue_t myStrings[] = {
        { "Tcp"      , BIP_StreamerProtocol_eTcp      },
        { "PlainUdp" , BIP_StreamerProtocol_ePlainUdp },
        { "Rtp"      , BIP_StreamerProtocol_eRtp      },
        { "Max"      , BIP_StreamerProtocol_eMax      },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_StreamerState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized"     , BIP_StreamerState_eUninitialized    },
        { "Idle"              , BIP_StreamerState_eIdle             },
        { "Prepared"          , BIP_StreamerState_ePrepared         },
        { "Streaming"         , BIP_StreamerState_eStreaming        },
        { "StreamingDone"     , BIP_StreamerState_eStreamingDone    },
        { "WaitingForStopApi" , BIP_StreamerState_eWaitingForStopApi},
        { "Max"               , BIP_StreamerState_eMax              },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_TranscodeState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized" , BIP_TranscodeState_eUninitialized   },
        { "Idle"          , BIP_TranscodeState_eIdle            },
        { "Prepared"      , BIP_TranscodeState_ePrepared        },
        { "Transcoding"   , BIP_TranscodeState_eTranscoding     },
        { "Max"           , BIP_TranscodeState_eMax             },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_UdpStreamerOutputState( int value)
{
    const namevalue_t myStrings[] = {
        { "NotSet" ,     BIP_UdpStreamerOutputState_eNotSet},
        { "Set"    ,     BIP_UdpStreamerOutputState_eSet   },
        { "Max"    ,     BIP_UdpStreamerOutputState_eMax   },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_UdpStreamerProtocol( int value)
{
    const namevalue_t myStrings[] = {
        { "PlainUdp"  , BIP_UdpStreamerProtocol_ePlainUdp },
        { "Rtp"       , BIP_UdpStreamerProtocol_eRtp      },
        { "Max"       , BIP_UdpStreamerProtocol_eMax      },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_UdpStreamerState( int value)
{
    const namevalue_t myStrings[] = {
        { "Uninitialized"     ,     BIP_UdpStreamerState_eUninitialized     },
        { "Idle"              ,     BIP_UdpStreamerState_eIdle              },
        { "SetupComplete"     ,     BIP_UdpStreamerState_eSetupComplete     },
        { "Streaming"         ,     BIP_UdpStreamerState_eStreaming         },
        { "StreamingDone"     ,     BIP_UdpStreamerState_eStreamingDone     },
        { "WaitingForStopApi" ,     BIP_UdpStreamerState_eWaitingForStopApi },
        { "Max"               ,     BIP_UdpStreamerState_eMax               },
        {NULL, 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerContainerType( int value)
{
    const namevalue_t myStrings[] = {
        { "BaseNexusContainerType"  , BIP_PlayerContainerType_eNexusTransportType   },
        { "HLS"                     , BIP_PlayerContainerType_eHls                  },
        { "MPEG-DASH"               , BIP_PlayerContainerType_eMpegDash             },
        { "Max"                     , BIP_PlayerContainerType_eMax                  },
        { "NotDefined", 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerDataAvailabilityModel( int value)
{
    const namevalue_t myStrings[] = {
        { "AutoDetermined"                      , BIP_PlayerDataAvailabilityModel_eAuto                 },
        { "NoRandomAccess"                      , BIP_PlayerDataAvailabilityModel_eNoRandomAccess       },
        { "LimitedRandomAccess"                 , BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess  },
        { "FullRandomAccess"                    , BIP_PlayerDataAvailabilityModel_eFullRandomAccess     },
        { "Max"                                 , BIP_PlayerDataAvailabilityModel_eMax                  },
        { "NotDefined", 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerClockRecoveryMode( int value)
{
    const namevalue_t myStrings[] = {
        { "Invalid"                 , BIP_PlayerClockRecoveryMode_eInvalid                  },
        { "Pull"                    , BIP_PlayerClockRecoveryMode_ePull                     },
        { "PushWithPcrSyncSlip"     , BIP_PlayerClockRecoveryMode_ePushWithPcrSyncSlip      },
        { "PushWithPcrNoSyncSlip"   , BIP_PlayerClockRecoveryMode_ePushWithPcrNoSyncSlip    },
        { "PushWithTtsNoSyncSlip"   , BIP_PlayerClockRecoveryMode_ePushWithTtsNoSyncSlip    },
        { "Max"                     , BIP_PlayerClockRecoveryMode_eMax                      },
        { "NotDefined", 0}
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerState( int value)
{
    const namevalue_t myStrings[] = {
        { "Disconnected"            , BIP_PlayerState_eDisconnected   },
        { "Connecting"              , BIP_PlayerState_eConnecting     },
        { "Connected"               , BIP_PlayerState_eConnected      },
        { "Probing"                 , BIP_PlayerState_eProbing        },
        { "Preparing"               , BIP_PlayerState_ePreparing      },
        { "Prepared"                , BIP_PlayerState_ePrepared       },
        { "Starting"                , BIP_PlayerState_eStarting       },
        { "Started"                 , BIP_PlayerState_eStarted        },
        { "Paused"                  , BIP_PlayerState_ePaused         },
        { "Aborted"                 , BIP_PlayerState_eAborted        },
        { "Max"                     , BIP_PlayerState_eMax            }
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerSubState( int value)
{
    const namevalue_t myStrings[] = {
        { "Idle"                                , BIP_PlayerSubState_eIdle                                  },
        { "ConnectingNew"                       , BIP_PlayerSubState_eConnectingNew                         },
        { "ConnectingWaitForResponse"           , BIP_PlayerSubState_eConnectingWaitForResponse             },
        { "ConnectingWaitForDtcpIpAke"          , BIP_PlayerSubState_eConnectingWaitForDtcpIpAke            },
        { "ConnectingDone"                      , BIP_PlayerSubState_eConnectingDone                        },
        { "ProbingNew"                          , BIP_PlayerSubState_eProbingNew                            },
        { "ProbingWaitForMediaInfo"             , BIP_PlayerSubState_eProbingWaitForMediaInfo               },
        { "ProbingDone"                         , BIP_PlayerSubState_eProbingDone                           },
        { "PreparingNew"                        , BIP_PlayerSubState_ePreparingNew                          },
        { "PreparingWaitForMediaInfo"           , BIP_PlayerSubState_ePreparingWaitForMediaInfo             },
        { "PreparingMediaInfoAvaialble"         , BIP_PlayerSubState_ePreparingMediaInfoAvailable           },
        { "PreparingDone"                       , BIP_PlayerSubState_ePreparingDone                         },
        { "StartingNew"                         , BIP_PlayerSubState_eStartingNew                           },
        { "StartingWaitForPbipStart"            , BIP_PlayerSubState_eStartingWaitForPbipStart              },
        { "StartingDone"                        , BIP_PlayerSubState_eStartingDone                          },
        { "StartedPlayingNormal"                , BIP_PlayerSubState_eStartedPlayingNormal                  },
        { "StartedPlayingTrickmode"             , BIP_PlayerSubState_eStartedPlayingTrickmode               },
        { "StartedWaitForPauseCompletion"       , BIP_PlayerSubState_eStartedWaitForPauseCompletion         },
        { "StartedWaitForSeekCompletion"        , BIP_PlayerSubState_eStartedWaitForSeekCompletion          },
        { "StartedWaitForPlayCompletion"        , BIP_PlayerSubState_eStartedWaitForPlayCompletion          },
        { "StartedWaitForPlayAtRateCompletion"  , BIP_PlayerSubState_eStartedWaitForPlayAtRateCompletion    },
        { "StartedWaitForStopCompletion"        , BIP_PlayerSubState_eStartedWaitForStopCompletion          },
        { "Paused"                              , BIP_PlayerSubState_ePaused                                },
        { "PausedWaitForPlayCompletion"         , BIP_PlayerSubState_ePausedWaitForPlayCompletion           },
        { "PausedWaitForPlayByFrameCompletion"  , BIP_PlayerSubState_ePausedWaitForPlayByFrameCompletion    },
        { "PausedWaitForSeekCompletion"         , BIP_PlayerSubState_ePausedWaitForSeekCompletion           },
        { "PausedWaitForPlayAtRateCompletion"   , BIP_PlayerSubState_ePausedWaitForPlayAtRateCompletion     },
        { "PausedWaitForStopCompletion"         , BIP_PlayerSubState_ePausedWaitForStopCompletion           },
        { "Aborted"                             , BIP_PlayerSubState_eAborted                               },
        { "AbortedWaitForStopCompletion"        , BIP_PlayerSubState_eAbortedWaitForStopCompletion          },
        { "DisconnectingNew"                    , BIP_PlayerSubState_eDisconnectingNew                      },
        { "Max"                                 , BIP_PlayerSubState_eMax                                   }
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState( int value)
{
    const namevalue_t myStrings[] = {
        { "Idle"                    , BIP_DtcpIpClientFactoryAkeEntryState_eIdle               },
        { "NewAke"                  , BIP_DtcpIpClientFactoryAkeEntryState_eNewAke             },
        { "WaitingForAke"           , BIP_DtcpIpClientFactoryAkeEntryState_eWaitingForAke      },
        { "AkeDone"                 , BIP_DtcpIpClientFactoryAkeEntryState_eAkeDone            },
        { "Max"                     , BIP_DtcpIpClientFactoryAkeEntryState_eMax                }
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_PlayerMode( int value)
{
    const namevalue_t myStrings[] = {
        { "VideoOnly"                       , BIP_PlayerMode_eVideoOnlyDecode               },
        { "AudioOnly"                       , BIP_PlayerMode_eAudioOnlyDecode               },
        { "AudioVideoDecode"                , BIP_PlayerMode_eAudioVideoDecode              },
        { "Record"                          , BIP_PlayerMode_eRecord                        },
    };
    return (lookup_name(myStrings, (value)));
}

const char * BIP_ToStr_BIP_MediaInfoType(int value)
{
    const namevalue_t myStrings[] = {
        { "stream"  ,  BIP_MediaInfoType_eStream },
        { "unknown" ,  BIP_MediaInfoType_eUnknown},
    };
    return (lookup_name(myStrings, (value)));
}

/* Here are some BIP_ToStr_ wrappers for the NEXUS enums. */

const char * BIP_ToStr_NEXUS_VideoFormat(int value)                     { return( lookup_name(g_videoFormatStrs, (value)         )); }
const char * BIP_ToStr_NEXUS_VideoFrameRate(int value)                  { return( lookup_name(g_videoFrameRateStrs, (value)      )); }
const char * BIP_ToStr_NEXUS_TransportType(int value)                   { return( lookup_name(g_transportTypeStrs, (value)       )); }
const char * BIP_ToStr_NEXUS_VideoCodec(int value)                      { return( lookup_name(g_videoCodecStrs, (value)          )); }
const char * BIP_ToStr_NEXUS_VideoCodecProfile(int value)               { return( lookup_name(g_videoCodecProfileStrs, (value)   )); }
const char * BIP_ToStr_NEXUS_VideoCodecLevel(int value)                 { return( lookup_name(g_videoCodecLevelStrs, (value)     )); }
const char * BIP_ToStr_NEXUS_AudioCodec(int value)                      { return( lookup_name(g_audioCodecStrs, (value)          )); }
const char * BIP_ToStr_NEXUS_StcChannelAutoModeBehavior(int value)      { return( lookup_name(g_stcChannelMasterStrs, (value)    )); }
const char * BIP_ToStr_NEXUS_PlaybackLoopMode(int value)                { return( lookup_name(g_endOfStreamActionStrs, (value)   )); }
const char * BIP_ToStr_NEXUS_TransportTimestampType(int value)          { return( lookup_name(g_tsTimestampType, (value)         )); }
const char * BIP_ToStr_NEXUS_VideoWindowContentMode(int value)          { return( lookup_name(g_contentModeStrs, (value)         )); }
const char * BIP_ToStr_NEXUS_FrontendVsbMode(int value)                 { return( lookup_name(g_vsbModeStrs, (value)             )); }
const char * BIP_ToStr_NEXUS_FrontendQamMode(int value)                 { return( lookup_name(g_qamModeStrs, (value)             )); }
const char * BIP_ToStr_NEXUS_FrontendOfdmMode(int value)                { return( lookup_name(g_ofdmModeStrs, (value)            )); }
const char * BIP_ToStr_NEXUS_FrontendSatelliteMode(int value)           { return( lookup_name(g_satModeStrs, (value)             )); }
const char * BIP_ToStr_NEXUS_FrontendDiseqcVoltage(int value)           { return( lookup_name(g_diseqcVoltageStrs, (value)       )); }
const char * BIP_ToStr_NEXUS_FrontendSatelliteNetworkSpec(int value)    { return( lookup_name(g_satNetworkSpecStrs, (value)      )); }
const char * BIP_ToStr_NEXUS_FrontendSatelliteNyquistFilter(int value)  { return( lookup_name(g_satNetworkSpecStrs, (value)      )); }
const char * BIP_ToStr_NEXUS_FrontendDvbt2Profile(int value)            { return( lookup_name(g_dvbt2ProfileStrs, (value)        )); }
const char * BIP_ToStr_NEXUS_VideoDecoderErrorHandling(int value)       { return( lookup_name(g_videoErrorHandling, (value)      )); }
const char * BIP_ToStr_NEXUS_VideoOrientation(int value)                { return( lookup_name(g_videoOrientation, (value)        )); }
const char * BIP_ToStr_NEXUS_Display3DSourceBuffer(int value)           { return( lookup_name(g_videoSourceBuffer, (value)       )); }
const char * BIP_ToStr_NEXUS_VideoDecoderSourceOrientation(int value)   { return( lookup_name(g_sourceOrientation, (value)       )); }
const char * BIP_ToStr_NEXUS_DisplayAspectRatio(int value)              { return( lookup_name(g_displayAspectRatioStrs, (value)  )); }
const char * BIP_ToStr_NEXUS_SecurityAlgorithm(int value)               { return( lookup_name(g_securityAlgoStrs, (value)        )); }
const char * BIP_ToStr_NEXUS_PlatformStandbyMode(int value)             { return( lookup_name(g_platformStandbyModeStrs, (value) )); }
const char * BIP_ToStr_NEXUS_ColorSpace(int value)                      { return( lookup_name(g_colorSpaceStrs, (value)          )); }
const char * BIP_ToStr_NEXUS_AudioLoudnessEquivalenceMode(int value)    { return( lookup_name(g_audioLoudnessStrs, (value)       )); }
const char * BIP_ToStr_NEXUS_AudioChannelMode(int value)                { return( lookup_name(g_audioChannelModeStrs, (value)    )); }
const char * BIP_ToStr_NEXUS_AudioDecoderDolbyDrcMode(int value)        { return( lookup_name(g_dolbyDrcModeStrs, (value)        )); }
