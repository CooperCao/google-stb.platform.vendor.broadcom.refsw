{
  'variables':  {
    'component%': 'static_library',
    'pkg-config': 'pkg-config'
  },
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
       'Release': {
           'cflags': ['-s', '-Os']
           },
       'Debug': {
           'cflags': ['-g', '-DBME_VERBOSE']
       },
    },
    'cflags': [
      '-fPIC',
      '-fno-rtti',
      '-Wno-unused-but-set-parameter',
    ],
  },
}
