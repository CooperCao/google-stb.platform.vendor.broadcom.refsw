{
    'targets': [
        {

            'target_name': 'bmeShared',
            'type': 'static_library',
            'sources': [
                'Debug.cpp',
            ],
            'include_dirs': [
                './../../',
                'include',
            ],
            'cflags': [
                '-std=c++0x',
                '-DLINUX',
            ],
        },
    ]
}
