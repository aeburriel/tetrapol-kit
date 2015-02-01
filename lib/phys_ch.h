#pragma once

#include <stdint.h>
#include <stdbool.h>

#define FRAME_NO_UNKNOWN -1

#define PHYS_CH_SCR_DETECT -1

typedef struct _phys_ch_t phys_ch_t;

typedef enum {
    FRAME_TYPE_AUTO = -1,
    FRAME_TYPE_VOICE = 0,
    FRAME_TYPE_DATA = 1,
    FRAME_TYPE_HR_DATA,
    FRAME_TYPE_RANDOM_ACCESS,
    FRAME_TYPE_TRAINING,
    FRAME_TYPE_DM_EMERGENCY,
    FRAME_TYPE_SCH_TI,
} frame_type_t;

// now only data frame, in future might comprise different types of frame
typedef struct {
    int frame_no;
    union {
        uint8_t data[74];
        uint8_t _tmpd[76];  // extra space, data frame have 2 stuffing bist
    };
    union {
        uint8_t err[74];
        uint8_t _tmpe[76];  // extra space, data frame have 2 stuffing bist
    };
} decoded_frame_t;

/**
  Create new TETRAPOL physical cahnnel instance.
  @param band VHF or UHF
  @param rch_type Radio channel type, control or traffic.

  @return net phys_ch_t instance of NULL.
  */
phys_ch_t *tetrapol_phys_ch_create(int band, int rch_type);
void tetrapol_phys_ch_destroy(phys_ch_t *phys_ch);
int tetrapol_phys_ch_process(phys_ch_t *phys_ch);

/**
  Get SCR, scrambling constant parameter.
  */
int tetrapol_phys_ch_get_scr(phys_ch_t *phys_ch);
/**
  Set SCR, scrambling constant parameter.
  */
void tetrapol_phys_ch_set_scr(phys_ch_t *phys_ch, int scr);

/**
  Get confidence for SRC detection (~ no. of valid frames).
  */
int tetrapol_phys_ch_get_scr_confidence(phys_ch_t *phys_ch);

/**
  Set confidence for SRC detection (~ no. of valid frames).
  */
void tetrapol_phys_ch_set_scr_confidence(phys_ch_t *phys_ch, int scr_confidence);

/**
  Eat some data from buf into channel decoder.

  @return number of bytes consumed
*/
int tetrapol_recv2(phys_ch_t *phys_ch, uint8_t *buf, int len);

bool decoded_frame_check_crc(const decoded_frame_t *df, frame_type_t df_type);

