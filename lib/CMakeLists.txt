# FIXME. supports non-gcc compillers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

add_executable (tetrapol
    misc.c
    multiblock.c
    radio.c
    tetrapol.c
    tpdu.c
    tsdu.c)

