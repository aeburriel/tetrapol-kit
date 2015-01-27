#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _tetrapol_phys_ch_t tetrapol_phys_ch_t;

    tetrapol_phys_ch_t *tetrapol_create(int fd);
    void tetrapol_destroy(tetrapol_phys_ch_t *t);
    int tetrapol_main(tetrapol_phys_ch_t *t);

#ifdef __cplusplus
}
#endif
