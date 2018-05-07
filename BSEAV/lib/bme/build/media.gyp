{
    'includes': [
        "common.gypi"
    ],
    'targets': [
        {
            'target_name': 'All',
            'type': 'none',
            'dependencies': [
                './../shared/shared.gyp:*',
                './../player/player.gyp:*',
                './../samples/samples.gyp:*',
            ],
        },
    ]
}
