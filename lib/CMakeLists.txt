find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

add_library (tetrapol
    addr.c
    bch.c
    data_block.c
    data_frame.c
    hdlc_frame.c
    misc.c
    multiblock.c
    phys_ch.c
    tpdu.c
    tsdu.c)

add_executable (test_data_block
    test_data_block.c)
target_link_libraries (test_data_block ${CMOCKA_LIBRARY})

add_executable (test_data_frame
    data_block.c
    test_data_frame.c)
target_link_libraries (test_data_frame ${CMOCKA_LIBRARY})

add_executable (test_hdlc_frame
    test_hdlc_frame.c)
target_link_libraries (test_hdlc_frame ${CMOCKA_LIBRARY})

add_executable (test_phys_ch
    addr.c
    bch.c
    data_block.c
    data_frame.c
    hdlc_frame.c
    misc.c
    multiblock.c
    test_phys_ch.c
    tpdu.c
    tsdu.c)
target_link_libraries (test_phys_ch ${CMOCKA_LIBRARY})

add_test(test_data_block ${CMAKE_CURRENT_BINARY_DIR}/test_data_block)
add_test(test_data_frame ${CMAKE_CURRENT_BINARY_DIR}/test_data_frame)
add_test(test_hdlc_frame ${CMAKE_CURRENT_BINARY_DIR}/test_hdlc_frame)
add_test(test_phys_ch ${CMAKE_CURRENT_BINARY_DIR}/test_phys_ch)
