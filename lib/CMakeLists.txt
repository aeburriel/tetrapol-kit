find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

add_library (tetrapol
    data_frame.c
    decoded_frame.c
    hdlc_frame.c
    misc.c
    multiblock.c
    phys_ch.c
    tpdu.c
    tsdu.c)

add_executable (test_decoded_frame
    test_decoded_frame.c)
target_link_libraries (test_decoded_frame ${CMOCKA_LIBRARY})

add_executable (test_hdlc_frame
    test_hdlc_frame.c)
target_link_libraries (test_hdlc_frame ${CMOCKA_LIBRARY})

add_executable (test_phys_ch
    decoded_frame.c
    data_frame.c
    misc.c
    multiblock.c
    test_phys_ch.c
    tpdu.c
    tsdu.c)
target_link_libraries (test_phys_ch ${CMOCKA_LIBRARY})

add_test(test_decoded_frame ${CMAKE_CURRENT_BINARY_DIR}/test_decoded_frame)
add_test(test_hdlc_frame ${CMAKE_CURRENT_BINARY_DIR}/test_hdlc_frame)
add_test(test_phys_ch ${CMAKE_CURRENT_BINARY_DIR}/test_phys_ch)
