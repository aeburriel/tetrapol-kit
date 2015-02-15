find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library (tetrapol
    addr.c
    bch.c
    bit_utils.c
    data_block.c
    data_frame.c
    hdlc_frame.c
    log.c
    misc.c
    phys_ch.c
    pch.c
    rch.c
    sdch.c
    timer.c
    tpdu.c
    tsdu.c
    tetrapol/addr.h
    tetrapol/bch.h
    tetrapol/bit_utils.h
    tetrapol/data_block.h
    tetrapol/data_frame.h
    tetrapol/hdlc_frame.h
    tetrapol/log.h
    tetrapol/misc.h
    tetrapol/phys_ch.h
    tetrapol/pch.h
    tetrapol/rch.h
    tetrapol/sdch.h
    tetrapol/system_config.h
    tetrapol/timer.h
    tetrapol/tpdu.h
    tetrapol/tsdu.h
)

add_executable (test_data_block
    log.c
    test_data_block.c)
target_link_libraries (test_data_block ${CMOCKA_LIBRARY})

add_executable (test_data_frame
    log.c
    data_block.c
    test_data_frame.c)
target_link_libraries (test_data_frame ${CMOCKA_LIBRARY})

add_executable (test_bit_utils
    test_bit_utils.c)
target_link_libraries (test_bit_utils ${CMOCKA_LIBRARY})

add_executable (test_timer
    log.c
    test_timer.c)
target_link_libraries (test_timer ${CMOCKA_LIBRARY})

add_executable (test_phys_ch
    addr.c
    bch.c
    bit_utils.c
    data_block.c
    data_frame.c
    hdlc_frame.c
    log.c
    misc.c
    pch.c
    rch.c
    sdch.c
    test_phys_ch.c
    timer.c
    tpdu.c
    tsdu.c)
target_link_libraries (test_phys_ch ${CMOCKA_LIBRARY})

add_test(test_data_block ${CMAKE_CURRENT_BINARY_DIR}/test_data_block)
add_test(test_data_frame ${CMAKE_CURRENT_BINARY_DIR}/test_data_frame)
add_test(test_bit_utils ${CMAKE_CURRENT_BINARY_DIR}/test_bit_utils)
add_test(test_phys_ch ${CMAKE_CURRENT_BINARY_DIR}/test_phys_ch)
add_test(test_timer ${CMAKE_CURRENT_BINARY_DIR}/test_timer)
