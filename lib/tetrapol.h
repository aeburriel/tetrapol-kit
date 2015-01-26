#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _tetrapol_phys_ch_t tetrapol_phys_ch_t;

    tetrapol_phys_ch_t *tetrapol_create(int fd);
    void tetrapol_destroy(tetrapol_phys_ch_t *t);
    int tetrapol_main(tetrapol_phys_ch_t *t);

    /**
      Eat some data from buf into channel decoder.

      @return number of bytes consumed
    */
    int tetrapol_recv2(tetrapol_phys_ch_t *t, uint8_t *buf, int len);

#ifdef __cplusplus
}
#endif
