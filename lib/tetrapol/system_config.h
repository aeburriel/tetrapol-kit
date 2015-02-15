#pragma once

/// system parameters
enum {
    // maximum numner of blocks (frame_t) per data_frame_t (excluding XOR block)
    SYS_PAR_DATA_FRAME_BLOCKS_MAX = 8,

    // maximum size of HDLC frame (using high-data rate)
    SYS_PAR_N200_BYTES_MAX = (92 * SYS_PAR_DATA_FRAME_BLOCKS_MAX + 7) / 8,

    // PAS 0001-3-3 9.8.2 maximum number of DUs for TSDU
    SYS_PAR_N452 = 64,

    // PAS 0001-3-3 9.8.4 timeout for receiving fragmented TSDU
    SYS_PAR_T454 = 10 * 1000000,
};
