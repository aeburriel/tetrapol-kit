#pragma once

#include "decoded_frame.h"

#define TPDU_DATA_SIZE_MAX ((64 * 8))

typedef struct _data_frame_t data_frame_t;

/**
  Create new data frame instance.
  */
data_frame_t *data_frame_create(void);

/**
  Reset internal state of data frame decoder.

  Must be called after data_frame_get_tpdu_data(), before new decoded frame is pushed in.
  */
void data_frame_reset(data_frame_t *data_fr);

/**
  Pass new decoded frame into data frame processing chain.

  @return true if new data frame(s) is/are available, false otherwise.
  */
bool data_frame_push_decoded_frame(data_frame_t *data_fr, decoded_frame_t *df);

/**
  Copy data frame into output buffer.

  @return size of result (with one bit per byte) or -1 when no more TPDUs are available
    or some error occured.
  */
int data_frame_get_tpdu_data(data_frame_t *data_fr, uint8_t *tpdu_data);

/**
  Destroy data frame instance
  */
void data_frame_destroy(data_frame_t *data_fr);

