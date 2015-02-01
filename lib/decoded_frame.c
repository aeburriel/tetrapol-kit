#include "decoded_frame.h"

#include <stdio.h>
#include <string.h>

// http://ghsi.de/CRC/index.php?Polynom=10010
static void mk_crc5(uint8_t *res, const uint8_t *input, int input_len)
{
    uint8_t inv;
    memset(res, 0, 5);

    for (int i = 0; i < input_len; ++i)
    {
        inv = input[i] ^ res[0];

        res[0] = res[1];
        res[1] = res[2];
        res[2] = res[3] ^ inv;
        res[3] = res[4];
        res[4] = inv;
    }
}

bool decoded_frame_check_crc(const decoded_frame_t *df, frame_type_t df_type)
{
    if (df_type == FRAME_TYPE_AUTO) {
        df_type = df->data[0];
    } else {
        if (df_type != df->data[0]) {
            return false;
        }
    }

    if (df_type == FRAME_TYPE_DATA) {
        uint8_t crc[5];

        mk_crc5(crc, df->data, 69);
        return !memcmp(df->data + 69, crc, 5);
    }

    if (df_type == FRAME_TYPE_VOICE) {
        // TODO
        fprintf(stderr, "CRC checking for VOICE frames not implemented");
        return false;
    }
    return false;
}

