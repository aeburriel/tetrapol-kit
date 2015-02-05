#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    TETRAPOL_BAND_VHF = 1,
    TETRAPOL_BAND_UHF = 2,
};

/** Radio channel type. */
enum {
    TETRAPOL_CONTROL_RCH = 1,
    TETRAPOL_TRAFFIC_RCH = 2,
};

#ifdef __cplusplus
}
#endif
