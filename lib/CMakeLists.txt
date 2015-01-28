find_library(CMOCKA_LIBRARY
    NAMES
    cmocka)

add_library (tetrapol
    misc.c
    multiblock.c
    phys_ch.c
    tpdu.c
    tsdu.c)

add_executable (test_phys_ch
    misc.c
    multiblock.c
    test_phys_ch.c
    tpdu.c
    tsdu.c)
target_link_libraries (test_phys_ch ${CMOCKA_LIBRARY})

add_test(test_phys_ch ${CMAKE_CURRENT_BINARY_DIR}/test_phys_ch)
