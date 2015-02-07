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
    RADIO_CH_TYPE_CONTROL = 1,
    RADIO_CH_TYPE_TRAFFIC = 2,
};

#ifdef __cplusplus
}
#endif
