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
    TETRAPOL_RCH_CONTROL = 1,
    TETRAPOL_RCH_TRAFFIC = 2,
};

#ifdef __cplusplus
}
#endif
