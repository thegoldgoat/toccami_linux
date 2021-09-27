project('toccami_linux', 'c')

executable('toccami_linux',
    'src/main.c',
    'src/utils/utils.c',
    'src/udp/udp.c',
    install: true,
    c_args : [
        '-std=c11',
        '-I/usr/include/libevdev-1.0'
    ],
    dependencies: [
        dependency('libevdev'),
        dependency('threads')
    ]
)