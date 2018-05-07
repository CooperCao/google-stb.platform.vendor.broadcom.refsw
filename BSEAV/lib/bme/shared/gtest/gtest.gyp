{
  'variables': {
#    'depth': '..',
  },
  'targets': [
    {
      'target_name': 'gtest',
      'type': 'static_library',
      'standalone_static_library': 1,
      'sources': [
        'gtest-all.cc',
        'gtest_main.cc'
      ],
      'include_dirs': [
        './../'
      ],
      'direct_dependent_settings': {
        'defines': [
          'UNIT_TEST',
        ],
        'include_dirs': [
          './../',
        ],
      },
      'all_dependent_settings': {
        'ldflags': [
          '-lpthread',
        ]
      },
    },
  ],
}
