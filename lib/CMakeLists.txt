find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

add_library (tetrapol
    misc.c
    multiblock.c
    radio.c
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
