#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _tetrapol_t tetrapol_t;

    tetrapol_t *tetrapol_create(int fd);
    void tetrapol_destroy(tetrapol_t *t);
    int tetrapol_main(tetrapol_t *t);

#ifdef __cplusplus
}
#endif
