# FIXME. supports non-gcc compillers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")
# TODO: debug only
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb3")

find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

add_executable (tetrapol
    misc.c
    multiblock.c
    radio.c
    tetrapol.c
    tpdu.c
    tsdu.c)

add_executable (test_radio
    misc.c
    multiblock.c
    test_radio.c
    tpdu.c
    tsdu.c)
target_link_libraries (test_radio ${CMOCKA_LIBRARY})

add_test(test_radio ${CMAKE_CURRENT_BINARY_DIR}/test_radio)
