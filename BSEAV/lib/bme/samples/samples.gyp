{
    'includes': [
        "./../build/common.gypi"
    ],
    'targets': [
    {
        'target_name': 'mediaplayer',
        'type': 'executable',
        'sources': [
            'player.cpp'
        ],
        'cflags': [
            '-std=c++0x',
        ],
        'include_dirs': [
            './../player/include',
            './../shared/',
        ],
        'dependencies': [
            './../player/player.gyp:bme',
            './../player/player.gyp:bmeDrm',
        ],
    },
    {
        'target_name': 'mosaicPlayer',
        'type': 'executable',
        'sources': [
            'mosaicPlayer.cpp'
        ],
        'cflags': [
            '-std=c++0x',
        ],
        'include_dirs': [
            './../player/include',
            './../shared/',
        ],
        'dependencies': [
            './../player/player.gyp:bme',
            './../player/player.gyp:bmeDrm',
        ],
    },
    {
        'target_name': 'texturePlayer',
        'type': 'executable',
        'sources': [
            'texturePlayer.cpp'
        ],
        'cflags': [
            '-std=c++0x',
            '<(NEXUS_CFLAGS)',
        ],
        'include_dirs': [
            './../player/include',
            './../shared/',
        ],
        'dependencies': [
            './../player/player.gyp:bme',
            './../player/player.gyp:bmeDrm',
        ],
    },
    {
        'target_name': 'pcmPlayer',
        'type': 'executable',
        'sources': [
            'pcmPlayer.cpp'
        ],
        'cflags': [
            '-std=c++0x',
        ],
        'include_dirs': [
            './../player/include',
            './../shared/',
        ],
        'dependencies': [
            './../player/player.gyp:bme',
            './../player/player.gyp:bmeDrm',
        ],
    }],
}
