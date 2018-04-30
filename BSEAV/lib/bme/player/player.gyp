{
    'includes': [
        "./../build/common.gypi"
    ],
    'targets': [
    {
        'target_name': 'bme',
        'type': 'shared_library',
        'sources': [
            'source/Media.cpp',
            'source/MediaPlayer.cpp',
            'source/ManualAnimate.cpp',
            'source/SimpleDecoderNxclient.cpp',
            'source/BaseSource.cpp',
            'source/PushTSSource.cpp',
            'source/PushESSource.cpp',
            'source/MediaDrmContext.cpp',
            'source/Playpump.cpp',
            'source/FileSource.cpp',
            'source/HdmiSource.cpp',
            'source/MediaProbe.cpp',
            'source/MediaStream.cpp',
            'source/AudioVolumeChange.cpp',
            'source/AudioPlayback.cpp',
            'source/WavFormatHeader.cpp',
            'source/MonitorSource.cpp',
            'source/HdrMetadata.cpp',
        ],
        'include_dirs': [
            './../',
            'include',
            './../shared/',
        ],
        'cflags': [
            '-fvisibility=hidden',
            '-fvisibility-inlines-hidden',
            '<(NEXUS_CFLAGS)',
            '-Wno-unused-parameter',
            '-Wno-variadic-macros',
            '-Wno-unused-local-typedefs',
            '-std=c++0x',
        ],
        'ldflags': [
            '<(NEXUS_LDFLAGS)',
        ],
        'conditions': [
            ['BME_ENABLE_TV=="y"', {
                'sources': [
                    'source/LiveSource.cpp',
                    'source/TimeshiftSource.cpp',
                ],
                'cflags': [
                    '-DBME_ENABLE_TV',
                ],
                'dependencies': [
                    #'./../tv/tv.gyp:*',
                ],
            }],
            ['SAGE_SUPPORT=="y"', {
                'cflags': [
                    '-DBRCM_SAGE',
                ],
                'ldflags': [
                    '<(SAGE_LDFLAGS)',
                ],
            }],
            ['BME_ENABLE_PLAYREADY=="y"', {
                'cflags': [
                    '-DENABLE_PLAYREADY',
                    '-DDRM_BUILD_PROFILE=900',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1',
                ],
                'conditions': [
                    ['PLAYREADY_VERSION_25=="y"', {
                        'cflags': [
                            '-DPRDY_SDK_V25',
                            '-DLIB_PLAYREADYPK25=<(LIB_PLAYREADYPK25)',
                        ],
                        'dependencies': [
                            'playReady25',
                        ]
                    }],
                    ['PLAYREADY_VERSION_32=="y"', {
                        'cflags': [
                            '-DPRDY_SDK_V30',
                            '-DLIB_PLAYREADYPK30=<(LIB_PLAYREADYPK30)',
                        ],
                        'dependencies': [
                            'playReady30',
                        ]
                    }],
                ],
            }],
            ['BME_ENABLE_WIDEVINE=="y"', {
                'libraries':[
                    '<(WIDEVINE_LDFLAGS)',
                ],
                'cflags': [
                    '-DENABLE_WIDEVINE',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DCERTI_PROVISIONING',
                    '-DSKIP_PEER_VERIFICATION',
                ],
                'include_dirs': [
                    '<(CENC_DIR)/cdm/include',
                    '<(CENC_DIR)/core/include',
                    '<(CENC_DIR)/oemcrypto/include',
                ]
            }],
        ],
        'dependencies': [
            './../shared/shared.gyp:*',
        ],
        'all_dependent_settings': {
            'ldflags': [
                '<(NEXUS_LDFLAGS)',
            ]
        },
    },
    {
        'target_name': 'bmeDrm',
        'type': 'shared_library',
        'sources': [
            'source/Drm.cpp',
            'source/MediaDrmAdaptor.cpp',
        ],
        'include_dirs': [
            './../',
            'include',
            './../shared/',
        ],
        'cflags': [
            '-fvisibility=hidden',
            '-fvisibility-inlines-hidden',
            '<(NEXUS_CFLAGS)',
            '-Wno-unused-parameter',
            '-Wno-variadic-macros',
            '-Wno-unused-local-typedefs',
            '-std=c++0x',
        ],
        'ldflags': [
            '<(NEXUS_LDFLAGS)',
            '<(BME_LDFLAGS)',
        ],
        'dependencies': [
            'bme',
        ],
        'conditions': [
            ['SAGE_SUPPORT=="y"', {
                'cflags': [
                    '-DBRCM_SAGE',
                ],
            }],
            ['BME_ENABLE_PLAYREADY=="y"', {
                'include_dirs': [
                    'source',
                ],
                'cflags': [
                    '-DENABLE_PLAYREADY',
                    '-DDRM_BUILD_PROFILE=900',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1',
                ],
                'conditions': [
                    ['PLAYREADY_VERSION_25=="y"', {
                        'sources': [
                            'source/DrmPlayReady.cpp',
                            'source/DrmPlayReady2x.cpp',
                        ],
                        'include_dirs': [
                            '<(BSEAV_TOP)/lib/security/common_drm/include/',
                            '<(BSEAV_TOP)/thirdparty/playready/2.5/inc',
                            '<(BSEAV_TOP)/lib/security/common_crypto/include/',
                        ],
                        'cflags': [
                            '-DPRDY_SDK_V25',
                        ],
                    }],
                ],
            }],
            ['BME_ENABLE_WIDEVINE=="y"', {
                'sources': [
                    'source/DrmWidevine.cpp',
                ],
                'include_dirs': [
                    '<(CENC_DIR)/cdm/include',
                    '<(CENC_DIR)/core/include',
                    '<(CENC_DIR)/oemcrypto/include',
                ],
                'cflags': [
                    '<(LIBCURL_CFLAGS)',
                    '-DENABLE_WIDEVINE',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DCERTI_PROVISIONING',
                    '-DSKIP_PEER_VERIFICATION',
                ],
            }],
        ],
        'all_dependent_settings': {
            'ldflags': [
                '<(NEXUS_LDFLAGS)',
                '<(BME_LDFLAGS)',
            ],
        },
    },
    {
        'target_name': 'playerTest',
        'type': 'executable',
        'sources': [
            'test/cpp/MediaTests.cpp',
            'test/cpp/NetworksourceTest.cpp',
            'test/cpp/PushTSSourceTest.cpp',
            'test/cpp/PushESSourceTest.cpp',
            'test/cpp/FileSourceTest.cpp',
            'test/cpp/MediaProbeTest.cpp',
            'test/cpp/HdmiSourceTest.cpp',
            #'test/cpp/PushYTSourceTest.cpp',
        ],
        'cflags': [
            '-std=c++0x',
            '<(NEXUS_CFLAGS)',
            '-Wno-variadic-macros',
        ],
        'ldflags': [
            #'-lcurl',
            #'-lz',
        ],
        'include_dirs': [
            'include',
            './../shared/',
        ],
        'dependencies': [
            'bme',
            'bmeDrm',
            './../shared/gtest/gtest.gyp:*',
        ],
    }

    ],
    'conditions': [
        ['BME_ENABLE_PLAYREADY=="y" and PLAYREADY_VERSION_25=="y"', {
            'targets': [
            {
                'target_name': 'playReady25',
                'type': 'static_library',
                'sources': [
                    'source/PlayReady25.cpp'
                ],
                'cflags': [
                    '-std=c++0x',
                    '<(NEXUS_CFLAGS)',
                    '-DPRDY_SDK_V25',
                    '-DENABLE_PLAYREADY',
                    '-DDRM_BUILD_PROFILE=900',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1',
                    '-DLIB_PLAYREADYPK25=<(LIB_PLAYREADYPK25)',
                ],
                'include_dirs': [
                    './../',
                    'include',
                    './../shared/',
                    '<(BSEAV_TOP)/thirdparty/playready/2.5/inc/',
                ],
            }],
        }],
        ['BME_ENABLE_PLAYREADY=="y" and PLAYREADY_VERSION_32=="y"', {
            'targets': [
            {
                'target_name': 'playReady30',
                'type': 'static_library',
                'sources': [
                    'source/PlayReady30.cpp'
                ],
                'cflags': [
                    '-std=c++0x',
                    '<(NEXUS_CFLAGS)',
                    '-DPRDY_SDK_V30',
                    '-DENABLE_PLAYREADY',
                    '-DDRM_BUILD_PROFILE=900',
                    '-DTARGET_LITTLE_ENDIAN=1',
                    '-DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1',
                    '-DLIB_PLAYREADYPK30=<(LIB_PLAYREADYPK30)',
                ],
                'include_dirs': [
                    './../',
                    'include',
                    './../shared/',
                    '<(BSEAV_TOP)/thirdparty/playready/3.0/inc/',
                ],
            }],
        }],
    ],
}
