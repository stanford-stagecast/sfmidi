cmake_minimum_required ( VERSION 2.8.5 )
cmake_policy ( SET CMP0054 NEW )
project ( SFMIDI )

include ( etc/build_defs.cmake )
include ( etc/build_type.cmake )
include ( etc/cflags.cmake )
include ( etc/clang_format.cmake )
include ( etc/clang_tidy.cmake )

find_package ( PkgConfig )

pkg_check_modules ( ALSA REQUIRED alsa )
include_directories ( ${ALSA_INCLUDE_DIRS} )
add_compile_options ( ${ALSA_CFLAGS_OTHER} )

file ( GLOB SFMIDI_SRC
    src/*
)

include_directories ( src )

add_library ( sfmidi STATIC
    ${SFMIDI_SRC}
)

set ( SFMIDI_LIBS
    sfmidi
)

add_executable ( midi-test src/frontend/midi-test.cc )
target_link_libraries ( midi-test ${SFMIDI_LIBS} )
